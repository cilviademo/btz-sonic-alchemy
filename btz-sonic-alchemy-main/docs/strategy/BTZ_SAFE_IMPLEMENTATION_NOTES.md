# BTZ Sonic Alchemy — Safe Implementation Notes

This document outlines surgical, low-risk changes that can be made immediately to improve the UX and trust of the plugin without risking the stability of the DSP or architecture.

## 1. Improve Labels & Naming

**Current:** `Output` (in Simple Mode)
**New:** `Target LUFS`
*Why:* Forces the user to interact with the flagship feature immediately.

**Current:** `Resonance Tamer`
**New:** `Resonance Suppression`
*Why:* Sounds more clinical and professional.

**Current:** `Neural 1`, `Neural 2`
**New:** `Neural: Modern Console`, `Neural: Vintage Tube`
*Why:* Gives the user an expectation of the sound profile.

## 2. Improve Defaults

**Current:** All knobs at 0, Target Lock off.
**New:**
- Saturation Model: `Tape`
- Drive: `15%`
- Target Lock: `On`
- Target LUFS: `-14.0`
- Dynamics Threshold: `6.0 dB`
*Why:* Provides an "instant good sound" that hits streaming targets out of the box.

## 3. Improve Trust Indicators

**Current:** `Delta` button.
**New:** `Delta (Listen to Harmonics)` tooltip. When engaged, dim the UI background by 10% to visually indicate a diagnostic mode.
*Why:* Delta listening is a pro feature; the UI should reflect that the user is "under the hood."

## 4. Improve Loudness Matching UX

**Current:** Standard Bypass button.
**New:** `Bypass (Loudness Matched)` toggle in the settings. When active, bypassing the plugin automatically applies a gain offset to the dry signal to match the LUFS of the processed signal.
*Why:* Proves the plugin is improving tone, not just volume.

## 5. What NOT to Do

- **DO NOT** rewrite the `TargetLockEngine` in `BTZDsp.h`. It is mathematically sound.
- **DO NOT** change the `PluginProcessor` threading model. The atomic latency reporting and lock-free audio thread are production-grade.
- **DO NOT** add an EQ section. It dilutes the product identity.
- **DO NOT** change the Ivory System color palette. It is a strong differentiator.
