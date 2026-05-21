# BTZ Sonic Alchemy v1.0 Baseline Archive

## Purpose

This document records the consolidation of all former development iterations (V1 through V12) into the new official **BTZ Sonic Alchemy v1.0 Baseline**. This is not a rollback or rewrite — it is a preservation and reframing of the accumulated stable state as the production foundation for all future work.

All code, DSP systems, UI systems, validation logic, tests, assets, and documentation that existed at the time of consolidation have been preserved. Only confirmed dead files (legacy `Source_OLD` stubs and obsolete backup archives) were removed during the V12 cleanup pass.

---

## Baseline Summary

### DSP Architecture

The plugin implements a complete saturation, dynamics, and tone-shaping signal chain in a single-header DSP library (`BTZDsp.h`). The architecture includes:

- **9 saturation models** (5 analog waveshapers + 4 neural GRU slots)
- **Multiband engine** with 1-6 band Linkwitz-Riley LR4 crossover
- **Glue compressor** with sidechain HPF, adjustable attack/release/ratio/link
- **True peak limiter** with 8-sample lookahead and sub-sample peak detection
- **Resonance tamer** (32-band spectral peak suppression)
- **Transient splitter** (envelope-based transient/sustain separation)
- **Shine EQ** (TDF-II biquad parametric)
- **Mid/Side processing** with full encode/decode
- **Auto-gain compensation** (smoothed input/output level matching)
- **LFO modulation** (up to 4 LFOs with sine/triangle/square/S&H)
- **Safety layer** (DC block, NaN/Inf guard, denormal flush, hard clip)
- **Delta monitoring** (wet minus dry difference signal)

### UI/UX Direction

The accepted visual identity is the **Ivory System** — a warm, premium light palette with ceramic-style knobs:

- Background: Ivory (`#F5F0E8`)
- Surface: Parchment (`#EDE7DB`)
- Primary accent: Sage (`#7A9E7E`)
- Secondary accent: Terracotta (`#C17B4A`)
- Text: Charcoal (`#2D2A26`)
- Typography: Inter (UI), JetBrains Mono (values)

Three progressive-disclosure view modes:

| Mode | Target | Content |
|------|--------|---------|
| Simple | Beginners | 3 macro knobs + harmonic visualizer |
| Standard | Producers | 6 character knobs + meters + sliders |
| Advanced | Sound designers | Full spectrum, neural browser, LFO, multiband, all meters |

### Parameter System

All parameters are registered via JUCE `AudioProcessorValueTreeState` with string IDs defined in the `btz::id` namespace. Current registered parameters include: drive, mix, master, bypass, midSide, width, model, quality, resSens, transSens, multibandCount, shine, shineFreq, shineQ, ceiling, intensity, glueAttack, glueRelease, glueRatio, glueLink.

### Macro Controls

Simple Mode maps 3 macro knobs (Drive, Tone, Output) to underlying parameters via the `MacroInterpreter` system. The macro system supports up to 4 slots with 4 mappings each, with linear/exponential/logarithmic/S-curve mapping options.

### Metering

- EBU R128 LUFS (Momentary, Short-term, Integrated)
- True Peak (dBTP)
- Stereo Correlation
- Gain Reduction History (60-second scrolling graph)
- Spectrum Buffer (FFT-based)
- Input/Output level meters with peak hold

### Validation and Debugging Work

- 80+ GoogleTest unit tests covering all DSP modules
- RT-safety verified (no allocations, no blocking, no String on audio thread)
- Claude Code review (CODE_REVIEW_v11.md) — all findings addressed
- Enhancement brief gap analysis (ENHANCEMENT_BRIEF_GAP_ANALYSIS.md) — all findings addressed
- pluginval validation target in CI (strictness 5)

### Git/Repo Cleanup

- Legacy `Source_OLD_20260213_223041/` directory removed (30+ dead stub files)
- Legacy `CMakeLists_OLD_20260213_223041.txt` removed
- Single clean source tree: `BTZ/Source/` (7 files)
- CI/CD workflow: `.github/workflows/ci.yml`
- Tests: `BTZ/tests/test_dsp_modules.cpp`

### Testing Status

All 80+ unit tests are written to match the current v1.0 API. Tests cover:
- All saturation models (Tanh, Tube, Tape, Transistor, Transformer)
- Neural model loading
- Filter correctness (ShineProcessor, SidechainHPF, LinkwitzRileyCrossover)
- Dynamics (GlueCompressor, TruePeakLimiter, EnvFollower)
- Modulation (LFO, MacroInterpreter)
- Safety (NaN, Inf, denormals, edge cases)
- State management (UndoStack, ABState, MIDILearnState)
- Metering (LoudnessMeter, SpectrumBuffer, GainReductionHistory)
- Analysis (ReferenceToneMatcher, PresetIntelligence)

