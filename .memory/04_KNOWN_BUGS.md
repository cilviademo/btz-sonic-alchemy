# Known Bugs

## Open

### Advanced tab page not implemented
- Files: `BTZ/Source/PluginEditor.cpp:266`
- Symptom: Advanced tab exists but displays empty page
- Severity: Medium (missing Quality Mode and Character controls in UI)
- Workaround: Parameters accessible via automation
- Planned Fix: Implement sliders for qualityMode, stabilityMode, autogain parameters

### Missing parameter tooltips
- Files: `BTZ/Source/PluginEditor.cpp`
- Symptom: No tooltips on knobs/sliders to explain parameters
- Severity: Low (UX improvement)
- Workaround: Trial and error
- Planned Fix: Add tooltip text for all 20 parameters

### Magic numbers in DSP code
- Files: `BTZ/Source/PluginProcessor.cpp`
- Symptom: Hardcoded numeric constants without named explanations
- Severity: Low (maintainability)
- Workaround: None needed
- Planned Fix: Extract to named constants with comments

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
