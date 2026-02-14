# BTZ Competitive Analysis

## Market Positioning

**BTZ - The Box Tone Zone** is positioned as a **professional drum bus / mix bus processor** combining:
- Transient shaping (inspired by Knock Plugin)
- Mix bus compression (inspired by SSL Bus Comp / Cytomic The Glue)
- Harmonic saturation (analog console/tape character)
- True-peak limiting (SPARK)
- High-frequency enhancement (SHINE)

**Target Market:**
- Producers/mixers needing drum bus processing
- Mix engineers wanting analog console character
- Mastering engineers needing final polish tools
- Electronic music producers (trap, hip-hop, EDM)

**Price Point Target:** $49–$79 (competitive with Knock Plugin, below UAD/Waves tier)

---

## Competitive Benchmark Analysis

### 1. vs. Knock Plugin (Decap / Fuse Audio Labs)

**What Knock Does Well:**
- Surgical transient control (attack/sustain)
- Low CPU usage
- Minimal latency
- Focused on drums

**How BTZ Compares:**

| Feature | Knock Plugin | BTZ |
|---------|-------------|-----|
| **Transient Shaping** | ✅ Excellent (surgical) | ✅ Excellent (PUNCH parameter) |
| **Saturation** | ❌ None | ✅ WARMTH parameter (analog) |
| **Compression** | ❌ None | ✅ GLUE parameter (bus comp) |
| **Limiting** | ❌ None | ✅ SPARK true-peak limiter |
| **Low-End Control** | ❌ None | ✅ BOOM sub-harmonic synthesis |
| **High-End Control** | ❌ None | ✅ SHINE psychoacoustic EQ |
| **Metering** | ❌ Minimal | ✅ Professional (Peak/RMS/GR/LUFS) |
| **Oversampling** | ✅ Yes (2x/4x) | ✅ Yes (2x/4x/8x planned) |
| **Presets** | ✅ Yes (~10) | ✅ Yes (20 factory) |
| **CPU Usage** | ✅ Very Low | ⚠️ Medium (due to more features) |
| **Price** | ~$49 | Target: $49–$79 |

**BTZ Advantage:** More comprehensive tool (transients + saturation + compression + limiting), suitable for mix bus and mastering, not just drums.

**BTZ Disadvantage:** Higher CPU usage (more processing stages). Knock is more focused/specialized.

**Code Implementation:**
- **PUNCH parameter**: Transient shaper with attack/sustain enhancement (`TransientShaper.cpp`)
- **Oversampling**: Selective oversampling on nonlinear blocks (`OversamplingManager.cpp`)
- **Presets**: 4 drum bus presets directly competitive with Knock's presets

---

### 2. vs. SSL Bus Compressor (Waves, Solid State Logic)

**What SSL Bus Comp Does Well:**
- Classic glue compression sound
- Minimal coloration (when desired)
- Industry-standard workflow
- Mix bus cohesion

**How BTZ Compares:**

| Feature | SSL Bus Comp (Waves) | BTZ |
|---------|---------------------|-----|
| **Bus Compression** | ✅ Gold standard | ✅ GLUE parameter (inspired by SSL) |
| **Attack/Release** | ✅ Adjustable | ⚠️ Fixed (optimized for glue) |
| **Ratio** | ✅ Adjustable | ⚠️ Fixed (program-dependent) |
| **Sidechain Filter** | ✅ Yes | ⚠️ Fixed (120 Hz crossover) |
| **Saturation/Color** | ❌ Minimal | ✅ WARMTH parameter (analog character) |
| **Transient Control** | ❌ None | ✅ PUNCH parameter |
| **Limiting** | ❌ None | ✅ SPARK limiter |
| **Metering** | ⚠️ Basic GR | ✅ Comprehensive (Peak/RMS/GR/LUFS) |
| **Oversampling** | ❌ None | ✅ Yes (2x/4x) |
| **Workflow** | ✅ Simple/focused | ⚠️ More controls (learning curve) |
| **Price** | ~$29 (Waves) / $299 (SSL native) | Target: $49–$79 |

