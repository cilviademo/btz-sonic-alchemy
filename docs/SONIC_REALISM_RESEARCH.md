# BTZ Sonic Realism Research & Implementation Plan

**Date:** 2026-01-14
**Author:** Senior DSP Engineering Team
**Objective:** Elevate BTZ to flagship commercial-grade sonic realism and reliability

---

## BTZ Sonic Signature (Invariants)

**Tonal Fingerprint:**
- **Punch:** Present, controlled transient aggression without harshness
- **Density:** "Glue" and cohesion without mud or excessive compression
- **Air:** Extended high-frequency extension (SHINE) that remains smooth, never brittle
- **Low-End Weight:** Solid sub-bass foundation without boom or resonance
- **Transient Silk:** Transients preserved but refined, not digital-sharp
- **Warmth:** Analog-style harmonic enrichment in 100-800Hz region
- **Stereo Depth:** Wide but mono-compatible, with natural phase relationships

**Invariants (Must NOT Change Across Versions):**
1. Overall harmonic bias (2nd > 3rd > 5th order)
2. Transient preservation characteristic
3. SPARK ceiling behavior at -0.3dBFS
4. SHINE frequency centers (10kHz, 20kHz, 40kHz)
5. Mix control linearity and perceived balance

---

## 25 Sonic Realism Enhancements

### Category A: Temporal & Dynamic Realism

#### 1. Long-Term Energy Integration (Program-Dependent Behavior)
**Audible Effect:** Plugin "learns" the material over seconds, adapting saturation depth and HF response. Makes sustained loud sections feel smoother; prevents ear fatigue.

**Public Sources:**
- UAD manuals describing "program-dependent" compression/limiting behavior
- Abbey Road TG Mastering Chain documentation (adaptive release)
- Academic: "Adaptive Digital Audio Effects" (Zölzer, DAFX book)

**Implementation Method:**
- Multi-second RMS/LUFS integrator (5-30s window)
- Influences: saturation drive scaling (-1 to -3dB when hot), SHINE HF softening (±2dB)
- Original implementation: exponential moving average with 10s time constant

**CPU Risk:** Negligible (1 moving average per channel)

**Test Plan:**
- Process 60s of sustained -10dBFS material vs. sparse hits
- Measure harmonic content over time → should show gradual HF reduction
- Success: Spectral centroid drops 5-10% after 20s of hot material

---

#### 2. Non-Resetting Envelopes (Attack/Release Memory)
**Audible Effect:** No abrupt "reset" when audio resumes after silence. Sounds more like analog gear that never truly powers down.

**Public Sources:**
- Empirical Audio forum: "Why do analog compressors sound smoother on starts?"
- JUCE forum: Managing envelope state across silence

**Implementation Method:**
- Envelope followers decay to floor (never hard-reset to 0)
- Minimum floor: -60dB relative
- Time constant: 500ms decay when silent

**CPU Risk:** None (standard envelope logic)

**Test Plan:**
- Send signal → silence → signal
- Measure attack time on second burst → should be shorter if envelope hasn't fully decayed
- Success: No audible "click" or abrupt character change

---

#### 3. Acceleration-Sensitive Dynamics (d/dt Aware)
**Audible Effect:** Faster transients produce different harmonic content than slow swells. Makes drums punchier, pads smoother.

**Public Sources:**
- "Slew-Rate Dependent Distortion" (KVR DSP forum)
- Analog Devices: "Op-Amp Slew Rate Effects"
- Academic: "Nonlinear Digital Audio Effects" (Välimäki)

**Implementation Method:**
- Calculate derivative of input signal (finite difference)
- Scale saturation asymmetry based on |d/dt|
- High slew → more 2nd harmonic; low slew → softer curve

**CPU Risk:** Low (1 subtraction + 1 abs per sample)

**Test Plan:**
- Compare 1kHz sine vs. 1kHz sine + noise burst
- FFT: Burst should show +3dB in 2nd harmonic vs. pure sine
- Success: Perceptually "faster" on drums, "warmer" on sustain

---

### Category B: Nonlinearity & Harmonic Realism

#### 4. Hysteresis-Based Saturation (Memory Nonlinearity)
**Audible Effect:** Output depends on input AND recent history. Produces "tape-like" smoothness and asymmetry.

