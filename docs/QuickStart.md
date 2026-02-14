# BTZ Quick Start Guide

## Installation (30 Seconds)

### Windows
1. Download `BTZ-Installer-Win-x64.exe`
2. Run installer → Follow prompts
3. **VST3** installs to: `C:\Program Files\Common Files\VST3\`
4. Restart your DAW

### macOS
1. Download `BTZ-Installer-macOS.dmg`
2. Drag **BTZ.vst3** → `/Library/Audio/Plug-Ins/VST3/`
3. Drag **BTZ.component** → `/Library/Audio/Plug-Ins/Components/` (for Logic/GarageBand)
4. Restart your DAW

### Linux
```bash
unzip BTZ-Linux-x64.zip
cp -R BTZ.vst3 ~/.vst3/
```

## First Use (2 Minutes)

### 1. Load Plugin
- **FL Studio**: Mixer track → Slot → BTZ - The Box Tone Zone
- **Ableton**: Audio track → Audio Effects → BTZ
- **Logic**: Insert → Audio Units → Dynamics → BTZ
- **Reaper**: FX → Add → BTZ

### 2. Try a Preset
1. Open plugin GUI
2. Click **Preset dropdown** (top-right)
3. Select: **"Drum Bus - Punch & Glue"**
4. Play your drums → Hear the difference!

### 3. Tweak to Taste
Adjust the **5 hero controls**:
- **PUNCH** → Transient enhancement (drums love this!)
- **WARMTH** → Analog saturation (add harmonics)
- **BOOM** → Low-frequency thickness
- **SHINE** → High-frequency air/clarity
- **DRIVE** → Input gain (pushes saturation)

### 4. Use the SPARK Limiter
1. Switch to **SPARK tab**
2. Set **Ceiling**: -0.3 dB (safe for streaming)
3. Adjust **SPARK Mix**: 100% (full limiting)
4. Watch **GR meter** → Shows gain reduction in real-time

## Common Use Cases

### Drum Bus
**Preset:** "Drum Bus - Punch & Glue"
- **PUNCH**: 75 → Aggressive transient shaping
- **WARMTH**: 40 → Subtle analog character
- **BOOM**: 30 → Thickness without mud
- **GLUE**: 50 → Cohesive mix bus compression
- **Width**: 50 → Keep natural stereo image

**What it does:** Makes drums hit harder and glue together like vintage console processing.

### 808 / Bass
**Preset:** "808 - Controlled Thump"
- **BOOM**: 60 → Enhanced sub-bass
- **WARMTH**: 30 → Controlled saturation
- **SPARK Ceiling**: -1.0 dB → Prevents clipping
- **Mono Safe**: ON → Keeps low-end focused

**What it does:** Adds weight and control to bass without losing low-end power in mono.

### Mix Bus
**Preset:** "Mix Bus - Clean Glue"
- **GLUE**: 40 → Gentle compression
- **WARMTH**: 25 → Subtle harmonic richness
- **SHINE**: 30 → Polished top-end
- **Width**: 40 → Slight stereo enhancement
- **Quality Mode**: Good (2x oversample)

**What it does:** Cohesive glue with transparent processing, like SSL bus compression.

### Mastering
**Preset:** "Master - Tight & Clear"
- **GLUE**: 30 → Light compression
- **SHINE**: 45 → Broadcast-ready top-end
- **SPARK Ceiling**: -0.3 dB → True-peak limiting
- **Quality Mode**: Best (4x oversample)
- **LUFS Target**: -14 LUFS (streaming platforms)

**What it does:** Final polish for loudness and clarity without artifacts.

## Understanding the Meters

### Input/Output Meters
- **Peak (L/R)**: Instantaneous loudest sample
- **RMS (L/R)**: Average loudness
- **Clip Indicators**: Flash red when signal exceeds 0 dBFS

### SPARK Gain Reduction
- Shows how much limiting is applied (in dB)
- **0 dB**: No limiting
- **-3 dB**: Moderate limiting
- **-6 dB+**: Heavy limiting (may sound compressed)

### LUFS Meter
- **ITU-R BS.1770** integrated loudness
- **Target values:**
  - **Spotify**: -14 LUFS
  - **Apple Music**: -16 LUFS
  - **YouTube**: -13 LUFS
  - **Broadcast (EBU R128)**: -23 LUFS

### Correlation Meter
- **+1.0**: Perfect mono (centered)
- **0.0**: Uncorrelated (wide stereo)
- **-1.0**: Inverted phase (will cancel in mono!)
- **Safe range**: +0.3 to +1.0 for low frequencies

## Oversampling / Quality Modes

### Draft (Off - 1x)
- **Latency**: ~64 samples
- **CPU**: Low
- **Use for**: Live performance, tracking
- **Sound**: Clean, minor aliasing on extreme settings

### Good (2x Oversample)
- **Latency**: ~128 samples
- **CPU**: Medium
- **Use for**: Mixing, most production work
- **Sound**: Very clean, minimal aliasing

### Best (4x Oversample)
- **Latency**: ~256 samples
- **CPU**: High
- **Use for**: Mastering, final bounces
- **Sound**: Pristine, no measurable aliasing

### Master (8x Oversample) - Future
- **Latency**: ~512 samples
- **CPU**: Very high
- **Use for**: Critical mastering work
- **Sound**: Reference-grade

**Tip:** Use Draft during mixing for low latency, switch to Best for final bounce.

## Mono Compatibility

### What is "Mono Safe"?
Collapses frequencies below **120 Hz** to mono to prevent:
- Phase cancellation on club/phone speakers
- Loss of bass power when summed to mono
- Unstable low-end in live PA systems

### When to Use
- ✅ **Bass-heavy tracks** (808s, kick drums, sub bass)
- ✅ **Mix bus** (ensure mono compatibility)
- ✅ **Mastering** (broadcast/streaming requirement)
- ❌ **Creative widening** (unless you want tight low-end)

### How to Enable
1. Switch to **Advanced tab**
2. Toggle **Mono Safe**: ON
3. Adjust **Crossover**: 80-150 Hz (default: 120 Hz)
4. Check **Correlation meter** → Should stay positive in low-end

## Troubleshooting

### Plugin Not Showing in DAW
1. **Rescan plugins** in DAW preferences
2. Check installation path:
   - Windows: `C:\Program Files\Common Files\VST3\`
   - macOS: `/Library/Audio/Plug-Ins/VST3/`
3. Restart DAW completely

### No Audio Output
1. Check **Bypass** button is OFF
2. Verify **MIX** knob is not at 0%
3. Ensure **Input Gain** is not at minimum
4. Check meters → If no input signal, problem is upstream

### High CPU Usage
1. Switch to **Draft** quality mode (Advanced tab)
2. Reduce **oversampling** to Off or 2x
3. Disable **SPARK limiter** if not needed
4. Close plugin GUI when not in use (meter updates stop automatically)

### Clicks/Pops
1. Increase DAW **buffer size** (512 or 1024 samples)
2. Use **Good** or **Best** quality mode (oversampling reduces aliasing)
3. Reduce **DRIVE** if saturating too hard
4. Enable **Parameter Smoothing** (default ON)

### Presets Sound Different Between Sessions
1. Check **sample rate** matches (44.1/48 kHz)
2. Verify **quality mode** is consistent
3. Ensure **Auto-Gain** setting is the same
4. Update BTZ to latest version (presets are forward-compatible)

## Keyboard Shortcuts (Optional Feature)

- **Ctrl/Cmd + Z**: Undo parameter change
- **Ctrl/Cmd + Shift + Z**: Redo
- **Double-click knob**: Reset to default
- **Shift + drag**: Fine adjustment (0.1x speed)
- **Alt + drag**: Super-fine adjustment (0.01x speed)

## Next Steps

- Read **UserManual.md** for deep-dive on each parameter
- See **Presets.md** for complete preset catalog
- Check **Specs.md** for technical details
- Review **Measurements.md** to verify performance

**Need Help?**
- GitHub: https://github.com/cilviademo/btz-sonic-alchemy/issues
- Email: support@btzaudio.com (placeholder)

---

**BTZ Audio**
*Professional Audio Processing*
Version 1.0.0 | 2026
