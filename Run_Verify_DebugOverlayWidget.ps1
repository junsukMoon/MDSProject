param(
    [int]$RunWaitSeconds = 35
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$EditorCmd = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor-Cmd.exe"
$EditorExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor.exe"
$BuildBat = "C:\UnrealEngine\Engine\Build\BatchFiles\Build.bat"
$CreateWidgetScript = Join-Path $Root "Tools\CreateDebugOverlayWidget.py"
$TempScriptDir = "C:\Temp\MDS"
$TempCreateWidgetScript = Join-Path $TempScriptDir "CreateDebugOverlayWidget.py"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$AssetLog = Join-Path $LogDir "MDS_DebugOverlayAsset.log"
$RuntimeLog = Join-Path $LogDir "MDS_DebugOverlayRuntime.log"

function Select-DebugOverlayPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDSDebugOverlayAsset|Debug overlay widget class configured|Debug overlay fallback layout initialized|Debug overlay widget created|Debug overlay widget class is not configured|Debug overlay widget creation failed|Using CommonUI without a CommonGameViewportClient|MDS Debug \| NetMode=Standalone|Fatal" |
        Select-Object -Last 120
}

if (-not (Test-Path -LiteralPath $BuildBat)) {
    throw "Build.bat was not found at $BuildBat"
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found at $ProjectFile"
}

if (-not (Test-Path -LiteralPath $CreateWidgetScript)) {
    throw "Widget creation script was not found at $CreateWidgetScript"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $TempScriptDir | Out-Null
Remove-Item -LiteralPath $AssetLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $RuntimeLog -Force -ErrorAction SilentlyContinue

Write-Host "Building MDSProjectEditor Win64 Development..."
& $BuildBat MDSProjectEditor Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) {
    throw "MDSProjectEditor build failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "Editor commandlet executable was not found at $EditorCmd"
}

if (-not (Test-Path -LiteralPath $EditorExe)) {
    throw "Editor executable was not found at $EditorExe"
}

Copy-Item -LiteralPath $CreateWidgetScript -Destination $TempCreateWidgetScript -Force

Write-Host "Compiling/saving debug overlay Widget Blueprint..."
& $EditorCmd $ProjectFile -NullRHI -unattended -nop4 -nosplash "-ExecutePythonScript=$TempCreateWidgetScript" -stdout -FullStdOutLogOutput -forcelogflush *>&1 |
    Out-File -FilePath $AssetLog -Encoding utf8

Write-Host "Launching headless game runtime for debug overlay creation log..."
$RuntimeJob = $null
try {
    $RuntimeJob = Start-Job -ScriptBlock {
        param($Exe, $Project, $Out)

        & $Exe $Project "/Game/TopDown/Lvl_TopDown" -game -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush *>&1 |
            Out-File -FilePath $Out -Encoding utf8
    } -ArgumentList $EditorExe, $ProjectFile, $RuntimeLog

    Start-Sleep -Seconds $RunWaitSeconds
    Receive-Job $RuntimeJob -Keep | Out-Null
}
finally {
    if ($RuntimeJob) {
        Stop-Job $RuntimeJob -ErrorAction SilentlyContinue
        Remove-Job $RuntimeJob -Force -ErrorAction SilentlyContinue
    }
}

foreach ($Log in @($AssetLog, $RuntimeLog)) {
    Write-Host "---- Patterns from $Log ----"
    $Matches = @(Select-DebugOverlayPatterns -Path $Log)
    if ($Matches.Count -eq 0) {
        Write-Host "No debug overlay verification patterns found."
    } else {
        $Matches | ForEach-Object { Write-Host $_.Line }
    }
}

$AssetText = if (Test-Path -LiteralPath $AssetLog) { Get-Content -LiteralPath $AssetLog -Raw -ErrorAction SilentlyContinue } else { "" }
$RuntimeText = if (Test-Path -LiteralPath $RuntimeLog) { Get-Content -LiteralPath $RuntimeLog -Raw -ErrorAction SilentlyContinue } else { "" }

$AssetOk = $AssetText -match "MDSDebugOverlayAsset: (Loaded existing|Created) widget blueprint" -and
    $AssetText -match "MDSDebugOverlayAsset: Compiled and saved widget blueprint" -and
    $AssetText -match "MDSDebugOverlayAsset: Done"
$ConfiguredOk = $RuntimeText -match "Debug overlay widget class configured as"
$FallbackLayoutOk = $RuntimeText -match "Debug overlay fallback layout initialized"
$CreatedOk = $RuntimeText -match "Debug overlay widget created on"
$StandaloneRuntimeOk = $RuntimeText -match "MDS Debug \| NetMode=Standalone"
$MissingClass = $RuntimeText -match "Debug overlay widget class is not configured"
$CreateFailed = $RuntimeText -match "Debug overlay widget creation failed"
$CommonUiViewportError = $RuntimeText -match "Using CommonUI without a CommonGameViewportClient"
$FatalError = $RuntimeText -match "Fatal error|LogWindows: Error:"

if ($AssetOk -and $ConfiguredOk -and $StandaloneRuntimeOk -and -not $MissingClass -and -not $CreateFailed -and -not $CommonUiViewportError -and -not $FatalError) {
    Write-Host "DEBUG OVERLAY VERIFY RESULT: PASS"
    exit 0
}

Write-Host "DEBUG OVERLAY VERIFY RESULT: INCOMPLETE"
Write-Host "Widget asset compile/save found: $AssetOk"
Write-Host "Widget class configured log found: $ConfiguredOk"
Write-Host "Fallback layout log found: $FallbackLayoutOk"
Write-Host "Runtime creation log found: $CreatedOk"
Write-Host "Standalone runtime debug line found: $StandaloneRuntimeOk"
Write-Host "Missing class log found: $MissingClass"
Write-Host "Creation failure log found: $CreateFailed"
Write-Host "CommonUI viewport error found: $CommonUiViewportError"
Write-Host "Fatal error found: $FatalError"
Write-Host "This script verifies configuration/runtime logs only; headless runtime may not construct viewport widgets, so visual pixels, F1 input, and TextBlock layout still require PIE/manual verification."
exit 2