**Public Sources:**
- ChowTape documentation: Hysteresis module behavior
- CHOW DAFx paper: "Complex Nonlinearities for Audio" (Chowdhury, 2020)
- GitHub: https://github.com/jatinchowdhury18/AnalogTapeModel

**Implementation Method:**
- Use simplified Jiles-Atherton model or two-state lookup
- MIT/BSD licensed: Implement original hysteresis approximation (not direct CHOW code copy)
- State: previous magnetization level, coercivity parameter

**License Compatibility:** Original implementation (inspired by public CHOW research, not code)

**CPU Risk:** Medium (2-3 nonlinear evaluations per sample)
- Mitigation: Quality tier (Eco=bypass, Normal=light, High=full)

**Test Plan:**
- Apply 1kHz square wave → measure THD
- Increase amplitude → THD should rise non-monotonically (hysteresis loop)
- Success: IMD products show memory (not just instant distortion)

---

#### 5. Program-Adaptive Harmonic Order Rotation
**Audible Effect:** Dominant harmonic shifts from 2nd → 3rd → 5th based on program loudness. Prevents "one-trick-pony" saturation character.

**Public Sources:**
- Sound on Sound: "Modeling Analog Saturation"
- Acustica Audio manuals: "Dynamic Kernel Selection"

**Implementation Method:**
- Track RMS over 100ms window
- Blend saturation curves:
  - Soft (2nd harmonic dominant) at low levels
  - Moderate (3rd) at -12dB
  - Aggressive (5th+) at -6dB
- Smooth crossfades (20ms ramp)

**CPU Risk:** Low (pre-computed curves, linear interpolation)

**Test Plan:**
- Process sine sweep at -20dB, -12dB, -6dB
- Measure THD composition → 2nd should decrease, 3rd/5th increase with level
- Success: Spectral character evolves, not static

---

#### 6. Component Tolerance & Instance Variance
**Audible Effect:** Each plugin instance sounds slightly different (like real analog units). L/R channels subtly differ. Adds "life" and stereo depth.

**Public Sources:**
- Plugin Alliance bx_console TMT documentation
- Blog: "Bad Circuit Modelling Ep. 1: Component Tolerances" (Chowdhury)
- Forum: Gearspace "Why do two 1176s sound different?"

**Implementation Method:**
- Seed per-instance RNG with: plugin instance ID hash
- Apply ±2% variance to:
  - Filter Q/cutoff (SHINE, console EQ)
  - Saturation curve coefficients
  - L/R channel balance (±0.2dB, ±0.5ms delay)
- Store variance in preset (deterministic recall)

**License Compatibility:** Original implementation (concept is public domain)

**CPU Risk:** None (variance computed once at init)

**Test Plan:**
- Load 3 instances with same settings → FFT
- Measure spectral difference → should be 0.1-0.3dB variance in 2-8kHz
- Stereo: Correlation should drop from 1.0 to 0.95-0.98
- Success: Audibly "wider" and "deeper" than single instance

---

### Category C: Psychoacoustic Intelligence

#### 7. Critical-Band Aware Processing (Bark/ERB)
**Audible Effect:** SHINE and saturation respect auditory masking. Less harshness in 2-5kHz region when already dense.

**Public Sources:**
- Academic: "Psychoacoustic Models for Audio Coding" (Painter & Spanias)
- Open-source: https://github.com/ebu/libebur128 (K-weighting concept)
- Bark scale reference: Traunmüller 1990 formula

**Implementation Method:**
- 24-band Bark-spaced filterbank (decimated, no phase issues)
- Calculate masking threshold per band
- Modulate SHINE boost: reduce 3-5kHz by up to -3dB when masked
- Update rate: 50ms (smooth, not jarring)

**CPU Risk:** Medium (24 filters @ 4x decimation = ~6 effective bands per sample)
- Mitigation: Eco mode disables, Normal uses 12 bands

**Test Plan:**
- White noise vs. narrow-band noise at 1kHz
- SHINE should reduce 2kHz boost more with wideband (masking)
- Success: THD+N in critical band stays <0.1% even at high SHINE

---

