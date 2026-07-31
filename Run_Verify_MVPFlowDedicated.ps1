param(
    [int]$Port = 7991,
    [int]$ServerWaitSeconds = 12,
    [int]$RuntimeSeconds = 24
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ProjectFile = Join-Path $ProjectRoot "MDSProject.uproject"
$ServerExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectEditor.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$LogDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $LogDir "R12_MVPFlow_Server.log"
$Client1Log = Join-Path $LogDir "R12_MVPFlow_Client1.log"
$Client2Log = Join-Path $LogDir "R12_MVPFlow_Client2.log"

function Stop-VerifyProcess {
    param([System.Management.Automation.Job]$Process)
    if ($Process) {
        Stop-Job $Process -ErrorAction SilentlyContinue
        Remove-Job $Process -Force -ErrorAction SilentlyContinue
    }
}

function Start-LoggedProcess {
    param([string]$Exe, [string[]]$Arguments, [string]$Log)
    Start-Job -ScriptBlock {
        param($ProcessExe, [string[]]$ProcessArguments, $ProcessLog)
        Set-Location -LiteralPath (Split-Path -Parent $ProcessExe)
        & $ProcessExe @ProcessArguments *>&1 | Out-File -FilePath $ProcessLog -Encoding utf8
    } -ArgumentList $Exe,$Arguments,$Log
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -LiteralPath $ServerLog,$Client1Log,$Client2Log -Force -ErrorAction SilentlyContinue
if (-not (Test-Path $ServerExe) -or -not (Test-Path $ClientExe)) { throw "Current editor server or staged client executable is missing." }
if (netstat -ano | Select-String ":$Port\s") { throw "Port $Port is already in use." }

$ServerArgs = @(
    $ProjectFile, "/Game/TopDown/Lvl_TopDown", "-server", "-NullRHI", "-unattended", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-DDC-ForceMemoryCache",
    "MDSWaveMaxCount=2", "MDSWaveInitialEnemyCount=2", "MDSWaveEnemyIncrement=0", "MDSWaveIntermission=2",
    "MDSSettlementDuration=5", "MDSActorBaselineMoveSpeed=0", "MDSKillCurrency=50", "MDSKillExperience=100", "-port=$Port"
)
$ClientArgs = @(
    "127.0.0.1:$Port", "-NullRHI", "-unattended", "-nosound", "-NoSplash", "-stdout", "-FullStdOutLogOutput", "-forcelogflush", "-DDC-ForceMemoryCache",
    "-MDSAutoAttackNearestEnemy", "MDSAutoAttackCount=30", "MDSAutoAttackDelay=2", "MDSAutoAttackRetryInterval=0.25",
    "MDSAttackDamage=100", "MDSAttackRange=5000", "MDSAttackCooldown=0.1", "-MDSAutoSelectLevelUp",
    "MDSLevelUpChoiceIndex=0", "-MDSAutoPurchaseShop", "MDSShopOfferIndex=2", "-MDSAutoSettlementAction"
)

$Server = $null; $Client1 = $null; $Client2 = $null
try {
    $Server = Start-LoggedProcess $ServerExe $ServerArgs $ServerLog
    $Listening = $false
    for ($Second = 0; $Second -lt $ServerWaitSeconds; ++$Second) {
        Start-Sleep -Seconds 1
        if (netstat -ano | Select-String ":$Port\s") { $Listening = $true; break }
    }
    if (-not $Listening) { throw "Dedicated server did not listen on UDP port $Port." }
    $Client1 = Start-LoggedProcess $ClientExe $ClientArgs $Client1Log
    $Client2 = Start-LoggedProcess $ClientExe $ClientArgs $Client2Log
    Start-Sleep -Seconds $RuntimeSeconds
}
finally {
    Stop-VerifyProcess $Client2
    Stop-VerifyProcess $Client1
    Stop-VerifyProcess $Server
}

$ServerText = Get-Content $ServerLog -Raw
$ClientText = (Get-Content $Client1Log -Raw) + (Get-Content $Client2Log -Raw)
$KnownWarning = "LogWindows: Error: Failed to open the Windows Event Log for writing (5)"
$Fatal = $ServerText.Replace($KnownWarning, "") -match "Fatal error|Ensure condition failed" -or
    $ClientText.Replace($KnownWarning, "") -match "Fatal error|Ensure condition failed"
$Checks = [ordered]@{
    "Two clients connected" = ([regex]::Matches($ServerText, "Join succeeded").Count -ge 2)
    "Round 1 started" = $ServerText -match "Round combat started on server: Round=1"
    "Level-up suspension occurred" = $ServerText -match "MDS LevelUp \| FlowState=EMDSLevelUpFlowState::Selection"
    "Level-up effect applied" = $ServerText -match "MDS LevelUp \| ChoiceApplied"
    "Shop offers published" = $ServerText -match "MDS Shop \| OffersPublished \| Count=3"
    "Shop purchase applied" = $ServerText -match "MDS Shop \| PurchaseApplied"
    "All players ready" = $ServerText -match "MDS Settlement \| AllPlayersReady \| Count=2"
    "Round 2 started" = $ServerText -match "Round combat started on server: Round=2"
    "Final settlement observed" = $ServerText -match "MDS Settlement \| Began \| Round=2 \| Final=true"
    "Match finished" = $ServerText -match "MDS Settlement \| MatchFinished \| Round=2"
    "Client replication observed" = $ClientText -match "Match state replicated on client"
    "No fatal runtime error" = -not $Fatal
}
$Checks.GetEnumerator() | ForEach-Object { Write-Host ("{0}: {1}" -f $_.Key, $(if ($_.Value) { "PASS" } else { "FAIL" })) }
if ($Checks.Values -contains $false) { throw "R12 DEDICATED MVP FLOW VERIFY RESULT: FAIL" }
Write-Host "R12 DEDICATED MVP FLOW VERIFY RESULT: PASS"
