#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# BTZ Sonic Alchemy — Linux build script
# Verified working recipe (Ubuntu 24.04, GCC 13, CMake + Ninja, JUCE 8.0.6).
# Usage:  scripts/build_linux.sh [Release|Debug] [--tests] [--clap]
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

CONFIG="${1:-Release}"
BUILD_TESTS="OFF"
BUILD_CLAP="OFF"
for arg in "$@"; do
  [ "$arg" = "--tests" ] && BUILD_TESTS="ON"
  [ "$arg" = "--clap" ]  && BUILD_CLAP="ON"
done

# Locate the JUCE/CMake project (this repo nests it under btz-sonic-alchemy-main/BTZ).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BTZ_DIR="$(cd "$SCRIPT_DIR/../btz-sonic-alchemy-main/BTZ" && pwd)"
echo ">> BTZ project: $BTZ_DIR"
echo ">> Config: $CONFIG  Tests: $BUILD_TESTS  CLAP: $BUILD_CLAP"

# Required JUCE Linux dev dependencies. juceaide (JUCE's build helper) needs
# X11/GL headers even for a headless build; juce_gui_extra needs gtk unless
# JUCE_WEB_BROWSER=0 (the plugin sets this; tests set it via CMakeLists).
echo ">> If configure fails on missing headers, install:"
echo "   sudo apt-get install -y libx11-dev libxext-dev libxrandr-dev \\"
echo "       libxinerama-dev libxcursor-dev libxcomposite-dev libxrender-dev \\"
echo "       libfreetype6-dev libfontconfig1-dev libasound2-dev \\"
echo "       libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev"
echo "   (add libgtk-3-dev libwebkit2gtk-4.1-dev only if you enable the web browser)"

cd "$BTZ_DIR"
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE="$CONFIG" \
  -DBTZ_BUILD_TESTS="$BUILD_TESTS" \
  -DBTZ_BUILD_CLAP="$BUILD_CLAP"

# Build plugin formats (and tests if requested).
TARGETS="BTZ_VST3 BTZ_Standalone"
[ "$BUILD_TESTS" = "ON" ] && TARGETS="$TARGETS BTZTests"
# shellcheck disable=SC2086
cmake --build build --config "$CONFIG" --target $TARGETS

echo ">> Artefacts:"
find build/BTZ_artefacts -maxdepth 3 \( -name "*.vst3" -o -name "*.so" -o -type d -name "Standalone" \) 2>/dev/null || true
[ "$BUILD_TESTS" = "ON" ] && echo ">> Run tests: ./build/BTZTests_artefacts/$CONFIG/BTZTests"

# KNOWN LINUX QUIRK: the VST3 post-link step (juce_vst3_helper -> moduleinfo.json)
# can fail on headless containers with "/bin/sh: Syntax error". The .so itself
# still links correctly; this does not occur on macOS/Windows.
echo ">> Done."
