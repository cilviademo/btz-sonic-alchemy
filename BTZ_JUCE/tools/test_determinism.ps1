# BTZ Offline Bounce Determinism Test (Windows PowerShell)
#
# Purpose: Verify that BTZ produces identical output for identical input
# across multiple render passes (required for professional workflows)
#
# Requirements:
#   - offline_render.exe built (cmake -DBTZ_BUILD_TOOLS=ON)
#   - Test audio file (pink noise recommended)
#
# Usage:
#   .\test_determinism.ps1 [InputFile] [NumBounces]
#
# Example:
#   .\test_determinism.ps1 test_audio.wav 5

param(
    [string]$InputFile = "test_input.wav",
    [int]$NumBounces = 5
)

$OutputDir = "determinism_test_outputs"
$OfflineRender = "..\build\tools\Release\offline_render.exe"

Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host "BTZ Offline Bounce Determinism Test" -ForegroundColor Cyan
Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Configuration:"
Write-Host "  Input file: $InputFile"
Write-Host "  Number of bounces: $NumBounces"
Write-Host "  Output directory: $OutputDir"
Write-Host ""

# Check if offline_render tool exists
if (!(Test-Path $OfflineRender)) {
    Write-Host "❌ ERROR: offline_render.exe not found" -ForegroundColor Red
    Write-Host "Build with: cmake -DBTZ_BUILD_TOOLS=ON" -ForegroundColor Yellow
    exit 1
}

# Check if input file exists
if (!(Test-Path $InputFile)) {
    Write-Host "❌ ERROR: Input file not found: $InputFile" -ForegroundColor Red
    Write-Host ""
    Write-Host "Export 10 seconds of pink noise from your DAW at -18 dBFS RMS" -ForegroundColor Yellow
    exit 1
}

# Clean output directory
if (Test-Path $OutputDir) {
    Write-Host "Cleaning previous test outputs..."
    Remove-Item -Recurse -Force $OutputDir
}
New-Item -ItemType Directory -Path $OutputDir | Out-Null

Write-Host "✅ Pre-flight checks passed" -ForegroundColor Green
Write-Host ""

# Render multiple bounces
Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host "Rendering $NumBounces consecutive bounces..." -ForegroundColor Cyan
Write-Host "===================================================================" -ForegroundColor Cyan

$StartTime = Get-Date

for ($i = 1; $i -le $NumBounces; $i++) {
    Write-Host ""
    Write-Host "--- Bounce $i/$NumBounces ---"

    $OutputFile = "$OutputDir\bounce_$i.wav"
    $LogFile = "$OutputDir\bounce_${i}_log.txt"

    & $OfflineRender $InputFile $OutputFile > $LogFile 2>&1

    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ FAIL: Render $i failed" -ForegroundColor Red
        Get-Content $LogFile
        exit 1
    }

    Write-Host "  ✓ Bounce $i complete"
}

$EndTime = Get-Date
$Elapsed = ($EndTime - $StartTime).TotalSeconds

Write-Host ""
Write-Host "✅ All $NumBounces bounces rendered successfully" -ForegroundColor Green
Write-Host "Total time: $([math]::Round($Elapsed, 2))s"
Write-Host ""

# Compute MD5 hashes
Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host "Computing MD5 hashes..." -ForegroundColor Cyan
Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host ""

$Hashes = @{}
for ($i = 1; $i -le $NumBounces; $i++) {
    $Hash = (Get-FileHash -Algorithm MD5 "$OutputDir\bounce_$i.wav").Hash
    $Hashes[$i] = $Hash
    Write-Host "Bounce $i: $Hash"
}

Write-Host ""

# Check if all hashes match
$ReferenceHash = $Hashes[1]
$AllMatch = $true

for ($i = 2; $i -le $NumBounces; $i++) {
    if ($Hashes[$i] -ne $ReferenceHash) {
        $AllMatch = $false
        break
    }
}

Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host "DETERMINISM TEST RESULT" -ForegroundColor Cyan
Write-Host "===================================================================" -ForegroundColor Cyan
Write-Host ""

if ($AllMatch) {
    Write-Host "✅ PASS: All $NumBounces bounces are IDENTICAL" -ForegroundColor Green
    Write-Host ""
    Write-Host "Reference MD5: $ReferenceHash"
    Write-Host ""
    Write-Host "BTZ produces deterministic output. This is REQUIRED for:"
    Write-Host "  • Professional mixing/mastering workflows"
    Write-Host "  • Collaborative projects"
    Write-Host "  • Bounce/export consistency"
    Write-Host "  • Regression testing"
    Write-Host ""
    exit 0
} else {
    Write-Host "❌ FAIL: Bounces DIFFER (non-deterministic processing)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Hash comparison:"
    for ($i = 1; $i -le $NumBounces; $i++) {
        if ($Hashes[$i] -eq $ReferenceHash) {
            Write-Host "  Bounce $i: $($Hashes[$i]) ✓" -ForegroundColor Green
        } else {
            Write-Host "  Bounce $i: $($Hashes[$i]) ✗ (MISMATCH)" -ForegroundColor Red
        }
    }
    Write-Host ""
    Write-Host "Possible causes:" -ForegroundColor Yellow
    Write-Host "  • Random number generators without fixed seed"
    Write-Host "  • Uninitialized memory reads"
    Write-Host "  • Non-deterministic algorithms (std::unordered_map iteration, etc.)"
    Write-Host "  • System clock dependencies in DSP code"
    Write-Host "  • Thread timing issues"
    Write-Host "  • Floating-point non-associativity (rare)"
    Write-Host ""
    Write-Host "Debug steps:"
    Write-Host "  1. Check for rand(), srand(), std::random_device in DSP code"
    Write-Host "  2. Check for uninitialized variables (use ASAN/valgrind)"
    Write-Host "  3. Check for std::unordered_map, std::unordered_set in hot path"
    Write-Host "  4. Check for std::chrono or time() calls in DSP"
    Write-Host "  5. Enable deterministic mode in JUCE (if available)"
    Write-Host ""
    exit 1
}
