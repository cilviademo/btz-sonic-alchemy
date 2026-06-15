# BTZ — Session Handoff (Sandbox + Repository State)

**Generated:** 2026-06-15
**Repo:** `cilviademo/btz-sonic-alchemy`
**Active branch:** `main` (also pushed to `claude/review-btz-compilation-aN5UD` for history)
**HEAD:** **`45a07c8`** (the v1.0.6 release: drive level honesty + ADAA on Tube)
**Predecessor squash:** `31f8029` (v1.0.4 baseline; 54 commits squashed)

This is the single doc to read first if you're picking up the work. It links to
everything else.

---

## 1. The 60-second summary

BTZ Sonic Alchemy went from "compiles but Mix did nothing" to a **measurable,
sanitizer-clean, RT-safe production baseline with two real DSP wins** in this
sandbox session:

- **86 unit tests + 16 integration tests pass** at HEAD.
- **ADAA (antiderivative anti-aliasing) on the default Tube saturator** — alias
  floor went from −56 dB to −66 dB at Eco (mastering-grade at the cheapest tier).
- **Drive level honesty restored** — sweep was −17 dB across the range; now
  ±1.12 dB. Locked down by a regression test.
- **ASan + UBSan + LeakSanitizer report zero findings** across the DSP suite.
- **CPU: 0.75 % → 3.73 %** single core (default → Ultra + everything on).
- **Windows build/install runbook** captured from the actual workstation pass,
  with the two script bugs (parens-as-block + VS 2026 lookup) fixed.

The remaining gap to ship is **operational/legal/listening**, not engineering:
JUCE commercial license, trademark search, DAW smoke tests on Reaper / Logic,
installers, signing/notarization, and an ear pass on the saturation character.

---

## 2. Branch + commit landscape

### Active

| Ref | What | State |
|---|---|---|
| `main` (remote) | the canonical line — squashed v1.0.4 baseline + the v1.0.5/v1.0.6 DSP improvements | **HEAD `45a07c8`** |
| `claude/review-btz-compilation-aN5UD` (remote) | full unsquashed 54-commit history kept for audit/blame | parked |

### Older / historical (do not develop on)

| Ref | Notes |
|---|---|
| `baseline/v1.0-consolidation` | the older v1.0.0 / v1.0.1 baseline that this session's work supersedes |
| `overhaul/v1.1-dsp-architecture` | the "v12 / Ivory" rewrite that became codebase #3 (now folded into main) |
| `audit/v1.0-baseline` | local-only workspace branch from this session; not on remote |

### Tags

- Remote: `v1.0.0`, `v1.0.1` (older baselines).
- **Local-only tags `v1.0.4`, `v1.0.5`, `v1.0.6`** — the proxy on this sandbox
  rejected every tag push for the session (network quirk, not code). Push from
  your workstation:

```cmd
git fetch origin
git tag v1.0.4 31f8029
git tag v1.0.5 3ea568e
git tag v1.0.6 45a07c8
git push origin v1.0.4 v1.0.5 v1.0.6
```

### The three-codebase reconciliation

A recurring confusion in prior handoffs: there were **three** BTZ codebases under one name. The unified state-of-truth doc (`docs/BTZ_UNIFIED_STATE_OF_TRUTH.md` §1) reconciles them:

| # | Codebase | Status |
|---|---|---|
| 1 | `BTZ_JUCE/` (29-module) | Out of this repo. Not active. |
| 2 | Simplified 20-param shaper (older `btz-sonic-alchemy-main/BTZ/`) | The one the Master Handoff described as "Mix dead, Master dead" in Reaper. **Superseded** by #3 in main. |
| 3 | **v12 / Ivory rewrite (this branch)** | The current `main`. ~50 parameters, 1700-line `BTZDsp.h`, 86/86 tests. Has the macro names of #2 plus Target Lock, LR4, K-weighted loudness, ADAA, etc. |

The Master Handoff's reports of broken-in-Reaper behavior are about **codebase #2 on the workstation**, not what's in `main` now.

---

## 3. What this session delivered (chronological)

### Phase A — Compile restoration (Apr-May session block)
- Found and fixed the v11 catastrophic compile blockers: undefined `palette::*`/`type::*` symbols, `SafetyIndicator` API drift, missing `NeuralSaturationModel::loadWeights`, rvalue-to-`OutputStream&` binding error, `kMaxLFOs` undeclared, attachment ID mismatches.
- Documented in `CODE_REVIEW_v11.md` and `ENHANCEMENT_BRIEF_GAP_ANALYSIS.md`.

