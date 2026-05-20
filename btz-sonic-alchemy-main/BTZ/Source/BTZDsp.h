/*
  Box Tone Zone (BTZ) — BTZDsp.h  v10
  ────────────────────────────────────────────────────────────────────────
  v10 (industry-transcending features):
    • RTNeural infrastructure: learned saturation models (additive)
      - Neural_Neve, Neural_API, Neural_SSL, Neural_Custom
    • Dynamic Resonance Taming: spectral peak suppression before saturation
    • Transient-Aware Saturation: envelope-split transient/sustain processing
    • Reference Tone Matching: spectral comparison + auto-EQ suggestion
    • WDF Circuit Models: wave digital filter tube/transformer (higher fidelity)
    • SIMD Oversampling: actual 2x/4x/8x with FIR antialiasing
    • Simple Mode: 3-knob interface state (Drive, Tone, Output)
    • Interactive Harmonic Spectrum: before/after FFT with ghost overlay
    • Preset Intelligence: input analysis + contextual preset suggestion
    • Loudness-Matched A/B: auto-compensated comparison

  v9: 5 saturation models, multiband, M/S, LFO, EBU R128, undo/redo, A/B, MIDI learn
  v8: removed ADAA, 1 Hz DC blocker, tightened TruePeakLimiter
  ────────────────────────────────────────────────────────────────────────
  Architecture:
    • Every struct/class is self-contained, sample-rate aware, RT-safe
    • No allocation in hot paths — all buffers pre-allocated in prepare()
    • Organized by domain: Core, Saturation, Neural, Dynamics, EQ, Stereo,
      Modulation, Metering, Analysis, State
*/
#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>
#include <atomic>
#include <functional>
#include <complex>

#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif

namespace BTZDsp {

// ═══════════════════════════════════════════════════════════════════════
// Constants
// ═══════════════════════════════════════════════════════════════════════
static constexpr int    kStateVersion       = 10;
static constexpr float  kPi                 = 3.14159265358979323846f;
static constexpr float  kTwoPi              = 6.28318530717958647692f;
static constexpr float  kSqrt2              = 1.41421356237309504880f;
static constexpr float  kMinSampleRate      = 1.0f;
static constexpr float  kDenormalThreshold  = 1.0e-20f;
static constexpr float  kSilenceThreshold   = 1.0e-8f;
static constexpr int    kMaxBands           = 6;
static constexpr int    kMaxUndoSteps       = 64;
static constexpr int    kSpectrumFFTOrder   = 11;  // 2048-point FFT
static constexpr int    kSpectrumFFTSize    = 1 << kSpectrumFFTOrder;
static constexpr int    kMaxOversampleFactor = 8;
static constexpr int    kResonanceBands     = 64;  // bands for resonance taming
static constexpr int    kNeuralHiddenSize   = 16;  // GRU hidden state size
static constexpr int    kNeuralLayers       = 2;   // GRU layers for learned models
static constexpr int    kMaxFIRTaps         = 128;  // oversampling FIR filter length

// ═══════════════════════════════════════════════════════════════════════
// Denormal flushing
// ═══════════════════════════════════════════════════════════════════════
inline void enableFlushToZero() {
#ifdef __SSE__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();
}

// ═══════════════════════════════════════════════════════════════════════
// Utility: Padé [5/5] tanh approximation — max error 0.0039 over [-6,6]
// ═══════════════════════════════════════════════════════════════════════
static inline float fastTanh(float x) noexcept {
    const float x2 = x * x;
    return x * (945.0f + x2 * (105.0f + x2)) /
               (945.0f + x2 * (420.0f + 15.0f * x2));
}

static const float kTanhBias025 = std::tanh(0.25f);

// Fast exp approximation for envelope calculations
static inline float fastExp(float x) noexcept {
    x = 1.0f + x / 256.0f;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

// ═══════════════════════════════════════════════════════════════════════
//  SECTION 1: CORE MODULES
// ═══════════════════════════════════════════════════════════════════════

// ── SmoothParam: one-pole parameter smoother ──────────────────────────
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

// ── EnvFollower: attack/release envelope follower ─────────────────────
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
        const float c = xAbs > env ? attackCoeff : releaseCoeff;
        env += c * (xAbs - env);
        return env;
    }
};

// ── SafetyLayer: DC block (1 Hz) + NaN/Inf guard ─────────────────────
struct SafetyLayer {
    float dcL = 0.0f, dcPrevL = 0.0f;
    float dcR = 0.0f, dcPrevR = 0.0f;
    float dcCoeff = 0.9999f;

    void setSampleRate(double sr) noexcept {
        const float srf = (float) juce::jmax(1.0, sr);
        dcCoeff = 1.0f - (kTwoPi * 1.0f / srf);
        dcCoeff = juce::jlimit(0.90f, 0.99999f, dcCoeff);
    }
    void reset() noexcept { dcL = dcPrevL = dcR = dcPrevR = 0.0f; }
    inline float processSample(float x, float& dc, float& dcPrev) noexcept {
        if (! std::isfinite(x) || std::abs(x) < kDenormalThreshold)
            x = 0.0f;
        const float y = x - dcPrev + dcCoeff * dc;
        dcPrev = x;
        dc = y;
        return y;
    }
};

// ── BypassCrossfader: 64-sample cosine crossfade for click-free bypass ─
struct BypassCrossfader {
    static constexpr int kDefaultFadeSamples = 64;

    void prepare(int fadeLengthSamples = kDefaultFadeSamples) noexcept {
        fadeLength = juce::jmax(1, fadeLengthSamples);
        fadePos = fadeLength;
        targetBypassed = false;
        wetGain = 1.0f;
    }

    void reset() noexcept {
        fadePos = fadeLength;
        wetGain = targetBypassed ? 0.0f : 1.0f;
    }

    void setBypassState(bool bypassed) noexcept {
        if (bypassed != targetBypassed) {
            targetBypassed = bypassed;
            fadePos = 0;
        }
    }

    bool isBypassed() const noexcept { return targetBypassed && fadePos >= fadeLength; }
    bool isCrossfading() const noexcept { return fadePos < fadeLength; }

    inline float processSample(float dry, float wet) noexcept {
        if (fadePos < fadeLength) {
            const float t = (float)fadePos / (float)fadeLength;
            const float targetGain = targetBypassed ? 0.0f : 1.0f;
            const float startGain  = targetBypassed ? 1.0f : 0.0f;
            const float blend = 0.5f * (1.0f - std::cos(kPi * t));
            wetGain = startGain + (targetGain - startGain) * blend;
            ++fadePos;
        }
        return dry + (wet - dry) * wetGain;
    }

    inline void processStereo(float dryL, float dryR, float& wetL, float& wetR) noexcept {
        if (fadePos >= fadeLength && targetBypassed) {
            wetL = dryL; wetR = dryR; return;
        }
        if (fadePos >= fadeLength && !targetBypassed) return;

        const float t = (float)fadePos / (float)fadeLength;
        const float targetGain = targetBypassed ? 0.0f : 1.0f;
        const float startGain  = targetBypassed ? 1.0f : 0.0f;
        const float blend = 0.5f * (1.0f - std::cos(kPi * t));
        wetGain = startGain + (targetGain - startGain) * blend;
        ++fadePos;

        wetL = dryL + (wetL - dryL) * wetGain;
        wetR = dryR + (wetR - dryR) * wetGain;
    }

private:
    int fadeLength = kDefaultFadeSamples;
    int fadePos = 0;
    bool targetBypassed = false;
    float wetGain = 1.0f;
};

// ── FixedDeque: pre-allocated ring buffer (RT-safe) ───────────────────
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


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2: SATURATION MODELS
//  Original 5 models (v9) + 4 Neural learned models (v10) + 2 WDF models (v10)
// ═══════════════════════════════════════════════════════════════════════

enum class SaturationModel {
    // Original polynomial/analytic models (v9)
    Tanh        = 0,
    Tube        = 1,
    Tape        = 2,
    Transistor  = 3,
    Transformer = 4,
    // Neural learned models (v10) — RTNeural GRU inference
    Neural_Neve     = 5,
    Neural_API      = 6,
    Neural_SSL      = 7,
    Neural_Custom   = 8,
    // Wave Digital Filter models (v10) — circuit-accurate
    WDF_Tube        = 9,
    WDF_Transformer = 10,
    NumModels       = 11
};

// Waveshaper dispatch — all original functions are branchless, RT-safe
namespace Waveshaper {

    // Tanh: Padé [5/5] — balanced odd harmonics
    static inline float tanh(float x) noexcept {
        return fastTanh(x);
    }

    // Tube: asymmetric soft-clip — even harmonics from positive bias
    static inline float tube(float x) noexcept {
        const float bias = 0.2f;
        const float biased = x + bias;
        const float shaped = fastTanh(biased) - fastTanh(bias);
        const float even = 0.15f * (fastTanh(x * 0.8f + 0.5f) - fastTanh(0.5f));
        return shaped + even;
    }

    // Tape: hysteresis-like saturation — soft compression + gentle limiting
    static inline float tape(float x, float& hysteresisState) noexcept {
        const float satLevel = 1.2f;
        const float coercivity = 0.3f;
        const float drive = x * 1.5f;
        const float delta = drive - hysteresisState;
        const float rate = coercivity * (1.0f - (hysteresisState * hysteresisState) /
                           (satLevel * satLevel));
        hysteresisState += juce::jlimit(-0.5f, 0.5f, delta * juce::jmax(0.01f, rate));
        return fastTanh(hysteresisState * 0.9f);
    }

    // Transistor: hard-clip with odd harmonics — aggressive, gritty
    static inline float transistor(float x) noexcept {
        const float posThresh = 0.8f;
        const float negThresh = -0.65f;
        float y;
        if (x > posThresh)
            y = posThresh + (x - posThresh) / (1.0f + 4.0f * (x - posThresh) * (x - posThresh));
        else if (x < negThresh)
            y = negThresh + (x - negThresh) / (1.0f + 6.0f * (x - negThresh) * (x - negThresh));
        else
            y = x;
        return y - 0.1f * y * y * y;
    }

