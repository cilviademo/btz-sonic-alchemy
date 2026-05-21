# BTZ Sonic Alchemy — Senior-Dev Audit (v1.0.1)

**Audited commit:** `9f6c415` (branch `baseline/v1.0-consolidation`, tag `v1.0.1`)
**Scope:** full repo — JUCE plugin (`btz-sonic-alchemy-main/BTZ/`), CI, tests,
docs, and the web/React mockup tree (`btz-sonic-alchemy-main/src/`).
**Method:** full read of all C++ sources, CMake, CI workflow, tests; cross-check
against the Manus changelog. No build was run (JUCE is fetched at configure
time and is not present in this environment) — compile findings are from static
analysis of symbol definitions vs. uses.

---

## 0. Verdict

The v1.0.1 work made **real, correct DSP improvements** (true-peak ISP detection,
BS.1770 K-weighting, a genuine LR4 crossover, the Target Lock engine, and a fully
parameterised Glue compressor). The processor (`PluginProcessor.cpp`) and the DSP
header (`BTZDsp.h`) are now internally consistent and almost certainly compile.

**But the plugin still does not build.** The blocker has moved from the DSP layer
(fixed) to the **UI layer**: `PluginEditor.cpp` was written against an "Ivory
System" theme/component API that was never actually committed to `BTZTheme.h` /
`BTZComponents.h`. There are ~60 compile-error sites in that one file. The
changelog's own "Known Issues" admits: *"Plugin has not yet achieved a successful
first compile."* That is still true.

Priority shape: **fix the editor↔theme desync first (P0), then the wiring gaps,
then the polish.** Everything else is blocked behind a green build.

---

## 1. BUILD BLOCKERS (P0 — `PluginEditor.cpp` will not compile)

All of these are isolated to `PluginEditor.cpp`. `BTZComponents.h` uses the theme
correctly, which proves the editor is simply out of sync with the committed theme.

### 1.1 ~45 undefined `palette::` tokens
`PluginEditor.cpp` references an Ivory palette that `BTZTheme.h` does not define:

| Used in editor | Exists in theme? | Closest actual token |
|---|---|---|
| `palette::charcoal` | ✗ | `palette::ink` |
| `palette::ivory` | ✗ | `palette::canvas` |
| `palette::linen` | ✗ | `palette::surfaceAlt` |
| `palette::porcelain` | ✗ | `palette::surface` |
| `palette::panelBorder` | ✗ | `palette::border` |
| `palette::sand` | ✗ | `palette::sageFaint` / `orangeFaint` |
| `palette::softBlack` | ✗ | `palette::inkStrong` |
| `palette::warmGray` | ✗ | `palette::inkMuted` |

The committed theme palette is the older scheme (`canvas, surface, surfaceAlt,
well, border, ink, inkMuted, inkFaint, inkStrong, sage, orange, clay, gold, teal,
amber …`). The Ivory names appear **only in comments** in `BTZTheme.h`.

**Fix options:** (a) add the 8 Ivory aliases to `palette::` as `inline constexpr
uint32_t` mapping to the intended colours, or (b) rewrite the 45 call sites to use
the existing tokens. Option (a) is faster and lower-risk; option (b) is cleaner
long-term. Recommend (a) now, (b) later.

### 1.2 `type::label()` / `type::brand()` called as functions
`BTZTheme.h` defines `type::label`, `type::brand`, `type::body`, `type::micro` as
`constexpr float` **sizes**, and only `type::sans()`, `type::mono()`,
`type::display()` as font-family functions. The editor calls `type::label()` (×5)
and `type::brand()` (×1) as if they were family functions.

**Fix:** replace `type::label()` → `type::sans()` and `type::brand()` →
`type::display()` for the family argument.

### 1.3 `type::*Size` constants don't exist
Editor uses `type::brandSize`, `type::labelSize`, `type::bodySize`,
`type::microSize` (×7 total). The theme defines these without the `Size` suffix:
`type::brand`, `type::label`, `type::body`, `type::micro`.

**Fix:** drop the `Size` suffix at all 7 sites, e.g.
`juce::Font(type::sans(), type::labelSize, …)` → `juce::Font(type::sans(),
type::label, …)`.

