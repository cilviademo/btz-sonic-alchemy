# Build BTZ VST3 (Release) from repo root. Uses Ninja if available.
$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptDir "..")
$BTZ = Join-Path $RepoRoot "btz-sonic-alchemy-main\BTZ"
if (-not (Test-Path (Join-Path $BTZ "CMakeLists.txt"))) {
    $BTZ = Join-Path $RepoRoot "BTZ"
}
$BuildDir = Join-Path $BTZ "build"

if (-not (Test-Path (Join-Path $BTZ "CMakeLists.txt"))) {
    Write-Error "BTZ folder not found at $BTZ"
    exit 1
}

Write-Host "Building BTZ plugin in $BuildDir"
if (-not (Test-Path $BuildDir)) { New-Item -ItemType Directory -Path $BuildDir | Out-Null }
Set-Location $BuildDir

$useNinja = $null -ne (Get-Command ninja -ErrorAction SilentlyContinue)
if ($useNinja) {
    cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
} else {
    cmake .. -DCMAKE_BUILD_TYPE=Release
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build . --config Release -j 8
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Build done. VST3: $BuildDir\VST3\BTZ.vst3"
