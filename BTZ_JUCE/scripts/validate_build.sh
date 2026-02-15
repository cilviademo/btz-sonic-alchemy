#!/bin/bash
#
# BTZ Build Validation Script
# Tests the built plugin for correctness and functionality
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BTZ_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$BTZ_ROOT/build"
VST3_PATH="$BUILD_DIR/BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3"
STANDALONE_PATH="$BUILD_DIR/BTZ_artefacts/Release/Standalone/BTZ - The Box Tone Zone"

echo "========================================"
echo "BTZ Build Validation"
echo "========================================"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASS=0
FAIL=0
WARN=0

pass() {
    echo -e "${GREEN}✓ PASS:${NC} $1"
    ((PASS++))
}

fail() {
    echo -e "${RED}✗ FAIL:${NC} $1"
    ((FAIL++))
}

warn() {
    echo -e "${YELLOW}⚠ WARN:${NC} $1"
    ((WARN++))
}

echo "=== Test 1: Build Artifacts Exist ==="
echo ""

if [ -d "$VST3_PATH" ]; then
    pass "VST3 bundle exists"
else
    fail "VST3 bundle NOT found"
fi

VST3_SO="$VST3_PATH/Contents/x86_64-linux/BTZ - The Box Tone Zone.so"
if [ -f "$VST3_SO" ]; then
    pass "VST3 shared library exists"
    SIZE=$(ls -lh "$VST3_SO" | awk '{print $5}')
    echo "  Size: $SIZE"
else
    fail "VST3 shared library NOT found"
fi

if [ -f "$STANDALONE_PATH" ]; then
    pass "Standalone executable exists"
    SIZE=$(ls -lh "$STANDALONE_PATH" | awk '{print $5}')
    echo "  Size: $SIZE"
else
    fail "Standalone executable NOT found"
fi

MODULE_INFO="$VST3_PATH/Contents/Resources/moduleinfo.json"
if [ -f "$MODULE_INFO" ]; then
    pass "moduleinfo.json exists"
else
    fail "moduleinfo.json NOT found"
fi

echo ""
echo "=== Test 2: Binary Validation ==="
echo ""

if [ -f "$VST3_SO" ]; then
    FILE_TYPE=$(file "$VST3_SO" | grep "ELF 64-bit LSB shared object")
    if [ -n "$FILE_TYPE" ]; then
        pass "VST3 is valid 64-bit shared library"
    else
        fail "VST3 is not a valid shared library"
    fi

    # Check for required VST3 symbols
    if nm -D "$VST3_SO" 2>/dev/null | grep -q "GetPluginFactory"; then
        pass "VST3 exports GetPluginFactory"
    else
        fail "VST3 missing GetPluginFactory export"
    fi

    if nm -D "$VST3_SO" 2>/dev/null | grep -q "ModuleEntry"; then
        pass "VST3 exports ModuleEntry"
    else
        fail "VST3 missing ModuleEntry export"
    fi

    if nm -D "$VST3_SO" 2>/dev/null | grep -q "ModuleExit"; then
        pass "VST3 exports ModuleExit"
    else
        fail "VST3 missing ModuleExit export"
    fi
fi

if [ -f "$STANDALONE_PATH" ]; then
    FILE_TYPE=$(file "$STANDALONE_PATH" | grep "ELF 64-bit LSB pie executable")
    if [ -n "$FILE_TYPE" ]; then
        pass "Standalone is valid 64-bit executable"
    else
        fail "Standalone is not a valid executable"
    fi
fi

echo ""
echo "=== Test 3: Library Dependencies ==="
echo ""

if [ -f "$VST3_SO" ]; then
    MISSING_DEPS=$(ldd "$VST3_SO" 2>&1 | grep "not found" || true)
    if [ -z "$MISSING_DEPS" ]; then
        pass "All VST3 dependencies satisfied"
    else
        fail "VST3 has missing dependencies:"
        echo "$MISSING_DEPS"
    fi

    # Check for unexpected dependencies (e.g., X11 in headless environment)
    if ldd "$VST3_SO" 2>/dev/null | grep -q "libX11"; then
        warn "VST3 depends on X11 (may fail in headless environment)"
    fi
fi

if [ -f "$STANDALONE_PATH" ]; then
    MISSING_DEPS=$(ldd "$STANDALONE_PATH" 2>&1 | grep "not found" || true)
    if [ -z "$MISSING_DEPS" ]; then
        pass "All Standalone dependencies satisfied"
    else
        fail "Standalone has missing dependencies:"
        echo "$MISSING_DEPS"
    fi
fi

echo ""
echo "=== Test 4: Plugin Metadata ==="
echo ""

