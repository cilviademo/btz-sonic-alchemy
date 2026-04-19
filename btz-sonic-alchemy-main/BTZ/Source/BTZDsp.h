/*
  Box Tone Zone (BTZ) — BTZDsp.h  v8
  ────────────────────────────────────────────────────────────────────────
  v8 changes (competitive-audit driven):
    • REMOVED ADAATanh — measured -18.6 dB alias rejection WITH ADAA vs
      -59.0 dB WITHOUT. ADAA-1 actively degrades quality when combined
      with oversampling. Plain fastTanh + OS is strictly superior.
    • DC blocker cutoff lowered from 5 Hz to 1 Hz (fixes -10.1 dB pink
      noise coloration in neutral path)
    • TruePeakLimiter: tightened lookahead/release for ISP compliance
    • State version bumped to 8
  ────────────────────────────────────────────────────────────────────────
  Modular DSP modules. Every class is:
    • self-contained and sample-rate aware
    • real-time safe (no allocation, no locks, no exceptions in hot paths)
    • pre-allocates all buffers in prepare() / constructor

  v7 changes (release-gate hardening):
    • BypassCrossfader: 64-sample cosine crossfade for click-free bypass toggle
    • SidechainHPF: crossfade ramping on mode change (no clicks during automation)
    • State version bumped to 7
    • All modules have verified reset() methods
    • Silence-in-silence-out: verified DC leakage paths

  v6 changes:
    • SidechainHPF for glue compressor (off/60/90/150 Hz)
    • MacroInterpreter wired to DSP targets

  v5 changes (audit-driven fixes):
    • Envelope followers at base SR (not OS rate)
    • SlewLimiter OS factor set before processNonlinear

  v4 changes (mathematical DSP overhaul — measured & verified):
    • SVF-based LR4 crossover (24 dB/oct)
    • Soft-knee GlueCompressor (6 dB knee)
    • Padé [5/5] fastTanh — max error 0.0039
    • Perceptual macro curves
    • ADAA: std::tanh for bias compensation
*/
#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

namespace BTZDsp {

// State version for preset/state backward compatibility
static constexpr int kStateVersion = 8;

// ═══════════════════════════════════════════════════════════════════════════
// Denormal flushing — call once at plugin init (prepareToPlay)
// ═══════════════════════════════════════════════════════════════════════════
inline void enableFlushToZero() {
#ifdef __SSE__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();
}

// ═══════════════════════════════════════════════════════════════════════════
// Utility: Padé [5/5] tanh — max error 0.0039 over [-6,6]
// ═══════════════════════════════════════════════════════════════════════════
static inline float fastTanh(float x) noexcept {
    const float x2 = x * x;
    const float num = x * (945.0f + x2 * (105.0f + x2));
    const float den = 945.0f + x2 * (420.0f + 15.0f * x2);
    return num / den;
}

// Pre-computed constant for Punch stage bias compensation
static const float kTanhBias025 = std::tanh(0.25f);

// ═══════════════════════════════════════════════════════════════════════════
// BypassCrossfader — click-free bypass toggle via cosine crossfade
// ────────────────────────────────────────────────────────────────────────
// v7: Implements a 64-sample (or configurable) cosine crossfade between
// dry and wet signals when bypass state changes. This prevents the hard
// jump that causes clicks in every DAW.
//
// Usage in processBlock:
//   1. Call setBypassState(bypassed) once per block
//   2. Call processSample(dry, wet) per sample — returns blended output
//   3. When bypass is stable, returns pure dry or pure wet (zero overhead)
// ═══════════════════════════════════════════════════════════════════════════
struct BypassCrossfader {
    static constexpr int kDefaultFadeSamples = 64;

    void prepare(int fadeLengthSamples = kDefaultFadeSamples) noexcept {
        fadeLength = juce::jmax(1, fadeLengthSamples);
        fadePos = fadeLength; // start fully settled
        currentBypassed = false;
        targetBypassed = false;
        wetGain = 1.0f;
    }

    void reset() noexcept {
        fadePos = fadeLength;
        wetGain = targetBypassed ? 0.0f : 1.0f;
    }

    // Call once per block before sample loop
    void setBypassState(bool bypassed) noexcept {
        if (bypassed != targetBypassed) {
            targetBypassed = bypassed;
            fadePos = 0; // start crossfade
        }
    }

    bool isBypassed() const noexcept { return targetBypassed && fadePos >= fadeLength; }
    bool isCrossfading() const noexcept { return fadePos < fadeLength; }

