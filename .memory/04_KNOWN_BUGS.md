# Known Bugs

## Open

### C4244 warnings in DSP headers
- Files: `BTZ/Source/TapeEmulator.h`, `BTZ/Source/GranularProcessor.h`
- Symptom: conversion from `double` to `float`, possible loss of data
- Severity: Low
- Workaround: None needed for current build/install
- Planned Fix: explicit `static_cast<float>(...)` where appropriate

## Recently Resolved

### JUCE Convolution API mismatch (JUCE 8)
- Symptom: `loadImpulseResponse` signature mismatch error
- Fix: updated call to enum-based signature (`Stereo`, `Trim`)
- Status: Resolved

### Duplicate destructor definition
- Symptom: C2084 duplicate destructor body
- Fix: removed out-of-line defaulted destructor in `.cpp`
- Status: Resolved

### x86/x64 linker conflicts in helper
- Symptom: LNK4272 and unresolved externals in `juce_vst3_helper.exe`
- Fix: enforced VS 2022 x64 flow + clean build script
- Status: Resolved (for current environment)

## TODO
- [TODO] Add issue IDs once formal tracking exists.
