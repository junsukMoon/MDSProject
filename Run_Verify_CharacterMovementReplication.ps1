param(
    [int]$Port = 7845,
    [int]$ServerWaitSeconds = 15,
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
$BuiltServerExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectServer.exe"
$BuiltClientExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProject.exe"
$StagedServerExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$StagedClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "MDS_CharacterMovement_Server.log"
$MoverClientLog = Join-Path $LogDir "MDS_CharacterMovement_MoverClient.log"
$ObserverClientLog = Join-Path $LogDir "MDS_CharacterMovement_ObserverClient.log"

function Stop-VerifyJob {
    param([System.Management.Automation.Job]$Job)

    if ($Job) {
        Stop-Job $Job -ErrorAction SilentlyContinue
        Remove-Job $Job -Force -ErrorAction SilentlyContinue
    }
}

function Get-MaxSnapshotMetric {
    param(
        [string]$Text,
        [string]$RequiredPattern,
        [string]$MetricName
    )

    $Maximum = 0.0
    $LinePattern = "MDS CharacterMovement \| Snapshot \|[^\r\n]*$RequiredPattern[^\r\n]*$MetricName=([0-9.]+)"
    foreach ($Match in [regex]::Matches($Text, $LinePattern)) {
        $Value = 0.0
        if ([double]::TryParse($Match.Groups[1].Value, [ref]$Value)) {
            $Maximum = [math]::Max($Maximum, $Value)
        }
    }

    return $Maximum
}

function Select-MovementPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDS CharacterMovement|Login request|Join succeeded|Fatal error|LogWindows: Error:|Ensure condition failed" |
        Select-Object -Last 120
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found at $ProjectFile"
}
if (-not (Test-Path -LiteralPath $BuildBat)) {
    throw "Build.bat was not found at $BuildBat"
}
if (-not (Test-Path -LiteralPath $RunUatBat)) {
    throw "RunUAT.bat was not found at $RunUatBat"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog,$MoverClientLog,$ObserverClientLog -Force -ErrorAction SilentlyContinue

if (-not $SkipBuild) {
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
}

if (-not $SkipStage) {
    Write-Host "Cooking/staging Win64 client..."
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -platform=Win64 -clientconfig=Development -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) {
        throw "Client BuildCookRun failed with exit code $LASTEXITCODE"
    }

    Write-Host "Cooking/staging WindowsServer..."
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -server -serverplatform=Win64 -serverconfig=Development -noclient -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) {
        throw "Server BuildCookRun failed with exit code $LASTEXITCODE"
    }
}

$ServerExe = if ($SkipStage) { $StagedServerExe } else { $StagedServerExe }
$ClientExe = if ($SkipStage) { $StagedClientExe } else { $StagedClientExe }
if (-not (Test-Path -LiteralPath $ServerExe)) {
    $ServerExe = $BuiltServerExe
}
if (-not (Test-Path -LiteralPath $ClientExe)) {
    $ClientExe = $BuiltClientExe
}
if (-not (Test-Path -LiteralPath $ServerExe) -or -not (Test-Path -LiteralPath $ClientExe)) {
    throw "Required client/server executables were not found."
}

$PortUsers = netstat -ano | Select-String ":$Port"
if ($PortUsers) {
    throw "Port $Port is already in use:`n$PortUsers"
}

