# BTZ — Product Identity

The contract for what BTZ Sonic Alchemy *is* — and, just as importantly,
what it is *not*. Every product decision (UI, presets, marketing, feature
roadmap) is measured against this document.

## In one sentence
**BTZ Sonic Alchemy is a modern analog tone-shaping plugin that lets producers
and engineers drive, shape, and *lock* tone and loudness intelligently — without
losing dynamics.**

## Flagship feature
**Target Lock.** Type a LUFS, RMS, or per-band dB target. Lock it. BTZ
intelligently drives the saturation and dynamics engines to hit that target
musically, while a Dynamics Threshold knob preserves the dynamic range you
choose to keep.

## What problem it solves
*"How do I get this loud and thick without crushing it?"* — the question
behind every modern mix and master. Limiters stop peaks. Compressors squash
dynamics. Saturators add color. **BTZ does all three intentionally, toward a
target you typed in.**

## Why someone buys this instead of...
| Competitor | Their hook | Why BTZ |
|---|---|---|
| FabFilter Pro-L | Transparent loudness ceiling | BTZ adds *density* to reach loudness, not just clipping at the top |
| FabFilter Saturn 2 | Manual multiband distortion | BTZ is target-driven, not patch-design — type a number, hear it landed |
| Soundtoys Decapitator | Analog grit | BTZ owns the same warm character + measurable target hitting |
| Soothe2 | Resonance removal | BTZ *adds* good harmonics to hit a target; Soothe *removes* bad ones |
| Gullfoss | Intelligent EQ balance | BTZ controls density + loudness, not spectral balance |
| Goodhertz Faraday / God Particle | Black-box "make it sound good" | BTZ has the same "instant good" feel — but with transparent, typed targets |

## Emotional experience
Calm, tactile, measured. A piece of modern analog furniture for sound — not
a sci-fi dashboard. Confidence through honesty: every meter tells the truth,
every control affects audio.

## First 60 seconds (the only first impression that matters)
1. Insert → audible *subtle warm glue* immediately (Tube model + light glue +
   sheen — the new v1.0.3 default patch).
2. The Harmonic Bloom moves in time with the audio — the plugin's "heartbeat."
3. Footer shows real LUFS / dBTP / GR with units.
4. One click on a preset → instant musical change.
5. Toggle **Loudness‑Matched Bypass** → hear *tone*, not level.
6. Type `-14` into Target LUFS, click LOCK → footer reads
   `TARGET LOCK  -14.0 LUFS  approaching → LOCKED`.

## Feature hierarchy

### Central (v1.0 — must be excellent)
- **Target Lock** (LUFS / RMS / per-band)
- **Drive + Saturation model menu** (5 analog + 2 WDF)
- **Glue compressor** with full attack/release/ratio/link
- **Loudness‑Matched Bypass** (honest A/B)
- **Harmonic Bloom visualizer** (the signature)
- **True‑Peak limiter** + ceiling
- **K‑weighted loudness meter**

### Supporting (v1.0 — present but secondary)
- Shine peaking EQ
- Width / M/S
- Resonance taming
- Transient splitter
- Auto-gain
- Sidechain HPF
- Delta monitoring

### Advanced (Advanced view only)
- Glue attack/release/ratio/link
- Target Lock Dynamics Threshold + per-band
- Saturation model selector
- Oversampling Quality (Eco/Standard/High/Ultra)
- Limiter ceiling
- Mid/Side toggle

### Deferred (v1.1 / v1.2 / future plugin)
- Real per-band multiband saturation (the audio path is not yet wired; UI greyed out)
- Trained neural saturation weights (slots present, currently fall back to tanh; UI greyed out)
- LFOs / Motion modulation (currently dormant; potentially a separate "BTZ Modulator")
- Reference tone matching (`ReferenceToneMatcher` exists, not wired)
- Macro routing (`MacroInterpreter` exists, not wired)
- External audio sidechain bus
- Tempo-synced motion
- HQ-on-render mode

## What BTZ is *not*
- Not a general-purpose multi-effect.
- Not a sound design / modulation playground (LFOs/Motion are not a focus).
- Not an "AI plugin" (no marketing of neural unless trained weights ship).
- Not a kitchen sink. *More features ≠ better.*

## Pricing / release assumptions (placeholder for commercial review)
- One-time purchase, no subscription.
- Cross-grade pricing to be considered after JUCE licensing decision.
- A free, time-limited demo is in scope; copy-protection model TBD.

## Owner sign-off
Before changing this document, talk to the product owner. UI changes, preset
additions, and DSP feature work are *governed* by this identity — not the
other way around.