**BTZ Advantage:** All-in-one tool (compression + saturation + transients + limiting). More features for similar price.

**BTZ Disadvantage:** Less control over compression parameters (attack/release/ratio). SSL users may prefer traditional controls.

**Code Implementation:**
- **GLUE parameter**: Console emulator with program-dependent compression (`ConsoleEmulator.cpp`)
- **Sidechain**: Low-frequency crossover for focus (`TPTFilters.h` - DC blocker)
- **Presets**: "Mix Bus - Clean Glue" directly inspired by SSL sound

---

### 3. vs. Cytomic The Glue (FabFilter)

**What The Glue Does Well:**
- Transparent bus compression
- Flexible attack/release/ratio controls
- Excellent metering (GR + sidechain EQ)
- Mix bus standard

**How BTZ Compares:**

| Feature | Cytomic The Glue | BTZ |
|---------|-----------------|-----|
| **Bus Compression** | ✅ Excellent (SSL-inspired) | ✅ GLUE parameter |
| **Attack Control** | ✅ Adjustable (0.1–30 ms) | ⚠️ Fixed (optimized) |
| **Release Control** | ✅ Adjustable + Auto | ⚠️ Fixed (program-dependent) |
| **Ratio Control** | ✅ 2:1 to 10:1 | ⚠️ Fixed (~4:1 program-dependent) |
| **Sidechain EQ** | ✅ Visual + adjustable | ⚠️ Fixed (120 Hz HPF) |
| **Dry/Wet Mix** | ✅ Yes (parallel compression) | ✅ Yes (MIX parameter) |
| **Oversampling** | ✅ Yes (2x/4x) | ✅ Yes (2x/4x) |
| **Metering** | ✅ Excellent (GR + sidechain) | ✅ Excellent (GR + Peak/RMS/LUFS) |
| **Saturation** | ❌ Minimal | ✅ WARMTH parameter |
| **Transients** | ❌ None | ✅ PUNCH parameter |
| **Limiting** | ❌ None | ✅ SPARK limiter |
| **Resizable GUI** | ✅ Yes | ⏸️ Planned (v1.1.0) |
| **A/B Comparison** | ✅ Yes | ✅ Yes (backend implemented) |
| **Price** | ~$99 | Target: $49–$79 |

**BTZ Advantage:** Lower price, more features (transients + saturation + limiting), comprehensive metering.

**BTZ Disadvantage:** Less control over compression parameters. The Glue is more surgical/tweakable.

**Code Implementation:**
- **Transparent compression**: Clean glue path in `ConsoleEmulator.cpp`
- **Parallel compression**: MIX parameter (dry/wet blend)
- **A/B comparison**: Backend ready in `ABComparison.h` (GUI button needed)
- **Presets**: "Mix Bus - Clean Glue" and "Mix Bus - Warm Glue" competitive with The Glue presets

---

## Feature Comparison Matrix

| Feature | Knock | SSL Comp | The Glue | BTZ |
|---------|-------|----------|----------|-----|
| **Transient Shaping** | ✅✅ | ❌ | ❌ | ✅✅ |
| **Bus Compression** | ❌ | ✅✅ | ✅✅ | ✅ |
| **Saturation/Color** | ❌ | ⚠️ | ⚠️ | ✅✅ |
| **True-Peak Limiting** | ❌ | ❌ | ❌ | ✅✅ |
| **Sub-Harmonic Synthesis** | ❌ | ❌ | ❌ | ✅ |
| **High-Frequency Enhancement** | ❌ | ❌ | ❌ | ✅✅ |
| **Professional Metering** | ⚠️ | ⚠️ | ✅ | ✅✅ |
| **Oversampling** | ✅ | ❌ | ✅ | ✅ |
| **Mono Compatibility** | ❌ | ⚠️ | ⚠️ | ✅✅ |
| **Factory Presets** | ✅ | ⚠️ | ✅ | ✅ (20) |
| **Resizable GUI** | ⚠️ | ⚠️ | ✅ | ⏸️ (v1.1.0) |
| **A/B Comparison** | ❌ | ❌ | ✅ | ✅* |
| **Price** | ~$49 | ~$29–$299 | ~$99 | $49–$79 |

