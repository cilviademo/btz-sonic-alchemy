# Build and Run Guide (Windows x64)

## Requirements

- Visual Studio 2022 (Community or Build Tools), Desktop C++ workload
- CMake on PATH
- Ninja on PATH
- JUCE source folder available and discoverable by one of:
  - `JUCE_DIR` environment variable
  - `JUCE_ROOT` environment variable
  - fallback local path in `BTZ/CMakeLists.txt`

## Release Build

From repo root:

```bat
scripts\build_windows_with_vs_env.bat
```

## Debug Build

From repo root:

```bat
scripts\build_windows_debug_with_vs_env.bat
```

## Output Locations

- Release VST3:
  - `btz-sonic-alchemy-main/BTZ/build/BTZ_artefacts/Release/VST3/Box Tone Zone (BTZ).vst3`
- Release Standalone:
  - `btz-sonic-alchemy-main/BTZ/build/BTZ_artefacts/Release/Standalone/Box Tone Zone (BTZ).exe`
- Debug VST3:
  - `btz-sonic-alchemy-main/BTZ/build-debug/BTZ_artefacts/Debug/VST3/Box Tone Zone (BTZ).vst3`
- Debug Standalone:
  - `btz-sonic-alchemy-main/BTZ/build-debug/BTZ_artefacts/Debug/Standalone/Box Tone Zone (BTZ).exe`

## Install VST3 (Windows)

```bat
scripts\install_vst3_windows.bat
```

Destination:

- `C:\Program Files\Common Files\VST3\Box Tone Zone (BTZ).vst3`

## DAW Validation Checklist

1. Scan plugins and load BTZ.
2. Confirm audio passes with bypass on/off.
3. Confirm parameter automation records and replays.
4. Save/reload DAW session and verify parameter recall.
5. Confirm no crackles at 44.1/48/96 kHz with 64/128/256 buffers.