### Known Remaining Issues

1. **No test compilation yet** — JUCE must be fetched by CMake before tests can actually compile and run
2. **Neural model weights** — The 4 neural slots (Neve, API, SSL, Custom) require trained RTNeural model files that do not yet exist
3. **Preset files** — No factory preset `.btzpreset` files have been created yet
4. **Code signing** — No Apple notarization or Windows Authenticode signing configured
5. **Installer** — No DMG/PKG (macOS) or NSIS/WiX (Windows) installer scripts

### Next Recommended Priorities

1. Attempt first successful CMake build on macOS or Windows
2. Fix any remaining type errors surfaced by the compiler
3. Create 5-10 factory presets
4. Train at least one neural model for the RTNeural slots
5. Set up code signing for distribution
6. Create installer scripts

---

## Historical Version Archive

### Former V1 — Initial DSP Architecture

**Date**: 2026-04-19  
**Commit**: `a625e86`

#### Summary
First modular DSP engine overhaul. Established the core architecture with SparkLimiter, ShineProcessor, and LR2 crossover.

#### Added
- Modular DSP architecture in `BTZDsp.h`
- SparkLimiter (initial limiter implementation)
- ShineProcessor (initial EQ implementation)
- LR2 crossover (2nd-order Linkwitz-Riley)
- Waveshaper with multiple saturation models

#### Improved
- Replaced monolithic processor with modular header-only DSP library

#### Fixed
- N/A (initial implementation)

#### Retained in New v1.0 Baseline
- Core architecture pattern (single-header DSP library)
- Waveshaper saturation models (Tanh, Tube, Tape, Transistor, Transformer)
- Modular struct-based design

#### Superseded / Deprecated Notes
- SparkLimiter superseded by TruePeakLimiter (V2)
- LR2 crossover superseded by LR4 (V4)
- Original ShineProcessor topology superseded by TDF-II biquad (V12)

---

### Former V2 — ADAA Saturation + True Peak Limiter

**Date**: 2026-04-19  
**Commits**: `4db1fd3`, `e5fc86a`, `c74bce9`, `5b57b79`

#### Summary
Major DSP quality upgrade introducing anti-aliased saturation, proper true-peak limiting, SVF-based Shine EQ, and FTZ/DAZ denormal handling. First unit tests added.

#### Added
- ADAA (Anti-Derivative Anti-Aliasing) tanh saturator
- TruePeakLimiter with ISP (Inter-Sample Peak) detection and monotonic deque
- SVF-based ShineProcessor
- FTZ/DAZ denormal flushing
- First GoogleTest suite (ADAATanh, TruePeakLimiter, SVF, RT-safety tests)
- Complete UI unification pass

#### Improved
- Saturation quality (reduced aliasing artifacts)
- Limiter accuracy (sub-sample peak detection)
- EQ precision (SVF topology)

#### Fixed
- Removed sparkMix slider/attachment (limiter no longer has dry/wet)

#### Retained in New v1.0 Baseline
- TruePeakLimiter architecture (lookahead + monotonic deque)
- ADAA concept (retained in waveshaper design)
- FTZ/DAZ safety approach
- GoogleTest framework choice

#### Superseded / Deprecated Notes
- SVF ShineProcessor superseded by TDF-II biquad (V12)
- Original ADAA implementation simplified in later versions

---

### Former V3 — Ecosystem Design System

**Date**: 2026-04-19  
**Commit**: `4fb4665`

#### Summary
First major UI design system. Introduced luxury refinement aesthetic and expanded window to 1280x800.

#### Added
- Ecosystem design system with design tokens
- Luxury refinement visual direction
- 1280x800 window size
- Theme token architecture

#### Improved
- Visual consistency across all components
- Professional appearance

#### Fixed
- N/A

#### Retained in New v1.0 Baseline
- Design token architecture concept (evolved into `btz::` namespaces)
- Separation of theme from components
- Window sizing approach

#### Superseded / Deprecated Notes
- Visual direction superseded by multiple theme iterations (V5-V9) and finally Ivory System (V12)

---

### Former V4 — Mathematical DSP Overhaul

**Date**: 2026-04-19  
**Commits**: `1ba28d3`, `b081749`

#### Summary
Rigorous mathematical overhaul of all DSP algorithms. Introduced proper LR4 crossover, soft-knee compression, Pade [5/5] approximation, and perceptual macro curves. 55 unit tests added.

