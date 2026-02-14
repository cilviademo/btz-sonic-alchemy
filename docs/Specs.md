# Box Tone Zone (BTZ) Technical Specs

## Product

- Name: Box Tone Zone (BTZ)
- Short Code: BTZ
- Manufacturer: BTZ Audio
- Framework: JUCE
- Targets: VST3, Standalone
- Platforms: Windows x64 (validated), macOS scripts included

## Core DSP Features

- Safety pre/post layers:
  - NaN/Inf sanitization
  - denormal suppression
  - DC blocking
- Saturation chain with controlled harmonic shaping
- BOOM low-band enhancement stage
- SHINE/AIR high-frequency emphasis stage
- SPARK limiter stage with gain reduction metering
- Mono-safe width processing (low-band widening clamp)

## Oversampling

- `qualityMode=0`: Eco, no oversampling
- `qualityMode=1`: 2x oversampling
- `qualityMode=2`: 4x oversampling
- Dynamic plugin latency reporting based on selected quality mode

## Frequency Response Tolerance Conditions

- Neutral reference condition:
  - `drive=0`, `mix=1`, `autogain=0`, defaults for all other parameters.
- Recommended acceptance target for neutral path validation:
  - approximately +/-0.5 dB average deviation across core band.

## Metering

Exposed to UI with atomic transport:

- Input Peak L/R
- Input RMS L/R
- Output Peak L/R
- Output RMS L/R
- SPARK gain reduction (dB)
- LUFS-style RMS proxy
- Clip indicators (in/out hold)
- Correlation estimate

## Real-Time Safety

- No heap allocation in `processBlock`
- No locks in audio thread
- GUI reads atomics only
- Parameter smoothing for automation safety

## Mono Compatibility

- Width stage applies low-band side constraint below approximately 120 Hz.
- Goal: preserve low-end coherence in mono fold-down while still allowing upper-band stereo width.

## Build

- CMake + JUCE plugin target (`juce_add_plugin`)
- Release/Debug builds expected via VS2022 x64 + Ninja on Windows
