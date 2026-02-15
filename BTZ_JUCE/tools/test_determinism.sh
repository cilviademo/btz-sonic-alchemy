#!/bin/bash
# BTZ Offline Bounce Determinism Test
#
# Purpose: Verify that BTZ produces identical output for identical input
# across multiple render passes (required for professional workflows)
#
# Requirements:
#   - offline_render tool built (cmake -DBTZ_BUILD_TOOLS=ON)
#   - Test audio file (pink noise recommended)
#
# Usage:
#   ./test_determinism.sh [input.wav] [num_bounces]
#
# Example:
#   ./test_determinism.sh test_audio.wav 5

set -e  # Exit on error

# Configuration
INPUT_FILE="${1:-test_input.wav}"
NUM_BOUNCES="${2:-5}"
OUTPUT_DIR="determinism_test_outputs"
OFFLINE_RENDER="../build/tools/offline_render"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "==================================================================="
echo "BTZ Offline Bounce Determinism Test"
echo "==================================================================="
echo ""
echo "Configuration:"
echo "  Input file: $INPUT_FILE"
echo "  Number of bounces: $NUM_BOUNCES"
echo "  Output directory: $OUTPUT_DIR"
echo ""

# Check if offline_render tool exists
if [ ! -f "$OFFLINE_RENDER" ]; then
    echo -e "${RED}❌ ERROR: offline_render tool not found${NC}"
    echo "Build with: cmake -DBTZ_BUILD_TOOLS=ON"
    exit 1
fi

# Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo -e "${RED}❌ ERROR: Input file not found: $INPUT_FILE${NC}"
    echo ""
    echo "To create a test file with sox:"
    echo "  sox -n -r 48000 -b 24 test_input.wav synth 10 pinknoise vol -18dB"
    echo ""
    echo "Or export 10 seconds of pink noise from your DAW at -18 dBFS RMS"
    exit 1
fi

# Clean output directory
if [ -d "$OUTPUT_DIR" ]; then
    echo "Cleaning previous test outputs..."
    rm -rf "$OUTPUT_DIR"
fi
mkdir -p "$OUTPUT_DIR"

echo -e "${GREEN}✅ Pre-flight checks passed${NC}"
echo ""

# Render multiple bounces
echo "==================================================================="
echo "Rendering $NUM_BOUNCES consecutive bounces..."
echo "==================================================================="

START_TIME=$(date +%s)

for i in $(seq 1 $NUM_BOUNCES); do
    echo ""
    echo "--- Bounce $i/$NUM_BOUNCES ---"

    $OFFLINE_RENDER \
        "$INPUT_FILE" \
        "$OUTPUT_DIR/bounce_$i.wav" \
        > "$OUTPUT_DIR/bounce_${i}_log.txt" 2>&1

    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ FAIL: Render $i failed${NC}"
        cat "$OUTPUT_DIR/bounce_${i}_log.txt"
        exit 1
    fi

    echo "  ✓ Bounce $i complete"
done

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo -e "${GREEN}✅ All $NUM_BOUNCES bounces rendered successfully${NC}"
echo "Total time: ${ELAPSED}s"
echo ""

# Compute MD5 hashes
echo "==================================================================="
echo "Computing MD5 hashes..."
echo "==================================================================="
echo ""

# Store hashes in array
declare -a HASHES
for i in $(seq 1 $NUM_BOUNCES); do
    HASH=$(md5sum "$OUTPUT_DIR/bounce_$i.wav" | awk '{print $1}')
    HASHES[$i]=$HASH
    echo "Bounce $i: $HASH"
done

echo ""

# Check if all hashes match
REFERENCE_HASH="${HASHES[1]}"
ALL_MATCH=true

for i in $(seq 2 $NUM_BOUNCES); do
    if [ "${HASHES[$i]}" != "$REFERENCE_HASH" ]; then
        ALL_MATCH=false
        break
    fi
done

echo "==================================================================="
echo "DETERMINISM TEST RESULT"
echo "==================================================================="
echo ""

if [ "$ALL_MATCH" = true ]; then
    echo -e "${GREEN}✅ PASS: All $NUM_BOUNCES bounces are IDENTICAL${NC}"
    echo ""
    echo "Reference MD5: $REFERENCE_HASH"
    echo ""
    echo "BTZ produces deterministic output. This is REQUIRED for:"
    echo "  • Professional mixing/mastering workflows"
    echo "  • Collaborative projects"
    echo "  • Bounce/export consistency"
    echo "  • Regression testing"
    echo ""
    exit 0
else
    echo -e "${RED}❌ FAIL: Bounces DIFFER (non-deterministic processing)${NC}"
    echo ""
    echo "Hash comparison:"
    for i in $(seq 1 $NUM_BOUNCES); do
        if [ "${HASHES[$i]}" = "$REFERENCE_HASH" ]; then
            echo "  Bounce $i: ${HASHES[$i]} ✓"
        else
            echo "  Bounce $i: ${HASHES[$i]} ✗ (MISMATCH)"
        fi
    done
    echo ""
    echo -e "${YELLOW}Possible causes:${NC}"
    echo "  • Random number generators without fixed seed"
    echo "  • Uninitialized memory reads"
    echo "  • Non-deterministic algorithms (std::unordered_map iteration, etc.)"
    echo "  • System clock dependencies in DSP code"
    echo "  • Thread timing issues"
    echo "  • Floating-point non-associativity (rare)"
    echo ""
    echo "Debug steps:"
    echo "  1. Check for rand(), srand(), std::random_device in DSP code"
    echo "  2. Check for uninitialized variables (use ASAN/valgrind)"
    echo "  3. Check for std::unordered_map, std::unordered_set in hot path"
    echo "  4. Check for std::chrono or time() calls in DSP"
    echo "  5. Enable deterministic mode in JUCE (if available)"
    echo ""
    exit 1
fi
