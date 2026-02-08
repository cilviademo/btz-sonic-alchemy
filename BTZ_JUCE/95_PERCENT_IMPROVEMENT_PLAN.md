# BTZ 85% → 95% Improvement Plan
## Based on 50 Professional Sources

**Goal:** Push BTZ from Release Candidate (85%) to Commercial Quality (95%)

---

## 🎯 CRITICAL IMPROVEMENTS (10% Quality Gain)

Based on comprehensive analysis of 50 professional sources, here are the **essential additions** to reach 95%:

### **1. pluginval Strict Compliance** (Sources #21-23)
**Priority:** CRITICAL
**Impact:** +3%
**Status:** TODO

**What pluginval Tests:**
- Thread safety (no locks, no allocations on audio thread)
- State save/restore reliability
- Parameter automation edge cases
- Bus configuration edge cases
- Extreme buffer sizes (1 sample, 4096 samples)
- Sample rate changes
- Editor open/close stress testing

**Implementation:**
```cpp
// Add to PluginProcessor.cpp

// PLUGINVAL FIX #1: Handle 1-sample buffers
void BTZAudioProcessor::processBlock(...)
{
    const int numSamples = buffer.getNumSamples();

    // pluginval tests with 1-sample buffers!
    if (numSamples < 1)
        return;

    // Process even single samples correctly
    // (smoothing needs to work at any buffer size)
}

// PLUGINVAL FIX #2: Thread-safe logging
// NEVER log on audio thread
#if JUCE_DEBUG
    #define AUDIO_THREAD_ASSERT(condition) jassert(condition)
#else
    #define AUDIO_THREAD_ASSERT(condition) ((void)0)
#endif

// PLUGINVAL FIX #3: Deterministic state
void BTZAudioProcessor::setStateInformation(...)
{
    // Reset all DSP state to ensure determinism
    transientShaper.reset();
    saturation.reset();
    // ... reset all modules
}
```

---

### **2. Proper RBJ Biquad Implementation** (Source #44)
**Priority:** HIGH
**Impact:** +2%
**Status:** TODO

**Current Issue:** ShineEQ uses simplified biquad, may have Q/frequency inaccuracies

**Fix:**
```cpp
// Replace ShineEQ with proper RBJ Audio EQ Cookbook implementation

class RBJBiquad
{
    // Based on http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    void calculateCoefficients(FilterType type, float fc, float Q, float gainDb)
    {
        float A = std::pow(10.0f, gainDb / 40.0f);
        float omega = 2.0f * M_PI * fc / sampleRate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / (2.0f * Q);

        switch (type)
        {
            case HighShelf:
            {
                float beta = std::sqrt(A) / Q;
                b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
                b1 = -2 * A * ((A - 1) + (A + 1) * cs);
                b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
                a0 = (A + 1) - (A - 1) * cs + beta * sn;
                a1 = 2 * ((A - 1) - (A + 1) * cs);
                a2 = (A + 1) - (A - 1) * cs - beta * sn;
                break;
            }
            // ... other filter types
        }

        // Normalize
        b0 /= a0; b1 /= a0; b2 /= a0;
        a1 /= a0; a2 /= a0;
    }
};
```

---

### **3. TPT (Topology-Preserving Transform) Filters** (Source #46)
**Priority:** HIGH
**Impact:** +2%
**Status:** TODO

**Why:** Vadim Zavalishin's TPT approach eliminates frequency warping and is more stable

**Implementation:**
```cpp
// Add TPT one-pole filter for smoother envelopes

class TPTOnePole
{
public:
    void setCutoff(float cutoffHz, float sampleRate)
    {
        g = std::tan(M_PI * cutoffHz / sampleRate);
    }

    float process(float input)
    {
        float v = (input - s) * g / (1.0f + g);
        float y = v + s;
        s = y + v;  // State update
        return y;
    }

    void reset() { s = 0.0f; }

private:
    float g = 0.0f;  // Embedded integrator gain
    float s = 0.0f;  // State
};

// Use for envelope followers in AdvancedTransientShaper
// More accurate than exponential smoothing
```