#### Added
- Linkwitz-Riley 4th-order (LR4) crossover
- Soft-knee glue compressor
- Pade [5/5] rational approximation for tanh
- Perceptual macro curves
- FixedDeque data structure
- 55 unit tests covering all new modules

#### Improved
- Crossover accuracy (LR2 → LR4, proper allpass phase alignment)
- Compressor musicality (hard-knee → soft-knee)
- Saturation efficiency (Pade approximation vs. std::tanh)
- Macro response curves (linear → perceptual)

#### Fixed
- Phase alignment issues in crossover
- Compressor pumping artifacts

#### Retained in New v1.0 Baseline
- LR4 crossover design (further corrected in V12)
- Soft-knee compression concept
- Perceptual macro curves
- FixedDeque for limiter lookahead
- All 55 tests (updated to match current API)

#### Superseded / Deprecated Notes
- LR4 implementation corrected in V12 (proper cascaded Butterworth)
- Pade approximation retained but waveshaper uses std::tanh for accuracy

---

### Former V5 — Audit-Driven Fixes

**Date**: 2026-04-19  
**Commits**: `d93fd72`, `c44d07c`, `96c9e35`

#### Summary
First external audit pass. Fixed bugs discovered through systematic testing. Added Python verification scripts and regression tests.

#### Added
- Python verification scripts for DSP validation
- Audit regression test suite
- Restored original tan/oak/sage green palette (theme iteration)

#### Improved
- Code correctness based on audit findings
- Test coverage for edge cases

#### Fixed
- Multiple DSP bugs identified by audit (inferred from commit message)
- Parameter range issues
- Edge case handling

#### Retained in New v1.0 Baseline
- All bug fixes
- Audit methodology (test-driven validation)
- Regression test patterns

#### Superseded / Deprecated Notes
- Python verification scripts may need updating for current API
- Tan/oak/sage palette superseded by Ivory System

---

### Former V6 — Macro Wiring + Sidechain HPF

**Date**: 2026-04-19  
**Commits**: `5960bb2`, `6e04234`

#### Summary
Wired the MacroInterpreter system to actual parameters. Added sidechain high-pass filter to the glue compressor. Integration tests for SidechainHPF and macro wiring.

#### Added
- MacroInterpreter wiring (macros now control real parameters)
- Sidechain HPF for glue compressor
- SidechainHPF integration tests
- MacroWiring integration tests

#### Improved
- Compressor behavior (HPF prevents kick from triggering excessive compression)
- Macro system now functional (was previously dead code)

#### Fixed
- Macros were previously declared but never invoked

#### Retained in New v1.0 Baseline
- MacroInterpreter architecture
- SidechainHPF (topology corrected in V12)
- Integration test patterns

#### Superseded / Deprecated Notes
- SidechainHPF topology corrected in V12 (proper 1-pole HPF)

---

### Former V7 — Release-Gate Hardening + CLAP

**Date**: 2026-04-19  
**Commits**: `b709df6`, `c154f61`, `4e78799`

#### Summary
Release-readiness pass. Added CLAP format support, pluginval integration targets, hardened regression tests, and validation scripts.

#### Added
- CLAP format build support (via clap-juce-extensions)
- pluginval integration targets in CMake
- Hardened regression tests
- Validation scripts

#### Improved
- Build system robustness
- Test reliability
- Release-gate criteria

#### Fixed
- DSP release-gate bugs (inferred from "release-gate hardening")

#### Retained in New v1.0 Baseline
- CLAP format support in CMakeLists.txt
- pluginval validation concept (now in CI/CD)
- All regression tests

#### Superseded / Deprecated Notes
- Original CI workflow removed (V1 era), re-added in V12 with GitHub Actions

---

### Former V8 — API Reconciliation

**Date**: 2026-04-19  
**Commit**: `640c884`

#### Summary
Reconciled test API expectations with actual DSP module signatures. Updated test suite. Created industry comparison roadmap.

#### Added
- Industry comparison roadmap document
- API compatibility layer concepts

#### Improved
- Test suite alignment with DSP API
- Documentation of competitive landscape

#### Fixed
- Test API mismatches (method signatures, return types)

#### Retained in New v1.0 Baseline
- API reconciliation approach
- Test/implementation alignment discipline

#### Superseded / Deprecated Notes
- Industry roadmap document superseded by ENHANCEMENT_BRIEF_GAP_ANALYSIS.md

---

### Former V9 — Industry Gap Closure

**Date**: 2026-04-19  
**Commit**: `3135cfc`