$ServerJob = $null
$ObserverClientJob = $null
$MoverClientJob = $null
try {
    $ServerArgs = @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-NoMDSWaveLoop", "-MDSMovementVerificationLog", "-port=$Port")
    $ObserverArgs = @("127.0.0.1:$Port", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSMovementVerificationLog")
    $MoverArgs = @("127.0.0.1:$Port", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSMovementVerificationLog", "-MDSAutoMoveVerification", "MDSAutoMoveDelay=6", "MDSAutoMoveDuration=3", "MDSAutoMoveDirectionX=1", "MDSAutoMoveDirectionY=0")

    Write-Host "Launching dedicated server on port $Port..."
    $ServerJob = Start-Job -ScriptBlock {
        param($Exe, $WorkingDirectory, $OutputPath, [string[]]$LaunchArgs)
        Set-Location $WorkingDirectory
        & $Exe @LaunchArgs *>&1 | Out-File -FilePath $OutputPath -Encoding utf8
    } -ArgumentList $ServerExe,(Split-Path -Parent $ServerExe),$ServerLog,$ServerArgs

    $ListenDetected = $false
    for ($Second = 0; $Second -lt $ServerWaitSeconds; ++$Second) {
        Start-Sleep -Seconds 1
        if (netstat -ano | Select-String ":$Port\s") {
            $ListenDetected = $true
            break
        }
    }
    if (-not $ListenDetected) {
        throw "Dedicated server did not listen on UDP port $Port."
    }

    Write-Host "Launching observer client..."
    $ObserverClientJob = Start-Job -ScriptBlock {
        param($Exe, $WorkingDirectory, $OutputPath, [string[]]$LaunchArgs)
        Set-Location $WorkingDirectory
        & $Exe @LaunchArgs *>&1 | Out-File -FilePath $OutputPath -Encoding utf8
    } -ArgumentList $ClientExe,(Split-Path -Parent $ClientExe),$ObserverClientLog,$ObserverArgs

    Start-Sleep -Seconds 2

    Write-Host "Launching mover client..."
    $MoverClientJob = Start-Job -ScriptBlock {
        param($Exe, $WorkingDirectory, $OutputPath, [string[]]$LaunchArgs)
        Set-Location $WorkingDirectory
        & $Exe @LaunchArgs *>&1 | Out-File -FilePath $OutputPath -Encoding utf8
    } -ArgumentList $ClientExe,(Split-Path -Parent $ClientExe),$MoverClientLog,$MoverArgs

    Start-Sleep -Seconds $ClientWaitSeconds
}
finally {
    Stop-VerifyJob $MoverClientJob
    Stop-VerifyJob $ObserverClientJob
    Stop-VerifyJob $ServerJob
}

foreach ($LogPath in @($ServerLog,$MoverClientLog,$ObserverClientLog)) {
    Write-Host "---- Patterns from $LogPath ----"
    @(Select-MovementPatterns -Path $LogPath) | ForEach-Object { Write-Host $_.Line }
}

$ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw } else { "" }
$MoverText = if (Test-Path -LiteralPath $MoverClientLog) { Get-Content -LiteralPath $MoverClientLog -Raw } else { "" }
$ObserverText = if (Test-Path -LiteralPath $ObserverClientLog) { Get-Content -LiteralPath $ObserverClientLog -Raw } else { "" }

$KnownEventLogWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
$CombinedFilteredText = ($ServerText + $MoverText + $ObserverText).Replace($KnownEventLogWarning, "")
$FatalError = $CombinedFilteredText -match "Fatal error|LogWindows: Error:|Ensure condition failed"
$ConnectionCount = ([regex]::Matches($ServerText, "Login request|Join succeeded")).Count
$AutoMoveStarted = $MoverText -match "MDS CharacterMovement \| AutoMoveStarted"
$AutoMoveFinished = $MoverText -match "MDS CharacterMovement \| AutoMoveFinished"
$AutoMoveFailed = $MoverText -match "MDS CharacterMovement \| AutoMoveFailed"
$PawnUsesEngineCharacterParent = $MoverText -match "AutoMoveStarted[^\r\n]*NativeParent=Character"
$NavigationUnavailable = $MoverText -match "AutoMoveStarted[^\r\n]*NavSystem=None[^\r\n]*NavData=None"
$PathFollowingUnavailable = $MoverText -match "SimpleMoveRequested[^\r\n]*PathFollowing=None[^\r\n]*PathStatus=-1"
$InputInjectionAccepted = $MoverText -match "InputInjected[^\r\n]*After=V\([^\r\n]*[XY]=[-0-9.]"
$InputConsumedByMovement = $MoverText -match "Snapshot[^\r\n]*LocalRole=AutonomousProxy[^\r\n]*LastInput=V\([^\r\n]*[XY]=[-0-9.]"

$MoverRolePattern = "NetMode=Client[^\r\n]*LocalRole=AutonomousProxy[^\r\n]*RemoteRole=Authority[^\r\n]*LocallyControlled=true"
$ObserverRolePattern = "NetMode=Client[^\r\n]*LocalRole=SimulatedProxy[^\r\n]*RemoteRole=Authority[^\r\n]*LocallyControlled=false"
$ServerRolePattern = "NetMode=DedicatedServer[^\r\n]*LocalRole=Authority"

$MoverDistance = Get-MaxSnapshotMetric -Text $MoverText -RequiredPattern $MoverRolePattern -MetricName "DistanceFromStart"
$MoverSpeed = Get-MaxSnapshotMetric -Text $MoverText -RequiredPattern $MoverRolePattern -MetricName "Speed2D"
$ObserverDistance = Get-MaxSnapshotMetric -Text $ObserverText -RequiredPattern $ObserverRolePattern -MetricName "DistanceFromStart"
$ObserverSpeed = Get-MaxSnapshotMetric -Text $ObserverText -RequiredPattern $ObserverRolePattern -MetricName "Speed2D"
$ServerDistance = Get-MaxSnapshotMetric -Text $ServerText -RequiredPattern $ServerRolePattern -MetricName "DistanceFromStart"
$ServerSpeed = Get-MaxSnapshotMetric -Text $ServerText -RequiredPattern $ServerRolePattern -MetricName "Speed2D"

$Passed = ($ConnectionCount -ge 4) -and
    $AutoMoveStarted -and $AutoMoveFinished -and -not $AutoMoveFailed -and
    ($MoverDistance -ge 250.0) -and ($MoverSpeed -ge 100.0) -and
    ($ServerDistance -ge 250.0) -and ($ServerSpeed -ge 100.0) -and
    ($ObserverDistance -ge 250.0) -and ($ObserverSpeed -ge 100.0) -and
    -not $FatalError

Write-Host "Server connection markers: $ConnectionCount"
Write-Host "Mover AutonomousProxy max distance/speed: $MoverDistance / $MoverSpeed"
Write-Host "Server Authority max distance/speed: $ServerDistance / $ServerSpeed"
Write-Host "Observer SimulatedProxy max distance/speed: $ObserverDistance / $ObserverSpeed"
Write-Host "Auto move started/finished/failed: $AutoMoveStarted / $AutoMoveFinished / $AutoMoveFailed"
Write-Host "Fatal error found: $FatalError"
Write-Host "Pawn native parent is engine Character: $PawnUsesEngineCharacterParent"
Write-Host "Client navigation/path following unavailable: $NavigationUnavailable / $PathFollowingUnavailable"
Write-Host "Movement input accepted/consumed: $InputInjectionAccepted / $InputConsumedByMovement"

if ($PawnUsesEngineCharacterParent -and $NavigationUnavailable -and $PathFollowingUnavailable -and $InputInjectionAccepted -and -not $InputConsumedByMovement) {
    Write-Host "CHARACTER MOVEMENT DIAGNOSTIC: BP native parent is ACharacter; client navigation is unavailable; injected input is accepted but not consumed by movement."
}

if ($Passed) {
    Write-Host "CHARACTER MOVEMENT REPLICATION VERIFY RESULT: PASS"
    exit 0
}

Write-Host "CHARACTER MOVEMENT REPLICATION VERIFY RESULT: INCOMPLETE"
exit 2
