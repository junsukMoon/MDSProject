param(
    [int]$Port = 7993,
    [int]$StartupTimeoutSeconds = 90,
    [int]$FlowTimeoutSeconds = 30
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$ServerExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "R15_SlowMoRecovery_Server.log"
$ClientLog = Join-Path $LogDir "R15_SlowMoRecovery_Client.log"

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class MDSModalUIWin32
{
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdc, uint flags);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
}
"@

function Start-MDSProcess {
    param([string]$Exe, [string]$Arguments, [string]$WorkingDirectory)
    $Info = New-Object System.Diagnostics.ProcessStartInfo
    $Info.FileName = $Exe
    $Info.Arguments = $Arguments
    $Info.WorkingDirectory = $WorkingDirectory
    $Info.UseShellExecute = $false
    return [System.Diagnostics.Process]::Start($Info)
}

function Stop-MDSProcess {
    param([System.Diagnostics.Process]$Process)
    if ($Process -and -not $Process.HasExited) {
        $Process.Kill()
        $Process.WaitForExit(5000) | Out-Null
    }
}

function Wait-LogPattern {
    param([string]$Path, [string]$Pattern, [int]$TimeoutSeconds)
    $Deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while ([DateTime]::UtcNow -lt $Deadline) {
        if ((Test-Path -LiteralPath $Path) -and (Select-String -LiteralPath $Path -Pattern $Pattern -Quiet)) { return $true }
        Start-Sleep -Milliseconds 250
    }
    return $false
}

function Get-LogPatternCount {
    param([string]$Path, [string]$Pattern)
    if (-not (Test-Path -LiteralPath $Path)) { return 0 }
    return @(Select-String -LiteralPath $Path -Pattern $Pattern).Count
}

function Wait-LogPatternCount {
    param([string]$Path, [string]$Pattern, [int]$MinimumCount, [int]$TimeoutSeconds)
    $Deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while ([DateTime]::UtcNow -lt $Deadline) {
        if ((Get-LogPatternCount $Path $Pattern) -ge $MinimumCount) { return $true }
        Start-Sleep -Milliseconds 250
    }
    return $false
}

function Get-ClientWindow {
    param([System.Diagnostics.Process]$Process, [int]$TimeoutSeconds)
    $Deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while ([DateTime]::UtcNow -lt $Deadline) {
        $Process.Refresh()
        if ($Process.MainWindowHandle -ne [IntPtr]::Zero) { return $Process.MainWindowHandle }
        Start-Sleep -Milliseconds 250
    }
    throw "Client window was not created."
}

