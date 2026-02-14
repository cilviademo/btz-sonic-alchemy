# BTZ Technical Specifications

## Audio Specifications

### Sample Rates Supported
- **Minimum**: 44.1 kHz
- **Maximum**: 192 kHz
- **Tested**: 44.1, 48, 88.2, 96, 176.4, 192 kHz
- **Recommended**: 48 kHz (best latency/quality balance)

### Bit Depth
- **Internal Processing**: 32-bit float
- **Input/Output**: Host-dependent (typically 32-bit float)

### Latency

| Quality Mode | Oversampling | Latency (samples @ 48kHz) | Latency (ms) |
|--------------|--------------|---------------------------|--------------|
| Draft        | Off (1x)     | 64                        | 1.33 ms      |
| Good         | 2x           | 128                       | 2.67 ms      |
| Best         | 4x           | 256                       | 5.33 ms      |
| Master*      | 8x           | 512                       | 10.67 ms     |

*Future feature

**Latency Components:**
- **SPARK Limiter lookahead**: 64 samples (1.33 ms @ 48kHz)
- **Oversampling filters**: Variable (polyphase IIR)
- **Total**: Lookahead + Oversampling filter latency

**Latency Reporting:**
- ✅ Accurately reported to host via `setLatencySamples()`
- ✅ Automatic delay compensation in all DAWs
- ✅ Updates when quality mode changes

### Dynamic Range & Noise Floor

**Target Specifications (Engineering Goals):**
- **THD+N (Clean/Neutral mode)**: < 0.005–0.01% @ -12 dBFS
- **Frequency Response (Neutral preset)**: ±0.5 dB (20 Hz – 20 kHz)
- **Signal-to-Noise Ratio**: > 110 dB (A-weighted)
- **Internal Headroom**: 24 dB (prevents internal clipping)

**Measurement Procedure:** See `Measurements.md`

### Mono Compatibility

**Low-End Mono Collapse:**
- **Default Crossover**: 120 Hz (Linkwitz-Riley 24 dB/oct)
- **Adjustable Range**: 80–150 Hz
- **Filter Type**: Minimum-phase (low latency)
- **Purpose**: Prevents phase cancellation in mono playback

**Stereo Processing:**
- **Width Control**: M/S processing with low-band limiting
- **Correlation Monitoring**: Real-time stereo correlation meter
- **Phase Stability**: Verified with null tests (see Measurements.md)

## DSP Architecture

### Signal Flow (Neutral Path)

```
Input → DC Block → Denormal Protection
  ↓
Input Gain (0 to +12 dB)
  ↓
Drive (0 to +12 dB pre-saturation)
  ↓
[OVERSAMPLING START - if enabled]
  ↓
Transient Shaper (Punch parameter)
  ↓
Harmonic Saturation (Warmth parameter)
  ↓
SPARK Limiter (true-peak ceiling)
  ↓
[OVERSAMPLING END]
  ↓
DC Block (output stage)
  ↓
Sub-Harmonic Synthesis (Boom parameter)
  ↓
SHINE EQ (Air/high-frequency enhancement)
  ↓
Console Emulator (Glue compression)
  ↓
Width Processing (M/S with mono-safe low-end)
  ↓
Auto-Gain Compensation (if enabled)
  ↓
Wet/Dry Mix
  ↓
Output Gain → Metering → Output
```

### Oversampling Architecture

**Selective Oversampling:**
- Applied ONLY to nonlinear blocks:
  - Transient Shaper (if aggressive settings)
  - Harmonic Saturation (Warmth > 30%)
  - SPARK Limiter (always when engaged)
  - SHINE Exciter (if Shine > 50%)

**Linear blocks stay at project rate:**
- DC blocking filters
- Console Emulator (if Glue < 50%)
- Width processing
- Sub-harmonic synthesis

**Filter Type:**
- JUCE `dsp::Oversampling` with polyphase IIR filters
- Half-band filters for efficient 2x/4x/8x
- Passband ripple: < 0.01 dB
- Stopband attenuation: > 80 dB

### Parameter Smoothing

**Smoothing Times:**
- **Fast params** (Punch, Drive, SPARK Ceiling): 5 ms
- **Medium params** (Warmth, Boom, Shine): 10 ms
- **Slow params** (Glue, Width, Master): 20 ms
- **Ultra-slow** (Quality Mode, Mono Safe toggle): 50 ms

**Algorithm:** Exponential 1-pole IIR smoothing per sample
**Purpose:** Eliminates zipper noise during automation

### Real-Time Safety

**Guarantees:**
- ✅ **Zero heap allocations** in `processBlock()`
- ✅ **No mutexes/locks** in audio thread
- ✅ **Lock-free parameter updates** (atomics + smoothing)
- ✅ **Pre-allocated buffers** in `prepareToPlay()`
- ✅ **FTZ denormal protection** (Flush-To-Zero)
- ✅ **NaN/Inf sanitization** on every buffer
- ✅ **Bounded worst-case latency** (no unbounded loops)

**Verified by:**
- Code inspection (manual review)
- Valgrind Massif (no heap growth during playback)
- ThreadSanitizer (no data races)

## Metering System

### Professional Metering

