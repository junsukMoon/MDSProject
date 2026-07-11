param(
    [int]$Port = 7777,
    [int]$ServerWaitSeconds = 15,
    [int]$ClientWaitSeconds = 15
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Join-Path $Root "MDSProject"
$ServerExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProjectServer.exe"
$ClientExe = Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Binaries\Win64\MDSProject.exe"
$StagedServerExe = Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
$BuiltClientExe = Join-Path $ProjectRoot "Binaries\Win64\MDSProject.exe"
$TempDir = "C:\Temp"
$ServerLog = Join-Path $TempDir "MDS_Smoke_Server.log"
$ClientLog = Join-Path $TempDir "MDS_Smoke_Client.log"

function Stop-IfRunning {
    param([System.Diagnostics.Process]$Process)

    if ($Process -and -not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Find-RecentLogs {
    param([datetime]$Since)

    $SearchRoots = @(
        (Join-Path $ProjectRoot "Saved\Logs"),
        (Join-Path $ProjectRoot "Saved\Cooked\WindowsServer\MDSProject\Saved\Logs"),
        (Join-Path $ProjectRoot "Saved\Cooked\WindowsServer\MDSProject\Binaries\Win64"),
        (Join-Path $ProjectRoot "Saved\Cooked\Windows\MDSProject\Saved\Logs"),
        (Join-Path $ProjectRoot "Saved\Cooked\Windows\MDSProject\Binaries\Win64"),
        (Join-Path $ProjectRoot "Saved\StagedBuilds\WindowsServer\MDSProject\Saved\Logs"),
        (Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\MDSProject\Saved\Logs"),
        (Join-Path $env:LOCALAPPDATA "MDSProject\Saved\Logs"),
        (Join-Path $env:LOCALAPPDATA "MDSProjectServer\Saved\Logs"),
        $TempDir
    )

    foreach ($SearchRoot in $SearchRoots) {
        if (Test-Path -LiteralPath $SearchRoot) {
            Get-ChildItem -LiteralPath $SearchRoot -Filter "*.log" -File -ErrorAction SilentlyContinue |
                Where-Object { $_.LastWriteTime -ge $Since } |
                Sort-Object LastWriteTime -Descending |
                ForEach-Object { $_.FullName }
        }
    }
}

function Select-LogPatterns {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    Select-String -Path $Path -Pattern "IpNetDriver listening|NetMode=DedicatedServer|NetMode=Client|ObjectiveHP=|Mass Spawned|Join succeeded|Login request|Error|Warning|Fatal" |
        Select-Object -Last 80
}

if (-not (Test-Path -LiteralPath $ServerExe)) {
    if (Test-Path -LiteralPath $StagedServerExe) {
        $ServerExe = $StagedServerExe
    } else {
        throw "No server executable found. Build MDSProjectServer first."
    }
}

if (-not (Test-Path -LiteralPath $ClientExe)) {
    if (Test-Path -LiteralPath $BuiltClientExe) {
        $ClientExe = $BuiltClientExe
    } else {
        throw "No client executable found. Build MDSProject first."
    }
}

New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
Remove-Item -LiteralPath $ServerLog -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ClientLog -Force -ErrorAction SilentlyContinue

$PortUsers = netstat -ano | Select-String ":$Port"
if ($PortUsers) {
    throw "Port $Port is already in use:`n$PortUsers"
}

$StartTime = Get-Date
$Server = $null
$Client = $null

try {
    $ServerDir = Split-Path -Parent $ServerExe
    $ClientDir = Split-Path -Parent $ClientExe

    $ServerArgs = @(
        "/Game/TopDown/Lvl_TopDown",
        "-NullRHI",
        "-unattended",
        "-stdout",
        "-FullStdOutLogOutput",
        "-forcelogflush",
        "-port=$Port"
    )

    Write-Host "Launching server:"
    Write-Host "  $ServerExe"
    Write-Host "  $($ServerArgs -join ' ')"
    $Server = Start-Process -FilePath $ServerExe -ArgumentList $ServerArgs -WorkingDirectory $ServerDir -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds $ServerWaitSeconds
    $Server.Refresh()

    Write-Host "Server PID=$($Server.Id) HasExited=$($Server.HasExited) ExitCode=$(if ($Server.HasExited) { $Server.ExitCode } else { 'Running' })"
    $ListenLine = netstat -ano | Select-String ":$Port"
    if ($ListenLine) {
        Write-Host "Port $Port listen detected:"
        $ListenLine | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "Port $Port listen was not detected."
    }

    $ClientArgs = @(
        "-NullRHI",
        "-unattended",
        "-nosound",
        "-NoSplash",
        "-stdout",
        "-FullStdOutLogOutput",
        "-forcelogflush",
        "127.0.0.1:$Port"
    )

    Write-Host "Launching client:"
    Write-Host "  $ClientExe"
    Write-Host "  $($ClientArgs -join ' ')"
    $Client = Start-Process -FilePath $ClientExe -ArgumentList $ClientArgs -WorkingDirectory $ClientDir -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds $ClientWaitSeconds
    $Client.Refresh()

    Write-Host "Client PID=$($Client.Id) HasExited=$($Client.HasExited) ExitCode=$(if ($Client.HasExited) { $Client.ExitCode } else { 'Running' })"

    $Logs = @(Find-RecentLogs -Since $StartTime | Select-Object -Unique)
    if ($Logs.Count -eq 0) {
        Write-Host "No recent log files were found."
    } else {
        Write-Host "Recent logs:"
        $Logs | ForEach-Object { Write-Host "  $_" }
    }

    foreach ($Log in $Logs) {
        Write-Host "---- Patterns from $Log ----"
        $Matches = @(Select-LogPatterns -Path $Log)
        if ($Matches.Count -eq 0) {
            Write-Host "No smoke patterns found."
        } else {
            $Matches | ForEach-Object { Write-Host $_.Line }
        }
    }

    $ServerPatterns = @()
    $ClientPatterns = @()
    foreach ($Log in $Logs) {
        $Text = Get-Content -LiteralPath $Log -Raw -ErrorAction SilentlyContinue
        if ($Text -match "NetMode=DedicatedServer|IpNetDriver listening|Mass Spawned") {
            $ServerPatterns += $Text
        }
        if ($Text -match "NetMode=Client|127\.0\.0\.1|Join succeeded") {
            $ClientPatterns += $Text
        }
    }

    $ServerOk = ($ServerPatterns -join "`n") -match "ObjectiveHP=20/100" -and ($ServerPatterns -join "`n") -match "Mass Spawned=16" -and ($ServerPatterns -join "`n") -match "Arrived=16" -and ($ServerPatterns -join "`n") -match "Damage=16"
    $ClientOk = ($ClientPatterns -join "`n") -match "NetMode=Client" -and ($ClientPatterns -join "`n") -match "ObjectiveHP=20/100"

    if ($ServerOk -and $ClientOk) {
        Write-Host "SMOKE RESULT: PASS"
        exit 0
    }

    Write-Host "SMOKE RESULT: INCOMPLETE"
    Write-Host "Server objective patterns found: $ServerOk"
    Write-Host "Client replication patterns found: $ClientOk"
    exit 2
}
finally {
    Stop-IfRunning -Process $Client
    Stop-IfRunning -Process $Server
}