### Phase B — DSP correctness pass
- **`fastTanh` bounded** (was unbounded; returned ~6.7 for input 100 — RT-safety hazard).
- **Smoother fix**: 9 block-rate smoothers (`sMix`, `sGlue`, `sShine`, `sShineFreq`, `sShineQ`, `sWidth`, `sMotion`, `sTransientMix`, `sBoom`) were never advanced → `sMix` stuck at 0 → plugin output 100% dry. Added `SmoothParam::advanceBlock`.
- **`snap()` all smoothers in `prepareToPlay`** — no first-block ramp-from-zero.
- `SafetyLayer` now clamps ±4 (~+12 dBFS) on top of NaN/Inf/DC guards.
- `ShineProcessor` switched to a correct TDF-II RBJ biquad.
- `SidechainHPF` switched to a real 1-pole HPF.
- `LinkwitzRileyCrossover` verified to be a real LR4 (allpass test passes).
- `TruePeakLimiter` cached `dbToGain(ceiling)`; truePeak meter reads the real 4× ISP detector.
- `AudioProcessor::reset()` override added, calling a complete `resetAll()`.
- Oversampling-aware modulation rate in `processNonlinear`.

### Phase C — Performance audit
- Documented in `docs/dev/BTZ_PERF_SECURITY_AUDIT.md`.
- Replaced per-sample `dbToGain` (master) with a per-block linear gain ramp.
- Collapsed `ResonanceTamer`'s 32-band redundant loop to a single computation
  (bit-identical, ~32× cheaper).
- `Shine recalcCoeffs` early-out cache.
- Removed `os16x` dead code.
- Final CPU: 0.75 % → 3.73 % single core.

### Phase D — Security review
- ASan + UBSan + LeakSanitizer integration; **zero findings** across DSP suite.
- Defensive guard for oversized-block contract violation (re-prepare instead of OOB).

### Phase E — Real units everywhere
- Parameter-layer `stringFromValue` / `valueFromString` for every visible knob.
  Units (dB / Hz / ms / % / ratio / LUFS / Q) now show in both BTZ's knob popups
  and the host's automation lanes / generic editor.

### Phase F — Product identity work
- 16 curated factory presets across 7 categories. Mastering bank features Target Lock.
- Target Lock live readout in the footer (LOCKED / approaching state).
- Default patch nudged to "instant good" subtle warm glue.
- Loudness-matched bypass implemented (opt-in; tracks processed/dry RMS ratio).
- Greyed-out neural saturation entries + 2-6 band multiband options (honesty pass).
- All identity docs: `BTZ_PRODUCT_IDENTITY.md`, `BTZ_FORMAT_ROADMAP.md`,
  `BTZ_RELEASE_READINESS_SCORECARD.md`, `BTZ_RELEASE_BLUEPRINT_AUDIT.md`.

### Phase G — Behavior test suite (the test gap)
- Added 15 integration assertions to `BTZProcessorCheck`:
  - Wet path active (regression guard for the dry-output bug).
  - Bypass passthrough.
  - State save/restore round-trip.
  - Sample-rate × block-size matrix (44.1 / 48 / 96 kHz × 32 / 128 / 512 / 1024).
  - Oversized-block defensive guard.
  - Factory preset validation.
  - Parameter unit display + type-from-string round-trip.
  - Target Lock readout publishes when engaged.
  - Default patch applies audible character.
  - Loudness-matched bypass works.
  - **Mix law** (mix=0 → dry, mix=0.5 → blend, mix=1 → wet active).
  - **Master law** (−6.00 / +6.00 dB at master 0.45 / 0.95).
  - Macro sweep stays finite + bounded for every macro.
  - **Editor attachment ID coherence** (catches the v11 catastrophe class).
  - **Target Lock convergence**.

### Phase H — Phase-0 measurement baseline
- Added `BTZMeasure` console-app target (in `bench/measure_main.cpp`).
- Five measurements: control-to-DSP truth audit, oversampling reality + latency,
  aliasing per saturation model, loudness honesty across macros, true-peak honesty.
- Output committed at `docs/measurements/BTZ_BASELINE.md`.

### Phase I — v1.0.5: ADAA on the Tanh saturator
- First-order Antiderivative Anti-Aliasing (Parker / Esqueda / Bilbao 2016).
  Closed form: `y[n] = (log cosh x[n] − log cosh x[n−1]) / (x[n] − x[n−1])`,
  with midpoint-tanh fall-back at the 0/0 ill-conditioning.
