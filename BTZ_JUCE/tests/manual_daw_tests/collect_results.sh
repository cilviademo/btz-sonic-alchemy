#!/bin/bash

#==============================================================================
# BTZ Manual DAW Test Results Collector
#
# Purpose: Ship Gate #2 - Collect all DAW testing artifacts into bundle
#
# Usage: ./collect_results.sh
# Output: BTZ_DAW_Test_Results_YYYYMMDD.zip
#==============================================================================

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "BTZ DAW Test Results Collector"
echo "========================================"

RESULTS_DIR="results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUTPUT_ZIP="BTZ_DAW_Test_Results_${TIMESTAMP}.zip"

# Check if results directory exists
if [ ! -d "$RESULTS_DIR" ]; then
    echo -e "${YELLOW}Warning: results/ directory not found${NC}"
    echo "Creating results directory..."
    mkdir -p "$RESULTS_DIR"
    echo ""
    echo "Please place your DAW test results in:"
    echo "  - $RESULTS_DIR/reaper_results.md"
    echo "  - $RESULTS_DIR/ableton_results.md"
    echo "  - $RESULTS_DIR/flstudio_results.md"
    echo "  - $RESULTS_DIR/*.wav (test audio recordings)"
    echo "  - $RESULTS_DIR/screenshots/*.png"
    exit 1
fi

# Count files
WAV_COUNT=$(find "$RESULTS_DIR" -name "*.wav" 2>/dev/null | wc -l)
MD_COUNT=$(find "$RESULTS_DIR" -name "*.md" 2>/dev/null | wc -l)
PNG_COUNT=$(find "$RESULTS_DIR" -name "*.png" 2>/dev/null | wc -l)

echo "Found in $RESULTS_DIR/:"
echo "  - Markdown reports: $MD_COUNT"
echo "  - Audio files (.wav): $WAV_COUNT"
echo "  - Screenshots (.png): $PNG_COUNT"
echo ""

if [ "$MD_COUNT" -lt 3 ]; then
    echo -e "${YELLOW}Warning: Expected 3 DAW result files (Reaper, Ableton, FL Studio)${NC}"
    echo "Found only $MD_COUNT markdown files"
fi

# Create zip archive
echo "Creating archive: $OUTPUT_ZIP"
zip -r "$OUTPUT_ZIP" "$RESULTS_DIR"/ -x "*.DS_Store"

if [ -f "$OUTPUT_ZIP" ]; then
    SIZE=$(du -h "$OUTPUT_ZIP" | cut -f1)
    echo ""
    echo -e "${GREEN}✅ Results collected successfully${NC}"
    echo ""
    echo "Archive: $OUTPUT_ZIP ($SIZE)"
    echo ""
    echo "Next steps:"
    echo "1. Upload $OUTPUT_ZIP for review"
    echo "2. Verify all 12 tests passed in all 3 DAWs"
    echo "3. Check for 0 crashes, 0 critical bugs"
    echo "4. Update Ship Gate #2 status"
else
    echo "ERROR: Failed to create archive"
    exit 1
fi
