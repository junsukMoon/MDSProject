param(
    [int]$Port = 7779,
    [int]$ServerWaitSeconds = 15,
    [int]$ClientWarmupSeconds = 10,
    [int]$ClientWaitSeconds = 20,
    [int]$ActorEnemyCount = 4
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$BuildBat = "C:\UnrealEngine\Engine\Build\BatchFiles\Build.bat"
$RunUatBat = "C:\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat"
$ServerExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "MDS_ReplicatedUIViewport_Server.log"
$ClientLog = Join-Path $LogDir "MDS_ReplicatedUIViewport_Client.log"
$ScreenshotPath = Join-Path $LogDir "MDS_ReplicatedUIViewport_Client_PrintWindow.png"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Select-ReplicatedUIPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDS Match HUD|MDS Objective World UI|MDS Enemy World UI|Objective World UI widget initialized|Enemy World UI widget initialized|Combat enemy wave spawn created|Objective HP replicated on client|MDS Debug \| NetMode=Client|Using CommonUI without a CommonGameViewportClient|Fatal|LogWindows: Error" |
        Select-Object -Last 180
}

function Capture-Window {
    param(
        [System.Diagnostics.Process]$Process,
        [string]$Path
    )

    Add-Type -AssemblyName System.Windows.Forms
    Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class MDSReplicatedUIWin32Capture
{
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, int nFlags);

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }
}
"@

    $Handle = $Process.MainWindowHandle
    if ($Handle -eq [IntPtr]::Zero) {
        $Process.Refresh()
        $Handle = $Process.MainWindowHandle
    }

    if ($Handle -eq [IntPtr]::Zero) {
        throw "Unable to find staged client main window handle."
    }

    [MDSReplicatedUIWin32Capture]::SetForegroundWindow($Handle) | Out-Null
    Start-Sleep -Milliseconds 500

    $Rect = New-Object MDSReplicatedUIWin32Capture+RECT
    [MDSReplicatedUIWin32Capture]::GetWindowRect($Handle, [ref]$Rect) | Out-Null
    $Width = [Math]::Max(1, $Rect.Right - $Rect.Left)
    $Height = [Math]::Max(1, $Rect.Bottom - $Rect.Top)
    $Bitmap = New-Object System.Drawing.Bitmap $Width, $Height
    $Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $Hdc = $Graphics.GetHdc()
    try {
        $Captured = [MDSReplicatedUIWin32Capture]::PrintWindow($Handle, $Hdc, 2)
    }
    finally {
        $Graphics.ReleaseHdc($Hdc)
        $Graphics.Dispose()
    }
    $Bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    $Bitmap.Dispose()
    Write-Host "PrintWindow captured: $Captured"
    Write-Host "Screenshot: $Path"

    if (-not (Test-ScreenshotHasVisiblePixels -Path $Path)) {
        Write-Host "PrintWindow screenshot appears blank; retrying with CopyFromScreen."
        $ScreenBitmap = New-Object System.Drawing.Bitmap $Width, $Height
        $ScreenGraphics = [System.Drawing.Graphics]::FromImage($ScreenBitmap)
        try {
            $ScreenGraphics.CopyFromScreen($Rect.Left, $Rect.Top, 0, 0, $ScreenBitmap.Size)
        }
        finally {
            $ScreenGraphics.Dispose()
        }
        $ScreenBitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
        $ScreenBitmap.Dispose()
        Write-Host "CopyFromScreen screenshot: $Path"
    }
}

function Test-ScreenshotHasVisiblePixels {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $false
    }

    Add-Type -AssemblyName System.Drawing
    $Bitmap = [System.Drawing.Bitmap]::FromFile($Path)
    try {
        $StepX = [Math]::Max(1, [int]($Bitmap.Width / 64))
        $StepY = [Math]::Max(1, [int]($Bitmap.Height / 36))
        $VisibleSamples = 0

        $StartX = [Math]::Min($Bitmap.Width - 1, 8)
        $StartY = [Math]::Min($Bitmap.Height - 1, 48)
        $EndX = [Math]::Max($StartX + 1, $Bitmap.Width - 8)
        $EndY = [Math]::Max($StartY + 1, $Bitmap.Height - 8)

        for ($Y = $StartY; $Y -lt $EndY; $Y += $StepY) {
            for ($X = $StartX; $X -lt $EndX; $X += $StepX) {
                $Pixel = $Bitmap.GetPixel($X, $Y)
                if (($Pixel.R + $Pixel.G + $Pixel.B) -gt 30) {
                    $VisibleSamples++
                    if ($VisibleSamples -ge 8) {
                        return $true
                    }
                }
            }
        }

        return $false
    }
    finally {
        $Bitmap.Dispose()
    }
}

if (-not (Test-Path -LiteralPath $BuildBat)) {
    throw "Build.bat was not found at $BuildBat"
}

if (-not (Test-Path -LiteralPath $RunUatBat)) {
    throw "RunUAT.bat was not found at $RunUatBat"
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found at $ProjectFile"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ScreenshotPath -Force -ErrorAction SilentlyContinue

Write-Host "Building MDSProjectEditor Win64 Development..."
& $BuildBat MDSProjectEditor Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) {
    throw "MDSProjectEditor build failed with exit code $LASTEXITCODE"
}

Write-Host "Building MDSProject Win64 Development..."
& $BuildBat MDSProject Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) {
    throw "MDSProject build failed with exit code $LASTEXITCODE"
}

Write-Host "Building MDSProjectServer Win64 Development..."
& $BuildBat MDSProjectServer Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) {
    throw "MDSProjectServer build failed with exit code $LASTEXITCODE"
}

