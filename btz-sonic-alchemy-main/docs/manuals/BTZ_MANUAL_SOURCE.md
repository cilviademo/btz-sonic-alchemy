# Box Tone Zone (BTZ) — Sonic Alchemy
**User Manual v1.0**

*Designed for Tone. Built for Mixes.*

---

## 1. Welcome to the Zone

Thank you for choosing BTZ Sonic Alchemy. 

We built BTZ because we love the sound of analog hardware, but we work in modern DAWs. We wanted the weight of a transformer, the glue of a classic bus compressor, and the air of a boutique EQ—all without the workflow friction of chaining five different plugins together.

BTZ is not a transparent utility. It is a character box. Every saturation model, every filter curve, and every dynamic response has been tuned to impart musical color. Whether you are looking for subtle mix bus glue or aggressive drum destruction, BTZ provides the harmonic intelligence to get you there quickly.

Welcome to your new favorite tone box.

---

## 2. Quick Start Guide

**How to get great results in under 60 seconds:**

1. **Insert the Plugin:** Place BTZ on a track, bus, or master fader.
2. **Choose a Preset:** Click the preset name in the top header to open the browser. Select a starting point (e.g., "Warm Tape Glue" for a mix bus).
3. **Adjust Drive:** Turn the large terracotta **Drive** knob until you hear the desired amount of saturation. Watch the Harmonic Visualizer to see the overtones bloom.
4. **Shape the Tone:** Use the sage green **Tone** knob to balance the frequency response.
5. **Blend to Taste:** Adjust the **Mix** slider if you want parallel processing.
6. **Check Loudness:** Look at the footer. The LUFS meter ensures you aren't fooling yourself with volume changes.
7. **Compare:** Click the **A/B** button to compare your settings against the dry signal or an alternate state.

*[INSERT ASSET: QUICK_START_DIAGRAM.png]*

---

## 3. Installation & Setup

### Supported Formats
BTZ Sonic Alchemy is available in the following formats:
- **macOS (10.14+)**: VST3, AU, CLAP, Standalone (Apple Silicon & Intel native)
- **Windows (10+)**: VST3, CLAP, Standalone