#### 8. Temporal Masking (Post-Transient HF Softening)
**Audible Effect:** After snare/kick, HF enhancement briefly reduces. Prevents "sizzle pile-up" on dense material.

**Public Sources:**
- PLOS: "Temporal Masking in Human Audition" (overview paper)
- Academic: "Psychoacoustic Masking" (van de Par)

**Implementation Method:**
- Envelope follower on 2-8kHz band (10ms attack, 100ms release)
- When envelope > -20dB → reduce SHINE by -2dB for 50ms
- Smooth ramp (no clicks)

**CPU Risk:** Low (1 envelope + 1 gain smoother)

**Test Plan:**
- Kick drum loop with SHINE at max
- FFT over time → 8kHz should dip for 40-60ms post-transient
- Success: Subjectively less fatiguing on 8-bar loop

---

#### 9. Perceptual Loudness Matching (Auto-Makeup Gain)
**Audible Effect:** Perceived loudness stays constant as you adjust Drive/Punch. No "louder = better" bias during A/B testing.

**Public Sources:**
- EBU R128 / ITU BS.1770 standard (open spec)
- libebur128: https://github.com/jiixyj/libebur128 (MIT license)
- Academic: "Algorithms for Assessment of Audio Programme Loudness"

**Implementation Method:**
- Integrate libebur128 or implement simplified LUFS:
  - K-weighted RMS (pre-filter: shelf at 100Hz, HF boost at 2kHz)
  - 400ms integration window
- Target: -14 LUFS (streaming standard)
- Auto-gain adjusts output to maintain target ±0.5LU

**License Compatibility:** MIT (libebur128) - COMPATIBLE

**CPU Risk:** Low-Medium (2 filters + RMS)
- Mitigation: Update at 10Hz, not per-sample

**Test Plan:**
- Set Drive to 0% → measure LUFS
- Set Drive to 100% with auto-gain → measure LUFS
- Delta should be <0.5LU
- Success: Blind A/B shows no level advantage

---

### Category D: Stereo & Spatial Realism

#### 10. Stereo Micro-Drift (Phase Density)
**Audible Effect:** Subtle L/R phase divergence at HF. Adds "air" and depth without losing mono compatibility.

**Public Sources:**
- Gearspace: "What makes analog stereo sound wider?"
- Academic: "Stereo Width Enhancement" (Orban patent discussion)

**Implementation Method:**
- All-pass filters (1st order) at 8kHz, 16kHz
- L channel: +15° phase shift
- R channel: -15° phase shift
- Depth control: 0-100% (scales shift)

**CPU Risk:** Negligible (2 all-pass filters)

**Test Plan:**
- Mono pink noise → process → measure correlation
- Correlation should drop from 1.0 to 0.92-0.96 above 8kHz
- Mono sum: verify <±0.5dB deviation (comb filter check)
- Success: Stereo feels "wider" but mono-safe

---

#### 11. Inter-Channel Micro-Timing (Sub-Sample Drift)
**Audible Effect:** L/R channels have tiny time offset (like real circuitry). Adds stereo "depth" and "separation."

**Public Sources:**
- Forum: "Haas effect vs. micro-delay for width"
- Academic: "Binaural Cues for Spatial Audio" (Blauert)

**Implementation Method:**
- Fractional delay (Lagrange interpolation, 3rd order)
- L channel: -0.05ms; R channel: +0.05ms (0.1ms total spread)
- Only above 1kHz (to preserve bass mono compatibility)

**CPU Risk:** Low (fractional delay = 4 taps)

**Test Plan:**
- Impulse → measure L/R time delta → should be ~2 samples @ 48kHz
- Low-pass at 500Hz → mono sum → verify no comb filtering
- Success: Headphone listening shows clear L/R separation

---

### Category E: Controlled Instability & "Life"

#### 12. Micro-Instability (Coefficient Modulation)
**Audible Effect:** Parameters subtly "breathe" over time. Prevents static, lifeless tone. Like analog drift.

**Public Sources:**
- UAD manuals: "Analog modeling randomness"
- Forum: "Why does my hardware compressor sound different each time?"

**Implementation Method:**
- LFO at 0.02-0.1Hz (very slow, sub-audible)
- Modulates filter cutoff ±1%, saturation drive ±0.5dB
- Bounded, correlated across L/R (not random walk)

