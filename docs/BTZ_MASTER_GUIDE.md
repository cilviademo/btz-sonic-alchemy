# Box Tone Zone (BTZ) — Master Guide

A manual/guide-style reference to what the plugin is, how the signal flows, what
every control does, and the current build status. Grounded in the actual v1.0.2
source (`btz-sonic-alchemy-main/BTZ/Source`).

---

## 1. What BTZ is

Box Tone Zone is a **stereo tone-shaping / saturation / bus-dynamics plugin**
(VST3 / AU / Standalone, optional CLAP), built on JUCE 8. It combines, in one
stage: multi-model saturation, a glue (bus) compressor, a shine EQ, a true-peak
limiter, stereo width, M/S, an optional Target-Lock loudness corrector, plus
metering and visualizers. Three UI tiers (Simple / Standard / Advanced) progress
from three macro knobs to full control.

## 2. Signal flow (per block, stereo)

```
Input
 → Safety (DC block + NaN/denormal guard)
 → [Auto-gain measures input]
 → M/S encode (optional)
 → Resonance Tamer (optional)
 → Glue Compressor (sidechain-HPF'd detector)
 → Shine EQ (high peaking band)
 → [Oversample up ×2/4/8 if Quality>Eco]
 →   Saturation (drive → model → warmth → density), transient-aware
 → [Oversample down]
 → Auto-gain compensation
 → Target Lock (optional: LUFS/RMS/per-band correction)
 → True-Peak Limiter (4× ISP detection, ceiling)
 → M/S decode
 → Stereo width
 → Wet/Dry mix
 → Master gain
 → Safety (final)
 → Bypass crossfade (click-free)
 → [Delta monitor: output = wet − dry, optional]
 → Output  (+ correlation, meters, spectrum, GR history updated)
```

## 3. Controls & parameters (what each does)

### Character (Standard mode, left)
| Control | Param ID | Effect |
|---|---|---|
| Punch | `punch` | Transient energy / low-mid compression feel |
| Warmth | `warmth` | Blends low-frequency harmonic content back in |
| Boom | `boom` | Low-end weight; also feeds the transformer model's low-drive |
| Glue | `glue` | Amount of bus compression cohesion (drives threshold + makeup) |
| Air | `air` | High-frequency lift |
| Width | `width` | Stereo width: 0=mono, 0.5=normal, 1=wide (side low-passed to stay safe) |

### Core (Standard mode, center)
| Control | Param ID | Effect |
|---|---|---|
| Drive | `drive` | Master saturation amount (0→0 dB … 1→+30 dB into the shaper, auto-normalized) |
| Mix | `mix` | Wet/dry blend (parallel saturation) |
| Master | `master` | Output trim (±~12 dB around the 0.7 default) |

### Macros / voicing (Standard, bottom)
| Control | Param ID | Effect |
|---|---|---|
| Density | `density` | Parallel soft-clip character (body/sustain) |
| Motion | `motion` | Subtle 2 Hz LFO movement on drive |
| Era | `era` | Vintage↔modern voicing |
| Intensity | `intensity` | Overall processing intensity macro |

### Advanced
| Control | Param ID | Effect |
|---|---|---|
| Saturation Model | `satModel` | Selects 1 of 11 models (see §4) |
| Quality | `quality` | Oversampling: Eco(1×)/Standard(2×)/High(4×)/Ultra(8×) |
| Multiband Count | `multibandCount` | 1–6 bands (crossover engine present; see §8 status) |
| Glue SC HPF | `glueScHpf` | Compressor sidechain HPF: Off/60/120/250 Hz |
| Glue Attack/Release/Ratio/Link | `glueAttack` `glueRelease` `glueRatio` `glueLink` | Full SSL-style bus-comp control + 0–100% stereo link |
| Ceiling | `ceiling` | True-peak limiter ceiling (−12…0 dBTP, default −0.3) |
| Shine / Freq / Q / Mix | `shine` `shineFreq` `shineQ` `shineMix` | High peaking EQ band |
| Res Tame | `resSens` `resDepth` `resEnabled` | Dynamic resonance suppression |
| Transient | `transSens` `transMix` `transEnabled` | Protects transients from over-saturation |
| Mid/Side | `midSide` | Process in M/S instead of L/R |
| Auto Gain | `autoGain` | Level-match output to input |
| Bypass | `bypass` | Click-free bypass |

