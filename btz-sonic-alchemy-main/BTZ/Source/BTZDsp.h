/*
  Box Tone Zone (BTZ) — BTZDsp.h
  Extracted DSP module classes for clean architecture and testability.
  Each module is self-contained, sample-rate aware, and real-time safe.
*/
#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <vector>

namespace BTZDsp {

// ═══════════════════════════════════════════════════════════════════════════
// Utility: Padé-approximant tanh — accurate for |x| < ~3, fast everywhere
// ═══════════════════════════════════════════════════════════════════════════
static inline float fastTanh(float x) {
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam — one-pole parameter smoother, sample-rate aware
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
    float next() { current += coeff * (target - current); return current; }
    void snapTo(float v) { current = target = v; }
};

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower — attack/release envelope follower, sample-rate aware
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
    float process(float xAbs) {
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
    float processSample(float x, float& dc, float& dcPrev) {
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
// ═══════════════════════════════════════════════════════════════════════════
struct SlewLimiter {
    float prev = 0.0f;
    float maxDelta = 0.02f;

    void setSampleRate(double sr) {
        maxDelta = 0.02f * (48000.0f / (float) juce::jmax(1.0, sr));
    }
    void reset() { prev = 0.0f; }
    float process(float x) {
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
    // State for two cascaded 1-pole lowpass stages, per channel
    float lp1L = 0.0f, lp2L = 0.0f;
    float lp1R = 0.0f, lp2R = 0.0f;
    float coeff = 0.0f;

    void prepare(double sr, float freqHz) {
        const float omega = 6.2831853f * freqHz / (float) juce::jmax(1.0, sr);
        coeff = omega / (1.0f + omega);
    }
    void reset() { lp1L = lp2L = lp1R = lp2R = 0.0f; }

    // Process one stereo sample. Writes lowL, lowR, highL, highR.
    void process(float inL, float inR,
                 float& lowL, float& lowR,
                 float& highL, float& highR)
    {
        // First-order stage 1
        lp1L += coeff * (inL - lp1L);
        lp1R += coeff * (inR - lp1R);
        // First-order stage 2 (cascaded)
        lp2L += coeff * (lp1L - lp2L);
        lp2R += coeff * (lp1R - lp2R);

        lowL  = lp2L;
        lowR  = lp2R;
        highL = inL - lp2L;
        highR = inR - lp2R;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SparkLimiter — soft-clip + hard ceiling with lookahead and GR tracking
// Replaces the old hard-clipper. True-peak safe when sparkMix == 1.0.
// ═══════════════════════════════════════════════════════════════════════════
class SparkLimiter {
public:
    void prepare(double sampleRate, int /* maxBlockSize */) {
        sr = sampleRate;
        // 1 ms lookahead
        lookaheadSamples = juce::jmax(1, (int)(sr * 0.001));
        delayL.assign((size_t)(lookaheadSamples + 1), 0.0f);
        delayR.assign((size_t)(lookaheadSamples + 1), 0.0f);
        writeIdx = 0;
        gainReduction = 1.0f;

        // Envelope coefficients
        attackCoeff  = 1.0f - std::exp(-1.0f / ((float)sr * 0.0002f));  // 0.2 ms
        releaseCoeff = 1.0f - std::exp(-1.0f / ((float)sr * 0.050f));   // 50 ms
    }

    void reset() {
        std::fill(delayL.begin(), delayL.end(), 0.0f);
        std::fill(delayR.begin(), delayR.end(), 0.0f);
        writeIdx = 0;
        gainReduction = 1.0f;
    }

    int getLatency() const { return lookaheadSamples; }

    // Process one stereo sample. Returns instantaneous GR in dB (positive).
    float processStereo(float& L, float& R, float ceilingLin, float sparkMix) {
        const int bufSize = (int)delayL.size();

        // Write current samples into delay line
        delayL[(size_t)writeIdx] = L;
        delayR[(size_t)writeIdx] = R;

        // Read delayed samples
        const int readIdx = (writeIdx - lookaheadSamples + bufSize) % bufSize;
        const float delL = delayL[(size_t)readIdx];
        const float delR = delayR[(size_t)readIdx];

        // Advance write pointer
        writeIdx = (writeIdx + 1) % bufSize;

        // Calculate target GR from current (future) input peak
        const float absPeak = juce::jmax(std::abs(L), std::abs(R));
        float targetGR = 1.0f;
        if (absPeak > ceilingLin && ceilingLin > 1.0e-10f) {
            // Soft-clip the gain reduction curve for musical behavior
            const float overRatio = absPeak / ceilingLin;
            // Soft knee: tanh-based compression of the overshoot
            targetGR = ceilingLin / (ceilingLin + fastTanh(overRatio - 1.0f) * (absPeak - ceilingLin));
            targetGR = juce::jlimit(0.01f, 1.0f, targetGR);
        }

        // Smooth GR envelope (fast attack, slow release)
        const float c = (targetGR < gainReduction) ? attackCoeff : releaseCoeff;
        gainReduction += c * (targetGR - gainReduction);

        // Apply GR to delayed signal, then blend with dry via sparkMix
        const float wetL = delL * gainReduction;
        const float wetR = delR * gainReduction;

        // Hard ceiling safety clamp (guarantees no overshoot at sparkMix == 1.0)
        const float clampL = juce::jlimit(-ceilingLin, ceilingLin, wetL);
        const float clampR = juce::jlimit(-ceilingLin, ceilingLin, wetR);

        // Mix: at sparkMix == 1.0, output is fully limited. At 0.0, passthrough.
        L = delL + (clampL - delL) * sparkMix;
        R = delR + (clampR - delR) * sparkMix;

        // Return GR in dB (positive value)
        return (gainReduction < 0.9999f)
            ? juce::jmax(0.0f, -20.0f * std::log10(juce::jmax(0.001f, gainReduction)))
            : 0.0f;
    }

private:
    double sr = 44100.0;
    int lookaheadSamples = 44;
    std::vector<float> delayL, delayR;
    int writeIdx = 0;
    float gainReduction = 1.0f;
    float attackCoeff = 0.2f;
    float releaseCoeff = 0.01f;
};

// ═══════════════════════════════════════════════════════════════════════════
// ShineProcessor — proper high-shelf EQ for air band enhancement
// Uses JUCE IIR coefficients for a musical, parametric air band.
// ═══════════════════════════════════════════════════════════════════════════
class ShineProcessor {
public:
    void prepare(double sampleRate) {
        sr = sampleRate;
        updateCoefficients();
    }

    void reset() {
        std::fill(stateL.begin(), stateL.end(), 0.0f);
        std::fill(stateR.begin(), stateR.end(), 0.0f);
    }

    void setParameters(float freqHz, float gainDb, float q) {
        frequency = juce::jlimit(1000.0f, 20000.0f, freqHz);
        gain      = juce::jlimit(-12.0f, 12.0f, gainDb);
        qFactor   = juce::jlimit(0.1f, 2.0f, q);
        updateCoefficients();
    }

    void processStereo(float& L, float& R) {
        L = processBiquad(L, stateL);
        R = processBiquad(R, stateR);
    }

private:
    void updateCoefficients() {
        // High-shelf biquad coefficients (Robert Bristow-Johnson cookbook)
        const float A  = std::pow(10.0f, gain / 40.0f);
        const float w0 = 6.2831853f * frequency / (float)juce::jmax(1.0, sr);
        const float sinW0 = std::sin(w0);
        const float cosW0 = std::cos(w0);
        const float alpha = sinW0 / (2.0f * qFactor);
        const float sqrtA = std::sqrt(A);

        const float a0 =        (A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha;
        const float a1 =  2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0);
        const float a2 =        (A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha;
        const float b0 =    A * ((A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha);
        const float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0);
        const float b2 =    A * ((A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha);

        const float invA0 = 1.0f / a0;
        coeffs[0] = b0 * invA0;
        coeffs[1] = b1 * invA0;
        coeffs[2] = b2 * invA0;
        coeffs[3] = a1 * invA0;
        coeffs[4] = a2 * invA0;
    }

    float processBiquad(float x, std::array<float, 4>& s) {
        const float y = coeffs[0] * x + s[0];
        s[0] = coeffs[1] * x - coeffs[3] * y + s[1];
        s[1] = coeffs[2] * x - coeffs[4] * y;
        return y;
    }

    double sr = 44100.0;
    float frequency = 12000.0f;
    float gain = 3.0f;
    float qFactor = 0.7f;
    std::array<float, 5> coeffs = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<float, 4> stateL = {}, stateR = {};
};

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor — envelope-following compressor with SR-dependent smoothing
// ═══════════════════════════════════════════════════════════════════════════
struct GlueCompressor {
    float glueGain = 1.0f;
    float attackCoeff  = 0.02f;
    float releaseCoeff = 0.002f;

    void prepare(double sampleRate) {
        // 5 ms attack, 80 ms release
        attackCoeff  = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.005f));
        releaseCoeff = 1.0f - std::exp(-1.0f / ((float)sampleRate * 0.080f));
    }
    void reset() { glueGain = 1.0f; }

    void processStereo(float& L, float& R, float glueAmount, float envVal) {
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
// AutoGainSmoother — smoothed block-level auto-gain to prevent block-boundary clicks
// ═══════════════════════════════════════════════════════════════════════════
struct AutoGainSmoother {
    float smoothedGain = 1.0f;
    float smoothCoeff  = 0.1f;

    void prepare(double /* sampleRate */) {
        smoothedGain = 1.0f;
        // Block-level smoothing: ~100 ms convergence at typical block rates
        smoothCoeff = 0.1f;
    }
    void reset() { smoothedGain = 1.0f; }

    // Call once per block with input/output RMS. Applies smoothed gain to buffer.
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

        // Smooth between blocks to prevent clicks
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

    // SR-dependent coefficients
    float holdDecay = 0.995f;
    float rmsCoeff  = 0.08f;
    float clipDecay = 0.92f;

    void prepare(double sampleRate, int blockSize) {
        // Target: ~300 ms peak hold at any sample rate
        // holdDecay^(blocksPerSecond) ≈ target decay per second
        const float blocksPerSec = (float)sampleRate / juce::jmax(1.0f, (float)blockSize);
        holdDecay = std::pow(0.05f, 1.0f / blocksPerSec);   // decay to 5% in 1 second
        rmsCoeff  = 1.0f - std::exp(-1.0f / (blocksPerSec * 0.3f)); // 300 ms RMS smoothing
        clipDecay = std::pow(0.01f, 1.0f / (blocksPerSec * 0.5f));  // 500 ms clip hold
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
        int targetIndex;   // index into the target parameter array
        float depth;       // -1.0 to 1.0
        Curve curve;
    };

    void clearMappings() {
        for (auto& m : mappings) m.clear();
    }

    void addMapping(int macroIndex, int targetIndex, float depth, Curve curve = Curve::Linear) {
        if (macroIndex >= 0 && macroIndex < kNumMacros)
            mappings[(size_t)macroIndex].push_back({ targetIndex, depth, curve });
    }

    // Apply macro modulation to a target value. Returns modulated value clamped to [0, 1].
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

    // Setup default BTZ macro assignments
    void setupDefaults() {
        clearMappings();
        // Macro 0 "PUNCH": drives punch + drive
        addMapping(0, 0, 0.6f, Curve::Linear);     // punch
        addMapping(0, 10, 0.3f, Curve::Exponential); // drive

        // Macro 1 "WARMTH": drives warmth + density
        addMapping(1, 1, 0.7f, Curve::Linear);     // warmth
        addMapping(1, 6, 0.4f, Curve::SCurve);     // density

        // Macro 2 "BOOM": drives boom + glue
        addMapping(2, 2, 0.6f, Curve::Linear);     // boom
        addMapping(2, 3, 0.3f, Curve::SCurve);     // glue

        // Macro 3 "AIR": drives air + shine
        addMapping(3, 4, 0.5f, Curve::Linear);     // air
        addMapping(3, 13, 0.4f, Curve::Exponential); // shineAmount
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
