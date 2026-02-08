# BTZ Sound Character Specification

**Version**: 1.0.0
**Purpose**: Define measurable sonic targets and character goals
**Last Updated**: 2026-01-08

---

## 🎯 SONIC IDENTITY

**BTZ Mission**: Deliver transparent tonal enhancement with optional analog warmth, competitive loudness, and "mix-ready" sound across all presets.

**Core Promise**:
- **Neutral at 0**: All macros at center positions = transparent pass-through (within ±0.5 dB)
- **Forgiving**: Hard to over-process; guardrails prevent runaway
- **Flexible**: Subtle enhancement → aggressive coloration range
- **Low CPU**: <5% per instance on modern CPUs

---

## 📐 MEASURABLE TARGETS

### Frequency Response (Neutral Settings)
| Band | Requirement | Measurement Method |
|------|-------------|-------------------|
| 20 Hz - 20 kHz | ±0.5 dB | Pink noise → FFT |
| <10 Hz | -12 dB/octave roll-off | DC blocker active |
| >20 kHz | Determined by sample rate | No ultrasonic aliasing |

**Test**: Feed pink noise at -18 dBFS RMS → measure output spectrum

---

### Harmonic Distortion (at Reference Level)

**Reference Level**: -18 dBFS RMS (sine wave)

| Setting | THD | THD+N | Dominant Harmonics |
|---------|-----|-------|-------------------|
| **Digital Clean Mode** | <0.01% | <0.02% | None |
| **Analog Mode = 30%** | 0.1% - 0.5% | 0.2% - 0.6% | H2 > H3 (warm) |
| **Analog Mode = 100%** | 0.5% - 2.0% | 0.6% - 2.5% | H2 ≈ H3 (punchy) |
| **SPARK Drive = High** | 1.0% - 5.0% | 1.5% - 6.0% | H3, H5 (saturation) |

**Asymmetry**: Analog modes favor even harmonics (H2, H4) for warmth

**Test**: 1 kHz sine @ -18 dBFS → FFT → measure H2, H3, H5, THD

---

### Transient Response (Crest Factor)

**Goal**: Preserve or enhance punch without crushing dynamics

| Material | Input CF | Target Output CF | Behavior |
|----------|----------|------------------|----------|
| **Drums (natural)** | 12-18 dB | 10-16 dB | Slight control |
| **Mix bus** | 8-14 dB | 6-12 dB | Gentle density |
| **Aggressive slam** | Any | 4-8 dB | Competitive loudness |

**Crest Factor** = Peak Level - RMS Level (dB)

**Test**: Feed drum loop → measure peak/RMS before and after

---

### Low-End Behavior

**Requirement**: Tight, controlled lows without mud or bloat

| Frequency | Character | Measurement |
|-----------|-----------|-------------|
| **20-60 Hz** | Controlled extension | No subsonic rumble (check <20 Hz) |
| **60-120 Hz** | Punch/body region | ±1 dB shaping allowed |
| **120-250 Hz** | Clarity region | No "boxiness" buildup |

**Sub-harmonic Module** (optional):
- Generates octave-down content at 40-60 Hz
- Must be phase-coherent and mono-compatible
- Should not exceed original level by >3 dB

**Test**:
- Feed kick drum → spectrum analysis → check <20 Hz energy
- Check phase coherence with correlation meter

---

### High-Frequency Air & Extension

**SHINE Module Behavior**:

| Mode | Center Freq | Boost Range | Character |
|------|-------------|-------------|-----------|
| **Fusion** | 20 kHz | 0 - +6 dB | Subtle open-top |
| **Maag** | 10k/20k/40k | 0 - +12 dB | Aggressive air |

**Requirements**:
- No aliasing above Nyquist (measure @ 96 kHz with 40 kHz boost)
- No harshness or brittleness (subjective + harmonic analysis)
- Shelf/bell hybrid shape (broad Q ~0.3-0.7)

**Test**:
- White noise @ 48 kHz SR → apply SHINE +6 dB @ 20 kHz → FFT → verify no alias
- Vocal recording → A/B test → confirm air without harshness

---

### Stereo Image & Mono Compatibility

**Requirements**:
- **Mono-compatible**: L+R sum has no phase cancellation (>90% correlation)
- **Stereo width**: Preserve natural width; no artificial widening by default
- **Crosstalk**: If analog modeling active, <-50 dB crosstalk (subtle realism)

**Test**:
- Feed stereo mix → check mono sum → correlation meter should read >0.90
- Mid-side analysis → verify M channel dominates (center-focused)

---

### Loudness & Dynamics

**SPARK Clipper Targets**:

| Ceiling | Behavior | Use Case |
|---------|----------|----------|
| **-0.3 dBTP** | Safe for streaming | Default |
| **-0.1 dBTP** | CD master | Competitive |
| **0.0 dBTP** | Maximum slam | Intentional (Guard OFF) |

**Integrated Loudness** (LUFS):
- BTZ does **not guarantee** specific LUFS targets
- Provides **monitoring** and **recommendations** only
- User controls final loudness via Drive/Ceiling