### Installation
1. Run the provided installer for your operating system.
2. The installer will place the plugin files in the standard system directories:
   - **macOS VST3:** `/Library/Audio/Plug-Ins/VST3/`
   - **macOS AU:** `/Library/Audio/Plug-Ins/Components/`
   - **Windows VST3:** `C:\Program Files\Common Files\VST3\`
3. Restart your DAW. BTZ will appear under the manufacturer name **BTZ Audio**.

---

## 4. User Interface Overview

BTZ features a progressive disclosure interface. We believe that tools should scale with your intent. Sometimes you just need three knobs; sometimes you need a surgical multiband crossover. BTZ offers three view modes, selectable via the tabs in the top header.

### The Ivory System Aesthetic
The interface uses our custom Ivory System design language. 
- **Terracotta** elements control saturation and drive.
- **Sage Green** elements control dynamics and EQ.
- **Charcoal** text on **Parchment** surfaces ensures readability without eye strain.

*[INSERT ASSET: UI_OVERVIEW_MODES.png]*

---

## 5. Simple Mode

Simple Mode is designed for speed and intuition. It abstracts complex DSP into three macro controls.

*[INSERT ASSET: SIMPLE_MODE_UI.png]*

### Drive (Terracotta)
Controls the input gain into the selected saturation model. As you turn this up, the signal is pushed harder against the nonlinear curve, generating harmonics. The internal auto-gain system automatically compensates for the volume increase, so you only hear the tonal change, not a level jump.

### Tone (Sage Green)
A macro control that simultaneously adjusts the Shine EQ and the Resonance Tamer. 
- **Below 50%:** Darkens the signal, rolling off highs and emphasizing warmth.
- **Above 50%:** Adds air and presence while dynamically suppressing harsh resonant peaks.

### Output (Charcoal)
The final trim control before the signal leaves the plugin.

### Harmonic Visualizer
The central display shows the real-time harmonic content being generated. Watch the overtones rise as you push the Drive knob.

---

## 6. Standard Mode

Standard Mode is the working producer's view. It exposes the core character controls without the distraction of surgical routing.

*[INSERT ASSET: STANDARD_MODE_UI.png]*

### Character Knobs
- **Punch:** Controls the attack behavior of the internal transient splitter. Higher values let more transients bypass the saturation, preserving impact.
- **Warmth:** Adjusts the low-mid harmonic focus of the saturation model.
- **Boom:** A targeted sub-harmonic enhancer.
- **Density:** Controls the ratio and threshold of the Glue Compressor.
- **Motion:** Adjusts the depth of the internal LFO modulating the drive circuit.
- **Air:** Controls the gain of the TDF-II Shine EQ.

### Mix & Master Sliders
- **Mix:** A true wet/dry blend. BTZ uses latency-compensated parallel paths to ensure zero phasing when blending.
- **Master:** The final output ceiling.

---

## 7. Advanced Mode

Advanced Mode opens the hood. This is the sound designer's toolkit, providing full access to the multiband crossover, neural models, and detailed metering.

*[INSERT ASSET: ADVANCED_MODE_UI.png]*

### Neural Model Browser
Select between 9 distinct saturation models (5 analog algorithmic models and 4 RTNeural GRU slots). See Section 8 for details.

### Multiband Crossover
BTZ features a Linkwitz-Riley 4th-order (LR4) crossover network, allowing up to 6 bands of independent processing. 
- Drag the vertical splitters in the spectrum display to set crossover frequencies.
- Select a band to adjust its specific Drive, Tone, and Mix settings.

### LFO Modulation
Assign the internal LFO to modulate parameters. Select between Sine, Triangle, Square, and Sample & Hold shapes.

### True Peak Limiter
The final stage of the plugin is an 8-sample lookahead True Peak limiter. Use the **Ceiling** control to set the absolute maximum output level, preventing inter-sample clipping during D/A conversion.

---

## 8. Saturation Models

BTZ includes 9 distinct saturation profiles.

### Algorithmic Models
1. **Tube:** Asymmetrical clipping with strong even-order harmonics. Warm, thick, and slightly rounded transients. Ideal for bass and vocals.
2. **Tape:** Symmetrical soft-clipping with odd-order harmonics and a subtle high-frequency roll-off. Excellent for mix bus glue.
3. **Transformer:** Low-frequency focused saturation with hysteresis modeling. Adds weight and iron to kicks and drum buses.
4. **Transistor:** Aggressive, fast-clipping edge. Great for parallel drum crushing or aggressive guitars.
5. **Console:** Extremely subtle, high-headroom saturation. Use this across every channel in a mix for cumulative analog summing vibe.

### Neural Models (RTNeural)
BTZ includes 4 slots for machine-learned GRU models of specific hardware units. 
*(Note: Neural models require slightly more CPU than algorithmic models).*

*[INSERT ASSET: SATURATION_MODELS_GRAPH.png]*

---

## 9. Metering & Loudness

Proper gain staging is critical for modern production. BTZ provides comprehensive metering in the footer bar.

*[INSERT ASSET: METERING_FOOTER.png]*

- **LUFS (EBU R128):** Displays Momentary, Short-Term, and Integrated loudness. Use this to ensure your mix meets streaming platform targets (typically around -14 LUFS).
- **Correlation Meter:** Ensures your stereo image is mono-compatible. +1 is perfect mono, 0 is wide stereo, -1 is out of phase.
- **Gain Reduction History:** In Advanced Mode, a scrolling 60-second ribbon displays the exact behavior of the Glue Compressor and True Peak Limiter.
- **Delta Monitoring:** Click the **Δ (Delta)** button in the header to hear *only* what the plugin is adding or removing from the signal (Wet minus Dry).

---

## 10. DSP Architecture & Performance

BTZ is built on a lock-free, zero-allocation audio thread. 

### Anti-Aliasing
Nonlinear processing generates harmonics that can reflect off the Nyquist frequency, causing harsh digital aliasing. BTZ uses a combination of Anti-Derivative Anti-Aliasing (ADAA) and an optional Oversampling Engine (up to 8x) to ensure pristine high frequencies.

### Auto-Gain
The internal Auto-Gain Smoother measures the RMS input level and matches the output level. This prevents the "louder is better" psychoacoustic illusion, allowing you to make objective mixing decisions.

### CPU Optimization
- **Draft Mode:** Runs at 1x sample rate. Ideal for tracking and composing.
- **HQ Mode:** Engages oversampling. Recommended for rendering and mastering.

*[INSERT ASSET: DSP_SIGNAL_FLOW.png]*

---

## 11. Troubleshooting

**No Audio Output:**
- Check the Bypass toggle in the header.
- Ensure the Master slider is not at -inf.
- Check if the plugin has triggered the Safety Layer (the indicator in the footer will turn red if it caught a NaN/Inf value).

**High CPU Usage:**
- Switch from a Neural model to an Algorithmic model.
- Reduce the oversampling rate.
- Disable unused multiband splits.

**Clicking during Automation:**
- BTZ uses parameter smoothing, but extreme, instantaneous jumps on the Crossover frequencies may cause artifacts. Smooth your automation curves in the DAW.

---

## 12. Appendix: Parameter List

| Parameter | ID | Range | Description |
|-----------|----|-------|-------------|
| Drive | `drive` | 0 to 100% | Input gain to saturation |
| Mix | `mix` | 0 to 100% | Wet/Dry blend |
| Master | `master` | -24 to +12 dB | Final output trim |
| Model | `model` | 0 to 8 | Saturation model selection |
| Ceiling | `ceiling` | -12 to 0 dBTP | True Peak limiter ceiling |
| Intensity | `intensity` | 0 to 100% | Global effect scaling |

---

*BTZ Sonic Alchemy is a trademark of BTZ Audio. All rights reserved.*
