# BTZ Legacy Code Archive

This directory contains historical/deprecated code that is preserved for reference but not actively used.

## BTZ_old_stubs/

**Date Archived**: 2026-02-08
**Original Location**: `/BTZ/`
**Status**: DEPRECATED - Replaced by `/BTZ_JUCE/`

### What This Was

An early prototype/stub implementation of the BTZ plugin with:
- 15 mostly-empty source files (<100 bytes each)
- CMake configuration targeting VST3
- No functional DSP implementation (stubs only)

### Why It Was Archived

1. **Superseded by BTZ_JUCE/**: The real plugin implementation lives in `/BTZ_JUCE/` with:
   - 29 fully-implemented DSP modules
   - Complete JUCE integration
   - Production-ready signal chain
   - 48 unit tests

2. **No References**: Nothing in the active codebase references these stubs

3. **Repo Hygiene**: Keeping stub files alongside production code causes confusion

### Contents

```
BTZ_old_stubs/
├── CMakeLists.txt           - Basic CMake stub (42 lines)
├── README.md                - Old README
├── Source/
│   ├── PluginProcessor.cpp  - 7KB stub implementation
│   ├── BassEnhancer.cpp     - 26 bytes (stub)
│   ├── ConsoleEmulator.cpp  - 29 bytes (stub)
│   ├── DeepFilterNet.cpp    - 27 bytes (stub)
│   ├── EQ.cpp               - 16 bytes (stub)
│   ├── Fuzzed.cpp           - 20 bytes (stub)
│   └── [9 other stubs]      - All <100 bytes
└── tests/
    └── test_processing.cpp  - 6KB placeholder test
```

### Historical Context

This was an early exploration of the plugin structure before the real JUCE implementation. The files were created as placeholders but never filled in with actual DSP code.

The actual production code is in `/BTZ_JUCE/` which was developed from scratch with:
- Airwindows-inspired saturation algorithms
- ITU-R BS.1770-4 compliant LUFS metering
- Jiles-Atherton hysteresis modeling
- 24 Bark band psychoacoustic processing
- Professional JUCE GUI with custom controls

### Migration Path

If you need to reference any of these files, they are preserved here unchanged.

**DO NOT USE THESE FILES FOR ACTIVE DEVELOPMENT**

Use `/BTZ_JUCE/` instead.

---

**Archived by**: Phase 1 Repo Hygiene (ship-package-import post-merge cleanup)
**Commit**: cleanup/merge-real-plugin branch