**Test**:
- Feed dynamic material → measure output true peak → verify ceiling enforcement
- Measure integrated LUFS → document behavior per preset

---

### Latency

**Requirement**: Report accurate latency for DAW delay compensation

| Module | Latency Source | Typical Value |
|--------|---------------|---------------|
| **Base processing** | Lookahead buffer | 64 samples |
| **Oversampling 4x** | FIR reconstruction | +128 samples |
| **Oversampling 16x** | FIR reconstruction | +512 samples |

**Total Latency** = Base + Active Oversampling

**Test**:
- Query `getLatencySamples()` → verify matches actual delay
- Impulse test → measure input-to-output delay

---

### CPU Efficiency

**Targets** (per instance, 48 kHz, 512 buffer):

| Configuration | CPU % (i7-10700K) | Status |
|---------------|-------------------|--------|
| **Digital Clean Mode** | <2% | Baseline |
| **Analog Mode + SPARK + SHINE (Auto OS)** | <5% | Normal |
| **All modules + 16x OS** | <15% | Heavy |
| **10 instances (typical mix)** | <30% total | Ship Gate #4 |

**Test**:
- Benchmark harness: 10 instances @ 48kHz/128 buffer
- Target: <60% total CPU (avg)

---

## 🎨 SONIC CHARACTER PROFILES

### Mode A: "Clean-Competitive"
- **Use Case**: Mastering, streaming, transparent loudness
- **Character**: Minimal coloration, maximum transparency
- **THD Target**: <0.5%
- **Crest Factor**: 6-10 dB
- **Harmonics**: Primarily H2 (warmth)

### Mode B: "Punch-Forward"
- **Use Case**: Drums, transient-heavy material, modern mixes
- **Character**: Transient emphasis, controlled density
- **THD Target**: 0.5-1.5%
- **Crest Factor**: 5-9 dB
- **Harmonics**: Balanced H2/H3 (punch)

### Mode C: "Aggro-Slam"
- **Use Case**: Aggressive EDM, hard rock, maximum impact
- **Character**: Density, saturation, competitive level
- **THD Target**: 1.5-5%
- **Crest Factor**: 4-6 dB
- **Harmonics**: H3, H5 dominant (grit)

---

## 🔬 VALIDATION METHODOLOGY

### Per-Release Testing

**1. Null Test (Digital Clean)**
- Bypass vs Digital Clean mode
- Requirement: <-60 dB difference (within DSP precision)

**2. Harmonic Sweep**
- 20 Hz - 20 kHz sine sweep @ -18 dBFS
- Measure THD at each frequency
- Plot THD curve

**3. Transient Integrity**
- Feed standard drum kit recording
- Measure crest factor reduction
- Verify punch preservation (subjective + waveform analysis)

**4. Alias Test**
- White noise @ 96 kHz SR
- Apply SHINE boost @ 40 kHz
- FFT analysis → verify no alias energy >Nyquist

**5. Mono Compatibility**
- Stereo mix → sum to mono → correlation meter
- Requirement: >0.90 correlation

**6. CPU Profiling**
- Benchmark harness → 10 instances
- Requirement: <60% avg CPU @ 48kHz/128 buffer

---

## 📊 REFERENCE MEASUREMENTS (Baseline)

### Competitive Analysis (Informational Only)
These measurements are for **internal reference** to understand professional standards. We do NOT claim to match these plugins.

| Plugin | THD @ -18dBFS | CPU (single) | Latency |
|--------|---------------|--------------|---------|
| Reference A | 0.1-0.3% | ~3% | 128 samples |
| Reference B | 0.5-1.5% | ~7% | 256 samples |
| Reference C | 1.0-3.0% | ~5% | 64 samples |

*(Plugin names anonymized; measurements from published specs)*

**BTZ Target**: Match or exceed these benchmarks in transparency, CPU, and quality.

---

## 🎯 SUCCESS CRITERIA

A preset or mode **passes sonic QA** if:
1. ✅ Frequency response within spec
2. ✅ THD within target range for character
3. ✅ No aliasing artifacts (measured)
4. ✅ Transient response predictable and musical
5. ✅ Mono-compatible (>0.90 correlation)
6. ✅ CPU within budget
7. ✅ Latency accurately reported
8. ✅ Subjective quality confirmed by audio engineer

---

## 📝 DOCUMENTATION REQUIREMENTS

For each major DSP module (SPARK, SHINE, Analog Stack):
- Document transfer curves / filter topologies
- Provide measurement data (THD, frequency response, etc.)
- Include A/B audio examples (dry/wet)
- Explain design choices and sonic goals

**Location**: `docs/dsp/MODULE_NAME_SPEC.md`

---

## 🔄 VERSIONING

When sonic behavior changes:
- **Major version** (1.0 → 2.0): Significant character change
- **Minor version** (1.0 → 1.1): New modes or features
- **Patch** (1.0.1): Bug fixes, no intentional sonic change

**Migration**: Presets from older versions should load and sound similar (within ±1 dB, similar THD)

---

**Bottom Line**: BTZ sonic quality is measurable, reproducible, and competitive. We define clear targets, test rigorously, and document transparently.

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ DSP Team