---

### **4. ChowDSP-Inspired Utilities** (Sources #24-25)
**Priority:** MEDIUM
**Impact:** +1%
**Status:** TODO

**Add Professional Utilities:**

```cpp
// ChowDSP-style buffer utilities

namespace BTZUtils
{
    // Safe buffer operations (prevents out-of-bounds)
    template<typename T>
    inline void copyBuffer(const T* src, T* dst, int numSamples)
    {
        AUDIO_THREAD_ASSERT(src != nullptr && dst != nullptr);
        AUDIO_THREAD_ASSERT(numSamples > 0);
        std::memcpy(dst, src, sizeof(T) * numSamples);
    }

    // Fast buffer clearing (SIMD-friendly)
    template<typename T>
    inline void clearBuffer(T* buffer, int numSamples)
    {
        juce::FloatVectorOperations::clear(buffer, numSamples);
    }

    // Level detection with proper ballistics
    class LevelDetector
    {
        float attack = 0.0f, release = 0.0f;
        float envelope = 0.0f;

        void setAttackMs(float ms, float sr)
        {
            attack = 1.0f - std::exp(-1.0f / (sr * ms / 1000.0f));
        }

        float process(float input)
        {
            float abs_input = std::abs(input);
            envelope = abs_input > envelope
                ? attack * abs_input + (1.0f - attack) * envelope
                : release * abs_input + (1.0f - release) * envelope;
            return envelope;
        }
    };
}
```

---

### **5. Improved APVTS Parameter Management** (Sources #34-36)
**Priority:** MEDIUM
**Impact:** +1%
**Status:** TODO

**Current Issue:** Parameters use default linear scaling, no custom string conversion

**Fix:**
```cpp
// Add proper parameter attributes

layout.add(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"inputGain", 1},  // Stable ID with version
    "Input Gain",
    juce::NormalisableRange<float>{
        -12.0f, 12.0f,  // Range
        0.1f,           // Step
        0.5f            // Skew factor (makes dB feel linear)
    },
    0.0f,  // Default
    "dB",  // Unit suffix
    juce::AudioProcessorParameter::genericParameter,
    // Value to text
    [](float value, int) { return juce::String(value, 1) + " dB"; },
    // Text to value
    [](const juce::String& text) { return text.dropLastCharacters(3).getFloatValue(); }
));

// Add parameter groups for better DAW organization
layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
    "spark", "SPARK Engine",
    "|",  // Separator
    std::make_unique<AudioParameterBool>(...),  // sparkEnabled
    std::make_unique<AudioParameterFloat>(...), // sparkLUFS
    // ... all SPARK params
));
```

---

### **6. Comprehensive DSP Validation** (Sources #43-50)
**Priority:** HIGH
**Impact:** +1%
**Status:** TODO

**Add Runtime DSP Checks:**

```cpp
// DSP validation helpers

namespace BTZValidation
{
    inline bool isValidSample(float sample)
    {
        return std::isfinite(sample) && std::abs(sample) < 100.0f;
    }

    inline void validateBuffer(const juce::AudioBuffer<float>& buffer)
    {
        #if JUCE_DEBUG
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                jassert(isValidSample(data[i]));  // Catch NaN/Inf
            }
        }
        #endif
    }

    // Test DSP module stability
    template<typename DSPModule>
    bool testModuleStability(DSPModule& module, float testSignal = 1.0f)
    {
        juce::AudioBuffer<float> testBuffer(2, 512);
        testBuffer.clear();
        testBuffer.setSample(0, 0, testSignal);  // Impulse

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        // Check output is finite
        for (int i = 0; i < 512; ++i)
        {
            if (!isValidSample(testBuffer.getSample(0, i)))
                return false;
        }
        return true;
    }
}
```