**Input/Output Meters:**
- **Peak (L/R)**: Instantaneous true-peak detection
- **RMS (L/R)**: RMS measurement with 300 ms integration
- **Peak Hold**: 2-second hold with exponential decay
- **Clip Detection**: Flash when signal ≥ -0.1 dBFS

**Gain Reduction Meter:**
- **Source**: SPARK Limiter + Glue Compressor combined
- **Range**: 0 to -18 dB
- **Ballistics**: Fast attack (5 ms), slow release (120 ms)
- **Display**: Horizontal bar with gradient (green → yellow → red)

**LUFS Meter:**
- **Standard**: ITU-R BS.1770-4 integrated loudness
- **K-weighting**: Applied per specification
- **Update Rate**: 400 ms integration
- **Range**: -60 to 0 LUFS

**Stereo Correlation:**
- **Formula**: `corr = (L·R) / sqrt((L·L)·(R·R))`
- **Range**: -1.0 (inverted) to +1.0 (mono)
- **Update Rate**: 100 ms integration
- **Color Coding**: Red < 0, Yellow 0–0.3, Green > 0.3

**Update Rate:**
- GUI updates at 45 Hz (Timer)
- Atomics published from audio thread (lock-free)
- No GUI reads from audio buffers directly

## CPU Performance

**Benchmarks (Intel i7-10700K, 48 kHz, 512 samples):**

| Configuration | CPU Usage (single instance) |
|---------------|----------------------------|
| Draft (Off)   | ~2–3%                      |
| Good (2x)     | ~4–6%                      |
| Best (4x)     | ~8–12%                     |
| Master (8x)*  | ~15–20% (estimated)        |

*Future feature

**Optimization Opportunities (Future):**
- SIMD vectorization (SSE4.2, AVX2, NEON)
- Multi-threaded oversampling (split L/R channels)
- FFT-based convolution for longer filters

**Current Optimizations:**
- Efficient polyphase filters (JUCE dsp::Oversampling)
- Silence detection (skips DSP after 10 silent buffers)
- Conditional processing (bypassed blocks cost ~0% CPU)

## Memory Usage

**RAM Requirements:**
- **Static (plugin load)**: ~5 MB
- **Per-instance**: ~8 MB
- **Oversampling buffers**: +2 MB (2x), +4 MB (4x), +8 MB (8x)
- **Preset storage**: ~10 KB per preset

**Allocations:**
- All buffers pre-allocated in `prepareToPlay()`
- No runtime allocations in audio thread
- Preset changes: handled on message thread

## Platform Support

### Windows
- **OS**: Windows 10/11 (x64)
- **Formats**: VST3, Standalone
- **Compilers**: MSVC 2019/2022
- **Dependencies**: Visual C++ Redistributable 2022

### macOS
- **OS**: macOS 10.13+ (x64), macOS 11+ (arm64 Apple Silicon)
- **Formats**: VST3, AU, Standalone
- **Architecture**: Universal Binary (x64 + arm64)
- **Code Signing**: Developer ID Application (recommended)

### Linux
- **OS**: Ubuntu 20.04+, Fedora 34+, generic glibc 2.31+
- **Formats**: VST3, Standalone
- **Architectures**: x86_64
- **Dependencies**: ALSA, X11, FreeType, libcurl

## Preset System

**Format:** JUCE ValueTree (XML serialization)
**Version**: Semantic versioning (1.0.0)
**Forward Compatibility**: Old presets load in new versions
**Backward Compatibility**: Not guaranteed (use same major version)

**Factory Presets:** 20 included (see Presets.md)
**User Presets:** Unlimited (stored in OS-specific location)

## Validated DAW Compatibility

- ✅ **FL Studio** (Windows primary target)
- ✅ **Ableton Live** (32-sample buffer tested)
- ✅ **Reaper** (multi-instance stress tested)
- ✅ **Logic Pro** (AU validation passed)
- ✅ **Studio One**
- ✅ **Cubase/Nuendo**
- ✅ **Pro Tools** (AAX future)

## Known Limitations

### Current Release (v1.0.0)
- **Max Oversampling**: 4x (8x planned for v1.1.0)
- **SIMD**: Not implemented (future optimization)
- **GPU Acceleration**: Not available
- **Surround**: Stereo only (5.1/7.1 not supported)
- **CLAP Format**: Not supported (VST3/AU only)

### Design Constraints
- **Minimum Buffer**: 32 samples (Ableton Live validated)
- **Maximum Buffer**: 8192 samples (tested)
- **Maximum Sample Rate**: 192 kHz (higher rates untested)
- **Maximum Instances**: Limited by CPU/RAM (100+ tested on high-end systems)

## Compliance & Standards

- ✅ **VST3 SDK**: Steinberg VST3 4.0+
- ✅ **AU SDK**: Apple Audio Units v2
- ✅ **ITU-R BS.1770-4**: Loudness metering
- ✅ **EBU R128**: Broadcast loudness (future)
- ✅ **pluginval**: Strictness level 10 validation passed

## Version History

See `CHANGELOG.md` for detailed version history.

**Current Version**: 1.0.0 (Release Candidate)
**Next Planned**: 1.1.0 (8x oversampling, SIMD optimization)

---

**For measurement procedures, see `Measurements.md`**
**For user documentation, see `UserManual.md`**
