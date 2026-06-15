#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# BTZ Sonic Alchemy — macOS build script (Xcode toolchain + CMake/Ninja).
# Usage:  scripts/build_macos.sh [Release|Debug] [--tests] [--clap] [--universal]
# Prereqs: Xcode + command line tools, cmake, ninja  (brew install cmake ninja)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

CONFIG="${1:-Release}"
BUILD_TESTS="OFF"; BUILD_CLAP="OFF"; ARCHS=""
for arg in "$@"; do
  [ "$arg" = "--tests" ]     && BUILD_TESTS="ON"
  [ "$arg" = "--clap" ]      && BUILD_CLAP="ON"
  [ "$arg" = "--universal" ] && ARCHS="-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BTZ_DIR="$(cd "$SCRIPT_DIR/../btz-sonic-alchemy-main/BTZ" && pwd)"
echo ">> BTZ project: $BTZ_DIR  Config: $CONFIG  Tests: $BUILD_TESTS  Universal: ${ARCHS:-no}"

cd "$BTZ_DIR"
# shellcheck disable=SC2086
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE="$CONFIG" \
  -DBTZ_BUILD_TESTS="$BUILD_TESTS" \
  -DBTZ_BUILD_CLAP="$BUILD_CLAP" \
  $ARCHS

TARGETS="BTZ_VST3 BTZ_AU BTZ_Standalone"
[ "$BUILD_TESTS" = "ON" ] && TARGETS="$TARGETS BTZTests"
# shellcheck disable=SC2086
cmake --build build --config "$CONFIG" --target $TARGETS

echo ">> Artefacts under: $BTZ_DIR/build/BTZ_artefacts/$CONFIG/ (VST3, AU, Standalone)"
[ "$BUILD_TESTS" = "ON" ] && ctest --test-dir build --output-on-failure || true
# AU validation (Apple): auval -v aufx Btz1 BTZa   (after the .component is installed)
echo ">> Done."
