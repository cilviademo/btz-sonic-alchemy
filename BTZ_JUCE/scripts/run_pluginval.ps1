# BTZ pluginval Validation Script (Windows PowerShell)
# Runs official VST3 validator with strict settings

param(
    [string]$BuildDir = "build",
    [int]$Strictness = 10,
    [int]$TimeoutMs = 30000
)

Write-Host "========================================"
Write-Host "BTZ - pluginval Validation (Windows)"
Write-Host "========================================"
Write-Host ""

# Check if pluginval exists
$pluginvalPath = Get-Command pluginval.exe -ErrorAction SilentlyContinue
if (-not $pluginvalPath) {
    # Try local directory
    if (Test-Path ".\pluginval.exe") {
        $pluginvalPath = ".\pluginval.exe"
    } else {
        Write-Host "ERROR: pluginval.exe not found!" -ForegroundColor Red
        Write-Host ""
        Write-Host "Download from: https://github.com/Tracktion/pluginval/releases"
        Write-Host "Place pluginval.exe in current directory or add to PATH"
        exit 1
    }
} else {
    $pluginvalPath = "pluginval.exe"
}

# Check build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "ERROR: Build directory not found: $BuildDir" -ForegroundColor Red
    Write-Host "Run cmake build first:"
    Write-Host "  cmake -B build -G 'Visual Studio 17 2022' -A x64"
    Write-Host "  cmake --build build --config Release"
    exit 1
}

# Locate VST3
$vst3Path = Get-ChildItem -Path "$BuildDir\BTZ_artefacts\Release\VST3" -Filter "*.vst3" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1

if (-not $vst3Path) {
    Write-Host "ERROR: No VST3 plugin found in build artifacts!" -ForegroundColor Red
    exit 1
}

Write-Host "Found plugin: $($vst3Path.FullName)" -ForegroundColor Green
Write-Host ""

# Create reports directory
$reportsDir = "pluginval_reports"
New-Item -ItemType Directory -Force -Path $reportsDir | Out-Null

# Run pluginval
Write-Host "Running pluginval with strictness level $Strictness..."
Write-Host "Timeout: $TimeoutMs ms"
Write-Host ""

$outputFile = "pluginval_output_windows.txt"

& $pluginvalPath `
    --strictness-level $Strictness `
    --validate-in-process `
    --timeout-ms $TimeoutMs `
    --verbose `
    --output-dir $reportsDir `
    $vst3Path.FullName 2>&1 | Tee-Object -FilePath $outputFile

# Check results
$exitCode = $LASTEXITCODE
$output = Get-Content $outputFile -Raw

Write-Host ""
Write-Host "========================================"
if ($output -match "FAILED|failed" -or $exitCode -ne 0) {
    Write-Host "❌ pluginval FAILED" -ForegroundColor Red
    Write-Host "Check $outputFile for details"
    exit 1
} else {
    Write-Host "✓ pluginval PASSED" -ForegroundColor Green
    Write-Host "Results: 0 plugins failed, 1 passed"
    exit 0
}