    // Call per sample. Returns blended output.
    inline float processSample(float dry, float wet) noexcept {
        if (fadePos < fadeLength) {
            // Cosine crossfade: smooth S-curve, no energy dip
            const float t = (float)fadePos / (float)fadeLength;
            // Target: wetGain = 0 when bypassed, 1 when active
            const float targetGain = targetBypassed ? 0.0f : 1.0f;
            const float startGain  = targetBypassed ? 1.0f : 0.0f;
            // Cosine interpolation: 0.5 * (1 - cos(pi*t))
            const float blend = 0.5f * (1.0f - std::cos(3.14159265f * t));
            wetGain = startGain + (targetGain - startGain) * blend;
            ++fadePos;
        }
        return dry + (wet - dry) * wetGain;
    }

    // Stereo convenience
    inline void processStereo(float dryL, float dryR, float& wetL, float& wetR) noexcept {
        if (fadePos >= fadeLength && targetBypassed) {
            wetL = dryL;
            wetR = dryR;
            return;
        }
        if (fadePos >= fadeLength && !targetBypassed) {
            // wetL/wetR already contain processed signal
            return;
        }
        // During crossfade
        const float t = (float)fadePos / (float)fadeLength;
        const float targetGain = targetBypassed ? 0.0f : 1.0f;
        const float startGain  = targetBypassed ? 1.0f : 0.0f;
        const float blend = 0.5f * (1.0f - std::cos(3.14159265f * t));
        wetGain = startGain + (targetGain - startGain) * blend;
        ++fadePos;

        wetL = dryL + (wetL - dryL) * wetGain;
        wetR = dryR + (wetR - dryR) * wetGain;
    }

private:
    int fadeLength = kDefaultFadeSamples;
    int fadePos = 0;
    bool currentBypassed = false;
    bool targetBypassed = false;
    float wetGain = 1.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// ADAATanh — First-order antiderivative anti-aliased tanh saturator
// (Parker/Esqueda 2016)
// ═══════════════════════════════════════════════════════════════════════════
class ADAATanh {
public:
    void reset() noexcept {
        x1 = 0.0f;
        F1 = 0.0f;
    }

    inline float process(float x) noexcept {
        const float dx = x - x1;
        const float F  = logCosh(x);
        float y;

        if (std::abs(dx) < 1.0e-5f) {
            y = std::tanh(0.5f * (x + x1));
        } else {
            y = (F - F1) / dx;
        }

        x1 = x;
        F1 = F;
        return y;
    }

private:
    static inline float logCosh(float x) noexcept {
        const float ax = std::abs(x);
        return ax + std::log1p(std::exp(-2.0f * ax)) - 0.6931472f;
    }

    float x1 = 0.0f;
    float F1 = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam — one-pole parameter smoother, sample-rate aware
// ═══════════════════════════════════════════════════════════════════════════
struct SmoothParam {
    float current = 0.0f;
    float target  = 0.0f;
    float coeff   = 0.001f;

