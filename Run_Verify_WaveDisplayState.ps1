param(
    [int]$Port = 7777,
    [int]$ServerWaitSeconds = 15,
    [int]$ClientWaitSeconds = 15
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$StagedServerRootExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProjectServer.exe"
$StagedServerNestedExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$BuiltServerExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectServer.exe"
$StagedClientRootExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject.exe"
$StagedClientNestedExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$BuiltClientExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProject.exe"
$TempDir = Join-Path $Root "SavedVerifyLogs"
$ServerLog = Join-Path $TempDir "MDS_WaveVerify_Server.log"
$ClientLog = Join-Path $TempDir "MDS_WaveVerify_Client.log"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Select-WavePatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "IpNetDriver listening|Initialized wave display state|Wave state set on server|Wave state replicated on client|MDS Debug \| NetMode=DedicatedServer|MDS Debug \| NetMode=Client|Wave=1 Active=false Remaining=0/0|ObjectiveHP=|Join succeeded|Login request|Error|Warning|Fatal" |
        Select-Object -Last 120
}

$ServerCandidates = @($StagedServerNestedExe, $BuiltServerExe, $StagedServerRootExe) |
    Where-Object { Test-Path -LiteralPath $_ } |
    ForEach-Object { Get-Item -LiteralPath $_ }

if ($ServerCandidates.Count -gt 0) {
    $ServerExe = @($ServerCandidates | Select-Object -First 1).FullName
} else {
    throw "No server executable found. Build or stage MDSProjectServer first."
}

$ClientCandidates = @($StagedClientNestedExe, $BuiltClientExe, $StagedClientRootExe) |
    Where-Object { Test-Path -LiteralPath $_ } |
    ForEach-Object { Get-Item -LiteralPath $_ }

if ($ClientCandidates.Count -gt 0) {
    $ClientExe = @($ClientCandidates | Select-Object -First 1).FullName
} else {
    throw "No client executable found. Build or stage MDSProject first."
}

New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
Remove-Item -LiteralPath $ServerLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue

$PortUsers = netstat -ano | Select-String ":$Port"
if ($PortUsers) {
    throw "Port $Port is already in use:`n$PortUsers"
}

$ServerJob = $null
$ClientJob = $null

try {
    $ServerDir = Split-Path -Parent $ServerExe
    $ClientDir = Split-Path -Parent $ClientExe

    Write-Host "Launching server:"
    Write-Host "  $ServerExe"
    Write-Host "  /Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -port=$Port"
    $ServerJob = Start-Job -ScriptBlock {
        param($Exe, $Dir, $Out, $ListenPort)

        Set-Location -LiteralPath $Dir
        & $Exe "/Game/TopDown/Lvl_TopDown" -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush "-port=$ListenPort" *>&1 |
            Out-File -FilePath $Out -Encoding utf8
    } -ArgumentList $ServerExe, $ServerDir, $ServerLog, $Port

    Start-Sleep -Seconds $ServerWaitSeconds
    Receive-Job $ServerJob -Keep | Out-Null

    $ListenLine = netstat -ano | Select-String ":$Port"
    if ($ListenLine) {
        Write-Host "Port $Port listen detected:"
        $ListenLine | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "Port $Port listen was not detected."
    }

    Write-Host "Launching client:"
    Write-Host "  $ClientExe"
    Write-Host "  127.0.0.1:$Port -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush"
    $ClientJob = Start-Job -ScriptBlock {
        param($Exe, $Dir, $Out, $ConnectAddress)

        Set-Location -LiteralPath $Dir
        & $Exe $ConnectAddress -NullRHI -unattended -nosound -NoSplash -stdout -FullStdOutLogOutput -forcelogflush *>&1 |
            Out-File -FilePath $Out -Encoding utf8
    } -ArgumentList $ClientExe, $ClientDir, $ClientLog, "127.0.0.1:$Port"

    Start-Sleep -Seconds $ClientWaitSeconds
    Receive-Job $ClientJob -Keep | Out-Null

    $LogPaths = @($ServerLog, $ClientLog)
    foreach ($Log in $LogPaths) {
        Write-Host "---- Patterns from $Log ----"
        $Matches = @(Select-WavePatterns -Path $Log)
        if ($Matches.Count -eq 0) {
            Write-Host "No wave verification patterns found."
        } else {
            $Matches | ForEach-Object { Write-Host $_.Line }
        }
    }

    $ServerText = if (Test-Path -LiteralPath $ServerLog) { Get-Content -LiteralPath $ServerLog -Raw -ErrorAction SilentlyContinue } else { "" }
    $ClientText = if (Test-Path -LiteralPath $ClientLog) { Get-Content -LiteralPath $ClientLog -Raw -ErrorAction SilentlyContinue } else { "" }

    $ServerInitOk = $ServerText -match "Initialized wave display state on server: Wave=1 Remaining=0 Total=0 Active=false"
    $ServerStateOk = $ServerText -match "Wave state set on server: Wave=1 Remaining=0 Total=0 Active=false"
    $ServerDebugOk = $ServerText -match "MDS Debug \| NetMode=DedicatedServer \| Wave=1 Active=false Remaining=0/0"
    $ClientConnectedOk = $ClientText -match "NetMode=Client|Join succeeded|Login request|Welcomed by server"
    $ClientWaveOk = $ClientText -match "Wave state replicated on client: Wave=1 Remaining=0 Total=0 Active=false|MDS Debug \| NetMode=Client \| Wave=1 Active=false Remaining=0/0"

    if ($ServerInitOk -and $ServerStateOk -and $ServerDebugOk -and $ClientConnectedOk -and $ClientWaveOk) {
        Write-Host "WAVE VERIFY RESULT: PASS"
        exit 0
    }

    Write-Host "WAVE VERIFY RESULT: INCOMPLETE"
    Write-Host "Server init found: $ServerInitOk"
    Write-Host "Server GameState set found: $ServerStateOk"
    Write-Host "Server debug line found: $ServerDebugOk"
    Write-Host "Client connection found: $ClientConnectedOk"
    Write-Host "Client replicated wave found: $ClientWaveOk"
    Write-Host "If server wave patterns are missing, the staged server binary may not include the latest C++ changes."
    exit 2
}
finally {
    if ($ClientJob) {
        Stop-Job $ClientJob -ErrorAction SilentlyContinue
        Remove-Job $ClientJob -Force -ErrorAction SilentlyContinue
    }

    if ($ServerJob) {
        Stop-Job $ServerJob -ErrorAction SilentlyContinue
        Remove-Job $ServerJob -Force -ErrorAction SilentlyContinue
    }
}
