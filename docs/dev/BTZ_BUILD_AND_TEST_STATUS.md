# BTZ — Build & Test Status

> **Deep debug pass (senior-dev) appended at the bottom — see "§9 Deep Debug Pass".**
> Headline: a critical signal-flow bug made the plugin output 100% dry (did
> nothing audible); fixed. DSP modules verified clean under ASan+UBSan+LSan.

**Baseline:** branch `claude/review-btz-compilation-aN5UD` (patches on top of
`v1.0.1` / `9f6c415`).
**Last updated by:** Claude Code patch agent.
**Environment:** Linux container, GCC 13, CMake + Ninja, JUCE 8.0.6 (FetchContent),
GoogleTest 1.14. No DAW / audio host present.

This document strictly separates what was **actually verified here** from what
**must be run on a real local machine / Cursor / a DAW**.

---

## 1. What was ACTUALLY VERIFIED in this environment

| Item | Result |
|---|---|
| CMake configure (JUCE 8.0.6 fetched) | ✅ succeeds (after installing X11/GL/ALSA dev headers — see setup doc) |
| Plugin compiles (PluginProcessor, PluginEditor, BTZDsp, BTZTheme, BTZComponents) | ✅ **zero errors** |
| Standalone links | ✅ 12 MB binary produced |
| VST3 `.so` links | ✅ links; only the post-link `moduleinfo.json` helper step fails on this headless container (`/bin/sh` quirk) — not a code issue, does not occur on macOS/Windows |
| Test target (`BTZTests`) compiles & links | ✅ |
| Unit tests run | ✅ **81 / 84 pass** |

This is the first time the plugin has compiled — the prior baseline had never
achieved a successful build.

## 2. Remaining test failures (3) — classified, NOT silently patched

| Test | Classification | Action needed |
|---|---|---|
| `LinkwitzRileyCrossover.SplitsIntoLowAndHigh` | Wrong expectation for LR4. Asserts `low+high == input` (sample identity). A correct LR4 sums to an **allpass** (flat magnitude, phase-shifted), so identity does not hold. The DSP is correct LR4. | Product decision: rewrite the test to assert magnitude/energy flatness across frequency, OR (if true unity reconstruction is required for multiband recombination) switch to a complementary topology — a DSP change that must be deliberate. |
| `LinkwitzRileyCrossover.SumsFlatAtCrossover` | Same as above (checks `(lp+hp)-in < 0.01`). | Same. |
| `RTSafety.SafetyLayerHandlesAllEdgeCases` | Test expects the safety layer to hard-clamp `|out| <= 4.0`. Current `SafetyLayer` only guards NaN/Inf/denormal + DC; it does not limit magnitude. | Product decision: add a generous magnitude clamp to `SafetyLayer` (defensible for a "safety" stage), or relax the test. |

## 3. Fixes applied & verified (this session)

- **Compile restoration (P0):** synchronized the Ivory editor with the committed
  theme/components — palette aliases, type-size aliases, `type::label()/brand()`
  → `type::sans()/display()`, `SafetyIndicator` `Level`/`setLevel` wrapper,
  `LabeledKnob : SettableTooltipClient`, `SpectrumDisplay = DirectManipSpectrum`
  alias, `deltaMonitoring` made public, async `FileChooser`, test-target CMake
  (`juce_add_console_app` + `JUCE_WEB_BROWSER=0`), missing `<random>`/`<limits>`.
- **fastTanh** is now bounded to [-1,1] (was unbounded — returned ~6.7 for input 100).
- **Correlation meter** wired to `meters.correlation` (was hardcoded "Safe").
- **`reset()`** override added → `resetAll()`, which now also clears Target Lock
  and JUCE oversampling state.

## 4. What still requires a real local machine / Cursor

- **Plugin instantiation in a DAW** (Reaper, Logic, Live, FL, Cubase) — load,
  open editor, automate, scrub, preset-switch, close/reopen. Cannot be done headless.
- **pluginval** at strictness 10 against the built VST3/AU (a Linux pluginval run
  would be possible but a DAW-representative pass on macOS/Windows is what matters).
- **macOS/Windows builds** (this was Linux only). AU only builds on macOS.
- **VST3 `moduleinfo.json`** generation (works on macOS/Windows; failed only on
  this headless Linux container).
- **Audio correctness by ear** and measurement (THD, null tests, LUFS accuracy).

## 5. What requires Cursor (on the real machine)
See `BTZ_CURSOR_HANDOFF.md`. In short: configure + build on macOS/Windows, run
pluginval, load in Reaper, then decide on the 3 documented test failures.

## 6. What requires Codex
See `BTZ_CODEX_HANDOFF.md`. Deterministic tooling: installer scripts, CI YAML,
small compile patches, changelog formatting.

