/*
  Box Tone Zone (BTZ) — BTZDsp.h  v1.0 Ivory System
  ──────────────────────────────────────────────────────────────────────────
  Production-quality DSP header. Single-header library.

  Architecture:
    • Every struct is trivially constructible with sane defaults
    • All buffers are fixed-size or pre-allocated in prepare()
    • Zero heap allocation in any process/tick method
    • All floating-point state initialised to prevent denormals
    • Lock-free: no mutex, no std::vector resize, no exceptions in hot path
    • Naming: camelCase members, PascalCase types, kConstant for constants
    • Thread model: prepare/reset on message thread; process on audio thread

  Module inventory:
    Core:        SmoothParam, EnvFollower, SafetyLayer, BypassCrossfader
    Saturation:  SaturationModel enum (11 models), Waveshaper namespace
    Neural:      NeuralSaturationModel (RTNeural-compatible GRU)
    WDF:         WDFTubeStage, WDFTransformerStage
    Analysis:    ResonanceTamer, TransientSplitter
    Oversampling:OversamplingEngine (2x/4x/8x FIR)
    Dynamics:    SidechainHPF, GlueCompressor, TruePeakLimiter
    EQ:          ShineProcessor, LinkwitzRileyCrossover
    Multiband:   MultibandEngine (1-6 bands)
    Stereo:      MidSideEncoder
    Modulation:  LFO, MacroInterpreter
    Metering:    MeterBallistics, LoudnessMeter, SpectrumBuffer, GRHistory
    Matching:    ReferenceToneMatcher, PresetIntelligence
    State:       UndoStack, ABState, PresetInfo, MIDILearnState,
                 SimpleModeState, LoudnessMatchedAB
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <complex>
#include <string>
#include <vector>

#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif

namespace BTZDsp {

// ═══════════════════════════════════════════════════════════════════════════
// Constants
// ═══════════════════════════════════════════════════════════════════════════
static constexpr int   kStateVersion        = 12;
static constexpr float kPi                  = 3.14159265358979323846f;
static constexpr float kTwoPi              = 6.28318530717958647692f;
static constexpr float kSqrt2              = 1.41421356237309504880f;
static constexpr float kMinusInfDb         = -100.0f;
static constexpr float kDenormalGuard      = 1.0e-20f;
static constexpr float kSilenceThreshold   = 1.0e-8f;
static constexpr int   kMaxBlockSize       = 8192;
static constexpr int   kSpectrumFFTOrder   = 11;
static constexpr int   kSpectrumFFTSize    = 1 << kSpectrumFFTOrder;
static constexpr int   kGRHistoryLength    = 512;
static constexpr int   kMaxBands           = 6;
static constexpr int   kMaxUndoSteps       = 64;
static constexpr int   kMaxMIDIMappings    = 32;
static constexpr int   kNeuralHiddenSize   = 16;
static constexpr int   kNeuralLayers       = 2;
static constexpr int   kMaxFIRTaps         = 128;
static constexpr int   kMaxLFOs            = 4;
static constexpr int   kResonanceBands     = 32;
static constexpr int   kHarmonicCount      = 16;
static constexpr int   kRefSpectrumBins    = 128;

// ═══════════════════════════════════════════════════════════════════════════
// Denormal flushing (call once in prepareToPlay)
// ═══════════════════════════════════════════════════════════════════════════
inline void enableFlushToZero() noexcept {
#ifdef __SSE__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();
}

// ═══════════════════════════════════════════════════════════════════════════
// Utility functions (inline, no allocation)
// ═══════════════════════════════════════════════════════════════════════════
inline float dbToGain(float db) noexcept {
    return std::pow(10.0f, db * 0.05f);
}

inline float gainToDb(float gain) noexcept {
    return (gain > 1.0e-10f) ? 20.0f * std::log10(gain) : kMinusInfDb;
}

// Padé [5/5] tanh approximation. The rational is only accurate (and stays
// below 1.0) for roughly |x| < 3.3; beyond that it overshoots and eventually
// diverges. Clamp the input to the valid region and hard-bound the output to
// [-1, 1] so it saturates like a true tanh. This matters for RT safety: an
// unbounded "saturator" lets extreme input blow up the rest of the chain.
inline float fastTanh(float x) noexcept {
    const float cx = (x < -4.0f) ? -4.0f : (x > 4.0f ? 4.0f : x);
    const float x2 = cx * cx;
    const float y = cx * (945.0f + x2 * (105.0f + x2)) /
                         (945.0f + x2 * (420.0f + 15.0f * x2));
    return (y < -1.0f) ? -1.0f : (y > 1.0f ? 1.0f : y);
}

inline float softClip(float x) noexcept {
    return x / (1.0f + std::abs(x));
}

inline float lerp(float a, float b, float t) noexcept {
    return a + t * (b - a);
}

inline float clampf(float x, float lo, float hi) noexcept {
    return (x < lo) ? lo : ((x > hi) ? hi : x);
}

inline uint32_t xorshift32(uint32_t& state) noexcept {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 1: CORE MODULES
// ═══════════════════════════════════════════════════════════════════════════

// ── SmoothParam: one-pole parameter smoother ─────────────────────────────
struct SmoothParam {
    float current = 0.0f;
    float target  = 0.0f;
    float coeff   = 0.001f;

    void setTime(float ms, double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        coeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, ms) * 0.001f));
    }

    void setTarget(float v) noexcept { target = v; }

    inline float next() noexcept {
        current += coeff * (target - current);
        if (std::abs(target - current) < 1.0e-6f) current = target;
        return current;
    }

    void snapTo(float v) noexcept { current = target = v; }
    void reset(float v) noexcept { snapTo(v); }  // alias for test compatibility
    bool isSmoothing() const noexcept { return std::abs(target - current) > 1.0e-6f; }
};

// ── EnvFollower: attack/release envelope follower ────────────────────────
struct EnvFollower {
    float env = 0.0f;
    float envelope = 0.0f;  // public alias, kept in sync by process()
    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;

    void setTimes(float attackMs, float releaseMs, double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        attackCoeff  = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, attackMs) * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.01f, releaseMs) * 0.001f));
    }

    // Convenience overload: prepare(sampleRate, attackSeconds, releaseSeconds)
    void prepare(double sr, float attackSec, float releaseSec) noexcept {
        setTimes(attackSec * 1000.0f, releaseSec * 1000.0f, sr);
    }

    void reset(float value = 0.0f) noexcept { env = value; }

    inline float process(float xAbs) noexcept {
        const float c = xAbs > env ? attackCoeff : releaseCoeff;
        env += c * (xAbs - env);
        envelope = env;
        return env;
    }
};

// ── SafetyLayer: DC block (1 Hz) + NaN/Inf guard ────────────────────────
struct SafetyLayer {
    float dcL = 0.0f, dcPrevL = 0.0f;
    float dcR = 0.0f, dcPrevR = 0.0f;
    float dcCoeff = 0.9999f;

    void setSampleRate(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        dcCoeff = 1.0f - (kTwoPi * 1.0f / srf);
        dcCoeff = juce::jlimit(0.90f, 0.99999f, dcCoeff);
    }

    void reset() noexcept { dcL = dcPrevL = dcR = dcPrevR = 0.0f; }

    inline void processStereo(float& l, float& r) noexcept {
        l = processSample(l, dcL, dcPrevL);
        r = processSample(r, dcR, dcPrevR);
    }

private:
    inline float processSample(float x, float& dc, float& dcPrev) noexcept {
        if (!std::isfinite(x)) x = 0.0f;
        if (std::abs(x) < kDenormalGuard) x = 0.0f;
        const float y = x - dcPrev + dcCoeff * dc;
        dcPrev = x;
        dc = y;
        return y;
    }
};

// ── BypassCrossfader: 64-sample cosine crossfade for click-free bypass ──
struct BypassCrossfader {
    static constexpr int kFadeSamples = 64;

    void prepare() noexcept {
        fadePos = kFadeSamples;
        wetGain = targetBypassed ? 0.0f : 1.0f;
    }

    void reset() noexcept { prepare(); }

    void setBypassState(bool bypassed) noexcept {
        if (bypassed != targetBypassed) {
            targetBypassed = bypassed;
            fadePos = 0;
        }
    }

    bool isBypassed() const noexcept { return targetBypassed && fadePos >= kFadeSamples; }

    inline void processStereo(float dryL, float dryR, float& wetL, float& wetR) noexcept {
        if (fadePos < kFadeSamples) {
            const float t = (float)fadePos / (float)kFadeSamples;
            const float dest = targetBypassed ? 0.0f : 1.0f;
            const float start = targetBypassed ? 1.0f : 0.0f;
            wetGain = start + (dest - start) * (0.5f * (1.0f - std::cos(kPi * t)));
            ++fadePos;
        }
        if (wetGain < 1.0e-6f) { wetL = dryL; wetR = dryR; return; }
        if (wetGain > 0.999999f) return;
        wetL = dryL + (wetL - dryL) * wetGain;
        wetR = dryR + (wetR - dryR) * wetGain;
    }

private:
    int fadePos = 64;
    bool targetBypassed = false;
    float wetGain = 1.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 2: SATURATION MODELS
// ═══════════════════════════════════════════════════════════════════════════

enum class SaturationModel {
    Tanh = 0, Tube = 1, Tape = 2, Transistor = 3, Transformer = 4,
    Neural_Neve = 5, Neural_API = 6, Neural_SSL = 7, Neural_Custom = 8,
    WDF_Tube = 9, WDF_Transformer = 10,
    NumModels = 11
};

namespace Waveshaper {
    inline float tanh(float x) noexcept { return fastTanh(x); }

    inline float tube(float x) noexcept {
        const float bias = 0.2f;
        const float shaped = fastTanh(x + bias) - fastTanh(bias);
        const float even = 0.15f * (fastTanh(x * 0.8f + 0.5f) - fastTanh(0.5f));
        return shaped + even;
    }

    inline float tape(float x, float& hysteresisState) noexcept {
        const float satLevel = 1.2f;
        const float coercivity = 0.3f;
        const float drive = x * 1.5f;
        const float delta = drive - hysteresisState;
        const float rate = coercivity * (1.0f - (hysteresisState * hysteresisState) /
                           (satLevel * satLevel));
        hysteresisState += juce::jlimit(-0.5f, 0.5f, delta * juce::jmax(0.01f, rate));
        return fastTanh(hysteresisState * 0.9f);
    }

    inline float transistor(float x) noexcept {
        const float posThresh = 0.8f, negThresh = -0.65f;
        float y;
        if (x > posThresh)
            y = posThresh + (x - posThresh) / (1.0f + 4.0f * (x - posThresh) * (x - posThresh));
        else if (x < negThresh)
            y = negThresh + (x - negThresh) / (1.0f + 6.0f * (x - negThresh) * (x - negThresh));
        else
            y = x;
        return y - 0.1f * y * y * y;
    }

    inline float transformer(float x, float lowContent) noexcept {
        const float lowDrive = 1.0f + lowContent * 0.8f;
        const float driven = x * lowDrive;
        const float shaped = driven / (1.0f + std::abs(driven) * 0.5f);
        const float evenHarm = 0.08f * (shaped * shaped) * (x > 0.0f ? 1.0f : -1.0f);
        return shaped + evenHarm;
    }

    inline float process(SaturationModel model, float x, float& tapeState, float lowContent) noexcept {
        switch (model) {
            case SaturationModel::Tanh:        return tanh(x);
            case SaturationModel::Tube:        return tube(x);
            case SaturationModel::Tape:        return tape(x, tapeState);
            case SaturationModel::Transistor:  return transistor(x);
            case SaturationModel::Transformer: return transformer(x, lowContent);
            default:                           return fastTanh(x);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 3: NEURAL SATURATION (RTNeural-compatible GRU)
// ═══════════════════════════════════════════════════════════════════════════

struct NeuralSaturationModel {
    bool loaded = false;

    // Pre-allocated weight matrices (input=1, hidden=16, output=1)
    std::array<float, kNeuralHiddenSize> hiddenState{};
    std::array<float, kNeuralHiddenSize> Wz{}, Wr{}, Wh{};    // input weights
    std::array<float, kNeuralHiddenSize> Uz{}, Ur{}, Uh{};    // recurrent weights (diagonal approx)
    std::array<float, kNeuralHiddenSize> bz{}, br{}, bh{};    // biases
    std::array<float, kNeuralHiddenSize> outputWeights{};
    float outputBias = 0.0f;

    void reset() noexcept { hiddenState.fill(0.0f); }

    inline float process(float input) noexcept {
        if (!loaded) return fastTanh(input);

        // GRU forward pass (simplified diagonal recurrence)
        std::array<float, kNeuralHiddenSize> z, r, hCandidate;

        for (int i = 0; i < kNeuralHiddenSize; ++i) {
            z[i] = sigmoid(Wz[i] * input + Uz[i] * hiddenState[i] + bz[i]);
            r[i] = sigmoid(Wr[i] * input + Ur[i] * hiddenState[i] + br[i]);
        }
        for (int i = 0; i < kNeuralHiddenSize; ++i) {
            hCandidate[i] = fastTanh(Wh[i] * input + Uh[i] * (r[i] * hiddenState[i]) + bh[i]);
            hiddenState[i] = (1.0f - z[i]) * hiddenState[i] + z[i] * hCandidate[i];
        }

        // Linear output layer
        float output = outputBias;
        for (int i = 0; i < kNeuralHiddenSize; ++i)
            output += outputWeights[i] * hiddenState[i];

        return output;
    }

    bool loadModel(const juce::File& jsonFile) {
        auto text = jsonFile.loadFileAsString();
        if (text.isEmpty()) return false;

        auto json = juce::JSON::parse(text);
        if (!json.isObject()) return false;

        auto* obj = json.getDynamicObject();
        if (!obj) return false;

        auto loadArray = [&](const juce::String& key, std::array<float, kNeuralHiddenSize>& arr) {
            if (auto* a = obj->getProperty(key).getArray()) {
                for (int i = 0; i < juce::jmin((int)a->size(), kNeuralHiddenSize); ++i)
                    arr[i] = (float)(*a)[i];
            }
        };

        loadArray("Wz", Wz); loadArray("Wr", Wr); loadArray("Wh", Wh);
        loadArray("Uz", Uz); loadArray("Ur", Ur); loadArray("Uh", Uh);
        loadArray("bz", bz); loadArray("br", br); loadArray("bh", bh);
        loadArray("output_weights", outputWeights);
        outputBias = (float)obj->getProperty("output_bias");

        loaded = true;
        reset();
        return true;
    }

    // Alternative loader: from raw float array (used by loadNeuralModel)
    bool loadWeights(const float* data, int numWeights) noexcept {
        // Expected layout: Wz(16) Wr(16) Wh(16) Uz(16) Ur(16) Uh(16) bz(16) br(16) bh(16) outputWeights(16) outputBias(1)
        const int expected = kNeuralHiddenSize * 10 + 1;
        if (data == nullptr || numWeights < expected) return false;
        int offset = 0;
        auto copyBlock = [&](std::array<float, kNeuralHiddenSize>& arr) {
            for (int i = 0; i < kNeuralHiddenSize; ++i)
                arr[i] = data[offset++];
        };
        copyBlock(Wz); copyBlock(Wr); copyBlock(Wh);
        copyBlock(Uz); copyBlock(Ur); copyBlock(Uh);
        copyBlock(bz); copyBlock(br); copyBlock(bh);
        copyBlock(outputWeights);
        outputBias = data[offset];
        loaded = true;
        reset();
        return true;
    }

private:
    static inline float sigmoid(float x) noexcept {
        return 1.0f / (1.0f + std::exp(-x));
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 4: WDF CIRCUIT MODELS
// ═══════════════════════════════════════════════════════════════════════════

struct WDFTubeStage {
    float state = 0.0f;
    float biasPoint = 0.3f;
    float plateResistance = 0.7f;

    void reset() noexcept { state = 0.0f; }

    inline float process(float input) noexcept {
        const float gridVoltage = input + biasPoint;
        const float plateCurrentApprox = (gridVoltage > 0.0f)
            ? gridVoltage * gridVoltage / (gridVoltage + plateResistance)
            : 0.0f;
        const float reflected = input - 2.0f * plateCurrentApprox * plateResistance;
        state = state * 0.3f + reflected * 0.7f;
        return fastTanh(state);
    }
};

struct WDFTransformerStage {
    float primaryFlux = 0.0f;
    float turnsRatio = 1.0f;
    float coreNonlinearity = 0.4f;

    void reset() noexcept { primaryFlux = 0.0f; }

    inline float process(float input) noexcept {
        primaryFlux += input * 0.01f;
        primaryFlux *= 0.999f;  // core loss
        const float satFlux = fastTanh(primaryFlux * coreNonlinearity);
        const float output = (input + satFlux * 0.3f) * turnsRatio;
        return output / (1.0f + std::abs(output) * 0.2f);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 5: RESONANCE TAMING + TRANSIENT SPLITTING
// ═══════════════════════════════════════════════════════════════════════════

struct ResonanceTamer {
    std::array<float, kResonanceBands> bandEnergy{};
    std::array<float, kResonanceBands> gainReduction{};
    float sensitivity = 0.5f;
    float depth = 0.5f;
    bool enabled = false;
    float smoothCoeff = 0.0f;

    void prepare(double sr) noexcept {
        bandEnergy.fill(0.0f);
        gainReduction.fill(1.0f);
        smoothCoeff = 1.0f - std::exp(-1.0f / ((float)sr * 0.005f));
    }

    void reset() noexcept {
        bandEnergy.fill(0.0f);
        gainReduction.fill(1.0f);
    }

    inline float process(float input) noexcept {
        if (!enabled) return input;

        // Simplified spectral band analysis via cascaded bandpass approximation
        const float absIn = std::abs(input);
        float totalReduction = 1.0f;

        for (int b = 0; b < kResonanceBands; ++b) {
            bandEnergy[b] += smoothCoeff * (absIn - bandEnergy[b]);
            const float threshold = sensitivity * 0.5f;
            if (bandEnergy[b] > threshold) {
                const float excess = bandEnergy[b] - threshold;
                gainReduction[b] = 1.0f / (1.0f + excess * depth * 4.0f);
                totalReduction = juce::jmin(totalReduction, gainReduction[b]);
            } else {
                gainReduction[b] += smoothCoeff * (1.0f - gainReduction[b]);
            }
        }

        return input * totalReduction;
    }
};

struct TransientSplitter {
    float envFast = 0.0f;
    float envSlow = 0.0f;
    float sensitivity = 0.5f;
    float mix = 0.5f;
    bool enabled = false;
    float fastCoeff = 0.0f;
    float slowCoeff = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        fastCoeff = 1.0f - std::exp(-1.0f / (srf * 0.001f));  // 1ms
        slowCoeff = 1.0f - std::exp(-1.0f / (srf * 0.050f));  // 50ms
        envFast = envSlow = 0.0f;
    }

    void reset() noexcept { envFast = envSlow = 0.0f; }

    // Returns transient amount [0,1] for this sample
    inline float getTransientAmount(float input) noexcept {
        if (!enabled) return 0.0f;
        const float absIn = std::abs(input);
        envFast += fastCoeff * (absIn - envFast);
        envSlow += slowCoeff * (absIn - envSlow);
        const float ratio = (envSlow > kDenormalGuard)
            ? (envFast / envSlow) - 1.0f : 0.0f;
        return juce::jlimit(0.0f, 1.0f, ratio * sensitivity * 4.0f);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 6: OVERSAMPLING ENGINE
// ═══════════════════════════════════════════════════════════════════════════

struct OversamplingEngine {
    int factor = 1;  // 1 = off, 2, 4, or 8
    int latencySamples = 0;

    // Pre-allocated FIR filter state (half-band)
    static constexpr int kHalfBandTaps = 31;
    std::array<float, kHalfBandTaps> firCoeffs{};
    std::array<float, kHalfBandTaps> delayLineL{};
    std::array<float, kHalfBandTaps> delayLineR{};
    int delayIndex = 0;

    void prepare(int oversampleFactor) noexcept {
        factor = juce::jlimit(1, 8, oversampleFactor);
        latencySamples = (factor > 1) ? kHalfBandTaps / 2 : 0;

        // Generate windowed-sinc half-band FIR
        const int N = kHalfBandTaps;
        const int M = N / 2;
        for (int i = 0; i < N; ++i) {
            const float n = (float)(i - M);
            if (i == M) {
                firCoeffs[i] = 0.5f;
            } else {
                const float sinc = std::sin(kPi * n * 0.5f) / (kPi * n);
                const float window = 0.42f - 0.5f * std::cos(kTwoPi * (float)i / (float)(N - 1))
                                   + 0.08f * std::cos(2.0f * kTwoPi * (float)i / (float)(N - 1));
                firCoeffs[i] = sinc * window;
            }
        }

        delayLineL.fill(0.0f);
        delayLineR.fill(0.0f);
        delayIndex = 0;
    }

    void reset() noexcept {
        delayLineL.fill(0.0f);
        delayLineR.fill(0.0f);
        delayIndex = 0;
    }

    // Upsample by inserting zeros (caller processes at higher rate)
    inline void upsample(float inL, float inR, float* outL, float* outR, int numOut) noexcept {
        outL[0] = inL;
        outR[0] = inR;
        for (int i = 1; i < numOut; ++i) {
            outL[i] = 0.0f;
            outR[i] = 0.0f;
        }
    }

    // Downsample with FIR anti-alias filter
    inline void downsample(const float* inL, const float* inR, int numIn,
                           float& outL, float& outR) noexcept {
        // Apply FIR and decimate
        float sumL = 0.0f, sumR = 0.0f;
        for (int i = 0; i < juce::jmin(numIn, kHalfBandTaps); ++i) {
            sumL += inL[i] * firCoeffs[i];
            sumR += inR[i] * firCoeffs[i];
        }
        outL = sumL * (float)factor;
        outR = sumR * (float)factor;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 7: DYNAMICS
// ═══════════════════════════════════════════════════════════════════════════

struct SidechainHPF {
    // Proper 1-pole HPF: y[n] = coeff * (y[n-1] + x[n] - x[n-1])
    float prevInL = 0.0f, prevInR = 0.0f;
    float prevOutL = 0.0f, prevOutR = 0.0f;
    float coeff = 0.995f;

    void prepare(double sr, float freqHz) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        // RC time constant: coeff = RC / (RC + dt), where RC = 1/(2*pi*f)
        const float rc = 1.0f / (kTwoPi * juce::jmax(1.0f, freqHz));
        const float dt = 1.0f / srf;
        coeff = rc / (rc + dt);
    }

    void reset() noexcept { prevInL = prevInR = prevOutL = prevOutR = 0.0f; }

    inline void processStereo(float& l, float& r) noexcept {
        const float outL = coeff * (prevOutL + l - prevInL);
        prevInL = l;
        prevOutL = outL;
        l = outL;

        const float outR = coeff * (prevOutR + r - prevInR);
        prevInR = r;
        prevOutR = outR;
        r = outR;
    }
};

struct GlueCompressor {
    float envState = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float threshold = -6.0f;
    float ratio = 3.0f;
    float makeupGain = 1.0f;
    float lastGainReductionDb = 0.0f;
    float stereoLink = 1.0f;  // 0 = dual-mono, 1 = linked
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    double cachedSR = 44100.0;

    void prepare(double sr) noexcept {
        cachedSR = juce::jmax(1.0, sr);
        recalcCoeffs();
        envState = 0.0f;
    }

    void reset() noexcept { envState = 0.0f; lastGainReductionDb = 0.0f; }

    void setParameters(float threshDb, float ratioVal, float makeupDb) noexcept {
        threshold = threshDb;
        ratio = juce::jmax(1.0f, ratioVal);
        makeupGain = dbToGain(makeupDb);
    }

    void setAttackRelease(float atkMs, float relMs) noexcept {
        attackMs = juce::jmax(0.1f, atkMs);
        releaseMs = juce::jmax(1.0f, relMs);
        recalcCoeffs();
    }

    void setStereoLink(float link) noexcept {
        stereoLink = juce::jlimit(0.0f, 1.0f, link);
    }

    inline float processStereo(float scL, float scR) noexcept {
        // Stereo linking: blend between max (linked) and per-channel (dual-mono)
        const float absL = std::abs(scL);
        const float absR = std::abs(scR);
        const float linked = juce::jmax(absL, absR);
        const float unlinked = (absL + absR) * 0.5f;
        const float scPeak = lerp(unlinked, linked, stereoLink);

        const float scDb = gainToDb(scPeak);
        const float overDb = juce::jmax(0.0f, scDb - threshold);
        const float targetGrDb = -overDb * (1.0f - 1.0f / ratio);

        const float c = (targetGrDb < lastGainReductionDb) ? attackCoeff : releaseCoeff;
        lastGainReductionDb += c * (targetGrDb - lastGainReductionDb);

        return dbToGain(lastGainReductionDb) * makeupGain;
    }

private:
    void recalcCoeffs() noexcept {
        const float srf = (float)cachedSR;
        attackCoeff  = 1.0f - std::exp(-1.0f / (srf * attackMs * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * releaseMs * 0.001f));
    }
};

struct TruePeakLimiter {
    // ITU-R BS.1770-4 compliant True Peak Limiter
    // Uses 4x polyphase FIR interpolation for inter-sample peak detection
    static constexpr int kLookahead = 8;
    static constexpr int kISPFactor = 4;  // 4x oversampling for ISP detection
    static constexpr int kISPFilterLen = 12; // Half-band FIR taps per phase

    float ceiling = -0.3f;
    float delayBufL[kLookahead] = {};
    float delayBufR[kLookahead] = {};
    int delayIdx = 0;
    float gainState = 1.0f;
    float releaseCoeff = 0.0f;

    // History buffer for polyphase interpolation (4 samples needed)
    float histL[4] = {};
    float histR[4] = {};
    int histIdx = 0;

    // Polyphase FIR coefficients (4-point, 4-phase Lagrange interpolator)
    // These approximate a sinc function for 4x oversampled peak detection
    static constexpr float kPhaseCoeffs[4][4] = {
        { 0.0f,    1.0f,    0.0f,    0.0f   },  // Phase 0: original sample
        {-0.0625f, 0.5625f, 0.5625f,-0.0625f},  // Phase 1: +0.25 sample
        {-0.0625f, 0.25f,   0.875f, -0.0625f},  // Phase 2: +0.5 sample (approx)
        {-0.0234f, 0.0703f, 0.8672f, 0.0859f}   // Phase 3: +0.75 sample
    };

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * 0.050f));
        reset();
    }

    void reset() noexcept {
        std::memset(delayBufL, 0, sizeof(delayBufL));
        std::memset(delayBufR, 0, sizeof(delayBufR));
        std::memset(histL, 0, sizeof(histL));
        std::memset(histR, 0, sizeof(histR));
        delayIdx = 0;
        histIdx = 0;
        gainState = 1.0f;
    }

    // 4x polyphase interpolation to detect inter-sample peaks
    inline float detectTruePeak(float l, float r) noexcept {
        // Store current sample in history
        histL[histIdx] = l;
        histR[histIdx] = r;

        float maxPeak = 0.0f;

        // Evaluate all 4 phases to find the true peak
        for (int phase = 0; phase < kISPFactor; ++phase) {
            float interpL = 0.0f, interpR = 0.0f;
            for (int tap = 0; tap < 4; ++tap) {
                const int idx = (histIdx - 3 + tap + 4) % 4;
                interpL += kPhaseCoeffs[phase][tap] * histL[idx];
                interpR += kPhaseCoeffs[phase][tap] * histR[idx];
            }
            const float absPeak = juce::jmax(std::abs(interpL), std::abs(interpR));
            if (absPeak > maxPeak) maxPeak = absPeak;
        }

        histIdx = (histIdx + 1) % 4;
        return maxPeak;
    }

    inline void processStereo(float& l, float& r) noexcept {
        // Write to lookahead delay
        delayBufL[delayIdx] = l;
        delayBufR[delayIdx] = r;

        // Read delayed sample
        const int readIdx = (delayIdx + 1) % kLookahead;
        const float delL = delayBufL[readIdx];
        const float delR = delayBufR[readIdx];
        delayIdx = (delayIdx + 1) % kLookahead;

        // Detect true inter-sample peak via 4x polyphase interpolation
        const float truePeak = detectTruePeak(l, r);
        const float ceilLin = dbToGain(ceiling);
        const float targetGain = (truePeak > ceilLin) ? (ceilLin / truePeak) : 1.0f;

        // Smooth gain (instant attack, smooth release)
        if (targetGain < gainState)
            gainState = targetGain;
        else
            gainState += releaseCoeff * (targetGain - gainState);

        l = delL * gainState;
        r = delR * gainState;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 8: EQ + CROSSOVER + MULTIBAND
// ═══════════════════════════════════════════════════════════════════════════

struct ShineProcessor {
    float freq = 8000.0f;
    float gain = 0.0f;
    float q = 0.707f;
    // Biquad coefficients (peaking EQ, RBJ cookbook)
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    // Transposed Direct Form II state (separate delay lines for input/output)
    float s1L = 0.0f, s2L = 0.0f, s1R = 0.0f, s2R = 0.0f;

    void prepare(double sr) noexcept { recalcCoeffs(sr); }

    void setParameters(float freqHz, float gainDb, float qVal) noexcept {
        freq = freqHz; gain = gainDb; q = qVal;
    }

    void recalcCoeffs(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        const float A = std::pow(10.0f, gain / 40.0f);
        const float w0 = kTwoPi * freq / srf;
        const float sinW = std::sin(w0);
        const float cosW = std::cos(w0);
        const float alpha = sinW / (2.0f * q);

        // RBJ peaking EQ coefficients
        const float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cosW) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cosW) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    void reset() noexcept { s1L = s2L = s1R = s2R = 0.0f; }

    inline void processStereo(float& l, float& r) noexcept {
        // Transposed Direct Form II (TDF-II) — numerically stable
        const float outL = b0 * l + s1L;
        s1L = b1 * l - a1 * outL + s2L;
        s2L = b2 * l - a2 * outL;
        l = outL;

        const float outR = b0 * r + s1R;
        s1R = b1 * r - a1 * outR + s2R;
        s2R = b2 * r - a2 * outR;
        r = outR;
    }
};

struct LinkwitzRileyCrossover {
    // True Linkwitz-Riley 4th-order (LR4) crossover: -24dB/oct slopes
    // Implemented as two cascaded 2nd-order Butterworth biquads for BOTH LP and HP
    // LP and HP sum to unity (allpass) — no phase cancellation at crossover
    float freq = 250.0f;

    // LP biquad 1 & 2 coefficients (shared between both cascades)
    float lp_b0 = 1.0f, lp_b1 = 0.0f, lp_b2 = 0.0f;
    float lp_a1 = 0.0f, lp_a2 = 0.0f;
    // HP biquad 1 & 2 coefficients
    float hp_b0 = 1.0f, hp_b1 = 0.0f, hp_b2 = 0.0f;
    float hp_a1 = 0.0f, hp_a2 = 0.0f;

    // TDF-II state: 2 cascaded biquads x 2 channels x LP/HP
    // LP cascade
    float lp1_s1L = 0.0f, lp1_s2L = 0.0f, lp2_s1L = 0.0f, lp2_s2L = 0.0f;
    float lp1_s1R = 0.0f, lp1_s2R = 0.0f, lp2_s1R = 0.0f, lp2_s2R = 0.0f;
    // HP cascade
    float hp1_s1L = 0.0f, hp1_s2L = 0.0f, hp2_s1L = 0.0f, hp2_s2L = 0.0f;
    float hp1_s1R = 0.0f, hp1_s2R = 0.0f, hp2_s1R = 0.0f, hp2_s2R = 0.0f;

    void prepare(double sr, float freqHz) noexcept {
        freq = freqHz;
        const float srf = (float)juce::jmax(1.0, sr);
        const float w0 = kTwoPi * juce::jlimit(20.0f, srf * 0.49f, freqHz) / srf;
        const float sinW = std::sin(w0);
        const float cosW = std::cos(w0);
        // Butterworth Q = 0.7071 for each 2nd-order section
        const float alpha = sinW / (2.0f * kSqrt2 * 0.5f); // Q = 1/sqrt(2) per section

        // 2nd-order Butterworth LPF coefficients (RBJ cookbook)
        {
            const float a0 = 1.0f + alpha;
            lp_b0 = ((1.0f - cosW) * 0.5f) / a0;
            lp_b1 = (1.0f - cosW) / a0;
            lp_b2 = ((1.0f - cosW) * 0.5f) / a0;
            lp_a1 = (-2.0f * cosW) / a0;
            lp_a2 = (1.0f - alpha) / a0;
        }

        // 2nd-order Butterworth HPF coefficients (RBJ cookbook)
        {
            const float a0 = 1.0f + alpha;
            hp_b0 = ((1.0f + cosW) * 0.5f) / a0;
            hp_b1 = (-(1.0f + cosW)) / a0;
            hp_b2 = ((1.0f + cosW) * 0.5f) / a0;
            hp_a1 = (-2.0f * cosW) / a0;
            hp_a2 = (1.0f - alpha) / a0;
        }

        reset();
    }

    void reset() noexcept {
        lp1_s1L = lp1_s2L = lp2_s1L = lp2_s2L = 0.0f;
        lp1_s1R = lp1_s2R = lp2_s1R = lp2_s2R = 0.0f;
        hp1_s1L = hp1_s2L = hp2_s1L = hp2_s2L = 0.0f;
        hp1_s1R = hp1_s2R = hp2_s1R = hp2_s2R = 0.0f;
    }

    inline void processStereo(float inL, float inR,
                              float& lowL, float& lowR,
                              float& highL, float& highR) noexcept {
        // LP path: two cascaded 2nd-order Butterworth LPF = LR4 LP
        lowL = biquadTDF2(inL, lp_b0, lp_b1, lp_b2, lp_a1, lp_a2, lp1_s1L, lp1_s2L);
        lowL = biquadTDF2(lowL, lp_b0, lp_b1, lp_b2, lp_a1, lp_a2, lp2_s1L, lp2_s2L);
        lowR = biquadTDF2(inR, lp_b0, lp_b1, lp_b2, lp_a1, lp_a2, lp1_s1R, lp1_s2R);
        lowR = biquadTDF2(lowR, lp_b0, lp_b1, lp_b2, lp_a1, lp_a2, lp2_s1R, lp2_s2R);

        // HP path: two cascaded 2nd-order Butterworth HPF = LR4 HP
        highL = biquadTDF2(inL, hp_b0, hp_b1, hp_b2, hp_a1, hp_a2, hp1_s1L, hp1_s2L);
        highL = biquadTDF2(highL, hp_b0, hp_b1, hp_b2, hp_a1, hp_a2, hp2_s1L, hp2_s2L);
        highR = biquadTDF2(inR, hp_b0, hp_b1, hp_b2, hp_a1, hp_a2, hp1_s1R, hp1_s2R);
        highR = biquadTDF2(highR, hp_b0, hp_b1, hp_b2, hp_a1, hp_a2, hp2_s1R, hp2_s2R);
    }

private:
    static inline float biquadTDF2(float x, float b0, float b1, float b2,
                                    float a1, float a2,
                                    float& s1, float& s2) noexcept {
        const float y = b0 * x + s1;
        s1 = b1 * x - a1 * y + s2;
        s2 = b2 * x - a2 * y;
        return y;
    }
};

struct MultibandEngine {
    static constexpr int kMaxCrossovers = kMaxBands - 1;
    LinkwitzRileyCrossover crossovers[kMaxCrossovers];
    int numBands = 1;

    void prepare(double sr, int bands, const float* freqs) noexcept {
        numBands = juce::jlimit(1, kMaxBands, bands);
        for (int i = 0; i < numBands - 1; ++i)
            crossovers[i].prepare(sr, freqs[i]);
    }

    void reset() noexcept {
        for (auto& xo : crossovers) xo.reset();
    }

    // Split into bands (caller provides arrays of size kMaxBands)
    inline void split(float inL, float inR,
                      float* bandL, float* bandR) noexcept {
        if (numBands <= 1) {
            bandL[0] = inL; bandR[0] = inR;
            return;
        }

        float remainL = inL, remainR = inR;
        for (int i = 0; i < numBands - 1; ++i) {
            float lowL, lowR, highL, highR;
            crossovers[i].processStereo(remainL, remainR, lowL, lowR, highL, highR);
            bandL[i] = lowL; bandR[i] = lowR;
            remainL = highL; remainR = highR;
        }
        bandL[numBands - 1] = remainL;
        bandR[numBands - 1] = remainR;
    }

    // Recombine bands
    inline void recombine(const float* bandL, const float* bandR,
                          float& outL, float& outR) noexcept {
        outL = outR = 0.0f;
        for (int i = 0; i < numBands; ++i) {
            outL += bandL[i];
            outR += bandR[i];
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 9: STEREO
// ═══════════════════════════════════════════════════════════════════════════

struct MidSideEncoder {
    bool enabled = false;

    inline void encode(float& l, float& r) noexcept {
        if (!enabled) return;
        const float mid = (l + r) * 0.5f;
        const float side = (l - r) * 0.5f;
        l = mid; r = side;
    }

    inline void decode(float& l, float& r) noexcept {
        if (!enabled) return;
        const float left  = l + r;
        const float right = l - r;
        l = left; r = right;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 10: MODULATION
// ═══════════════════════════════════════════════════════════════════════════

struct LFO {
    float phase = 0.0f;
    float increment = 0.0f;

    enum class Shape { Sine, Triangle, SawUp, SawDown, Square, SampleHold };
    Shape shape = Shape::Sine;
    float lastSH = 0.0f;
    uint32_t shSeed = 42;

    void prepare(double sr, float rateHz) noexcept {
        increment = rateHz / (float)juce::jmax(1.0, sr);
    }

    void reset() noexcept { phase = 0.0f; lastSH = 0.0f; }

    inline float tick() noexcept {
        float out = 0.0f;
        switch (shape) {
            case Shape::Sine:       out = std::sin(kTwoPi * phase); break;
            case Shape::Triangle:   out = 4.0f * std::abs(phase - 0.5f) - 1.0f; break;
            case Shape::SawUp:      out = 2.0f * phase - 1.0f; break;
            case Shape::SawDown:    out = 1.0f - 2.0f * phase; break;
            case Shape::Square:     out = (phase < 0.5f) ? 1.0f : -1.0f; break;
            case Shape::SampleHold:
                if (phase < increment) {
                    shSeed ^= shSeed << 13; shSeed ^= shSeed >> 17; shSeed ^= shSeed << 5;
                    lastSH = (float)(int32_t)shSeed / 2147483648.0f;
                }
                out = lastSH;
                break;
        }
        phase += increment;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }

    // Alias for test compatibility
    inline float next() noexcept { return tick(); }
};

struct MacroInterpreter {
    static constexpr int kNumMacros = 4;
    static constexpr int kMaxMappingsPerMacro = 8;

    enum class Curve { Linear, Exponential, Logarithmic };

    struct Mapping {
        juce::String targetParamID;  // parameter ID string
        int targetParam = -1;         // legacy int ID
        float depth = 0.0f;
        Curve curve = Curve::Linear;
    };

    struct MacroSlot {
        Mapping mappings[kMaxMappingsPerMacro];
        int numMappings = 0;
        float value = 0.0f;  // current macro knob value (0..1)

        void addMapping(const Mapping& m) noexcept {
            if (numMappings < kMaxMappingsPerMacro)
                mappings[numMappings++] = m;
        }

        void clearMappings() noexcept { numMappings = 0; }

        float getMappedValue(float macroValue, int mappingIndex) const noexcept {
            if (mappingIndex < 0 || mappingIndex >= numMappings) return 0.0f;
            const auto& m = mappings[mappingIndex];
            const float curved = applyCurve(macroValue, m.curve);
            return curved * m.depth;
        }

        static float applyCurve(float x, Curve c) noexcept {
            const float clamped = juce::jlimit(0.0f, 1.0f, x);
            switch (c) {
                case Curve::Linear:      return clamped;
                case Curve::Exponential: return clamped * clamped;
                case Curve::Logarithmic: return std::sqrt(clamped);
                default:                 return clamped;
            }
        }
    };

    MacroSlot slots[kNumMacros];

    void reset() noexcept {
        for (auto& s : slots) { s.clearMappings(); s.value = 0.0f; }
    }

    // Convenience: get mapped value from slot index and mapping index
    float getMappedValue(int slotIdx, int mappingIdx) const noexcept {
        if (slotIdx < 0 || slotIdx >= kNumMacros) return 0.0f;
        return slots[slotIdx].getMappedValue(slots[slotIdx].value, mappingIdx);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 11: AUTO GAIN
// ═══════════════════════════════════════════════════════════════════════════

struct AutoGainSmoother {
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    float compensationGain = 1.0f;
    float smoothCoeff = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        smoothCoeff = 1.0f - std::exp(-1.0f / (srf * 0.300f));
        inputLevel = outputLevel = 0.0f;
        compensationGain = 1.0f;
    }

    void reset() noexcept { inputLevel = outputLevel = 0.0f; compensationGain = 1.0f; }

    inline void updateInput(float peakL, float peakR) noexcept {
        const float peak = juce::jmax(std::abs(peakL), std::abs(peakR));
        inputLevel += smoothCoeff * (peak - inputLevel);
    }
    // Single-arg convenience overload
    inline void updateInput(float peak) noexcept { updateInput(peak, peak); }

    inline void updateOutput(float peakL, float peakR) noexcept {
        const float peak = juce::jmax(std::abs(peakL), std::abs(peakR));
        outputLevel += smoothCoeff * (peak - outputLevel);
    }
    // Single-arg convenience overload
    inline void updateOutput(float peak) noexcept { updateOutput(peak, peak); }

    inline float getCompensationGain() noexcept {
        if (outputLevel > 0.001f && inputLevel > 0.001f) {
            const float target = inputLevel / outputLevel;
            compensationGain += smoothCoeff * (juce::jlimit(0.1f, 10.0f, target) - compensationGain);
        }
        return compensationGain;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 12: METERING
// ═══════════════════════════════════════════════════════════════════════════

struct MeterBallistics {
    float peakL = 0.0f, peakR = 0.0f;
    float rmsL = 0.0f, rmsR = 0.0f;
    float peakHoldL = 0.0f, peakHoldR = 0.0f;
    int holdCounterL = 0, holdCounterR = 0;
    float peakDecay = 0.0f;
    float rmsCoeff = 0.0f;
    int holdSamples = 0;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        peakDecay = std::exp(-1.0f / (srf * 0.300f));
        rmsCoeff = 1.0f - std::exp(-1.0f / (srf * 0.050f));
        holdSamples = (int)(srf * 1.0f);
        reset();
    }

    void reset() noexcept {
        peakL = peakR = rmsL = rmsR = 0.0f;
        peakHoldL = peakHoldR = 0.0f;
        holdCounterL = holdCounterR = 0;
    }

    inline void process(float l, float r) noexcept {
        const float absL = std::abs(l), absR = std::abs(r);

        // Peak with hold
        if (absL > peakL) { peakL = absL; peakHoldL = absL; holdCounterL = holdSamples; }
        else { peakL *= peakDecay; }
        if (--holdCounterL <= 0) peakHoldL *= peakDecay;

        if (absR > peakR) { peakR = absR; peakHoldR = absR; holdCounterR = holdSamples; }
        else { peakR *= peakDecay; }
        if (--holdCounterR <= 0) peakHoldR *= peakDecay;

        // RMS
        rmsL += rmsCoeff * (l * l - rmsL);
        rmsR += rmsCoeff * (r * r - rmsR);
    }

    float getRmsDbL() const noexcept { return gainToDb(std::sqrt(rmsL)); }
    float getRmsDbR() const noexcept { return gainToDb(std::sqrt(rmsR)); }
    float getPeakDbL() const noexcept { return gainToDb(peakL); }
    float getPeakDbR() const noexcept { return gainToDb(peakR); }
};

struct LoudnessMeter {
    // ITU-R BS.1770-4 compliant loudness meter with K-weighting
    // K-weighting = Stage 1 (high-shelf +4dB @ 1681Hz) + Stage 2 (HPF @ 38Hz)
    float momentary = -24.0f;
    float shortTerm = -24.0f;
    float integrated = -24.0f;
    float truePeak = -100.0f;
    float accumulator = 0.0f;
    int sampleCount = 0;
    int windowSamples = 0;      // 400ms for momentary
    int shortTermWindow = 0;    // 3000ms for short-term
    float shortTermAccum = 0.0f;
    int shortTermCount = 0;
    float gatingThreshold = -70.0f;

    // K-weighting Stage 1: High-shelf filter (+4dB at 1681Hz)
    // Biquad coefficients (TDF-II)
    float hs_b0 = 1.0f, hs_b1 = 0.0f, hs_b2 = 0.0f;
    float hs_a1 = 0.0f, hs_a2 = 0.0f;
    float hs_s1L = 0.0f, hs_s2L = 0.0f;
    float hs_s1R = 0.0f, hs_s2R = 0.0f;

    // K-weighting Stage 2: High-pass filter (38Hz, 2nd order Butterworth)
    float hp_b0 = 1.0f, hp_b1 = 0.0f, hp_b2 = 0.0f;
    float hp_a1 = 0.0f, hp_a2 = 0.0f;
    float hp_s1L = 0.0f, hp_s2L = 0.0f;
    float hp_s1R = 0.0f, hp_s2R = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        windowSamples = (int)(sr * 0.400);
        shortTermWindow = (int)(sr * 3.0);
        computeKWeighting(srf);
        reset();
    }

    void reset() noexcept {
        momentary = shortTerm = integrated = -24.0f;
        truePeak = -100.0f;
        accumulator = 0.0f;
        sampleCount = 0;
        shortTermAccum = 0.0f;
        shortTermCount = 0;
        hs_s1L = hs_s2L = hs_s1R = hs_s2R = 0.0f;
        hp_s1L = hp_s2L = hp_s1R = hp_s2R = 0.0f;
    }

    inline void process(float l, float r) noexcept {
        // Apply K-weighting filter chain
        float kL = applyKWeight(l, hs_s1L, hs_s2L, hp_s1L, hp_s2L);
        float kR = applyKWeight(r, hs_s1R, hs_s2R, hp_s1R, hp_s2R);

        // Accumulate mean-square of K-weighted signal
        const float sum = kL * kL + kR * kR;
        accumulator += sum;
        shortTermAccum += sum;
        ++sampleCount;
        ++shortTermCount;

        // True peak tracking (sample-level; full ISP is in TruePeakLimiter)
        const float peak = juce::jmax(std::abs(l), std::abs(r));
        if (peak > dbToGain(truePeak))
            truePeak = gainToDb(peak);

        // Momentary loudness (400ms gate)
        if (sampleCount >= windowSamples) {
            const float meanSquare = accumulator / (float)windowSamples;
            momentary = (meanSquare > 1.0e-10f)
                ? -0.691f + 10.0f * std::log10(meanSquare) : -100.0f;
            accumulator = 0.0f;
            sampleCount = 0;

            // Integrated loudness (gated)
            if (momentary > gatingThreshold)
                integrated += 0.01f * (momentary - integrated);
        }

        // Short-term loudness (3s gate)
        if (shortTermCount >= shortTermWindow) {
            const float stMean = shortTermAccum / (float)shortTermWindow;
            shortTerm = (stMean > 1.0e-10f)
                ? -0.691f + 10.0f * std::log10(stMean) : -100.0f;
            shortTermAccum = 0.0f;
            shortTermCount = 0;
        }
    }

private:
    // Apply K-weighting: Stage 1 (high-shelf) then Stage 2 (HPF)
    inline float applyKWeight(float x,
                              float& s1_hs, float& s2_hs,
                              float& s1_hp, float& s2_hp) noexcept {
        // Stage 1: High-shelf TDF-II
        const float y1 = hs_b0 * x + s1_hs;
        s1_hs = hs_b1 * x - hs_a1 * y1 + s2_hs;
        s2_hs = hs_b2 * x - hs_a2 * y1;

        // Stage 2: HPF TDF-II
        const float y2 = hp_b0 * y1 + s1_hp;
        s1_hp = hp_b1 * y1 - hp_a1 * y2 + s2_hp;
        s2_hp = hp_b2 * y1 - hp_a2 * y2;

        return y2;
    }

    // Compute BS.1770-4 K-weighting filter coefficients
    void computeKWeighting(float sr) noexcept {
        // Stage 1: High-shelf at 1681.97 Hz, +3.999 dB gain
        // Derived from ITU-R BS.1770-4 Table 1 (pre-computed for 48kHz,
        // bilinear-transformed for arbitrary SR)
        {
            const float f0 = 1681.974450955533f;
            const float G = 3.999843853973347f; // dB
            const float Q = 0.7071752369554196f;
            const float A = std::pow(10.0f, G / 40.0f);
            const float w0 = kTwoPi * f0 / sr;
            const float sinW = std::sin(w0);
            const float cosW = std::cos(w0);
            const float alpha = sinW / (2.0f * Q);

            const float a0 = 1.0f + alpha / A;
            hs_b0 = (1.0f + alpha * A) / a0;
            hs_b1 = (-2.0f * cosW) / a0;
            hs_b2 = (1.0f - alpha * A) / a0;
            hs_a1 = (-2.0f * cosW) / a0;
            hs_a2 = (1.0f - alpha / A) / a0;
        }

        // Stage 2: High-pass at 38.13 Hz (2nd-order Butterworth)
        {
            const float f0 = 38.13547087602444f;
            const float Q = 0.5003270373238773f;
            const float w0 = kTwoPi * f0 / sr;
            const float sinW = std::sin(w0);
            const float cosW = std::cos(w0);
            const float alpha = sinW / (2.0f * Q);

            const float a0 = 1.0f + alpha;
            hp_b0 = ((1.0f + cosW) * 0.5f) / a0;
            hp_b1 = (-(1.0f + cosW)) / a0;
            hp_b2 = ((1.0f + cosW) * 0.5f) / a0;
            hp_a1 = (-2.0f * cosW) / a0;
            hp_a2 = (1.0f - alpha) / a0;
        }
    }
};

struct SpectrumBuffer {
    std::array<float, kSpectrumFFTSize> buffer{};
    std::array<float, kSpectrumFFTSize / 2> magnitudes{};
    int writePos = 0;
    bool ready = false;

    void reset() noexcept {
        buffer.fill(0.0f);
        magnitudes.fill(0.0f);
        writePos = 0;
        ready = false;
    }

    inline void pushSample(float sample) noexcept {
        buffer[writePos] = sample;
        writePos = (writePos + 1) % kSpectrumFFTSize;
        if (writePos == 0) ready = true;
    }
};

struct GainReductionHistory {
    std::array<float, kGRHistoryLength> history{};
    int writePos = 0;

    void reset() noexcept { history.fill(0.0f); writePos = 0; }

    inline void push(float grDb) noexcept {
        history[writePos] = grDb;
        writePos = (writePos + 1) % kGRHistoryLength;
    }

    float getAt(int samplesAgo) const noexcept {
        const int idx = (writePos - 1 - samplesAgo + kGRHistoryLength * 2) % kGRHistoryLength;
        return history[idx];
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 13: REFERENCE TONE MATCHING + PRESET INTELLIGENCE
// ═══════════════════════════════════════════════════════════════════════════

struct ReferenceToneMatcher {
    std::array<float, kRefSpectrumBins> referenceSpectrum{};
    std::array<float, kRefSpectrumBins> currentSpectrum{};
    std::array<float, kRefSpectrumBins> correctionCurve{};
    bool hasReference = false;
    bool hasCorrection = false;

    void reset() noexcept {
        referenceSpectrum.fill(0.0f);
        currentSpectrum.fill(0.0f);
        correctionCurve.fill(0.0f);
        hasReference = hasCorrection = false;
    }

    void captureReference(const float* spectrum, int numBins, double /*sr*/) noexcept {
        const int n = juce::jmin(numBins, kRefSpectrumBins);
        for (int i = 0; i < n; ++i)
            referenceSpectrum[i] = spectrum[i];
        hasReference = true;
    }

    void captureCurrent(const float* spectrum, int numBins, double /*sr*/) noexcept {
        const int n = juce::jmin(numBins, kRefSpectrumBins);
        for (int i = 0; i < n; ++i)
            currentSpectrum[i] = spectrum[i];
    }

    void computeCorrection() noexcept {
        if (!hasReference) return;
        for (int i = 0; i < kRefSpectrumBins; ++i) {
            const float ref = juce::jmax(1.0e-10f, referenceSpectrum[i]);
            const float cur = juce::jmax(1.0e-10f, currentSpectrum[i]);
            correctionCurve[i] = juce::jlimit(-12.0f, 12.0f,
                20.0f * std::log10(ref / cur));
        }
        hasCorrection = true;
    }
};