#### Summary
Addressed 14 competitive improvements identified in the industry comparison. Audit cleanup pass.

#### Added
- 14 competitive improvements (inferred: likely includes features like reference tone matching, preset intelligence, stereo correlation, etc.)
- Audit cleanup

#### Improved
- Feature parity with commercial competitors
- Code cleanliness

#### Fixed
- Various issues identified in audit cleanup

#### Retained in New v1.0 Baseline
- All competitive improvements
- ReferenceToneMatcher, PresetIntelligence, LoudnessMatchedAB (likely added here)

#### Superseded / Deprecated Notes
- None — all V9 features retained

---

### Former V10 — Complete UI/UX + DSP Overhaul

**Date**: 2026-05-20  
**Commits**: `eb8a2ac`, `92ca619`

#### Summary
Major overhaul branded "Greatest Plugin Edition." Industry-transcending DSP architecture with complete UI/UX redesign. This was the largest single iteration.

#### Added
- "Industry-transcending" DSP architecture (expanded module set)
- Complete UI/UX overhaul
- Neural saturation model infrastructure (RTNeural GRU slots)
- Wave Digital Filter model concepts
- Expanded metering (EBU R128, correlation, GR history)
- MIDI Learn system (up to 32 mappings)
- Undo/Redo (64-step state history)
- A/B comparison (loudness-matched)
- Oversampling engine
- Stereo width control

#### Improved
- Every DSP module expanded and refined
- UI architecture (3-mode progressive disclosure concept introduced)
- State serialization (versioned with migration)
- Parameter count expanded significantly

#### Fixed
- Multiple architectural issues from V1-V9

#### Retained in New v1.0 Baseline
- All V10 DSP modules
- 3-mode UI concept
- Neural model infrastructure
- MIDI Learn, Undo/Redo, A/B systems
- Metering architecture
- State versioning

#### Superseded / Deprecated Notes
- Visual theme superseded (went through V5-V9 theme iterations before settling on Ivory)
- Some V10 modules were initially dead code (wired up in V12)

---

### Former V11 — Senior-Dev Quality Rewrite

**Date**: 2026-05-20  
**Commits**: `3905c4f`, `518f81c`

#### Summary
Complete senior-developer quality pass. Rewrote all code to production standards. Claude Code review identified and fixed compilation errors, DSP correctness bugs, RT-safety violations, and UX wiring gaps.

#### Added
- CODE_REVIEW_v11.md (Claude Code compilation audit)
- ENHANCEMENT_BRIEF_GAP_ANALYSIS.md (gap analysis)
- Complete test suite rewrite (80+ tests)
- RT-safety atomic patterns

#### Improved
- Code quality (zero dead code policy)
- RT-safety (no allocations on audio thread)
- API consistency (all modules have matching test coverage)
- Documentation (inline comments, architecture notes)

#### Fixed
- 4 hard compile errors (missing constants, rvalue binding, missing methods)
- 5 APVTS parameter ID mismatches
- 3 DSP filter topology bugs (ShineProcessor, SidechainHPF, LinkwitzRileyCrossover)
- 3 RT-safety violations (setLatencySamples, multibandEngine.prepare, MIDI learn String)
- 4 dead code modules wired up (MultibandEngine, LFOs, GlueCompressor params, Limiter ceiling)
- 4 UX wiring gaps (LabeledKnob interactivity, preset save, preset browser, simple mode knobs)
- 5 test API mismatches

#### Retained in New v1.0 Baseline
- All V11 fixes and improvements
- Review documents (for historical reference)
- RT-safety patterns
- Test suite

#### Superseded / Deprecated Notes
- V11 version numbering superseded by v1.0 baseline naming

---

### Former V12 — Ivory System Design + CI/CD

**Date**: 2026-05-21  
**Commit**: `51662d3`

#### Summary
Final design system implementation (Ivory System) with complete CI/CD pipeline. This is the last iteration before consolidation into v1.0.

#### Added
- Ivory System design tokens (BTZTheme.h rewrite)
- Ceramic-style knob components (BTZComponents.h rewrite)
- 3-mode PluginEditor layout (BTZEditor rewrite)
- Delta monitoring (wet - dry)
- Stereo correlation measurement
- GitHub Actions CI/CD (macOS/Windows/Linux + pluginval)
- Updated README with full architecture documentation

#### Improved
- Visual identity finalized (Ivory System)
- Component interactivity (all knobs now functional)
- Build system (removed hardcoded paths, auto-fetch JUCE)
- Documentation (README, signal flow, design system)