### 1.4 `SafetyIndicator::Level` / `setLevel` don't exist
`btz::SafetyIndicator` (BTZComponents.h:593) exposes `enum class State { Safe,
Caution, Warning }` and `void setState(State, const juce::String& = {})`. The
editor calls `safetyTruePeak.setLevel(SafetyIndicator::Level::Warning)` (×7,
PluginEditor.cpp:405-415). There is no `Level` enum and no `setLevel(enum)` on
that class (the `setLevel(float)` that exists belongs to the unrelated `MeterBar`).

**Fix:** change the 7 sites to `setState(SafetyIndicator::State::…)`.

### Root cause
`PluginEditor.cpp` is the "Ivory System" rewrite; `BTZTheme.h`/`BTZComponents.h`
were not migrated to match. Because the project has never compiled, this was
never caught. **This is the single most important thing to fix.**

---

## 2. DSP CORRECTNESS — what's now right, what's still off

### 2.1 Fixed and verified correct
- **`ShineProcessor`** — now a proper RBJ peaking biquad in Transposed-Direct-Form-II
  (`BTZDsp.h:790-836`). Correct.
- **`SidechainHPF`** — now a true 1-pole HPF `y = coeff·(y₋₁ + x − x₋₁)`
  (`BTZDsp.h:599-625`). Correct.
- **`LinkwitzRileyCrossover`** — now a real LR4: two cascaded 2nd-order Butterworth
  sections for **both** LP and HP, TDF-II (`BTZDsp.h:838-923`). Correct topology;
  LP+HP sum to allpass (flat magnitude) as expected for LR4.
- **`GlueCompressor`** — now fully parameterised: attack/release/ratio/stereo-link,
  all surfaced as APVTS params and read per block (`BTZDsp.h:628-689`,
  `PluginProcessor.cpp:513-521`). This closes the biggest pro-credibility gap from
  the prior review.
- **`LoudnessMeter`** — now applies BS.1770-4 K-weighting (high-shelf @1681.97 Hz +
  2nd-order HPF @38.13 Hz) before the mean-square accumulation
  (`BTZDsp.h:1184-1328`). Coefficients match the spec. Honest to call this
  "K-weighted loudness."

### 2.2 Still not reference-grade (functional, but caveat before claiming spec compliance)
- **True-Peak limiter ISP detection** (`BTZDsp.h:691-784`): the 4× interpolation is
  real, but the `kPhaseCoeffs` are a crude 4-point Lagrange-ish approximation (phase 2
  is commented "approx"), not a proper polyphase FIR. Combined with only **8 samples
  of lookahead** (~0.17 ms @ 48 kHz) and **instant-attack** gain, it will still
  distort on fast transients. For a mastering-grade true-peak limiter you want a
  real polyphase FIR + 1.5–5 ms lookahead with a min-search envelope.
  **⚠ Changelog/code mismatch:** the changelog claims `kLookahead` was increased
  8→16; the code still reads `kLookahead = 8` (`BTZDsp.h:694`).
- **LoudnessMeter integrated/short-term**: `integrated` uses a leaky IIR
  (`integrated += 0.01·(momentary − integrated)`), not the BS.1770 gated mean
  (absolute −70 LUFS + relative −10 LU gating). Momentary uses a single 400 ms
  window rather than overlapping 400 ms blocks at 100 ms hop. Fine as a moving
  loudness readout; **don't claim full R128 integrated/LRA compliance.**
- **`LoudnessMeter::truePeak`** is still **sample-peak** (`BTZDsp.h:1244`), even
  though the limiter now does ISP. The footer reads "TRUE PEAK … dBTP" from this
  sample-peak value, so the displayed number under-reads real ISP. Wire the meter's
  truePeak from the limiter's ISP detector, or relabel.
- **Target Lock recombination** (`PluginProcessor.cpp:701-727`): when active it
  splits via two LR4 crossovers and recombines `low+mid+high`. LR4 band sums are
  allpass (flat magnitude, non-linear phase), so even at unity band gains the output
  is phase-shifted vs. input, and per-band gains create magnitude bumps at the
  200 Hz / 4 kHz crossovers. Acceptable for a slow corrector, but it's not
  transparent — note it in the manual.
