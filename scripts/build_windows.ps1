# ─────────────────────────────────────────────────────────────────────────────
# BTZ Sonic Alchemy — Windows build script (Visual Studio 2022 + CMake).
# Usage:  pwsh scripts/build_windows.ps1 [-Config Release|Debug] [-Tests] [-Clap]
# Prereqs: Visual Studio 2022 with "Desktop development with C++", CMake, Git.
#          Run from a "Developer PowerShell for VS 2022" or have CMake find VS.
# ─────────────────────────────────────────────────────────────────────────────
param(
    [string]$Config = "Release",
    [switch]$Tests,
    [switch]$Clap
)
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BtzDir    = Resolve-Path (Join-Path $ScriptDir "..\btz-sonic-alchemy-main\BTZ")
Write-Host ">> BTZ project: $BtzDir  Config: $Config  Tests: $Tests  CLAP: $Clap"

$testsFlag = if ($Tests) { "ON" } else { "OFF" }
$clapFlag  = if ($Clap)  { "ON" } else { "OFF" }

Push-Location $BtzDir
try {
    # Multi-config VS generator: -DCMAKE_BUILD_TYPE is ignored; pass --config at build.
    cmake -B build -G "Visual Studio 17 2022" -A x64 `
        "-DBTZ_BUILD_TESTS=$testsFlag" `
        "-DBTZ_BUILD_CLAP=$clapFlag"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

    $targets = @("BTZ_VST3", "BTZ_Standalone")
    if ($Tests) { $targets += "BTZTests" }
    cmake --build build --config $Config --target $targets
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }

    Write-Host ">> Artefacts under: $BtzDir\build\BTZ_artefacts\$Config\ (VST3, Standalone)"
    if ($Tests) { ctest --test-dir build -C $Config --output-on-failure }
}
finally { Pop-Location }
Write-Host ">> Done."
