# BTZ Implementation Roadmap (Impact vs Effort)

## P0 - Critical (Do First)

1. **Build robustness**
   - Keep JUCE discovery via env fallback and script verification.
   - Impact: high, Effort: low.

2. **Metering correctness**
   - Validate RMS/peak/GR scaling at multiple sample rates.
   - Impact: high, Effort: medium.

3. **State and automation stability**
   - Verify APVTS recall and DAW automation for all parameters.
   - Impact: high, Effort: medium.

## P1 - Sonic Quality Upgrades

4. **Nonlinear block isolation**
   - Expand oversampling isolation around strongest nonlinear segments.
   - Impact: high, Effort: medium.

5. **True-peak behavior hardening**
   - Add ISP-safe check strategy and conservative default ceiling guidance.
   - Impact: high, Effort: medium.

6. **Mono-safe controls**
   - Optional Mono Safe user toggle while preserving current compatibility.
   - Impact: medium-high, Effort: low-medium.

## P2 - Competitive Differentiation

7. **Adaptive macro layer**
   - Bounded program-aware adaptation using lightweight feature extraction.
   - Impact: medium-high, Effort: medium-high.

8. **Enhanced visual diagnostics**
   - Add compact spectrum and history overlays with strict GUI-thread rendering.
   - Impact: medium, Effort: medium.

9. **Preset intelligence**
   - Add workflow-tagged presets (drum bus, punch, loud-safe, mono-safe).
   - Impact: medium, Effort: low-medium.

## P3 - Advanced R&D

10. **Hybrid DSP/ML experiments**
    - Offline-trained suggestion engine, runtime bounded correction only.
    - Impact: medium, Effort: high.

## Acceptance Criteria Pattern (for each task)

- Build: Debug + Release pass.
- Runtime: no audio-thread allocation/lock regressions.
- Host: no automation/state regressions.
- Sonic: measurable deltas documented in `docs/Measurements.md`.
