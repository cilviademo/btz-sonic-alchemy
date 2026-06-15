# BTZ — Parameter Manifest

**Source of truth:** `btz-sonic-alchemy-main/BTZ/Source/PluginProcessor.cpp` →
`BTZAudioProcessor::createParameterLayout()`.
**State version:** `BTZDsp::kStateVersion = 12` (preset XML carries this; older
versions migrate through `BTZAudioProcessor::migrateState`).

> **ABI promise.** Parameter IDs below are a **public, stable contract** —
> they appear in saved sessions, presets, automation lanes, and host
> mappings. They must never be renamed without a state-migration step. New
> parameters must be appended; defaults of existing parameters may be tuned
> but should be considered minor-breaking for muscle memory.

## Float parameters

| ID | Display Name | Range | Default | Skew | Smoothed? | Display |
|---|---|---|---|---|---|---|
| `punch` | Punch | 0 – 1 | 0.50 | linear | per-sample (next) | `0–100 %` |
| `warmth` | Warmth | 0 – 1 | 0.50 | linear | per-sample | `0–100 %` |
| `boom` | Boom | 0 – 1 | 0.30 | linear | block (advanceBlock) | `0–100 %` |
| `glue` | Glue | 0 – 1 | **0.45** | linear | block | `0–100 %` |
| `air` | Air | 0 – 1 | 0.30 | linear | (inert) | `0–100 %` |
| `width` | Width | 0 – 1 | 0.50 (=normal) | linear | block | `0–100 %` |
| `drive` | Drive | 0 – 1 | 0.30 | linear | per-sample | `0–100 %` |
| `mix` | Mix (wet/dry) | 0 – 1 | 1.00 | linear | block | `0–100 %` |
| `master` | Master | 0 – 1 (= -16.8 … +7.2 dB via `(v-0.7)·24`) | 0.70 (=0 dB) | linear | block (gain-ramp) | `dB` |
| `density` | Density | 0 – 1 | 0.50 | linear | per-sample | `0–100 %` |
| `motion` | Motion | 0 – 1 | 0.00 | linear | block | `0–100 %` |
| `era` | Era | 0 – 1 | 0.50 | linear | (inert) | `0–100 %` |
| `macro0..3` | Macro A..D | 0 – 1 | 0.50 | linear | (inert) | `0–100 %` |
| `intensity` | Intensity | 0 – 1 | 0.50 | linear | (inert) | `0–100 %` |
| `shine` | Shine (peaking EQ gain trim) | 0 – 1 | **0.20** | linear | block | `0–100 %` |
| `shineMix` | Shine Mix | 0 – 1 | 1.00 | linear | (inert) | `0–100 %` |
| `shineFreq` | Shine Freq | 1000 – 16000 Hz | 8000 | sqrt skew (0.5) | block | `Hz` / `kHz` |
| `shineQ` | Shine Q | 0.3 – 8.0 | 0.707 | sqrt skew | block | `Q 0.70` |
| `ceiling` | Limiter Ceiling | -12 – 0 dB | -0.3 | linear | (raw read per block; cached in limiter) | `dB` |
| `glueAttack` | Glue Attack | 0.1 – 100 ms | 10 | sqrt skew | raw read | `ms` |
| `glueRelease` | Glue Release | 10 – 1000 ms | 100 | sqrt skew | raw read | `ms` |
| `glueRatio` | Glue Ratio | 1 – 20 | 3.0 | sqrt skew | raw read | `:1` |
| `glueLink` | Glue Stereo Link | 0 – 1 | 1.00 (full) | linear | raw read | `0–100 %` |
| `resSens` | Resonance Sensitivity | 0 – 1 | 0.50 | linear | per-sample | `0–100 %` |
| `resDepth` | Resonance Depth | 0 – 1 | 0.50 | linear | per-sample | `0–100 %` |
| `transSens` | Transient Sensitivity | 0 – 1 | 0.50 | linear | raw read | `0–100 %` |
| `transMix` | Transient Mix | 0 – 1 | 0.50 | linear | block | `0–100 %` |
| `toneMatchAmt` | Tone Match Amount | 0 – 1 | 0.00 | linear | (inert) | `0–100 %` |
| `simpleDrive` | Simple Drive | 0 – 1 | 0.50 | linear | (Simple-mode UI only) | `0–100 %` |
| `simpleTone` | Simple Tone | 0 – 1 | 0.50 | linear | (Simple-mode UI only) | `0–100 %` |
| `simpleOutput` | Simple Output | 0 – 1 | 0.50 | linear | (Simple-mode UI only) | `0–100 %` |
| `targetLUFS` | Target LUFS | -60 – 0 dB | -14 | linear | raw read | `LUFS` |
| `targetRMS` | Target RMS | -60 – 0 dB | -14 | linear | raw read | `dB` |
| `targetDynThresh` | Dynamics Threshold | 0 – 24 dB | 3.0 | linear | raw read | `dB` |
| `targetLowDb` / `targetMidDb` / `targetHighDb` | Target Low/Mid/High dB | -60 – +6 dB | 0 | linear | raw read | `dB` |

## Int parameters

| ID | Range | Default | UI |
|---|---|---|---|
| `satModel` | 0 – 10 | **1 (Tube)** | Combo: Tanh, Tube, Tape, Transistor, Transformer, Neural Neve, Neural API, Neural SSL, Neural Custom, WDF Tube, WDF Transformer |
| `glueScHpf` | 0 – 3 | 0 (Off) | Combo: Off, 60 Hz, 120 Hz, 250 Hz |
| `quality` | 0 – 3 | 1 (Standard 2×) | Combo: Eco / Standard / High / Ultra |
| `multibandCount` | 0 – 5 | 0 (1 band) | Combo: 1–6 Bands — **currently inert** (see audit) |
| `lfoCount` | 0 – 4 | 0 | No UI |

## Bool parameters

| ID | Default |
|---|---|
| `bypass` | false |
| `bypassMatch` | false (opt-in loudness-matched A/B) |
| `autoGain` | true |
| `midSide` | false |
| `resEnabled` | false |
| `transEnabled` | false |
| `simpleMode` | false |
| `targetLUFSLock` / `targetRMSLock` | false / false |
| `targetLowLock` / `targetMidLock` / `targetHighLock` | false × 3 |

## Notes for implementers

- **Smoothing taxonomy.** Three classes:
  1. **per-sample** (`next()` in a per-sample loop) — drive, warmth, punch,
     density, master (now a gain-ramp), resSens, resDepth.
  2. **block** (`advanceBlock(numSamples)` once per block) — mix, glue, shine,
     shineFreq, shineQ, width, motion, transientMix, boom.
  3. **raw read** (no smoother, read once per block from APVTS) — booleans,
     ints, glueAttack/Release/Ratio/Link, ceiling, transSens, target*. Where
     these can step audibly on automation, individual modules apply their
     own coefficient-recompute or attack/release smoothing.
- **Inert parameters** (`air`, `era`, `shineMix`, `toneMatchAmt`,
  `macro0..3`, `intensity`) have IDs reserved for future wiring; they
  serialize correctly and don't break anything, but currently have no
  audible effect. See the **Release Blueprint Audit §6** for the
  wire-or-remove decision.
- **`lfoCount` / Multiband** are similarly reserved.
