param(
    [int]$Port = 7850,
    [int]$ServerWaitSeconds = 15,
    [int]$ManualInputSeconds = 45
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ServerExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "MDS_CharacterMovementVisible_Server.log"
$MoverClientLog = Join-Path $LogDir "MDS_CharacterMovementVisible_MoverClient.log"
$ObserverClientLog = Join-Path $LogDir "MDS_CharacterMovementVisible_ObserverClient.log"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
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

if (-not (Test-Path -LiteralPath $ServerExe)) {
    throw "Staged server executable was not found at $ServerExe"
}
if (-not (Test-Path -LiteralPath $ClientExe)) {
    throw "Staged client executable was not found at $ClientExe"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog,$MoverClientLog,$ObserverClientLog -Force -ErrorAction SilentlyContinue

$PortUsers = netstat -ano | Select-String ":$Port\s"
if ($PortUsers) {
    throw "Port $Port is already in use:`n$PortUsers"
}

$ServerProcess = $null
$ObserverClientProcess = $null
$MoverClientProcess = $null

try {
    $ServerDir = Split-Path -Parent $ServerExe
    $ClientDir = Split-Path -Parent $ClientExe

    Write-Host "Launching dedicated server on port $Port..."
    $ServerProcess = Start-Process -FilePath $ServerExe `
        -ArgumentList @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSMovementVerificationLog", "-MDSCombatPresentationLog", "-MDSAutoStartWave", "MDSWaveEnemyCount=1", "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=5000", "-port=$Port") `
        -WorkingDirectory $ServerDir `
        -RedirectStandardOutput $ServerLog `
        -PassThru `
        -WindowStyle Hidden

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

    Write-Host "Launching OBSERVER client on the left..."
    $ObserverClientProcess = Start-Process -FilePath $ClientExe `
        -ArgumentList @("127.0.0.1:$Port", "-windowed", "-ResX=900", "-ResY=700", "-WinX=0", "-WinY=20", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSMovementVerificationLog", "-MDSCombatPresentationLog") `
        -WorkingDirectory $ClientDir `
        -RedirectStandardOutput $ObserverClientLog `
        -PassThru

    Start-Sleep -Seconds 3

    Write-Host "Launching MOVER client on the right..."
    $MoverClientProcess = Start-Process -FilePath $ClientExe `
        -ArgumentList @("127.0.0.1:$Port", "-windowed", "-ResX=900", "-ResY=700", "-WinX=920", "-WinY=20", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSMovementVerificationLog", "-MDSCombatPresentationLog") `
        -WorkingDirectory $ClientDir `
        -RedirectStandardOutput $MoverClientLog `
        -PassThru

    Write-Host ""
    Write-Host "MANUAL ACTION REQUIRED"
    Write-Host "1. Click the RIGHT (mover) client window."
    Write-Host "2. Test W=up, S=down, A=left, D=right, then hold W+A for diagonal movement."
    Write-Host "3. While holding W+A, aim the cursor to the right and click Left Mouse to fire."
    Write-Host "4. Click empty space once and the enemy direction once; movement must continue in both cases."
    Write-Host "5. Observe locomotion, aim rotation, and fire presentation in both windows."
    Write-Host "6. The session will close automatically after $ManualInputSeconds seconds."
    Write-Host ""

    Start-Sleep -Seconds $ManualInputSeconds
}
finally {
    Stop-IfRunning -Process $MoverClientProcess
    Stop-IfRunning -Process $ObserverClientProcess
    Stop-IfRunning -Process $ServerProcess
}

$ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw } else { "" }
$MoverText = if (Test-Path -LiteralPath $MoverClientLog) { Get-Content -LiteralPath $MoverClientLog -Raw } else { "" }
$ObserverText = if (Test-Path -LiteralPath $ObserverClientLog) { Get-Content -LiteralPath $ObserverClientLog -Raw } else { "" }

$KnownEventLogWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
$CombinedFilteredText = ($ServerText + $MoverText + $ObserverText).Replace($KnownEventLogWarning, "")
$FatalError = $CombinedFilteredText -match "Fatal error|LogWindows: Error:|Ensure condition failed"
$ConnectionCount = ([regex]::Matches($ServerText, "Login request|Join succeeded")).Count
$DirectionalFireCount = ([regex]::Matches($ServerText, "ServerAttackResolved .* Valid=true")).Count
$DirectionalHitCount = ([regex]::Matches($ServerText, "ServerAttackResolved .* Valid=true .* Hit=true")).Count
$DirectionalMissCount = ([regex]::Matches($ServerText, "ServerAttackResolved .* Valid=true .* Hit=false")).Count

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
    ($MoverDistance -ge 250.0) -and ($MoverSpeed -ge 100.0) -and
    ($ServerDistance -ge 250.0) -and ($ServerSpeed -ge 100.0) -and
    ($ObserverDistance -ge 250.0) -and ($ObserverSpeed -ge 100.0) -and
    -not $FatalError

Write-Host "Server connection markers: $ConnectionCount"
Write-Host "Mover AutonomousProxy max distance/speed: $MoverDistance / $MoverSpeed"
Write-Host "Server Authority max distance/speed: $ServerDistance / $ServerSpeed"
Write-Host "Observer SimulatedProxy max distance/speed: $ObserverDistance / $ObserverSpeed"
Write-Host "Fatal error found: $FatalError"
Write-Host "Directional fire/hit/miss count: $DirectionalFireCount / $DirectionalHitCount / $DirectionalMissCount"
Write-Host "Locomotion pose confirmation: MANUAL REVIEW REQUIRED"

if ($Passed) {
    Write-Host "CHARACTER MOVEMENT VISIBLE VERIFY RESULT: PASS_WITH_MANUAL_POSE_REVIEW"
    exit 0
}

Write-Host "CHARACTER MOVEMENT VISIBLE VERIFY RESULT: INCOMPLETE"
exit 2