**CPU Risk:** Negligible (1 sine LFO)

**Test Plan:**
- 60s sustain tone → FFT over time → should see ±0.2dB spectral wander
- Success: Sounds "alive" vs. null test with bypass

---

#### 13. Correlated Noise Injection (Program-Dependent)
**Audible Effect:** Gentle noise floor that rises/falls with signal. Mimics analog self-noise.

**Public Sources:**
- Tape machine specs: noise floor -60dB to -70dB
- Academic: "Modeling Analog Noise" (circuit noise sources)

**Implementation Method:**
- Pink noise generator (-72dBFS)
- Scale by program RMS (0dB signal → -66dB noise; -40dB signal → -90dB noise)
- Shaped: gentle HF roll-off (4kHz -3dB)

**CPU Risk:** Negligible (1 noise gen + 1 gain)

**Test Plan:**
- Silence → noise floor should be <-90dBFS
- -10dBFS signal → noise should rise to -68dBFS
- FFT: pink noise spectrum with HF shelf
- Success: Adds "warmth" and "analog feel" without obscuring detail

---

### Category F: Latency, Timing & Phase

#### 14. Sub-Sample Transient Alignment
**Audible Effect:** Transients preserved with sub-sample accuracy. No "digital smear."

**Public Sources:**
- Academic: "Transient Preservation in Oversampled Systems" (Reiss)
- JUCE forum: "Phase-linear oversampling and transient response"

**Implementation Method:**
- Use linear-phase FIR oversampling filters (existing in JUCE)
- Ensure transient detector operates at oversampled rate
- Align output transients to within 0.1 samples (interpolation)

**CPU Risk:** Included in existing oversampling

**Test Plan:**
- Impulse at t=0 → process → measure peak location
- Should be within ±0.1 samples of input
- Success: Kick/snare transients feel "tight" and "immediate"

---

#### 15. Micro-Latency Compensation Between Stages
**Audible Effect:** Internal DSP stages stay phase-coherent. No internal "smear."

**Public Sources:**
- Fabfilter blog: "Why latency compensation matters inside plugins"
- Academic: "Phase Coherence in Cascaded Filters"

**Implementation Method:**
- Calculate latency of each stage (oversampling, filters)
- Insert compensating delays (fractional if needed)
- Total plugin latency reported accurately to host

**CPU Risk:** Low (delays are cheap)

**Test Plan:**
- Impulse → process with all modules enabled → measure group delay
- Should be flat ±1 sample across 20Hz-20kHz
- Success: No phase rotation artifacts on complex material

---

### Category G: Safety & Failure Modes

#### 16. Graceful NaN/Inf Handling
**Audible Effect:** Plugin never produces NaN/Inf output. Instead, mutes gracefully and logs error.

**Public Sources:**
- JUCE best practices: "RT-safe error handling"
- IEEE 754 floating-point spec

**Implementation Method:**
```cpp
if (!std::isfinite(sample))
{
    sample = 0.0f;
    rtSafeLog("NaN detected");
}
```

**CPU Risk:** Negligible (1 check per sample, branch predictor friendly)

**Test Plan:**
- Inject NaN/Inf into input buffer
- Verify output is 0.0 (not NaN)
- Success: No DAW crash, audio mutes safely

---

#### 17. DC Offset Auto-Blocker
**Audible Effect:** Removes DC offsets before saturation stages. Prevents lopsided distortion.

**Public Sources:**
- Music DSP mailing list: "DC blocking filter design"
- Academic: Cookbook filters (Bristow-Johnson)

**Implementation Method:**
- 1-pole HPF at 5Hz (fc = 5Hz)
- Applied at input, before all nonlinear stages
- Coefficient: b1 = 0.999, a0 = 1.0, a1 = -0.999

**CPU Risk:** Negligible (1 filter per channel)

**Test Plan:**
- Input: 1kHz sine + 0.2 DC offset
- Output: DC should be <0.001 (60dB reduction)
- Success: Saturation remains symmetric

---

#### 18. Denormal Prevention
**Audible Effect:** No CPU spikes when processing near-silence.

**Public Sources:**
- Intel optimization manual: "Denormal handling"
- JUCE: flush-to-zero macros