---

### **7. Proper Denormal Handling (Beyond FTZ)** (Sources #37-38, KVR)
**Priority:** MEDIUM
**Impact:** +0.5%
**Status:** PARTIAL (FTZ enabled, but add DC trap)

**Add DC Trap for Safety:**

```cpp
class DCBlocker
{
public:
    void prepare(double sampleRate)
    {
        // High-pass at 5Hz
        float cutoff = 5.0f;
        float RC = 1.0f / (2.0f * M_PI * cutoff);
        float dt = 1.0f / sampleRate;
        coeff = RC / (RC + dt);
    }

    float process(float input)
    {
        output = input - prevInput + coeff * output;
        prevInput = input;

        // Denormal killer
        if (std::abs(output) < 1e-15f)
            output = 0.0f;

        return output;
    }

private:
    float coeff = 0.0f;
    float prevInput = 0.0f;
    float output = 0.0f;
};

// Add to PluginProcessor after all non-linear processing
DCBlocker dcBlockerL, dcBlockerR;
```

---

### **8. Host-Specific Workarounds** (Sources #41-42 - KVR)
**Priority:** LOW
**Impact:** +0.5%
**Status:** TODO

**Known Host Issues:**

```cpp
// Add host detection and workarounds

juce::String hostName = getHostType();

if (hostName.contains("FL Studio"))
{
    // FL Studio can change buffer size during playback
    // Always re-check in processBlock
    if (buffer.getNumSamples() != lastBufferSize)
    {
        lastBufferSize = buffer.getNumSamples();
        // Don't call prepareToPlay, just resize internal buffers
    }
}

if (hostName.contains("Pro Tools"))
{
    // Pro Tools requires exact sample counts for latency
    // Report latency in samples, not ms
    int exactLatency = oversampler.getLatencyInSamples();
    setLatencySamples(exactLatency);  // NOT estimated
}

if (hostName.contains("Ableton"))
{
    // Ableton can send automation ramps
    // Ensure smooth parameter changes work
    // (already handled by SmoothedValue)
}
```

---

## 📊 IMPROVEMENT BREAKDOWN

| Improvement | Impact | Effort | Priority |
|-------------|--------|--------|----------|
| **1. pluginval compliance** | +3% | 4 hours | CRITICAL |
| **2. RBJ biquad filters** | +2% | 2 hours | HIGH |
| **3. TPT filters** | +2% | 3 hours | HIGH |
| **4. ChowDSP utils** | +1% | 2 hours | MEDIUM |
| **5. APVTS refinement** | +1% | 1 hour | MEDIUM |
| **6. DSP validation** | +1% | 2 hours | HIGH |
| **7. DC blocking** | +0.5% | 1 hour | MEDIUM |
| **8. Host workarounds** | +0.5% | 2 hours | LOW |
| **TOTAL** | **+11%** | **17 hours** | **85% → 96%** |

---

## 🚀 IMPLEMENTATION PRIORITY

### **Phase 1: Critical (85% → 90%)** - 1 week
1. ✅ pluginval strict compliance
2. ✅ DSP validation suite
3. ✅ RBJ biquad implementation

**Result:** Crash-free, validated, correct DSP

### **Phase 2: Professional (90% → 93%)** - 1 week
4. ✅ TPT filters for envelopes
5. ✅ ChowDSP-style utilities
6. ✅ DC blocking everywhere

**Result:** Professional-grade stability

### **Phase 3: Commercial Polish (93% → 96%)** - 1 week
7. ✅ APVTS parameter refinement
8. ✅ Host-specific workarounds
9. ✅ Final testing in all DAWs

**Result:** Commercial release-ready

---

## 📝 NEXT STEPS

I'll now implement these improvements in order of priority. Want me to:

**Option A:** Implement Phase 1 (critical fixes) first
**Option B:** Implement all phases at once
**Option C:** Focus on specific improvements you care most about

Which would you prefer?
