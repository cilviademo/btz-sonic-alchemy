# Remove the CMake build directory (safe — never touches source).
# Usage: pwsh scripts/clean_build.ps1
$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BtzDir    = Resolve-Path (Join-Path $ScriptDir "..\btz-sonic-alchemy-main\BTZ")
$Build     = Join-Path $BtzDir "build"
if (Test-Path $Build) {
    Write-Host ">> Removing $Build"
    Remove-Item -Recurse -Force $Build
    Write-Host ">> Clean."
} else {
    Write-Host ">> Nothing to clean ($Build does not exist)."
}