**Implementation Method:**
- Set FTZ/DAZ flags at plugin init
- Add tiny noise (-150dBFS) to feedback paths
- Clamp to zero if |x| < 1e-15

**CPU Risk:** None (prevents spikes)

**Test Plan:**
- Process -96dBFS signal → measure CPU
- CPU should be same as -20dBFS signal (no denormal penalty)
- Success: Predictable CPU at all levels

---

### Category H: Performance & Determinism

#### 19. Dynamic Oversampling Budget
**Audible Effect:** Oversampling engages only when needed. Saves CPU on gentle material.

**Public Sources:**
- UAD Apollo: "Realtime UAD-2 processing" (adaptive quality)
- Academic: "Perceptual Audio Coding" (dynamic bit allocation concept)

**Implementation Method:**
- Monitor input RMS + peak-to-RMS ratio
- High crest factor + high level → enable 4x/8x oversampling
- Low level or sustained → drop to 2x or 1x
- Hysteresis: 3dB threshold, 500ms smoothing

**CPU Risk:** Saves CPU (adaptive)

**Test Plan:**
- Process quiet ambient track → measure CPU (should be low)
- Process drum bus → measure CPU (should rise)
- Success: 40-60% CPU savings on typical mixed material

---

#### 20. CPU Worst-Case Guardrails
**Audible Effect:** Plugin never exceeds defined CPU budget. Gracefully reduces quality if needed.

**Public Sources:**
- Steinberg VST3 SDK: "Realtime guarantees"
- iPlug2 docs: "CPU budget management"

**Implementation Method:**
- Measure processBlock time per call
- If > 80% of buffer time → reduce oversampling tier
- Log warning (RT-safe)
- Restore quality when CPU headroom returns

**CPU Risk:** Prevents overruns

**Test Plan:**
- Artificially limit CPU (run many instances)
- Verify plugin doesn't glitch, drops to Eco mode
- Success: Continuous audio, no dropouts

---

#### 21. Deterministic Processing (Sample-Accurate Recall)
**Audible Effect:** Same input + parameters = identical output. Essential for stems, offline bounces.

**Public Sources:**
- Steinberg: "Offline processing requirements"
- Academic: "Reproducibility in Audio DSP" (AES paper)

**Implementation Method:**
- Seed all RNG with deterministic values (not system time)
- No host-dependent behavior
- State fully serialized in preset

**CPU Risk:** None

**Test Plan:**
- Process 10s file twice with same preset
- Null test → difference should be -∞ dB (bit-identical)
- Success: Perfect null

---

### Category I: Perceptual UX

#### 22. Perceptual Parameter Scaling
**Audible Effect:** Knobs feel musical. 50% sounds "halfway" perceptually, not mathematically.

**Public Sources:**
- Mixing engineers: "Why do some plugins feel better to use?"
- Academic: "Perceptual Scales for Audio Parameters" (logarithmic, cube-root curves)

**Implementation Method:**
- Drive: Cube-root (x^0.33) → feels linear musically
- Mix: -4.5dB pan law (equal power)
- SHINE: dB scale (already perceptual)
- Punch: Exponential (subtle to extreme)

**CPU Risk:** None (mapping at parameter change)

**Test Plan:**
- Blind user test: "Move Drive to 50%"
- Measured change should feel like "half the effect"
- Success: Users rate usability >8/10

---

#### 23. Meter & Visual Alignment (What You See = What You Hear)
**Audible Effect:** Input/output meters match actual loudness perception. No misleading peak meters.

**Public Sources:**
- EBU R128 loudness metering spec
- Bob Katz: "Loudness metering best practices"

**Implementation Method:**
- RMS + true-peak meters (LUFS-weighted)
- Update at 30Hz (smooth, responsive)
- Ballistics: 300ms integration, 1.5s hold

**CPU Risk:** Low (metering off audio thread)

**Test Plan:**
- -14 LUFS pink noise → meters should read -14 LUFS ±0.5
- True-peak should catch intersample overs
- Success: Meters trusted by users

---

#### 24. Contextual Tooltips (Signal-Aware Help)
**Audible Effect:** UI adapts to audio content. E.g., "Your signal is clipping, try reducing Drive."