- Per-channel state (`adaaTanhL`, `adaaTanhR`).
- Measured: Tanh alias floor at Eco improved from Tube-class (~−56 dB) to **−66 dB**.

### Phase J — v1.0.6: drive level honesty + ADAA on Tube
- Removed the buggy `invDrive = 1/driveGain` post-saturation normalize
  (which pulled level *down* as drive grew on any compressive shaper).
- New makeup curve: `makeup_dB = −12·√drive`, empirically fit and verified
  by a new integration test §16. Drive sweep is now **within ±1.12 dB** of
  dry RMS across the full range with autoGain OFF + Master unity.
- Extended ADAA to the Tube model (the default). Tube antiderivative:
  `F = log cosh(x+0.2) + 0.1875·log cosh(0.8x+0.5) − C·x`.
- Tube alias floor at Eco improved from **−56.4 dB → −66.5 dB** (default sound is now mastering-grade at Eco).

### Phase K — Windows build + install runbook
- Field-validated from a real workstation pass.
- Two script fixes: `scripts/install_vst3_windows.bat` now uses
  `if exist "%VST3_SRC%\"` instead of `\.` (the `(BTZ)` parens
  broke cmd's block parsing). `scripts/build_windows_with_vs_env.bat`
  now finds VS 2026 (18) Community / Professional / BuildTools before
  falling back to VS 2022.

---

## 4. The repo — key file inventory

### Source (`btz-sonic-alchemy-main/BTZ/Source/`)

| File | Lines | Purpose |
|---|---|---|
| `BTZDsp.h` | ~1850 | Single-header DSP library: `SmoothParam`, `SafetyLayer`, `BypassCrossfader`, `Waveshaper`, `ADAATanh`, `ADAATube`, `NeuralSaturationModel`, `WDFTubeStage`/`WDFTransformerStage`, `ResonanceTamer`, `TransientSplitter`, `OversamplingEngine`, `SidechainHPF`, `GlueCompressor`, `TruePeakLimiter`, `ShineProcessor`, `LinkwitzRileyCrossover`, `MultibandEngine`, `MidSideEncoder`, `LFO`, `MacroInterpreter`, `AutoGainSmoother`, `MeterBallistics`, `LoudnessMeter`, `SpectrumBuffer`, `GainReductionHistory`, `ReferenceToneMatcher`, `PresetIntelligence`, `UndoStack`, `ABState`, `PresetInfo`, `MIDILearnState`, `SimpleModeState`, `LoudnessMatchedAB`, `TargetLockBand`, `TargetLockEngine`. |
| `PluginProcessor.h/.cpp` | ~200 / ~1100 | `BTZAudioProcessor` — APVTS, processBlock, state save/restore, factory presets, MIDI learn, signal chain orchestration. |
| `PluginEditor.h/.cpp` | ~200 / ~1000 | UI: three view modes (Simple / Standard / Advanced), Ivory theme look-and-feel, knob/combo/button setup, visualizer wiring, preset browser, Target Lock UI. |
| `BTZTheme.h` | ~410 | Ivory System palette, type scale, spacing, knob/animation tokens, layout constants, parameter tooltips. |
| `BTZComponents.h` | ~700 | Custom widgets: `HarmonicVisualizer`, `GainReductionRibbon`, `SpectrumDisplay` / `DirectManipSpectrum`, `ProcessingIndicator`, `TabBar`, `LabeledKnob`, `PresetBrowser`, `SafetyIndicator`, `MeterBar`, `CorrelationMeter`. |

### Tests (`btz-sonic-alchemy-main/BTZ/`)

| File | Purpose |
|---|---|
| `tests/test_dsp_modules.cpp` | 86 GoogleTest unit tests, one per DSP module. |
| `bench/processor_check.cpp` | `BTZProcessorCheck` — 16 integration assertions (the behavior tests). |
| `bench/measure_main.cpp` | `BTZMeasure` — Phase-0 measurement harness: control audit, OS reality, aliasing, loudness honesty, ISP. |
| `bench/bench_main.cpp` | `BTZBench` — CPU benchmark at every quality tier. |

### Build (`btz-sonic-alchemy-main/BTZ/`)

| File | Purpose |
|---|---|
| `CMakeLists.txt` | Plugin + tests + 3 console-app targets. Options: `BTZ_BUILD_TESTS`, `BTZ_BUILD_BENCH`, `BTZ_BUILD_CLAP`. |
| `.github/workflows/ci.yml` | CI: macOS / Windows / Ubuntu builds + tests + pluginval (strictness 10, no `\|\| true`). |

### Scripts (`scripts/`)

| File | Purpose |
|---|---|
| `build_linux.sh`, `build_macos.sh`, `build_windows.ps1` | Standard cross-platform build scripts. |
| `build_windows_with_vs_env.bat` | Windows: loads vcvars64 for VS 2026 (18) → fallback to VS 2022, then runs cmake/ninja. |
| `install_vst3_windows.bat` | Windows: copies built `.vst3` bundle to `Program Files\Common Files\VST3\`. Parens-safe. |
| `install_vst3_macos.sh` | macOS: copies to `~/Library/Audio/Plug-Ins/VST3/`. |
| `run_pluginval.sh`, `run_pluginval.ps1` | Cross-platform pluginval runner. |
| `clean_build.sh`, `clean_build.ps1` | Remove `build/` dir. |

### Documentation (`docs/`)

**Read first** — orientation:
- `BTZ_SESSION_HANDOFF.md` (this file).
- `BTZ_UNIFIED_STATE_OF_TRUTH.md` — reconciles all prior handoffs; resolves contradictions.
- `BTZ_CURRENT_BASELINE_LOCK.md` — "must not regress" contract.
- `BTZ_MASTER_GUIDE.md` — manual-style reference of every control and the signal flow.
- `BTZ_CHANGELOG.md` — Keep-a-Changelog history (v1.0.0 through v1.0.6).

**Engineering** — `docs/dev/`:
- `PARAMETER_MANIFEST.md` — every parameter's ABI contract.
- `BTZ_RELEASE_BLUEPRINT_AUDIT.md` — 14-phase release-readiness compliance.
- `BTZ_PERF_SECURITY_AUDIT.md` — measured CPU + ASan/UBSan + security review.
- `BTZ_BUILD_AND_TEST_STATUS.md` — what's verified vs. needs DAW.
- `BTZ_VALIDATION_CHECKLIST.md` — compile/test/pluginval/DAW/DSP/UI/release gates.
- `BTZ_LOCAL_DEV_SETUP.md` — toolchains + dev deps.
- `BTZ_WINDOWS_BUILD_INSTALL_RUNBOOK.md` — field-validated Windows pipeline.
- `BTZ_CI_NOTES.md` — CI workflow notes.
- `BTZ_CURSOR_HANDOFF.md`, `BTZ_CODEX_HANDOFF.md` — handoff guides for other agents.

**Product** — `docs/product/`:
- `BTZ_PRODUCT_IDENTITY.md` — Target Lock as flagship; feature hierarchy.

**Release** — `docs/release/`:
- `BTZ_FORMAT_ROADMAP.md` — VST3 first, CLAP next, AAX after commercial review.
- `BTZ_RELEASE_READINESS_SCORECARD.md` — measured 1-10 scores per category.

**Measurements** — `docs/measurements/`:
- `BTZ_BASELINE.md` — Phase-0 measured baseline + v1.0.5/v1.0.6 before-after evidence.

---

## 5. What is verified at HEAD

| Item | Result | Evidence |
|---|---|---|
| Compiles + links | ✅ | Linux Standalone (13 MB) + VST3 `.so` (13 MB) |
| **Unit tests** | ✅ **86/86 pass** | `./build/BTZTests_artefacts/Release/BTZTests` |
| **Integration tests** | ✅ **16/16 pass** | `./build/BTZProcessorCheck_artefacts/Release/BTZProcessorCheck` |
| ASan + UBSan + LSan | ✅ **0 findings** | `build-san/` |
| CPU | 0.75 % Eco · 3.73 % Ultra+all-on | `BTZMeasure` §2 |
| Mix law | mix=0 → dry (±0.5 dB); mix=1 → wet active; mix=0.5 → blend | integration §11 |
| Master law | master 0.45 → **−6.00 dB**; master 0.95 → **+6.00 dB** | integration §12 |
| Drive level honesty | sweep within **±1.12 dB** of dry across 0..1 | integration §16 |
| Tube alias floor (default) | **−66.5 dB** at Eco (was −56.4) | `BTZMeasure` §3 |
| Tanh alias floor | **−66.1 dB** at Eco (with ADAA) | `BTZMeasure` §3 |
| Target Lock convergence | engages, reports typed target, audio keeps flowing | integration §15 |
| Loudness honesty (autoGain on) | < 0.5 LU drift across every macro sweep | `BTZMeasure` §4 |

### What is **not** verified (needs real machine / DAW / ears)

- macOS Universal build + `auval` + Apple Silicon CPU.
- Windows VST3 in Cubase / Studio One / FL / Pro Tools (Reaper smoke is done per the runbook).
- pluginval @10 on the patched binary (CI is wired; never run since the patches landed).
- Accessibility behavior against VoiceOver / Narrator (no `AccessibilityHandler` in code yet).
- HiDPI / multi-display visual scaling.
- Latency reporting against a real host's PDC.
- *The sound*. Specifically: are the 5 analog saturation models *distinct* and *musical*, and is the Tube THD at drive=0.5 (currently ~27 %) the character you want, or do you want it gentler (3–8 %)?

---

## 6. Open backlog (synthesized from `BTZ_UNIFIED_STATE_OF_TRUTH.md` §6)

### P0 — operational / legal release blockers (none are engineering work)

1. **JUCE 8 commercial license** purchase or GPL distribution decision.
2. **Trademark clearance** on chosen name (currently three: "Box Tone Zone" / "Sonic Alchemy" / "BTZ Ivory" — pick one).
3. **EULA** + privacy notice.
4. **macOS Universal build** verified locally; AU `auval` pass.
5. **Windows VST3 build** verified locally (per the runbook).
6. **pluginval @10** run on the patched binary.
7. **DAW smoke tests** in Reaper + ≥1 of Logic / Live / Cubase.
8. **Installers** (`.pkg`, Inno Setup) + code signing / macOS notarization / Windows Authenticode.
9. **No support channel / no crash reporter / no end-user manual** yet.

### P1 — engineering items recommended next

10. **Three-stimuli truth audit** in `BTZMeasure` (pink noise + transient burst, in addition to sine) so dead-vs-stimulus-mismatch is separable.
11. **`AccessibilityHandler`** + roles + labels on every interactive control.
12. **External audio sidechain bus** for the Glue compressor.
13. **Tempo sync** for Motion / LFO.
14. **HQ-on-render** mode.
15. **ADAA on Transistor + Transformer** (same closed-form pattern as Tube; easy).
16. **Tape model aliasing fix** (its hysteresis integrator reinjects wideband content — needs DSP design).
17. **True-peak meter calibration** (currently underreports by ~1.4 dB vs independent 4× ISP).
18. Decide and act: wire-or-remove multiband (`split`/`recombine` are never called); train-or-remove neural saturation slots.

### P2 — product / UX

19. **First-run tooltip tour** (Drive → Target Lock → A/B → Bloom → Output).
20. **Gain-match factory presets** to a target LUFS.
21. **Right-click context menu** on every knob (reset / copy-paste / type / fine-drag).
22. **Preset browser** upgrade (audition, tags, favorites, "surprise me").
23. **Mastering credibility**: lengthen limiter lookahead beyond 8 samples (or stop marketing as mastering).
24. **`punch` is unused** — design where it should wire (transient attack? drive envelope?) and implement.

### P3 — documentation

25. Split `BTZ_MASTER_GUIDE.md` into chapters: Quick Start / Manual / DSP Appendix / Installation / Troubleshooting / FAQ.
26. `LICENSE_RISK_AUDIT.md` + dependency license inventory.

---

## 7. How to pick this up

### On the Windows workstation (the primary test environment)

```cmd
:: Pull the latest baseline
cd C:\Users\marcm\OneDrive\Desktop\BTZ\btz-sonic-alchemy
git fetch origin
git checkout main
git pull --ff-only

:: Optional: push the v1.0.4/5/6 tags the sandbox couldn't push
git tag v1.0.4 31f8029
git tag v1.0.5 3ea568e
git tag v1.0.6 45a07c8
git push origin v1.0.4 v1.0.5 v1.0.6

:: One-time: JUCE junction (skip if C:\JUCE-8.0.6 already exists)
mklink /J "C:\JUCE-8.0.6" "C:\Users\marcm\OneDrive\Desktop\DESKTOP (2026)\JUCE-8.0.6"
set JUCE_DIR=C:\JUCE-8.0.6

:: Build + tests (preferred path)
pwsh scripts\build_windows.ps1 -Config Release -Tests

:: Install (CLOSE REAPER FIRST)
scripts\install_vst3_windows.bat
```

Then in Reaper: `Options → Preferences → Plug-ins → VST → Re-scan`. The plugin is **Box Tone Zone (BTZ)** by **BTZ Audio**.

Full Windows runbook (build pitfalls, install workarounds, troubleshooting): `docs/dev/BTZ_WINDOWS_BUILD_INSTALL_RUNBOOK.md`.

### On Linux (for measurements / sanitizer runs)

```bash
sudo apt-get install libx11-dev libxext-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxcomposite-dev libxrender-dev libfreetype6-dev \
  libfontconfig1-dev libasound2-dev libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
                       -DBTZ_BUILD_TESTS=ON -DBTZ_BUILD_BENCH=ON \
                       btz-sonic-alchemy-main/BTZ
cmake --build build --target BTZ_Standalone BTZTests BTZProcessorCheck BTZMeasure BTZBench
./build/BTZTests_artefacts/Release/BTZTests             # 86 unit tests
./build/BTZProcessorCheck_artefacts/Release/BTZProcessorCheck   # 16 integration assertions
./build/BTZMeasure_artefacts/Release/BTZMeasure         # regenerate BTZ_BASELINE.md
./build/BTZBench_artefacts/Release/BTZBench             # CPU bench
```

### Re-run only the measurements after a change

```bash
cmake --build build --target BTZMeasure
./build/BTZMeasure_artefacts/Release/BTZMeasure > docs/measurements/BTZ_BASELINE.md
```

Every claim about sound or efficiency should be backed by re-running this and committing the new numbers.

### Hand the project to another agent

- **Cursor** (local builds, DAW validation): `docs/dev/BTZ_CURSOR_HANDOFF.md`.
- **Codex** (deterministic tooling, CI, small patches): `docs/dev/BTZ_CODEX_HANDOFF.md`.
- **Anyone**: read this file, then `BTZ_UNIFIED_STATE_OF_TRUTH.md`, then dive in.

---

## 8. Critical operating rules carried forward

From the session's accumulated discipline:

- **Patch / diff-first.** Don't rewrite working systems. The 86/86 + 16/16 status
  is hard-won; respect it.
- **Parameter IDs are a public ABI.** Renames require state-migration logic
  keyed off `BTZDsp::kStateVersion`. See `PARAMETER_MANIFEST.md`.
- **`processBlock` stays RT-safe.** No allocations, no locks, no UI work,
  denormals flushed. ASan-verified.
- **No sound-quality claim without measurement.** Re-run `BTZMeasure` and
  commit the new numbers; the changelog gets the before/after.
- **No DAW or pluginval claim without an actual run.** The sandbox cannot
  perform either — those claims have to come from the workstation.
- **Honesty.** Greyed-out features (neural, multiband) are documented as
  "coming in v1.1"; leaving active-looking dead controls is forbidden.

---

## 9. Honest limits of what was done in this sandbox

- **No DAW load**. Reaper validation has been done by the user on the workstation
  per the runbook; the sandbox has no audio host.
- **No macOS / Windows build attempted here**. Scripts are written and the
  Windows path has been field-validated; the sandbox is headless Linux only.
- **No pluginval run** on the patched binary. CI is wired for it; needs a real
  runner.
- **No ears, no taste pass.** The numerical fixes are landed; whether Tube at
  drive=0.5 sounds like a tube, or whether the 5 analog models are *distinct
  enough to keep all 5*, is a workstation-with-headphones call.

---

## 10. The "what should I touch next" cheat sheet

If you have an hour: **listen to the v1.0.6 build in Reaper.** Sweep Drive 0→1
on Tube with Master at unity, autoGain off. Confirm the level stays roughly
constant and the character grows. If yes, the engineering side of "Drive feels
right" is closed. If no, capture what's still wrong and re-open the §16
integration test with the new gate.

If you have a day: pick **two P0 owner decisions** from §6 (JUCE license,
trademark, single canonical name, mastering positioning) and resolve them.
None are engineering work and they unblock everything downstream.

If you have a week: **macOS Universal build + `auval` + Reaper/Logic/Live smoke
tests + pluginval @10 in CI**. After that, you're an installer + signing
session away from a real RC1.

If you have a month: P1 engineering items in order. Accessibility first (legal
surface), then sidechain bus (the missing pro-workflow capability), then the
honest decisions on multiband and neural.

---

*Generated by Claude (Anthropic) at the end of the v1.0.6 sandbox session.
The repo is the source of truth; this document is a navigation aid.*