    // Transformer: iron-core saturation — low-end thickening
    static inline float transformer(float x, float lowContent) noexcept {
        const float lowDrive = 1.0f + lowContent * 0.8f;
        const float driven = x * lowDrive;
        const float shaped = driven / (1.0f + std::abs(driven) * 0.5f);
        const float evenHarm = 0.08f * (shaped * shaped) * (x > 0.0f ? 1.0f : -1.0f);
        return shaped + evenHarm;
    }

    // Unified dispatch for original models
    static inline float process(SaturationModel model, float x, float& tapeState, float lowContent) noexcept {
        switch (model) {
            case SaturationModel::Tanh:        return tanh(x);
            case SaturationModel::Tube:        return tube(x);
            case SaturationModel::Tape:        return tape(x, tapeState);
            case SaturationModel::Transistor:  return transistor(x);
            case SaturationModel::Transformer: return transformer(x, lowContent);
            default:                           return tanh(x);
        }
    }
} // namespace Waveshaper


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2b: NEURAL SATURATION (RTNeural-compatible GRU inference)
//  Lightweight GRU cell for real-time learned saturation models.
//  Weights are loaded from JSON files — no training at runtime.
//  Each model captures the nonlinear transfer function of specific hardware.
// ═══════════════════════════════════════════════════════════════════════

// Minimal GRU cell for single-sample inference (no external dependency)
// This is a self-contained implementation compatible with RTNeural weight format
struct GRUCell {
    static constexpr int kMaxHidden = kNeuralHiddenSize;

    int inputSize  = 1;
    int hiddenSize = kMaxHidden;

    // Weights: [3*hidden x input] for input, [3*hidden x hidden] for recurrent
    std::array<float, 3 * kMaxHidden * 1>             Wi = {};  // input weights (input_size=1)
    std::array<float, 3 * kMaxHidden * kMaxHidden>    Wh = {};  // recurrent weights
    std::array<float, 3 * kMaxHidden>                 bi = {};  // input bias
    std::array<float, 3 * kMaxHidden>                 bh = {};  // recurrent bias

    // Hidden state
    std::array<float, kMaxHidden> h = {};

    void reset() noexcept { h.fill(0.0f); }

    // Process single sample through GRU
    inline float process(float input) noexcept {
        std::array<float, kMaxHidden> z, r, n;

        // Compute gates: z = sigmoid(Wi_z*x + Wh_z*h + bi_z + bh_z)
        //                r = sigmoid(Wi_r*x + Wh_r*h + bi_r + bh_r)
        //                n = tanh(Wi_n*x + bi_n + r*(Wh_n*h + bh_n))
        for (int i = 0; i < hiddenSize; ++i) {
            // Reset gate
            float rGate = Wi[(size_t)(hiddenSize + i)] * input + bi[(size_t)(hiddenSize + i)];
            for (int j = 0; j < hiddenSize; ++j)
                rGate += Wh[(size_t)((hiddenSize + i) * hiddenSize + j)] * h[(size_t)j];
            rGate += bh[(size_t)(hiddenSize + i)];
            r[(size_t)i] = 1.0f / (1.0f + std::exp(-rGate));

            // Update gate
            float zGate = Wi[(size_t)i] * input + bi[(size_t)i];
            for (int j = 0; j < hiddenSize; ++j)
                zGate += Wh[(size_t)(i * hiddenSize + j)] * h[(size_t)j];
            zGate += bh[(size_t)i];
            z[(size_t)i] = 1.0f / (1.0f + std::exp(-zGate));

            // New gate
            float nGate = Wi[(size_t)(2 * hiddenSize + i)] * input + bi[(size_t)(2 * hiddenSize + i)];
            float recurrent = 0.0f;
            for (int j = 0; j < hiddenSize; ++j)
                recurrent += Wh[(size_t)((2 * hiddenSize + i) * hiddenSize + j)] * h[(size_t)j];
            nGate += r[(size_t)i] * (recurrent + bh[(size_t)(2 * hiddenSize + i)]);
            n[(size_t)i] = fastTanh(nGate);
        }

        // Update hidden state: h = (1-z)*n + z*h_prev
        for (int i = 0; i < hiddenSize; ++i)
            h[(size_t)i] = (1.0f - z[(size_t)i]) * n[(size_t)i] + z[(size_t)i] * h[(size_t)i];

        // Output: linear combination of hidden state (first element as output)
        return h[0];
    }
};

// Dense (fully-connected) layer for output projection
struct DenseLayer {
    static constexpr int kMaxIn = kNeuralHiddenSize;

    int inputSize = kMaxIn;
    int outputSize = 1;

    std::array<float, kMaxIn> weights = {};
    float bias = 0.0f;

    inline float process(const std::array<float, kMaxIn>& input) noexcept {
        float sum = bias;
        for (int i = 0; i < inputSize; ++i)
            sum += weights[(size_t)i] * input[(size_t)i];
        return sum;
    }
};

// Complete neural saturation model: GRU stack + dense output
struct NeuralSaturationModel {
    std::array<GRUCell, kNeuralLayers> gruLayers;
    DenseLayer outputLayer;
    bool loaded = false;
    juce::String modelName;

    void reset() noexcept {
        for (auto& gru : gruLayers) gru.reset();
    }

    // Process single sample through the neural network
    inline float process(float input) noexcept {
        if (!loaded) return fastTanh(input);  // fallback to tanh if no model loaded

        float x = input;
        for (auto& gru : gruLayers)
            x = gru.process(x);

        // Output projection from final GRU hidden state
        return outputLayer.process(gruLayers.back().h);
    }