- **Target Lock uses stale loudness**: it reads `loudnessMeter.momentary` at
  `processLinearPost` line 717, but `loudnessMeter.process()` for the current block
  runs later (line 762). So the corrector always acts on the previous block's
  momentary value (and momentary only updates every 400 ms). For a slow loudness
  lock this is tolerable, but it's a latency in the control loop worth documenting.

### 2.3 Saturation / circuit models (unchanged from prior review — still toy-grade)
- `WDFTubeStage` / `WDFTransformerStage` are heuristic shapers, not real WDF. The
  transformer integrator (`primaryFlux += in·0.01; *= 0.999`) still has no hard
  clamp — long sustained input can drift. Fine as "flavour" models; don't market
  them as circuit-accurate.
- **No ADAA anywhere.** All saturation is plain pointwise waveshaping; anti-aliasing
  relies entirely on JUCE oversampling. The changelog's pre-v1.0 history claims
  "ADAA saturation" (V2) — that is not in the current code.
- Neural models: `loadWeights()` now exists and parses a flat float array
  (`BTZDsp.h:387-404`), but **no model weight files exist**, so all 4 neural slots
  fall back to `fastTanh`. The 16-wide diagonal-recurrence GRU is also a heavy
  simplification of a real RTNeural GRU.

---

## 3. RT-SAFETY / THREADING

Mostly improved; remaining items are minor.

- ✅ `juce::ScopedNoDenormals` at top of `processBlock`; FTZ/DAZ set in ctor.
- ✅ `setLatencySamples` no longer called mid-block — deferred via
  `pendingLatency` atomic and applied at block start (`PluginProcessor.cpp:344-359`).
  (Note: the comment "safe from any thread" is optimistic; applying at block start
  is the right call regardless.)
- ✅ `multibandEngine.prepare()` is no longer called from the audio thread — now
  only `numBands` is set (`PluginProcessor.cpp:311-314`). Good fix.
- ⚠ **MIDI learn still touches `juce::String` on the audio thread**
  (`PluginProcessor.cpp:783-788`, `addMapping` copies a `juce::String`). A `String`
  copy is a refcount bump (no alloc when shared), so it's *usually* RT-safe, but the
  pattern is fragile. Preferred: push the learned CC number through a lock-free FIFO
  and resolve the parameter-ID string on the message thread.
- ⚠ Per-block `recalcCoeffs()` in `GlueCompressor` (2× `exp`) and `ShineProcessor`
  (`pow`+`sin`+`cos`) whenever those stages are active. Cheap, but only needs to run
  when the relevant smoothed params actually changed.
- ⚠ `lfoModSources[i].prepare()` is called every block in `processNonlinear`
  (`PluginProcessor.cpp:553-554`) — harmless (only sets the increment) but should be
  a `setRate()` not a `prepare()`.

---

## 4. DEAD / UNWIRED CODE (compiles, does nothing)

These modules are constructed and `prepare`d but never affect the output. They
inflate the binary and mislead readers. Either wire them in or delete them.

