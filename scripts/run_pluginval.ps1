# ─────────────────────────────────────────────────────────────────────────────
# BTZ Sonic Alchemy — pluginval runner (Windows).
# Usage:  pwsh scripts/run_pluginval.ps1 [-Strictness 10] [-Plugin <path.vst3>]
# Download pluginval: https://github.com/Tracktion/pluginval/releases
# Set $env:PLUGINVAL to pluginval.exe, or place it on PATH.
# ─────────────────────────────────────────────────────────────────────────────
param(
    [int]$Strictness = 10,
    [string]$Plugin = ""
)
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BtzDir    = Resolve-Path (Join-Path $ScriptDir "..\btz-sonic-alchemy-main\BTZ")

if (-not $Plugin) {
    $Plugin = (Get-ChildItem -Path (Join-Path $BtzDir "build\BTZ_artefacts") -Recurse -Filter *.vst3 -ErrorAction SilentlyContinue | Select-Object -First 1).FullName
}
if (-not $Plugin) { throw "No VST3 found. Build first (scripts\build_windows.ps1)." }

$pv = if ($env:PLUGINVAL) { $env:PLUGINVAL } else { "pluginval" }
Write-Host ">> Validating: $Plugin  (strictness $Strictness)"
& $pv --validate "$Plugin" --strictness-level $Strictness --timeout-ms 120000 `
      --repeat-count 2 --randomise --validate-in-process
if ($LASTEXITCODE -ne 0) { throw "pluginval FAILED" }
Write-Host ">> pluginval passed at strictness $Strictness"