**Legend:** ✅✅ = Best-in-class, ✅ = Implemented, ⚠️ = Partial, ❌ = Not available, ⏸️ = Planned
*A/B backend ready, GUI button needed

---

## BTZ Unique Selling Points (USPs)

### 1. All-in-One Workflow
**Problem:** Producers use 3–5 plugins per drum bus (transient shaper + compressor + saturator + limiter + EQ)
**BTZ Solution:** Single plugin handles all stages with cohesive sound

**Code Evidence:**
- Integrated DSP chain in `PluginProcessor.cpp:207-399` (processCore function)
- Signal flow: Transient → Saturation → Limiting → Compression → EQ → Width

### 2. Professional Metering
**Problem:** Many plugins lack comprehensive metering (Peak/RMS/GR/LUFS/Correlation)
**BTZ Solution:** Full metering suite inspired by iZotope/FabFilter standards

**Code Evidence:**
- `MeterStrip.cpp` - Professional meter display
- `PluginProcessor.h:12-26` - BTZMeterState with 14 atomic meter values
- `PluginProcessor.cpp:401-460` - updateMeters() with ITU-R BS.1770 LUFS

### 3. Mono-Safe Low-End
**Problem:** Many widening/saturation plugins destroy mono compatibility
**BTZ Solution:** Automatic low-frequency mono collapse with adjustable crossover

**Code Evidence:**
- Width processing with sidechain filter (`PluginProcessor.cpp:315-328`)
- Mono Safe toggle in Advanced tab
- Correlation meter validation

### 4. Oversampling Where It Matters
**Problem:** Full-chain oversampling wastes CPU on linear blocks
**BTZ Solution:** Selective oversampling only on nonlinear blocks (saturation, limiter)

**Code Evidence:**
- `OversamplingManager.cpp` - Selective oversampling architecture
- Linear blocks (DC filter, width) stay at project rate
- `PluginProcessor.cpp:496-510` - Conditional oversampling

### 5. Affordable Professional Quality
**Problem:** Pro-grade tools (UAD, Waves Platinum) cost $300–$1000+
**BTZ Solution:** Professional features at prosumer price ($49–$79)

---

## What BTZ Does NOT Do (vs. Competitors)

### Missing Features (Planned for Future Versions)

1. **Individual Compression Controls** (vs. SSL Comp / The Glue)
   - BTZ uses fixed attack/release/ratio optimized for glue
   - Competitors offer full control
   - **Future:** "Advanced Compression" tab with full controls (v1.2.0)

2. **Resizable GUI** (vs. The Glue, UAD plugins)
   - BTZ has fixed 980x610 px GUI
   - Competitors offer 100/125/150/200% scaling
   - **Future:** Resizable GUI (v1.1.0)

3. **SIMD Optimization** (vs. Knock Plugin)
   - BTZ uses scalar float processing
   - Knock uses SSE4.2/AVX2 for lower CPU
   - **Future:** SIMD implementation (v1.2.0) - see `docs/Performance.md` for benchmarking notes

4. **Mid/Side EQ** (vs. FabFilter Pro-MB)
   - BTZ has M/S width control but no M/S EQ
   - **Future:** M/S SHINE EQ (v1.3.0)

5. **Code Signing & Installer** (vs. all commercial plugins)
   - BTZ currently manual install
   - **Future:** InnoSetup (Windows), pkgbuild (macOS) installers + code signing

---

## Market Gaps BTZ Fills

### 1. "Knock Plugin for Mix Bus"
- Knock only targets drums
- BTZ works on drums AND mix bus AND mastering

