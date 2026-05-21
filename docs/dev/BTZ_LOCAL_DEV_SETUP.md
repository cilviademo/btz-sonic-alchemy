# BTZ — Local Dev Environment Setup

The CMake/JUCE project lives at **`btz-sonic-alchemy-main/BTZ/`**. JUCE 8.0.6 is
fetched automatically via CMake `FetchContent` (no manual JUCE install needed),
unless you set `JUCE_DIR`/`JUCE_ROOT` to a local copy.

Plugin formats: **VST3, AU (macOS), Standalone**. CLAP is optional
(`-DBTZ_BUILD_CLAP=ON`, fetches `clap-juce-extensions`). Tests are optional
(`-DBTZ_BUILD_TESTS=ON`, fetches GoogleTest 1.14).

---

## Windows (preferred for VST3 + Standalone)

Install:
- **Visual Studio 2022 Community** with the **"Desktop development with C++"** workload
- **CMake** (3.22+) — bundled with VS, or standalone
- **Ninja** (optional; the VS generator also works)
- **Git**
- **Python 3** (only if you script packaging)
- **pluginval** — https://github.com/Tracktion/pluginval/releases
- **REAPER** (cheap, scriptable host for testing) and optionally **PluginDoctor**
- Optional: **LLVM/Clang** tools for clang-tidy

Build:
```powershell
pwsh scripts/build_windows.ps1 -Config Release -Tests
# artefacts: btz-sonic-alchemy-main/BTZ/build/BTZ_artefacts/Release/{VST3,Standalone}
```

## macOS (required for AU + notarization)

Install:
- **Xcode** + Command Line Tools (`xcode-select --install`)
- `brew install cmake ninja git`
- **pluginval** (macOS build) and **REAPER**
- For release: an Apple **Developer ID Application** + **Installer** certificate

Build:
```bash
scripts/build_macos.sh Release --tests            # add --universal for arm64+x86_64
auval -v aufx Btz1 BTZa                            # validate AU after install
```

## Linux (validation / CI; no AU)

Install toolchain + JUCE deps (Ubuntu/Debian):
```bash
sudo apt-get install -y \
  build-essential cmake ninja-build git \
  libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libxcomposite-dev libxrender-dev \
  libfreetype6-dev libfontconfig1-dev libasound2-dev \
  libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev
# Only if you enable the JUCE web browser (BTZ does NOT — JUCE_WEB_BROWSER=0):
#   libgtk-3-dev libwebkit2gtk-4.1-dev
```
Build:
```bash
scripts/build_linux.sh Release --tests
```

**Verified note (Linux):** without the X11/GL headers above, CMake configure
fails while building JUCE's `juceaide` helper (`X11/extensions/Xrandr.h not
found`). With them, the plugin and tests build cleanly. The VST3
`moduleinfo.json` post-link step may fail on headless containers — harmless;
the `.so` still links and macOS/Windows are unaffected.

---

## Apple Silicon (ARM64) note

`BTZDsp.h` uses SSE intrinsics **only** under `#ifdef __SSE__` (FTZ/DAZ denormal
flushing); on arm64 those blocks are skipped and JUCE's
`FloatVectorOperations::disableDenormalisedNumberSupport()` handles denormals.
So the ARM build does not require NEON shims for the current code. Build a
universal binary with `scripts/build_macos.sh Release --universal`.

## Useful CMake flags
- `-DBTZ_BUILD_TESTS=ON` — build + register GoogleTest suite (ctest)
- `-DBTZ_BUILD_CLAP=ON` — also build CLAP (fetches clap-juce-extensions; currently pinned to `main` — see CI notes)
- `-DJUCE_DIR=/path/to/JUCE` — use a local JUCE instead of fetching
- `-DPLUGINVAL_PATH=/path/to/pluginval` — enables `validate_vst3` / `validate_au` CMake targets
