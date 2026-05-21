# BTZ Sonic Alchemy — Changelog

All notable changes to this project are documented in this file. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) principles, and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) starting from v1.0.

---

## v1.0.3 — Correctness, Efficiency & QA Pass (2026-05-21)

Deep debugging + performance/security audit against a real JUCE 8.0.6 build
(plugin + tests compile & link; measured with ASan/UBSan + a CPU benchmark).

### Fixed (correctness)
- **CRITICAL: plugin output was 100% dry.** Block-rate parameter smoothers
  (`Mix`, `Width`, `Glue`, `Shine`, `Boom`, `Motion`, …) were read via `.current`
  but never advanced, so they stayed frozen at 0 — Mix=0 meant the wet path was
  discarded and Width=0 collapsed to mono. Added `SmoothParam::advanceBlock()`,
  advance them per block, and snap all smoothers in `prepareToPlay`.
- **`fastTanh` was unbounded** (returned ~6.7 for input 100) — now saturates to [-1,1].
- **Oversampling modulation ran 2–8× too fast** at Quality > Eco — now rate-correct.
- **Transient Sensitivity knob was inert** — wired to the splitter.
- **`reset()` lifecycle** added (clears all DSP state incl. Target Lock + oversampling).

### Changed (efficiency — measured, behavior-preserving)
- TruePeakLimiter caches `dbToGain(ceiling)` (was a `std::pow` per sample).
- Master gain smoothed as a per-block linear ramp (2 `dbToGain`/block vs per sample).
- ResonanceTamer's 32-"band" loop collapsed to one computation — **bit-identical**
  output (all bands were identical), ~32× cheaper for that module.
- Result: worst case (Ultra 8× + all modules) **4.49% → 3.73%** of one core
  (27× realtime); typical Standard path ~1% per instance.

### Fixed (hardening / security)
- `processBlock` re-prepares on an oversized block (host contract violation) to
  avoid an out-of-bounds write on `dryBuffer` / oversampling buffers.
- `SafetyLayer` now clamps runaway values to ±4.0 (~+12 dBFS) — last-resort guard.
- Reviewed state/preset/neural-weight parsing: bounds-checked, robust. ASan/UBSan/
  LSan: **zero findings** across the DSP suite.

### Added (UI / QA)
- Spectrum + harmonic **visualizers now receive live FFT data** (were empty).
- **Processor-level integration test** (`BTZProcessorCheck`): wet-path-active,
  bypass, state round-trip, SR/block-size matrix, oversized-block guard — all pass.
- **CPU benchmark** (`BTZBench`) for regression tracking.
- LinkwitzRiley tests rewritten to the correct LR4 allpass property; whole suite
  now **86/86 green**.

---

## v1.0.2 — First Successful Compile + Build Hardening (2026-05-21)

First build that actually compiles and links. Verified against real JUCE 8.0.6
(FetchContent) + GoogleTest on Linux: plugin Standalone + VST3 link, 81/84 unit
tests pass. All changes surgical — no architecture, DSP topology, or Ivory UI
redesign.

### Fixed (compile restoration — the plugin had never compiled before)

- **Editor ↔ theme sync.** `PluginEditor.cpp` (Ivory System) referenced a theme
  API that was never committed. Resolved via theme-layer compatibility aliases:
  - `palette::` aliases (`ivory`, `porcelain`, `linen`, `sand`, `panelBorder`,
    `charcoal`, `softBlack`, `warmGray`) mapping onto the existing semantic tokens.
  - `type::*Size` aliases (`brandSize`…`valueSize`) onto the scale floats.
  - `type::label()/brand()` (called as functions but defined as floats) → the
    family functions `type::sans()/display()` at the 6 call sites.
- **SafetyIndicator**: added `using Level = State;` + `setLevel()` wrapper for the
  editor's calls (primary API remains `setState`/`State`).
- **LabeledKnob**: now inherits `juce::SettableTooltipClient` so `setTooltip()` resolves.
- **SpectrumDisplay**: added `using SpectrumDisplay = DirectManipSpectrum;` alias.
- **deltaMonitoring**: moved to the processor's public section (editor writes it).
- **Preset save**: converted modal `FileChooser::browseForFileToSave` to async
  `launchAsync` (no modal loops — plugin-safe).
- **Test build**: `BTZTests` now built via `juce_add_console_app` +
  `juce_generate_juce_header` (so `<JuceHeader.h>` resolves) with
  `JUCE_WEB_BROWSER=0`/`JUCE_USE_CURL=0` (drops the gtk dependency). Added missing
  `<random>`/`<limits>` includes.

### Fixed (DSP / wiring — verified by the test run)

- **fastTanh** is now bounded to [-1, 1]. The Padé [5/5] approximation was
  unbounded (returned ~6.7 for input 100, overshoots >1.0 above x≈3.3) — a real
  RT-safety hazard for a saturator. Input is clamped to the valid region and the
  output hard-bounded.
- **Correlation meter** wired to `meters.correlation` (was hardcoded "Safe"):
  ≥0.3 sage / 0–0.3 amber / <0 clay.
- **reset() lifecycle**: added `AudioProcessor::reset()` → `resetAll()`, and
  `resetAll()` now also clears Target Lock + JUCE oversampling state.

### Build / CI / Tooling

- CI: added `baseline/**` trigger; pluginval now gates (strictness 10,
  repeat/randomise, removed `|| true`).
- Added cross-platform scripts: `scripts/build_{linux,macos,windows}.{sh,ps1}`,
  `scripts/run_pluginval.{sh,ps1}`, `scripts/clean_build.{sh,ps1}`.
- Added dev docs under `docs/dev/`: local setup, Cursor/Codex handoffs, validation
  checklist, CI notes, build/test status.

### Known issues (documented, not silently changed — need product decision)