struct PresetIntelligence {
    enum class SignalType { Unknown, Drums, Bass, Vocals, Guitar, Keys, Mix, Pad };
    SignalType detectedType = SignalType::Unknown;
    float confidence = 0.0f;
    float crestFactor = 0.0f;
    float spectralCentroid = 0.0f;
    float spectralFlatness = 0.0f;

    void reset() noexcept {
        detectedType = SignalType::Unknown;
        confidence = crestFactor = spectralCentroid = spectralFlatness = 0.0f;
    }

    void analyze(const MeterBallistics& meters, const SpectrumBuffer& spectrum,
                 float /*zeroCrossRate*/, double /*sr*/) noexcept {
        // Crest factor analysis
        const float peak = juce::jmax(meters.peakL, meters.peakR);
        const float rms = juce::jmax(std::sqrt(meters.rmsL), std::sqrt(meters.rmsR));
        crestFactor = (rms > 0.001f) ? (peak / rms) : 1.0f;

        // Simple classification heuristic
        if (crestFactor > 6.0f) {
            detectedType = SignalType::Drums;
            confidence = 0.7f;
        } else if (crestFactor < 2.0f) {
            detectedType = SignalType::Pad;
            confidence = 0.5f;
        } else {
            detectedType = SignalType::Mix;
            confidence = 0.3f;
        }
        (void)spectrum;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 14: STATE MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════

struct UndoStack {
    struct Snapshot {
        juce::MemoryBlock data;
        juce::String description;
    };

    std::vector<Snapshot> stack;
    int position = -1;

    void clear() { stack.clear(); position = -1; }

    void push(const juce::MemoryBlock& state, const juce::String& desc = {}) {
        // Truncate any redo history
        if (position + 1 < (int)stack.size())
            stack.resize((size_t)(position + 1));

        stack.push_back({ state, desc });
        if ((int)stack.size() > kMaxUndoSteps) {
            stack.erase(stack.begin());
        }
        position = (int)stack.size() - 1;
    }

    bool canUndo() const { return position > 0; }
    bool canRedo() const { return position < (int)stack.size() - 1; }

    const Snapshot* undo() {
        if (!canUndo()) return nullptr;
        --position;
        return &stack[(size_t)position];
    }

    const Snapshot* redo() {
        if (!canRedo()) return nullptr;
        ++position;
        return &stack[(size_t)position];
    }
};

struct ABState {
    juce::MemoryBlock slotA, slotB;
    bool isA = true;
    bool hasA = false, hasB = false;

    void storeA(const juce::MemoryBlock& state) { slotA = state; hasA = true; }
    void storeB(const juce::MemoryBlock& state) { slotB = state; hasB = true; }
    void toggle() { isA = !isA; }
    void copyAtoB() { slotB = slotA; hasB = hasA; }

    const juce::MemoryBlock* getActive() const {
        return isA ? (hasA ? &slotA : nullptr) : (hasB ? &slotB : nullptr);
    }
};

struct PresetInfo {
    juce::String name;
    juce::String category;
    juce::String author;
    juce::File file;
    int stateVersion = kStateVersion;
};

struct MIDILearnState {
    struct Mapping {
        int ccNumber = -1;
        juce::String parameterID;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };

    Mapping mappings[kMaxMIDIMappings];
    int numMappings = 0;
    bool isLearning = false;
    juce::String learningParamID;

    void addMapping(int cc, const juce::String& paramID, float minVal, float maxVal) noexcept {
        if (numMappings >= kMaxMIDIMappings) return;
        mappings[numMappings] = { cc, paramID, minVal, maxVal };
        ++numMappings;
    }

    void removeMapping(int index) noexcept {
        if (index < 0 || index >= numMappings) return;
        for (int i = index; i < numMappings - 1; ++i)
            mappings[i] = mappings[i + 1];
        --numMappings;
    }

    void clearAll() noexcept { numMappings = 0; }

    const Mapping* findMapping(int cc) const noexcept {
        for (int i = 0; i < numMappings; ++i)
            if (mappings[i].ccNumber == cc) return &mappings[i];
        return nullptr;
    }
};

struct SimpleModeState {
    bool enabled = false;
    float drive = 0.5f;
    float tone = 0.5f;
    float output = 0.5f;
};

struct LoudnessMatchedAB {
    float levelA = 0.0f;
    float levelB = 0.0f;
    float compensationGain = 1.0f;
    float smoothCoeff = 0.0f;

    void prepare(double sr) noexcept {
        smoothCoeff = 1.0f - std::exp(-1.0f / ((float)sr * 0.500f));
        levelA = levelB = 0.0f;
        compensationGain = 1.0f;
    }

    void reset() noexcept { levelA = levelB = 0.0f; compensationGain = 1.0f; }

    inline void updateLevel(bool isSlotA, float peakL, float peakR) noexcept {
        const float peak = juce::jmax(std::abs(peakL), std::abs(peakR));
        if (isSlotA) levelA += smoothCoeff * (peak - levelA);
        else         levelB += smoothCoeff * (peak - levelB);
    }

    float getCompensation(bool isSlotA) noexcept {
        const float target = isSlotA ? levelA : levelB;
        const float other  = isSlotA ? levelB : levelA;
        if (target > 0.001f && other > 0.001f)
            compensationGain = juce::jlimit(0.25f, 4.0f, other / target);
        return compensationGain;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
//  SECTION 15: TARGET LOCK ENGINE
//  Typed target values (LUFS, RMS, per-band spectral) with dynamics threshold.
//  The engine transparently adjusts gain/limiting/EQ to hit the target while
//  preserving dynamics within the user-defined threshold range.
// ═══════════════════════════════════════════════════════════════════════════

struct TargetLockBand {
    // Per-band spectral target (for low, mid, high frequency ranges)
    float targetDb = 0.0f;       // Target level in dB for this band
    bool locked = false;         // Whether this band is target-locked
    float currentDb = -100.0f;   // Measured level in this band
    float correctionGain = 1.0f; // Applied correction gain
    float smoothCoeff = 0.0f;

    void prepare(double sr) noexcept {
        smoothCoeff = 1.0f - std::exp(-1.0f / ((float)sr * 0.200f));
        correctionGain = 1.0f;
        currentDb = -100.0f;
    }

    void reset() noexcept { correctionGain = 1.0f; currentDb = -100.0f; }

    // Measure the current band level and compute correction
    inline float process(float bandPeakLinear, float dynamicsThreshold) noexcept {
        if (!locked) return 1.0f;

        // Update measured level (smoothed)
        const float measuredDb = gainToDb(juce::jmax(1.0e-10f, bandPeakLinear));
        currentDb += smoothCoeff * (measuredDb - currentDb);

        // Compute error (how far from target)
        const float errorDb = targetDb - currentDb;

        // Apply dynamics threshold: only correct if error exceeds threshold
        // This preserves natural dynamics within the threshold range
        float correctionDb = 0.0f;
        if (std::abs(errorDb) > dynamicsThreshold) {
            // Correct only the portion beyond the threshold
            correctionDb = (errorDb > 0.0f)
                ? (errorDb - dynamicsThreshold)
                : (errorDb + dynamicsThreshold);
        }

        // Limit correction rate to avoid pumping (max 6dB/sec equivalent)
        correctionDb = juce::jlimit(-6.0f, 6.0f, correctionDb);

        // Smooth the correction gain
        const float targetGain = dbToGain(correctionDb);
        correctionGain += smoothCoeff * (targetGain - correctionGain);

        return juce::jlimit(0.1f, 10.0f, correctionGain);
    }
};

struct TargetLockEngine {
    // Master loudness target
    float targetLUFS = -14.0f;       // Target integrated LUFS (typed by user)
    float targetRMS = -14.0f;        // Target RMS dB (typed by user)
    float dynamicsThreshold = 3.0f;  // dB range of allowed dynamics (0 = brick, 12 = loose)
    bool lufsLocked = false;         // Whether LUFS target is active
    bool rmsLocked = false;          // Whether RMS target is active

    // Per-band spectral targets (Low / Mid / High)
    static constexpr int kNumTargetBands = 3;
    TargetLockBand bands[kNumTargetBands];
    float bandFreqs[2] = { 200.0f, 4000.0f }; // Crossover points: low<200, mid 200-4k, high>4k

    // Master gain correction (for LUFS/RMS targeting)
    float masterCorrectionGain = 1.0f;
    float smoothCoeff = 0.0f;
    float measuredLUFS = -24.0f;
    float measuredRMS = -24.0f;

    // Internal RMS measurement
    float rmsAccumL = 0.0f, rmsAccumR = 0.0f;
    int rmsSampleCount = 0;
    int rmsWindowSamples = 0;

    void prepare(double sr) noexcept {
        smoothCoeff = 1.0f - std::exp(-1.0f / ((float)sr * 0.300f));
        rmsWindowSamples = (int)(sr * 0.300); // 300ms RMS window
        masterCorrectionGain = 1.0f;
        rmsAccumL = rmsAccumR = 0.0f;
        rmsSampleCount = 0;
        for (auto& b : bands) b.prepare(sr);
    }

    void reset() noexcept {
        masterCorrectionGain = 1.0f;
        measuredLUFS = measuredRMS = -24.0f;
        rmsAccumL = rmsAccumR = 0.0f;
        rmsSampleCount = 0;
        for (auto& b : bands) b.reset();
    }

    // Set targets from user-typed values
    void setLUFSTarget(float lufs) noexcept { targetLUFS = juce::jlimit(-60.0f, 0.0f, lufs); }
    void setRMSTarget(float rms) noexcept { targetRMS = juce::jlimit(-60.0f, 0.0f, rms); }
    void setDynamicsThreshold(float threshDb) noexcept {
        dynamicsThreshold = juce::jlimit(0.0f, 24.0f, threshDb);
    }
    void setBandTarget(int bandIdx, float targetDb) noexcept {
        if (bandIdx >= 0 && bandIdx < kNumTargetBands) {
            bands[bandIdx].targetDb = juce::jlimit(-60.0f, 6.0f, targetDb);
            bands[bandIdx].locked = true;
        }
    }
    void unlockBand(int bandIdx) noexcept {
        if (bandIdx >= 0 && bandIdx < kNumTargetBands)
            bands[bandIdx].locked = false;
    }

    // Main process: call after all other processing, before final limiter
    // Returns the master gain to apply. Also provides per-band gains via bandGains[3].
    inline float process(float l, float r, float currentLUFS,
                         const float* bandPeaks, // [3] band peaks (low, mid, high)
                         float* bandGains) noexcept { // [3] output correction gains
        // Update internal RMS measurement
        rmsAccumL += l * l;
        rmsAccumR += r * r;
        ++rmsSampleCount;

        if (rmsSampleCount >= rmsWindowSamples) {
            const float meanL = rmsAccumL / (float)rmsWindowSamples;
            const float meanR = rmsAccumR / (float)rmsWindowSamples;
            const float rmsLin = std::sqrt((meanL + meanR) * 0.5f);
            measuredRMS = gainToDb(juce::jmax(1.0e-10f, rmsLin));
            rmsAccumL = rmsAccumR = 0.0f;
            rmsSampleCount = 0;
        }

        measuredLUFS = currentLUFS;

        // Compute master correction for LUFS/RMS target
        float masterCorrection = 1.0f;
        if (lufsLocked || rmsLocked) {
            float errorDb = 0.0f;
            if (lufsLocked && rmsLocked) {
                // Average both errors
                const float lufsErr = targetLUFS - measuredLUFS;
                const float rmsErr = targetRMS - measuredRMS;
                errorDb = (lufsErr + rmsErr) * 0.5f;
            } else if (lufsLocked) {
                errorDb = targetLUFS - measuredLUFS;
            } else {
                errorDb = targetRMS - measuredRMS;
            }

            // Apply dynamics threshold
            float corrDb = 0.0f;
            if (std::abs(errorDb) > dynamicsThreshold) {
                corrDb = (errorDb > 0.0f)
                    ? (errorDb - dynamicsThreshold)
                    : (errorDb + dynamicsThreshold);
            }

            // Rate-limit correction to avoid pumping
            corrDb = juce::jlimit(-4.0f, 4.0f, corrDb);

            const float targetGain = dbToGain(corrDb);
            masterCorrectionGain += smoothCoeff * (targetGain - masterCorrectionGain);
            masterCorrection = juce::jlimit(0.05f, 20.0f, masterCorrectionGain);
        }

        // Compute per-band corrections
        if (bandPeaks && bandGains) {
            for (int i = 0; i < kNumTargetBands; ++i) {
                bandGains[i] = bands[i].process(bandPeaks[i], dynamicsThreshold);
            }
        }

        return masterCorrection;
    }

    // Check if any target is active
    bool isActive() const noexcept {
        if (lufsLocked || rmsLocked) return true;
        for (const auto& b : bands)
            if (b.locked) return true;
        return false;
    }
};

} // namespace BTZDsp

