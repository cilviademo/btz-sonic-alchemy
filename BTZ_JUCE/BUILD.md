# BTZ Build Instructions

## Prerequisites

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# No additional dependencies needed (JUCE will auto-download)
```

### Windows
```powershell
# Visual Studio 2019 or newer with:
# - C++ desktop development workload
# - CMake tools for Windows

# Or use standalone CMake 3.15+
```

### Linux
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libasound2-dev \
    libfreetype6-dev \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libgl1-mesa-dev \
    libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev

# Fedora/RHEL
sudo dnf install -y \
    gcc-c++ \
    cmake \
    alsa-lib-devel \
    freetype-devel \
    libX11-devel \
    libXext-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    mesa-libGL-devel \
    libcurl-devel \
    webkit2gtk3-devel
```

## Build Commands

### Standard Release Build (All Platforms)

```bash
cd BTZ_JUCE
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### macOS Universal Binary (x86_64 + arm64)

```bash
cd BTZ_JUCE
mkdir build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build . --config Release
```

### Windows (Visual Studio)

```powershell
cd BTZ_JUCE
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### Debug Build (All Platforms)

```bash
cd BTZ_JUCE
mkdir build-debug
cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

## Output Locations

After successful build, plugin binaries will be copied to:

- **macOS**:
  - VST3: `~/Library/Audio/Plug-Ins/VST3/BTZ.vst3`
  - AU: `~/Library/Audio/Plug-Ins/Components/BTZ.component`
  - Standalone: `build/BTZ_artefacts/Release/Standalone/BTZ.app`

- **Windows**:
  - VST3: `C:\Program Files\Common Files\VST3\BTZ.vst3`
  - Standalone: `build\BTZ_artefacts\Release\Standalone\BTZ.exe`

- **Linux**:
  - VST3: `~/.vst3/BTZ.vst3`
  - Standalone: `build/BTZ_artefacts/Release/Standalone/BTZ`

## Known Compilation Issues (As of Phase 2)

### AdvancedTransientShaper.cpp (EXCLUDED)
**Status**: Commented out in CMakeLists.txt (line ~49)

**Issue**: TPTOnePole API mismatch
```cpp
// Error: TPTOnePole::processSample() expects 2 arguments, receives 1
envelopeFollower.processSample(rectified);  // FAILS
```

**Fix Required**: Update to correct JUCE 7.0.12 juce_dsp API:
```cpp
float envelopeFollower.processSample(0, rectified);  // channel, sample
```

### WDFSaturation.cpp (EXCLUDED)
**Status**: Commented out in CMakeLists.txt (line ~50)

**Issue**: Array initialization in WDF models
```cpp
// Error: Cannot initialize arrays in this context
// Likely std::array or juce::Array initialization issue
```

**Fix Required**: Review WDF (Wave Digital Filter) implementation and fix array initialization syntax for C++17.

## FL Studio Scan Safety Verification ✅

**Constructor** (`PluginProcessor.cpp:9-23`):
- ✅ Only lightweight initialization (APVTS, PresetManager reference)
- ✅ No DSP allocation
- ✅ No file I/O or network calls

**prepareToPlay** (`PluginProcessor.cpp:89-146`):
- ✅ ALL DSP modules prepared here (transientShaper, saturation, enhancedSpark, etc.)
- ✅ Denormal protection enabled
- ✅ Parameter smoothing initialization
- ✅ Component variance randomization

**Result**: Plugin is FL Studio scan-safe and follows JUCE best practices.

## Build Validation Checklist

After building, verify:

- [ ] Plugin scans successfully in FL Studio (Windows primary target)
- [ ] Plugin scans in Ableton Live, Reaper, Studio One, Bitwig
- [ ] macOS: AU validation passes (`auval -v aufx Btzp Btzz`)
- [ ] VST3: Steinberg pluginval passes (see `docs/QA_CHECKLIST.md`)
- [ ] No crashes on instantiation
- [ ] No crashes on preset loading
- [ ] Offline bounce is deterministic (same input → same output every time)

## Troubleshooting

### "JUCE submodule not found"
✅ **Expected behavior** - CMakeLists.txt will automatically download JUCE 7.0.12 via FetchContent.

### "X11 headers not found" (Linux)
Install X11 development libraries (see Prerequisites → Linux).

### "Visual Studio not found" (Windows)
Install Visual Studio 2019+ with C++ desktop development workload, or use CMake with Ninja:
```powershell
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Build succeeds but plugin doesn't load
1. Check DAW's plugin blacklist/preferences
2. Verify plugin binary is in correct location (see Output Locations)
3. Run pluginval for detailed error messages

---

**Last Updated**: Phase 2 (Build Reliability) - 2026-02-08
