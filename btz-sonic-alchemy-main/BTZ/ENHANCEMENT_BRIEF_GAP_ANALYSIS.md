# BTZ Enhancement Brief — Gap Analysis

**Companion document to** `CODE_REVIEW_v11.md`.
Branch: `overhaul/v1.1-dsp-architecture` @ `3905c4f`.

This document cross-references a third-party "Deep-Dive Enhancement
Brief" (drafted by another Claude session) against the actual state of
the BTZ source tree. The brief is well-organized but its baseline
assumption is wrong: it claims BTZ already has *"ADAA tanh saturation,
true-peak lookahead limiter, SVF crossover, Glue (SSL-style VCA)
compressor, macro system C++ port, pluginval + Steinberg VST3 validator
compliant."* From the actual code:

* **The plugin does not currently compile.**
* ADAA is not implemented anywhere.
* The "TruePeakLimiter" is sample-peak only (its own code comments
  admit this).
* The "LinkwitzRileyCrossover" is a 1-pole RC filter.
* The macro system is implemented as a dead class with no caller.
* pluginval has never been run against this build.

Treat the brief as a wish list, not an enhancement plan. The mapping
below shows, per brief item, what is actually present vs. partial
vs. missing.

## Legend

| Symbol | Meaning |
|---|---|
| ✓ | Present and correct |
| ◐ | Partial — scaffolding present, semantics missing |
| ✗ | Missing entirely |
| ⚠ | Present but broken / misnamed |
| 🚩 | Brief's premise about reality is wrong |

---

## §1 — Saturation Stage

| Brief item | Actual state | Verdict |
|---|---|---|
| 1.1 ADAA hardening | No ADAA anywhere. `Waveshaper::tanh/tube/tape/transistor/transformer` are plain pointwise shapers. JUCE oversampling handles aliasing the brute-force way. | 🚩✗ Brief is hardening something that doesn't exist. Implementing ADAA from scratch is itself P0 if you want it. |
| 1.2 Second-order ADAA / quality tiers | A `quality` parameter exists (Eco/Standard/High/Ultra) but only switches OS factor 1x/2x/4x/8x. No ADAA path. | ◐ Knob exists; semantics need real implementation. |
| 1.3 Nonlinearity menu expansion | 11 enum slots: Tanh, Tube, Tape, Transistor, Transformer, 4× Neural, WDF Tube, WDF Transformer. UI combo only lists 9. The neural slots fall back to `fastTanh` (no working weights loader). WDF slots produce sound but the models are toy-level. | ◐ More variety than the brief assumes, but most of it is hollow. The brief's additions (quadratic softclip, hard clip, asymmetric tanh, sin(atan(x))) are all worth having and all need analytic antiderivatives if going ADAA. |
| 1.4 DC blocker + polyphase FIR OS | `SafetyLayer` runs a DC block but with a non-standard formula (`y = x - dcPrev + coeff*dc`, `coeff ≈ 0.9999`) that likely lets DC through. JUCE OS is set to `filterHalfBandPolyphaseIIR` only — no FIR option. | ◐ Replace SafetyLayer DC stage with the standard `y[n] = x[n] − x[n-1] + 0.995·y[n-1]` form; add FIR OS mode for Insane tier. |

**Net:** §1 is mostly new work, not refinement.

---

## §2 — True-Peak Lookahead Limiter

| Brief item | Actual state | Verdict |
|---|---|---|
| 2.1 BS.1770 compliance | `TruePeakLimiter` peak detector is `max(\|l\|, \|r\|)` on the un-oversampled signal. Code comments in `LoudnessMeter::process` admit "True peak (sample-level — for proper ISP, needs 4x oversampling)". | 🚩⚠ Currently false advertising. Either implement 4× ISP detection (polyphase FIR on the sidechain) or rename to "Peak Limiter". |
| 2.2 Ceiling defaults / dBTP meter | Default ceiling = -0.3 dB. No streaming presets, no dBTP-vs-dBFS unit switch. Meter readouts are `gainToDb` on sample peak. | ◐ Defaults are reasonable; presets and unit-switching missing. |
| 2.3 Lookahead 1.5–5 ms | `kLookahead = 8` samples. At 48 kHz that's 0.17 ms. | 🚩✗ An order of magnitude too short. Lookahead at 8 samples can't catch a wide-band transient. Buffer needs to be allocated in `prepare`, sized to `maxLookaheadMs × sr/1000`. |
| 2.4 Dual-stage / auto release | Single exponential release with fixed `releaseCoeff` (50 ms). No program-dependence. | ✗ Missing. |
| 2.5 ISR + dither | Not present. | ✗ Missing. |

