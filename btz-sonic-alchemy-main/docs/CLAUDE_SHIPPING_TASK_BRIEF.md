# BTZ Sonic Alchemy — Shipping Quality Task Brief

This document outlines the remaining tasks required to bring the BTZ Sonic Alchemy plugin from its current v1.0.1 baseline to a fully shippable, commercial-grade product. 

Claude Code: Use this brief as your master checklist. The codebase is structurally sound, DSP is mathematically correct, and the UI is fully designed. Your job is to cross the finish line.

---

## 1. Compilation & Build System

The plugin has not yet been successfully compiled. The CMake build system is configured, but it needs to be run and verified.

- [ ] **First Compile**: Run the CMake build on macOS or Windows.
- [ ] **Fix Type Mismatches**: Resolve any remaining C++ type mismatches, missing includes, or JUCE API changes that surface during the first compile.
- [ ] **ARM Portability**: The `BTZDsp.h` file uses SSE intrinsics (`<xmmintrin.h>`, `<emmintrin.h>`). Ensure these are properly guarded with `#ifdef __SSE__` and provide fallback implementations or NEON equivalents for Apple Silicon (ARM64) builds.
- [ ] **JUCE Boilerplate**: Verify that `PluginProcessor.cpp` correctly implements all required JUCE virtual methods (e.g., `acceptsMidi`, `producesMidi`, `isBusesLayoutSupported`, `getTailLengthSeconds`).

## 2. DSP & Neural Models

The DSP architecture is complete, but the neural models are currently stubs.

- [ ] **Neural Weights**: The `NeuralSaturationModel` struct expects weight files, but none exist in the repository. You need to either:
  - Train and provide actual `.json` or `.weights` files for the 4 neural slots (Neve, API, SSL, Custom).
  - Or, implement a fallback mathematical approximation for these models until weights are available.
- [ ] **Correlation Meter**: The UI has a correlation meter (`safetyCorrelation`), but `PluginEditor.cpp` notes it as a placeholder. Implement the correlation calculation in `PluginProcessor` and wire it to the UI.

## 3. Presets & Factory Content

A commercial plugin needs factory presets to demonstrate its capabilities.

- [ ] **Factory Presets**: Create a set of 10-20 factory presets covering common use cases (e.g., "Mix Bus Glue", "Drum Smash", "Vocal Warmth", "Mastering Clean").
- [ ] **Preset Embedding**: Embed these factory presets into the plugin binary using JUCE's `BinaryData` system so they are always available, even if the user hasn't installed them to disk.
- [ ] **Preset Browser**: Ensure the `PresetBrowser` UI correctly loads and displays both factory and user presets.

## 4. Packaging & Distribution

The plugin needs to be packaged for easy installation by end-users.

- [ ] **macOS Installer**: Create a `.pkg` installer script (e.g., using `pkgbuild` or a tool like Packages) that installs the VST3, AU, and Standalone versions to the correct system directories.
- [ ] **Windows Installer**: Create an Inno Setup (`.iss`) or NSIS script for Windows installation.
- [ ] **Code Signing & Notarization**: Add scripts or GitHub Actions steps to handle macOS code signing and notarization, and Windows Authenticode signing. (Note: This requires actual certificates, but the infrastructure should be set up).

## 5. Polish & UX

Final touches to make the plugin feel premium.

- [ ] **Tooltips**: Ensure all UI components have helpful tooltips.
- [ ] **Undo/Redo**: Verify that the `UndoStack` is correctly wired to the UI and APVTS state.
- [ ] **A/B Comparison**: Verify that the A/B state switching works flawlessly without audio glitches.
- [ ] **Graphics Assets**: Ensure all required SVG or PNG assets (e.g., logos, background textures) are present and correctly loaded.

---

## Execution Strategy for Claude Code

1. **Start with Compilation**: Do not attempt any other tasks until the plugin compiles successfully on at least one platform.
2. **Fix ARM Build**: Ensure the SSE intrinsics don't break the Apple Silicon build.
3. **Wire the Missing Pieces**: Fix the correlation meter and neural model loading.
4. **Create Presets**: Build the factory preset library.
5. **Setup Installers**: Write the packaging scripts.

Log all changes in `docs/BTZ_CHANGELOG.md` under a new `v1.1.0` heading.
