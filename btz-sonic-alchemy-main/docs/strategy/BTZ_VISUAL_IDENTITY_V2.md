# BTZ Sonic Alchemy — Visual Identity Evolution (Ivory System V2)

## 1. The Ivory System Philosophy

The current Ivory System (warm cream backgrounds, parchment panels, sage green and terracotta accents, charcoal text) is highly successful. It feels premium, analog-inspired but distinctly modern, and avoids the cliché "dark mode neon" aesthetic of typical EDM plugins.

**The Goal:** Do not redesign. Refine, strengthen, and animate.

## 2. Signature Visual Elements

**1. The Harmonic Bloom Visualizer (The Hook)**
- **Current:** A standard FFT spectrum or waveform.
- **Evolution:** The visualizer should become the signature branding element. When saturation is applied, the visualizer shouldn't just show a static EQ curve; it should show a "bloom" — a glowing, terracotta-tinted aura that expands upward from the fundamental frequencies, visually representing the added harmonics.
- **Why:** It makes the invisible (harmonics) visible and beautiful.

**2. Professional Ballistics**
- **Current:** Standard meter smoothing.
- **Evolution:** Implement precise, industry-standard ballistics for all meters. The LUFS meter must move with the exact integration times specified by ITU-R BS.1770. The TruePeak meter must have a fast attack (0ms) and a smooth, exponential release (e.g., 1.5 seconds/20dB) to feel trustworthy and expensive.

**3. UI Trust Signals**
- **Current:** Clean typography.
- **Evolution:** Add subtle "engineering credibility" indicators. When hovering over a knob, display the exact mathematical value (e.g., `+2.4 dB`, `1.20 THD%`) in a monospaced font (JetBrains Mono) next to the cursor. This bridges the gap between analog feel and digital precision.

**4. Animation Philosophy (60 FPS Target)**
- **Current:** Static UI with moving meters.
- **Evolution:** Every interaction should have a micro-animation.
  - **Knobs:** A subtle shadow shift when clicked.
  - **Mode Switching:** When moving from Simple to Advanced mode, the UI should not instantly snap. The panels should slide and fade in over 150ms using a smooth ease-out curve.
  - **Target Lock:** When a target is locked, a subtle, slow-pulsing glow should appear around the locked value, indicating that the engine is actively monitoring and adjusting.

## 5. What Makes BTZ Recognizable?

If you blur the screen, BTZ should be recognizable by:
1. The warm Ivory/Parchment color palette.
2. The large, central Harmonic Bloom visualizer.
3. The distinct Sage Green (dynamics) and Terracotta (saturation) color coding.
