# Box Tone Zone (BTZ) User Manual

## 1. Product Overview

Box Tone Zone (BTZ) is a JUCE-based enhancement plugin for drums, percussion buses, and rhythm-focused material.  
It combines punch shaping, harmonic saturation, low-end reinforcement, stereo image control, and loudness-safe finishing.

Intended users:

- Mixing engineers who need fast punch/warmth shaping.
- Producers who want modern loudness with controlled artifacts.
- Home studio users who need consistent results across headphones and speakers.

## 2. Core Workflow

1. Start at conservative levels (`drive=0`, `mix=1`, `quality=1`).
2. Dial body and impact with Punch, Warmth, Boom, Glue.
3. Add presence with Air and Shine.
4. Control loudness and peak safety with SPARK controls.
5. Check mono translation with Width and correlation meter.

## 3. Signal Flow

Text signal diagram:

`Input -> SafetyPre -> Drive -> Preamp/Color -> Band Sat -> Glue -> Width (mono-safe low band) -> Air/Shine -> Density -> SPARK -> SafetyPost -> Mix -> AutoGain -> Output`

Design intent:

- Safety layers sanitize invalid floating point values and suppress denormals.
- Tone stages are musical and bounded.
- SPARK provides guardrail behavior rather than aggressive brickwall limiting.

## 4. Parameter Reference

### Main Tone Parameters

- `Punch` (0.0..1.0, default 0.18): transient and attack-energy emphasis.
- `Warmth` (0.0..1.0, default 0.22): harmonic tone shaping.
- `Boom` (0.0..1.0, default 0.10): low-band reinforcement.
- `Glue` (0.0..1.0, default 0.25): bus-style dynamic cohesion.
- `Air` (0.0..1.0, default 0.12): high-frequency lift.
- `Width` (0.0..1.0, default 0.50): stereo width amount.
- `Density` (0.0..1.0, default 0.16): nonlinear density.
- `Motion` (0.0..1.0, default 0.04): analog-style noise motion.
- `Era` (-1.0..1.0, default 0.0): vintage/modern contour bias.

### Output and Safety

- `Drive` (0..12 dB, default 0): input drive into color chain.
- `Mix` (0.0..1.0, default 1.0): wet/dry blend.
- `Master` (0.0..1.0, default 0.42): macro intensity scale.
- `TP Ceil` (-3..0 dB, default -0.3): SPARK ceiling.
- `Spark Mix` (0.0..1.0, default 1.0): SPARK blend.
- `Shine` (0..6, default 1.2): additional presence contour.
- `Shine Mix` (0.0..1.0, default 0.30): Shine blend amount.
- `AutoGain` (0/1, default 1): output level compensation.
- `Quality` (0/1/2, default 1): Eco/2x/4x processing quality.
- `Character` (0/1, default 1): character slot for future voicing expansion.
- `Bypass` (0/1, default 0): hard bypass.

## 5. Metering

The plugin provides:

- Input Peak L/R
- Input RMS L/R
- Output Peak L/R
- Output RMS L/R
- SPARK gain reduction (dB)
- LUFS proxy
- Input and output clip indicators
- Stereo correlation estimate

UI refresh:

- 45 Hz timer-driven updates on GUI thread.
- Data transfer via atomics only.

## 6. Oversampling and Latency

Quality modes:

- `0` = Eco (no oversampling)
- `1` = 2x oversampling (recommended default)
- `2` = 4x oversampling (best anti-aliasing, higher CPU)

Latency:

- Updated dynamically based on selected quality mode.
- Reported to host via plugin latency API.

## 7. Mono Compatibility

BTZ constrains low-band side widening below approximately 120 Hz to reduce mono cancellation risk.

Practical guidance:

- For club playback and mono-critical environments, keep Width near center.
- Watch correlation during aggressive widening.
- If low-end feels unstable in mono, reduce Width and Boom together.

## 8. Preset and Session Compatibility

- Parameter IDs are preserved for existing sessions.
- APVTS state is serialized/restored for DAW recall.
- Automation playback uses smoothed parameter transitions.

## 9. Installation

### Windows VST3

1. Build Release (`scripts\build_windows_with_vs_env.bat`).
2. Copy/install `Box Tone Zone (BTZ).vst3` to:
   `C:\Program Files\Common Files\VST3`
3. Rescan plugins in your DAW.

### Standalone

Run:

- Release: `.../BTZ_artefacts/Release/Standalone/Box Tone Zone (BTZ).exe`
- Debug: `.../BTZ_artefacts/Debug/Standalone/Box Tone Zone (BTZ).exe`

## 10. Typical Use Cases

### Drum Bus Tightening

- Increase Punch + Glue moderately.
- Add Boom carefully to avoid mud.
- Keep Spark Mix high for peak safety.

### Presence and Air

- Raise Air first, then Shine.
- Use Quality 1 or 2 when driving highs hard.

### Loudness-Forward Bus

- Use Drive and Density gradually.
- Watch Output Peak and SPARK GR.
- Use AutoGain for level-matched decisions.

## 11. Troubleshooting

- Plugin not found:
  - verify VST3 install path
  - rescan plugin database
- Build failure:
  - use x64 VS developer environment
  - confirm JUCE path env var or fallback path
- CPU spikes:
  - reduce quality mode
  - increase DAW buffer
- Crackles:
  - lower Drive/Density
  - check for accidental clipping in source chain

## 12. Additional Technical References

- `docs/Build.md`
- `docs/Metering.md`
- `docs/Specs.md`
- `docs/Measurements.md`
- `docs/CompetitiveAnalysis.md`
- `docs/ResearchDigest.md`
- `docs/ImplementationRoadmap.md`
- `docs/PerformanceTargets.md`
