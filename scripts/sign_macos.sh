#!/bin/bash

#==============================================================================
# BTZ macOS Code Signing & Notarization Script
#
# Purpose: Ship Gate #5 - Code sign and notarize macOS plugins
#
# Prerequisites:
#   - Apple Developer account
#   - Valid Developer ID Application certificate in Keychain
#   - App-specific password for notarization
#   - Xcode Command Line Tools installed
#
# Usage:
#   ./sign_macos.sh [--identity "Developer ID"] [--team-id TEAM_ID]
#==============================================================================

set -e

# Default values (REPLACE THESE WITH YOUR VALUES)
IDENTITY="${SIGNING_IDENTITY:-Developer ID Application: Your Name (TEAM_ID)}"
TEAM_ID="${APPLE_TEAM_ID:-YOUR_TEAM_ID}"
APPLE_ID="${APPLE_ID_EMAIL:-your.email@example.com}"
APP_PASSWORD="${APP_SPECIFIC_PASSWORD:-@keychain:AC_PASSWORD}"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --identity)
            IDENTITY="$2"
            shift 2
            ;;
        --team-id)
            TEAM_ID="$2"
            shift 2
            ;;
        --help)
            echo "BTZ macOS Code Signing & Notarization"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --identity ID    Developer ID (default: from env or placeholder)"
            echo "  --team-id ID     Apple Team ID (default: from env or placeholder)"
            echo "  --help           Show this help"
            echo ""
            echo "Environment Variables:"
            echo "  SIGNING_IDENTITY     Developer ID Application certificate name"
            echo "  APPLE_TEAM_ID        Your Apple Developer Team ID"
            echo "  APPLE_ID_EMAIL       Your Apple ID email"
            echo "  APP_SPECIFIC_PASSWORD  App-specific password (or @keychain:name)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "BTZ macOS Code Signing & Notarization"
echo "========================================"

# Check if placeholder values are still in use
if [[ "$IDENTITY" == *"Your Name"* ]] || [[ "$TEAM_ID" == "YOUR_TEAM_ID"* ]]; then
    echo -e "${RED}ERROR: Placeholder values detected${NC}"
    echo ""
    echo "Please configure your signing credentials:"
    echo "1. Export environment variables:"
    echo "   export SIGNING_IDENTITY=\"Developer ID Application: Your Name (TEAM_ID)\""
    echo "   export APPLE_TEAM_ID=\"YOUR_TEAM_ID\""
    echo "   export APPLE_ID_EMAIL=\"your.email@example.com\""
    echo ""
    echo "2. Or edit this script and replace the default values"
    echo ""
    echo "3. Store app-specific password in keychain:"
    echo "   xcrun notarytool store-credentials --apple-id your.email@example.com --team-id TEAM_ID"
    exit 1
fi

# Find built plugins
VST3_PATH="BTZ_JUCE/build/BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3"
AU_PATH="BTZ_JUCE/build/BTZ_artefacts/Release/AU/BTZ - The Box Tone Zone.component"

if [ ! -d "$VST3_PATH" ] && [ ! -d "$AU_PATH" ]; then
    echo -e "${RED}ERROR: No plugin bundles found${NC}"
    echo "Please build the plugins first:"
    echo "  cd BTZ_JUCE && cmake --build build --config Release"
    exit 1
fi

echo "Signing Identity: $IDENTITY"
echo "Team ID: $TEAM_ID"
echo ""

# Sign VST3
if [ -d "$VST3_PATH" ]; then
    echo "Signing VST3..."
    codesign --force --sign "$IDENTITY" \
        --options runtime \
        --deep \
        --timestamp \
        "$VST3_PATH"

    echo "Verifying VST3 signature..."
    codesign --verify --deep --strict --verbose=2 "$VST3_PATH"

    echo -e "${GREEN}✅ VST3 signed successfully${NC}"
    echo ""
fi

# Sign AU
if [ -d "$AU_PATH" ]; then
    echo "Signing AU..."
    codesign --force --sign "$IDENTITY" \
        --options runtime \
        --deep \
        --timestamp \
        "$AU_PATH"

    echo "Verifying AU signature..."
    codesign --verify --deep --strict --verbose=2 "$AU_PATH"

    echo -e "${GREEN}✅ AU signed successfully${NC}"
    echo ""
fi

# Create zip for notarization
echo "Creating notarization archive..."
NOTARIZE_ZIP="BTZ_macOS_Notarization.zip"
ditto -c -k --keepParent "$VST3_PATH" "$NOTARIZE_ZIP"

if [ -d "$AU_PATH" ]; then
    # If AU exists, recreate zip with both
    rm "$NOTARIZE_ZIP"
    zip -r "$NOTARIZE_ZIP" "$VST3_PATH" "$AU_PATH"
fi

echo -e "${GREEN}✅ Created: $NOTARIZE_ZIP${NC}"
echo ""

# Submit for notarization
echo "Submitting for notarization..."
echo "(This may take 5-15 minutes)"
echo ""

xcrun notarytool submit "$NOTARIZE_ZIP" \
    --apple-id "$APPLE_ID" \
    --team-id "$TEAM_ID" \
    --password "$APP_PASSWORD" \
    --wait

echo ""
echo -e "${GREEN}✅ Notarization complete${NC}"
echo ""

# Staple notarization ticket
if [ -d "$VST3_PATH" ]; then
    echo "Stapling VST3..."
    xcrun stapler staple "$VST3_PATH"
    echo -e "${GREEN}✅ VST3 stapled${NC}"
fi

if [ -d "$AU_PATH" ]; then
    echo "Stapling AU..."
    xcrun stapler staple "$AU_PATH"
    echo -e "${GREEN}✅ AU stapled${NC}"
fi

echo ""
echo "========================================"
echo -e "${GREEN}Ship Gate #5: Code Signing COMPLETE${NC}"
echo "========================================"
echo ""
echo "Signed and notarized:"
[ -d "$VST3_PATH" ] && echo "  ✅ $VST3_PATH"
[ -d "$AU_PATH" ] && echo "  ✅ $AU_PATH"
echo ""
echo "Next steps:"
echo "1. Test installation on clean macOS system"
echo "2. Verify no Gatekeeper warnings"
echo "3. Update Ship Gate #5 status to PASS"
