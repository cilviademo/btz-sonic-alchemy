# BTZ Open-Source Reconnaissance & Integration Strategy

**Version**: 1.0.0
**Purpose**: Document open-source references, license compliance, and integration decisions
**Last Updated**: 2026-01-08

---

## 🎯 INTEGRATION PHILOSOPHY

**Principle**: Learn from the best, integrate legally, attribute properly.

**Rules**:
1. **MIT/BSD/ISC**: May integrate with proper attribution
2. **GPL/LGPL**: Reference-only unless project opts into GPL
3. **Proprietary**: Study behavior only, implement clean-room
4. **No License**: Assume proprietary, do not use

---

## 📚 OPEN-SOURCE LIBRARY SURVEY

### INTEGRATE CANDIDATES (License-Safe)

| Name | Repository | License | Integration | Justification |
|------|-----------|---------|-------------|---------------|
| **JUCE dsp::Convolution** | [JUCE Framework](https://github.com/juce-framework/JUCE) | GPL v3 / Commercial | ✅ **INTEGRATE** | Already using JUCE Commercial. Built-in partitioned convolution with safe IR loading. |
| **HiFi-LoFi/FFTConvolver** | [GitHub](https://github.com/HiFi-LoFi/FFTConvolver) | MIT | ⭐ **BACKUP** | Lightweight partitioned FFT convolution. Use if JUCE Convolution insufficient. |
| **APF (Audio Processing Framework)** | [GitHub](https://github.com/AudioProcessingFramework/apf) | MIT | 🔍 **REFERENCE** | Advanced partitioned convolution patterns. Study for optimization ideas. |
| **PFFFT** | [Android Source](https://android.googlesource.com/platform/external/pffft/) | BSD-like | 🔍 **REFERENCE** | Fast FFT primitives. Consider if custom convolution needed. |
| **FFTS** | [JUCE Forum](https://forum.juce.com/t/ffts-fastest-fft-implementation-and-free-bsd-license/13056) | BSD | 🔍 **REFERENCE** | Fastest FFT backend option for custom convolution. |
| **Airwindows** | [GitHub](https://github.com/airwindows/airwindows) | MIT | ⭐ **STUDY** | MIT-licensed analog saturation/console patterns. Study algorithms for AnalogCharacterLayer. |
| **Tracktion/pluginval** | [GitHub](https://github.com/Tracktion/pluginval) | GPL v3 | 🔍 **TOOL** | Validator binary (not linked). Use as CI tool, not integrated code. |

### REFERENCE-ONLY (License Risk)

| Name | Repository | License | Integration | Reason for Exclusion |
|------|-----------|---------|-------------|---------------------|
| **zita-convolver** | [Slackware](https://www.slackware.com/~alien/slackbuilds/zita-convolver/) | GPL v3 | ❌ **REFERENCE** | GPL contamination risk. Study concepts only. |
| **Acustica Audio** | Proprietary | Commercial | ❌ **NO** | Proprietary. Inspire KCL design but no code reuse. |
| **Waves/UAD/Plugin Alliance** | Proprietary | Commercial | ❌ **NO** | Study sonic behavior only. Clean-room implementation. |

---

## 🏗️ INTEGRATION DECISIONS

### Decision 1: Convolution Engine for KCL

**Chosen Strategy**: **JUCE dsp::Convolution (Primary) + FFTConvolver (Fallback)**

**Rationale**:
- ✅ **JUCE dsp::Convolution** is already licensed (JUCE Commercial)
- ✅ Supports partitioned convolution for low latency
- ✅ Safe IR loading with `loadImpulseResponse()`
- ✅ Handles stereo, sample rate conversion, latency reporting
- ✅ Well-tested in production environments

**Fallback**: If JUCE Convolution lacks features (e.g., dynamic IR switching), vendor **FFTConvolver (MIT)**.

**Evidence**: [JUCE Convolution Documentation](https://docs.juce.com/master/classjuce_1_1dsp_1_1Convolution.html)

**Implementation Path**:
```cpp
// Primary: Use JUCE dsp::Convolution
#include <juce_dsp/juce_dsp.h>

class KernelColorLayer {
    juce::dsp::Convolution convolution;  // JUCE built-in
    // ... dynamic switching logic
};
```

---

### Decision 2: Dynamic Kernel Switching

**Approach**: **Dual-Convolution Crossfade**

**Inspiration**: Study Acustica Audio's "Core Kernel" system (behavior only, no code):
- Multiple intensity kernels (K0-K4)
- Program-dependent selection (RMS/peak envelope)
- Crossfade between kernels (5-50ms)

**Implementation Strategy** (Clean-Room):
```cpp
// Use two parallel convolution instances
juce::dsp::Convolution convA, convB;  // Current and target kernels
LinearSmoothedValue<float> crossfadeAmount;  // 0.0 = A, 1.0 = B

// When switching kernels:
// 1. Load new kernel into inactive convolver (async thread)
// 2. Start crossfade (5-50ms ramp)
// 3. Swap active/inactive
```

**RT-Safety**: Load kernels in `handleAsyncUpdate()`, never in `processBlock()`.

**References**:
- [JUCE Forum: High CPU Convolution](https://forum.juce.com/t/convolution-reverb-with-juce-convolution-class-high-cpu-usage-why/27315)
- [Partitioned Convolution Theory](https://thewolfsound.com/fast-convolution-fft-based-overlap-add-overlap-save-partitioned/)

---

### Decision 3: Analog Modeling Patterns

**Source**: **Airwindows (MIT)**

**Airwindows Algorithms to Study**:
1. **Console Series** (Console4/5/6) - Channel strip saturation
2. **ToTape6** - Tape saturation with HF softening
3. **ADClip7** - Anti-aliased clipping
4. **Density** - Subtle harmonic generation

**Integration Plan**:
- ✅ Study algorithm concepts (transfer curves, oversampling strategies)
- ✅ Adapt to BTZ architecture (tie to Warmth/Texture macros)
- ✅ Preserve MIT license headers if adapting code
- ✅ Add attribution to THIRD_PARTY_NOTICES.md

**Evidence**: [Airwindows Repository](https://github.com/airwindows/airwindows)

---

### Decision 4: Validation Tooling

**Tool**: **pluginval (GPL v3)**

**Usage**: **External CI Tool Only** (not linked into BTZ binary)

**Integration**:
- ✅ Use pluginval binary as CI validation step
- ✅ Parse output for pass/fail
- ❌ Do NOT link pluginval code into BTZ
- ✅ Document in CI scripts (already done in Phase 1)

**License Compliance**: Validator binary usage does not trigger GPL contamination (separate process).

**Evidence**: [pluginval Repository](https://github.com/Tracktion/pluginval)

---

## 📋 INTEGRATION STATUS SUMMARY

### Currently Integrated
| Component | License | Integration Method | Attribution |
|-----------|---------|-------------------|-------------|
| JUCE Framework | GPL v3 / Commercial | Commercial License | "About" dialog + LICENSE.txt |

### Planned Integration (This Session)
| Component | License | Integration Method | Attribution |
|-----------|---------|-------------------|-------------|
| JUCE dsp::Convolution | JUCE Commercial | Direct use (part of JUCE) | Covered by JUCE license |
| Airwindows patterns | MIT | Adapted algorithms | THIRD_PARTY_NOTICES.md + code comments |

### Reference-Only (No Integration)
| Component | License | Usage |
|-----------|---------|-------|
| zita-convolver | GPL v3 | Concept reference (partitioned convolution theory) |
| FFTConvolver | MIT | Backup option (not currently needed) |
| APF | MIT | Optimization patterns reference |
| Acustica Audio | Proprietary | Behavioral inspiration (KCL design) |

---

## ⚖️ LICENSE COMPLIANCE CHECKLIST

### Pre-Integration Review (Every Dependency)
- [ ] **License identified** and compatible with BTZ distribution model
- [ ] **Attribution requirements** documented in THIRD_PARTY_NOTICES.md
- [ ] **Source files** include original license headers (if required)
- [ ] **No GPL contamination** (unless project explicitly opts in)
- [ ] **Commercial use allowed** (check license explicitly)

### Post-Integration Verification
- [ ] **THIRD_PARTY_NOTICES.md** updated
- [ ] **About dialog** mentions third-party libraries (if UI-visible)
- [ ] **README** links to license information
- [ ] **CI builds** pass with all dependencies

---

## 🔍 RESEARCH NOTES

### JUCE dsp::Convolution Analysis

**Capabilities** (verified in JUCE 7.0.12):
- ✅ Stereo processing
- ✅ Partitioned convolution (low latency)
- ✅ `loadImpulseResponse()` with normalization options
- ✅ `prepare()` preallocates buffers (RT-safe after this)
- ✅ Latency reporting via `getLatency()`
- ✅ Sample rate handling (IR can be different SR, JUCE resamples)

**Limitations**:
- ❌ No built-in dynamic IR switching
- ❌ No crossfade between IRs
- ⚠️ High CPU for long IRs (>8192 samples)

**Solution**: Implement dual-convolution crossfade wrapper around JUCE Convolution.

**Evidence**: [JUCE Convolution Class Reference](https://docs.juce.com/master/classjuce_1_1dsp_1_1Convolution.html)

---

### Partitioned Convolution Theory (Clean-Room Knowledge)

**Overlap-Save (OLS) Method**:
- Split long IR into blocks
- FFT each block
- Convolve in frequency domain
- Accumulate results

**Overlap-Add (OLA) Method**:
- Similar to OLS but different boundary handling

**Uniform Partitioning**:
- All blocks same size (simple, predictable latency)

**Non-Uniform Partitioning**:
- Early blocks small (low latency)
- Later blocks large (CPU efficiency)

**BTZ Strategy**: Use JUCE's built-in partitioning (uniform), optimize IR length (<4096 samples).

**Reference**: [Fast Convolution Tutorial](https://thewolfsound.com/fast-convolution-fft-based-overlap-add-overlap-save-partitioned/)

---

### Airwindows Console Algorithm Patterns

**Studied Algorithms** (from GitHub inspection):
1. **Console4Buss** (MIT):
   - Asymmetric saturation (even harmonics)
   - Drive-dependent tonal shift
   - Slew limiting for HF control

2. **ToTape6** (MIT):
   - Flutter simulation (subtle modulation)
   - HF softening (low-pass behavior at drive)
   - Bias control (affects harmonic content)

3. **ADClip7** (MIT):
   - Anti-aliased hard clipping
   - Oversampling strategy (2x minimum)
   - Softer knee option

**Adaptation Plan**:
- Extract core transfer curve concepts
- Integrate into BTZ Saturation module (enhance existing)
- Preserve MIT license in code comments
- Test with automation torture test

**Evidence**: [Airwindows GitHub](https://github.com/airwindows/airwindows)

---

## 🚧 FUTURE CONSIDERATIONS

### If More Advanced Convolution Needed

**Option A**: Vendor **FFTConvolver (MIT)**
- Pros: Lightweight, well-tested, MIT-safe
- Cons: Another dependency to maintain
- Decision: Only if JUCE Convolution insufficient

**Option B**: Custom Implementation with **PFFFT (BSD)**
- Pros: Maximum control, potential optimization
- Cons: More code to maintain, reinvent wheel
- Decision: Avoid unless benchmarking proves necessary

**Option C**: Accept **GPL** and use **zita-convolver**
- Pros: Best-in-class performance
- Cons: BTZ becomes GPL (entire plugin)
- Decision: ❌ **NOT RECOMMENDED** for commercial product

---

## 📞 QUESTIONS FOR LEGAL REVIEW

Before commercial release:
1. ✅ JUCE Commercial license covers all dsp::* usage?
2. ✅ Airwindows MIT algorithms can be adapted commercially?
3. ✅ pluginval binary usage (separate process) does not trigger GPL?
4. ⚠️ Are all attributions in THIRD_PARTY_NOTICES.md legally sufficient?

**Recommendation**: Consult IP attorney for final sign-off.

---

## 📊 METRICS

### Dependencies Reviewed: 10
- **Integrate**: 2 (JUCE Convolution, Airwindows patterns)
- **Reference**: 5 (FFTConvolver, APF, PFFFT, FFTS, zita-convolver)
- **Tool**: 1 (pluginval)
- **Excluded**: 2 (Acustica, Waves/UAD/etc.)

### Licenses Analyzed: 4
- **MIT**: 4 libraries (safe)
- **BSD**: 2 libraries (safe)
- **GPL v3**: 2 libraries (reference or tool only)
- **Commercial**: 2 libraries (excluded)

### Integration Risk: ✅ **LOW**
- All planned integrations are MIT/BSD or already-licensed JUCE
- No GPL contamination
- All proprietary code avoided

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Next Review**: 2026-04-08 (quarterly)
**Maintained By**: BTZ Release Engineering

**Bottom Line**: BTZ will use JUCE dsp::Convolution (already licensed) for KCL, optionally study Airwindows MIT algorithms for analog modeling, and treat all GPL code as reference-only. License risk is minimal.
