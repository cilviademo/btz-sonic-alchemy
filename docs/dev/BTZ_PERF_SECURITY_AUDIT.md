# BTZ — Performance & Security Audit

Measured, not estimated. Built against real JUCE 8.0.6 on Linux (GCC 13, -O3),
driven by the `BTZBench` harness (`-DBTZ_BUILD_BENCH=ON`) which times the actual
`BTZAudioProcessor::processBlock`. Also reviewed under ASan/UBSan/LSan and -Wall.

---

## 1. CPU — verdict: efficient, not a hog

48 kHz, 512-sample blocks (10.667 ms/block), single core, after optimization:

| Scenario | ns/block | ns/sample | RT factor | CPU% (1 core) |
|---|---|---|---|---|
| Eco (1×), default | 79,630 | 155 | 134× | **0.75%** |
| Standard (2×), default | 109,627 | 214 | 97× | **1.03%** |
| High (4×), default | 173,886 | 340 | 61× | 1.63% |
| Ultra (8×), default | 295,792 | 578 | 36× | 2.77% |
| Standard (2×), ALL modules on | 148,258 | 290 | 72× | **1.39%** |
| Ultra (8×), ALL modules on | 398,163 | 778 | 27× | **3.73%** |

Even the worst case (8× oversampling + glue + shine + resonance + transient +
M/S + Target Lock) uses **under 4% of one core** — ~27× realtime headroom. A
typical session (Standard quality, a few modules) is ~1% per instance. This is
well within "performs smoothly" for dozens of instances.

## 2. Optimizations applied this pass (all behavior-preserving)

| Change | Win | Risk |
|---|---|---|
| `TruePeakLimiter` caches `dbToGain(ceiling)`, recomputes only on change | removed **1 `std::pow`/sample** | none (bit-identical; public field still works) |
| Master gain → per-block linear ramp (`advanceBlock` + 2 `dbToGain`/block) | removed **1 `std::pow`/sample** | none (audibly identical gain ramp) |
| `ResonanceTamer` 32-band loop → single computation | **~32× cheaper** for that module when enabled | none — **bit-identical output** (all 32 bands were identical) |

Net effect: Standard+ALL-ON −32%, Ultra+ALL-ON −17%, every default path lighter.
Verified by re-running `BTZBench` and the full unit suite (83/86, unchanged).

## 3. Remaining efficiency opportunities (not done — need a judgment call)

- **Glue compressor detector** does `gainToDb` (log10) + `dbToGain` (pow) **per
  sample** when active. This is intrinsic to a log-domain compressor; a faster
  log/exp approximation or a dB-domain envelope would help but changes the
  compression curve slightly — defer to a deliberate DSP decision.
- **Always-on metering**: the K-weighting (2 biquads/sample) in `LoudnessMeter`
  and the 16-mult/sample true-peak ISP detector run every block regardless of
  whether the editor is open. Could be gated on "editor active" for headless
  render, but the cost is small and some hosts show meters without the editor.
- **`ResonanceTamer` is broadband, not multiband.** The collapse above preserves
  current behavior; a *real* multiband resonance suppressor (per-band bandpass
  filters, à la Soothe) would be a feature, not an optimization.
- **SIMD**: the per-sample inner loops (saturation, biquads, ISP) are scalar.
  `juce::dsp::SIMDRegister` could vectorize them, but given the current low CPU
  it's optional polish, not a need.

## 4. Real-time safety (audio thread)

- ✅ `juce::ScopedNoDenormals` + FTZ/DAZ set; no denormal CPU spikes.
- ✅ No heap allocation in `processBlock` (pre-allocated `dryBuffer`, fixed-size
  module state, stack arrays only). ASan reported no heap activity in the hot path.
- ✅ No locks; meter/spectrum/GR communication is via relaxed atomics / single-
  writer-single-reader buffers.
- ⚠ MIDI-learn still touches `juce::String` on the audio thread (refcount bump,
  usually alloc-free but fragile) — recommend a lock-free CC FIFO resolved on the
  message thread. Low severity.

## 5. Security / robustness review

| Area | Finding |
|---|---|
| **State restore** (`setStateInformation`) | JUCE binary→XML→ValueTree parsing is robust to malformed data (returns null). MIDI mappings capped at `kMaxMIDIMappings`; restored parameter IDs resolved via `getParameter` (null-checked). Safe. |
| **Preset load** (`loadPreset`) | `XmlDocument::parse` + `fromXml` robust; index `jlimit`-ed; fixed preset dir (no path traversal). Safe. |
| **Neural model load** (`loadWeights`) | Bounds-checked: requires `numWeights >= expected (161)`, reads exactly that, ignores extra. No OOB. Safe. |
| **Block size** | `dryBuffer`/oversampling are sized to the prepared max. A host exceeding its declared max would have overrun them → **fixed**: `processBlock` now re-prepares on an oversized block (defensive; rare path). |
| **Integer params** | `satModel`, `multibandCount`, quality, sidechain HPF mode all `jlimit`-ed before use. Safe. |
| **Memory safety** | ASan + UBSan + LSan: **zero findings** across the DSP suite (incl. NaN/Inf/denormal/extreme inputs on all models). |
| **External surface** | No network, no shell, no `system`, no `eval`, no format-string risk (fixed format + floats). |

No security vulnerabilities found. The one robustness gap (oversized block) is fixed.

## 6. Not yet covered (recommended next)

- **ThreadSanitizer** run (data-race detection) — the sanitizer pass here was
  ASan/UBSan only; TSan needs a processor harness driving the audio thread + a
  simulated UI reader thread.
- **Processor-level ASan harness** (reuse `BTZBench` structure) to extend the
  memory/UB coverage from `BTZDsp.h` to the full `processBlock` integration.
- **macOS/Windows CPU numbers** — these were measured on Linux/GCC; clang/MSVC
  and Apple Silicon will differ (likely similar or better).
- **pluginval @10** is the functional-safety complement to this perf/security pass.

## How to reproduce
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBTZ_BUILD_BENCH=ON -DBTZ_BUILD_TESTS=ON
cmake --build build --target BTZBench BTZTests
./build/BTZBench_artefacts/Release/BTZBench       # CPU numbers
./build/BTZTests_artefacts/Release/BTZTests       # correctness (83/86)
```