**Net:** the limiter needs the most surgery of any module — including the marketing name.

---

## §3 — SVF Crossover

The brief asks for LR4 alignment as P0. The code's `LinkwitzRileyCrossover`
is **not Linkwitz–Riley at any order** — it's a single one-pole LP
with `high = in - lp`. The brief's white-noise-sum reconstruction test
would fail immediately.

| Brief item | Actual state | Verdict |
|---|---|---|
| 3.1 Linear-phase mode | No FIR crossover. | ✗ |
| 3.2 LR4 alignment | 1-pole RC with complementary HP, mislabeled. | ⚠ Critical — replace with `juce::dsp::LinkwitzRileyFilter`. |
| 3.3 12/24/48 dB/oct | One slope, and it's 6 dB/oct. | ✗ |

Also: the crossover is never invoked from `processBlock`. The multiband
engine is dead code.

---

## §4 — Glue Compressor

| Brief item | Actual state | Verdict |
|---|---|---|
| 4.1 Feedback detector topology | `GlueCompressor::processStereo(scL, scR)` takes sidechain as **input**, not output → feedforward. | ◐ Quick swap to feedback if that's the character you want, but the brief's "SSL is feedback" claim is partially wrong — SL 4000 G is feedback, modern UAD/SSL Native reissues vary, Cytomic The Glue is feedforward. Pick one and document. |
| 4.2 THAT 2181 VCA character | No VCA-style 2nd-harmonic injection. | ✗ |
| 4.3 Sweepable sidechain HPF | `SidechainHPF` exists but: (a) only 4 fixed positions (20/60/120/250 Hz) wired into a 0–3 param, (b) combo UI shows 5 options, (c) the filter formula isn't actually a 1-pole HPF. | ⚠ Triple bug. |
| 4.4 Mix knob | Global wet/dry only. No per-compressor parallel. | ◐ |
| 4.5 Attack / release menu | **Attack & release are hard-coded to 10 ms / 100 ms.** Not exposed as parameters. | 🚩✗ The brief assumes user control. There is none. This alone disqualifies BTZ from any pro context. |
| 4.6 Ratio set 1.5/2/4/10/∞ | Continuous, no stepping. `ratio = 2 + glueAmt × 4` clamped at min 1. | ◐ |
| 4.7 Stereo linking 0–100% | Hard-coded linked (`max(\|scL\|, \|scR\|)`). | ✗ |

**Brief is wrong about how much of the Glue compressor is real.**
Attack/release/ratio/link knobs are all simply missing.

---

## §5 — Cross-Cutting