function Capture-Window {
    param([IntPtr]$Handle, [string]$Path)
    $Rect = New-Object MDSModalUIWin32+RECT
    [MDSModalUIWin32]::GetWindowRect($Handle, [ref]$Rect) | Out-Null
    $Width = [Math]::Max(1, $Rect.Right - $Rect.Left)
    $Height = [Math]::Max(1, $Rect.Bottom - $Rect.Top)
    $Bitmap = New-Object System.Drawing.Bitmap $Width, $Height
    $Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $Hdc = $Graphics.GetHdc()
    try { [MDSModalUIWin32]::PrintWindow($Handle, $Hdc, 2) | Out-Null }
    finally { $Graphics.ReleaseHdc($Hdc); $Graphics.Dispose() }
    $Bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    $Bitmap.Dispose()
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog,$ClientLog -Force -ErrorAction SilentlyContinue
if (-not (Test-Path $ServerExe) -or -not (Test-Path $ClientExe)) { throw "Server or staged client executable is missing." }

$ServerArgs = "`"$ProjectFile`" /Game/TopDown/Lvl_TopDown -server -NullRHI -unattended -DDC-ForceMemoryCache -forcelogflush -abslog=`"$ServerLog`" MDSWaveMaxCount=1 MDSWaveInitialEnemyCount=2 MDSWaveEnemyIncrement=0 MDSWaveIntermission=2 MDSSettlementDuration=20 MDSActorBaselineMoveSpeed=0 MDSKillCurrency=50 MDSKillExperience=100 -port=$Port"
$ClientArgs = "127.0.0.1:$Port -NullRHI -unattended -nosound -NoSplash -DDC-ForceMemoryCache -forcelogflush -abslog=`"$ClientLog`" -MDSAutoAttackNearestEnemy MDSAutoAttackCount=20 MDSAutoAttackDelay=2 MDSAutoAttackRetryInterval=0.35 MDSAttackDamage=25 MDSAttackRange=5000 -MDSVerifyClickLevelUpWidget -MDSCombatPresentationLog"
$Server = $null
$Client = $null
try {
    $Server = Start-MDSProcess $ServerExe $ServerArgs (Split-Path -Parent $ServerExe)
    if (-not (Wait-LogPattern $ServerLog "IpNetDriver listening on port" $StartupTimeoutSeconds)) { throw "Server did not start listening." }
    $Client = Start-MDSProcess $ClientExe $ClientArgs (Split-Path -Parent $ClientExe)
    if (-not (Wait-LogPattern $ServerLog "TransitionInScheduled .* RealDuration=2.00" $FlowTimeoutSeconds)) { throw "Level-up transition was not scheduled for two real seconds." }
    $LevelUpTransitionObservedAt = [DateTime]::UtcNow
    if (-not (Wait-LogPattern $ServerLog "FlowState=EMDSLevelUpFlowState::Selection" $FlowTimeoutSeconds)) { throw "Level-up selection did not open." }
    $LevelUpTransitionSeconds = ([DateTime]::UtcNow - $LevelUpTransitionObservedAt).TotalSeconds
    if ($LevelUpTransitionSeconds -lt 1.5) { throw "Level-up transition was shorter than expected: $LevelUpTransitionSeconds seconds." }
    $DamageCountBeforeChoice = Get-LogPatternCount $ServerLog "Enemy damage applied by GA_Player_Fire"
    $MovementResumeCountBeforeChoice = Get-LogPatternCount $ServerLog "Enemy movement resumed after hit reaction.*CombatSuspended=false.*MovementMode=Walking"
    if (-not (Wait-LogPattern $ServerLog "MDS LevelUp \| ChoiceApplied" $FlowTimeoutSeconds)) { throw "Augment card OnClicked delegate did not apply a choice." }
    if (-not (Wait-LogPatternCount $ServerLog "Enemy damage applied by GA_Player_Fire" ($DamageCountBeforeChoice + 1) $FlowTimeoutSeconds)) { throw "Player fire did not resume after closing the level-up modal." }
    if (-not (Wait-LogPatternCount $ServerLog "Enemy movement resumed after hit reaction.*CombatSuspended=false.*MovementMode=Walking" ($MovementResumeCountBeforeChoice + 1) $FlowTimeoutSeconds)) { throw "Hit-reacting enemy movement did not resume after closing the level-up modal." }
    if (-not (Wait-LogPattern $ClientLog "ShotTracerStarted .* TimeDilation=0.25" $FlowTimeoutSeconds)) { throw "A shot tracer was not observed during slow motion." }
    if (-not (Wait-LogPattern $ClientLog "ShotTracerExpired .* RealDuration=0.1[0-9][0-9] .* TimeDilation=0.25" $FlowTimeoutSeconds)) { throw "Shot tracer lifetime was affected by slow motion." }
    if (-not (Wait-LogPattern $ServerLog "MDS Settlement \| TransitionScheduled .* RealDuration=2.00" $FlowTimeoutSeconds)) { throw "Round-end transition was not scheduled for two real seconds." }
    $SettlementTransitionObservedAt = [DateTime]::UtcNow
    if (-not (Wait-LogPattern $ServerLog "MDS Settlement \| Began \| Round=1" $FlowTimeoutSeconds)) { throw "Round settlement did not open." }
    $SettlementTransitionSeconds = ([DateTime]::UtcNow - $SettlementTransitionObservedAt).TotalSeconds
    if ($SettlementTransitionSeconds -lt 1.5) { throw "Round-end transition was shorter than expected: $SettlementTransitionSeconds seconds." }
    $CombinedLog = (Get-Content $ServerLog -Raw) + (Get-Content $ClientLog -Raw)
    if ($CombinedLog -match "Fatal error|Ensure condition failed") { throw "Fatal or ensure found in runtime logs." }
    if (-not (Select-String -LiteralPath $ClientLog -Pattern "MDS LevelUp \| VerificationWidgetClick" -Quiet)) { throw "Widget OnClicked verification log was not found." }
    Write-Host "Level-up card OnClicked delegate: PASS"
    Write-Host "Level-up two-second transition: PASS ($([Math]::Round($LevelUpTransitionSeconds, 2))s observed after schedule log)"
    Write-Host "Player fire recovery after modal: PASS"
    Write-Host "Repeated damage during hit reaction: PASS"
    Write-Host "Hit-reacting enemy movement recovery: PASS"
    Write-Host "Real-time shot tracer lifetime during slow motion: PASS"
    Write-Host "Round-end two-second transition: PASS ($([Math]::Round($SettlementTransitionSeconds, 2))s observed after schedule log)"
    Write-Host "Round settlement opened: PASS"
    Write-Host "No fatal runtime error: PASS"
    Write-Host "R16 ENEMY RESUME AND REAL-TIME TRACER VERIFY RESULT: PASS"
}
finally {
    Stop-MDSProcess $Client
    Stop-MDSProcess $Server
}