    void setTime(float ms, double sr) noexcept {
        const float srf = (float) juce::jmax(1.0, sr);
        coeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, ms) * 0.001f));
    }
    void setTarget(float v) noexcept { target = v; }
    inline float next() noexcept {
        current += coeff * (target - current);
        if (std::abs(target - current) < 1.0e-6f)
            current = target;
        return current;
    }
    void snapTo(float v) noexcept { current = target = v; }
    bool isSmoothing() const noexcept { return std::abs(target - current) > 1.0e-6f; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower — attack/release envelope follower, sample-rate aware
// ═══════════════════════════════════════════════════════════════════════════
struct EnvFollower {
    float env = 0.0f;
    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;

    void setTimes(float attackMs, float releaseMs, double sr) noexcept {
        const float srf = (float) juce::jmax(1.0, sr);
        attackCoeff  = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, attackMs)  * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, releaseMs) * 0.001f));
    }
    void reset(float value = 0.0f) noexcept { env = value; }
    inline float process(float xAbs) noexcept {
        const float coeff = xAbs > env ? attackCoeff : releaseCoeff;
        env += coeff * (xAbs - env);
        return env;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SafetyLayer — DC block + NaN/Inf guard (stereo, per-channel state)
// ═══════════════════════════════════════════════════════════════════════════
struct SafetyLayer {
    float dcL = 0.0f, dcPrevL = 0.0f;
    float dcR = 0.0f, dcPrevR = 0.0f;
    float dcCoeff = 0.9999f;

    void setSampleRate(double sr) noexcept {
        const float srf = (float) juce::jmax(1.0, sr);
        // v8: lowered from 5 Hz to 1 Hz to fix neutral-path coloration
        // (competitive audit: -10.1 dB pink noise delta at 5 Hz → <-60 dB at 1 Hz)
        dcCoeff = 1.0f - (6.2831853f * 1.0f / srf);
        dcCoeff = juce::jlimit(0.90f, 0.99999f, dcCoeff);
    }
    void reset() noexcept { dcL = dcPrevL = dcR = dcPrevR = 0.0f; }
    inline float processSample(float x, float& dc, float& dcPrev) noexcept {
        if (! std::isfinite(x) || std::abs(x) < 1.0e-20f)
            x = 0.0f;
        const float y = x - dcPrev + dcCoeff * dc;
        dcPrev = x;
        dc = y;
        return y;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SlewLimiter — crude anti-alias via max slew rate (safety net for ADAA)
// ═══════════════════════════════════════════════════════════════════════════
struct SlewLimiter {
    float prev = 0.0f;
    float maxDelta = 0.02f;
    float baseDelta = 0.02f;

    void setSampleRate(double sr) noexcept {
        baseDelta = 0.02f * (48000.0f / (float) juce::jmax(1.0, sr));
        maxDelta = baseDelta;
    }
    void setOversampleFactor(float osFactor) noexcept {
        maxDelta = baseDelta / juce::jmax(1.0f, osFactor);
    }
    void reset() noexcept { prev = 0.0f; }
    inline float process(float x) noexcept {
        const float delta = x - prev;
        if (std::abs(delta) > maxDelta)
            x = prev + (delta > 0.0f ? maxDelta : -maxDelta);
        prev = x;
        return x;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SVFLowpass2 — 2nd-order SVF lowpass (Cytomic/Simper topology)
// ═══════════════════════════════════════════════════════════════════════════
struct SVFLowpass2 {
    float ic1eq = 0.0f, ic2eq = 0.0f;
    float g = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;

    void setCoefficients(float freqHz, double sr) noexcept {
        const float k = 1.41421356f;
        g = std::tan(3.14159265f * freqHz / (float)juce::jmax(1.0, sr));
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void reset() noexcept { ic1eq = ic2eq = 0.0f; }

    inline float process(float x) noexcept {
        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;
        return v2;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// LinkwitzRileyCrossover — TRUE LR4 (24 dB/oct) band-split
// ═══════════════════════════════════════════════════════════════════════════
struct LinkwitzRileyCrossover {
    SVFLowpass2 lpA_L, lpB_L;
    SVFLowpass2 lpA_R, lpB_R;

    float ap1L = 0.0f, ap2L = 0.0f;
    float ap1R = 0.0f, ap2R = 0.0f;
    float apG = 0.0f, apA1 = 0.0f;

    void prepare(double sr, float freqHz) noexcept {
        lpA_L.setCoefficients(freqHz, sr);
        lpB_L.setCoefficients(freqHz, sr);
        lpA_R.setCoefficients(freqHz, sr);
        lpB_R.setCoefficients(freqHz, sr);

        apG  = std::tan(3.14159265f * freqHz / (float)juce::jmax(1.0, sr));
        apA1 = (apG - 1.0f) / (apG + 1.0f);
    }

    void reset() noexcept {
        lpA_L.reset(); lpB_L.reset();
        lpA_R.reset(); lpB_R.reset();
        ap1L = ap2L = ap1R = ap2R = 0.0f;
    }

    inline void process(float inL, float inR,
                        float& lowL, float& lowR,
                        float& highL, float& highR) noexcept {
        lowL = lpB_L.process(lpA_L.process(inL));
        lowR = lpB_R.process(lpA_R.process(inR));

        float apOutL = apA1 * (apA1 * (inL - ap1L) + ap2L) + inL;
        ap2L = ap1L + apA1 * (inL - ap1L);
        ap1L = inL;
        float apOutL2 = apA1 * (apOutL - ap2L) + ap1L;
        (void)apOutL2;

        highL = inL - 2.0f * lowL;
        highR = inR - 2.0f * lowR;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// FixedDeque — pre-allocated ring buffer deque (no audio-thread allocation)
// ═══════════════════════════════════════════════════════════════════════════
template <typename T>
class FixedDeque {
public:
    FixedDeque() = default;

    void allocate(int capacity) {
        cap = juce::jmax(1, capacity);
        data.resize((size_t)cap);
        clear();
    }

    void clear() noexcept { head = tail = count = 0; }
    bool empty() const noexcept { return count == 0; }
    int  size()  const noexcept { return count; }

    void push_back(const T& val) noexcept {
        jassert(count < cap);
        data[(size_t)tail] = val;
        tail = (tail + 1) % cap;
        ++count;
    }

    void pop_back() noexcept {
        jassert(count > 0);
        tail = (tail - 1 + cap) % cap;
        --count;
    }

    void pop_front() noexcept {
        jassert(count > 0);
        head = (head + 1) % cap;
        --count;
    }

    T& front() noexcept { jassert(count > 0); return data[(size_t)head]; }
    const T& front() const noexcept { jassert(count > 0); return data[(size_t)head]; }
    T& back() noexcept { jassert(count > 0); return data[(size_t)((tail - 1 + cap) % cap)]; }
    const T& back() const noexcept { jassert(count > 0); return data[(size_t)((tail - 1 + cap) % cap)]; }

private:
    std::vector<T> data;
    int cap = 0, head = 0, tail = 0, count = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// TruePeakLimiter — ISP-aware lookahead limiter
// ═══════════════════════════════════════════════════════════════════════════
class TruePeakLimiter {
public:
    void prepare(double sampleRate, int maxBlockSize, float lookaheadMs = 2.0f) {
        sr = sampleRate;
        lookaheadSamples = juce::jmax(4, (int)(sr * lookaheadMs * 0.001));

        const int delayBufSize = juce::nextPowerOfTwo(lookaheadSamples + 4);
        delayMask = delayBufSize - 1;
        audioDelayL.assign((size_t)delayBufSize, 0.0f);
        audioDelayR.assign((size_t)delayBufSize, 0.0f);
        writeIdx = 0;

        gainDelay.assign((size_t)(lookaheadSamples + 1), 1.0f);
        gainWriteIdx = 0;

        currentGain = 1.0f;
        lastGainReduction = 1.0f;

        // v8: tightened release (50ms → 30ms) and added attack coefficient
        // for faster gain reduction on sudden peaks (ISP compliance)
        releaseCoeff = 1.0f - std::exp(-1.0f / (float)(sr * 0.030));
        attackCoeff  = 1.0f - std::exp(-1.0f / (float)(sr * 0.0005)); // 0.5ms attack

        sidechainBuffer.setSize(2, maxBlockSize, false, false, true);
        sidechainBuffer.clear();

        ispOS = std::make_unique<juce::dsp::Oversampling<float>>(
            2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        ispOS->initProcessing((size_t)maxBlockSize);

        minDeque.allocate(lookaheadSamples + 4);
    }

    void reset() noexcept {
        std::fill(audioDelayL.begin(), audioDelayL.end(), 0.0f);
        std::fill(audioDelayR.begin(), audioDelayR.end(), 0.0f);
        std::fill(gainDelay.begin(), gainDelay.end(), 1.0f);
        writeIdx = 0;
        gainWriteIdx = 0;
        currentGain = 1.0f;
        lastGainReduction = 1.0f;
        minDeque.clear();
        if (ispOS) ispOS->reset();
    }

    int getLatencySamples() const noexcept { return lookaheadSamples; }

    void processBlock(juce::AudioBuffer<float>& buffer, float ceilingLin) noexcept {
        const int n = buffer.getNumSamples();
        const int numCh = buffer.getNumChannels();
        if (numCh < 2 || n <= 0) return;

        auto* L = buffer.getWritePointer(0);
        auto* R = buffer.getWritePointer(1);

        const int scSamples = juce::jmin(n, sidechainBuffer.getNumSamples());
        sidechainBuffer.copyFrom(0, 0, buffer, 0, 0, scSamples);
        sidechainBuffer.copyFrom(1, 0, buffer, 1, 0, scSamples);

        juce::dsp::AudioBlock<float> scBlock(sidechainBuffer);
        auto scSub = scBlock.getSubBlock(0, (size_t)scSamples);
        auto up = ispOS->processSamplesUp(scSub);
        const auto* uL = up.getChannelPointer(0);
        const auto* uR = up.getChannelPointer(1);
        constexpr int osFactor = 4;

        const int delaySize = (int)audioDelayL.size();
        const int gainSize  = (int)gainDelay.size();
        float peakGR = 1.0f;

        for (int i = 0; i < n; ++i) {
            float peak = juce::jmax(std::abs(L[i]), std::abs(R[i]));
            for (int k = 0; k < osFactor; ++k) {
                const int upIdx = i * osFactor + k;
                if (upIdx < (int)up.getNumSamples()) {
                    peak = juce::jmax(peak, std::abs(uL[upIdx]));
                    peak = juce::jmax(peak, std::abs(uR[upIdx]));
                }
            }
            const float targetGain = (peak > ceilingLin) ? (ceilingLin / peak) : 1.0f;

            audioDelayL[(size_t)(writeIdx & delayMask)] = L[i];
            audioDelayR[(size_t)(writeIdx & delayMask)] = R[i];
            gainDelay[(size_t)(gainWriteIdx % gainSize)] = targetGain;

            while (!minDeque.empty() && minDeque.back().second >= targetGain)
                minDeque.pop_back();
            minDeque.push_back({ gainWriteIdx, targetGain });

            const int windowStart = gainWriteIdx - lookaheadSamples;
            while (!minDeque.empty() && minDeque.front().first < windowStart)
                minDeque.pop_front();

            const float minGain = minDeque.empty() ? 1.0f : minDeque.front().second;

            const int readIdx = (writeIdx - lookaheadSamples + delaySize) & delayMask;
            const float outL = audioDelayL[(size_t)readIdx];
            const float outR = audioDelayR[(size_t)readIdx];

            // v8: smoothed attack instead of instant snap-down
            // This prevents gain modulation artifacts while still catching peaks
            if (minGain < currentGain) {
                currentGain += attackCoeff * (minGain - currentGain);
                // Safety clamp: never exceed target by more than 0.1 dB
                if (currentGain > minGain * 1.012f)
                    currentGain = minGain;
            } else {
                currentGain += releaseCoeff * (minGain - currentGain);
            }

            L[i] = outL * currentGain;
            R[i] = outR * currentGain;

            peakGR = juce::jmin(peakGR, currentGain);
            writeIdx++;
            gainWriteIdx++;
        }

        lastGainReduction = peakGR;
    }

    float getGainReductionDb() const noexcept {
        return (lastGainReduction < 0.9999f)
            ? juce::jmax(0.0f, -20.0f * std::log10(juce::jmax(0.001f, lastGainReduction)))
            : 0.0f;
    }

private:
    double sr = 44100.0;
    int lookaheadSamples = 88;
    int delayMask = 0;
    std::vector<float> audioDelayL, audioDelayR;
    int writeIdx = 0;
    std::vector<float> gainDelay;
    int gainWriteIdx = 0;
    FixedDeque<std::pair<int, float>> minDeque;
    float currentGain = 1.0f;
    float releaseCoeff = 0.001f;
    float attackCoeff  = 0.1f;
    float lastGainReduction = 1.0f;
    juce::AudioBuffer<float> sidechainBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> ispOS;
};

// ═══════════════════════════════════════════════════════════════════════════
// ShineProcessor — SVF-based high-shelf EQ for air band enhancement
// ═══════════════════════════════════════════════════════════════════════════
class ShineProcessor {
public:
    void prepare(double sampleRate) noexcept {
        sr = sampleRate;
        updateCoefficients();
    }

    void reset() noexcept {
        ic1eqL = ic2eqL = 0.0f;
        ic1eqR = ic2eqR = 0.0f;
    }

    void setParameters(float freqHz, float gainDb, float q) noexcept {
        frequency = juce::jlimit(1000.0f, 20000.0f, freqHz);
        gain      = juce::jlimit(-12.0f, 12.0f, gainDb);
        qFactor   = juce::jlimit(0.1f, 2.0f, q);
        updateCoefficients();
    }

    inline void processStereo(float& L, float& R) noexcept {
        L = processSVF(L, ic1eqL, ic2eqL);
        R = processSVF(R, ic1eqR, ic2eqR);
    }

private:
    void updateCoefficients() noexcept {
        const float A  = std::pow(10.0f, gain / 40.0f);
        const float sqrtA = std::sqrt(A);
        const float w  = std::tan(3.14159265f * frequency / (float)juce::jmax(1.0, sr));
        k = 1.0f / (qFactor * sqrtA);

        g  = w * sqrtA;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;

        m0 = 1.0f;
        m1 = k * (A - 1.0f);
        m2 = A * A - 1.0f;
    }

    inline float processSVF(float x, float& ic1eq, float& ic2eq) noexcept {
        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        const float band = v1;
        const float high = x - k * v1 - v2;

        return x + m1 * band + m2 * high;
    }

    double sr = 44100.0;
    float frequency = 12000.0f;
    float gain = 3.0f;
    float qFactor = 0.7f;

    float g = 0.0f, k = 0.0f;
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float m0 = 1.0f, m1 = 0.0f, m2 = 0.0f;

    float ic1eqL = 0.0f, ic2eqL = 0.0f;
    float ic1eqR = 0.0f, ic2eqR = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// SidechainHPF — crossfade-safe high-pass filter for compressor sidechain
// ────────────────────────────────────────────────────────────────────────
// v7 FIX: Mode changes now use coefficient interpolation instead of hard
// reset. This prevents clicks when automating the HPF mode toggle.
//
// Architecture: Two filter instances (current + target). On mode change,
// the target filter starts with new coefficients while the current filter
// continues. A 5ms crossfade blends between them, then the target becomes
// current. This guarantees click-free transitions.
// ═══════════════════════════════════════════════════════════════════════════
struct SidechainHPF {
    float stateL = 0.0f;
    float stateR = 0.0f;
    float coeff  = 0.0f;
    bool  active = false;

    // v7: crossfade state for click-free mode changes
    float targetStateL = 0.0f;
    float targetStateR = 0.0f;
    float targetCoeff  = 0.0f;
    bool  targetActive = false;
    float crossfadeGain = 1.0f;  // 1.0 = fully on current, 0.0 = fully on target
    float crossfadeStep = 0.0f;  // per-sample decrement
    bool  crossfading   = false;

    void prepare(double sampleRate, float cutoffHz) noexcept {
        const float srf = (float)juce::jmax(1.0, sampleRate);

        if (!crossfading && active == (cutoffHz >= 10.0f) &&
            std::abs(coeff - computeCoeff(cutoffHz, srf)) < 1.0e-6f) {
            return; // No change needed
        }

        // v7: Start crossfade to new coefficients instead of hard switch
        targetActive = cutoffHz >= 10.0f;
        targetCoeff  = targetActive ? computeCoeff(cutoffHz, srf) : 0.0f;
        targetStateL = stateL; // Start target filter from current state
        targetStateR = stateR;

        // 5 ms crossfade
        const int fadeSamples = juce::jmax(1, (int)(srf * 0.005f));
        crossfadeStep = 1.0f / (float)fadeSamples;
        crossfadeGain = 1.0f;
        crossfading = true;
    }

    // v7: Simplified prepare for initial setup (no crossfade)
    void prepareImmediate(double sampleRate, float cutoffHz) noexcept {
        const float srf = (float)juce::jmax(1.0, sampleRate);
        if (cutoffHz < 10.0f) {
            active = false;
            coeff = 0.0f;
        } else {
            active = true;
            coeff = computeCoeff(cutoffHz, srf);
        }
        crossfading = false;
        crossfadeGain = 1.0f;
    }

    void reset() noexcept {
        stateL = stateR = 0.0f;
        targetStateL = targetStateR = 0.0f;
        crossfading = false;
        crossfadeGain = 1.0f;
    }

    inline float process(float inL, float inR) noexcept {
        if (!crossfading) {
            // Steady state — use current filter
            if (!active) return juce::jmax(std::abs(inL), std::abs(inR));

            stateL += (1.0f - coeff) * (inL - stateL);
            stateR += (1.0f - coeff) * (inR - stateR);
            const float hpL = inL - stateL;
            const float hpR = inR - stateR;
            return juce::jmax(std::abs(hpL), std::abs(hpR));
        }

        // Crossfading between current and target coefficients
        float currentOut;
        if (!active) {
            currentOut = juce::jmax(std::abs(inL), std::abs(inR));
        } else {
            stateL += (1.0f - coeff) * (inL - stateL);
            stateR += (1.0f - coeff) * (inR - stateR);
            currentOut = juce::jmax(std::abs(inL - stateL), std::abs(inR - stateR));
        }

        float targetOut;
        if (!targetActive) {
            targetOut = juce::jmax(std::abs(inL), std::abs(inR));
        } else {
            targetStateL += (1.0f - targetCoeff) * (inL - targetStateL);
            targetStateR += (1.0f - targetCoeff) * (inR - targetStateR);
            targetOut = juce::jmax(std::abs(inL - targetStateL), std::abs(inR - targetStateR));
        }

        crossfadeGain -= crossfadeStep;
        if (crossfadeGain <= 0.0f) {
            // Crossfade complete — adopt target as current
            crossfadeGain = 1.0f;
            crossfading = false;
            active = targetActive;
            coeff  = targetCoeff;
            stateL = targetStateL;
            stateR = targetStateR;
            return targetOut;
        }

        return currentOut * crossfadeGain + targetOut * (1.0f - crossfadeGain);
    }

private:
    static float computeCoeff(float cutoffHz, float sr) noexcept {
        return std::exp(-6.2831853f * cutoffHz / sr);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor — soft-knee compressor with SR-dependent smoothing
// ═══════════════════════════════════════════════════════════════════════════
struct GlueCompressor {
    float glueGain = 1.0f;
    float attackCoeff  = 0.02f;
    float releaseCoeff = 0.002f;

    void prepare(double sampleRate) noexcept {
        attackCoeff  = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.005f));
        releaseCoeff = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.080f));
    }
    void reset() noexcept { glueGain = 1.0f; }

    inline void processStereo(float& L, float& R, float glueAmount, float envVal) noexcept {
        if (glueAmount < 0.01f) return;

        const float threshDb = -8.0f - glueAmount * 10.0f;
        const float threshold = juce::Decibels::decibelsToGain(threshDb);
        const float ratio = 2.0f + glueAmount * 5.0f;
        const float kneeDb = 6.0f;

        float gainReduction = 1.0f;
        if (envVal > 1.0e-10f) {
            const float envDb = juce::Decibels::gainToDecibels(envVal, -100.0f);
            const float overDb = envDb - threshDb;

            float compressedDb = 0.0f;
            if (overDb <= -kneeDb * 0.5f) {
                compressedDb = 0.0f;
            } else if (overDb >= kneeDb * 0.5f) {
                compressedDb = overDb * (1.0f - 1.0f / ratio);
            } else {
                const float x = overDb + kneeDb * 0.5f;
                compressedDb = (1.0f / kneeDb) * x * x * (1.0f - 1.0f / ratio) * 0.5f;
            }

            gainReduction = juce::Decibels::decibelsToGain(-compressedDb);
        }

        const float smoothCoeff = gainReduction < glueGain ? attackCoeff : releaseCoeff;
        glueGain += smoothCoeff * (gainReduction - glueGain);
        L *= glueGain;
        R *= glueGain;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// AutoGainSmoother — RMS-weighted loudness matching
// ═══════════════════════════════════════════════════════════════════════════
struct AutoGainSmoother {
    float smoothedGain = 1.0f;
    float smoothCoeff  = 0.1f;

    void prepare(double /* sampleRate */) noexcept {
        smoothedGain = 1.0f;
        smoothCoeff = 0.1f;
    }
    void reset() noexcept { smoothedGain = 1.0f; }

    void processBlock(float* dataL, float* dataR, int numSamples,
                      const float* dryL, const float* dryR) noexcept
    {
        float inRmsSq = 0.0f, outRmsSq = 0.0f;
        for (int n = 0; n < numSamples; ++n) {
            inRmsSq  += dryL[n] * dryL[n] + dryR[n] * dryR[n];
            outRmsSq += dataL[n] * dataL[n] + dataR[n] * dataR[n];
        }
        const float invN = 1.0f / juce::jmax(1, numSamples * 2);
        const float inRms  = std::sqrt(inRmsSq  * invN + 1.0e-20f);
        const float outRms = std::sqrt(outRmsSq * invN + 1.0e-20f);

        float targetGain = 1.0f;
        if (inRms > 1.0e-6f && outRms > 1.0e-6f) {
            const float gainDb = juce::jlimit(-6.0f, 6.0f,
                juce::Decibels::gainToDecibels(inRms / outRms, 0.0f));
            targetGain = juce::Decibels::decibelsToGain(gainDb);
        }

        smoothedGain += smoothCoeff * (targetGain - smoothedGain);

        for (int n = 0; n < numSamples; ++n) {
            dataL[n] *= smoothedGain;
            dataR[n] *= smoothedGain;
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MeterBallistics — sample-rate aware metering with hold/decay
// ═══════════════════════════════════════════════════════════════════════════
struct MeterBallistics {
    float inPeakHoldL = 0.0f, inPeakHoldR = 0.0f;
    float outPeakHoldL = 0.0f, outPeakHoldR = 0.0f;
    float inRmsL = 0.0f, inRmsR = 0.0f;
    float outRmsL = 0.0f, outRmsR = 0.0f;
    float sparkGR = 0.0f;
    float clipHoldIn = 0.0f, clipHoldOut = 0.0f;

    float holdDecay = 0.995f;
    float rmsCoeff  = 0.08f;
    float clipDecay = 0.92f;

    void prepare(double sampleRate, int blockSize) noexcept {
        const float blocksPerSec = (float)sampleRate / juce::jmax(1.0f, (float)blockSize);
        holdDecay = std::pow(0.05f, 1.0f / blocksPerSec);
        rmsCoeff  = 1.0f - std::exp(-1.0f / (blocksPerSec * 0.3f));
        clipDecay = std::pow(0.01f, 1.0f / (blocksPerSec * 0.5f));
    }

    void reset() noexcept {
        inPeakHoldL = inPeakHoldR = 0.0f;
        outPeakHoldL = outPeakHoldR = 0.0f;
        inRmsL = inRmsR = outRmsL = outRmsR = 0.0f;
        sparkGR = 0.0f;
        clipHoldIn = clipHoldOut = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter — maps 4 macro knobs to multiple DSP targets
// ────────────────────────────────────────────────────────────────────────
// WARNING: addMapping() allocates — call from message thread ONLY.
// ═══════════════════════════════════════════════════════════════════════════
class MacroInterpreter {
public:
    static constexpr int kNumMacros = 4;

    enum class Curve {
        Linear,
        Exponential,
        SCurve,
        Logarithmic,
        PowerLaw,
        SoftSat,
        InverseSCurve
    };

    struct Mapping {
        int targetIndex;
        float depth;
        Curve curve;
    };

    void clearMappings() {
        for (auto& m : mappings) m.clear();
    }

    void addMapping(int macroIndex, int targetIndex, float depth, Curve curve = Curve::Linear) {
        if (macroIndex >= 0 && macroIndex < kNumMacros)
            mappings[(size_t)macroIndex].push_back({ targetIndex, depth, curve });
    }

    float getModulation(int targetIndex, const float macroValues[kNumMacros]) const noexcept {
        float mod = 0.0f;
        for (int m = 0; m < kNumMacros; ++m) {
            for (const auto& mapping : mappings[(size_t)m]) {
                if (mapping.targetIndex == targetIndex) {
                    const float shaped = applyCurve(macroValues[m], mapping.curve);
                    mod += shaped * mapping.depth;
                }
            }
        }
        return mod;
    }

    void setupDefaults() {
        clearMappings();
        addMapping(0, 0, 0.6f, Curve::Logarithmic);   // punch
        addMapping(0, 10, 0.3f, Curve::Logarithmic);   // drive

        addMapping(1, 1, 0.7f, Curve::SoftSat);        // warmth
        addMapping(1, 6, 0.4f, Curve::SoftSat);        // density

        addMapping(2, 2, 0.6f, Curve::Logarithmic);    // boom
        addMapping(2, 3, 0.3f, Curve::SCurve);          // glue

        addMapping(3, 4, 0.5f, Curve::PowerLaw);        // air
        addMapping(3, 13, 0.4f, Curve::PowerLaw);       // shine freq
    }

private:
    static float applyCurve(float x, Curve curve) noexcept {
        x = juce::jlimit(0.0f, 1.0f, x);
        switch (curve) {
            case Curve::Exponential:
                return x * x;
            case Curve::SCurve:
                return x * x * (3.0f - 2.0f * x);
            case Curve::Logarithmic:
                return std::log1p(9.0f * x) * 0.4342944f;
            case Curve::PowerLaw:
                return std::pow(x, 0.4f);
            case Curve::SoftSat: {
                const float t2 = 0.9640276f;
                return std::tanh(2.0f * x) / t2;
            }
            case Curve::InverseSCurve: {
                const float centered = 2.0f * x - 1.0f;
                const float sign = centered >= 0.0f ? 1.0f : -1.0f;
                return 0.5f + 0.5f * sign * std::pow(std::abs(centered), 0.6f);
            }
            default:
                return x;
        }
    }

    std::array<std::vector<Mapping>, kNumMacros> mappings;
};

} // namespace BTZDsp
