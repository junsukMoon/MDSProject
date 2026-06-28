@echo off
setlocal

set "ROOT=%~dp0"
set "SERVER_EXE=%ROOT%MDSProject\Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64\MDSProjectServer.exe"
set "LOG_FILE=%ROOT%MDSProject\Saved\Logs\Manual_DedicatedServer_Console.log"
set "SERVER_DIR=%ROOT%MDSProject\Saved\StagedBuilds\WindowsServer\MDSProject\Binaries\Win64"

if not exist "%SERVER_EXE%" (
    echo Dedicated server executable was not found:
    echo "%SERVER_EXE%"
    echo.
    echo Build, cook, and stage the WindowsServer target first.
    pause
    exit /b 1
)

echo Starting MDSProject dedicated server.
echo.
echo Server:
echo "%SERVER_EXE%"
echo.
echo Log:
echo "%LOG_FILE%"
echo.
echo Press Ctrl+C to stop the server.
echo If port 7777 is already in use, close the existing server process first.
echo.

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$serverExe = '%SERVER_EXE%';" ^
    "$serverDir = '%SERVER_DIR%';" ^
    "$logFile = '%LOG_FILE%';" ^
    "$logDir = Split-Path -Parent $logFile;" ^
    "New-Item -ItemType Directory -Force -Path $logDir | Out-Null;" ^
    "Remove-Item -LiteralPath $logFile -Force -ErrorAction SilentlyContinue;" ^
    "$portUsers = netstat -ano | Select-String ':7777';" ^
    "if ($portUsers) { Write-Host ''; Write-Host 'WARNING: Port 7777 appears to be in use:' -ForegroundColor Yellow; $portUsers | ForEach-Object { Write-Host $_ }; Write-Host ''; Write-Host 'Close the existing server before continuing, or this run may fail.' -ForegroundColor Yellow; Write-Host 'Press Enter to continue anyway, or Ctrl+C to cancel.'; [void][Console]::ReadLine(); }" ^
    "$serverArgs = '/Game/TopDown/Lvl_TopDown -NullRHI -unattended -stdout -FullStdOutLogOutput -forcelogflush -port=7777 -abslog=\"' + $logFile + '\"';" ^
    "Write-Host 'Launching dedicated server process...';" ^
    "$server = Start-Process -FilePath $serverExe -ArgumentList $serverArgs -WorkingDirectory $serverDir -PassThru;" ^
    "Write-Host ('Server PID: ' + $server.Id);" ^
    "Write-Host ('Live log: ' + $logFile);" ^
    "Write-Host ''; Write-Host 'Press Ctrl+C to stop watching the log. Close the server process when finished.';" ^
    "while (-not (Test-Path -LiteralPath $logFile) -and -not $server.HasExited) { Start-Sleep -Milliseconds 250; $server.Refresh(); }" ^
    "if (Test-Path -LiteralPath $logFile) { Get-Content -LiteralPath $logFile -Wait } else { Write-Host 'Server exited before writing the log.' -ForegroundColor Red; exit $server.ExitCode }"

echo.
echo Console log watcher exited.
pause