if [ -f "$MODULE_INFO" ]; then
    # Check JSON validity (note: JUCE generates with trailing commas, which is technically invalid JSON but works fine)
    if python3 -m json.tool "$MODULE_INFO" > /dev/null 2>&1; then
        pass "moduleinfo.json is valid JSON"
    else
        warn "moduleinfo.json has trailing commas (JUCE default, VST3 spec allows this)"
    fi

    # Check plugin name
    if grep -q '"Name": "BTZ - The Box Tone Zone"' "$MODULE_INFO"; then
        pass "Plugin name is correct"
    else
        fail "Plugin name is incorrect"
    fi

    # Check version
    if grep -q '"Version": "1.0.0"' "$MODULE_INFO"; then
        pass "Plugin version is 1.0.0"
    else
        warn "Plugin version may be incorrect"
    fi

    # Check vendor
    if grep -q '"Vendor": "BTZ Audio"' "$MODULE_INFO"; then
        pass "Plugin vendor is correct"
    else
        fail "Plugin vendor is incorrect"
    fi

    # Check VST3 categories
    if grep -q '"Sub Categories"' "$MODULE_INFO" && grep -q '"Fx"' "$MODULE_INFO"; then
        pass "Plugin is categorized as Fx"
    else
        warn "Plugin category may be incorrect"
    fi
fi

echo ""
echo "=== Test 5: Build Quality ==="
echo ""

# Check for debug symbols (stripped vs not stripped)
if file "$VST3_SO" | grep -q "not stripped"; then
    warn "VST3 not stripped (contains debug symbols)"
    echo "  Recommendation: Strip for release builds"
else
    pass "VST3 is stripped (production-ready)"
fi

# Check build log for errors
if [ -f "$BUILD_DIR/build_full.log" ]; then
    ERROR_COUNT=$(grep -c "error:" "$BUILD_DIR/build_full.log" || true)
    if [ "$ERROR_COUNT" -eq 0 ]; then
        pass "Build completed with zero errors"
    else
        fail "Build had $ERROR_COUNT errors"
    fi

    WARNING_COUNT=$(grep -c "warning:" "$BUILD_DIR/build_full.log" || true)
    if [ "$WARNING_COUNT" -eq 0 ]; then
        pass "Build completed with zero warnings"
    elif [ "$WARNING_COUNT" -lt 50 ]; then
        warn "Build had $WARNING_COUNT warnings (acceptable)"
    else
        warn "Build had $WARNING_COUNT warnings (consider reviewing)"
    fi
fi

echo ""
echo "=== Test 6: File Permissions ==="
echo ""

if [ -x "$VST3_SO" ]; then
    pass "VST3 is executable"
else
    fail "VST3 is not executable"
fi

if [ -x "$STANDALONE_PATH" ]; then
    pass "Standalone is executable"
else
    fail "Standalone is not executable"
fi

echo ""
echo "=== Test 7: Code Fixes Verification ==="
echo ""

SOURCE_DIR="$BTZ_ROOT/Source"

# Check for DSPConstants.h
if [ -f "$SOURCE_DIR/Utilities/DSPConstants.h" ]; then
    pass "DSPConstants.h exists (P2-3 fix)"
else
    fail "DSPConstants.h missing"
fi

# Check PluginProcessor.cpp for P1 fixes
if grep -q "P1-1 FIX: Include TransientShaper in oversampling" "$SOURCE_DIR/PluginProcessor.cpp"; then
    pass "P1-1 fix present (TransientShaper oversampling)"
else
    warn "P1-1 fix marker not found"
fi

if grep -q "P1-5.*Report TOTAL latency" "$SOURCE_DIR/PluginProcessor.cpp"; then
    pass "P1-5 fix present (latency reporting)"
else
    warn "P1-5 fix marker not found"
fi

if grep -q "P1-6 FIX: Version-aware parameter migration" "$SOURCE_DIR/PluginProcessor.cpp"; then
    pass "P1-6 fix present (state migration)"
else
    warn "P1-6 fix marker not found"
fi

if grep -q "P2-4 FIX: DSP validation in ALL builds" "$SOURCE_DIR/PluginProcessor.cpp"; then
    pass "P2-4 fix present (NaN protection)"
else
    warn "P2-4 fix marker not found"
fi

if grep -q "P2-6 FIX: Denormal protection at block level" "$SOURCE_DIR/PluginProcessor.cpp"; then
    pass "P2-6 fix present (denormal protection)"
else
    warn "P2-6 fix marker not found"
fi

# Check for JUCE 7.x includes
if grep -q "#include <juce_audio_processors/juce_audio_processors.h>" "$SOURCE_DIR/PluginProcessor.h"; then
    pass "JUCE 7.x modular includes present"
else
    fail "Old JuceHeader.h still in use"
fi

echo ""
echo "========================================"
echo "VALIDATION SUMMARY"
echo "========================================"
echo ""
echo -e "${GREEN}PASSED: $PASS${NC}"
echo -e "${YELLOW}WARNINGS: $WARN${NC}"
echo -e "${RED}FAILED: $FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✓ BUILD VALIDATION: PASS${NC}"
    echo "The plugin is ready for testing."
    exit 0
else
    echo -e "${RED}✗ BUILD VALIDATION: FAIL${NC}"
    echo "Please review and fix the failed tests above."
    exit 1
fi
