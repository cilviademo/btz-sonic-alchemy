#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# BTZ Sonic Alchemy — pluginval runner (macOS/Linux).
# Usage:  scripts/run_pluginval.sh [strictness] [path-to-vst3]
#   strictness defaults to 10; path defaults to the built VST3 in build/.
# Download pluginval: https://github.com/Tracktion/pluginval/releases
# Set PLUGINVAL env var to the executable, or place it on PATH.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

STRICT="${1:-10}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BTZ_DIR="$(cd "$SCRIPT_DIR/../btz-sonic-alchemy-main/BTZ" && pwd)"

PLUGIN="${2:-}"
if [ -z "$PLUGIN" ]; then
  PLUGIN="$(find "$BTZ_DIR/build/BTZ_artefacts" -name "*.vst3" -maxdepth 4 2>/dev/null | head -1)"
fi
[ -z "$PLUGIN" ] && { echo "No VST3 found. Build first (scripts/build_*.sh)."; exit 1; }

PV="${PLUGINVAL:-pluginval}"
command -v "$PV" >/dev/null 2>&1 || { echo "pluginval not found. Set PLUGINVAL=/path/to/pluginval"; exit 1; }

echo ">> Validating: $PLUGIN  (strictness $STRICT)"
"$PV" --validate "$PLUGIN" \
      --strictness-level "$STRICT" \
      --timeout-ms 120000 \
      --repeat-count 2 \
      --randomise \
      --validate-in-process
echo ">> pluginval passed at strictness $STRICT"
