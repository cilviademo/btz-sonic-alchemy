# Box Tone Zone (BTZ) Competitive Analysis

## Compared Processors

- iZotope Ozone (Dynamics/Exciter/Imager conceptual reference)
- FabFilter Pro-MB (multiband dynamic control reference)
- SSL/Cytomic glue-style bus processors (glue and cohesion reference)

## Where BTZ Wins

- Fast workflow for drum/bus enhancement with macro-style controls.
- Integrated BOOM/SHINE/SPARK voicing focused on punch and excitement.
- Built-in mono-safe width strategy to protect low-end in mono.
- Lightweight metering directly tied to plugin behavior.

## Feature Matrix (Conceptual)

| Area | BTZ | Ozone | Pro-MB | SSL/Cytomic Glue |
|---|---|---|---|---|
| Drum-focused macro workflow | Strong | Medium | Low | Medium |
| Integrated punch/warmth/boom/shine chain | Strong | Medium | Low | Medium |
| Dedicated multiband dynamics depth | Medium | Strong | Strong | Low |
| Loudness guard and clip workflow | Medium-Strong | Strong | Medium | Medium |
| Mix speed for producers | Strong | Medium | Medium | Medium |

## Current Weaknesses

- BTZ is intentionally narrower in scope than full mastering suites.
- No full dynamic multiband UI and routing depth comparable to Pro-MB.
- SPARK is a simplified limiter stage; not a full true-peak lookahead limiter design.

## Improvements Implemented In This Upgrade

- Added 2x/4x oversampling modes and latency reporting.
- Added professional metering transport:
  - input/output peak and RMS (L/R),
  - SPARK GR meter,
  - clip hold indicators,
  - correlation estimate.
- Hardened DSP safety:
  - removed static shared filter state,
  - maintained denormal/NaN guards,
  - improved mono compatibility in the width stage.
- Standardized brand metadata to BTZ Audio and Box Tone Zone naming.

## Next Competitive Steps

- Add full true-peak lookahead limiting and ISP-safe verification.
- Add deeper analyzer views (spectrum/history/loudness timeline).
- Add automated QA measurements in CI for regression tracking.
