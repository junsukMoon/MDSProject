param(
    [int]$Port = 7791,
    [int]$ServerWaitSeconds = 15,
    [int]$ClientWaitSeconds = 18,
    [switch]$SkipBuild,
    [switch]$SkipStage,
    [switch]$VerifyAnimNotify
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

function Stop-VerifyJob {
    param([System.Management.Automation.Job]$Job)

    if ($Job) {
        Stop-Job $Job -ErrorAction SilentlyContinue
        Remove-Job $Job -Force -ErrorAction SilentlyContinue
    }
}

function Select-CombatPresentationPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDS CombatPresentation|MDS CombatAnimationPlayback|MDS CombatAnimNotify|PresentationOnly|AutoAttackIntent|ServerAttackResolved|ServerAttackRejected|Enemy damage applied by PlayerAttack|Enemy HP replicated on client|Enemy death handled on server|Wave enemy death consumed on server|Combat enemy wave spawn created|Join succeeded|Login request|Fatal error|LogWindows: Error:" |
        Select-Object -Last 180
}

function Get-MatchCount {
    param(
        [string]$Text,
        [string]$Pattern
    )

    return ([regex]::Matches($Text, $Pattern)).Count
}

function Get-FirstIndex {
    param(
        [string]$Text,
        [string]$Pattern
    )

    $Match = [regex]::Match($Text, $Pattern)
    if ($Match.Success) {
        return $Match.Index
    }

    return -1
}

function Test-Ordered {
    param(
        [string]$Text,
        [string]$FirstPattern,
        [string]$SecondPattern
    )

    $FirstIndex = Get-FirstIndex -Text $Text -Pattern $FirstPattern
    $SecondIndex = Get-FirstIndex -Text $Text -Pattern $SecondPattern
    return ($FirstIndex -ge 0) -and ($SecondIndex -ge 0) -and ($FirstIndex -lt $SecondIndex)
}

