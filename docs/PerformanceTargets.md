# BTZ Performance Targets

## CPU Targets (Windows x64, modern desktop)

At 48 kHz, stereo:

- Eco mode (`qualityMode=0`): under 1.0% typical DAW CPU per instance
- 2x mode (`qualityMode=1`): under 2.0% typical DAW CPU per instance
- 4x HQ (`qualityMode=2`): under 3.5% typical DAW CPU per instance

Targets should be validated at 64, 128, 256 sample buffers.

## Latency Targets

- Eco: 0 samples expected (non-oversampled path)
- 2x/4x: report exact oversampling latency via `setLatencySamples(...)`
- Host compensation must update when quality mode changes.

## Real-Time Safety Targets

- No heap allocation in `processBlock`.
- No locks in audio callback.
- No synchronous disk/network I/O in audio callback.
- Denormal protection enabled.

## Metering / UI Targets

- Meter updates 30-60 Hz (current 45 Hz).
- No GPU/CPU-heavy repaint loops.
- UI must consume atomics only (no direct audio buffer reads).

## QA Targets

- Session recall correctness across DAW restart.
- Automation sweep stability without zipper noise.
- Mono fold-down check: low-end remains stable with width engaged.
- Peak/clip and GR meters remain stable across sample-rate changes.
