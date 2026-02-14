# BTZ Research Digest (2023-2026 Applied Summary)

## 1) Transient Shaping and Punch

High-value pattern: crest-aware transient shaping rather than peak-only shaping.

- Use dual envelope tracks (peak + RMS) to derive crest behavior.
- Apply dynamic odd/even weighting from crest ratio to increase punch without brittle click.
- Keep attack emphasis bounded and smoothed to avoid over-fragmentation on dense material.

Applied to BTZ:

- `PluginProcessor.cpp` uses peak/RMS envelope paths and crest-informed harmonic emphasis.

## 2) Saturation and Warmth

Recommended architecture:

- Multi-stage gentle saturation (preamp + split-band saturation + density stage).
- Separate macro intensity from stage drive for controllable behavior.
- Oversample around nonlinear sections to reduce fold-back.

Applied to BTZ:

- Nonlinear stages are processed through selectable oversampling modes (Eco/2x/4x).

## 3) Low-End Enhancement

Best-practice priorities:

- Maintain phase stability in low-frequency processing.
- Prevent mono collapse by limiting low-band side gain.
- Prefer dynamic enhancement tied to existing low-band content.

Applied to BTZ:

- Width stage clamps low-band side scaling, while preserving high-band width control.

## 4) Clipping/Limiting

For musical loudness workflows:

- Soft-clip topology plus ceiling stage for tone + control.
- True-peak-safe strategy requires conservative ceilings and anti-alias support.
- Use gain-reduction metering with sensible attack/release for user confidence.

Applied to BTZ:

- SPARK stage with GR tracking and dedicated meter channel.

## 5) Adaptive / AI-Assisted Direction

Recommended real-time-safe feature extraction:

- transient energy, crest factor, spectral centroid proxy, loudness proxy.
- bounded adaptation only, with hard limits and smoothing.

Current BTZ status:

- Core architecture is prepared for deterministic adaptive features without introducing non-RT dependencies.

## 6) JUCE/VST3 Engineering Notes

- Keep APVTS parameter IDs stable.
- Use atomics for GUI metering transport.
- Keep process path allocation-free.
- Report plugin latency whenever oversampling mode changes.
