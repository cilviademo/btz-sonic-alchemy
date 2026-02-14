# VST Development & Installation (BTZ)

## 1. Project type

- **CMake workflow** — source of truth is `BTZ/CMakeLists.txt`.
- No `.jucer` (Projucer) in this repo.

## 2. Prerequisites (one-time)

### Windows

- **Visual Studio Build Tools 2022** (or VS 2022) with **Desktop development with C++**.
- **CMake** (3.22+) on PATH.
- **Ninja** (optional, recommended): `winget install Ninja-build.Ninja` or [ninja-build](https://github.com/ninja-build/ninja/releases).
- **JUCE**: either
  - Install JUCE (e.g. via CMake) and set `JUCE_DIR` to the directory containing `JUCEConfig.cmake`, or
  - Add JUCE as a submodule and in the **repo root** CMake use `add_subdirectory(JUCE)` and build BTZ from a root CMake that includes `BTZ`.

### macOS

- **Xcode** or **Xcode Command Line Tools**.
- **CMake** (e.g. `brew install cmake`).
- **JUCE** (path or submodule, same idea as above).

## 3. Build from repo root

### Windows (Cursor terminal)

```bat
cd BTZ
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 8
```

Or use the helper script (run from repo root):

```bat
scripts\build_windows_release.bat
```

### macOS

```bash
cd BTZ
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 8
```

Or:

```bash
./scripts/build_macos_release.sh
```

### Where is the VST3?

With `juce_add_plugin`, the VST3 is produced under the build directory; the exact path depends on your generator and JUCE version. Typical locations:

- `BTZ/build/BTZ_artefacts/Release/VST3/BTZ.vst3`
- `BTZ/build/VST3/Release/BTZ.vst3`

Search under `BTZ/build` for `*.vst3` to find it.

## 4. Install into FL Studio

### Windows

Copy the `BTZ.vst3` **folder** (the whole bundle) to:

```
C:\Program Files\Common Files\VST3\
```

You may need to run Explorer (or your script) **as Administrator**. Then in FL Studio: **Options → Manage Plugins → Find installed plugins**.

Or run (from repo root, after building):

```bat
scripts\install_vst3_windows.bat
```

(Adjust that script if your build output path is different; see above.)

### macOS

Copy the `BTZ.vst3` bundle to:

```
/Library/Audio/Plug-Ins/VST3/
```

Example (after locating the built bundle):

```bash
sudo cp -R path/to/BTZ.vst3 /Library/Audio/Plug-Ins/VST3/
```

Then rescan in FL Studio.

## 5. Optional: JUCE as submodule (no system install)

From repo root:

```bash
git submodule add https://github.com/juce-framework/JUCE.git JUCE
```

Then use a **root** `CMakeLists.txt` that does `add_subdirectory(JUCE)` and `add_subdirectory(BTZ)`, and build from the root so JUCE is available to BTZ. Alternatively, build from `BTZ` and set `JUCE_DIR` to the JUCE build or install directory that contains `JUCEConfig.cmake`.

## 6. Debug / verification

- A debug log line is written when the plugin’s `prepareToPlay` runs (e.g. after loading in a host).
- Log path (this session): `c:\Users\marcm\OneDrive\Desktop\btz-sonic-alchemy-main\.cursor\debug.log`
- One NDJSON line per prepare, with `"message":"BTZ plugin prepared"` and `hypothesisId":"H4"`. Use this to confirm the plugin loads in FL Studio (or another host).

## 7. One-click builds in Cursor

Use **Terminal → Run Task** (or run the scripts from the integrated terminal):

- **Windows**: `scripts\build_windows_release.bat` or `scripts\build_windows_release.ps1`
- **macOS**: `./scripts/build_macos_release.sh`

Install scripts:

- **Windows**: `scripts\install_vst3_windows.bat` (copy to `C:\Program Files\Common Files\VST3\`)
- **macOS**: `./scripts/install_vst3_macos.sh` (copy to `/Library/Audio/Plug-Ins/VST3/`)
