#==============================================================================
# BTZ Windows Code Signing Script
#
# Purpose: Ship Gate #5 - Code sign Windows VST3 plugin
#
# Prerequisites:
#   - Valid Code Signing certificate (.pfx file)
#   - Windows SDK installed (for signtool.exe)
#   - Certificate password (stored securely)
#
# Usage:
#   .\sign_windows.ps1 -CertPath "path\to\cert.pfx" -CertPassword "password"
#==============================================================================

param(
    [Parameter(Mandatory=$false)]
    [string]$CertPath = $env:CODE_SIGNING_CERT_PATH,

    [Parameter(Mandatory=$false)]
    [SecureString]$CertPassword = $null,

    [Parameter(Mandatory=$false)]
    [string]$TimestampUrl = "http://timestamp.digicert.com",

    [Parameter(Mandatory=$false)]
    [switch]$Help
)

if ($Help) {
    Write-Host "BTZ Windows Code Signing Script"
    Write-Host ""
    Write-Host "Usage: .\sign_windows.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -CertPath PATH       Path to .pfx certificate file"
    Write-Host "  -CertPassword PWD    Certificate password (SecureString)"
    Write-Host "  -TimestampUrl URL    Timestamp server URL"
    Write-Host "  -Help                Show this help"
    Write-Host ""
    Write-Host "Environment Variables:"
    Write-Host "  CODE_SIGNING_CERT_PATH    Path to certificate (.pfx)"
    Write-Host "  CODE_SIGNING_CERT_PASS    Certificate password"
    Write-Host ""
    Write-Host "Example:"
    Write-Host '  .\sign_windows.ps1 -CertPath "C:\certs\mycert.pfx"'
    exit 0
}

Write-Host "========================================"
Write-Host "BTZ Windows Code Signing"
Write-Host "========================================"

# Check for certificate
if (-not $CertPath) {
    Write-Host "ERROR: Certificate path not specified" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please provide certificate path:"
    Write-Host '  .\sign_windows.ps1 -CertPath "C:\path\to\cert.pfx"'
    Write-Host ""
    Write-Host "Or set environment variable:"
    Write-Host '  $env:CODE_SIGNING_CERT_PATH = "C:\path\to\cert.pfx"'
    exit 1
}

if (-not (Test-Path $CertPath)) {
    Write-Host "ERROR: Certificate not found: $CertPath" -ForegroundColor Red
    exit 1
}

# Get password if not provided
if (-not $CertPassword) {
    if ($env:CODE_SIGNING_CERT_PASS) {
        $CertPassword = ConvertTo-SecureString $env:CODE_SIGNING_CERT_PASS -AsPlainText -Force
    } else {
        $CertPassword = Read-Host "Enter certificate password" -AsSecureString
    }
}

# Find signtool
$signtoolPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe"
)

$signtool = $null
foreach ($path in $signtoolPaths) {
    if (Test-Path $path) {
        $signtool = $path
        break
    }
}

if (-not $signtool) {
    Write-Host "ERROR: signtool.exe not found" -ForegroundColor Red
    Write-Host "Please install Windows SDK"
    exit 1
}

Write-Host "Using signtool: $signtool"
Write-Host "Certificate: $CertPath"
Write-Host ""

# Find VST3
$vst3Path = "BTZ_JUCE\build\BTZ_artefacts\Release\VST3\BTZ - The Box Tone Zone.vst3\Contents\x86_64-win\BTZ - The Box Tone Zone.vst3"

if (-not (Test-Path $vst3Path)) {
    Write-Host "ERROR: VST3 binary not found" -ForegroundColor Red
    Write-Host "Expected: $vst3Path"
    Write-Host ""
    Write-Host "Please build the plugin first:"
    Write-Host "  cd BTZ_JUCE"
    Write-Host "  cmake --build build --config Release"
    exit 1
}

Write-Host "Signing VST3..."

# Convert SecureString to plain text for signtool
$BSTR = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($CertPassword)
$plainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR)

# Sign the binary
& $signtool sign `
    /f "$CertPath" `
    /p "$plainPassword" `
    /t "$TimestampUrl" `
    /fd SHA256 `
    /v `
    "$vst3Path"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Signing failed" -ForegroundColor Red
    exit 1
}

# Verify signature
Write-Host ""
Write-Host "Verifying signature..."
& $signtool verify /pa /v "$vst3Path"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Verification failed" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Ship Gate #5: Windows Signing COMPLETE" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Signed: $vst3Path"
Write-Host ""
Write-Host "Next steps:"
Write-Host "1. Test installation on clean Windows system"
Write-Host "2. Verify no SmartScreen warnings"
Write-Host "3. Update Ship Gate #5 status to PASS"
