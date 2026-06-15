# BTZ v11 — Compilation & Code Review

Branch: `overhaul/v1.1-dsp-architecture` @ `3905c4f`
Scope: `btz-sonic-alchemy-main/BTZ/` (plugin source, CMake, test suite)

The v11 commit message and the accompanying summary claim a "definitive
senior‑dev quality pass." In practice the tree as currently committed
**will not compile** as a plugin and **will not compile** as the test
binary either. Several pieces are also wired up but never used in the
signal path. This document is a punch list — what's broken, why, and
what to do about it.

---

## 1. Plugin will not compile (hard errors)

### 1.1 Undefined identifiers in `BTZTheme.h` referenced from `PluginEditor.cpp`

`PluginEditor.cpp` uses the following symbols that are **not declared** in
`BTZTheme.h`:

| Reference | File:Line | Status |
|---|---|---|
| `btz::layout::maxW` | `PluginEditor.cpp:146` | undefined |
| `btz::layout::maxH` | `PluginEditor.cpp:146` | undefined |
| `btz::layout::defaultW` | `PluginEditor.cpp:147, 271` | undefined |
| `btz::layout::defaultH` | `PluginEditor.cpp:147, 271` | undefined |
| `btz::type::display()` | `PluginEditor.cpp:419` | undefined |
| `btz::type::title` | `PluginEditor.cpp:419` | undefined |
| `btz::type::small` | `PluginEditor.cpp:431` | undefined |
| `btz::id::ceiling` | `PluginEditor.cpp:214` | undefined |

`BTZTheme.h` only defines `layout::{windowW, windowH, minW, minH, headerH,
footerH, tabH, knob*, meter*, buttonH}`, `type::{sans(), mono(), brand,
h1, h2, body, label, micro, value, trackLabel, trackBody}`, and
`id::{drive, mix, output, punch, warmth, boom, glue, air, width, density,
motion, era, intensity, tone, resTame, transient}`. The summary
description claims `type::heading()/body()/mono()` returning
Syne/Inter Tight/IBM Plex Mono — the file actually exposes `sans()`
returning "Inter" and `mono()` returning "JetBrains Mono". Description
and code have drifted apart.

**Fix:** either add the missing constants to `BTZTheme.h` or change the
references in `PluginEditor.cpp` to existing names (e.g. `windowW`
instead of `defaultW`, `h1` instead of `title`, `micro` instead of
`small`, etc.).

### 1.2 `NeuralSaturationModel::loadWeights` does not exist

`PluginProcessor.cpp:884` calls

```cpp
target->loadWeights(weights.data(), (int)weights.size());
```

but `BTZDsp.h:313–381` only declares `loadModel(const juce::File&)` and
`process(float)`. There is no `loadWeights(const float*, int)` overload.
Hard compile error.

**Fix:** either implement `loadWeights` in `NeuralSaturationModel` (it
needs to parse the flat float array into the Wz/Wr/Wh/Uz/Ur/Uh/bz/br/bh/
outputWeights/outputBias slots, in a documented order) or change
`loadNeuralModel` to call `target->loadModel(modelFile)` and let
`NeuralSaturationModel` do its own JSON parsing.

### 1.3 Binding an rvalue `MemoryOutputStream` to a non‑const lvalue ref

`PluginProcessor.cpp:105`:

```cpp
apvts.state.writeToStream(juce::MemoryOutputStream(initialState, false));
```

`juce::ValueTree::writeToStream` takes `juce::OutputStream&`
(non‑const lvalue reference). You cannot bind a temporary to it under
standard C++; Clang and GCC reject it, and MSVC with `/permissive-`
(which `juce_recommended_config_flags` enables in JUCE 8) also rejects
it. Hard error on all three compilers.

**Fix:**

```cpp
juce::MemoryOutputStream stream(initialState, false);
apvts.state.writeToStream(stream);
```

(The pattern is already used correctly elsewhere — e.g.
`pushUndoState`, `storeA`, `storeB`.)

---

## 2. Editor compiles but crashes on plugin load

Even after the §1 fixes, the editor constructor wires up
`SliderAttachment` / `ComboBoxAttachment` objects against parameter IDs
that the processor never registers. `APVTS::*Attachment` ctors call
`getParameter`, hit `jassertfalse`, and on release builds dereference
null when the host first instantiates the editor.

