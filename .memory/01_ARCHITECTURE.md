# BTZ Architecture

## Frontend Stack
- Plugin editor UI in JUCE C++ (`PluginEditor.*`).
- Non-plugin web UI files exist in repo but are not part of VST build by default.

## Backend Stack
- C++17 audio processing using JUCE modules (`juce_audio_utils`, `juce_dsp`).
- Plugin target generated via `juce_add_plugin(...)` in `BTZ/CMakeLists.txt`.

## State Management
- Plugin parameters primarily via `juce::AudioProcessorValueTreeState` in `PluginProcessor.h`.
- Runtime DSP chain configured in `prepareToPlay()` and executed in `processBlock()`.

## Data Flow
1. Host loads plugin bundle (`BTZ.vst3`).
2. JUCE plugin client creates processor via `createPluginFilter()`.
3. `prepareToPlay()` initializes DSP modules.
4. `processBlock()` applies processing chain per buffer.
5. Output returned to host.

## Folder Structure (Relevant)
- `btz-sonic-alchemy-main/BTZ/` - plugin project root.
- `btz-sonic-alchemy-main/BTZ/Source/` - processor/editor + DSP modules.
- `scripts/` - Windows/macOS build/install scripts.
- `.memory/` - persistent context system.

## System Dependencies
- Visual Studio 2022 Build Tools/Community (x64 toolchain).
- CMake 3.22+.
- Ninja.
- JUCE 8.0.6 source at fixed path in CMake.

## Architecture Guardrails
- Keep JUCE CMake plugin flow (`juce_add_plugin`) as source of truth.
- Do not switch toolchain version mid-build directory.
- Always use clean build dir when changing compiler environment.

## TODO
- [TODO] Document module-level DSP architecture in deeper detail.
- [TODO] Add plugin parameter map table with ranges/defaults.
