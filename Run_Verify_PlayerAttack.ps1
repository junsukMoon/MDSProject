param(
    [ValidateSet("All", "Valid", "OutOfRange", "Cooldown", "InvalidDirection", "InvalidDamage", "NoPawn", "Rejects")]
    [string]$Scenario = "All",
    [int]$Port = 7782,
    [int]$ServerWaitSeconds = 15,
    [int]$ClientWaitSeconds = 18,
    [int]$WaveEnemyCount = 1,
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

function Stop-VerifyJob {
    param([System.Management.Automation.Job]$Job)

    if ($Job) {
        Stop-Job $Job -ErrorAction SilentlyContinue
        Remove-Job $Job -Force -ErrorAction SilentlyContinue
    }
}

function Select-PlayerAttackPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "MDS Combat|AutoAttack|ClientAttackIntent|ServerAttackResolved|ServerAttackRejected|PlayerAttack|Enemy damage applied|Enemy HP replicated on client|Enemy death handled on server|Wave enemy death consumed on server|Wave cleared on server|Combat enemy wave spawn created|Join succeeded|Login request|Fatal error|LogWindows: Error:" |
        Select-Object -Last 160
}

function Get-MatchCount {
    param(
        [string]$Text,
        [string]$Pattern
    )

    return ([regex]::Matches($Text, $Pattern)).Count
}

function Get-ScenarioConfig {
    param([string]$Name)

    switch ($Name) {
        "Valid" {
            return [pscustomobject]@{
                Name = "Valid"
                AttackRange = 5000.0
                AttackDamage = 25.0
                AttackCooldown = 0.5
                AutoAttackCount = 4
                AutoAttackDelay = 4.0
                AutoAttackRetryInterval = 0.75
                ClientWaitSeconds = $ClientWaitSeconds
                RejectScenario = ""
            }
        }
        "OutOfRange" {
            return [pscustomobject]@{
                Name = "OutOfRange"
                AttackRange = 100.0
                AttackDamage = 25.0
                AttackCooldown = 0.5
                AutoAttackCount = 1
                AutoAttackDelay = 4.0
                AutoAttackRetryInterval = 0.75
                ClientWaitSeconds = [Math]::Max(10, $ClientWaitSeconds)
                RejectScenario = ""
            }
        }
        "Cooldown" {
            return [pscustomobject]@{
                Name = "Cooldown"
                AttackRange = 5000.0
                AttackDamage = 10.0
                AttackCooldown = 10.0
                AutoAttackCount = 2
                AutoAttackDelay = 4.0
                AutoAttackRetryInterval = 0.25
                ClientWaitSeconds = [Math]::Max(12, $ClientWaitSeconds)
                RejectScenario = ""
            }
        }
        "InvalidDirection" {
            return [pscustomobject]@{ Name = "InvalidDirection"; AttackRange = 5000.0; AttackDamage = 25.0; AttackCooldown = 0.5; AutoAttackCount = 1; AutoAttackDelay = 4.0; AutoAttackRetryInterval = 0.75; ClientWaitSeconds = [Math]::Max(10, $ClientWaitSeconds); RejectScenario = "InvalidDirection" }
        }
        "InvalidDamage" {
            return [pscustomobject]@{ Name = "InvalidDamage"; AttackRange = 5000.0; AttackDamage = 0.0; AttackCooldown = 0.5; AutoAttackCount = 1; AutoAttackDelay = 4.0; AutoAttackRetryInterval = 0.75; ClientWaitSeconds = [Math]::Max(10, $ClientWaitSeconds); RejectScenario = "InvalidDamage" }
        }
        "NoPawn" {
            return [pscustomobject]@{ Name = "NoPawn"; AttackRange = 5000.0; AttackDamage = 25.0; AttackCooldown = 0.5; AutoAttackCount = 1; AutoAttackDelay = 4.0; AutoAttackRetryInterval = 0.75; ClientWaitSeconds = [Math]::Max(10, $ClientWaitSeconds); RejectScenario = "NoPawn" }
        }
    }

    throw "Unknown scenario: $Name"
}