Write-Host "Cooking/staging Win64 client..."
& $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -utf8output
if ($LASTEXITCODE -ne 0) {
    throw "Client BuildCookRun failed with exit code $LASTEXITCODE"
}

Write-Host "Cooking/staging Win64 server..."
& $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -server -serverplatform=Win64 -serverconfig=Development -noclient -cook -stage -pak -utf8output
if ($LASTEXITCODE -ne 0) {
    throw "Server BuildCookRun failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path -LiteralPath $ServerExe)) {
    throw "Staged server executable was not found at $ServerExe"
}

if (-not (Test-Path -LiteralPath $ClientExe)) {
    throw "Staged client executable was not found at $ClientExe"
}

$PortUsers = netstat -ano | Select-String ":$Port"
if ($PortUsers) {
    throw "Port $Port is already in use:`n$PortUsers"
}

$ServerProcess = $null
$ClientProcess = $null

try {
    $ServerDir = Split-Path -Parent $ServerExe
    $ClientDir = Split-Path -Parent $ClientExe

    Write-Host "Launching server with actor baseline UI source:"
    Write-Host "  $ServerExe"
    $ServerProcess = Start-Process -FilePath $ServerExe `
        -ArgumentList @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSActorBaseline", "MDSActorBaselineCount=$ActorEnemyCount", "-port=$Port") `
        -WorkingDirectory $ServerDir `
        -RedirectStandardOutput $ServerLog `
        -PassThru `
        -WindowStyle Hidden

    Start-Sleep -Seconds $ServerWaitSeconds

    Write-Host "Launching visible client:"
    Write-Host "  $ClientExe"
    $ClientProcess = Start-Process -FilePath $ClientExe `
        -ArgumentList @("127.0.0.1:$Port", "-windowed", "-ResX=1280", "-ResY=720", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush") `
        -WorkingDirectory $ClientDir `
        -RedirectStandardOutput $ClientLog `
        -PassThru

    Start-Sleep -Seconds $ClientWarmupSeconds
    Capture-Window -Process $ClientProcess -Path $ScreenshotPath

    $RemainingWait = [Math]::Max(0, $ClientWaitSeconds - $ClientWarmupSeconds)
    if ($RemainingWait -gt 0) {
        Start-Sleep -Seconds $RemainingWait
    }
}
finally {
    Stop-IfRunning -Process $ClientProcess
    Stop-IfRunning -Process $ServerProcess
}

foreach ($Log in @($ServerLog, $ClientLog)) {
    Write-Host "---- Patterns from $Log ----"
    $Matches = @(Select-ReplicatedUIPatterns -Path $Log)
    if ($Matches.Count -eq 0) {
        Write-Host "No replicated UI verification patterns found."
    } else {
        $Matches | ForEach-Object { Write-Host $_.Line }
    }
}

$ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw -ErrorAction SilentlyContinue } else { "" }
$ClientText = if (Test-Path -LiteralPath $ClientLog) { Get-Content -LiteralPath $ClientLog -Raw -ErrorAction SilentlyContinue } else { "" }
$ScreenshotOk = (Test-Path -LiteralPath $ScreenshotPath) -and ((Get-Item -LiteralPath $ScreenshotPath).Length -gt 0)
$ScreenshotVisibleOk = Test-ScreenshotHasVisiblePixels -Path $ScreenshotPath

$ActorSpawnOk = $ServerText -match "Combat enemy wave spawn created $ActorEnemyCount/$ActorEnemyCount enemies"
$MatchHudOk = $ClientText -match "MDS Match HUD read GameState wave state"
$ObjectiveWorldOk = $ClientText -match "MDS Objective World UI read ObjectiveActor health"
$EnemyWorldOk = $ClientText -match "MDS Enemy World UI read CombatEnemy health"
$ObjectiveReplicationOk = $ClientText -match "Objective HP replicated on client|MDS Debug \| NetMode=Client .*ObjectiveHP="
$ClientConnectionOk = $ServerText -match "Join succeeded|Login request"
$CommonUiViewportError = $ClientText -match "Using CommonUI without a CommonGameViewportClient"
$FatalError = ($ServerText -match "Fatal error|LogWindows: Error:") -or ($ClientText -match "Fatal error|LogWindows: Error:")

if ($ActorSpawnOk -and $MatchHudOk -and $ObjectiveWorldOk -and $EnemyWorldOk -and $ObjectiveReplicationOk -and $ClientConnectionOk -and $ScreenshotOk -and $ScreenshotVisibleOk -and -not $CommonUiViewportError -and -not $FatalError) {
    Write-Host "REPLICATED UI VIEWPORT VERIFY RESULT: PASS"
    exit 0
}

Write-Host "REPLICATED UI VIEWPORT VERIFY RESULT: INCOMPLETE"
Write-Host "Actor baseline spawn found: $ActorSpawnOk"
Write-Host "Match HUD GameState read found: $MatchHudOk"
Write-Host "Objective World UI ObjectiveActor read found: $ObjectiveWorldOk"
Write-Host "Enemy World UI CombatEnemy read found: $EnemyWorldOk"
Write-Host "Objective replication observed: $ObjectiveReplicationOk"
Write-Host "Client connection observed: $ClientConnectionOk"
Write-Host "Screenshot captured: $ScreenshotOk"
Write-Host "Screenshot has visible pixels: $ScreenshotVisibleOk"
Write-Host "CommonUI viewport error found: $CommonUiViewportError"
Write-Host "Fatal error found: $FatalError"
exit 2
