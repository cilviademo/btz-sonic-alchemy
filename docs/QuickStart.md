# Box Tone Zone (BTZ) Quick Start

## Overview

Box Tone Zone (BTZ) is a JUCE-based enhancement plugin for punch, warmth, width, and loudness-safe finishing.

## Install

1. Build BTZ (Release) from scripts or CMake.
2. Copy `Box Tone Zone (BTZ).vst3` to:
   - Windows: `C:\Program Files\Common Files\VST3`
3. Rescan plugins in your DAW.

## Three Workflows

1. Drum Bus Tighten
   - Start with Punch, Glue, Width around 0.4-0.6.
   - Keep Spark Mix high for safety.

2. Presence Lift
   - Increase Air and Shine gradually.
   - Use 2x/4x quality mode for cleaner highs.

3. Loudness Guard
   - Raise Drive/Density while watching SPARK GR and clip indicators.
   - Keep Output Peak below target ceiling.

## Gain Staging

- Feed around -18 dBFS RMS for balanced response.
- Use Mix for parallel blend.
- Leave AutoGain enabled for quick leveling; disable for strict A/B.
- If mono translation is critical, reduce Width and keep low-end in center.

## Oversampling Guidance

- Eco (`qualityMode=0`): lowest CPU, highest alias risk.
- 2x (`qualityMode=1`): recommended default.
- 4x (`qualityMode=2`): highest quality, higher CPU and latency.
