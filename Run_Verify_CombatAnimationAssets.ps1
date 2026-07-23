param(
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$BuildBat = "C:\UnrealEngine\Engine\Build\BatchFiles\Build.bat"
$EditorCmd = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor-Cmd.exe"
$VerifyScript = Join-Path $Root "Tools\VerifyCombatAnimationAssets.py"
$TempScriptDir = "C:\Temp\MDS"
$TempVerifyScript = Join-Path $TempScriptDir "VerifyCombatAnimationAssets.py"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$AssetLog = Join-Path $LogDir "MDS_CombatAnimationAssets.log"

function Select-CombatAnimationAssetPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDSCombatAnimationAssets|Fatal error|LogWindows: Error:" |
        Select-Object -Last 220
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found at $ProjectFile"
}

if (-not (Test-Path -LiteralPath $BuildBat)) {
    throw "Build.bat was not found at $BuildBat"
}

if (-not (Test-Path -LiteralPath $VerifyScript)) {
    throw "Combat animation asset verification script was not found at $VerifyScript"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $TempScriptDir | Out-Null
Remove-Item -LiteralPath $AssetLog -Force -ErrorAction SilentlyContinue

if (-not $SkipBuild) {
    Write-Host "Building MDSProjectEditor Win64 Development..."
    & $BuildBat MDSProjectEditor Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) {
        throw "MDSProjectEditor build failed with exit code $LASTEXITCODE"
    }
}

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "Editor commandlet executable was not found at $EditorCmd"
}

Copy-Item -LiteralPath $VerifyScript -Destination $TempVerifyScript -Force

Write-Host "Verifying existing combat animation assets..."
& $EditorCmd $ProjectFile -NullRHI -unattended -nop4 -nosplash "-ExecutePythonScript=$TempVerifyScript" -stdout -FullStdOutLogOutput -forcelogflush *>&1 |
    Out-File -FilePath $AssetLog -Encoding utf8
$EditorExitCode = $LASTEXITCODE

Write-Host "---- Patterns from $AssetLog ----"
$Matches = @(Select-CombatAnimationAssetPatterns -Path $AssetLog)
if ($Matches.Count -eq 0) {
    Write-Host "No combat animation asset verification patterns found."
} else {
    $Matches | ForEach-Object { Write-Host $_.Line }
}

$AssetText = if (Test-Path -LiteralPath $AssetLog) { Get-Content -LiteralPath $AssetLog -Raw -ErrorAction SilentlyContinue } else { "" }
$FatalError = $AssetText -match "Fatal error|LogWindows: Error:"
$PythonError = $AssetText -match "LogPython: Error:"
$CoreReady = $AssetText -match "MDSCombatAnimationAssets: CORE_READINESS \| PASS=True"
$CharacterLineageReady = $AssetText -match "MDSCombatAnimationAssets: CHARACTER_LINEAGE_READINESS \| PASS=True"
$NotifyReady = $AssetText -match "MDSCombatAnimationAssets: ATTACK_NOTIFY_READINESS \| PASS=True"
$NoNotifyIncomplete = $AssetText -match "MDSCombatAnimationAssets: INCOMPLETE \| No authored attack notify"
$CharacterLineageIncomplete = $AssetText -match "MDSCombatAnimationAssets: INCOMPLETE \| BP_TopDownCharacter lineage"

if (($EditorExitCode -eq 0) -and $CoreReady -and -not $FatalError -and -not $PythonError) {
    if ($NotifyReady -and $CharacterLineageReady) {
        Write-Host "COMBAT ANIMATION ASSET VERIFY RESULT: PASS"
    } else {
        Write-Host "COMBAT ANIMATION ASSET VERIFY RESULT: PASS_WITH_INCOMPLETE_ITEMS"
    }
    Write-Host "BP_TopDownCharacter lineage proven: $CharacterLineageReady"
    Write-Host "Attack notify authored/readable: $NotifyReady"
    if ($NoNotifyIncomplete) {
        Write-Host "Attack notify readiness note: INCOMPLETE"
    }
    if ($CharacterLineageIncomplete) {
        Write-Host "Character lineage readiness note: INCOMPLETE"
    }
    Write-Host "This verifies existing asset loadability and skeleton compatibility only; it does not verify runtime montage playback or visible pose changes."
    exit 0
}

Write-Host "COMBAT ANIMATION ASSET VERIFY RESULT: INCOMPLETE"
Write-Host "Editor-Cmd exit code: $EditorExitCode"
Write-Host "Core readiness found: $CoreReady"
Write-Host "BP_TopDownCharacter lineage proven: $CharacterLineageReady"
Write-Host "Attack notify authored/readable: $NotifyReady"
Write-Host "Fatal error found: $FatalError"
Write-Host "Python error found: $PythonError"
Write-Host "This verifies existing asset loadability and skeleton compatibility only; it does not verify runtime montage playback or visible pose changes."
exit 2
