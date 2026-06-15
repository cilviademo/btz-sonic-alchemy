# BTZ Sonic Alchemy — Testing & Validation Checklist

This document provides a systematic guide for validating the DSP and UI behavior of the BTZ plugin. Use this checklist during QA to ensure every feature sounds and acts exactly as intended.

---

## 1. Core Saturation & Drive

**Test Setup:** Feed a clean, dynamic drum loop or a pure sine wave (1kHz) into the plugin.

| Feature | Action | What You Should Hear / See |
|---------|--------|----------------------------|
| **Drive Knob** | Turn from 0% to 100% | The signal should become increasingly distorted and harmonically rich. **Crucially, the overall perceived volume should remain relatively constant** due to the Auto-Gain system. You should hear density, not just loudness. |
| **Tube Model** | Select "Tube", push Drive | Warm, thick saturation. On a sine wave, you should see strong 2nd and 4th order (even) harmonics in the visualizer. Transients should round off smoothly. |
| **Tape Model** | Select "Tape", push Drive | Symmetrical clipping. You should hear odd harmonics (3rd, 5th) and a slight softening of the extreme high frequencies. |
| **Transformer** | Select "Transformer", feed low freq | You should hear low-frequency weight and hysteresis. The sub frequencies should feel "anchored" and slightly compressed. |
| **Transistor** | Select "Transistor", push Drive | Aggressive, biting distortion. The top end should sound fizzy and the clipping should feel harder than the Tube model. |

---

## 2. Dynamics & Tone Shaping

**Test Setup:** Feed a full mix bus or a complex synth chord progression.

| Feature | Action | What You Should Hear / See |
|---------|--------|----------------------------|
| **Tone Knob** | Sweep from 0% to 100% | At 0%, the signal should sound dark and muffled. At 50%, it should be neutral. At 100%, it should sound bright and airy, but harsh resonant peaks should be dynamically suppressed (not just a static EQ boost). |
| **Punch Knob** | Turn from 0% to 100% on drums | At 0%, the drum transients should be flattened by the saturation. At 100%, the initial stick attack of the snare should poke through cleanly before the saturation clamps down on the tail. |
| **Density (Glue)** | Increase Density on a mix bus | The mix should feel more cohesive. You should hear the quiet details come up and the loud peaks tuck in. The gain reduction meter should show 2-4dB of movement. |
| **Sidechain HPF** | Toggle HPF while compressing | With HPF on, a heavy kick drum should no longer cause the entire mix to "pump" or duck. The compression should react more to the snare and vocals. |

---

## 3. Advanced Processing

**Test Setup:** Feed a wide stereo acoustic guitar or piano track.

| Feature | Action | What You Should Hear / See |
|---------|--------|----------------------------|
| **Multiband Split** | Enable 2 bands, solo the low band | You should hear only the frequencies below the crossover point. The crossover should be completely transparent (no phase smearing) when bands are summed back together. |
| **Mid/Side Mode** | Enable M/S, push Drive on Side | The center of the image (vocals, kick, bass) should remain clean, while the wide stereo information (reverbs, wide guitars) becomes saturated and harmonically rich. |
| **Width Slider** | Sweep from 0% to 200% | At 0%, the signal should collapse to perfect mono. At 100%, it should be unchanged. At 200%, the side information should be exaggerated, making the mix sound artificially wide. |
| **LFO Modulation** | Assign LFO to Drive, set to Sine | You should hear the saturation amount rhythmically swelling in and out. The UI Drive knob should visually indicate the modulation range. |

---

## 4. Utility & Metering

**Test Setup:** Feed any commercial reference track.

| Feature | Action | What You Should Hear / See |
|---------|--------|----------------------------|
| **Delta (Δ) Mode** | Click the Delta button | You should hear *only* the difference between the wet and dry signals. If Drive is at 0%, you should hear silence. If Drive is high, you should hear a thin, fizzy, distorted signal (the generated harmonics). |
| **Mix Slider** | Sweep from 0% to 100% | The transition should be perfectly smooth. There should be absolutely no comb filtering or phase cancellation (hollow sound) during the sweep, proving latency compensation is correct. |
| **True Peak Limiter** | Push Master output, set Ceiling to -1dB | The output meter should never exceed -1.0 dBTP, no matter how hard you push the input. You should not hear digital clipping clicks. |
| **Correlation Meter** | Feed a mono signal, then a wide signal | Mono signal: Meter reads +1. Wide signal: Meter hovers around 0 to +0.5. Out-of-phase signal: Meter drops below 0. |
| **Bypass** | Toggle Bypass on and off | The transition should be click-free (a fast 64-sample crossfade occurs internally). |

---

## 5. UI & Interaction

| Feature | Action | What You Should Hear / See |
|---------|--------|----------------------------|
| **Mode Switching** | Click Simple, Standard, Advanced tabs | The UI should instantly resize and reveal/hide controls. Audio should not glitch or drop out during the switch. |
| **Knob Interaction** | Drag a knob up/down | The movement should feel smooth. Holding Shift while dragging should enable fine-tuning (slower movement). Double-clicking should reset to default. |
| **Undo/Redo** | Change a parameter, click Undo | The parameter should revert to its previous state. Audio should reflect the change immediately. |
| **A/B Comparison** | Set state A, click B, change state, click A | You should be able to toggle between two completely different plugin states instantly without audio dropouts. |
