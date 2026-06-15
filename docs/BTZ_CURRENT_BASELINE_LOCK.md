# BTZ — Current Baseline Lock

This is a contract. Anything below the line must not regress without explicit
sign-off + a state-migration / changelog entry.

## Commit
- **Branch:** `claude/review-btz-compilation-aN5UD`
- **HEAD:** see `git log -1 --format='%h %s'` — last verified at the commit
  containing the v1.0.3 honesty pass (greyed-out neural / multiband entries).

## What is verified at this commit
- **Builds:** Linux Standalone (12 MB) + VST3 `.so` link with JUCE 8.0.6
  (FetchContent). macOS/Windows build scripts present and configured the same way.
- **Tests:** 86/86 GoogleTest unit tests pass.
- **Integration:** 10/10 `BTZProcessorCheck` scenarios pass (wet-≠-dry,
  bypass, state round-trip, SR×blocksize matrix, oversized-block guard,
  every factory preset, parameter unit display, Target Lock readout,
  default patch, loudness-matched bypass).
- **Sanitizers:** AddressSanitizer + UndefinedBehaviorSanitizer + LeakSanitizer
  run clean across the full DSP suite (0 findings).
- **CPU (measured, 48 kHz, 512-sample blocks, single core):**
  Eco default 0.75 % · Standard default 1.03 % · Ultra+everything-on 3.73 %.

## Invariants — must not regress
1. **Parameter IDs are a public ABI.** Renames require state migration logic
   keyed off `BTZDsp::kStateVersion`. See `PARAMETER_MANIFEST.md`.
2. **`processBlock` stays RT-safe:** no allocations, no locks, no UI work,
   denormals flushed (`ScopedNoDenormals` + FTZ/DAZ).
3. **Block-rate smoothers stay advanced.** `sMix`, `sGlue`, `sShine`,
   `sShineFreq`, `sShineQ`, `sWidth`, `sMotion`, `sTransientMix`, `sBoom`
   are advanced via `SmoothParam::advanceBlock(numSamples)` once per block.
   The bug they fix: a frozen smoother makes `Mix` stick at 0 (= fully dry —
   the plugin does nothing).
4. **`fastTanh` stays bounded** (input clamped, output ∈ [-1, 1]).
5. **`TruePeakLimiter` true-peak readout** in `meters.truePeak` comes from the
   limiter's 4× ISP detector (`truePeakHold`), not the loudness meter's
   sample peak. The footer label "dBTP" therefore is honest.
6. **`SafetyLayer` magnitude clamp** at ±4.0 (~+12 dBFS) is intentional.
7. **`LinkwitzRileyCrossover` is real LR4** (two cascaded Butterworth, TDF-II).
   The empirical allpass test verifies it across 50 Hz–16 kHz.
8. **CI gates pluginval @ strictness 10** with no `|| true` bypass.

## Open product risks (already documented elsewhere)
- **Legal:** JUCE 8 license (GPL vs commercial) and trademark search on
  "Box Tone Zone" are unresolved → see `BTZ_RELEASE_BLUEPRINT_AUDIT.md` §0.
- **DAW validation:** pluginval + Reaper/Logic/Live smoke tests have not been
  run on the patched binary (needs a real machine).
- **Inert features still in code** (reserved for v1.1): `MultibandEngine`
  prepared but not routed; neural saturation slots fall back to `fastTanh`.
  The honesty pass greys these out in the UI but does not remove the params
  (preset compatibility).
- **Accessibility:** no `juce::AccessibilityHandler` usage yet.

## Companion docs (this commit and prior)
- `docs/dev/BTZ_RELEASE_BLUEPRINT_AUDIT.md` — full 14-phase compliance audit
- `docs/dev/PARAMETER_MANIFEST.md` — every parameter's ABI contract
- `docs/dev/BTZ_PERF_SECURITY_AUDIT.md` — measured CPU + ASan + security review
- `docs/dev/BTZ_BUILD_AND_TEST_STATUS.md` — what's actually verified
- `docs/BTZ_MASTER_GUIDE.md` — signal flow + control reference
- `docs/BTZ_CHANGELOG.md` — Keep-a-Changelog history

Update this file (and only this file's "Commit" + "What is verified" sections)
when a new validated baseline is reached.
