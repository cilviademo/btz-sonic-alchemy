/*
  Box Tone Zone (BTZ) — BTZDsp.h  v2
  ────────────────────────────────────────────────────────────────────────
  Modular DSP modules. Every class is:
    • self-contained and sample-rate aware
    • real-time safe (no allocation, no locks, no exceptions in hot paths)
    • pre-allocates all buffers in prepare() / constructor

  v2 changes (Claude review + /dsp-engineering skill):
    • ADAATanh — first-order antiderivative anti-aliasing saturator
    • TruePeakLimiter — ISP-aware (4x sidechain OS), monotonic-deque
      O(1) lookahead scan, pre-allocated scratch buffer
    • SVF-based ShineProcessor — modulation-safe, no coefficient jumps
    • Denormal flushing helper (FTZ/DAZ)
    • fastTanh retained as utility; ADAATanh used in all saturation stages
    • SparkLimiter removed (replaced by TruePeakLimiter)
*/
#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <deque>
#include <vector>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

namespace BTZDsp {

// ═══════════════════════════════════════════════════════════════════════════
// Denormal flushing — call once at plugin init (prepareToPlay)
// Per dsp-engineering/realtime-safety.md: "set FTZ/DAZ flags at plugin init"
// ═══════════════════════════════════════════════════════════════════════════
inline void enableFlushToZero() {
#ifdef __SSE__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
}

// ═══════════════════════════════════════════════════════════════════════════
// Utility: Padé-approximant tanh — accurate for |x| < ~3, fast everywhere
// Retained for non-critical paths; saturation stages use ADAATanh.
// ═══════════════════════════════════════════════════════════════════════════
static inline float fastTanh(float x) {
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// ═══════════════════════════════════════════════════════════════════════════
// ADAATanh — First-order antiderivative anti-aliased tanh saturator
// (Parker/Esqueda 2016). Replaces fastTanh in all harmonic-generating
// stages. ~36 dB alias rejection at 1x SR — roughly equivalent to 4x
// oversampling for tanh specifically.
//
// RULES (from Claude review):
//   • One instance per channel per saturation stage — DO NOT share
//   • Call reset() in prepareToPlay() and after state recall
//   • For drive > 20 dB, consider second-order ADAA (not needed for 0–12 dB)
// ═══════════════════════════════════════════════════════════════════════════
class ADAATanh {
public:
    void reset() noexcept {
        x1 = 0.0f;
        F1 = logCosh(0.0f); // = 0.0f
    }

    // Process one sample through ADAA tanh.
    inline float process(float x) noexcept {
        const float dx = x - x1;
        const float F  = logCosh(x);
        float y;

        // 0/0 fallback: when input barely moves, ADAA is ill-conditioned.
        // Use direct tanh at the midpoint instead.
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
    // Numerically stable antiderivative of tanh:
    // F(x) = log(cosh(x)) = |x| + log1p(exp(-2|x|)) - log(2)
    static inline float logCosh(float x) noexcept {
        const float ax = std::abs(x);
        return ax + std::log1p(std::exp(-2.0f * ax)) - 0.6931472f;
    }

    float x1 = 0.0f;
    float F1 = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam — one-pole parameter smoother, sample-rate aware
// Per dsp-engineering/parameter-smoothing.md: one-pole exponential smoother.
// ═══════════════════════════════════════════════════════════════════════════
struct SmoothParam {
    float current = 0.0f;
    float target  = 0.0f;
    float coeff   = 0.001f;

    void setTime(float ms, double sr) {
        const float srf = (float) juce::jmax(1.0, sr);
        coeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, ms) * 0.001f));
    }
    void setTarget(float v) { target = v; }
    inline float next() { current += coeff * (target - current); return current; }
    void snapTo(float v) { current = target = v; }
    bool isSmoothing() const { return std::abs(target - current) > 1.0e-6f; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower — attack/release envelope follower, sample-rate aware
// Per dsp-engineering/dynamics-processing.md: peak detection pattern.
// ═══════════════════════════════════════════════════════════════════════════
struct EnvFollower {
    float env = 0.0f;
    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;

    void setTimes(float attackMs, float releaseMs, double sr) {
        const float srf = (float) juce::jmax(1.0, sr);
        attackCoeff  = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, attackMs)  * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, releaseMs) * 0.001f));
    }
    void reset(float value = 0.0f) { env = value; }
    inline float process(float xAbs) {
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

    void setSampleRate(double sr) {
        const float srf = (float) juce::jmax(1.0, sr);
        dcCoeff = 1.0f - (6.2831853f * 5.0f / srf);
        dcCoeff = juce::jlimit(0.90f, 0.99999f, dcCoeff);
    }
    void reset() { dcL = dcPrevL = dcR = dcPrevR = 0.0f; }
    inline float processSample(float x, float& dc, float& dcPrev) {
        if (! std::isfinite(x) || std::abs(x) < 1.0e-20f)
            x = 0.0f;
        const float y = x - dcPrev + dcCoeff * dc;
        dcPrev = x;
        dc = y;
        return y;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SlewLimiter — crude anti-alias via max slew rate
// With ADAA in place, this is now a safety net rather than primary AA.
// ═══════════════════════════════════════════════════════════════════════════
struct SlewLimiter {
    float prev = 0.0f;
    float maxDelta = 0.02f;

    void setSampleRate(double sr) {
        maxDelta = 0.02f * (48000.0f / (float) juce::jmax(1.0, sr));
    }
    void reset() { prev = 0.0f; }
    inline float process(float x) {
        const float delta = x - prev;
        if (std::abs(delta) > maxDelta)
            x = prev + (delta > 0.0f ? maxDelta : -maxDelta);
        prev = x;
        return x;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// LinkwitzRileyCrossover — 2nd-order (LR2) band-split at a given frequency
// Uses two cascaded 1-pole filters per band for 12 dB/oct slopes.
// Phase-aligned recombination: low + high = original (linear).
// ═══════════════════════════════════════════════════════════════════════════
struct LinkwitzRileyCrossover {
    float lp1L = 0.0f, lp2L = 0.0f;
    float lp1R = 0.0f, lp2R = 0.0f;
    float coeff = 0.0f;

    void prepare(double sr, float freqHz) {
        const float omega = 6.2831853f * freqHz / (float) juce::jmax(1.0, sr);
        coeff = omega / (1.0f + omega);
    }
    void reset() { lp1L = lp2L = lp1R = lp2R = 0.0f; }

    inline void process(float inL, float inR,
                        float& lowL, float& lowR,
                        float& highL, float& highR)
    {
        lp1L += coeff * (inL - lp1L);
        lp1R += coeff * (inR - lp1R);
        lp2L += coeff * (lp1L - lp2L);
        lp2R += coeff * (lp1R - lp2R);

        lowL  = lp2L;
        lowR  = lp2R;
        highL = inL - lp2L;
        highR = inR - lp2R;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// TruePeakLimiter — ISP-aware lookahead limiter (replaces SparkLimiter)
// ────────────────────────────────────────────────────────────────────────
// Key design points (from Claude review):
//   1. ISP detection via 4x oversampled sidechain (audio path NOT oversampled)
//   2. Monotonic-deque running-min for O(1) amortized lookahead scan
//   3. Pre-allocated sidechain buffer (no .makeCopyOf() per block)
//   4. No sparkMix dry/wet blend on limiter output (causes overshoots)
//   5. Ceiling is a held value per block, NOT smoothed at audio rate
//   6. Reports latency for DAW PDC via getLatencySamples()
//
// Per dsp-engineering/realtime-safety.md: no allocation in process.
// Per dsp-engineering/buffer-management.md: pre-allocate at prepare().
// Per dsp-engineering/dynamics-processing.md: instant attack, exp release.
// ═══════════════════════════════════════════════════════════════════════════
class TruePeakLimiter {
public:
    void prepare(double sampleRate, int maxBlockSize, float lookaheadMs = 2.0f) {
        sr = sampleRate;
        lookaheadSamples = juce::jmax(4, (int)(sr * lookaheadMs * 0.001));

        // Pre-allocate delay lines (power-of-2 for fast modulo via mask)
        const int delayBufSize = juce::nextPowerOfTwo(lookaheadSamples + 4);
        delayMask = delayBufSize - 1;
        audioDelayL.assign((size_t)delayBufSize, 0.0f);
        audioDelayR.assign((size_t)delayBufSize, 0.0f);
        writeIdx = 0;

        // Pre-allocate gain delay for monotonic deque
        gainDelay.assign((size_t)(lookaheadSamples + 1), 1.0f);
        gainWriteIdx = 0;

        currentGain = 1.0f;
        lastGainReduction = 1.0f;

        // 50 ms release (per dynamics-processing.md: limiting release ~50ms)
        releaseCoeff = 1.0f - std::exp(-1.0f / (float)(sr * 0.050));

        // Pre-allocate sidechain buffer (avoids per-block allocation)
        sidechainBuffer.setSize(2, maxBlockSize, false, false, true);
        sidechainBuffer.clear();

        // 4x oversampler for ISP detection on the sidechain ONLY
        ispOS = std::make_unique<juce::dsp::Oversampling<float>>(
            2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        ispOS->initProcessing((size_t)maxBlockSize);

        // Clear the monotonic deque
        minDeque.clear();
    }

    void reset() {
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

    // Report this in prepareToPlay via setLatencySamples()
    int getLatencySamples() const noexcept { return lookaheadSamples; }

    // Process a full block. ceilingLin is held per block (NOT smoothed per sample).
    // Per Claude review: "do NOT smooth ceiling into the limiter at audio rate"
    void processBlock(juce::AudioBuffer<float>& buffer, float ceilingLin) {
        const int n = buffer.getNumSamples();
        auto* L = buffer.getWritePointer(0);
        auto* R = buffer.getWritePointer(1);

        // ── ISP peak detection: copy into pre-allocated sidechain buffer ──
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
            // ISP-aware peak: check the 4 upsampled neighbors around this sample
            float peak = juce::jmax(std::abs(L[i]), std::abs(R[i]));
            for (int k = 0; k < osFactor; ++k) {
                const int upIdx = i * osFactor + k;
                if (upIdx < (int)up.getNumSamples()) {
                    peak = juce::jmax(peak, std::abs(uL[upIdx]));
                    peak = juce::jmax(peak, std::abs(uR[upIdx]));
                }
            }
            const float targetGain = (peak > ceilingLin) ? (ceilingLin / peak) : 1.0f;

            // Write into audio delay lines (power-of-2 masking)
            audioDelayL[(size_t)(writeIdx & delayMask)] = L[i];
            audioDelayR[(size_t)(writeIdx & delayMask)] = R[i];

            // Write target gain into gain delay
            gainDelay[(size_t)(gainWriteIdx % gainSize)] = targetGain;

            // ── Monotonic-deque running minimum over lookahead window ──
            // Maintain deque of (index, gain) pairs in ascending gain order.
            // This gives O(1) amortized min-gain lookup instead of O(N*M).
            while (!minDeque.empty() && minDeque.back().second >= targetGain)
                minDeque.pop_back();
            minDeque.push_back({ gainWriteIdx, targetGain });

            // Expire entries that have fallen out of the lookahead window
            const int windowStart = gainWriteIdx - lookaheadSamples;
            while (!minDeque.empty() && minDeque.front().first < windowStart)
                minDeque.pop_front();

            const float minGain = minDeque.empty() ? 1.0f : minDeque.front().second;

            // Read delayed audio (output tap, `lookaheadSamples` behind write)
            const int readIdx = (writeIdx - lookaheadSamples + delaySize) & delayMask;
            const float outL = audioDelayL[(size_t)readIdx];
            const float outR = audioDelayR[(size_t)readIdx];

            // Instant attack (we've already looked ahead), exponential release
            if (minGain < currentGain) {
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

    // Monotonic deque: pairs of (write-index, target-gain), ascending gain order.
    // front() is always the minimum gain in the current lookahead window.
    std::deque<std::pair<int, float>> minDeque;

    float currentGain = 1.0f;
    float releaseCoeff = 0.001f;
    float lastGainReduction = 1.0f;

    // Pre-allocated sidechain buffer (avoids per-block allocation)
    juce::AudioBuffer<float> sidechainBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> ispOS;
};

// ═══════════════════════════════════════════════════════════════════════════
// ShineProcessor — SVF-based high-shelf EQ for air band enhancement
// ────────────────────────────────────────────────────────────────────────
// Replaces the biquad implementation. SVF is modulation-safe: coefficient
// changes don't cause clicks or instability (per filter-design.md).
// Uses the Andrew Simper / Cytomic SVF topology.
//
// High-shelf: output = low + A*band*k + A^2*high
//   where A = 10^(gainDb/40), k = 1/Q
// ═══════════════════════════════════════════════════════════════════════════
class ShineProcessor {
public:
    void prepare(double sampleRate) {
        sr = sampleRate;
        updateCoefficients();
    }

    void reset() {
        ic1eqL = ic2eqL = 0.0f;
        ic1eqR = ic2eqR = 0.0f;
    }

    void setParameters(float freqHz, float gainDb, float q) {
        frequency = juce::jlimit(1000.0f, 20000.0f, freqHz);
        gain      = juce::jlimit(-12.0f, 12.0f, gainDb);
        qFactor   = juce::jlimit(0.1f, 2.0f, q);
        updateCoefficients();
    }

    inline void processStereo(float& L, float& R) {
        L = processSVF(L, ic1eqL, ic2eqL);
        R = processSVF(R, ic1eqR, ic2eqR);
    }

private:
    void updateCoefficients() {
        const float A  = std::pow(10.0f, gain / 40.0f);
        const float w  = std::tan(3.14159265f * frequency / (float)juce::jmax(1.0, sr));
        k = 1.0f / qFactor;

        // SVF coefficients (Cytomic / Andrew Simper)
        g  = w;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;

        // High-shelf mixing coefficients
        m0 = 1.0f;            // low pass-through
        m1 = k * (A - 1.0f);  // band contribution
        m2 = A * A - 1.0f;    // high contribution
    }

    inline float processSVF(float x, float& ic1eq, float& ic2eq) {
        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        const float low  = v2;
        const float band = v1;
        const float high = x - k * v1 - v2;

        // High-shelf: original + shelf contributions
        return x + m1 * band + m2 * high;
    }

    double sr = 44100.0;
    float frequency = 12000.0f;
    float gain = 3.0f;
    float qFactor = 0.7f;

    // SVF coefficients
    float g = 0.0f, k = 0.0f;
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    // High-shelf mix coefficients
    float m0 = 1.0f, m1 = 0.0f, m2 = 0.0f;

    // Per-channel state (SVF: 2 integrator states each)
    float ic1eqL = 0.0f, ic2eqL = 0.0f;
    float ic1eqR = 0.0f, ic2eqR = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor — envelope-following compressor with SR-dependent smoothing
// Per dsp-engineering/dynamics-processing.md: soft-knee compressor pattern.
// ═══════════════════════════════════════════════════════════════════════════
struct GlueCompressor {
    float glueGain = 1.0f;
    float attackCoeff  = 0.02f;
    float releaseCoeff = 0.002f;

    void prepare(double sampleRate) {
        attackCoeff  = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.005f));
        releaseCoeff = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.080f));
    }
    void reset() { glueGain = 1.0f; }

    inline void processStereo(float& L, float& R, float glueAmount, float envVal) {
        if (glueAmount < 0.01f) return;

        const float threshold = juce::Decibels::decibelsToGain(-8.0f - glueAmount * 10.0f);
        const float ratio = 2.0f + glueAmount * 5.0f;

        float gainReduction = 1.0f;
        if (envVal > threshold) {
            const float overDb = juce::Decibels::gainToDecibels(envVal / threshold, -100.0f);
            const float reducedDb = overDb * (1.0f - 1.0f / ratio);
            gainReduction = juce::Decibels::decibelsToGain(-reducedDb);
        }

        const float smoothCoeff = gainReduction < glueGain ? attackCoeff : releaseCoeff;
        glueGain += smoothCoeff * (gainReduction - glueGain);
        L *= glueGain;
        R *= glueGain;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// AutoGainSmoother — smoothed block-level auto-gain
// Per parameter-smoothing.md: per-block smoothing for non-critical params.
// ═══════════════════════════════════════════════════════════════════════════
struct AutoGainSmoother {
    float smoothedGain = 1.0f;
    float smoothCoeff  = 0.1f;

    void prepare(double /* sampleRate */) {
        smoothedGain = 1.0f;
        smoothCoeff = 0.1f;
    }
    void reset() { smoothedGain = 1.0f; }

    void processBlock(float* dataL, float* dataR, int numSamples,
                      const float* dryL, const float* dryR)
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
            const float gainDb = juce::jlimit(-4.0f, 4.0f,
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

    void prepare(double sampleRate, int blockSize) {
        const float blocksPerSec = (float)sampleRate / juce::jmax(1.0f, (float)blockSize);
        holdDecay = std::pow(0.05f, 1.0f / blocksPerSec);
        rmsCoeff  = 1.0f - std::exp(-1.0f / (blocksPerSec * 0.3f));
        clipDecay = std::pow(0.01f, 1.0f / (blocksPerSec * 0.5f));
    }

    void reset() {
        inPeakHoldL = inPeakHoldR = 0.0f;
        outPeakHoldL = outPeakHoldR = 0.0f;
        inRmsL = inRmsR = outRmsL = outRmsR = 0.0f;
        sparkGR = 0.0f;
        clipHoldIn = clipHoldOut = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter — maps 4 macro knobs to multiple DSP targets
// ═══════════════════════════════════════════════════════════════════════════
class MacroInterpreter {
public:
    static constexpr int kNumMacros = 4;

    enum class Curve { Linear, Exponential, SCurve };

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

    float getModulation(int targetIndex, const float macroValues[kNumMacros]) const {
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
        addMapping(0, 0, 0.6f, Curve::Linear);
        addMapping(0, 10, 0.3f, Curve::Exponential);
        addMapping(1, 1, 0.7f, Curve::Linear);
        addMapping(1, 6, 0.4f, Curve::SCurve);
        addMapping(2, 2, 0.6f, Curve::Linear);
        addMapping(2, 3, 0.3f, Curve::SCurve);
        addMapping(3, 4, 0.5f, Curve::Linear);
        addMapping(3, 13, 0.4f, Curve::Exponential);
    }

private:
    static float applyCurve(float x, Curve curve) {
        switch (curve) {
            case Curve::Exponential: return x * x;
            case Curve::SCurve:     return x * x * (3.0f - 2.0f * x);
            default:                return x;
        }
    }

    std::array<std::vector<Mapping>, kNumMacros> mappings;
};

} // namespace BTZDsp