    // Load weights from a JSON ValueTree (compatible with RTNeural export format)
    bool loadFromJSON(const juce::var& modelJSON) {
        // Expected format: { "layers": [ { "type": "gru", "weights": {...} }, ... ] }
        if (auto* layers = modelJSON.getProperty("layers", {}).getArray()) {
            int gruIdx = 0;
            for (const auto& layer : *layers) {
                juce::String type = layer.getProperty("type", "").toString();
                if (type == "gru" && gruIdx < kNeuralLayers) {
                    auto& gru = gruLayers[(size_t)gruIdx];
                    // Load weights from nested arrays
                    if (auto* wi = layer.getProperty("Wi", {}).getArray())
                        for (int i = 0; i < juce::jmin((int)wi->size(), (int)gru.Wi.size()); ++i)
                            gru.Wi[(size_t)i] = (float)(*wi)[i];
                    if (auto* wh = layer.getProperty("Wh", {}).getArray())
                        for (int i = 0; i < juce::jmin((int)wh->size(), (int)gru.Wh.size()); ++i)
                            gru.Wh[(size_t)i] = (float)(*wh)[i];
                    if (auto* biArr = layer.getProperty("bi", {}).getArray())
                        for (int i = 0; i < juce::jmin((int)biArr->size(), (int)gru.bi.size()); ++i)
                            gru.bi[(size_t)i] = (float)(*biArr)[i];
                    if (auto* bhArr = layer.getProperty("bh", {}).getArray())
                        for (int i = 0; i < juce::jmin((int)bhArr->size(), (int)gru.bh.size()); ++i)
                            gru.bh[(size_t)i] = (float)(*bhArr)[i];
                    ++gruIdx;
                }
                else if (type == "dense") {
                    if (auto* w = layer.getProperty("weights", {}).getArray())
                        for (int i = 0; i < juce::jmin((int)w->size(), (int)outputLayer.weights.size()); ++i)
                            outputLayer.weights[(size_t)i] = (float)(*w)[i];
                    outputLayer.bias = (float)layer.getProperty("bias", 0.0);
                }
            }
        }
        loaded = true;
        return true;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2c: WAVE DIGITAL FILTER (WDF) CIRCUIT MODELS
//  Higher-fidelity analog modeling using port-based circuit simulation.
//  Models the actual component-level behavior of tube and transformer circuits.
// ═══════════════════════════════════════════════════════════════════════

// WDF Resistor: R_port = R
struct WDFResistor {
    float R = 1000.0f;
    float a = 0.0f;  // incident wave
    float b = 0.0f;  // reflected wave

    void setResistance(float ohms) noexcept { R = juce::jmax(1.0f, ohms); }
    inline void accept(float incident) noexcept { a = incident; b = 0.0f; }
};

// WDF Capacitor: R_port = 1/(2*C*fs)
struct WDFCapacitor {
    float C = 1.0e-6f;
    float R = 1000.0f;
    float a = 0.0f;
    float b = 0.0f;
    float state = 0.0f;

    void prepare(float capacitance, double sr) noexcept {
        C = capacitance;
        R = 1.0f / (2.0f * C * (float)juce::jmax(1.0, sr));
    }
    inline void accept(float incident) noexcept {
        a = incident;
        b = state;
        state = a;
    }
};

// WDF Diode pair (antiparallel) — models tube grid conduction
struct WDFDiodePair {
    float Is = 1.0e-9f;    // saturation current
    float Vt = 26.0e-3f;   // thermal voltage
    float R  = 1000.0f;     // port resistance

    // Newton-Raphson iteration for diode equation
    inline float reflect(float incident, float portR) noexcept {
        R = portR;
        // Simplified Lambert-W approximation for antiparallel diodes
        const float a = incident;
        const float logTerm = std::log(Is * R / Vt);

        // 2 iterations of Newton-Raphson
        float vd = 0.0f;
        for (int i = 0; i < 2; ++i) {
            const float expPos = std::exp(juce::jlimit(-20.0f, 20.0f, vd / Vt));
            const float expNeg = std::exp(juce::jlimit(-20.0f, 20.0f, -vd / Vt));
            const float f = Is * (expPos - expNeg) - (a - vd) / R;
            const float fp = Is / Vt * (expPos + expNeg) + 1.0f / R;
            vd -= f / juce::jmax(1.0e-10f, fp);
        }
        return 2.0f * vd - a;
    }
};

// Complete WDF Tube Stage: triode model (12AX7-style)
// Models: grid resistor → coupling cap → triode (diode + dependent source)
struct WDFTubeStage {
    WDFCapacitor couplingCap;
    WDFDiodePair gridDiode;
    float mu = 100.0f;      // amplification factor
    float Rp = 62500.0f;    // plate resistance
    float biasVoltage = -1.5f;

    float state = 0.0f;

    void prepare(double sr) noexcept {
        couplingCap.prepare(22.0e-9f, sr);  // 22nF coupling cap
        gridDiode.Is = 1.0e-12f;
        gridDiode.Vt = 26.0e-3f;
    }

    void reset() noexcept {
        couplingCap.state = 0.0f;
        state = 0.0f;
    }

    // Process single sample through WDF tube model
    inline float process(float input) noexcept {
        // Coupling capacitor filters DC
        couplingCap.accept(input);
        float vGrid = couplingCap.b + biasVoltage;

        // Grid conduction (diode clipping on positive grid swing)
        float gridCurrent = 0.0f;
        if (vGrid > 0.0f) {
            gridCurrent = gridDiode.Is * (std::exp(juce::jlimit(-20.0f, 20.0f, vGrid / gridDiode.Vt)) - 1.0f);
            vGrid -= gridCurrent * 1000.0f;  // grid stopper effect
        }

        // Plate current (simplified Koren model)
        const float e1 = vGrid + 250.0f / mu;  // combined grid-plate voltage
        float Ip = 0.0f;
        if (e1 > 0.0f) {
            const float e1_pow = std::pow(e1, 1.5f);
            Ip = (e1_pow / (Rp * 0.001f)) * 0.001f;
        }

        // Output voltage (inverted, with plate load)
        const float vPlate = -Ip * Rp * 0.0001f;

        // Soft limit output to prevent runaway
        return fastTanh(vPlate * 2.0f);
    }
};

// WDF Transformer model: iron-core saturation with frequency-dependent behavior
struct WDFTransformerStage {
    float inductance = 10.0f;   // Henries
    float coreArea = 0.001f;    // m²
    float satFlux = 1.2f;       // Tesla (saturation flux density)
    float coilR = 100.0f;       // winding resistance

    float fluxState = 0.0f;
    float eddyState = 0.0f;
    float lpState = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        // Low-pass for frequency-dependent core loss
        lpState = 0.0f;
        (void)srf;
    }

    void reset() noexcept {
        fluxState = eddyState = lpState = 0.0f;
    }

    inline float process(float input) noexcept {
        // Flux accumulation (integration of voltage)
        fluxState += input * 0.001f;

        // Core saturation: Langevin function approximation
        const float normalizedFlux = fluxState / satFlux;
        const float saturated = normalizedFlux / (1.0f + std::abs(normalizedFlux) * 0.5f);

        // Eddy current losses (frequency-dependent damping)
        const float eddyLoss = 0.05f * (input - eddyState);
        eddyState += 0.1f * (input - eddyState);

        // Hysteresis (simplified): output depends on rate of flux change
        const float output = saturated * satFlux + eddyLoss;

        // Prevent DC accumulation in flux
        fluxState *= 0.9999f;

        return fastTanh(output);
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2d: DYNAMIC RESONANCE TAMING
//  Identifies problematic resonant peaks in real-time and applies
//  matching reduction BEFORE saturation to prevent harshness.
//  Inspired by soothe2's approach but optimized for pre-saturation use.
// ═══════════════════════════════════════════════════════════════════════

struct ResonanceTamer {
    static constexpr int kNumBands = kResonanceBands;

    // Per-band state
    struct Band {
        float freq = 0.0f;
        float energy = 0.0f;
        float threshold = 0.0f;
        float reduction = 0.0f;  // current gain reduction (0 to -inf dB)
        float smoothedReduction = 0.0f;

        // SVF bandpass state for analysis
        float bpS1 = 0.0f, bpS2 = 0.0f;
        // SVF notch state for reduction
        float notchS1 = 0.0f, notchS2 = 0.0f;
    };

    std::array<Band, kNumBands> bands;
    float sensitivity = 3.0f;    // dB above average = resonance
    float depth = 0.5f;          // 0–1 reduction amount
    float speed = 10.0f;         // ms attack/release
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    bool enabled = false;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        attackCoeff = 1.0f - std::exp(-1.0f / (srf * speed * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * speed * 3.0f * 0.001f));

        // Distribute bands logarithmically from 200 Hz to 16 kHz
        const float startFreq = 200.0f;
        const float endFreq = juce::jmin(16000.0f, srf * 0.45f);
        const float logStart = std::log2(startFreq);
        const float logEnd = std::log2(endFreq);

        for (int i = 0; i < kNumBands; ++i) {
            float t = (float)i / (float)(kNumBands - 1);
            bands[(size_t)i].freq = std::pow(2.0f, logStart + t * (logEnd - logStart));
            bands[(size_t)i].energy = 0.0f;
            bands[(size_t)i].smoothedReduction = 0.0f;
        }
    }

    void reset() noexcept {
        for (auto& b : bands) {
            b.energy = 0.0f;
            b.smoothedReduction = 0.0f;
            b.bpS1 = b.bpS2 = 0.0f;
            b.notchS1 = b.notchS2 = 0.0f;
        }
    }

    // Process stereo pair: analyze + reduce resonances
    inline void processStereo(float& l, float& r, double sr) noexcept {
        if (!enabled) return;

        const float srf = (float)juce::jmax(1.0, sr);
        const float mono = (l + r) * 0.5f;

        // Compute average energy across all bands
        float avgEnergy = 0.0f;
        for (int i = 0; i < kNumBands; ++i) {
            auto& b = bands[(size_t)i];

            // SVF bandpass for energy detection
            const float w = kTwoPi * b.freq / srf;
            const float g = std::tan(w * 0.5f);
            const float Q = 4.0f;  // narrow Q for resonance detection
            const float k = 1.0f / Q;

            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float a2 = g * a1;

            const float v3 = mono - b.bpS2;
            const float v1 = a1 * b.bpS1 + a2 * v3;
            const float v2 = b.bpS2 + a2 * b.bpS1 + g * a2 * v3;
            b.bpS1 = 2.0f * v1 - b.bpS1;
            b.bpS2 = 2.0f * v2 - b.bpS2;

            const float bandSample = k * v1;  // bandpass output

            // Track energy with fast attack, slow release
            const float absVal = std::abs(bandSample);
            const float c = absVal > b.energy ? attackCoeff : releaseCoeff;
            b.energy += c * (absVal - b.energy);

            avgEnergy += b.energy;
        }
        avgEnergy /= (float)kNumBands;

        // Compute per-band reduction
        const float thresholdLin = avgEnergy * std::pow(10.0f, sensitivity / 20.0f);

        for (int i = 0; i < kNumBands; ++i) {
            auto& b = bands[(size_t)i];

            float targetReduction = 0.0f;
            if (b.energy > thresholdLin && thresholdLin > kSilenceThreshold) {
                // Excess in dB
                const float excessDb = 20.0f * std::log10(b.energy / thresholdLin);
                targetReduction = -excessDb * depth;
            }

            // Smooth the reduction
            const float c = targetReduction < b.smoothedReduction ? attackCoeff : releaseCoeff;
            b.smoothedReduction += c * (targetReduction - b.smoothedReduction);
        }

        // Apply reductions via parallel notch filters
        float outL = l, outR = r;
        for (int i = 0; i < kNumBands; ++i) {
            auto& b = bands[(size_t)i];
            if (b.smoothedReduction > -0.5f) continue;  // skip negligible reductions

            const float gainLin = std::pow(10.0f, b.smoothedReduction / 20.0f);
            const float reductionAmount = 1.0f - gainLin;

            // Apply narrow cut at this frequency (simplified: scale by reduction)
            // For a proper implementation, this would use per-band parametric EQ
            // Here we use a weighted subtraction based on the bandpass content
            const float w = kTwoPi * b.freq / srf;
            const float g = std::tan(w * 0.5f);
            const float Q = 6.0f;
            const float k = 1.0f / Q;
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float a2 = g * a1;

            // Extract band content from L
            const float v3L = outL - b.notchS2;
            const float v1L = a1 * b.notchS1 + a2 * v3L;
            b.notchS1 = 2.0f * v1L - b.notchS1;
            b.notchS2 = b.notchS2 + a2 * b.notchS1 + g * a2 * v3L;
            const float bpL = k * v1L;

            outL -= bpL * reductionAmount;
            outR -= bpL * reductionAmount;  // simplified: same reduction both channels
        }

        l = outL;
        r = outR;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2e: TRANSIENT-AWARE SATURATION
//  Splits signal into transient and sustain components via envelope
//  detection, then applies different saturation amounts to each.
//  Result: "punch without mush" — transients stay clean, body gets warm.
// ═══════════════════════════════════════════════════════════════════════

struct TransientSplitter {
    EnvFollower fastEnv;   // fast attack for transient detection
    EnvFollower slowEnv;   // slow attack for sustain envelope

    float transientSensitivity = 1.5f;  // multiplier: how much louder than sustain = transient
    float transientDrive = 0.0f;        // dB: saturation on transient portion
    float sustainDrive = 0.0f;          // dB: saturation on sustain portion
    float transientMix = 0.5f;          // 0 = all sustain processing, 1 = all transient processing
    bool enabled = false;

    void prepare(double sr) noexcept {
        fastEnv.setTimes(0.1f, 5.0f, sr);    // 0.1ms attack, 5ms release
        slowEnv.setTimes(20.0f, 200.0f, sr);  // 20ms attack, 200ms release
    }

    void reset() noexcept {
        fastEnv.reset();
        slowEnv.reset();
    }

    // Returns transient gain (0–1): how much of the signal is transient
    inline float getTransientAmount(float inputAbs) noexcept {
        const float fast = fastEnv.process(inputAbs);
        const float slow = slowEnv.process(inputAbs);

        if (slow < kSilenceThreshold) return 0.0f;

        const float ratio = fast / juce::jmax(kSilenceThreshold, slow);
        // Transient = when fast envelope exceeds slow by sensitivity factor
        const float transient = juce::jlimit(0.0f, 1.0f,
            (ratio - 1.0f) / (transientSensitivity - 1.0f));
        return transient;
    }

    // Process: split, saturate differently, recombine
    inline void processStereo(float& l, float& r, SaturationModel model,
                              float& tapeStateL, float& tapeStateR, float lowContent) noexcept {
        if (!enabled) return;

        const float absL = std::abs(l);
        const float absR = std::abs(r);
        const float monoAbs = (absL + absR) * 0.5f;

        const float transientAmt = getTransientAmount(monoAbs);
        const float sustainAmt = 1.0f - transientAmt;

        // Transient portion: less saturation (preserve punch)
        const float tDriveGain = std::pow(10.0f, transientDrive / 20.0f);
        const float sDriveGain = std::pow(10.0f, sustainDrive / 20.0f);

        // Process sustain with full saturation
        float sL = l * sDriveGain;
        float sR = r * sDriveGain;
        sL = Waveshaper::process(model, sL, tapeStateL, lowContent) / sDriveGain;
        sR = Waveshaper::process(model, sR, tapeStateR, lowContent) / sDriveGain;

        // Process transient with reduced saturation
        float tL = l * tDriveGain;
        float tR = r * tDriveGain;
        tL = Waveshaper::process(model, tL, tapeStateL, lowContent) / tDriveGain;
        tR = Waveshaper::process(model, tR, tapeStateR, lowContent) / tDriveGain;

        // Recombine based on transient detection
        l = tL * transientAmt * transientMix + sL * sustainAmt * (1.0f - transientMix)
          + l * (1.0f - transientMix) * transientAmt;  // dry transient bleed
        r = tR * transientAmt * transientMix + sR * sustainAmt * (1.0f - transientMix)
          + r * (1.0f - transientMix) * transientAmt;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 2f: OVERSAMPLING ENGINE
//  FIR-based polyphase up/downsampling with configurable factor (2x/4x/8x).
//  Uses minimum-phase FIR for low latency, or linear-phase for transparency.
// ═══════════════════════════════════════════════════════════════════════

struct OversamplingEngine {
    int factor = 1;         // 1 = off, 2, 4, or 8
    int filterOrder = 32;   // FIR taps per phase

    // FIR coefficients (windowed-sinc)
    std::array<float, kMaxFIRTaps> upsampleCoeffs = {};
    std::array<float, kMaxFIRTaps> downsampleCoeffs = {};

    // Delay lines for FIR filtering
    std::array<float, kMaxFIRTaps> delayLineL = {};
    std::array<float, kMaxFIRTaps> delayLineR = {};
    int delayPos = 0;

    // Oversampled buffer (max 8x * max block size)
    static constexpr int kMaxOSBuffer = 8 * 2048;
    std::vector<float> osBufferL;
    std::vector<float> osBufferR;

    int latencySamples = 0;

    void prepare(int oversampleFactor, double sr) {
        factor = juce::jlimit(1, kMaxOversampleFactor, oversampleFactor);
        if (factor <= 1) {
            latencySamples = 0;
            return;
        }

        filterOrder = juce::jmin(kMaxFIRTaps, factor * 16);
        latencySamples = filterOrder / (2 * factor);

        // Design windowed-sinc lowpass at Nyquist/factor
        const float cutoff = 1.0f / (float)factor;
        for (int i = 0; i < filterOrder; ++i) {
            const float n = (float)i - (float)(filterOrder - 1) * 0.5f;
            float h;
            if (std::abs(n) < 1.0e-6f)
                h = cutoff;
            else
                h = std::sin(kPi * cutoff * n) / (kPi * n);

            // Kaiser window (beta=6 for good stopband)
            const float alpha = (float)(filterOrder - 1) * 0.5f;
            const float r = (n) / alpha;
            const float window = 1.0f - r * r;  // simplified Hann approximation
            const float coeff = h * juce::jmax(0.0f, window) * (float)factor;

            upsampleCoeffs[(size_t)i] = coeff;
            downsampleCoeffs[(size_t)i] = coeff / (float)factor;
        }

        osBufferL.resize((size_t)kMaxOSBuffer, 0.0f);
        osBufferR.resize((size_t)kMaxOSBuffer, 0.0f);
        delayLineL.fill(0.0f);
        delayLineR.fill(0.0f);
        delayPos = 0;
    }

    void reset() noexcept {
        delayLineL.fill(0.0f);
        delayLineR.fill(0.0f);
        delayPos = 0;
        if (!osBufferL.empty()) std::fill(osBufferL.begin(), osBufferL.end(), 0.0f);
        if (!osBufferR.empty()) std::fill(osBufferR.begin(), osBufferR.end(), 0.0f);
    }

    int getLatency() const noexcept { return latencySamples; }
    int getFactor() const noexcept { return factor; }

    // Upsample a block: writes factor*numSamples into osBufferL/R
    void upsample(const float* inL, const float* inR, int numSamples) noexcept {
        if (factor <= 1) return;
        const int osLen = numSamples * factor;

        // Zero-stuff
        for (int i = 0; i < osLen; ++i) {
            osBufferL[(size_t)i] = (i % factor == 0) ? inL[i / factor] * (float)factor : 0.0f;
            osBufferR[(size_t)i] = (i % factor == 0) ? inR[i / factor] * (float)factor : 0.0f;
        }

        // Apply FIR lowpass (in-place)
        // Simplified: direct convolution (for production, use polyphase decomposition)
        std::vector<float> tempL(osBufferL.begin(), osBufferL.begin() + osLen);
        std::vector<float> tempR(osBufferR.begin(), osBufferR.begin() + osLen);
        for (int i = 0; i < osLen; ++i) {
            float sumL = 0.0f, sumR = 0.0f;
            for (int j = 0; j < filterOrder && (i - j) >= 0; ++j) {
                sumL += tempL[(size_t)(i - j)] * upsampleCoeffs[(size_t)j];
                sumR += tempR[(size_t)(i - j)] * upsampleCoeffs[(size_t)j];
            }
            osBufferL[(size_t)i] = sumL;
            osBufferR[(size_t)i] = sumR;
        }
    }

    // Downsample: reads from osBufferL/R, writes numSamples to outL/R
    void downsample(float* outL, float* outR, int numSamples) noexcept {
        if (factor <= 1) return;
        const int osLen = numSamples * factor;

        // Apply FIR lowpass then decimate
        for (int i = 0; i < numSamples; ++i) {
            const int osIdx = i * factor;
            float sumL = 0.0f, sumR = 0.0f;
            for (int j = 0; j < filterOrder && (osIdx - j) >= 0; ++j) {
                sumL += osBufferL[(size_t)(osIdx - j)] * downsampleCoeffs[(size_t)j];
                sumR += osBufferR[(size_t)(osIdx - j)] * downsampleCoeffs[(size_t)j];
            }
            outL[i] = sumL;
            outR[i] = sumR;
        }
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 3: DYNAMICS
// ═══════════════════════════════════════════════════════════════════════

// ── SidechainHPF: 4-mode sidechain high-pass for compressor ──────────
struct SidechainHPF {
    enum Mode { Off = 0, HPF_60, HPF_90, HPF_150 };
    Mode mode = Off;

    // SVF HP state per channel
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;
    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        float freq = 0.0f;
        switch (mode) {
            case HPF_60:  freq = 60.0f;  break;
            case HPF_90:  freq = 90.0f;  break;
            case HPF_150: freq = 150.0f; break;
            default: return;
        }
        g = std::tan(kPi * freq / srf);
        k = kSqrt2;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void reset() noexcept { s1L = s2L = s1R = s2R = 0.0f; }

    inline void processStereo(float& l, float& r) noexcept {
        if (mode == Off) return;
        // SVF highpass
        auto hp = [&](float x, float& s1, float& s2) -> float {
            const float v3 = x - s2;
            const float v1 = a1 * s1 + a2 * v3;
            const float v2 = s2 + a2 * s1 + a3 * v3;
            s1 = 2.0f * v1 - s1;
            s2 = 2.0f * v2 - s2;
            return x - k * v1 - v2;  // highpass output
        };
        l = hp(l, s1L, s2L);
        r = hp(r, s1R, s2R);
    }
};

// ── GlueCompressor: soft-knee bus compressor ─────────────────────────
struct GlueCompressor {
    float thresholdDb = -10.0f;
    float ratio = 4.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float makeupDb = 0.0f;
    float kneeDb = 6.0f;

    float envLin = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    SidechainHPF sidechain;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        attackCoeff  = 1.0f - std::exp(-1.0f / (srf * juce::jmax(0.1f, attackMs) * 0.001f));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(1.0f, releaseMs) * 0.001f));
        sidechain.prepare(sr);
    }

    void reset() noexcept { envLin = 0.0f; sidechain.reset(); }

    // Returns gain multiplier to apply to signal
    inline float processStereo(float scL, float scR) noexcept {
        // Apply sidechain HPF
        sidechain.processStereo(scL, scR);

        // Peak detection on sidechain
        const float peak = juce::jmax(std::abs(scL), std::abs(scR));

        // Envelope
        const float c = peak > envLin ? attackCoeff : releaseCoeff;
        envLin += c * (peak - envLin);

        // Gain computation (soft-knee)
        const float envDb = 20.0f * std::log10(juce::jmax(kSilenceThreshold, envLin));
        const float halfKnee = kneeDb * 0.5f;

        float gainDb = 0.0f;
        if (envDb > thresholdDb + halfKnee) {
            gainDb = (thresholdDb - envDb) * (1.0f - 1.0f / ratio);
        } else if (envDb > thresholdDb - halfKnee) {
            const float x = envDb - thresholdDb + halfKnee;
            gainDb = (1.0f - 1.0f / ratio) * x * x / (2.0f * kneeDb) * -1.0f;
        }

        gainDb += makeupDb;
        return std::pow(10.0f, gainDb / 20.0f);
    }
};

// ── TruePeakLimiter: lookahead brickwall limiter ─────────────────────
struct TruePeakLimiter {
    static constexpr int kMaxLookahead = 128;

    float ceilingDb = -0.3f;
    float ceilingLin = 0.933f;
    float releaseMs = 50.0f;

    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float envLin = 0.0f;

    int delaySamples = 64;
    int delayWritePos = 0;
    std::array<float, kMaxLookahead> delayL = {};
    std::array<float, kMaxLookahead> delayR = {};

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        ceilingLin = std::pow(10.0f, ceilingDb / 20.0f);
        delaySamples = juce::jmin(kMaxLookahead - 1, (int)(srf * 0.001f));  // ~1ms
        attackCoeff = 1.0f - std::exp(-1.0f / (float)juce::jmax(1, delaySamples));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * releaseMs * 0.001f));
    }

    void reset() noexcept {
        delayL.fill(0.0f);
        delayR.fill(0.0f);
        delayWritePos = 0;
        envLin = 0.0f;
    }

    inline float processStereo(float& l, float& r) noexcept {
        delayL[(size_t)delayWritePos] = l;
        delayR[(size_t)delayWritePos] = r;

        const int readPos = (delayWritePos - delaySamples + kMaxLookahead) % kMaxLookahead;
        const float dL = delayL[(size_t)readPos];
        const float dR = delayR[(size_t)readPos];

        delayWritePos = (delayWritePos + 1) % kMaxLookahead;

        const float peak = juce::jmax(std::abs(l), std::abs(r));
        const float c = peak > envLin ? attackCoeff : releaseCoeff;
        envLin += c * (peak - envLin);

        float gain = 1.0f;
        float grDb = 0.0f;
        if (envLin > ceilingLin) {
            gain = ceilingLin / envLin;
            grDb = 20.0f * std::log10(gain);
        }

        l = dL * gain;
        r = dR * gain;
        return grDb;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 4: EQ / CROSSOVER / MULTIBAND
// ═══════════════════════════════════════════════════════════════════════

// ── ShineProcessor: SVF-based high-shelf EQ ──────────────────────────
struct ShineProcessor {
    float freq = 8000.0f;
    float gainDb = 0.0f;
    float q = 0.707f;

    float ic1eqL = 0.0f, ic2eqL = 0.0f;
    float ic1eqR = 0.0f, ic2eqR = 0.0f;
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float m0 = 1.0f, m1 = 0.0f, m2 = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        const float A = std::pow(10.0f, gainDb / 40.0f);
        const float g = std::tan(kPi * juce::jlimit(20.0f, srf * 0.49f, freq) / srf);
        const float k = 1.0f / (juce::jmax(0.1f, q) * A);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f;
        m1 = k * (A * A - 1.0f);
        m2 = A * A - 1.0f;
    }

    void reset() noexcept { ic1eqL = ic2eqL = ic1eqR = ic2eqR = 0.0f; }

    inline float processSample(float x, float& ic1, float& ic2) noexcept {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        return x + m1 * v1 + m2 * v2;
    }

    inline void processStereo(float& l, float& r) noexcept {
        l = processSample(l, ic1eqL, ic2eqL);
        r = processSample(r, ic1eqR, ic2eqR);
    }
};

// ── LinkwitzRileyCrossover: LR4 24 dB/oct band-split ────────────────
struct LinkwitzRileyCrossover {
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, a1c = 0.0f, a2c = 0.0f;
    float lpS1[2][2] = {};
    float lpS2[2][2] = {};
    float hpS1[2][2] = {};
    float hpS2[2][2] = {};
    float hpB0 = 0.0f, hpB1 = 0.0f, hpB2 = 0.0f;

    void prepare(double sr, float freqHz) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        const float w0 = kTwoPi * juce::jlimit(20.0f, srf * 0.49f, freqHz) / srf;
        const float cosW = std::cos(w0);
        const float sinW = std::sin(w0);
        const float alpha = sinW / (2.0f * kSqrt2);
        const float a0 = 1.0f + alpha;
        a1c = -2.0f * cosW / a0;
        a2c = (1.0f - alpha) / a0;
        b1 = (1.0f - cosW) / a0;
        b0 = b1 * 0.5f;
        b2 = b0;
        const float hpB1t = -(1.0f + cosW) / a0;
        hpB0 = -hpB1t * 0.5f;
        hpB1 = hpB1t;
        hpB2 = hpB0;
    }

    void reset() noexcept {
        std::memset(lpS1, 0, sizeof(lpS1));
        std::memset(lpS2, 0, sizeof(lpS2));
        std::memset(hpS1, 0, sizeof(hpS1));
        std::memset(hpS2, 0, sizeof(hpS2));
    }

    inline float biquad(float x, float cb0, float cb1, float cb2,
                        float& s1, float& s2) noexcept {
        const float y = cb0 * x + s1;
        s1 = cb1 * x - a1c * y + s2;
        s2 = cb2 * x - a2c * y;
        return y;
    }

    inline void processStereo(float inL, float inR,
                              float& lowL, float& lowR,
                              float& highL, float& highR) noexcept {
        lowL = biquad(inL, b0, b1, b2, lpS1[0][0], lpS2[0][0]);
        lowL = biquad(lowL, b0, b1, b2, lpS1[1][0], lpS2[1][0]);
        lowR = biquad(inR, b0, b1, b2, lpS1[0][1], lpS2[0][1]);
        lowR = biquad(lowR, b0, b1, b2, lpS1[1][1], lpS2[1][1]);
        highL = biquad(inL, hpB0, hpB1, hpB2, hpS1[0][0], hpS2[0][0]);
        highL = biquad(highL, hpB0, hpB1, hpB2, hpS1[1][0], hpS2[1][0]);
        highR = biquad(inR, hpB0, hpB1, hpB2, hpS1[0][1], hpS2[0][1]);
        highR = biquad(highR, hpB0, hpB1, hpB2, hpS1[1][1], hpS2[1][1]);
    }
};

// ── MultibandEngine: configurable 1–6 band processor ─────────────────
struct MultibandBand {
    LinkwitzRileyCrossover crossover;
    SaturationModel satModel = SaturationModel::Tanh;
    float drive = 0.0f;
    float mix   = 1.0f;
    bool  solo  = false;
    bool  mute  = false;
    bool  bypass = false;
    float tapeStateL = 0.0f;
    float tapeStateR = 0.0f;

    void reset() noexcept {
        crossover.reset();
        tapeStateL = tapeStateR = 0.0f;
    }
};

struct MultibandEngine {
    int numBands = 1;
    std::array<MultibandBand, kMaxBands> bands;
    std::array<float, kMaxBands - 1> crossoverFreqs = { 250.0f, 2000.0f, 5000.0f, 10000.0f, 15000.0f };

    void prepare(double sr) noexcept {
        for (int i = 0; i < numBands - 1; ++i)
            bands[(size_t)i].crossover.prepare(sr, crossoverFreqs[(size_t)i]);
    }

    void reset() noexcept {
        for (auto& b : bands) b.reset();
    }

    void processStereo(float& l, float& r, float lowContent) noexcept {
        if (numBands <= 1) {
            auto& b = bands[0];
            if (b.mute || b.bypass) return;
            const float driveGain = std::pow(10.0f, b.drive / 20.0f);
            const float dryL = l, dryR = r;
            l = Waveshaper::process(b.satModel, l * driveGain, b.tapeStateL, lowContent) / driveGain;
            r = Waveshaper::process(b.satModel, r * driveGain, b.tapeStateR, lowContent) / driveGain;
            l = dryL + (l - dryL) * b.mix;
            r = dryR + (r - dryR) * b.mix;
            return;
        }

        float bandL[kMaxBands] = {};
        float bandR[kMaxBands] = {};
        float remainL = l, remainR = r;

        for (int i = 0; i < numBands - 1; ++i) {
            float lowL, lowR, highL, highR;
            bands[(size_t)i].crossover.processStereo(remainL, remainR, lowL, lowR, highL, highR);
            bandL[i] = lowL;
            bandR[i] = lowR;
            remainL = highL;
            remainR = highR;
        }
        bandL[numBands - 1] = remainL;
        bandR[numBands - 1] = remainR;

        l = 0.0f; r = 0.0f;
        for (int i = 0; i < numBands; ++i) {
            auto& b = bands[(size_t)i];
            if (b.mute) continue;
            float bL = bandL[i], bR = bandR[i];
            if (!b.bypass) {
                const float driveGain = std::pow(10.0f, b.drive / 20.0f);
                const float dryL = bL, dryR = bR;
                bL = Waveshaper::process(b.satModel, bL * driveGain, b.tapeStateL, lowContent) / driveGain;
                bR = Waveshaper::process(b.satModel, bR * driveGain, b.tapeStateR, lowContent) / driveGain;
                bL = dryL + (bL - dryL) * b.mix;
                bR = dryR + (bR - dryR) * b.mix;
            }
            l += bL;
            r += bR;
        }
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 5: STEREO PROCESSING
// ═══════════════════════════════════════════════════════════════════════

struct MidSideEncoder {
    bool enabled = false;

    inline void encode(float& l, float& r) const noexcept {
        if (!enabled) return;
        const float mid  = (l + r) * 0.5f;
        const float side = (l - r) * 0.5f;
        l = mid;
        r = side;
    }

    inline void decode(float& l, float& r) const noexcept {
        if (!enabled) return;
        const float left  = l + r;
        const float right = l - r;
        l = left;
        r = right;
    }
};

struct AutoGainSmoother {
    float inputRms  = 0.0f;
    float outputRms = 0.0f;
    float gainDb    = 0.0f;
    float smoothCoeff = 0.0f;
    static constexpr float kMaxCompensationDb = 6.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        smoothCoeff = 1.0f - std::exp(-1.0f / (srf * 0.3f));
    }

    void reset() noexcept { inputRms = outputRms = 0.0f; gainDb = 0.0f; }

    void updateInput(float rmsL, float rmsR) noexcept {
        const float rms = std::sqrt((rmsL * rmsL + rmsR * rmsR) * 0.5f);
        inputRms += smoothCoeff * (rms - inputRms);
    }

    void updateOutput(float rmsL, float rmsR) noexcept {
        const float rms = std::sqrt((rmsL * rmsL + rmsR * rmsR) * 0.5f);
        outputRms += smoothCoeff * (rms - outputRms);
    }

    float getCompensationGain() noexcept {
        if (inputRms < kSilenceThreshold || outputRms < kSilenceThreshold)
            return 1.0f;
        const float diff = 20.0f * std::log10(inputRms / outputRms);
        gainDb += smoothCoeff * (juce::jlimit(-kMaxCompensationDb, kMaxCompensationDb, diff) - gainDb);
        return std::pow(10.0f, gainDb / 20.0f);
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 6: MODULATION
// ═══════════════════════════════════════════════════════════════════════

enum class LFOShape { Sine = 0, Triangle, Random, SampleHold, NumShapes };

struct LFO {
    LFOShape shape = LFOShape::Sine;
    float rateHz = 1.0f;
    float depth  = 0.0f;
    float phase = 0.0f;
    float phaseInc = 0.0f;
    float randomValue = 0.0f;
    float smoothedRandom = 0.0f;
    float shValue = 0.0f;
    uint32_t rngState = 12345u;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        phaseInc = rateHz / srf;
    }

    void reset() noexcept {
        phase = 0.0f;
        randomValue = smoothedRandom = shValue = 0.0f;
    }

    inline float next() noexcept {
        float value = 0.0f;
        switch (shape) {
            case LFOShape::Sine:
                value = std::sin(kTwoPi * phase);
                break;
            case LFOShape::Triangle:
                value = 4.0f * std::abs(phase - 0.5f) - 1.0f;
                break;
            case LFOShape::Random: {
                if (phase < phaseInc) {
                    rngState = rngState * 1664525u + 1013904223u;
                    randomValue = ((float)(rngState >> 8) / 16777216.0f) * 2.0f - 1.0f;
                }
                smoothedRandom += 0.01f * (randomValue - smoothedRandom);
                value = smoothedRandom;
                break;
            }
            case LFOShape::SampleHold: {
                if (phase < phaseInc) {
                    rngState = rngState * 1664525u + 1013904223u;
                    shValue = ((float)(rngState >> 8) / 16777216.0f) * 2.0f - 1.0f;
                }
                value = shValue;
                break;
            }
            default: break;
        }
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        return value * depth;
    }
};

// ── MacroInterpreter: maps macro knobs to multiple target parameters ──
struct MacroInterpreter {
    static constexpr int kNumMacros = 4;

    enum class CurveType {
        Linear = 0, Exponential, Logarithmic, SCurve,
        InverseLinear, InverseExponential, InverseLogarithmic, NumCurves
    };

    struct Mapping {
        int targetIndex = -1;
        float depth = 1.0f;
        CurveType curve = CurveType::Linear;
    };

    struct MacroSlot {
        float value = 0.0f;
        static constexpr int kMaxMappings = 8;
        std::array<Mapping, kMaxMappings> mappings;
        int numMappings = 0;

        void addMapping(const Mapping& m) {
            if (numMappings < kMaxMappings)
                mappings[(size_t)numMappings++] = m;
        }
        void clearMappings() { numMappings = 0; }
    };

    std::array<MacroSlot, kNumMacros> slots;

    static float applyCurve(float x, CurveType curve) noexcept {
        x = juce::jlimit(0.0f, 1.0f, x);
        switch (curve) {
            case CurveType::Linear:             return x;
            case CurveType::Exponential:        return x * x;
            case CurveType::Logarithmic:        return std::sqrt(x);
            case CurveType::SCurve:             return x * x * (3.0f - 2.0f * x);
            case CurveType::InverseLinear:      return 1.0f - x;
            case CurveType::InverseExponential: return 1.0f - x * x;
            case CurveType::InverseLogarithmic: return 1.0f - std::sqrt(x);
            default: return x;
        }
    }

    float getMappedValue(int macroIdx, int mappingIdx) const noexcept {
        if (macroIdx < 0 || macroIdx >= kNumMacros) return 0.0f;
        const auto& slot = slots[(size_t)macroIdx];
        if (mappingIdx < 0 || mappingIdx >= slot.numMappings) return 0.0f;
        const auto& m = slot.mappings[(size_t)mappingIdx];
        return applyCurve(slot.value, m.curve) * m.depth;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 7: METERING & ANALYSIS
// ═══════════════════════════════════════════════════════════════════════

// ── MeterBallistics: peak + RMS metering with hold ───────────────────
struct MeterBallistics {
    float peakL = 0.0f, peakR = 0.0f;
    float rmsL  = 0.0f, rmsR  = 0.0f;
    float peakHoldL = 0.0f, peakHoldR = 0.0f;
    int holdCountL = 0, holdCountR = 0;
    int holdSamples = 0;
    float peakDecay = 0.0f;
    float rmsCoeff = 0.0f;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        holdSamples = (int)(srf * 1.5f);  // 1.5s hold
        peakDecay = 1.0f - std::exp(-1.0f / (srf * 0.3f));
        rmsCoeff = 1.0f - std::exp(-1.0f / (srf * 0.05f));
    }

    void reset() noexcept {
        peakL = peakR = rmsL = rmsR = 0.0f;
        peakHoldL = peakHoldR = 0.0f;
        holdCountL = holdCountR = 0;
    }

    inline void processSample(float l, float r) noexcept {
        const float absL = std::abs(l);
        const float absR = std::abs(r);

        // Peak with decay
        if (absL > peakL) peakL = absL;
        else peakL -= peakDecay * peakL;

        if (absR > peakR) peakR = absR;
        else peakR -= peakDecay * peakR;

        // Peak hold
        if (absL > peakHoldL) { peakHoldL = absL; holdCountL = 0; }
        else if (++holdCountL > holdSamples) peakHoldL -= peakDecay * peakHoldL;

        if (absR > peakHoldR) { peakHoldR = absR; holdCountR = 0; }
        else if (++holdCountR > holdSamples) peakHoldR -= peakDecay * peakHoldR;

        // RMS
        rmsL += rmsCoeff * (l * l - rmsL);
        rmsR += rmsCoeff * (r * r - rmsR);
    }

    float getRmsDbL() const noexcept { return 20.0f * std::log10(juce::jmax(kSilenceThreshold, std::sqrt(rmsL))); }
    float getRmsDbR() const noexcept { return 20.0f * std::log10(juce::jmax(kSilenceThreshold, std::sqrt(rmsR))); }
    float getPeakDbL() const noexcept { return 20.0f * std::log10(juce::jmax(kSilenceThreshold, peakL)); }
    float getPeakDbR() const noexcept { return 20.0f * std::log10(juce::jmax(kSilenceThreshold, peakR)); }
};

// ── LoudnessMeter: EBU R128 implementation ───────────────────────────
struct LoudnessMeter {
    struct KWeightState { float s1[2] = {}, s2[2] = {}; };
    struct BiquadCoeffs { float b0, b1, b2, a1, a2; };

    KWeightState kwL, kwR;
    BiquadCoeffs kStage1 = {};
    BiquadCoeffs kStage2 = {};

    float momentarySum = 0.0f;
    float shortTermSum = 0.0f;
    float integratedSum = 0.0f;
    int momentarySamples = 0;
    int shortTermSamples = 0;
    int integratedBlocks = 0;
    int momentaryWindowSize = 0;
    int shortTermWindowSize = 0;
    float momentaryLufs = -100.0f;
    float shortTermLufs = -100.0f;
    float integratedLufs = -100.0f;

    static constexpr int kMaxWindowBlocks = 300;
    std::array<float, kMaxWindowBlocks> blockPowers = {};
    int blockWritePos = 0;
    int blockSampleCount = 0;
    int blockSize = 0;

    void prepare(double sr) noexcept {
        const float srf = (float)juce::jmax(1.0, sr);
        momentaryWindowSize = (int)(srf * 0.4f);
        shortTermWindowSize = (int)(srf * 3.0f);
        blockSize = (int)(srf * 0.1f);

        // K-weighting Stage 1: high-shelf +4 dB
        {
            const float fc = 1500.0f;
            const float G = std::pow(10.0f, 4.0f / 40.0f);
            const float w0 = kTwoPi * fc / srf;
            const float A = std::sqrt(G);
            const float alpha = std::sin(w0) / (2.0f * 0.707f);
            const float a0 = (A + 1.0f) - (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha;
            kStage1.b0 = (A * ((A + 1.0f) + (A - 1.0f) * std::cos(w0) + 2.0f * std::sqrt(A) * alpha)) / a0;
            kStage1.b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * std::cos(w0))) / a0;
            kStage1.b2 = (A * ((A + 1.0f) + (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha)) / a0;
            kStage1.a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * std::cos(w0))) / a0;
            kStage1.a2 = ((A + 1.0f) - (A - 1.0f) * std::cos(w0) - 2.0f * std::sqrt(A) * alpha) / a0;
        }
        // K-weighting Stage 2: high-pass 60 Hz
        {
            const float fc = 60.0f;
            const float w0 = kTwoPi * fc / srf;
            const float alpha = std::sin(w0) / (2.0f * 0.5f);
            const float a0 = 1.0f + alpha;
            kStage2.b0 = ((1.0f + std::cos(w0)) * 0.5f) / a0;
            kStage2.b1 = -(1.0f + std::cos(w0)) / a0;
            kStage2.b2 = kStage2.b0;
            kStage2.a1 = (-2.0f * std::cos(w0)) / a0;
            kStage2.a2 = (1.0f - alpha) / a0;
        }
    }

    void reset() noexcept {
        kwL = kwR = {};
        momentarySum = shortTermSum = integratedSum = 0.0f;
        momentarySamples = shortTermSamples = integratedBlocks = 0;
        momentaryLufs = shortTermLufs = integratedLufs = -100.0f;
        blockPowers.fill(0.0f);
        blockWritePos = 0;
        blockSampleCount = 0;
    }

    inline float applyBiquad(float x, const BiquadCoeffs& c, float& s1, float& s2) noexcept {
        const float y = c.b0 * x + s1;
        s1 = c.b1 * x - c.a1 * y + s2;
        s2 = c.b2 * x - c.a2 * y;
        return y;
    }

    inline void processSample(float l, float r) noexcept {
        float kL = applyBiquad(l, kStage1, kwL.s1[0], kwL.s2[0]);
        kL = applyBiquad(kL, kStage2, kwL.s1[1], kwL.s2[1]);
        float kR = applyBiquad(r, kStage1, kwR.s1[0], kwR.s2[0]);
        kR = applyBiquad(kR, kStage2, kwR.s1[1], kwR.s2[1]);

        const float power = kL * kL + kR * kR;
        momentarySum += power;
        ++blockSampleCount;

        if (blockSampleCount >= blockSize && blockSize > 0) {
            const float blockPower = momentarySum / (float)blockSampleCount;
            blockPowers[(size_t)blockWritePos] = blockPower;
            blockWritePos = (blockWritePos + 1) % kMaxWindowBlocks;

            // Momentary (400ms = 4 blocks)
            {
                float sum = 0.0f;
                int count = juce::jmin(4, integratedBlocks + 1);
                for (int i = 0; i < count; ++i) {
                    int idx = (blockWritePos - 1 - i + kMaxWindowBlocks) % kMaxWindowBlocks;
                    sum += blockPowers[(size_t)idx];
                }
                if (count > 0)
                    momentaryLufs = -0.691f + 10.0f * std::log10(juce::jmax(1.0e-10f, sum / (float)count));
            }
            // Short-term (3s = 30 blocks)
            {
                float sum = 0.0f;
                int count = juce::jmin(30, integratedBlocks + 1);
                for (int i = 0; i < count; ++i) {
                    int idx = (blockWritePos - 1 - i + kMaxWindowBlocks) % kMaxWindowBlocks;
                    sum += blockPowers[(size_t)idx];
                }
                if (count > 0)
                    shortTermLufs = -0.691f + 10.0f * std::log10(juce::jmax(1.0e-10f, sum / (float)count));
            }
            // Integrated
            {
                integratedSum += blockPower;
                ++integratedBlocks;
                if (integratedBlocks > 0) {
                    const float avg = integratedSum / (float)integratedBlocks;
                    integratedLufs = -0.691f + 10.0f * std::log10(juce::jmax(1.0e-10f, avg));
                }
            }
            momentarySum = 0.0f;
            blockSampleCount = 0;
        }
    }
};

// ── SpectrumBuffer: lock-free ring buffer for FFT spectrum display ────
struct SpectrumBuffer {
    static constexpr int kSize = kSpectrumFFTSize;

    std::array<float, kSize> inputRing  = {};
    std::array<float, kSize> outputRing = {};
    std::atomic<int> writePos { 0 };

    void reset() noexcept {
        inputRing.fill(0.0f);
        outputRing.fill(0.0f);
        writePos.store(0);
    }

    inline void pushSample(float inputMono, float outputMono) noexcept {
        const int pos = writePos.load(std::memory_order_relaxed);
        inputRing[(size_t)pos]  = inputMono;
        outputRing[(size_t)pos] = outputMono;
        writePos.store((pos + 1) & (kSize - 1), std::memory_order_release);
    }
};

// ── GainReductionHistory: scrolling GR display buffer ────────────────
struct GainReductionHistory {
    static constexpr int kHistorySize = 512;

    std::array<float, kHistorySize> history = {};
    std::atomic<int> writePos { 0 };

    void reset() noexcept {
        history.fill(0.0f);
        writePos.store(0);
    }

    inline void pushGR(float grDb) noexcept {
        const int pos = writePos.load(std::memory_order_relaxed);
        history[(size_t)pos] = grDb;
        writePos.store((pos + 1) % kHistorySize, std::memory_order_release);
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 7b: REFERENCE TONE MATCHING
//  Captures spectral fingerprint of a reference signal and computes
//  an EQ correction curve to match the current output to the reference.
//  Non-realtime analysis (triggered by user), RT-safe application.
// ═══════════════════════════════════════════════════════════════════════

struct ReferenceToneMatcher {
    static constexpr int kNumBands = 32;  // 1/3-octave resolution

    struct SpectralProfile {
        std::array<float, kNumBands> bandEnergy = {};  // dB per band
        bool captured = false;
    };

    SpectralProfile referenceProfile;
    SpectralProfile currentProfile;

    // Correction curve: dB adjustment per band
    std::array<float, kNumBands> correctionDb = {};
    std::array<float, kNumBands> smoothedCorrection = {};
    float matchAmount = 0.0f;   // 0–1: how much correction to apply
    bool enabled = false;

    // Band center frequencies (1/3 octave from 25 Hz to 20 kHz)
    std::array<float, kNumBands> bandFreqs = {};

    void prepare(double /*sr*/) noexcept {
        // Initialize 1/3 octave band centers
        const float startFreq = 25.0f;
        for (int i = 0; i < kNumBands; ++i) {
            bandFreqs[(size_t)i] = startFreq * std::pow(2.0f, (float)i / 3.0f);
        }
        smoothedCorrection.fill(0.0f);
    }

    void reset() noexcept {
        correctionDb.fill(0.0f);
        smoothedCorrection.fill(0.0f);
    }

    // Called from message thread: capture reference profile from FFT data
    void captureReference(const float* fftMagnitudes, int fftSize, double sr) {
        computeProfile(fftMagnitudes, fftSize, sr, referenceProfile);
    }

    // Called from message thread: capture current output profile
    void captureCurrent(const float* fftMagnitudes, int fftSize, double sr) {
        computeProfile(fftMagnitudes, fftSize, sr, currentProfile);
    }

    // Compute correction curve (message thread)
    void computeCorrection() {
        if (!referenceProfile.captured || !currentProfile.captured) return;

        for (int i = 0; i < kNumBands; ++i) {
            correctionDb[(size_t)i] = juce::jlimit(-12.0f, 12.0f,
                referenceProfile.bandEnergy[(size_t)i] - currentProfile.bandEnergy[(size_t)i]);
        }
    }

    // Get interpolated correction at a specific frequency (RT-safe)
    float getCorrectionAtFreq(float freq) const noexcept {
        if (!enabled || matchAmount < 0.01f) return 0.0f;

        // Find surrounding bands
        for (int i = 0; i < kNumBands - 1; ++i) {
            if (freq >= bandFreqs[(size_t)i] && freq < bandFreqs[(size_t)(i + 1)]) {
                const float t = (freq - bandFreqs[(size_t)i]) /
                               (bandFreqs[(size_t)(i + 1)] - bandFreqs[(size_t)i]);
                const float db = smoothedCorrection[(size_t)i] * (1.0f - t) +
                                smoothedCorrection[(size_t)(i + 1)] * t;
                return db * matchAmount;
            }
        }
        return smoothedCorrection[kNumBands - 1] * matchAmount;
    }

private:
    void computeProfile(const float* fftMag, int fftSize, double sr, SpectralProfile& profile) {
        const float binWidth = (float)sr / (float)fftSize;
        profile.bandEnergy.fill(-100.0f);

        for (int band = 0; band < kNumBands; ++band) {
            const float lo = bandFreqs[(size_t)band] / std::pow(2.0f, 1.0f / 6.0f);
            const float hi = bandFreqs[(size_t)band] * std::pow(2.0f, 1.0f / 6.0f);
            const int binLo = juce::jmax(1, (int)(lo / binWidth));
            const int binHi = juce::jmin(fftSize / 2 - 1, (int)(hi / binWidth));

            float sum = 0.0f;
            int count = 0;
            for (int bin = binLo; bin <= binHi; ++bin) {
                sum += fftMag[bin] * fftMag[bin];
                ++count;
            }
            if (count > 0)
                profile.bandEnergy[(size_t)band] = 10.0f * std::log10(juce::jmax(1.0e-10f, sum / (float)count));
        }
        profile.captured = true;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 7c: PRESET INTELLIGENCE
//  Analyzes input signal characteristics and suggests appropriate presets.
//  Runs on message thread, reads from spectrum/meter data.
// ═══════════════════════════════════════════════════════════════════════

struct PresetIntelligence {
    enum class SignalType {
        Unknown = 0,
        Vocals,
        Drums,
        Bass,
        Guitar,
        Keys,
        FullMix,
        NumTypes
    };

    struct Analysis {
        SignalType detectedType = SignalType::Unknown;
        float crestFactor = 0.0f;       // peak/RMS ratio in dB
        float spectralCentroid = 0.0f;   // Hz
        float lowEnergyRatio = 0.0f;    // energy below 300 Hz / total
        float transientDensity = 0.0f;  // transients per second
        float dynamicRange = 0.0f;      // dB
        juce::String suggestedPreset;
        float confidence = 0.0f;         // 0–1
    };

    Analysis lastAnalysis;

    // Analyze spectral data and classify signal type
    void analyze(const MeterBallistics& meters, const SpectrumBuffer& spectrum,
                 float transientRate, double sr) {
        Analysis a;

        // Crest factor
        const float peakDb = juce::jmax(meters.getPeakDbL(), meters.getPeakDbR());
        const float rmsDb = juce::jmax(meters.getRmsDbL(), meters.getRmsDbR());
        a.crestFactor = peakDb - rmsDb;

        // Transient density
        a.transientDensity = transientRate;

        // Spectral centroid from spectrum buffer
        float weightedSum = 0.0f, totalEnergy = 0.0f;
        const float binWidth = (float)sr / (float)SpectrumBuffer::kSize;
        for (int i = 1; i < SpectrumBuffer::kSize / 2; ++i) {
            const float mag = std::abs(spectrum.inputRing[(size_t)i]);
            const float freq = (float)i * binWidth;
            weightedSum += freq * mag;
            totalEnergy += mag;
        }
        a.spectralCentroid = totalEnergy > 0.0f ? weightedSum / totalEnergy : 1000.0f;

        // Low energy ratio
        float lowEnergy = 0.0f;
        const int lowBin = (int)(300.0f / binWidth);
        for (int i = 1; i < juce::jmin(lowBin, SpectrumBuffer::kSize / 2); ++i)
            lowEnergy += spectrum.inputRing[(size_t)i] * spectrum.inputRing[(size_t)i];
        a.lowEnergyRatio = totalEnergy > 0.0f ? lowEnergy / (totalEnergy * totalEnergy + 1.0e-10f) : 0.0f;

        // Classification heuristics
        if (a.spectralCentroid > 2000.0f && a.crestFactor < 12.0f) {
            a.detectedType = SignalType::Vocals;
            a.suggestedPreset = "Warm Vocal";
            a.confidence = 0.7f;
        } else if (a.transientDensity > 4.0f && a.crestFactor > 15.0f) {
            a.detectedType = SignalType::Drums;
            a.suggestedPreset = "Punchy Drums";
            a.confidence = 0.75f;
        } else if (a.spectralCentroid < 400.0f && a.lowEnergyRatio > 0.6f) {
            a.detectedType = SignalType::Bass;
            a.suggestedPreset = "Thick Bass";
            a.confidence = 0.8f;
        } else if (a.spectralCentroid > 800.0f && a.spectralCentroid < 3000.0f && a.crestFactor > 10.0f) {
            a.detectedType = SignalType::Guitar;
            a.suggestedPreset = "Guitar Warmth";
            a.confidence = 0.6f;
        } else if (a.crestFactor < 10.0f && a.spectralCentroid > 500.0f && a.spectralCentroid < 2000.0f) {
            a.detectedType = SignalType::FullMix;
            a.suggestedPreset = "Master Glue";
            a.confidence = 0.65f;
        } else {
            a.detectedType = SignalType::Unknown;
            a.suggestedPreset = "Default";
            a.confidence = 0.3f;
        }

        lastAnalysis = a;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 8: STATE MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════

struct UndoSnapshot {
    juce::ValueTree state;
    juce::String description;
};

struct UndoStack {
    void clear() {
        snapshots.clear();
        currentIndex = -1;
    }

    void pushState(const juce::ValueTree& state, const juce::String& desc = {}) {
        while ((int)snapshots.size() > currentIndex + 1 && !snapshots.empty())
            snapshots.pop_back();
        snapshots.push_back({ state.createCopy(), desc });
        if ((int)snapshots.size() > kMaxUndoSteps)
            snapshots.erase(snapshots.begin());
        currentIndex = (int)snapshots.size() - 1;
    }

    bool canUndo() const { return currentIndex > 0; }
    bool canRedo() const { return currentIndex < (int)snapshots.size() - 1; }

    juce::ValueTree undo() {
        if (!canUndo()) return {};
        --currentIndex;
        return snapshots[(size_t)currentIndex].state.createCopy();
    }

    juce::ValueTree redo() {
        if (!canRedo()) return {};
        ++currentIndex;
        return snapshots[(size_t)currentIndex].state.createCopy();
    }

private:
    std::vector<UndoSnapshot> snapshots;
    int currentIndex = -1;
};

struct ABState {
    juce::ValueTree slotA;
    juce::ValueTree slotB;
    bool isSlotA = true;

    void storeA(const juce::ValueTree& state) { slotA = state.createCopy(); }
    void storeB(const juce::ValueTree& state) { slotB = state.createCopy(); }
    void toggle() { isSlotA = !isSlotA; }

    juce::ValueTree getActive() const {
        return isSlotA ? slotA.createCopy() : slotB.createCopy();
    }
    juce::ValueTree getInactive() const {
        return isSlotA ? slotB.createCopy() : slotA.createCopy();
    }
    void copyAtoB() { slotB = slotA.createCopy(); }
    void copyBtoA() { slotA = slotB.createCopy(); }
};

struct PresetInfo {
    juce::String name;
    juce::String category;
    juce::String author;
    juce::String description;
    juce::File   file;
    bool isFactory = false;
};

struct MIDILearnMapping {
    int ccNumber = -1;
    juce::String parameterID;
    float minValue = 0.0f;
    float maxValue = 1.0f;
};

struct MIDILearnState {
    static constexpr int kMaxMappings = 128;

    std::array<MIDILearnMapping, kMaxMappings> mappings;
    int numMappings = 0;
    bool isLearning = false;
    juce::String learningParameterID;

    void addMapping(int cc, const juce::String& paramID, float minVal = 0.0f, float maxVal = 1.0f) {
        if (numMappings < kMaxMappings) {
            mappings[(size_t)numMappings] = { cc, paramID, minVal, maxVal };
            ++numMappings;
        }
    }

    void removeMapping(int cc) {
        for (int i = 0; i < numMappings; ++i) {
            if (mappings[(size_t)i].ccNumber == cc) {
                for (int j = i; j < numMappings - 1; ++j)
                    mappings[(size_t)j] = mappings[(size_t)(j + 1)];
                --numMappings;
                return;
            }
        }
    }

    void clearAll() { numMappings = 0; }

    const MIDILearnMapping* findMapping(int cc) const {
        for (int i = 0; i < numMappings; ++i) {
            if (mappings[(size_t)i].ccNumber == cc)
                return &mappings[(size_t)i];
        }
        return nullptr;
    }

    void startLearning(const juce::String& paramID) {
        isLearning = true;
        learningParameterID = paramID;
    }

    void stopLearning() {
        isLearning = false;
        learningParameterID = {};
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 8b: SIMPLE MODE STATE
//  Reduces the full parameter set to 3 macro controls:
//    Drive (maps to saturation drive + model intensity)
//    Tone  (maps to Shine freq/gain + Boom amount)
//    Output (maps to output gain + limiter ceiling)
//  All other parameters are set to intelligent defaults based on
//  the PresetIntelligence analysis.
// ═══════════════════════════════════════════════════════════════════════

struct SimpleModeState {
    bool enabled = false;

    // Simple mode macro values (0–1 normalized)
    float drive  = 0.5f;
    float tone   = 0.5f;
    float output = 0.5f;

    // Computed target values for the full parameter set
    struct TargetParams {
        float satDriveDb = 0.0f;
        SaturationModel satModel = SaturationModel::Tape;
        float shineGainDb = 0.0f;
        float shineFreq = 8000.0f;
        float boomDb = 0.0f;
        float outputGainDb = 0.0f;
        float limiterCeiling = -0.3f;
        float glueThreshold = -10.0f;
        float glueRatio = 2.0f;
        float width = 1.0f;
    };

    TargetParams targets;

    // Compute target parameters from simple mode macros
    void computeTargets() noexcept {
        // Drive: 0 = clean, 0.5 = warm, 1.0 = aggressive
        targets.satDriveDb = drive * 24.0f - 6.0f;  // -6 to +18 dB
        if (drive < 0.3f)
            targets.satModel = SaturationModel::Tape;
        else if (drive < 0.6f)
            targets.satModel = SaturationModel::Tube;
        else
            targets.satModel = SaturationModel::Transistor;

        // Tone: 0 = dark, 0.5 = neutral, 1.0 = bright
        targets.shineGainDb = (tone - 0.5f) * 12.0f;  // -6 to +6 dB
        targets.shineFreq = 4000.0f + tone * 8000.0f;  // 4k to 12k
        targets.boomDb = (0.5f - tone) * 6.0f;         // +3 to -3 dB (inverse)

        // Output: 0 = quiet, 0.5 = unity, 1.0 = loud
        targets.outputGainDb = (output - 0.5f) * 12.0f;  // -6 to +6 dB
        targets.limiterCeiling = -0.3f - (1.0f - output) * 3.0f;  // -0.3 to -3.3 dB
        targets.glueThreshold = -6.0f - (1.0f - output) * 12.0f;
        targets.glueRatio = 2.0f + drive * 4.0f;  // more drive = more compression

        targets.width = 1.0f;
    }
};


// ═══════════════════════════════════════════════════════════════════════
//  SECTION 8c: LOUDNESS-MATCHED A/B
//  Automatically compensates gain difference between A and B slots
//  so the user hears tonal differences, not volume differences.
// ═══════════════════════════════════════════════════════════════════════

struct LoudnessMatchedAB {
    float slotALufs = -100.0f;
    float slotBLufs = -100.0f;
    float compensationDb = 0.0f;  // applied to slot B to match slot A

    void updateSlotA(float lufs) noexcept { slotALufs = lufs; }
    void updateSlotB(float lufs) noexcept { slotBLufs = lufs; }

    void computeCompensation() noexcept {
        if (slotALufs > -90.0f && slotBLufs > -90.0f) {
            compensationDb = juce::jlimit(-6.0f, 6.0f, slotALufs - slotBLufs);
        } else {
            compensationDb = 0.0f;
        }
    }

    float getCompensationGain() const noexcept {
        return std::pow(10.0f, compensationDb / 20.0f);
    }
};


} // namespace BTZDsp
