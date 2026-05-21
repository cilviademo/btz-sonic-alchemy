# BTZ — Build & Test Status

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