| Editor uses | Actual processor param | Status |
|---|---|---|
| `"ceiling"` (`PluginEditor.cpp:252`) | not registered | crash |
| `"intensity"` (`PluginEditor.cpp:253`) | not registered | crash |
| `"resTame"` (`PluginEditor.cpp:262`) | `"resSens"` | crash |
| `"transientSens"` (`PluginEditor.cpp:263`) | `"transSens"` | crash |
| `"numBands"` (`PluginEditor.cpp:268`) | `"multibandCount"` | crash |

Note that `setupSmallKnob(kCeiling, lCeiling, "CEILING", id::ceiling);`
also fails to compile per §1.1 — fixing the ID lookup is necessary but
not sufficient; you need to decide whether these are *new* parameters
to register, or whether the editor should drop them.

**Fix (recommended):**

* Remove `kCeiling`, `lCeiling`, `kIntensity`, `lIntensity` and their
  attachments (the "Spark page" they're meant to fill is not actually
  wired into `layoutStandard`/`layoutAdvanced`).
* Rename `aResTame` to attach `"resSens"` and `aTransSens` to attach
  `"transSens"`.
* Rename `aMultiband` to attach `"multibandCount"`.

Also note that the `cSatModel` combo lists only 9 entries
(Tanh…Neural Custom) while the `satModel` parameter has range
`0..NumModels-1 = 0..10` (WDF_Tube and WDF_Transformer are reachable
from `processNonlinear`'s switch but unreachable from the UI). Either
add the two WDF entries to `cSatModel.addItemList(...)` at
`PluginEditor.cpp:232` or tighten the parameter range to 9. Similarly
`cGlueScHpf` has 5 items ("Off", "60 Hz", "100 Hz", "200 Hz", "300 Hz")
but the underlying `glueScHpf` `AudioParameterInt` has range `0..3`,
and `scHpfFreqs` in `prepareToPlay` only has 4 entries — the "300 Hz"
combo entry would never produce 300 Hz.

---

## 3. Tests will not compile (`-DBTZ_BUILD_TESTS=ON`)

`tests/test_dsp_modules.cpp` was clearly not rebuilt against the v11
DSP API. The following calls reference symbols that do not exist on
the corresponding structs in `BTZDsp.h`:

| Test line | Call | Actual API in `BTZDsp.h` |
|---|---|---|
| `94` | `env.prepare(kSR, 0.001f, 0.050f)` | `setTimes(attMs, relMs, sr)` |
| `98, 103` | `env.envelope` | member is `env` |
| `593, 608, 612, 629, 631` | `lfo.next()` | `lfo.tick()` |
| `638, 651` | `m.targetParamID` (`juce::String`) | `m.targetParam` (`int`) |
| `640, 653` | `MacroInterpreter::Curve::{Linear,Exponential}` | `Mapping::curve` is a `float` (1.0 linear, <1 log, >1 exp); enum doesn't exist |
| `643, 656` | `mi.slots[0].value` | no such member |
| `644, 657` | `mi.getMappedValue(macroIdx, mapIdx)` | function lives on `MacroSlot` and takes `(macroValue, mappingIndex)` |
| `668, 669, 679, 680` | `ags.updateInput(float)`, `ags.updateOutput(float)` | both take `(peakL, peakR)` |

Either rewrite the tests against the actual v11 API or restore the
APIs the tests expect. Pick one and be consistent — what's currently
there is the worst of both worlds.

---

## 4. Real-time / thread-safety violations

These compile and run but break the "no allocation, no locks, no
non‑RT‑safe ops in `processBlock`" claim from the file headers.

1. **`updateTargetsFromAPVTS` calls `multibandEngine.prepare(...)`
   every block** (`PluginProcessor.cpp:266`). `prepare` walks a fixed
   array so it doesn't malloc, but it also resets filter state and
   recomputes coefficients — running this per block produces audible
   crossover glitches whenever multiband is in use.

2. **`updateLatencyFromQuality` is called from the audio thread**
   (`PluginProcessor.cpp:335`). `AudioProcessor::setLatencySamples`
   notifies the host on the message thread internally, but several
   hosts (notably Pro Tools, Live, JUCE Standalone) will lock or
   restart processing on a latency change. Latency changes belong on
   `prepareToPlay` or in response to a parameter listener callback,
   not the audio block.

3. **`processMIDILearn` mutates `juce::String learningParamID` and
   `juce::String parameterID` on the audio thread**
   (`PluginProcessor.cpp:638–644`). `juce::String` uses a reference‑counted
   shared buffer; assignment can allocate and is not lock‑free. The
   same `MIDILearnState` is read by `getStateInformation` on the
   message thread, so this is also racy. Move learn‑state writes off
   the audio thread (queue the CC number and resolve it on the message
   thread).

4. **`processBlock` reads boolean parameters via
   `*apvts.getRawParameterValue("autoGain")` etc. on every block.**
   That's fine on its own, but combined with `updateTargetsFromAPVTS`
   doing the same work it's wasteful. Cache once per block.

5. **`shineProcessor.recalcCoeffs(currentSampleRate)` is called every
   block when shine is engaged** (`PluginProcessor.cpp:460`). That's
   a `std::pow`, two `std::sin/cos`, and several divisions per block —
   acceptable but only worth doing when the smoothed parameters
   actually changed.

---

## 5. DSP correctness bugs

These will compile but produce wrong / unstable / misleading output.

### 5.1 `ShineProcessor::processStereo` is not a biquad

`BTZDsp.h:712–720`:

```cpp
float outL = b0 * l + b1 * z1L + b2 * z2L - a1 * z1L - a2 * z2L;
z2L = z1L; z1L = outL;
```

The biquad direct‑form‑I uses **input** delay taps for `b1, b2` and
**output** delay taps for `a1, a2`. Here both pairs are applied to the
same `z1L, z2L`, and `z1/z2` are updated to be previous outputs only —
the input is never delayed. The transfer function being implemented
isn't the one `setParameters`/`recalcCoeffs` is solving for. This
filter will not produce the requested peak / shelf response and is
likely to be unstable for high‑Q settings.

Use a TDF‑II form or split into `xz1/xz2/yz1/yz2`.

### 5.2 `SidechainHPF` is not an HPF

`BTZDsp.h:581–588`:

```cpp
const float newL = l - stateL;
stateL = l - coeff * stateL;
l = newL;
```

`coeff = exp(-2π·fc/sr)` is the LP pole. A first‑order HPF on this
pole is `y[n] = x[n] - x[n-1] + coeff*y[n-1]`, i.e. you need to
remember the **previous input** and use the LP state as the output
state. As written, the filter has a frequency response that does not
correspond to a high‑pass at `freqHz`. The "PassesHighFrequency" /
"RemovesLowFrequency" tests pass only because the formula happens to
attenuate roughly the right band; the actual transfer function is
wrong.

### 5.3 `LinkwitzRileyCrossover` is a one‑pole low‑pass

`BTZDsp.h:723–746` advertises Linkwitz‑Riley but implements
`lp += coeff*(in - lp); high = in - lp;` — that's a 1‑pole RC LP with
a complementary HP. A proper LR4 is two cascaded Butterworth LP/HP at
the same cutoff, with the HP being phase‑inverted on summation. The
current code will produce poor band isolation (–6 dB/oct stopband) and
audible crossover smear, and the "low + high ≈ input" sum has a
phase response that's nothing like LR's allpass sum.

Replace with either two cascaded `juce::dsp::LinkwitzRileyFilter`
instances or a hand‑rolled biquad pair.

### 5.4 `WDFTransformerStage::primaryFlux` has no saturation guard on the integrator

`BTZDsp.h:412–418` integrates `primaryFlux += input * 0.01f` and
multiplies by `0.999f` per sample. For sustained near‑DC input the
asymptote is `input * 10` — at sample rate this is well within range,
but at very high SR and large input the integrator can reach +/-1e3
range before `fastTanh` saturates the output. More importantly,
without a DC trap the integrator drifts and adds long‑term low‑frequency
artifacts. Add a leakage factor much closer to e.g. `0.9995` plus a
hard‑clamp on `primaryFlux`.

### 5.5 `OversamplingEngine` is dead code

`BTZDsp.h:501–564` defines an FIR oversampling engine but
`processBlock` uses `juce::dsp::Oversampling<float>` instead and
`OversamplingEngine::upsample` "inserts zeros" without doing the
companion FIR — so even if it were called, it would alias. The engine
is `prepare`d and `reset` in `prepareToPlay`/`resetAll` but never used
in the signal path. Delete or wire it in.

### 5.6 `MultibandEngine` is dead code

`MultibandEngine::split`/`recombine` are never called from
`processBlock`. The processor calls `multibandEngine.prepare(...)`
from `updateTargetsFromAPVTS` but otherwise ignores it. The
"multibandCount" parameter currently controls nothing.

### 5.7 `Waveshaper::tape` clamps its delta to `[-0.5, 0.5]` then runs through `fastTanh(state*0.9)`

Looks fine for small drives, but at high drive the `hysteresisState`
saturates to ~±1 and the delta clamp blocks state inversion until the
input swings back. The audible effect is asymmetric hysteresis that
freezes for tens of samples on transients — closer to "broken DC
servo" than "tape". Worth a perceptual A/B against a reference tape
emulation.

### 5.8 `MacroInterpreter` is wired up but never invoked

`MacroInterpreter macroInterpreter;` is declared in `PluginProcessor.h`
but never used. The `macro0..macro3` parameters smooth into
`sMacro0..sMacro3` but neither the smoothed values nor the interpreter
ever modulate anything. Macros are inert.

### 5.9 LFOs are wired up but never invoked

`lfoModSources[kMaxLFOs]` is declared but `kMaxLFOs` is **not defined
anywhere** — searching `BTZDsp.h` and the editor turns up no
declaration. This is itself a compile error in `PluginProcessor.h:134`
that I missed in §1 — add it to the §1 list. Even if `kMaxLFOs` is
added, no code calls `lfo.tick()` anywhere in `processBlock`.

---

## 6. Build system

* `CMakeLists.txt:60–72` — `FORMATS VST3 AU Standalone`. The summary
  claims CLAP is in the formats list; in fact CLAP is added via
  `clap_juce_extensions_plugin` (correct approach), but the v11
  summary is slightly misleading.
* `CMakeLists.txt:43` — `BTZ_BUILD_CLAP ON` by default fetches
  `clap-juce-extensions@main`. That's a moving target; pin to a tag
  for reproducible builds.
* `CMakeLists.txt:10` — hard‑coded Windows default JUCE path. Harmless
  fallback, but the comment chain would be tidier if the env‑var path
  was the only documented option.
* No `set_target_properties(BTZ PROPERTIES CXX_EXTENSIONS OFF)` — JUCE
  picks this up via its own config flags, but worth being explicit
  given the v11 manifesto.
* `BUNDLE_ID` and `PLUGIN_MANUFACTURER_CODE` look fine. `PLUGIN_CODE
  Btz1` is mixed case — JUCE requires exactly one uppercase letter
  and three lowercase. `Btz1` is fine; just flagging that the
  validator does check this.

---

## 7. UX / wiring gaps

* `btnPresetSave.onClick` is never assigned (`PluginEditor.cpp:178–180`).
  The Save button is dead.
* Simple Mode hides `kDrive/kShine/kMaster` (`hideAllControls`) and
  uses `btz::LabeledKnob` (which is read‑only — no mouse handling).
  The user has no way to change values in Simple Mode. Either make
  `LabeledKnob` interactive or have Simple Mode show the actual
  Sliders.
* `PresetBrowser` is added to the view but `setPresets(...)` is never
  called — the list is empty.
* `lfoCount`, `simpleMode`, `simpleDrive`, `simpleTone`, `simpleOutput`,
  `autoGain` (well, autoGain is used), `toneMatchAmt`, `transMix`,
  `transEnabled`, `resEnabled` are all declared but a chunk of them
  (`simpleDrive`/`simpleTone`/`simpleOutput`, `lfoCount`, `toneMatchAmt`)
  are never attached to UI controls and never read in the audio path
  beyond what `updateTargetsFromAPVTS` smooths. They are inert.

---

## 8. Recommendation

The v11 commit can be salvaged but it is **not "definitive production
quality"** in its current state — it's pre‑first‑successful‑build. The
quickest path to a green build:

1. Fix the four hard compile errors in §1 (theme constants, neural
   loader, the rvalue stream, `kMaxLFOs`).
2. Fix the five APVTS attachment string mismatches in §2.
3. Decide what to do about the dead modules (Multiband, Oversampling
   FIR, MacroInterpreter, LFOs). Wire them up or delete — don't ship
   half‑finished.
4. Either rewrite the tests against the v11 API or restore the v10‑era
   API the tests still target.
5. Fix `ShineProcessor`, `SidechainHPF`, and `LinkwitzRileyCrossover`
   so they implement the filters their names advertise. The tests
   that pass against these today are not catching the bugs because
   they only check coarse "energy lower / higher than X" properties.

Only after the build is green and the DSP filters are corrected does
it make sense to chase the "potential next steps" in the v11 summary
(installers, notarization, neural training pipeline, CI).