function Invoke-PlayerAttackScenario {
    param(
        [object]$Config,
        [int]$ListenPort,
        [string]$ServerExe,
        [string]$ClientExe
    )

    $ServerLog = Join-Path $LogDir "MDS_PlayerAttack_$($Config.Name)_Server.log"
    $ClientLog = Join-Path $LogDir "MDS_PlayerAttack_$($Config.Name)_Client.log"
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

        Write-Host "==== Player Attack Scenario: $($Config.Name) ===="
        Write-Host "Launching server:"
        Write-Host "  $ServerExe"
        Write-Host "  /Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -MDSAutoStartWave MDSWaveEnemyCount=$WaveEnemyCount MDSActorBaselineMoveSpeed=0 MDSAttackRange=$($Config.AttackRange) MDSAttackDamage=$($Config.AttackDamage) MDSAttackCooldown=$($Config.AttackCooldown) -port=$ListenPort"
        $ServerJob = Start-Job -ScriptBlock {
            param($Exe, $Dir, $Out, $ListenPort, $EnemyCount, $Range, $Damage, $Cooldown, $RejectScenario)

            Set-Location -LiteralPath $Dir
            $ServerArguments = @("/Game/TopDown/Lvl_TopDown", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-MDSAutoStartWave", "MDSWaveEnemyCount=$EnemyCount", "MDSActorBaselineMoveSpeed=0", "MDSAttackRange=$Range", "MDSAttackDamage=$Damage", "MDSAttackCooldown=$Cooldown", "-port=$ListenPort")
            if ($RejectScenario) {
                $ServerArguments += "MDSAutoAttackReject=$RejectScenario"
            }
            & $Exe @ServerArguments *>&1 |
                Out-File -FilePath $Out -Encoding utf8
        } -ArgumentList $ServerExe, $ServerDir, $ServerLog, $ListenPort, $WaveEnemyCount, $Config.AttackRange, $Config.AttackDamage, $Config.AttackCooldown, $Config.RejectScenario

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
        Write-Host "  127.0.0.1:$ListenPort -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush -MDSAutoAttackNearestEnemy MDSAutoAttackCount=$($Config.AutoAttackCount) MDSAutoAttackDelay=$($Config.AutoAttackDelay) MDSAutoAttackRetryInterval=$($Config.AutoAttackRetryInterval) MDSAttackRange=$($Config.AttackRange) MDSAttackDamage=$($Config.AttackDamage) MDSAttackCooldown=$($Config.AttackCooldown)"
        $ClientJob = Start-Job -ScriptBlock {
            param($Exe, $Dir, $Out, $ConnectAddress, $Count, $Delay, $Interval, $Range, $Damage, $Cooldown, $RejectScenario)

            Set-Location -LiteralPath $Dir
            if ($RejectScenario) {
                & $Exe $ConnectAddress -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush "MDSAutoAttackReject=$RejectScenario" "MDSAutoAttackRejectDelay=$Delay" "MDSAttackRange=$Range" "MDSAttackDamage=$Damage" "MDSAttackCooldown=$Cooldown" *>&1 |
                    Out-File -FilePath $Out -Encoding utf8
                return
            }
            & $Exe $ConnectAddress -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush -MDSAutoAttackNearestEnemy "MDSAutoAttackCount=$Count" "MDSAutoAttackDelay=$Delay" "MDSAutoAttackRetryInterval=$Interval" "MDSAttackRange=$Range" "MDSAttackDamage=$Damage" "MDSAttackCooldown=$Cooldown" *>&1 |
                Out-File -FilePath $Out -Encoding utf8
        } -ArgumentList $ClientExe, $ClientDir, $ClientLog, "127.0.0.1:$ListenPort", $Config.AutoAttackCount, $Config.AutoAttackDelay, $Config.AutoAttackRetryInterval, $Config.AttackRange, $Config.AttackDamage, $Config.AttackCooldown, $Config.RejectScenario

        Start-Sleep -Seconds $Config.ClientWaitSeconds
        Receive-Job $ClientJob -Keep | Out-Null

        foreach ($Log in @($ServerLog, $ClientLog)) {
            Write-Host "---- Patterns from $Log ----"
            $Matches = @(Select-PlayerAttackPatterns -Path $Log)
            if ($Matches.Count -eq 0) {
                Write-Host "No player attack verification patterns found."
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
        $ClientAutoIntentOk = $ClientText -match "MDS Combat \| AutoAttackIntent"
        $ValidCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackResolved .* Valid=true"
		$MissCount = Get-MatchCount -Text $ServerText -Pattern "MDS Combat \| ServerAttackResolved .* Valid=true .* Hit=false"
        $DamageCount = Get-MatchCount -Text $ServerText -Pattern "Enemy damage applied by PlayerAttack"
        $ClientReplicationCount = Get-MatchCount -Text $ClientText -Pattern "Enemy HP replicated on client"
        $OutOfRangeRejectCount = Get-MatchCount -Text $ServerText -Pattern "ServerAttackRejected .* Reason=OutOfRange"
        $CooldownRejectCount = Get-MatchCount -Text $ServerText -Pattern "ServerAttackRejected .* Reason=Cooldown"
        $ExpectedRejectCount = if ($Config.RejectScenario) { Get-MatchCount -Text $ServerText -Pattern "ServerAttackRejected .* Reason=$($Config.RejectScenario)" } else { 0 }
        $DeathOk = $ServerText -match "Enemy death handled on server from PlayerAttack"
        $WaveConsumedOk = $ServerText -match "Wave enemy death consumed on server"

        $ScenarioOk = $false
        switch ($Config.Name) {
            "Valid" {
                $ScenarioOk = $ClientAutoIntentOk -and ($ValidCount -ge 1) -and ($DamageCount -ge 1) -and ($ClientReplicationCount -ge 1) -and $ConnectionOk -and -not $FatalError
            }
            "OutOfRange" {
				# Directional fire remains valid without a target; a short range must resolve as a miss with zero damage.
                $ScenarioOk = $ClientAutoIntentOk -and ($MissCount -ge 1) -and ($DamageCount -eq 0) -and ($ValidCount -ge 1) -and $ConnectionOk -and -not $FatalError
            }
            "Cooldown" {
                $ScenarioOk = $ClientAutoIntentOk -and ($ValidCount -eq 1) -and ($DamageCount -eq 1) -and ($CooldownRejectCount -ge 1) -and ($ClientReplicationCount -ge 1) -and $ConnectionOk -and -not $FatalError
            }
            { $_ -in @("InvalidDirection", "InvalidDamage", "NoPawn") } {
                $RejectIntentOk = $ClientText -match "AttackRejectVerificationIntent .* Scenario=$($Config.RejectScenario)"
                $ScenarioOk = $RejectIntentOk -and ($ExpectedRejectCount -eq 1) -and ($ValidCount -eq 0) -and ($DamageCount -eq 0) -and (-not $DeathOk) -and (-not $WaveConsumedOk) -and $ConnectionOk -and -not $FatalError
            }
        }

        if ($ScenarioOk) {
            Write-Host "PLAYER ATTACK $($Config.Name) VERIFY RESULT: PASS"
        } else {
            Write-Host "PLAYER ATTACK $($Config.Name) VERIFY RESULT: INCOMPLETE"
        }

        Write-Host "Client auto attack intent found: $ClientAutoIntentOk"
        Write-Host "Valid attack count: $ValidCount"
		Write-Host "Valid miss count: $MissCount"
        Write-Host "PlayerAttack damage count: $DamageCount"
        Write-Host "Client Enemy HP replication count: $ClientReplicationCount"
        Write-Host "OutOfRange reject count: $OutOfRangeRejectCount"
        Write-Host "Cooldown reject count: $CooldownRejectCount"
        Write-Host "Expected reject count: $ExpectedRejectCount"
        Write-Host "Connection found: $ConnectionOk"
        Write-Host "Death observed: $DeathOk"
        Write-Host "Wave enemy death consumed: $WaveConsumedOk"
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

$ScenarioNames = if ($Scenario -eq "All") { @("Valid", "OutOfRange", "Cooldown", "InvalidDirection", "InvalidDamage", "NoPawn") } elseif ($Scenario -eq "Rejects") { @("InvalidDirection", "InvalidDamage", "NoPawn") } else { @($Scenario) }
$AllPassed = $true
$ScenarioIndex = 0
foreach ($ScenarioName in $ScenarioNames) {
    $Config = Get-ScenarioConfig -Name $ScenarioName
    $ScenarioPort = $Port + $ScenarioIndex
    $ScenarioPassed = Invoke-PlayerAttackScenario -Config $Config -ListenPort $ScenarioPort -ServerExe $ServerExe -ClientExe $ClientExe
    $AllPassed = $AllPassed -and $ScenarioPassed
    $ScenarioIndex++
}

if ($AllPassed) {
    Write-Host "PLAYER ATTACK VERIFY RESULT: PASS"
    exit 0
}

Write-Host "PLAYER ATTACK VERIFY RESULT: INCOMPLETE"
exit 2