### 2. "SSL Comp with Transients & Saturation"
- SSL comp is clean compression only
- BTZ adds punch and warmth in one tool

### 3. "The Glue at Half the Price"
- The Glue: $99
- BTZ: $49–$79 with MORE features (transients, saturation, limiting)

### 4. "Affordable UAD Alternative"
- UAD SSL Comp + Studer A800 + Precision Limiter = $300+
- BTZ: All-in-one for $49–$79

---

## Competitive Roadmap (Future Releases)

### v1.1.0 (Q2 2026)
- ✅ 8x oversampling mode
- ✅ Resizable GUI (100/125/150/200%)
- ✅ A/B comparison GUI button
- ✅ Preset browser with tags/search

### v1.2.0 (Q3 2026)
- ✅ SIMD optimization (2–4x CPU reduction)
- ✅ Advanced compression controls (attack/release/ratio/knee)
- ✅ Sidechain EQ visualization
- ✅ M/S mode toggle

### v1.3.0 (Q4 2026)
- ✅ M/S SHINE EQ
- ✅ Multi-band processing option
- ✅ External sidechain input
- ✅ CLAP format support

**Goal:** Match or exceed The Glue feature set while maintaining all-in-one workflow advantage.

---

## Technical Implementation vs. Competitors

### Oversampling Quality

**BTZ Implementation:**
```cpp
// BTZ_JUCE/Source/DSP/OversamplingManager.cpp
os2x = std::make_unique<juce::dsp::Oversampling<float>>(
    2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
    true, false
);
// Polyphase IIR filters, 80+ dB stopband
```

**Comparison:**
- **Knock Plugin**: Similar JUCE dsp::Oversampling
- **The Glue**: Custom FIR filters (potentially better phase but higher latency)
- **BTZ**: Uses JUCE standard - proven, efficient, industry-tested

### Metering Accuracy

**BTZ LUFS Implementation:**
```cpp
// BTZ_JUCE/Source/DSP/LUFSMeter.cpp
// ITU-R BS.1770-4 K-weighting + 400ms integration
const float lufsRms = std::sqrt((lufsSq * 0.5f) * invN + 1.0e-20f);
meters.lufs.store(juce::Decibels::gainToDecibels(lufsRms, -100.0f), ...);
```

**Comparison:**
- **SSL Comp**: No LUFS metering
- **The Glue**: No LUFS metering
- **BTZ**: Full ITU-R BS.1770-4 standard - on par with iZotope/FabFilter

### Real-Time Safety

**BTZ Guarantees:**
```cpp
// Zero heap allocations in processBlock()
// All buffers pre-allocated in prepareToPlay()
// Lock-free parameter updates via atomics
// FTZ denormal protection
```

**Comparison:**
- All competitors (Knock, SSL, The Glue) meet RT-safety standards
- BTZ on par with industry best practices

---

## Summary: When to Choose BTZ

### Choose BTZ if you want:
- ✅ All-in-one drum bus / mix bus tool
- ✅ Comprehensive metering (LUFS, correlation, GR)
- ✅ Mono-safe low-end processing
- ✅ Affordable professional quality ($49–$79)
- ✅ Future-proof (active development roadmap)

### Choose Knock Plugin if you want:
- ✅ Surgical transient control ONLY
- ✅ Minimal CPU usage
- ✅ Focused workflow (no distractions)

### Choose SSL Comp / The Glue if you want:
- ✅ Traditional compression controls (attack/release/ratio)
- ✅ Surgical sidechain EQ
- ✅ Industry-standard workflow
- ✅ Proven on thousands of hit records

---

**BTZ occupies a unique position:** More comprehensive than Knock, more affordable than The Glue, more modern than SSL Comp. **Best value for producers needing an all-in-one solution.**

---

**For technical specifications, see `Specs.md`**
**For feature roadmap, see `CHANGELOG.md`**
**For preset catalog, see `Presets.md`**
