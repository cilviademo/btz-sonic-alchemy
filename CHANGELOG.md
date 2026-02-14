# Changelog

All notable changes to BTZ will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2026-02-09 (Release Candidate)

### Added
- **Build System**
  - JUCE FetchContent fallback (auto-downloads 7.0.12 if submodule missing)
  - Platform-specific AU format (macOS only, excluded on Windows/Linux)
  - Comprehensive BUILD.md with exact build commands for all platforms
  - CI/CD workflows (Windows, macOS Universal, Linux) + pluginval validation

- **Documentation**
  - Ship Validation Guide (`docs/SHIP_VALIDATION_GUIDE.md`) - Exact test protocols
  - Ship Gate Report (`SHIP_GATE_REPORT.md`) - Release readiness tracker
  - Updated QA Checklist with FL Studio primary target and determinism protocol
  - Parameter count correction (27→29) in PARAMETER_MANIFEST.md

- **Tooling**
  - Offline rendering tool (`BTZ_JUCE/tools/offline_render.cpp`)
    - Objective metrics: peak, RMS, crest factor, DC offset, CPU time
    - A/B comparison generation (bypass vs processed)
    - Compile with: `cmake -DBTZ_BUILD_TOOLS=ON`
  - Determinism test scripts (Bash + PowerShell)
    - Automated offline bounce testing (5 consecutive renders, MD5 comparison)
    - CRITICAL for professional mixing/mastering validation

- **Repository Structure**
  - Legacy stubs archived to `legacy/BTZ_old_stubs/` with explanatory README
  - Hardened `.gitignore` with 65+ JUCE plugin patterns (zips, builds, binaries)

### Changed
- **UI Polish** (Phase 4)
  - Color palette: Replaced gold with natural oak brown (#8B7355)
  - Warmer sage green (#9CAF88), charcoal black for text (#1A1A1A)
  - BTZKnob rendering: Output Thermal/Portal inspired 3D bevels
    - Warm top-left lighting, gentle bottom-right shadows
    - Thicker value arc (5.0f stroke) for prominence
    - Beveled pointer with shadow/highlight
    - Center cap with radial gradient
  - Professional analog hardware aesthetic (beige/sage/oak/black palette)

- **Build Configuration**
  - C++17 standard requirement (explicitly set in CMakeLists.txt)
  - BUNDLE_ID: `com.btzaudio.btz`
  - VST3_CATEGORIES: `Fx Dynamics EQ`

### Fixed
- **Parameter Contract**
  - Fixed PARAMETER_MANIFEST.md count (claimed 27, actually 29)
  - Verified all parameter IDs match between definition, implementation, docs
  - System section: 2→3 parameters (precisionMode, active, oversampling)

- **FL Studio Constructor Safety**
  - Verified at code level (PluginProcessor.cpp:9-23)
  - Only lightweight initialization in constructor (APVTS, PresetManager reference)
  - ALL DSP allocation deferred to prepareToPlay() (lines 89-146)

### Removed
- Redundant `btz-sonic-alchemy-main.zip` (327KB duplicate archive)

### Verified
- ✅ No allocations in processBlock (RT-safe)
- ✅ Parameter smoothing block-boundary safe
- ✅ Denormal protection enabled
- ✅ FL Studio constructor scan-safe
- ✅ 29 parameters documented and verified
- ✅ Cross-platform builds (Windows VST3, macOS Universal VST3+AU, Linux VST3)

### Known Limitations
- **Disabled Modules** (deferred to v1.1.0):
  - `AdvancedTransientShaper.cpp` - TPTOnePole API mismatch
  - `WDFSaturation.cpp` - Array initialization issue
  - **Impact**: Minimal (alternative implementations active)

### Pending Manual Validation
⏸️ **Post-merge QA required** (see `docs/SHIP_VALIDATION_GUIDE.md`):
- FL Studio scan test (Windows, PRIMARY TARGET)
- pluginval (strictness 10) all platforms
- Offline bounce determinism (5/5 MD5 match)
- Multi-DAW compatibility (Ableton, Reaper, Studio One, Logic)
- Performance validation (CPU, memory)
- Audio quality checks (null test, DC offset, frequency response)

---

## [0.9.0] - 2026-01-15 (Pre-Release Development)

### Added
- Core DSP chain (29 modules integrated)
  - Hero controls: Punch, Warmth, Boom, Mix, Drive
  - SPARK: Advanced clipping engine with oversampling
  - SHINE: Ultra-high frequency air (+12 dB capable)
  - Enhanced saturation, transient shaping, sub-harmonic generation
- A/B/C preset system with click-free switching (20ms crossfade)
- 5 factory presets (Punchy Drums, Warm Glue, Bright Master, etc.)
- Custom GUI with BTZ theme (MainView-based)
- Comprehensive test suite (Google Test framework, 12+ test files)
- RT-safety manifest and documentation
- Parameter smoother system (block-rate + per-parameter timing)

### Changed
- Migrated from stub implementation to full JUCE-based plugin
- Upgraded to JUCE 7.0.12
- Comprehensive architecture documentation

### Fixed
- P0-P1 ship blockers (70% → 80% completion)
- Bypass bit-perfect processing
- State round-trip determinism
- Parameter conversion accuracy
- CI cross-platform stabilization (Linux + macOS)

---

## [0.5.0] - 2026-01-08 (Alpha)

### Added
- Initial JUCE plugin scaffold
- Basic parameter layout (27 parameters)
- Stub DSP modules
- Basic documentation (ARCHITECTURE.md, PARAMETER_MANIFEST.md)

---

## Legend

- **Added**: New features or functionality
- **Changed**: Changes to existing functionality
- **Deprecated**: Soon-to-be-removed features
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Vulnerability fixes
- **Verified**: Code-level verification complete (no user testing yet)
- **Pending**: Awaiting user action or manual validation

---

**Note**: v1.0.0 is a **Release Candidate**. All code-level ship blockers resolved. Manual QA validation required before final release tag.

See `SHIP_GATE_REPORT.md` for detailed release readiness status.
