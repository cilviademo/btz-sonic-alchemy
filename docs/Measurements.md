# Box Tone Zone (BTZ) Measurements

## Scope

This document defines repeatable measurements for the current BTZ engine:

- Frequency neutrality at neutral settings
- Nonlinear alias control with oversampling modes
- SPARK limiter gain reduction behavior
- Stereo/mono compatibility

Suggested tools:

- PluginDoctor for transfer, harmonic, and aliasing inspection
- REW for sweep verification and comparative transfer checks

## Test Conditions

- Sample rates: 44.1 kHz, 48 kHz, 96 kHz
- Block sizes: 64, 128, 256, 512
- Modes:
  - `qualityMode=0`: Eco (no oversampling)
  - `qualityMode=1`: 2x oversampling
  - `qualityMode=2`: 4x oversampling

## Frequency Neutrality Procedure

1. Set all macro controls to default values.
2. Set `drive=0`, `mix=1`, `autogain=0`, `masterIntensity` default.
3. Feed pink noise at -18 dBFS RMS for 30 seconds.
4. Measure transfer curve with 1/12 octave smoothing.

Expected result:

- Broadband response around neutral target with low coloration.
- Target acceptance: average deviation within +/-0.5 dB over 30 Hz to 16 kHz.

## Aliasing Procedure

1. Feed 1 kHz sine at -6 dBFS.
2. Set high nonlinearity (Warmth, Density, Shine, Spark Mix high).
3. Compare output spectra for Eco/2x/4x.

Expected result:

- 2x and 4x reduce fold-back energy versus Eco.
- 4x gives best suppression in upper band.

## SPARK Meter Ballistics

SPARK gain reduction meter:

- Attack: 8 ms equivalent
- Release: 120 ms equivalent
- Transfer: instantaneous reduction estimate smoothed with attack/release envelope

Verification:

- Step input into limiter threshold produces fast GR rise and musically smooth release.

## Mono Compatibility Procedure

1. Feed stereo low-frequency-rich material.
2. Increase Width to maximum.
3. Sum output to mono and compare LF energy.

Expected result:

- Low band widening is constrained below ~120 Hz.
- Minimal low-end cancellation under mono sum.
