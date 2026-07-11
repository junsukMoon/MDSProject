param(
    [int]$RunWaitSeconds = 20,
    [int]$WarmupSeconds = 8
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$BuildBat = "C:\UnrealEngine\Engine\Build\BatchFiles\Build.bat"
$RunUatBat = "C:\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat"
$EditorCmd = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor-Cmd.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$ConfigureScript = Join-Path $Root "Tools\ConfigureDebugOverlayController.py"
$TempScriptDir = "C:\Temp\MDS"
$TempConfigureScript = Join-Path $TempScriptDir "ConfigureDebugOverlayController.py"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ConfigLog = Join-Path $LogDir "MDS_DebugOverlayControllerConfig.log"
$ClientLog = Join-Path $LogDir "MDS_DebugOverlayViewport_Client.log"
$ClientErr = Join-Path $LogDir "MDS_DebugOverlayViewport_Client.err"
$ScreenshotPath = Join-Path $LogDir "MDS_DebugOverlayViewport_Client_PrintWindow.png"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Select-DebugOverlayViewportPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDSOverlayControllerConfig|Debug overlay widget class configured|Debug overlay fallback layout initialized|Debug overlay widget created|MDS Debug \| NetMode=Standalone|Default input mapping context is not configured|Called AddMappingContext with a null Mapping Context|Debug overlay widget class is not configured|Debug overlay widget creation failed|Using CommonUI without a CommonGameViewportClient|Fatal|LogWindows: Error" |
        Select-Object -Last 160
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

if (-not (Test-Path -LiteralPath $ConfigureScript)) {
    throw "Controller configure script was not found at $ConfigureScript"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $TempScriptDir | Out-Null
Remove-Item -LiteralPath $ConfigLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientErr -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ScreenshotPath -Force -ErrorAction SilentlyContinue

Write-Host "Building MDSProjectEditor Win64 Development..."
& $BuildBat MDSProjectEditor Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) {
    throw "MDSProjectEditor build failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "Editor commandlet executable was not found at $EditorCmd"
}

Copy-Item -LiteralPath $ConfigureScript -Destination $TempConfigureScript -Force

Write-Host "Configuring BP_TopDownController parent..."
& $EditorCmd $ProjectFile -NullRHI -unattended -nop4 -nosplash "-ExecutePythonScript=$TempConfigureScript" -stdout -FullStdOutLogOutput -forcelogflush *>&1 |
    Out-File -FilePath $ConfigLog -Encoding utf8

Write-Host "Cooking/staging Win64 client..."
& $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -utf8output
if ($LASTEXITCODE -ne 0) {
    throw "BuildCookRun failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path -LiteralPath $ClientExe)) {
    throw "Staged client executable was not found at $ClientExe"
}

$ClientProcess = $null
try {
    $ClientDir = Split-Path -Parent $ClientExe
    Write-Host "Launching visible staged client:"
    Write-Host "  $ClientExe"
    Write-Host "  /Game/TopDown/Lvl_TopDown -windowed -ResX=1280 -ResY=720 -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush"

    $ClientProcess = Start-Process -FilePath $ClientExe `
        -ArgumentList @("/Game/TopDown/Lvl_TopDown", "-windowed", "-ResX=1280", "-ResY=720", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush") `
        -WorkingDirectory $ClientDir `
        -RedirectStandardOutput $ClientLog `
        -RedirectStandardError $ClientErr `
        -PassThru

    Start-Sleep -Seconds $WarmupSeconds

    Add-Type -AssemblyName System.Windows.Forms
    Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class MDSWin32Capture
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

    $Handle = $ClientProcess.MainWindowHandle
    if ($Handle -eq [IntPtr]::Zero) {
        $ClientProcess.Refresh()
        $Handle = $ClientProcess.MainWindowHandle
    }

    if ($Handle -eq [IntPtr]::Zero) {
        throw "Unable to find staged client main window handle."
    }

    [MDSWin32Capture]::SetForegroundWindow($Handle) | Out-Null
    Start-Sleep -Milliseconds 500
    [System.Windows.Forms.SendKeys]::SendWait("{F1}")
    Start-Sleep -Seconds 3

    $Rect = New-Object MDSWin32Capture+RECT
    [MDSWin32Capture]::GetWindowRect($Handle, [ref]$Rect) | Out-Null
    $Width = [Math]::Max(1, $Rect.Right - $Rect.Left)
    $Height = [Math]::Max(1, $Rect.Bottom - $Rect.Top)
    $Bitmap = New-Object System.Drawing.Bitmap $Width, $Height
    $Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $Hdc = $Graphics.GetHdc()
    try {
        $Captured = [MDSWin32Capture]::PrintWindow($Handle, $Hdc, 2)
    }
    finally {
        $Graphics.ReleaseHdc($Hdc)
        $Graphics.Dispose()
    }
    $Bitmap.Save($ScreenshotPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $Bitmap.Dispose()
    Write-Host "PrintWindow captured: $Captured"
    Write-Host "Screenshot: $ScreenshotPath"

    $RemainingWait = [Math]::Max(0, $RunWaitSeconds - $WarmupSeconds - 3)
    if ($RemainingWait -gt 0) {
        Start-Sleep -Seconds $RemainingWait
    }
}
finally {
    Stop-IfRunning -Process $ClientProcess
}

foreach ($Log in @($ConfigLog, $ClientLog)) {
    Write-Host "---- Patterns from $Log ----"
    $Matches = @(Select-DebugOverlayViewportPatterns -Path $Log)
    if ($Matches.Count -eq 0) {
        Write-Host "No debug overlay viewport verification patterns found."
    } else {
        $Matches | ForEach-Object { Write-Host $_.Line }
    }
}

$ConfigText = if (Test-Path -LiteralPath $ConfigLog) { Get-Content -LiteralPath $ConfigLog -Raw -ErrorAction SilentlyContinue } else { "" }
$ClientText = if (Test-Path -LiteralPath $ClientLog) { Get-Content -LiteralPath $ClientLog -Raw -ErrorAction SilentlyContinue } else { "" }
$ScreenshotOk = (Test-Path -LiteralPath $ScreenshotPath) -and ((Get-Item -LiteralPath $ScreenshotPath).Length -gt 0)

$ConfiguredControllerOk = $ConfigText -match "MDSOverlayControllerConfig: .*uses /Script/MDSProject.MDSProjectPlayerController|MDSOverlayControllerConfig: Reparented"
$WidgetClassOk = $ClientText -match "Debug overlay widget class configured as"
$FallbackLayoutOk = $ClientText -match "Debug overlay fallback layout initialized"
$WidgetCreatedOk = $ClientText -match "Debug overlay widget created on"
$StandaloneDebugLineOk = $ClientText -match "MDS Debug \| NetMode=Standalone"
$ProjectMissingInput = $ClientText -match "Default input mapping context is not configured|Click input action is not configured|Touch input action is not configured"
$EnhancedInputNullWarning = $ClientText -match "Called AddMappingContext with a null Mapping Context"
$MissingClass = $ClientText -match "Debug overlay widget class is not configured"
$CreateFailed = $ClientText -match "Debug overlay widget creation failed"
$CommonUiViewportError = $ClientText -match "Using CommonUI without a CommonGameViewportClient"
$FatalError = $ClientText -match "Fatal error|LogWindows: Error:"

if ($ConfiguredControllerOk -and $WidgetClassOk -and $FallbackLayoutOk -and $WidgetCreatedOk -and $StandaloneDebugLineOk -and $ScreenshotOk -and -not $ProjectMissingInput -and -not $MissingClass -and -not $CreateFailed -and -not $CommonUiViewportError -and -not $FatalError) {
    Write-Host "DEBUG OVERLAY VIEWPORT VERIFY RESULT: PASS"
    exit 0
}

Write-Host "DEBUG OVERLAY VIEWPORT VERIFY RESULT: INCOMPLETE"
Write-Host "Controller configured: $ConfiguredControllerOk"
Write-Host "Widget class configured log found: $WidgetClassOk"
Write-Host "Fallback layout log found: $FallbackLayoutOk"
Write-Host "Widget creation log found: $WidgetCreatedOk"
Write-Host "Standalone debug line found: $StandaloneDebugLineOk"
Write-Host "Screenshot captured: $ScreenshotOk"
Write-Host "Project missing input warning found: $ProjectMissingInput"
Write-Host "Enhanced Input null mapping warning found: $EnhancedInputNullWarning"
Write-Host "Missing class log found: $MissingClass"
Write-Host "Creation failure log found: $CreateFailed"
Write-Host "CommonUI viewport error found: $CommonUiViewportError"
Write-Host "Fatal error found: $FatalError"
exit 2
