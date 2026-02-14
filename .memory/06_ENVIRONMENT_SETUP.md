# Environment Setup

## Windows Build Prerequisites
- Visual Studio 2022 Community or Build Tools (Desktop development with C++).
- CMake 3.22+.
- Ninja.
- JUCE source extracted to:
  - `C:/Users/marcm/OneDrive/Desktop/JUCE-8.0.6`

## Canonical Build/Install Commands

### One-step (recommended)
- From project root:
  - `scripts/build_and_install_windows_vst3.bat`

### Split flow
- Build:
  - `scripts/build_windows_with_vs_env.bat`
- Install:
  - `scripts/install_vst3_windows.bat`

## Memory Auto-Update Setup
- Install auto-update tooling:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\install_memory_autoupdate.ps1`
- Manual sync:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\update_memory.ps1 -Message "manual sync"`
- Live watcher (optional, long-running):
  - `powershell -ExecutionPolicy Bypass -File .\scripts\start_memory_watcher.ps1`

## Build Artifact Location
- Primary:
  - `btz-sonic-alchemy-main/BTZ/build/BTZ_artefacts/Release/VST3/BTZ.vst3`
- Alternate:
  - `btz-sonic-alchemy-main/BTZ/build/VST3/...`

## Host Setup
- Copy installed plugin bundle to:
  - `C:\Program Files\Common Files\VST3\BTZ.vst3`
- In FL Studio:
  - `Options -> Manage Plugins -> Find installed plugins`

## Troubleshooting
- x86/x64 conflict errors:
  - Use script that loads VS 2022 `vcvars64.bat`.
  - Clean build directory before configure/build.
- Access denied on install:
  - Run terminal/script as Administrator.

## TODO
- [TODO] Add macOS-specific verified setup once actively used.
