#!/usr/bin/env bash
# Remove the CMake build directory (safe — never touches source).
# Usage: scripts/clean_build.sh
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BTZ_DIR="$(cd "$SCRIPT_DIR/../btz-sonic-alchemy-main/BTZ" && pwd)"
if [ -d "$BTZ_DIR/build" ]; then
  echo ">> Removing $BTZ_DIR/build"
  rm -rf "$BTZ_DIR/build"
  echo ">> Clean."
else
  echo ">> Nothing to clean ($BTZ_DIR/build does not exist)."
fi
