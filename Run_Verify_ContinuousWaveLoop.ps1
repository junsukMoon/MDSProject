param(
    [int]$Port = 7855,
    [int]$ServerWaitSeconds = 15,
    [int]$RuntimeSeconds = 24,
    [switch]$SkipBuild,
    [switch]$SkipStage
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$BuildBat = "C:\UnrealEngine\Engine\Build\BatchFiles\Build.bat"
$RunUatBat = "C:\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat"
$StagedServerExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$StagedClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "MDS_ContinuousWaveLoop_Server.log"
$ClientLog = Join-Path $LogDir "MDS_ContinuousWaveLoop_Client.log"

function Stop-VerifyJob {
    param([System.Management.Automation.Job]$Job)
    if ($Job) {
        Stop-Job $Job -ErrorAction SilentlyContinue
        Remove-Job $Job -Force -ErrorAction SilentlyContinue
    }
}

function Get-MatchCount {
    param([string]$Text, [string]$Pattern)
    return ([regex]::Matches($Text, $Pattern)).Count
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog,$ClientLog -Force -ErrorAction SilentlyContinue

if (-not $SkipBuild) {
    & $BuildBat MDSProject Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) { throw "MDSProject build failed with exit code $LASTEXITCODE" }
    & $BuildBat MDSProjectServer Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) { throw "MDSProjectServer build failed with exit code $LASTEXITCODE" }
}

if (-not $SkipStage) {
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) { throw "Client staging failed with exit code $LASTEXITCODE" }
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -server -serverplatform=Win64 -serverconfig=Development -noclient -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) { throw "Server staging failed with exit code $LASTEXITCODE" }
}

if (-not (Test-Path -LiteralPath $StagedServerExe) -or -not (Test-Path -LiteralPath $StagedClientExe)) {
    throw "Staged client/server executables were not found."
}
if (netstat -ano | Select-String ":$Port\s") {
    throw "Port $Port is already in use."
}

$ServerArgs = @(
    "/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush",
    "MDSWaveMaxCount=3", "MDSWaveInitialEnemyCount=3", "MDSWaveEnemyIncrement=1", "MDSWaveIntermission=1",
    "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=5000", "MDSAttackDamage=100", "MDSAttackCooldown=0.1",
    "-port=$Port"
)
$ClientArgs = @(
    "127.0.0.1:$Port", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush",
    "-MDSAutoAttackNearestEnemy", "MDSAutoAttackCount=80", "MDSAutoAttackDelay=3",
    "MDSAutoAttackRetryInterval=0.2", "MDSAttackRange=5000", "MDSAttackDamage=100", "MDSAttackCooldown=0.1"
)

$ServerJob = $null
$ClientJob = $null
try {
    $ServerJob = Start-Job -ScriptBlock {
        param($Exe, $WorkingDirectory, $OutputPath, [string[]]$LaunchArgs)
        Set-Location -LiteralPath $WorkingDirectory
        & $Exe @LaunchArgs *>&1 | Out-File -FilePath $OutputPath -Encoding utf8
    } -ArgumentList $StagedServerExe,(Split-Path -Parent $StagedServerExe),$ServerLog,$ServerArgs

    $Listening = $false
    for ($Second = 0; $Second -lt $ServerWaitSeconds; ++$Second) {
        Start-Sleep -Seconds 1
        if (netstat -ano | Select-String ":$Port\s") {
            $Listening = $true
            break
        }
    }
    if (-not $Listening) { throw "Dedicated server did not listen on UDP port $Port." }

    $ClientJob = Start-Job -ScriptBlock {
        param($Exe, $WorkingDirectory, $OutputPath, [string[]]$LaunchArgs)
        Set-Location -LiteralPath $WorkingDirectory
        & $Exe @LaunchArgs *>&1 | Out-File -FilePath $OutputPath -Encoding utf8
    } -ArgumentList $StagedClientExe,(Split-Path -Parent $StagedClientExe),$ClientLog,$ClientArgs

    Start-Sleep -Seconds $RuntimeSeconds
}
finally {
    Stop-VerifyJob $ClientJob
    Stop-VerifyJob $ServerJob
}

$ServerText = Get-Content -LiteralPath $ServerLog -Raw
$ClientText = Get-Content -LiteralPath $ClientLog -Raw
$KnownEventLogWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
$FatalError = $ServerText.Replace($KnownEventLogWarning, "") -match "Fatal error|LogWindows: Error:|Ensure condition failed" -or
    $ClientText.Replace($KnownEventLogWarning, "") -match "Fatal error|LogWindows: Error:|Ensure condition failed"

$Checks = [ordered]@{
    "Wave 1 spawned 3" = $ServerText -match "Wave started on server: Wave=1 RequestedEnemies=3 SpawnedEnemies=3 Active=true"
    "Wave 2 spawned 4" = $ServerText -match "Wave started on server: Wave=2 RequestedEnemies=4 SpawnedEnemies=4 Active=true"
    "Wave 3 spawned 5" = $ServerText -match "Wave started on server: Wave=3 RequestedEnemies=5 SpawnedEnemies=5 Active=true"
    "Twelve deaths consumed" = (Get-MatchCount $ServerText "Wave enemy death consumed on server") -eq 12
    "Three waves cleared" = (Get-MatchCount $ServerText "Wave cleared on server") -eq 3
    "Final completion emitted once" = (Get-MatchCount $ServerText "Demo wave loop completed on server: FinalWave=3") -eq 1
    "Client observed Wave 1" = $ClientText -match "Wave state replicated on client: Wave=1 .* Active=true"
    "Client observed Wave 2" = $ClientText -match "Wave state replicated on client: Wave=2 .* Active=true"
    "Client observed Wave 3" = $ClientText -match "Wave state replicated on client: Wave=3 .* Active=true"
    "No fatal runtime error" = -not $FatalError
}

$Checks.GetEnumerator() | ForEach-Object {
    Write-Host ("{0}: {1}" -f $_.Key, $(if ($_.Value) { "PASS" } else { "FAIL" }))
}

Select-String -Path $ServerLog -Pattern "Wave loop configured|Next wave scheduled|Wave started on server|Wave enemy death consumed|Wave cleared on server|Demo wave loop completed" |
    ForEach-Object { Write-Host $_.Line }

if ($Checks.Values -contains $false) {
    throw "CONTINUOUS WAVE LOOP VERIFY RESULT: FAIL"
}

Write-Host "CONTINUOUS WAVE LOOP VERIFY RESULT: PASS"
Write-Host "Server log: $ServerLog"
Write-Host "Client log: $ClientLog"