#### Fixed
- Legacy backup directory removed (Source_OLD)
- CMakeLists.txt hardcoded Windows path removed
- Test version assertions updated

#### Retained in New v1.0 Baseline
- Everything — V12 IS the v1.0 baseline

#### Superseded / Deprecated Notes
- None — this is the current state

---

## New v1.0 Baseline Definition

The following are the accepted and locked-in decisions for the v1.0 baseline:

### Accepted UI Direction
- **Ivory System** design language
- Warm light palette (ivory, parchment, sage, terracotta, charcoal)
- Ceramic-style knobs with accent arcs
- Inter + JetBrains Mono typography
- 3-mode progressive disclosure (Simple / Standard / Advanced)
- Header: logo, mode tabs, preset nav, A/B, delta, undo/redo
- Footer: LUFS, correlation, safety indicator

### Accepted DSP Architecture
- Single-header library (`BTZDsp.h`) with all modules as structs
- Signal flow: Safety → AutoGain In → M/S → ResTame → Glue → Shine → Multiband → Saturation → LFO → Recombine → AutoGain Out → Limiter → M/S Decode → Width → Mix → Master → Safety → Delta → Metering
- 9 saturation models (5 analog + 4 neural)
- TDF-II biquad filters
- LR4 crossover via cascaded Butterworth
- Lock-free audio thread (atomics only, no allocations)

### Accepted Macro System
- MacroInterpreter with 4 slots, 4 mappings each
- Simple Mode maps Drive/Tone/Output to underlying params
- Linear, exponential, logarithmic, S-curve mapping options

### Accepted Validation Standards
- 80+ GoogleTest unit tests
- pluginval strictness 5 (CI), strictness 10 (release)
- RT-safety: no allocations, no blocking, no String on audio thread
- All parameters registered in APVTS with string IDs

### Accepted Branding Direction
- Product name: **Box Tone Zone (BTZ)**
- Suite name: **BTZ Sonic Alchemy**
- Visual identity: Ivory System
- Version: v1.0 Baseline

### Accepted Repo Organization
```
btz-sonic-alchemy-main/
├── .github/workflows/ci.yml
├── BTZ/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── CODE_REVIEW_v11.md
│   ├── ENHANCEMENT_BRIEF_GAP_ANALYSIS.md
│   ├── Source/
│   │   ├── BTZDsp.h
│   │   ├── BTZTheme.h
│   │   ├── BTZComponents.h
│   │   ├── PluginProcessor.h
│   │   ├── PluginProcessor.cpp
│   │   ├── PluginEditor.h
│   │   └── PluginEditor.cpp
│   └── tests/
│       └── test_dsp_modules.cpp
├── docs/
│   ├── BTZ_V1_BASELINE_ARCHIVE.md
│   ├── BTZ_PROJECT_PRIMER.md
│   ├── BTZ_MASTER_ARCHITECTURE.md
│   ├── BTZ_AI_WORKFLOW.md
│   └── BTZ_CHANGELOG.md
└── README.md
```

---

## Future Versioning Rule

All future work after this consolidation must follow semantic versioning:

| Version | Scope |
|---------|-------|
| **v1.0.x** | Bug fixes, cleanup, validation, small refinements |
| **v1.1.x** | Meaningful feature additions or UI/DSP enhancements |
| **v1.2.x** | Larger subsystem upgrades |
| **v2.0** | Only for major architecture-breaking redesigns |

Examples:
- `v1.0.1` — Fix a type error found during first compile
- `v1.0.2` — Add 5 factory presets
- `v1.1.0` — Add sidechain input support
- `v1.2.0` — Replace RTNeural with custom inference engine
- `v2.0.0` — Rewrite to multi-plugin suite architecture

---

## AI Agent Continuity Instructions

All future AI agents working on this repository must:

1. **Read this archive first** before making any changes
2. **Preserve the v1.0 baseline** — do not delete retained features or rewrite architecture
3. **Use patch mode** — make surgical edits, not wholesale rewrites
4. **Do not rewrite architecture** — the DSP signal flow, UI mode system, and component structure are locked
5. **Do not delete retained features** — all modules listed above are intentional and must remain
6. **Update the changelog** (`/docs/BTZ_CHANGELOG.md`) after every meaningful edit
7. **Document all future changes** with clear commit messages following conventional commits
8. **Respect parameter IDs** — never rename parameter string IDs without migration code
9. **Respect preset compatibility** — state version must increment with migration support
10. **Test before pushing** — ensure tests still match the API after any DSP changes

If in doubt about whether something should be changed, **ask the user first**.
