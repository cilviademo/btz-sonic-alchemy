#!/bin/bash

#==============================================================================
# BTZ Support Bundle Collector
#
# Purpose: Collect diagnostic information for bug reports
#
# Usage: ./collect_support_bundle.sh
# Output: BTZ_Support_Bundle_YYYYMMDD.zip
#==============================================================================

set -e

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BUNDLE_DIR="BTZ_Support_Bundle_${TIMESTAMP}"
OUTPUT_ZIP="${BUNDLE_DIR}.zip"

echo "========================================"
echo "BTZ Support Bundle Collector"
echo "========================================"
echo ""

mkdir -p "$BUNDLE_DIR"

# System Information
echo "Collecting system information..."
cat > "$BUNDLE_DIR/system_info.txt" <<EOF
BTZ Support Bundle
Generated: $(date)

=== SYSTEM ===
OS: $(uname -s)
Version: $(uname -r)
Architecture: $(uname -m)

=== CPU ===
$(sysctl -n machdep.cpu.brand_string 2>/dev/null || grep "model name" /proc/cpuinfo 2>/dev/null | head -1 || echo "Unknown")

=== MEMORY ===
$(free -h 2>/dev/null || vm_stat 2>/dev/null || echo "Unknown")

=== AUDIO DRIVERS ===
EOF

# macOS specific
if [[ "$(uname)" == "Darwin" ]]; then
    system_profiler SPAudioDataType >> "$BUNDLE_DIR/system_info.txt" 2>/dev/null || true
fi

# Linux specific
if [[ "$(uname)" == "Linux" ]]; then
    aplay -l >> "$BUNDLE_DIR/system_info.txt" 2>/dev/null || true
fi

# Plugin Information
echo "Collecting plugin information..."
cat > "$BUNDLE_DIR/plugin_info.txt" <<EOF
=== BTZ PLUGIN ===
Version: 1.0.0
Build Date: $(date -r BTZ_JUCE/build/BTZ_artefacts/Release/VST3/*.vst3 2>/dev/null || echo "Not built")

=== INSTALLED FORMATS ===
EOF

# Check for installed plugins
if [[ "$(uname)" == "Darwin" ]]; then
    echo "VST3:" >> "$BUNDLE_DIR/plugin_info.txt"
    ls -lh ~/Library/Audio/Plug-Ins/VST3/*BTZ*.vst3 2>/dev/null >> "$BUNDLE_DIR/plugin_info.txt" || echo "Not installed" >> "$BUNDLE_DIR/plugin_info.txt"
    echo "" >> "$BUNDLE_DIR/plugin_info.txt"
    echo "AU:" >> "$BUNDLE_DIR/plugin_info.txt"
    ls -lh ~/Library/Audio/Plug-Ins/Components/*BTZ*.component 2>/dev/null >> "$BUNDLE_DIR/plugin_info.txt" || echo "Not installed" >> "$BUNDLE_DIR/plugin_info.txt"
elif [[ "$(uname)" == "Linux" ]]; then
    echo "VST3:" >> "$BUNDLE_DIR/plugin_info.txt"
    ls -lh ~/.vst3/*BTZ*.vst3 2>/dev/null >> "$BUNDLE_DIR/plugin_info.txt" || echo "Not installed" >> "$BUNDLE_DIR/plugin_info.txt"
fi

# Collect logs (if any)
echo "Collecting logs..."
if [ -d ~/Library/Logs/BTZ ] || [ -d ~/.config/BTZ/logs ]; then
    cp -r ~/Library/Logs/BTZ "$BUNDLE_DIR/" 2>/dev/null || \
    cp -r ~/.config/BTZ/logs "$BUNDLE_DIR/" 2>/dev/null || \
    echo "No logs found" > "$BUNDLE_DIR/logs.txt"
else
    echo "No logs found" > "$BUNDLE_DIR/logs.txt"
fi

# Crash reports (macOS)
if [[ "$(uname)" == "Darwin" ]]; then
    echo "Collecting crash reports..."
    find ~/Library/Logs/DiagnosticReports -name "*BTZ*" -mtime -7 -exec cp {} "$BUNDLE_DIR/" \; 2>/dev/null || \
    echo "No crash reports found" > "$BUNDLE_DIR/crash_reports.txt"
fi

# Create README
cat > "$BUNDLE_DIR/README.txt" <<EOF
BTZ Support Bundle
==================

This bundle contains diagnostic information to help resolve your issue.

Contents:
- system_info.txt: System and audio hardware information
- plugin_info.txt: Installed plugin versions and locations
- logs.txt: Application logs (if available)
- crash_reports: Recent crash reports (macOS only)

Please attach this bundle to your bug report or support ticket.

DO NOT share this bundle publicly - it may contain system-specific information.

Generated: $(date)
EOF

# Create zip
echo "Creating archive..."
zip -r "$OUTPUT_ZIP" "$BUNDLE_DIR/" > /dev/null

# Cleanup
rm -rf "$BUNDLE_DIR"

if [ -f "$OUTPUT_ZIP" ]; then
    SIZE=$(du -h "$OUTPUT_ZIP" | cut -f1)
    echo ""
    echo "✅ Support bundle created successfully"
    echo ""
    echo "File: $OUTPUT_ZIP ($SIZE)"
    echo ""
    echo "Next steps:"
    echo "1. Attach $OUTPUT_ZIP to your bug report"
    echo "2. Describe the issue you're experiencing"
    echo "3. Include steps to reproduce if possible"
    echo ""
    echo "Submit at: https://github.com/cilviademo/btz-sonic-alchemy/issues"
else
    echo "ERROR: Failed to create support bundle"
    exit 1
fi