- `LinkwitzRileyCrossover.{SplitsIntoLowAndHigh,SumsFlatAtCrossover}` tests assert
  `low+high == input` (identity). Correct LR4 sums to an allpass (flat magnitude,
  phase-shifted), so identity does not hold. DSP is correct LR4; tests encode a
  first-order-complementary assumption.
- `RTSafety.SafetyLayerHandlesAllEdgeCases` expects the safety layer to hard-clamp
  `|out| ≤ 4.0`; the current design only guards NaN/Inf/denormal + DC.
- Spectrum/harmonic visualizers are not yet fed live FFT data (render empty).
- No factory presets; neural slots have no weights (fall back to tanh); no
  installers / code signing yet.

---

## v1.0.1 — DSP Correctness + Target Lock (2026-05-20)

### Added

- **Target Lock Engine** — type in LUFS, RMS, or per-band (Low/Mid/High) dB targets and lock the output to those values. Dynamics Threshold knob controls how much dynamic range is preserved around the target (0 = brick-wall, 24 dB = gentle correction only).
- Target Lock parameters: `targetLUFS`, `targetRMS`, `targetDynThresh`, `targetLUFSLock`, `targetRMSLock`, `targetLowDb`, `targetMidDb`, `targetHighDb`, `targetLowLock`, `targetMidLock`, `targetHighLock`
- Target Lock UI in Advanced Mode: type-in text fields with LOCK toggles for each target, plus DYN RANGE knob
- 7 new unit tests: TargetLock (3), TruePeakLimiter ISP (1), LoudnessMeter K-weighting (1), LinkwitzRileyCrossover flat sum (1)

### Fixed

- **TruePeakLimiter**: Now uses 4x polyphase FIR interpolation for true inter-sample peak detection (was sample-peak only). Proper lookahead with gain smoothing.
- **LoudnessMeter**: Now implements ITU-R BS.1770-4 K-weighting pre-filter (high-shelf at 1681 Hz + 2nd-order HPF at 38 Hz). Previously was unweighted RMS with a LUFS label.
- **LinkwitzRileyCrossover**: Now proper LR4 using cascaded 2nd-order Butterworth biquads for both LP and HP paths. Previously used subtraction-based HP which doesn't sum flat.

### Changed

- `TruePeakLimiter::kLookahead` increased from 8 to 16 samples (to accommodate ISP detection)
- Signal flow: Target Lock processing inserted after auto-gain, before true-peak limiter

---

## v1.0.0 — Baseline Consolidation (2026-05-21)

This entry marks the consolidation of all former V1 through V12 development iterations into the new official v1.0 baseline. No code was rewritten during this consolidation — only documentation was added and version strings were updated.

### Consolidated

- Preserved all accumulated DSP modules (9 saturation models, multiband, dynamics, EQ, modulation, metering, analysis)
- Preserved Ivory System UI direction (3-mode progressive disclosure, ceramic knobs, sage/terracotta palette)
- Preserved all 80+ GoogleTest unit tests
- Preserved GitHub Actions CI/CD pipeline (macOS, Windows, Linux + pluginval)
- Preserved RT-safety patterns (lock-free audio thread, atomic communication)
- Preserved state serialization with version migration support

### Added

- `docs/BTZ_V1_BASELINE_ARCHIVE.md` — full V1-V12 historical archive with per-version summaries
- `docs/BTZ_PROJECT_PRIMER.md` — quick-start orientation for new contributors
- `docs/BTZ_MASTER_ARCHITECTURE.md` — technical architecture deep-dive
- `docs/BTZ_AI_WORKFLOW.md` — mandatory instructions for AI agents
- `docs/BTZ_CHANGELOG.md` — this file
- Future versioning rules (v1.0.x / v1.1.x / v1.2.x / v2.0)
- AI agent continuity instructions

### Changed

- Version strings in source files updated from "v12" to "v1.0 Baseline"
- `CMakeLists.txt` project version updated to `1.0.0`
- `BTZ/README.md` version reference updated

### Removed

- Nothing. This is a preservation operation.

### Known Issues

- Plugin has not yet achieved a successful first compile (JUCE fetch required)
- Neural model weight files do not exist yet (4 empty slots)
- No factory presets created yet
- No code signing or installer scripts

---

## Pre-v1.0 History

For detailed history of the V1 through V12 development iterations (2026-02-13 to 2026-05-21), see [BTZ_V1_BASELINE_ARCHIVE.md](./BTZ_V1_BASELINE_ARCHIVE.md).

Summary of major milestones:

| Former Version | Date | Key Achievement |
|----------------|------|-----------------|
| V1 | 2026-04-19 | Modular DSP architecture established |
| V2 | 2026-04-19 | ADAA saturation + True Peak Limiter + first tests |
| V3 | 2026-04-19 | Design system + luxury UI |
| V4 | 2026-04-19 | Mathematical DSP overhaul (LR4, soft-knee, Pade) |
| V5 | 2026-04-19 | Audit-driven bug fixes |
| V6 | 2026-04-19 | Macro wiring + sidechain HPF |
| V7 | 2026-04-19 | CLAP support + pluginval + release hardening |
| V8 | 2026-04-19 | API reconciliation + industry comparison |
| V9 | 2026-04-19 | 14 competitive improvements |
| V10 | 2026-05-20 | Complete UI/UX + DSP overhaul ("Greatest Plugin Edition") |
| V11 | 2026-05-20 | Senior-dev quality rewrite + Claude Code review fixes |
| V12 | 2026-05-21 | Ivory System design + CI/CD + final cleanup |

---

## Future Entries

All changes after this consolidation should be logged here following this format:

```markdown
## v1.0.x — Short Title (YYYY-MM-DD)

### Added
### Changed
### Fixed
### Removed
```
