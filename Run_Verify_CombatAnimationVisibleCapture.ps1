param(
    [int]$Port = 7820,
    [int]$ServerWaitSeconds = 12,
    [int]$ClientWaitSeconds = 22,
    [switch]$SkipBuild,
    [switch]$SkipStage
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
$ServerLog = Join-Path $LogDir "MDS_CombatAnimationVisible_Server.log"
$ClientLog = Join-Path $LogDir "MDS_CombatAnimationVisible_Client.log"
$AttackBeforePath = Join-Path $LogDir "MDS_CombatAnimationVisible_AttackBefore.png"
$AttackPosePath = Join-Path $LogDir "MDS_CombatAnimationVisible_AttackPose.png"
$HitBeforePath = Join-Path $LogDir "MDS_CombatAnimationVisible_HitBefore.png"
$HitPosePath = Join-Path $LogDir "MDS_CombatAnimationVisible_HitPose.png"
$DeathBeforePath = Join-Path $LogDir "MDS_CombatAnimationVisible_DeathBefore.png"
$DeathPosePath = Join-Path $LogDir "MDS_CombatAnimationVisible_DeathPose.png"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Get-MatchCount {
    param(
        [string]$Text,
        [string]$Pattern
    )

    return ([regex]::Matches($Text, $Pattern)).Count
}

function Select-CombatAnimationVisiblePatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDS CombatAnimationPlayback|MDS CombatAnimationVisibleCapture|MDS CombatPresentation|AutoAttackIntent|ServerAttackResolved|ServerAttackRejected|Enemy damage applied by PlayerAttack|Enemy HP replicated on client|Enemy death handled on server|Join succeeded|Login request|Fatal error|LogWindows: Error:" |
        Select-Object -Last 220
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

function Measure-GameplayPixelDelta {
    param([string]$BeforePath, [string]$PosePath)

    Add-Type -AssemblyName System.Drawing
    $Before = [System.Drawing.Bitmap]::FromFile($BeforePath)
    $Pose = [System.Drawing.Bitmap]::FromFile($PosePath)
    try {
        if (($Before.Width -ne $Pose.Width) -or ($Before.Height -ne $Pose.Height)) {
            return [pscustomobject]@{ Compatible = $false; Changed = 0; Samples = 0; Ratio = 0.0 }
        }

        $Changed = 0
        $Samples = 0
        $StartX = [int]($Before.Width * 0.1)
        $EndX = [int]($Before.Width * 0.9)
        $StartY = [int]($Before.Height * 0.1)
        $EndY = [int]($Before.Height * 0.85)
        for ($Y = $StartY; $Y -lt $EndY; $Y += 3) {
            for ($X = $StartX; $X -lt $EndX; $X += 3) {
                $A = $Before.GetPixel($X, $Y)
                $B = $Pose.GetPixel($X, $Y)
                $Difference = [Math]::Max([Math]::Abs([int]$A.R - [int]$B.R), [Math]::Max([Math]::Abs([int]$A.G - [int]$B.G), [Math]::Abs([int]$A.B - [int]$B.B)))
                if ($Difference -ge 20) { $Changed++ }
                $Samples++
            }
        }

        return [pscustomobject]@{ Compatible = $true; Changed = $Changed; Samples = $Samples; Ratio = if ($Samples -gt 0) { $Changed / $Samples } else { 0.0 } }
    }
    finally {
        $Before.Dispose()
        $Pose.Dispose()
    }
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found at $ProjectFile"
}

if (-not $SkipBuild -and -not (Test-Path -LiteralPath $BuildBat)) {
    throw "Build.bat was not found at $BuildBat"
}

if (-not $SkipStage -and -not (Test-Path -LiteralPath $RunUatBat)) {
    throw "RunUAT.bat was not found at $RunUatBat"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $AttackBeforePath, $AttackPosePath, $HitBeforePath, $HitPosePath, $DeathBeforePath, $DeathPosePath -Force -ErrorAction SilentlyContinue

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

    Write-Host "Cooking/staging Win64 server..."
    & $RunUatBat BuildCookRun "-project=$ProjectFile" -noP4 -server -serverplatform=Win64 -serverconfig=Development -noclient -cook -stage -pak -utf8output
    if ($LASTEXITCODE -ne 0) {
        throw "Server BuildCookRun failed with exit code $LASTEXITCODE"
    }
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

    Write-Host "Launching dedicated server:"
    Write-Host "  $ServerExe"
    $ServerProcess = Start-Process -FilePath $ServerExe `
        -ArgumentList @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSCombatPresentationLog", "-MDSAutoStartWave", "MDSWaveEnemyCount=1", "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5", "-port=$Port") `
        -WorkingDirectory $ServerDir `
        -RedirectStandardOutput $ServerLog `
        -PassThru `
        -WindowStyle Hidden

    Start-Sleep -Seconds $ServerWaitSeconds

    Write-Host "Launching visible client:"
    Write-Host "  $ClientExe"
    $ClientProcess = Start-Process -FilePath $ClientExe `
        -ArgumentList @("127.0.0.1:$Port", "-windowed", "-ResX=1280", "-ResY=720", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSCombatPresentationLog", "-MDSCombatAnimationVisibleShot", "-MDSCombatAnimationPoseDelta", "-MDSCombatAnimationVisibleShotDir=`"$LogDir`"", "-MDSAutoAttackNearestEnemy", "MDSAutoAttackCount=4", "MDSAutoAttackDelay=6", "MDSAutoAttackRetryInterval=1.0", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5") `
        -WorkingDirectory $ClientDir `
        -RedirectStandardOutput $ClientLog `
        -PassThru

    Start-Sleep -Seconds $ClientWaitSeconds
}
finally {
    Stop-IfRunning -Process $ClientProcess
    Stop-IfRunning -Process $ServerProcess
}

foreach ($Log in @($ServerLog, $ClientLog)) {
    Write-Host "---- Patterns from $Log ----"
    $Matches = @(Select-CombatAnimationVisiblePatterns -Path $Log)
    if ($Matches.Count -eq 0) {
        Write-Host "No combat animation visible verification patterns found."
    } else {
        $Matches | ForEach-Object { Write-Host $_.Line }
    }
}

$ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw -ErrorAction SilentlyContinue } else { "" }
$ClientText = if (Test-Path -LiteralPath $ClientLog) { Get-Content -LiteralPath $ClientLog -Raw -ErrorAction SilentlyContinue } else { "" }

$ServerPlaybackCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatAnimationPlayback"
$ServerCaptureCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatAnimationVisibleCapture"
$AttackPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatAnimationPlayback \| AttackMontagePlaybackAttempted .*PlaybackSucceeded=true"
$HitPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatAnimationPlayback \| EnemyHitAnimationPlaybackAttempted .*PlaybackSucceeded=true"
$DeathPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatAnimationPlayback \| EnemyDeathAnimationPlaybackAttempted .*PlaybackSucceeded=true"
$ExpectedCaptureTypes = @("AttackBefore", "AttackPose", "HitBefore", "HitPose", "DeathBefore", "DeathPose")
$AllCapturesRequested = ($ExpectedCaptureTypes | Where-Object { $ClientText -notmatch "MDS CombatAnimationVisibleCapture \| ScreenshotRequested \| Type=$_" }).Count -eq 0
$ClientConnectionOk = $ServerText -match "Join succeeded|Login request"
$ValidAttackCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackResolved .*Valid=true"
$RejectedAttackCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackRejected"
$DamageCount = Get-MatchCount -Text $ServerText -Pattern "Enemy damage applied by PlayerAttack"
$HpReplicationCount = Get-MatchCount -Text $ClientText -Pattern "Enemy HP replicated on client"
$FatalError = ($ServerText -match "Fatal error|LogWindows: Error:") -or ($ClientText -match "Fatal error|LogWindows: Error:")

$CapturePaths = @($AttackBeforePath, $AttackPosePath, $HitBeforePath, $HitPosePath, $DeathBeforePath, $DeathPosePath)
$AllScreenshotsOk = ($CapturePaths | Where-Object { (-not (Test-Path -LiteralPath $_)) -or ((Get-Item -LiteralPath $_).Length -le 0) }).Count -eq 0
$AllScreenshotsVisible = $AllScreenshotsOk -and (($CapturePaths | Where-Object { -not (Test-ScreenshotHasVisiblePixels -Path $_) }).Count -eq 0)
$AttackDelta = if ($AllScreenshotsOk) { Measure-GameplayPixelDelta $AttackBeforePath $AttackPosePath } else { $null }
$HitDelta = if ($AllScreenshotsOk) { Measure-GameplayPixelDelta $HitBeforePath $HitPosePath } else { $null }
$DeathDelta = if ($AllScreenshotsOk) { Measure-GameplayPixelDelta $DeathBeforePath $DeathPosePath } else { $null }
$PoseDeltaOk = $AttackDelta -and $HitDelta -and $DeathDelta -and $AttackDelta.Compatible -and $HitDelta.Compatible -and $DeathDelta.Compatible -and ($AttackDelta.Changed -ge 40) -and ($HitDelta.Changed -ge 40) -and ($DeathDelta.Changed -ge 40)

$FirstHpReplicationIndex = $ClientText.IndexOf("Enemy HP replicated on client")
$FirstHitPlaybackIndex = $ClientText.IndexOf("MDS CombatAnimationPlayback | EnemyHitAnimationPlaybackAttempted")
$DeathHpReplicationIndex = $ClientText.IndexOf("Enemy HP replicated on client: 0.0 / 100.0")
$DeathPlaybackIndex = $ClientText.IndexOf("MDS CombatAnimationPlayback | EnemyDeathAnimationPlaybackAttempted")
$HitAfterHpReplication = $FirstHpReplicationIndex -ge 0 -and $FirstHitPlaybackIndex -gt $FirstHpReplicationIndex
$DeathAfterHpZero = $DeathHpReplicationIndex -ge 0 -and $DeathPlaybackIndex -gt $DeathHpReplicationIndex

$Passed = ($ServerPlaybackCount -eq 0) -and
    ($ServerCaptureCount -eq 0) -and
    ($AttackPlaybackSuccessCount -eq 4) -and
    ($HitPlaybackSuccessCount -eq 3) -and
    ($DeathPlaybackSuccessCount -eq 1) -and
    $AllCapturesRequested -and
    $AllScreenshotsOk -and
    $AllScreenshotsVisible -and
    $PoseDeltaOk -and
    $ClientConnectionOk -and
    ($ValidAttackCount -eq 4) -and
    ($RejectedAttackCount -eq 0) -and
    ($DamageCount -eq 4) -and
    ($HpReplicationCount -eq 4) -and
    $HitAfterHpReplication -and
    $DeathAfterHpZero -and
    -not $FatalError

if ($Passed) {
    Write-Host "Attack/Hit/Death changed samples: $($AttackDelta.Changed) / $($HitDelta.Changed) / $($DeathDelta.Changed)"
    Write-Host "Attack/Hit/Death delta ratios: $($AttackDelta.Ratio) / $($HitDelta.Ratio) / $($DeathDelta.Ratio)"
    Write-Host "COMBAT ANIMATION POSE DELTA VERIFY RESULT: PASS"
    exit 0
}

Write-Host "COMBAT ANIMATION VISIBLE CAPTURE VERIFY RESULT: INCOMPLETE"
Write-Host "Server animation playback count: $ServerPlaybackCount"
Write-Host "Server visible capture count: $ServerCaptureCount"
Write-Host "Attack playback success count: $AttackPlaybackSuccessCount"
Write-Host "Hit playback success count: $HitPlaybackSuccessCount"
Write-Host "Death playback success count: $DeathPlaybackSuccessCount"
Write-Host "All paired captures requested: $AllCapturesRequested"
Write-Host "All paired screenshots exist/nonempty/visible: $AllScreenshotsOk / $AllScreenshotsVisible"
Write-Host "Attack/Hit/Death changed samples: $($AttackDelta.Changed) / $($HitDelta.Changed) / $($DeathDelta.Changed)"
Write-Host "Attack/Hit/Death delta ratios: $($AttackDelta.Ratio) / $($HitDelta.Ratio) / $($DeathDelta.Ratio)"
Write-Host "Pose delta threshold passed: $PoseDeltaOk"
Write-Host "Client connection observed: $ClientConnectionOk"
Write-Host "Valid attack count: $ValidAttackCount"
Write-Host "Rejected attack count: $RejectedAttackCount"
Write-Host "Damage count: $DamageCount"
Write-Host "Client HP replication count: $HpReplicationCount"
Write-Host "Hit playback after HP replication: $HitAfterHpReplication"
Write-Host "Death playback after HP zero replication: $DeathAfterHpZero"
Write-Host "Fatal error found: $FatalError"
exit 2