### Target Lock (Advanced) — `targetLUFS/RMS/DynThresh` + per-band `targetLow/Mid/HighDb` + `*Lock` toggles
Type a target loudness (LUFS or RMS) or per-band level, toggle LOCK, and the
engine nudges gain (and per-band gain via a 3-band crossover) toward the target.
The **Dynamics Threshold** knob sets how much dynamic range is preserved (0 =
brick-wall lock, 24 dB = gentle correction only).

## 4. Saturation models (`satModel`)
1. **Tanh** — clean symmetric soft clip (now bounded; see v1.0.2)
2. **Tube** — asymmetric, adds even harmonics
3. **Tape** — hysteresis-style with memory
4. **Transistor** — asymmetric thresholds, odd harmonics
5. **Transformer** — low-drive dependent, even harmonics
6–9. **Neural Neve / API / SSL / Custom** — RTNeural-style GRU slots. *No weight
   files ship yet → these currently fall back to tanh.*
10. **WDF Tube** / 11. **WDF Transformer** — heuristic circuit-style shapers.

## 5. Metering & safety
- **Input/Output peak** ballistics with hold.
- **GR** (gain reduction) value + scrolling ribbon.
- **LUFS** (momentary/short-term) — BS.1770-4 **K-weighted** (high-shelf @1681 Hz
  + HPF @38 Hz). Integrated/LRA are approximate (not full gated R128).
- **True Peak** — limiter uses 4× polyphase ISP detection. (Footer TP readout is
  currently sample-peak — see status.)
- **Correlation** — +1 mono-safe / ~0 wide / <0 phase risk; drives the safety badge.
- **SafetyLayer** — DC block + NaN/Inf/denormal guard on input and output.

## 6. UI modes
- **Simple** — Drive, Character (=shine), Mix/Output + harmonic bloom. Knobs are
  interactive and drive the underlying params.
- **Standard** — 6 character knobs + Drive/Mix/Master + 4 macros + meters + viz.
- **Advanced** — full model/quality/multiband/HPF selectors, glue detail knobs,
  res/transient/ceiling/shine, Target Lock panel, spectrum + preset browser.

All colours/spacing come from the **Ivory System** theme (`BTZTheme.h`): warm
ivory canvas, orange primary accent, sage secondary, gold/clay/teal supporting.

## 7. Build & run
See `docs/dev/BTZ_LOCAL_DEV_SETUP.md`. Quick:
```bash
scripts/build_macos.sh Release --tests      # macOS
pwsh scripts/build_windows.ps1 -Tests        # Windows
scripts/build_linux.sh Release --tests       # Linux
```
Validate: `scripts/run_pluginval.sh 10`. Test in Reaper (see Cursor handoff).

## 8. Current status (v1.0.2)
**Verified:** plugin + tests compile and link against real JUCE 8.0.6 on Linux;
81/84 unit tests pass; fastTanh bounded; correlation wired; reset() lifecycle added.

**Open / not yet wired:**
- Spectrum + harmonic **visualizers render empty** (no live FFT feed yet).
- **MultibandEngine** prepared but `split/recombine` not invoked in the audio path
  (the `multibandCount` selector is inert until wired).
- **Neural weights** absent (4 slots fall back to tanh).
- **3 known test failures** are design questions (LR4 reconstruction semantics ×2,
  SafetyLayer clamp policy) — see `docs/dev/BTZ_BUILD_AND_TEST_STATUS.md`.
- **Bespoke OversamplingEngine / ReferenceToneMatcher / PresetIntelligence /
  macro routing** are present but not in the signal path (future-facing).
- No factory presets, installers, or code signing yet.

**Roadmap pointers:** `SENIOR_DEV_AUDIT_v1.0.1.md` (deep audit),
`docs/dev/BTZ_VALIDATION_CHECKLIST.md` (release gate),
`docs/dev/BTZ_CURSOR_HANDOFF.md` (next local steps).
