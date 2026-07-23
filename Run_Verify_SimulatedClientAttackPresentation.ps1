param(
    [int]$Port = 7910,
    [int]$ServerWaitSeconds = 15,
    [int]$ObserverWarmupSeconds = 5,
    [int]$ClientWaitSeconds = 18,
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
$ServerLog = Join-Path $LogDir "MDS_SimulatedClientAttack_Server.log"
$ObserverLog = Join-Path $LogDir "MDS_SimulatedClientAttack_Observer.log"
$AttackerLog = Join-Path $LogDir "MDS_SimulatedClientAttack_Attacker.log"

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

function Start-LoggedJob {
    param([string]$Exe, [string]$OutputPath, [string[]]$LaunchArgs)
    $ExeDir = Split-Path -Parent $Exe
    return Start-Job -ScriptBlock {
        param($JobExe, $JobDir, $JobOutput, [string[]]$JobArgs)
        Set-Location -LiteralPath $JobDir
        & $JobExe @JobArgs *>&1 | Out-File -FilePath $JobOutput -Encoding utf8
    } -ArgumentList $Exe, $ExeDir, $OutputPath, $LaunchArgs
}

if (-not $SkipBuild) {
    & $BuildBat MDSProject Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) { throw "Client build failed with exit code $LASTEXITCODE" }
    & $BuildBat MDSProjectServer Win64 Development $ProjectFile -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) { throw "Server build failed with exit code $LASTEXITCODE" }
}

if (-not $SkipStage) {
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) { throw "Client stage failed with exit code $LASTEXITCODE" }
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -server -serverplatform=Win64 -serverconfig=Development -noclient -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) { throw "Server stage failed with exit code $LASTEXITCODE" }
}

foreach ($Path in @($StagedServerExe, $StagedClientExe)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Executable was not found at $Path" }
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog, $ObserverLog, $AttackerLog -Force -ErrorAction SilentlyContinue
if (netstat -ano | Select-String ":$Port") { throw "Port $Port is already in use." }

$ServerArgs = @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSCombatPresentationLog", "-MDSAutoStartWave", "MDSWaveEnemyCount=1", "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5", "-port=$Port")
$ObserverArgs = @("127.0.0.1:$Port", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSCombatPresentationLog")
$AttackerArgs = $ObserverArgs + @("-MDSAutoAttackNearestEnemy", "MDSAutoAttackCount=4", "MDSAutoAttackDelay=4", "MDSAutoAttackRetryInterval=0.75", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5")

$ServerJob = $null
$ObserverJob = $null
$AttackerJob = $null
try {
    Write-Host "Launching Dedicated Server on port $Port..."
    $ServerJob = Start-LoggedJob -Exe $StagedServerExe -OutputPath $ServerLog -LaunchArgs $ServerArgs
    Start-Sleep -Seconds $ServerWaitSeconds
    Write-Host "Launching observer client..."
    $ObserverJob = Start-LoggedJob -Exe $StagedClientExe -OutputPath $ObserverLog -LaunchArgs $ObserverArgs
    Start-Sleep -Seconds $ObserverWarmupSeconds
    Write-Host "Launching attacker client..."
    $AttackerJob = Start-LoggedJob -Exe $StagedClientExe -OutputPath $AttackerLog -LaunchArgs $AttackerArgs
    Start-Sleep -Seconds $ClientWaitSeconds

    $ServerText = Get-Content -LiteralPath $ServerLog -Raw -ErrorAction SilentlyContinue
    $ObserverText = Get-Content -LiteralPath $ObserverLog -Raw -ErrorAction SilentlyContinue
    $AttackerText = Get-Content -LiteralPath $AttackerLog -Raw -ErrorAction SilentlyContinue
    $KnownEventLogWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
    $AllFilteredText = ($ServerText + $ObserverText + $AttackerText).Replace($KnownEventLogWarning, "")

    $ConnectionCount = Get-MatchCount -Text $ServerText -Pattern "Login request|Join succeeded"
    $ValidAttackCount = Get-MatchCount -Text $ServerText -Pattern "ServerAttackResolved .* Valid=true .* Hit=true"
    $DamageCount = Get-MatchCount -Text $ServerText -Pattern "Enemy damage applied by PlayerAttack"
    $ServerPlaybackCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatAnimationPlayback"
    $ObserverRemoteReceiptCount = Get-MatchCount -Text $ObserverText -Pattern "RemoteAttackPresentationReceived .* NetMode=Client .* LocalRole=SimulatedProxy .* LocallyControlled=false"
    $ObserverRemotePlaybackCount = Get-MatchCount -Text $ObserverText -Pattern "RemoteAttackAnimationPlaybackAttempted .* PlaybackSucceeded=true"
    $ObserverRemotePresentationCount = Get-MatchCount -Text $ObserverText -Pattern "AttackPresentationRequested .* Source=ServerDirectionalFire"
    $ObserverReplicationCount = Get-MatchCount -Text $ObserverText -Pattern "Enemy HP replicated on client"
    $AttackerIntentCount = Get-MatchCount -Text $AttackerText -Pattern "AutoAttackIntent"
    $AttackerRemotePlaybackCount = Get-MatchCount -Text $AttackerText -Pattern "RemoteAttackAnimationPlaybackAttempted"
    $AttackerReplicationCount = Get-MatchCount -Text $AttackerText -Pattern "Enemy HP replicated on client"
    $FatalError = $AllFilteredText -match "Fatal error|LogWindows: Error:|Ensure condition failed"

    Write-Host "Server connection markers: $ConnectionCount"
    Write-Host "Server valid hit / damage count: $ValidAttackCount / $DamageCount"
    Write-Host "Server animation playback count: $ServerPlaybackCount"
    Write-Host "Observer remote receipt / playback / presentation: $ObserverRemoteReceiptCount / $ObserverRemotePlaybackCount / $ObserverRemotePresentationCount"
    Write-Host "Observer Enemy HP replication count: $ObserverReplicationCount"
    Write-Host "Attacker intent / remote playback / HP replication: $AttackerIntentCount / $AttackerRemotePlaybackCount / $AttackerReplicationCount"
    Write-Host "Fatal or ensure found: $FatalError"

    $Passed = ($ConnectionCount -ge 4) -and
        ($ValidAttackCount -eq 4) -and ($DamageCount -eq 4) -and
        ($ServerPlaybackCount -eq 0) -and
        ($ObserverRemoteReceiptCount -eq 4) -and
        ($ObserverRemotePlaybackCount -eq 4) -and
        ($ObserverRemotePresentationCount -eq 4) -and
        ($ObserverReplicationCount -eq 4) -and
        ($AttackerIntentCount -eq 4) -and
        ($AttackerRemotePlaybackCount -eq 0) -and
        ($AttackerReplicationCount -eq 4) -and
        (-not $FatalError)

    if ($Passed) {
        Write-Host "SIMULATED CLIENT ATTACK PRESENTATION VERIFY RESULT: PASS"
        exit 0
    }

    Write-Host "SIMULATED CLIENT ATTACK PRESENTATION VERIFY RESULT: INCOMPLETE"
    exit 2
}
finally {
    Stop-VerifyJob -Job $AttackerJob
    Stop-VerifyJob -Job $ObserverJob
    Stop-VerifyJob -Job $ServerJob
}