| Brief item | Actual state | Verdict |
|---|---|---|
| 5.1 `ScopedNoDenormals` + FTZ/DAZ | `juce::ScopedNoDenormals` at top of `processBlock`. `enableFlushToZero()` in ctor sets FTZ/DAZ. | ✓ Already correct. |
| 5.2 SIMD | Zero use of `juce::dsp::SIMDRegister`. | ✗ Missing. (Caveat: the brief's "22% CPU reduction" figure is workload-specific — don't quote as a guarantee.) |
| 5.3 Sample-rate robustness | Coefficients are recomputed in `prepareToPlay`. | ✓ |
| 5.4 Buffer-size independence | No fixed-block assumptions in the audio path. | ✓ |
| 5.5 Parameter smoothing | `SmoothParam` used for most knobs, but ratio/attack/release/ceiling are never smoothed (they're not parameters yet). Mid/Side enable is a hard bool. | ◐ Fix once §4.5 is in. |

§5 is the strongest area in current code — solid RT-safety scaffolding,
just with broken DSP on top of it.

---

## §6 — Validator / Host Compatibility

| Brief item | Actual state | Verdict |
|---|---|---|
| 6.1 pluginval level 10 | `validate_vst3` cmake target exists, gated on `PLUGINVAL_PATH` env var. Never run in CI. Repo has no `.github/workflows/`. | ◐ Plumbing exists, automation doesn't. |
| 6.2 Steinberg validator | `validate_vst3_sdk` target gated on `VST3_VALIDATOR_PATH`. Same story. | ◐ |
| 6.3 State versioning | `kStateVersion = 11`, `migrateState()` exists but is empty. State serialization uses `copyXmlToBinary` correctly. | ◐ Versioning scaffold, no actual migration logic. |
| 6.4 AU `auval` | `validate_auvaltool` target exists. Same gating story. | ◐ |
| 6.5 Notarization + code-signing | No scripts, no certs, no Info.plist tweaks for hardened runtime. | ✗ |
| 6.6 Host matrix | No documentation, no test plan. | ✗ |

🚩 The brief claims BTZ is "pluginval + Steinberg VST3 validator
compliant." It is not. The plugin doesn't build.

---

## §7 — UX / GUI

| Brief item | Actual state | Verdict |
|---|---|---|
| 7.1 Resizable window | `ResizableCornerComponent` + `ComponentBoundsConstrainer` are set up. But constraints reference `layout::defaultW/maxW/defaultH/maxH` which don't exist in `BTZTheme.h` → won't compile. | ⚠ |
| 7.2 HiDPI / Retina | Editor uses vector paint code throughout (no PNG assets). JUCE handles DPI. Should be fine once it compiles. | ✓ |
| 7.3 Accessibility | Zero `AccessibilityHandler` usage. No keyboard navigation. No screen-reader labels. | ✗ |
| 7.4 Metering quality | `MeterBallistics` has peak + hold + RMS. `LoudnessMeter` claims EBU R128 but **has no K-weighting filter** — just dB-RMS over a 400 ms window. Will read 3–10 LU off real LUFS on real material. | 🚩⚠ "EBU R128 integrated/short-term/momentary" in the v11 summary is the same marketing inflation as the limiter naming. |
| 7.5 Visual feedback | `HarmonicVisualizer`, `SpectrumDisplay`, `DirectManipSpectrum`, `GainReductionRibbon` all exist. **`HarmonicVisualizer` is never fed data, `SpectrumDisplay` is never called with FFT bins, `DirectManipSpectrum` is never bound to crossover params, GR ribbon shows 192 samples ≈ 4 ms.** | ⚠ Widgets exist; data plumbing doesn't. |
| 7.6 Knob feel | Popup display on drag is enabled. Right-click menus, modifier-key sensitivity, double-click default — all default JUCE behavior with no customization for MIDI Learn / Copy/Paste. | ◐ |
| 7.7 Preset system | `UndoStack`, `ABState`, file-based preset I/O exist. `PresetBrowser` widget exists but `setPresets()` is never called → empty list. **Save button has no `onClick` handler.** No bundled factory presets. | ◐ Foundations there, surface is broken. |
| 7.8 Modulation / macros | `MacroInterpreter` is declared as a member, fully implemented in `BTZDsp.h`, and **never invoked anywhere.** Macro knobs 0–3 are smoothed but their values aren't read. | ⚠ Dead infrastructure. |

---

## §8 — Competitive Positioning

Pure marketing copy. No code implications. Two takes worth keeping:

* The brief's pitch — "saturation + bus comp + TP limiter in one stage"
  as the differentiator — is the right story. SSL-style all-in-one
  plugins (iZotope Neutron, FabFilter Pro-MB stacks) sell on
  integration. That's the wedge.
* "Publish measurements, not adjectives" is genuinely under-used by
  indies and cheap to do once the DSP is actually correct.

---

## §9 — Build / Release Infrastructure

| Brief item | Actual state | Verdict |
|---|---|---|
| 9.1 CI matrix | No GitHub Actions workflows. | ✗ |
| 9.2 Crash reporting | None. Brief mentions Sentry C++ — Crashpad/Breakpad more common in audio. | ✗ |
| 9.3 Telemetry | None, and that's probably the right answer for indie. | ✗ (intentional) |
| 9.4 Update mechanism | None. Sparkle/WinSparkle would be the path. | ✗ |

---

## What's missing entirely from the brief that BTZ also needs

The brief skips a few items that matter specifically for BTZ's
current state:

1. **A working build.** §6 mentions validators; none of it matters
   until the plugin compiles. The eight hard compile errors from
   `CODE_REVIEW_v11.md` are P-1 (before P0).
2. **Removing the dead modules** — `MultibandEngine`, the bespoke
   `OversamplingEngine` (duplicating JUCE's), `MacroInterpreter`, the
   LFO bank, and the unused neural slots. Dead code is a quality
   signal — keeping it makes the codebase look bloated to reviewers
   and is the first thing pluginval-strictness-10 surfaces via
   "phantom parameters."
3. **Fixing the EBU R128 claim or implementing K-weighting properly.**
   libebur128 (MIT) is the obvious dependency.
4. **Real preset content.** The brief mentions a preset browser but
   not that BTZ ships zero presets. Pro plugins ship 100–300. Drum
   bus, vocal bus, mastering, parallel, creative — 5 categories × 10
   presets is the minimum to look serious.
5. **Honest threading-model documentation.** The header claims
   "lock-free, no allocation on audio thread" but MIDI learn mutates
   `juce::String` from `processBlock`. Either fix it or stop claiming
   it.

---

## Where to push back on the brief

A few brief items are weaker than they sound; flag for review:

* **§1.4 polyphase FIR oversampling as "the Insane choice."** Cleaner
  phase, yes — but latency jumps from ~10 samples to 30–60+. For a
  real-time tracking plugin that matters. Not a clean win.
* **§4.1 SSL feedback topology.** The SL 4000 G original is feedback;
  the SL 9000 J and most modern reissues are feedforward; Cytomic The
  Glue is feedforward; Softube Bus Processor is configurable. The
  brief's framing makes it sound settled — it isn't. Pick a topology,
  A/B both, choose based on sound, not "authenticity."
* **§4.2 THAT 2181 character.** The 2181 is one VCA in the SSL
  lineage; dbx 202, 2150, Blackmer are others. A `VCA Drive` knob is
  fine; framing it as 2181-specific is overspecific.
* **§5.2 SIMD "22% CPU reduction."** That figure is from a specific
  JUCE tutorial on a specific filter. Real-world wins vary from 5% to
  4×. Don't quote the number publicly.
* **§7.3 European Accessibility Act enforcement** for audio plugins is
  legally untested in 2026. Do it for the right reasons — a meaningful
  share of engineers have visual or motor impairments — rather than
  for compliance fear.
* **§9.2 Sentry C++ SDK in audio plugins.** Sentry's C++ support
  exists but is rarely deployed in JUCE plugins. Crashpad (Chromium)
  and Backtrace are the more common picks. The minidump-from-the-
  audio-thread point still stands either way.

---

## Revised priority ordering (grounded in actual state)

The brief's priorities assume a working baseline. Real ordering:

### P-1 (precondition — nothing else matters until done)
1. Fix the compile errors documented in `CODE_REVIEW_v11.md`
2. Fix the editor's APVTS attachment ID mismatches
3. Either delete or wire up the dead modules

### P0 (correctness — what "ships" means)
4. Replace the fake biquad in `ShineProcessor`
5. Replace the fake HPF in `SidechainHPF`
6. Replace the fake LR crossover with `juce::dsp::LinkwitzRileyFilter`
7. Make the limiter actually do 4× true-peak detection, or rename it
8. Add K-weighting to the LUFS meter, or stop calling it EBU R128
9. Add user-controllable attack/release/ratio/link/mix to the Glue
   compressor
10. Fix the audio-thread `juce::String` mutation in MIDI learn
11. Move `setLatencySamples` out of the audio thread
12. Stand up a CI workflow that runs pluginval level 10 on every
    commit

### P1 (the brief's P0 items that depend on P0 above being done)
13. ADAA (1st-order) on the now-correct saturation path
14. Lookahead lengthening + dual-stage release
15. Macro system wired through to parameter modulation
16. Preset content (≥50 factory presets across 5 categories)
17. Accessibility handlers and keyboard navigation
18. Notarization / code-signing scripts

### P2 (the brief's P1+P2 — the polish that closes the gap with FabFilter/Softube)
19. 2nd-order ADAA, additional nonlinearity shapes
20. Linear-phase crossover mode
21. THAT-2181-style VCA character knob
22. ISR + dither
23. Live transfer-curve visualization
24. Update mechanism + crash reporter

---

## Cross-reference summary

| Brief P-tier | Items in brief | Actually feasible without P-1 fixes |
|---|---|---|
| P0 (must) | 18 items | 0 |
| P1 (strongly recommended) | 17 items | 0 |
| P2 (differentiator) | 3 items | 0 |

Every item in the brief is blocked behind the plugin compiling.
That is the entire shape of this report.

---

*End of gap analysis. Pair with `CODE_REVIEW_v11.md` for the
underlying compile-error and DSP-correctness detail.*
