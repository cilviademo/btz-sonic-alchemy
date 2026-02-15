# BTZ Offline Rendering Tool

Minimal offline audio rendering tool for sound quality validation and tuning.

## Purpose

- Render test audio through BTZ with specific parameter settings
- Measure objective metrics (peak, RMS, DC offset, crest factor)
- Generate A/B comparison (bypass vs processed)
- CPU profiling (realtime factor)

## Building

```bash
cd BTZ_JUCE
mkdir build
cd build
cmake .. -DBTZ_BUILD_TOOLS=ON
cmake --build . --target offline_render
```

Output: `build/tools/offline_render` (or `.exe` on Windows)

## Usage

Basic usage:
```bash
./offline_render input.wav output.wav
```

With bypass generation for A/B comparison:
```bash
./offline_render input.wav output.wav --bypass bypass.wav
```

## Test Materials

Recommended test signals (create your own or source from your DAW):

1. **Pink noise** (30s, -18 dBFS RMS)
   - Purpose: Frequency response verification (±0.5 dB target)
   - Analysis: FFT spectrum comparison (input vs output)

2. **1 kHz sine wave** (10s, -18 dBFS)
   - Purpose: THD measurement (<0.01% target in neutral mode)
   - Analysis: Harmonic content (H2, H3, H5)

3. **Drum loop** (30-60s, natural dynamics)
   - Purpose: Transient response, crest factor, low-end behavior
   - Analysis: Peak/RMS ratio, phase coherence

4. **Full mix** (60s, -14 LUFS)
   - Purpose: Real-world master bus behavior
   - Analysis: Loudness, spectral balance, stereo width

## Output Metrics

The tool prints:
- **Peak Level** (dBFS) - Maximum absolute sample value
- **RMS Level** (dBFS) - Root mean square (perceived loudness)
- **Crest Factor** (dB) - Peak minus RMS (dynamic range indicator)
- **DC Offset** (dBFS) - Average signal value (should be near -infinity)
- **CPU Time** (ms) - Processing time
- **Realtime Factor** - Audio duration / CPU time (higher = more efficient)

## Sound Quality Targets

From `docs/SOUND_CHARACTER_SPEC.md`:

### Neutral Mode (All macros at 0)
- Frequency response: ±0.5 dB (20 Hz - 20 kHz)
- THD: <0.01%
- DC offset: <-90 dBFS

### Analog Mode (Warmth = 30%)
- THD: 0.1% - 0.5%
- Even harmonics (H2, H4) dominant for warmth

### Transient Response
- Drums: Crest factor 12-18 dB (input) → 10-16 dB (output)
- Mix bus: Crest factor 8-14 dB (input) → 6-12 dB (output)

## Known Issues to Check

Per master finalization checklist (Step 2.2), listen for:

1. **Brittle high-end** (Shine/Air)
   - Test: Full mix with Shine enabled at +6 dB, 20 kHz
   - Fix: Adjust EQ curve, check for aliasing

2. **Low-end phase wobble** (Boom)
   - Test: Kick drum with Boom = 100%, check correlation meter
   - Fix: Verify phase coherence, ensure mono-compatible sub generation

3. **Pumping artifacts** (Glue)
   - Test: Full mix with fast attack/release settings
   - Fix: Adjust envelope smoothing, increase release time minimum

4. **Transient splatter** (Spark ceiling)
   - Test: Drum transients with Spark ceiling = -0.3 dB
   - Fix: Improve lookahead, soft-knee curve, oversampling

5. **Stereo widening collapse** (mono incompatibility)
   - Test: Process stereo mix, then sum to mono - check for cancellation
   - Fix: Ensure mid/side processing is phase-coherent

## Workflow

1. **Render bypass version**:
   ```bash
   ./offline_render drums.wav drums_out.wav --bypass drums_bypass.wav
   ```

2. **Import both to DAW** (separate tracks)

3. **Null test** (invert phase on bypass track):
   - If output is silence → bit-perfect bypass ✅
   - If output has signal → check DC offset, level differences

4. **A/B comparison** (toggle bypass track mute):
   - Listen for artifacts mentioned above
   - Check spectrum analyzer for frequency response
   - Check correlation meter for stereo issues

5. **Iterate**: Adjust DSP parameters in code, rebuild, re-render

## Example Session

```bash
# Generate test files (in your DAW or with sox)
# Example: 30s pink noise at -18 dBFS RMS

# Render with BTZ
./offline_render pink_noise.wav pink_processed.wav --bypass pink_bypass.wav

# Output:
# Loaded: pink_noise.wav
#   Sample Rate: 48000 Hz
#   Channels: 2
#   Duration: 30 seconds
#
# === Input Audio ===
# Peak: -3.02 dBFS (0.707)
# RMS: -18.01 dBFS (0.126)
# Crest Factor: 14.99 dB
# DC Offset: 2.1e-07 (-133.6 dBFS)
#
# === Processing with BTZ ===
# Factory preset: Default (neutral)
# ✅ Processing complete
# CPU Time: 245.32 ms
# Realtime Factor: 122.4x
#
# === Output Audio (Processed) ===
# Peak: -3.05 dBFS (0.705)
# RMS: -18.09 dBFS (0.124)
# Crest Factor: 15.04 dB
# DC Offset: 1.8e-07 (-134.9 dBFS)
#
# ✅ All done! Compare input vs output in your DAW.

# Import to DAW and verify ±0.5 dB frequency response
```

## Limitations

- **No parameter automation**: Uses factory default preset only
  - Future: Add `--preset` option to load specific presets
  - Future: Add `--param punch=0.5 --param warmth=0.3` CLI args

- **No GUI**: Console-only tool
  - This is intentional for scriptable, repeatable testing

- **Single pass**: Processes entire file at once
  - For very long files, split into chunks in your DAW first

## Integration with CI/CD

Add to automated build validation:
```yaml
- name: Sound Quality Regression Test
  run: |
    tools/offline_render tests/audio/pink_noise_48k.wav output.wav
    # Compare output metrics against baseline (future: automated check)
```

## References

- Sound character spec: `docs/SOUND_CHARACTER_SPEC.md`
- Parameter manifest: `docs/PARAMETER_MANIFEST.md`
- Master finalization prompt: Step 2.2 ("Fix cheap plugin tells")
