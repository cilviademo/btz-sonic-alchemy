#!/bin/bash
# BTZ pluginval Validation Script
# Runs official VST3/AU validator with strict settings

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "BTZ - pluginval Validation"
echo "========================================"
echo ""

# Check if pluginval is installed
if ! command -v pluginval &> /dev/null; then
    echo -e "${RED}ERROR: pluginval not found!${NC}"
    echo ""
    echo "Install pluginval:"
    echo "  macOS: brew install pluginval"
    echo "  Linux: Download from https://github.com/Tracktion/pluginval/releases"
    echo "  Or build from source: https://github.com/Tracktion/pluginval"
    exit 1
fi

# Check build directory
BUILD_DIR="${BUILD_DIR:-build}"
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}ERROR: Build directory not found: $BUILD_DIR${NC}"
    echo "Run cmake build first:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build . --config Release"
    exit 1
fi

# Detect platform and locate plugin
PLATFORM=$(uname -s)
PLUGIN_FOUND=false

echo "Platform: $PLATFORM"
echo ""

if [ "$PLATFORM" = "Darwin" ]; then
    # macOS - test both VST3 and AU
    VST3_PATH="$BUILD_DIR/BTZ_artefacts/Release/VST3/BTZ.vst3"
    AU_PATH="$BUILD_DIR/BTZ_artefacts/Release/AU/BTZ.component"

    if [ -d "$VST3_PATH" ]; then
        echo -e "${GREEN}Found VST3:${NC} $VST3_PATH"
        PLUGIN_FOUND=true

        echo ""
        echo "========================================"
        echo "Testing VST3..."
        echo "========================================"
        echo ""

        pluginval --strictness-level 10 \
                  --validate-in-process \
                  --timeout-ms 30000 \
                  --verbose \
                  --output-dir "pluginval_reports" \
                  "$VST3_PATH"

        VST3_RESULT=$?
    fi

    if [ -d "$AU_PATH" ]; then
        echo ""
        echo -e "${GREEN}Found AU:${NC} $AU_PATH"
        PLUGIN_FOUND=true

        echo ""
        echo "========================================"
        echo "Testing AU..."
        echo "========================================"
        echo ""

        pluginval --strictness-level 10 \
                  --validate-in-process \
                  --timeout-ms 30000 \
                  --verbose \
                  --output-dir "pluginval_reports" \
                  "$AU_PATH"

        AU_RESULT=$?
    fi

elif [ "$PLATFORM" = "Linux" ]; then
    # Linux - VST3 only
    VST3_PATH="$BUILD_DIR/BTZ_artefacts/Release/VST3/BTZ.vst3"

    if [ -d "$VST3_PATH" ]; then
        echo -e "${GREEN}Found VST3:${NC} $VST3_PATH"
        PLUGIN_FOUND=true

        echo ""
        echo "========================================"
        echo "Testing VST3..."
        echo "========================================"
        echo ""

        pluginval --strictness-level 10 \
                  --validate-in-process \
                  --timeout-ms 30000 \
                  --verbose \
                  --output-dir "pluginval_reports" \
                  "$VST3_PATH"

        VST3_RESULT=$?
    fi
else
    echo -e "${YELLOW}WARNING: Windows not yet supported by this script${NC}"
    echo "Manually run: pluginval.exe --strictness-level 10 <path-to-BTZ.vst3>"
    exit 0
fi

if [ "$PLUGIN_FOUND" = false ]; then
    echo -e "${RED}ERROR: No plugin binaries found!${NC}"
    echo ""
    echo "Expected locations:"
    echo "  VST3: $BUILD_DIR/BTZ_artefacts/Release/VST3/BTZ.vst3"
    echo "  AU:   $BUILD_DIR/BTZ_artefacts/Release/AU/BTZ.component"
    echo ""
    echo "Build the plugin first:"
    echo "  cd build && cmake --build . --config Release"
    exit 1
fi

# Summary
echo ""
echo "========================================"
echo "VALIDATION SUMMARY"
echo "========================================"

EXIT_CODE=0

if [ -n "$VST3_RESULT" ]; then
    if [ $VST3_RESULT -eq 0 ]; then
        echo -e "VST3: ${GREEN}PASS ✓${NC}"
    else
        echo -e "VST3: ${RED}FAIL ✗${NC} (exit code: $VST3_RESULT)"
        EXIT_CODE=1
    fi
fi

if [ -n "$AU_RESULT" ]; then
    if [ $AU_RESULT -eq 0 ]; then
        echo -e "AU:   ${GREEN}PASS ✓${NC}"
    else
        echo -e "AU:   ${RED}FAIL ✗${NC} (exit code: $AU_RESULT)"
        EXIT_CODE=1
    fi
fi

echo ""

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ All validation tests PASSED${NC}"
    echo ""
    echo "Reports saved to: pluginval_reports/"
else
    echo -e "${RED}✗ Validation FAILED - see errors above${NC}"
    echo ""
    echo "Common issues:"
    echo "  - RT violations: Check for allocations/locks in processBlock"
    echo "  - State corruption: Verify getStateInformation/setStateInformation"
    echo "  - Bus layout: Check isBusesLayoutSupported"
    echo "  - Parameters: Verify all parameter IDs are unique and stable"
    echo ""
    echo "Full reports in: pluginval_reports/"
fi

exit $EXIT_CODE