| Module | State | Evidence |
|---|---|---|
| `MultibandEngine` | **Dead.** `numBands` is set, but `split()`/`recombine()` are never called in `processNonlinear`. The `useMultiband`, `bandL[]`, `bandR[]` locals (PluginProcessor.cpp:557-559) are computed-but-unused. | The `multibandCount` param controls nothing. |
| `OversamplingEngine` (bespoke) | **Dead.** JUCE's `dsp::Oversampling` is used instead. | Never called in process path. |
| `ReferenceToneMatcher` (`toneMatcher`) | **Dead.** `toneMatchAmt` is smoothed (`sToneMatchAmount`) but never applied. | No call to `computeCorrection`/apply. |
| `PresetIntelligence` | **Dead.** Never instantiated in process path. | — |
| `SpectrumBuffer` | **Write-only.** `pushSample()` is called but no FFT is ever computed and the editor never reads it. | Spectrum displays render empty (see §5). |
| `GainReductionHistory` (`grHistory`) | **Write-only.** `push()` called; `getAt()` never read (the editor's `grRibbon` keeps its own ring buffer fed by `meters.grDb`). | — |
| Macro knobs (`macro0..3`) | **Inert.** `sMacro0..3` are smoothed but never read in processing. | No macro→param routing. |
| `simpleMode`, `simpleDrive/Tone/Output`, `lfoCount` UI | partially inert | `simpleMode`/`simpleDrive`/`simpleTone`/`simpleOutput` params exist but Simple-mode UI drives `drive`/`shine`/`master` directly instead. |
| `resetAll()` | **Never called**, and `AudioProcessor::reset()` is **not overridden**. Hosts that call `reset()` between plays won't clear BTZ filter/envelope state (e.g. `hpStateL`, `tapeStateL`, `sideLowState`). | PluginProcessor.cpp:317; no override in header. |

---

## 5. UI ↔ DSP WIRING GAPS (compiles once §1 is fixed, but features are hollow)

- **Visualizers are never fed data.** `harmonicViz` (`HarmonicVisualizer`),
  `spectrumDisplay` (`SpectrumDisplay`), and `spectrumAdvanced`
  (`DirectManipSpectrum`) are positioned and shown, but the editor never calls
  `setMagnitudes()` / `setData()` / `setSpectrum()`. They render empty frames. The
  processor's `SpectrumBuffer` accumulates samples but never runs an FFT, and the
  editor never reads it. **The spectrum analyzer and harmonic display are currently
  decorative.**
- **Correlation meter computed but not shown.** The processor computes
  `meters.correlation` every block (`PluginProcessor.cpp:459-468`), and a
  `CorrelationMeter` component exists in `BTZComponents.h` — but the editor uses a
  `SafetyIndicator` for correlation and **hardcodes it to `Safe`**
  (`PluginEditor.cpp:414-415`). Wire the real value through, ideally via the unused
  `CorrelationMeter`.
- **Target Lock text fields don't reflect later changes.** `syncTargetLockFromAPVTS()`
  runs once at construction. Preset loads / host automation of `targetLUFS` etc.
  won't update the displayed text. Add an `APVTS::Listener` (or a Timer poll) to
  refresh the editors.
- **Simple-mode feedback loop risk.** `timerCallback` writes `simpleKnob*.setValue`
  from `kDrive/kShine/kMaster` every frame (PluginEditor.cpp:388-392) while the same
  knobs also push to those params via `onValueChange`. Guard against the timer
  fighting an in-progress drag.
- **`btnAutoGain` exists** and is attached to `autoGain` — good. Verify it's laid out
  in all three view modes (only placed in the header).

---

## 6. BUILD SYSTEM & CI

- ✅ `CMakeLists.txt` project version is `1.0.0`; JUCE pinned to `8.0.6`.
- ✅ CI builds VST3/AU/Standalone on macOS-14, windows-latest, ubuntu-22.04; runs
  ctest; uploads artifacts.
- ⚠ **CI will currently fail at the Build step** because `PluginEditor.cpp` doesn't
  compile (§1). The pipeline is real but red.
- ⚠ **pluginval is non-gating.** The validate job runs at `--strictness-level 5`
  and ends with `|| true` (`ci.yml:168`), so validation failures never fail CI.
  Once the build is green, raise to 10 and drop the `|| true`.
- ⚠ **Artifact paths likely wrong on macOS/Linux.** With Ninja (single-config),
  JUCE writes to `BTZ_artefacts/VST3/`, not `BTZ_artefacts/Release/VST3/`. The
  upload globs include the `Release/` segment, so macOS/Linux artifacts may upload
  empty. (Windows MSVC multi-config does use `Release/`.)
- ⚠ CI triggers on `main`, `overhaul/**`, `feature/**`, `claude/**` — **not** on
  `baseline/**`, so the v1.0 baseline branch itself doesn't run CI.
- ⚠ `clap-juce-extensions` is pinned to `main` (`CMakeLists.txt:51`) — a moving
  target. Pin to a tag/SHA for reproducible local/release builds (CI disables CLAP
  so it doesn't see this).
- ✗ No code-signing / notarization / installer steps (macOS notarization is
  required for the plugin to load in modern Logic; Windows wants Authenticode).

---

## 7. TESTING

- 84 GoogleTest cases; the v1.0.1 additions (TargetLock ×3, TruePeakLimiter ISP,
  K-weighting, LR4 flat-sum) are present and reference the real APIs.
- The earlier v11 test/API mismatches (`env.envelope`, `lfo.next()`,
  `MacroInterpreter::Curve`, `AutoGainSmoother::updateInput(float)`) were resolved —
  the DSP header even added `LFO::next()` and single-arg `updateInput/Output`
  aliases specifically for the tests.
- Gaps: tests cover DSP modules only. There is **no test that instantiates the
  `AudioProcessor`** (process a block, save/restore state, change sample rate,
  bypass) — which is exactly the class of bug pluginval catches. Add a minimal
  processor-level harness once it compiles.
- No golden-file regression test (hash a known input→output) to catch silent DSP
  drift.

---

## 8. MISSING FOR "INDUSTRY-GRADE" (beyond build)

From the changelog's own Known Issues plus this audit:

1. **No factory presets.** Pro plugins ship 100–300. Need a `.btzpreset` library
   across Drum Bus / Vocal / Mastering / Parallel / Creative, bundled and installed.
2. **No neural model weights.** 4 slots, all falling back to `tanh`. Either ship
   trained `.json`/weight files or hide the neural models until they exist.
3. **No code signing / notarization / installers.** Blocks distribution on macOS
   (Gatekeeper/Logic) and hurts Windows SmartScreen reputation.
4. **Accessibility:** no `AccessibilityHandler` roles, no keyboard navigation,
   no screen-reader labels. (Tooltips exist, which is a start.)
5. **Metering honesty:** relabel or fix the footer "TRUE PEAK … dBTP" (currently
   sample-peak); don't claim full R128 integrated/LRA.
6. **Manual / measurements:** the changelog mentions a user manual was added — once
   the DSP is final, publish frequency-response / THD / latency / TP-accuracy plots
   (reviewers want measurements; indies skip them).

---

## 9. CHANGELOG vs. CODE discrepancies (for your awareness)

- Changelog `kLookahead` 8→16: **code still says 8.**
- Changelog dates are reversed: v1.0.1 dated 2026-05-20, v1.0.0 dated 2026-05-21.
- Changelog pre-v1.0 history credits "ADAA saturation" (V2) and "LR4" (V4) — ADAA
  is not in the current code; LR4 only became real in v1.0.1.
- v1.0.0 "Known Issues" honestly lists: no successful compile, no neural weights,
  no presets, no signing. All four are still open.

---

## 10. PRIORITISED ACTION LIST

### P0 — make it compile (all in `PluginEditor.cpp` / theme)
1. Add the 8 missing Ivory palette aliases to `BTZTheme.h` **or** rewrite the 45
   `palette::` sites to existing tokens (§1.1).
2. Replace `type::label()`→`type::sans()`, `type::brand()`→`type::display()` (§1.2).
3. Drop the `Size` suffix on the 7 `type::*Size` references (§1.3).
4. Change the 7 `SafetyIndicator::Level`/`setLevel` sites to `State`/`setState` (§1.4).
5. Configure + build locally (or via CI) and iterate to a clean compile.

### P0 — correctness once it builds
6. Override `AudioProcessor::reset()` to call `resetAll()` (or delete `resetAll`).
7. Fix the footer true-peak readout (wire from limiter ISP, or relabel) (§2.2).
8. Decide on dead modules: wire multiband + spectrum FFT, or delete them (§4).

### P1 — wiring + pro-credibility
9. Feed the visualizers real data (FFT for spectrum, harmonic magnitudes) (§5).
10. Show the real correlation value via `CorrelationMeter` (§5).
11. Lengthen limiter lookahead (1.5–5 ms) + program-dependent release; upgrade ISP
    to a proper polyphase FIR (§2.2). Reconcile `kLookahead` with the changelog.
12. Add a processor-level test + a golden-file regression test (§7).
13. Raise CI pluginval to strictness 10, drop `|| true`, fix artifact paths, add
    `baseline/**` trigger, pin clap-juce-extensions (§6).

### P2 — product polish / differentiators
14. Factory preset library (§8.1).
15. Real neural weights or hide neural slots (§8.2).
16. ADAA on the saturation path (the one true anti-aliasing upgrade) (§2.3).
17. Code signing / notarization / installers (§8.3).
18. Accessibility pass (§8.4).
19. Macro→parameter modulation routing; reference-tone matching wired through (§4).

---

*Companion docs on this branch: `CODE_REVIEW_v11.md` (original compile/DSP review of
the v11 tree) and `ENHANCEMENT_BRIEF_GAP_ANALYSIS.md` (third-party brief vs. reality).
This audit supersedes both for anything concerning the v1.0.1 baseline.*
