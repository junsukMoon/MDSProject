param(
	[int]$Port = 7830,
	[int]$ServerWaitSeconds = 15,
	[int]$ClientWaitSeconds = 18,
	[switch]$SkipBuild,
	[switch]$SkipStage
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$PresentationScript = Join-Path $Root "Run_Verify_CombatPresentationHooks.ps1"
if (-not (Test-Path -LiteralPath $PresentationScript)) {
	throw "Combat presentation verification script was not found at $PresentationScript"
}

$Arguments = @(
	"-NoProfile",
	"-ExecutionPolicy", "Bypass",
	"-File", $PresentationScript,
	"-Port", $Port,
	"-ServerWaitSeconds", $ServerWaitSeconds,
	"-ClientWaitSeconds", $ClientWaitSeconds,
	"-VerifyAnimNotify"
)
if ($SkipBuild) {
	$Arguments += "-SkipBuild"
}
if ($SkipStage) {
	$Arguments += "-SkipStage"
}

& powershell.exe @Arguments
exit $LASTEXITCODE