function Invoke-CombatPresentationScenario {
    param(
        [string]$Name,
        [int]$ListenPort,
        [string]$ServerExe,
        [string]$ClientExe
    )

    $ServerLog = Join-Path $LogDir "MDS_CombatPresentation_${Name}_Server.log"
    $ClientLog = Join-Path $LogDir "MDS_CombatPresentation_${Name}_Client.log"
    Remove-Item -LiteralPath $ServerLog -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue

    $PortUsers = netstat -ano | Select-String ":$ListenPort"
    if ($PortUsers) {
        throw "Port $ListenPort is already in use:`n$PortUsers"
    }

    $ServerJob = $null
    $ClientJob = $null

    try {
        $ServerDir = Split-Path -Parent $ServerExe
        $ClientDir = Split-Path -Parent $ClientExe

        $ServerArgs = @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-NoMDSWaveLoop", "-MDSCombatPresentationLog", "-port=$ListenPort")
        $ClientArgs = @("127.0.0.1:$ListenPort", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSCombatPresentationLog")

		if ($VerifyAnimNotify) {
			$ClientArgs += "-MDSCombatAnimNotifyVerify"
		}

        if ($Name -eq "Valid") {
            $ServerArgs += @("-MDSAutoStartWave", "MDSWaveEnemyCount=1", "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5")
            $ClientArgs += @("-MDSAutoAttackNearestEnemy", "MDSAutoAttackCount=4", "MDSAutoAttackDelay=4", "MDSAutoAttackRetryInterval=0.75", "MDSAttackRange=5000", "MDSAttackDamage=25", "MDSAttackCooldown=0.5")
        } elseif ($Name -eq "PresentationOnly") {
            $ClientArgs += @("-MDSPresentationOnlyAttackMarker", "MDSPresentationOnlyAttackDelay=4")
        } else {
            throw "Unknown scenario: $Name"
        }

        Write-Host "==== Combat Presentation Scenario: $Name ===="
        Write-Host "Launching server:"
        Write-Host "  $ServerExe"
        Write-Host "  $($ServerArgs -join ' ')"
        $ServerJob = Start-Job -ScriptBlock {
            param($Exe, $Dir, $Out, [string[]]$LaunchArgs)

            Set-Location -LiteralPath $Dir
            & $Exe @LaunchArgs *>&1 | Out-File -FilePath $Out -Encoding utf8
        } -ArgumentList $ServerExe, $ServerDir, $ServerLog, $ServerArgs

        Start-Sleep -Seconds $ServerWaitSeconds
        Receive-Job $ServerJob -Keep | Out-Null

        $ListenLine = netstat -ano | Select-String ":$ListenPort"
        if ($ListenLine) {
            Write-Host "Port $ListenPort listen detected:"
            $ListenLine | ForEach-Object { Write-Host "  $_" }
        } else {
            Write-Host "Port $ListenPort listen was not detected."
        }

        Write-Host "Launching client:"
        Write-Host "  $ClientExe"
        Write-Host "  $($ClientArgs -join ' ')"
        $ClientJob = Start-Job -ScriptBlock {
            param($Exe, $Dir, $Out, [string[]]$LaunchArgs)

            Set-Location -LiteralPath $Dir
            & $Exe @LaunchArgs *>&1 | Out-File -FilePath $Out -Encoding utf8
        } -ArgumentList $ClientExe, $ClientDir, $ClientLog, $ClientArgs

        Start-Sleep -Seconds $ClientWaitSeconds
        Receive-Job $ClientJob -Keep | Out-Null

        foreach ($Log in @($ServerLog, $ClientLog)) {
            Write-Host "---- Patterns from $Log ----"
            $Matches = @(Select-CombatPresentationPatterns -Path $Log)
            if ($Matches.Count -eq 0) {
                Write-Host "No combat presentation verification patterns found."
            } else {
                $Matches | ForEach-Object { Write-Host $_.Line }
            }
        }

        $ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw -ErrorAction SilentlyContinue } else { "" }
        $ClientText = if (Test-Path -LiteralPath $ClientLog) { Get-Content -LiteralPath $ClientLog -Raw -ErrorAction SilentlyContinue } else { "" }

		$KnownEventLogWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
		$FilteredServerText = $ServerText.Replace($KnownEventLogWarning, "")
		$FilteredClientText = $ClientText.Replace($KnownEventLogWarning, "")
		$FatalError = ($FilteredServerText -match "Fatal error|LogWindows: Error:") -or ($FilteredClientText -match "Fatal error|LogWindows: Error:")
        $ConnectionOk = ($ServerText -match "Login request|Join succeeded") -or ($ClientText -match "Join succeeded|Welcomed by server")
        $ServerPresentationCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatPresentation"
        $ServerAnimationPlaybackCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatAnimationPlayback"
        $AttackPresentationCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatPresentation \| AttackPresentationRequested \| Controller="
        $AttackTimingMarkerCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatPresentation \| AttackTimingMarker \| Controller="
        $HitPresentationCount = Get-MatchCount -Text $ClientText -Pattern "EnemyHitPresentationRequested"
        $DeathPresentationCount = Get-MatchCount -Text $ClientText -Pattern "EnemyDeathPresentationRequested"
        $AttackAnimationPlaybackCount = Get-MatchCount -Text $ClientText -Pattern "AttackMontagePlaybackAttempted"
        $AttackAnimationPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "AttackMontagePlaybackAttempted .* PlaybackSucceeded=true"
        $HitAnimationPlaybackCount = Get-MatchCount -Text $ClientText -Pattern "EnemyHitAnimationPlaybackAttempted"
        $HitAnimationPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "EnemyHitAnimationPlaybackAttempted .* PlaybackSucceeded=true"
        $DeathAnimationPlaybackCount = Get-MatchCount -Text $ClientText -Pattern "EnemyDeathAnimationPlaybackAttempted"
        $DeathAnimationPlaybackSuccessCount = Get-MatchCount -Text $ClientText -Pattern "EnemyDeathAnimationPlaybackAttempted .* PlaybackSucceeded=true"
        $ValidAttackCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackResolved .* Valid=true"
        $RejectedAttackCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackRejected"
        $DamageCount = Get-MatchCount -Text $ServerText -Pattern "Enemy damage applied by PlayerAttack"
        $ClientReplicationCount = Get-MatchCount -Text $ClientText -Pattern "Enemy HP replicated on client"
        $PresentationOnlyMarkerCount = Get-MatchCount -Text $ClientText -Pattern "MDS Combat \| PresentationOnlyAttackMarker"
		$ServerAnimNotifyCount = Get-MatchCount -Text $ServerText -Pattern "MDS CombatAnimNotify \| Fired"
		$ClientAnimNotifyCount = Get-MatchCount -Text $ClientText -Pattern "MDS CombatAnimNotify \| Fired .* GameplayDamage=false \| ServerRequestSent=false"
		$ExpectedAnimNotifyCount = if ($VerifyAnimNotify) { if ($Name -eq "Valid") { 4 } else { 1 } } else { 0 }

        $ServerDamageOrdered = Test-Ordered -Text $ServerText -FirstPattern "Enemy damage applied by PlayerAttack" -SecondPattern "MDS Combat \| ServerAttackResolved .* Valid=true"
        $FirstHitOrdered = Test-Ordered -Text $ClientText -FirstPattern "Enemy HP replicated on client: 75\.0 / 100\.0\. Dead=false" -SecondPattern "EnemyHitPresentationRequested .* EnemyHP=100\.0->75\.0"
        $FirstHitAnimationOrdered = Test-Ordered -Text $ClientText -FirstPattern "Enemy HP replicated on client: 75\.0 / 100\.0\. Dead=false" -SecondPattern "EnemyHitAnimationPlaybackAttempted .* EnemyHP=100\.0->75\.0"
        $DeathOrdered = Test-Ordered -Text $ClientText -FirstPattern "Enemy HP replicated on client: 0\.0 / 100\.0\. Dead=true" -SecondPattern "EnemyDeathPresentationRequested"
        $DeathAnimationOrdered = Test-Ordered -Text $ClientText -FirstPattern "Enemy HP replicated on client: 0\.0 / 100\.0\. Dead=true" -SecondPattern "EnemyDeathAnimationPlaybackAttempted .* EnemyHP=25\.0->0\.0"

        $ScenarioOk = $false
        if ($Name -eq "Valid") {
            $ScenarioOk = $ConnectionOk -and
                ($ServerPresentationCount -eq 0) -and
                ($ServerAnimationPlaybackCount -eq 0) -and
                ($AttackPresentationCount -eq 4) -and
                ($AttackTimingMarkerCount -eq 4) -and
                ($HitPresentationCount -eq 3) -and
                ($DeathPresentationCount -eq 1) -and
                ($AttackAnimationPlaybackCount -eq 4) -and
                ($AttackAnimationPlaybackSuccessCount -eq $AttackAnimationPlaybackCount) -and
                ($HitAnimationPlaybackCount -eq 3) -and
                ($HitAnimationPlaybackSuccessCount -eq $HitAnimationPlaybackCount) -and
                ($DeathAnimationPlaybackCount -eq 1) -and
                ($DeathAnimationPlaybackSuccessCount -eq $DeathAnimationPlaybackCount) -and
                ($ValidAttackCount -eq 4) -and
                ($RejectedAttackCount -eq 0) -and
                ($DamageCount -eq 4) -and
                ($ClientReplicationCount -eq 4) -and
				($ServerAnimNotifyCount -eq 0) -and
				($ClientAnimNotifyCount -eq $ExpectedAnimNotifyCount) -and
                $ServerDamageOrdered -and
                $FirstHitOrdered -and
                $FirstHitAnimationOrdered -and
                $DeathOrdered -and
                $DeathAnimationOrdered -and
                -not $FatalError
        } elseif ($Name -eq "PresentationOnly") {
            $ScenarioOk = $ConnectionOk -and
                ($ServerPresentationCount -eq 0) -and
                ($ServerAnimationPlaybackCount -eq 0) -and
                ($PresentationOnlyMarkerCount -eq 1) -and
                ($AttackPresentationCount -eq 1) -and
                ($AttackTimingMarkerCount -eq 1) -and
                ($AttackAnimationPlaybackCount -eq 1) -and
                ($AttackAnimationPlaybackSuccessCount -eq $AttackAnimationPlaybackCount) -and
                ($HitAnimationPlaybackCount -eq 0) -and
                ($DeathAnimationPlaybackCount -eq 0) -and
                ($ValidAttackCount -eq 0) -and
                ($RejectedAttackCount -eq 0) -and
                ($DamageCount -eq 0) -and
                ($ClientReplicationCount -eq 0) -and
				($ServerAnimNotifyCount -eq 0) -and
				($ClientAnimNotifyCount -eq $ExpectedAnimNotifyCount) -and
                -not $FatalError
        }

        if ($ScenarioOk) {
            Write-Host "COMBAT PRESENTATION $Name VERIFY RESULT: PASS"
        } else {
            Write-Host "COMBAT PRESENTATION $Name VERIFY RESULT: INCOMPLETE"
        }

        Write-Host "Connection found: $ConnectionOk"
        Write-Host "Server presentation hook count: $ServerPresentationCount"
        Write-Host "Server animation playback count: $ServerAnimationPlaybackCount"
        Write-Host "Attack presentation count: $AttackPresentationCount"
        Write-Host "Attack timing marker count: $AttackTimingMarkerCount"
        Write-Host "Hit presentation count: $HitPresentationCount"
        Write-Host "Death presentation count: $DeathPresentationCount"
        Write-Host "Attack animation playback count: $AttackAnimationPlaybackCount"
        Write-Host "Attack animation playback success count: $AttackAnimationPlaybackSuccessCount"
        Write-Host "Hit animation playback count: $HitAnimationPlaybackCount"
        Write-Host "Hit animation playback success count: $HitAnimationPlaybackSuccessCount"
        Write-Host "Death animation playback count: $DeathAnimationPlaybackCount"
        Write-Host "Death animation playback success count: $DeathAnimationPlaybackSuccessCount"
        Write-Host "Presentation-only marker count: $PresentationOnlyMarkerCount"
        Write-Host "Valid attack count: $ValidAttackCount"
        Write-Host "Rejected attack count: $RejectedAttackCount"
        Write-Host "PlayerAttack damage count: $DamageCount"
        Write-Host "Client Enemy HP replication count: $ClientReplicationCount"
		Write-Host "Server AnimNotify fired count: $ServerAnimNotifyCount"
		Write-Host "Client AnimNotify fired count: $ClientAnimNotifyCount"
		Write-Host "Expected client AnimNotify fired count: $ExpectedAnimNotifyCount"
        Write-Host "Server damage before resolved log: $ServerDamageOrdered"
        Write-Host "First hit presentation after HP replication: $FirstHitOrdered"
        Write-Host "First hit animation after HP replication: $FirstHitAnimationOrdered"
        Write-Host "Death presentation after HP replication: $DeathOrdered"
        Write-Host "Death animation after HP replication: $DeathAnimationOrdered"
        Write-Host "Fatal error found: $FatalError"

        return $ScenarioOk
    }
    finally {
        Stop-VerifyJob -Job $ClientJob
        Stop-VerifyJob -Job $ServerJob
    }
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

$ServerExe = if (Test-Path -LiteralPath $StagedServerExe) { $StagedServerExe } else { $BuiltServerExe }
$ClientExe = if (Test-Path -LiteralPath $StagedClientExe) { $StagedClientExe } else { $BuiltClientExe }

if (-not (Test-Path -LiteralPath $ServerExe)) {
    throw "Server executable was not found at $ServerExe"
}

if (-not (Test-Path -LiteralPath $ClientExe)) {
    throw "Client executable was not found at $ClientExe"
}

$ValidPassed = Invoke-CombatPresentationScenario -Name "Valid" -ListenPort $Port -ServerExe $ServerExe -ClientExe $ClientExe
$PresentationOnlyPassed = Invoke-CombatPresentationScenario -Name "PresentationOnly" -ListenPort ($Port + 1) -ServerExe $ServerExe -ClientExe $ClientExe

if ($ValidPassed -and $PresentationOnlyPassed) {
    Write-Host "COMBAT PRESENTATION VERIFY RESULT: PASS"
    Write-Host "COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: PASS"
	if ($VerifyAnimNotify) {
		Write-Host "COMBAT ANIMNOTIFY VERIFY RESULT: PASS"
	}
    exit 0
}

Write-Host "COMBAT PRESENTATION VERIFY RESULT: INCOMPLETE"
Write-Host "COMBAT ANIMATION PLAYBACK ATTEMPT VERIFY RESULT: INCOMPLETE"
if ($VerifyAnimNotify) {
	Write-Host "COMBAT ANIMNOTIFY VERIFY RESULT: INCOMPLETE"
}
exit 2
