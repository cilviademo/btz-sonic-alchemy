# BTZ Sonic Alchemy — Changelog

All notable changes to this project are documented in this file. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) principles, and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) starting from v1.0.

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