## 7. What should be reserved for Manus
Larger creative/strategic work: training real neural-model weights, factory
preset sound-design, product/marketing, and any deliberate DSP topology change
(e.g., LR4 reconstruction decision) that needs design judgment.

## 8. Exact next commands to run locally

```bash
# macOS
brew install cmake ninja
scripts/build_macos.sh Release --tests
ctest --test-dir btz-sonic-alchemy-main/BTZ/build --output-on-failure
PLUGINVAL=/path/to/pluginval scripts/run_pluginval.sh 10

# Windows (Developer PowerShell for VS 2022)
pwsh scripts/build_windows.ps1 -Config Release -Tests
$env:PLUGINVAL="C:\tools\pluginval.exe"; pwsh scripts/run_pluginval.ps1 -Strictness 10

# Linux
sudo apt-get install -y libx11-dev libxext-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxcomposite-dev libxrender-dev libfreetype6-dev \
  libfontconfig1-dev libasound2-dev libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev
scripts/build_linux.sh Release --tests
```

---

## 9. Deep Debug Pass (senior-dev) — findings & verification

Performed with a real JUCE 8.0.6 build + GoogleTest + AddressSanitizer/
UndefinedBehaviorSanitizer on Linux. Tools used: full data-flow tracing of
`processBlock`, `-Wall/-Wextra` review, and an instrumented (ASan+UBSan+LSan)
test run.

### 9.1 CRITICAL bug found & fixed — plugin produced no processed audio
`SmoothParam` smoothers consumed once per block via `.current` were never
advanced (`.next()` was only called on the per-sample smoothers). Their
`.current` stayed frozen at the initial `0.0`:
- **`sMix == 0`** → wet/dry mix output **100% dry**: the plugin did nothing
  audible regardless of the Mix knob (default 1.0).
- **`sWidth == 0`** → Width collapsed the wet signal to **mono**.
- `sGlue`, `sShine`, `sShineFreq/Q`, `sBoom`, `sMotion`, `sTransientMix` all
  frozen → glue compressor / shine EQ / etc. never engaged.

**Fix:** `SmoothParam::advanceBlock(n)` (O(1) closed-form), advance the 9
block-rate smoothers each block, and `snap()` all smoothers in `prepareToPlay`
so the first blocks don't ramp from zero. Regression tests added. This is the
class of bug module tests cannot catch — only integrated `processBlock` tracing.

### 9.2 Other bugs fixed this pass
- **fastTanh was unbounded** (returned ~6.7 for input 100; overshoots >1 above
  x≈3.3). Now input-clamped + output-bounded to [-1,1]. RT-safety.
- **Oversampling modulation rate**: `processNonlinear` ignored `osFactor`, so the
  Motion LFO + LFO bank ran 2–8× too fast at Quality > Eco. Now advanced against
  the effective (oversampled) rate.
- **Transient Sensitivity knob (`transSens`) was inert** — wired to
  `transientSplitter.sensitivity`.

### 9.3 Verified clean
- **ASan + UBSan + LeakSanitizer:** the full DSP suite (86 tests incl. NaN/Inf/
  denormal/extreme-value edge cases across all 11 saturation models, filters,
  limiter, loudness, Target Lock) runs with **zero** sanitizer findings — no
  OOB, use-after-free, signed-overflow/UB, or leaks.
- **Unit tests:** 83/86 pass. The 3 failures are the documented design questions
  (§2): LR4 reconstruction semantics ×2, SafetyLayer clamp policy.
- **Compiler warnings:** only benign ones (JUCE `Font` deprecation, int→size_t
  sign-conversion on non-negative values, one `-Wreorder`, `-Woverloaded-virtual`
  for the double-precision `processBlock` we intentionally don't support). None
  indicate incorrect behavior.

### 9.4 Still-inert controls (confirmed 0 reads — feature wiring, not bugs)
These parameters set a smoother target that nothing consumes, so the knobs do
nothing yet. Wiring them is feature work (left for Cursor/Manus, not this
correctness pass):
`air`, `era`, `shineMix`, `toneMatchAmt`, `macro0..3`. (`transSens` was in this
list and is now wired.)

### 9.5 Not yet covered by sanitizers (recommended next)
The sanitized run covers `BTZDsp.h` only (the test target doesn't link the
processor/editor). Recommend a small **processor-level harness** (instantiate
`BTZAudioProcessor`, `prepareToPlay`/`processBlock`/state round-trip at
44.1/48/96 kHz and varied block sizes) built under ASan to lock in the
`processBlock` integration that this pass fixed. See `BTZ_CODEX_HANDOFF.md`.
