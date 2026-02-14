# Metering Design (BTZ)

## Signals Published From Audio Thread

Published with `std::atomic<float>`:

- Input Peak L/R (dBFS)
- Input RMS L/R (dBFS)
- Output Peak L/R (dBFS)
- Output RMS L/R (dBFS)
- SPARK gain reduction (dB)
- LUFS proxy (RMS-derived dB value)
- Input clip indicator hold
- Output clip indicator hold
- Correlation estimate

## Ballistics

- Peak hold decay:
  - `hold = max(blockPeak, hold * 0.995)`
- RMS smoothing:
  - one-pole smoothing coefficient `0.08`
- SPARK GR smoothing:
  - one-pole coefficient `0.2`
- Clip indicator hold:
  - `hold = max(clipEvent ? 1.0 : 0.0, hold * 0.92)`

## dB Conversion

- `gainToDecibels(value, -100.0f)` used for peak/RMS/LUFS display floor.
- GR is shown as positive dB attenuation amount.

## Correlation

- Computed as:
  - `corr = sum(L*R) / sqrt(sum(L^2) * sum(R^2) + eps)`
- Clamped to `[-1.0, 1.0]`.

## GUI Update Model

- GUI polls meter atomics via `Timer` at 45 Hz.
- GUI smoothing is applied using simple linear interpolation for stable visuals.
- No audio thread access from GUI.
- All painting runs on message thread only.

## Clip Rules

- Clip event when `abs(sample) >= 0.999` on corresponding stream.
- Visual indicator is held and decays gradually.
