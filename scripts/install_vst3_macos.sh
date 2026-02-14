#!/usr/bin/env bash
# Copy BTZ.vst3 to system VST3 folder. May need sudo for /Library.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VST3_SRC="$REPO_ROOT/BTZ/build/VST3/BTZ.vst3"
if [[ ! -d "$VST3_SRC" ]]; then
    VST3_SRC="$REPO_ROOT/btz-sonic-alchemy-main/BTZ/build/BTZ_artefacts/Release/VST3/Box Tone Zone (BTZ).vst3"
fi
VST3_DEST="/Library/Audio/Plug-Ins/VST3"

if [[ ! -d "$VST3_SRC" ]]; then
    echo "Error: Build not found at $VST3_SRC"
    echo "Run scripts/build_macos_release.sh first."
    exit 1
fi

echo "Copying Box Tone Zone (BTZ).vst3 to $VST3_DEST"
sudo mkdir -p "$VST3_DEST"
sudo cp -R "$VST3_SRC" "$VST3_DEST/"

echo "Done. Rescan plugins in FL Studio (macOS)."