**Public Sources:**
- UX research: "Context-aware interfaces"
- Plugin Alliance: Adaptive UI hints

**Implementation Method:**
- Analyze audio stats (peak, RMS, crest factor)
- If peak > -0.1dBFS → tooltip: "SPARK ceiling reached"
- If RMS < -40dB → tooltip: "Increase input gain for effect"

**CPU Risk:** None (UI thread)

**Test Plan:**
- Process hot signal → verify tooltip appears
- Success: Users understand plugin state faster

---

#### 25. Sonic Identity Regression Tests
**Audible Effect:** Future updates don't accidentally change the "BTZ sound."

**Public Sources:**
- JUCE forum: "Automated audio unit testing"
- Academic: "Regression Testing for Audio Software" (AES)

**Implementation Method:**
- Golden reference files (kick, snare, mix, sine sweep)
- Process with BTZ → store output hash
- CI: Re-process and compare:
  - FFT magnitude delta <0.1dB
  - Crest factor delta <0.5%
  - Correlation >0.999
- Fail build if sonic drift detected

**CPU Risk:** None (offline test)

**Test Plan:**
- Modify saturation curve → regression test should fail
- Restore → should pass
- Success: Sonic identity locked

---

## License Compatibility Summary

**Direct Dependencies:**
- **libebur128 (MIT):** ✅ SAFE for commercial use
- **JUCE (GPL/Commercial):** ✅ Using commercial license
- **chowdsp_wdf (GPL):** ⚠️ CANNOT link directly
  - Solution: Implement WDF concepts from academic papers (original code)

**Indirect References (Concept Only):**
- ChowTape Model: Use for research, implement original hysteresis
- Plugin Alliance TMT: Concept only (component variance)
- All academic papers: Public domain concepts

**Verdict:** All implementations will be original MIT/BSD-compatible code.

---

## CPU Budget & Quality Tiers

| Tier | Oversampling | Hysteresis | Variance | Psychoacoustic | Total CPU (%) |
|------|--------------|------------|----------|----------------|---------------|
| Eco  | 1x           | Off        | Off      | 12-band        | 8-12%         |
| Normal | 2x         | Light      | On       | 24-band        | 15-25%        |
| High | 4x           | Full       | On       | 24-band        | 30-50%        |

**Adaptive Mode:** Switches tier based on CPU headroom (default: Normal, drop to Eco if >80% CPU)

---

## Testing Matrix

| Enhancement | Unit Test | Integration Test | Listening Test | Success Metric |
|-------------|-----------|------------------|----------------|----------------|
| #1 Energy Integration | ✓ | ✓ | ✓ | Spectral centroid drift |
| #4 Hysteresis | ✓ | ✓ | ✓ | IMD asymmetry |
| #6 Variance | ✓ | ✓ | ✓ | Inter-instance delta |
| #7 Critical-Band | ✓ | ✓ | ✓ | Masking threshold match |
| #9 Loudness | ✓ | ✓ | ✓ | LUFS delta <0.5LU |
| #10 Stereo Drift | ✓ | ✓ | ✓ | Correlation 0.92-0.96 |
| #16 NaN Handling | ✓ | ✗ | ✗ | No crash |
| #21 Determinism | ✓ | ✓ | ✗ | Bit-perfect null |
| #25 Regression | ✗ | ✓ | ✓ | Sonic hash match |

---

## Implementation Priority

**Phase 1 (Critical Path - Week 1):**
1. Enhanced SPARK with oversampling + hysteresis (#4)
2. SHINE with psychoacoustic awareness (#7, #8)
3. Component variance (#6)
4. Safety layer (#16, #17, #18)

**Phase 2 (Realism - Week 2):**
5. Long-term integration (#1, #2)
6. Adaptive harmonics (#3, #5)
7. Stereo depth (#10, #11)
8. Micro-instability (#12, #13)

**Phase 3 (Polish - Week 3):**
9. Loudness matching (#9)
10. CPU guardrails (#19, #20)
11. Determinism (#21)
12. Perceptual UX (#22, #23, #24)

**Phase 4 (Validation - Week 4):**
13. Regression tests (#25)
14. Cross-DAW validation
15. Documentation

---

**Next Steps:** Implement Phase 1 components, starting with enhanced SPARK limiter.
