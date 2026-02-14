#!/usr/bin/env bash
# Build BTZ VST3 (Release) from repo root.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BTZ="$REPO_ROOT/BTZ"
BUILD_DIR="$BTZ/build"

if [[ ! -f "$BTZ/CMakeLists.txt" ]]; then
    echo "Error: BTZ folder not found at $BTZ"
    exit 1
fi

echo "Building BTZ plugin in $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$BTZ" -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 8

echo ""
echo "Build done. VST3: $BUILD_DIR/VST3/BTZ.vst3"
