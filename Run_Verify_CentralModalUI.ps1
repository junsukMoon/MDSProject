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
$ServerLog = Join-Path $LogDir "R14_CentralModalUI_Server.log"
$ClientLog = Join-Path $LogDir "R14_CentralModalUI_Client.log"
$LevelUpShot = Join-Path $LogDir "R14_LevelUpModal.png"
$SettlementShot = Join-Path $LogDir "R14_RoundSettlement.png"

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
Remove-Item -LiteralPath $ServerLog,$ClientLog,$LevelUpShot,$SettlementShot -Force -ErrorAction SilentlyContinue
if (-not (Test-Path $ServerExe) -or -not (Test-Path $ClientExe)) { throw "Server or staged client executable is missing." }

$ServerArgs = "`"$ProjectFile`" /Game/TopDown/Lvl_TopDown -server -NullRHI -unattended -DDC-ForceMemoryCache -forcelogflush -abslog=`"$ServerLog`" MDSWaveMaxCount=2 MDSWaveInitialEnemyCount=1 MDSWaveEnemyIncrement=0 MDSWaveIntermission=2 MDSSettlementDuration=20 MDSActorBaselineMoveSpeed=0 MDSKillCurrency=50 MDSKillExperience=100 -port=$Port"
$ClientArgs = "127.0.0.1:$Port -windowed -ResX=1280 -ResY=720 -WinX=80 -WinY=80 -nosound -NoSplash -DDC-ForceMemoryCache -forcelogflush -abslog=`"$ClientLog`" -MDSAutoAttackNearestEnemy MDSAutoAttackCount=20 MDSAutoAttackDelay=2 MDSAutoAttackRetryInterval=0.25 MDSAttackDamage=100 MDSAttackRange=5000 MDSAttackCooldown=0.1 -MDSVerifyClickLevelUpWidget"
$Server = $null
$Client = $null
try {
    $Server = Start-MDSProcess $ServerExe $ServerArgs (Split-Path -Parent $ServerExe)
    if (-not (Wait-LogPattern $ServerLog "IpNetDriver listening on port" $StartupTimeoutSeconds)) { throw "Server did not start listening." }
    $Client = Start-MDSProcess $ClientExe $ClientArgs (Split-Path -Parent $ClientExe)
    $Handle = Get-ClientWindow $Client $StartupTimeoutSeconds
    if (-not (Wait-LogPattern $ServerLog "FlowState=EMDSLevelUpFlowState::Selection" $FlowTimeoutSeconds)) { throw "Level-up selection did not open." }
    Start-Sleep -Seconds 1
    Capture-Window $Handle $LevelUpShot
    if (-not (Wait-LogPattern $ServerLog "MDS LevelUp \| ChoiceApplied" $FlowTimeoutSeconds)) { throw "Augment card OnClicked delegate did not apply a choice." }
    if (-not (Wait-LogPattern $ServerLog "MDS Settlement \| Began \| Round=1" $FlowTimeoutSeconds)) { throw "Round settlement did not open." }
    Start-Sleep -Seconds 1
    Capture-Window $Handle $SettlementShot
    $CombinedLog = (Get-Content $ServerLog -Raw) + (Get-Content $ClientLog -Raw)
    if ($CombinedLog -match "Fatal error|Ensure condition failed") { throw "Fatal or ensure found in runtime logs." }
    if (-not (Select-String -LiteralPath $ClientLog -Pattern "MDS LevelUp \| VerificationWidgetClick" -Quiet)) { throw "Widget OnClicked verification log was not found." }
    Write-Host "Level-up card OnClicked delegate: PASS"
    Write-Host "Round settlement opened: PASS"
    Write-Host "No fatal runtime error: PASS"
    Write-Host "R14 CENTRAL MODAL UI VERIFY RESULT: PASS"
}
finally {
    Stop-MDSProcess $Client
    Stop-MDSProcess $Server
}
