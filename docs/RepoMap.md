# BTZ Repository Map

## Active Plugin Source of Truth

- `btz-sonic-alchemy-main/BTZ/CMakeLists.txt`
- `btz-sonic-alchemy-main/BTZ/Source/PluginProcessor.h`
- `btz-sonic-alchemy-main/BTZ/Source/PluginProcessor.cpp`
- `btz-sonic-alchemy-main/BTZ/Source/PluginEditor.h`
- `btz-sonic-alchemy-main/BTZ/Source/PluginEditor.cpp`

## Build and Install Scripts

- `scripts/build_windows_with_vs_env.bat` - Windows x64 Release configure/build
- `scripts/build_windows_debug_with_vs_env.bat` - Windows x64 Debug configure/build
- `scripts/build_and_install_windows_vst3.bat` - one-step build + install
- `scripts/install_vst3_windows.bat` - install VST3 bundle to system folder
- `scripts/build_windows_release.ps1` / `scripts/build_windows_release.bat` - alternate release helpers

## Documentation

- `docs/Build.md` - build/run/install steps
- `docs/Metering.md` - meter math and ballistics
- `docs/Specs.md` - publishable technical specs
- `docs/Measurements.md` - repeatable measurement procedures
- `docs/CompetitiveAnalysis.md` - competitive comparison
- `docs/QuickStart.md` - operational quick start
- `docs/UserManual.md` - full user-facing manual
- `docs/ResearchDigest.md` - research synthesis for future evolution
- `docs/ImplementationRoadmap.md` - impact/effort roadmap
- `docs/PerformanceTargets.md` - CPU/latency and QA targets

## Legacy / Non-Active Directories

- `btz-sonic-alchemy-main/BTZ/Source_OLD_20260213_223041/` - archived prior DSP tree
- `.tmp_zip_apply/btz-juce-rewrite/` - temporary rewrite snapshot, not active build target
- `btz-sonic-alchemy-main/BTZ/build/` and `build-debug/` - generated artifacts

## Plugin Targets

- VST3 bundle output
- Standalone executable output
