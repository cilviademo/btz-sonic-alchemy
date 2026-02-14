# BTZ Build Instructions

## Windows x64 Build (Visual Studio 2022)

### Prerequisites

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Install "Desktop development with C++" workload
   - Ensure C++17 support is enabled

2. **CMake 3.15+**
   - Download from https://cmake.org/download/
   - Add to PATH during installation

3. **Git** (for JUCE submodule)
   - Download from https://git-scm.com/

### Build Steps

```powershell
# 1. Clone repository
git clone https://github.com/cilviademo/btz-sonic-alchemy.git
cd btz-sonic-alchemy/BTZ_JUCE

# 2. Initialize JUCE submodule (if not using FetchContent)
git submodule update --init --recursive

# 3. Configure CMake (Visual Studio 2022 x64)
cmake -B build -G "Visual Studio 17 2022" -A x64

# 4. Build Release
cmake --build build --config Release -j8

# 5. Build Debug (for development)
cmake --build build --config Debug -j8
```

### Build Outputs

**VST3 Plugin:**
```
build/BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3/
└── Contents/
    └── x86_64-win/
        └── BTZ - The Box Tone Zone.vst3
```

**Standalone Application:**
```
build/BTZ_artefacts/Release/Standalone/BTZ - The Box Tone Zone.exe
```

### Installation

**VST3 (Manual):**
```powershell
# Copy to system VST3 directory
xcopy /E /I "build\BTZ_artefacts\Release\VST3\BTZ - The Box Tone Zone.vst3" ^
    "C:\Program Files\Common Files\VST3\BTZ - The Box Tone Zone.vst3"
```

**VST3 (Automatic - if COPY_PLUGIN_AFTER_BUILD=TRUE in CMakeLists.txt):**
- Plugin automatically copied to system directory after build

**Standalone:**
- Run `build\BTZ_artefacts\Release\Standalone\BTZ - The Box Tone Zone.exe` directly
- Or copy to desired location

### Troubleshooting

**JUCE Not Found:**
```powershell
# Ensure submodule is initialized
git submodule update --init --recursive

# Or let CMake auto-download (slower first time)
# CMakeLists.txt has FetchContent fallback
```

**Visual Studio Version Mismatch:**
```powershell
# Use correct generator for your VS version:
# VS 2019: cmake -B build -G "Visual Studio 16 2019" -A x64
# VS 2022: cmake -B build -G "Visual Studio 17 2022" -A x64
```

**Missing x64 Toolchain:**
- Open Visual Studio Installer
- Modify installation → Add "MSVC v143 - VS 2022 C++ x64/x86 build tools"

**CMake Cache Issues:**
```powershell
# Clean and reconfigure
rmdir /S /Q build
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## macOS Build (Xcode or Makefile)

### Prerequisites

1. **Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **CMake 3.15+**
   ```bash
   brew install cmake
   ```

### Build Steps

```bash
# 1. Clone and init submodules
cd BTZ_JUCE
git submodule update --init --recursive

# 2. Configure (Xcode)
cmake -B build -G Xcode

# 3. Build
cmake --build build --config Release -j8

# Alternative: Makefile generator
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

### Build Outputs

**VST3:** `build/BTZ_artefacts/Release/VST3/BTZ.vst3`
**AU:** `build/BTZ_artefacts/Release/AU/BTZ.component`
**Standalone:** `build/BTZ_artefacts/Release/Standalone/BTZ.app`

### Installation

```bash
# VST3
cp -R "build/BTZ_artefacts/Release/VST3/BTZ.vst3" \
    ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R "build/BTZ_artefacts/Release/AU/BTZ.component" \
    ~/Library/Audio/Plug-Ins/Components/

# Standalone
cp -R "build/BTZ_artefacts/Release/Standalone/BTZ.app" \
    /Applications/
```

## Linux Build

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
    libasound2-dev libfreetype6-dev libgl1-mesa-dev libcurl4-openssl-dev

# Fedora/RHEL
sudo dnf install -y cmake ninja-build gcc-c++ \
    libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel \
    alsa-lib-devel freetype-devel mesa-libGL-devel libcurl-devel
```

### Build Steps

```bash
cd BTZ_JUCE
git submodule update --init --recursive

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

### Build Outputs

**VST3:** `build/BTZ_artefacts/Release/VST3/BTZ.vst3/`
**Standalone:** `build/BTZ_artefacts/Release/Standalone/BTZ`

### Installation

```bash
# VST3
mkdir -p ~/.vst3
cp -R build/BTZ_artefacts/Release/VST3/BTZ.vst3 ~/.vst3/

# Standalone
sudo cp build/BTZ_artefacts/Release/Standalone/BTZ /usr/local/bin/
```

## Validation

### Run pluginval (Recommended)

```bash
# Download pluginval from: https://github.com/Tracktion/pluginval/releases

# Windows
pluginval.exe --strictness-level 10 --validate-in-process ^
    "C:\Program Files\Common Files\VST3\BTZ - The Box Tone Zone.vst3"

# macOS
pluginval --strictness-level 10 --validate-in-process \
    ~/Library/Audio/Plug-Ins/VST3/BTZ.vst3

# Linux
pluginval --strictness-level 10 --validate-in-process \
    ~/.vst3/BTZ.vst3
```

**Expected Output:** `Results: 0 plugins failed, 1 passed`

### Manual DAW Testing

1. **FL Studio** (Windows primary target)
   - Plugin → Manage plugins → Find more plugins
   - Verify BTZ appears in list
   - Load on mixer track
   - Test automation, save/load project

2. **Ableton Live**
   - Test with 32-sample buffer size
   - Verify no audio glitches

3. **Reaper**
   - Load 10+ instances
   - Check CPU usage stability

4. **Logic Pro** (macOS AU)
   - Run auval: `auval -v aufx Btzp Btzz`

## Build Configuration Options

### CMakeLists.txt Variables

```cmake
# Enable offline rendering tool
cmake -B build -DBTZ_BUILD_TOOLS=ON

# Copy plugin after build (requires admin on Windows)
# Controlled by COPY_PLUGIN_AFTER_BUILD in juce_add_plugin()

# Custom JUCE path
cmake -B build -DJUCE_DIR=/path/to/juce

# Custom install prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/custom/path
```

### Performance Build (Advanced)

```bash
# Enable LTO and native optimization
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
    -DCMAKE_CXX_FLAGS="-march=native -O3"
```

## Development Build

```bash
# Debug build with symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Enable sanitizers (GCC/Clang)
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"
```

## Continuous Integration

GitHub Actions workflows are configured in `.github/workflows/`:
- **build-windows.yml** - Windows x64 VST3 + Standalone
- **build-macos.yml** - macOS Universal VST3 + AU
- **build-linux.yml** - Linux x64 VST3

Builds run on push/PR and upload artifacts.

## Next Steps

- See `QuickStart.md` for end-user setup
- See `UserManual.md` for feature documentation
- See `Specs.md` for technical specifications
- See `Measurements.md` for validation procedures
