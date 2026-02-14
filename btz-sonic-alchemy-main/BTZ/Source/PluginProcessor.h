/*
  Box Tone Zone (BTZ) - PluginProcessor.h
*/
#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>

struct BTZMeterState {
    std::atomic<float> inputPeakL { -100.0f };
    std::atomic<float> inputPeakR { -100.0f };
    std::atomic<float> inputRmsL  { -100.0f };
    std::atomic<float> inputRmsR  { -100.0f };
    std::atomic<float> outputPeakL { -100.0f };
    std::atomic<float> outputPeakR { -100.0f };
    std::atomic<float> outputRmsL  { -100.0f };
    std::atomic<float> outputRmsR  { -100.0f };
    std::atomic<float> sparkGainReductionDb { 0.0f };
    std::atomic<float> lufs { -24.0f };
    std::atomic<float> inputClip { 0.0f };
    std::atomic<float> outputClip { 0.0f };
    std::atomic<float> correlation { 1.0f };
};

struct SlewLimiter {
    float prev = 0.0f;
    float maxDelta = 0.02f;

    void setSampleRate(double sr) noexcept {
        constexpr float kSlewReferenceRate = 48000.0f;
        constexpr float kSlewMaxDeltaBase = 0.02f;
        maxDelta = kSlewMaxDeltaBase * (kSlewReferenceRate / static_cast<float>(juce::jmax(1.0, sr)));
    }

    void reset() noexcept { prev = 0.0f; }

    float process(float x) noexcept {
        const float delta = x - prev;
        if (std::abs(delta) > maxDelta)
            x = prev + (delta > 0.0f ? maxDelta : -maxDelta);
        prev = x;
        return x;
    }
};

struct EnvFollower {
    float env = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    void setTimes(float attackMs, float releaseMs, double sr) noexcept {
        constexpr float kMinAttackMs = 0.01f;
        constexpr float kMsToSeconds = 0.001f;
        const float srf = static_cast<float>(juce::jmax(1.0, sr));
        attackCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(kMinAttackMs, attackMs) * kMsToSeconds));
        releaseCoeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(kMinAttackMs, releaseMs) * kMsToSeconds));
    }

    void reset(float value = 0.0f) noexcept { env = value; }

    float process(float xAbs) noexcept {
        const float coeff = xAbs > env ? attackCoeff : releaseCoeff;
        env += coeff * (xAbs - env);
        return env;
    }
};

struct SafetyLayer {
    float dcL = 0.0f, dcPrevL = 0.0f;
    float dcR = 0.0f, dcPrevR = 0.0f;
    float dcCoeff = 0.9999f;

    void setSampleRate(double sr) noexcept {
        constexpr float kTwoPi = 6.2831853f;
        constexpr float kDcBlockerFreq = 5.0f;
        constexpr float kMinCoeff = 0.90f;
        constexpr float kMaxCoeff = 0.99999f;
        const float srf = static_cast<float>(juce::jmax(1.0, sr));
        dcCoeff = 1.0f - (kTwoPi * kDcBlockerFreq / srf);
        dcCoeff = juce::jlimit(kMinCoeff, kMaxCoeff, dcCoeff);
    }

    void reset() noexcept { dcL = dcPrevL = dcR = dcPrevR = 0.0f; }

    float processSample(float x, float& dc, float& dcPrev) noexcept {
        constexpr float kDenormalThreshold = 1.0e-20f;
        if (!std::isfinite(x) || std::abs(x) < kDenormalThreshold)
            x = 0.0f;
        const float y = x - dcPrev + dcCoeff * dc;
        dcPrev = x;
        dc = y;
        return y;
    }
};

struct SmoothParam {
    float current = 0.0f;
    float target = 0.0f;
    float coeff = 0.001f;

    void setTime(float ms, double sr) noexcept {
        constexpr float kMinTimeMs = 0.01f;
        constexpr float kMsToSeconds = 0.001f;
        const float srf = static_cast<float>(juce::jmax(1.0, sr));
        coeff = 1.0f - std::exp(-1.0f / (srf * juce::jmax(kMinTimeMs, ms) * kMsToSeconds));
    }

    void setTarget(float v) noexcept { target = v; }
    float next() noexcept { current += coeff * (target - current); return current; }
    void snapTo(float v) noexcept { current = target = v; }
};

class BTZAudioProcessor : public juce::AudioProcessor {
public:
    BTZAudioProcessor();
    ~BTZAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Box Tone Zone (BTZ)"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    BTZMeterState& getMeters() { return meters; }

private:
    struct MeterBallistics {
        float inPeakHoldL = 0.0f, inPeakHoldR = 0.0f;
        float outPeakHoldL = 0.0f, outPeakHoldR = 0.0f;
        float inRmsL = 0.0f, inRmsR = 0.0f;
        float outRmsL = 0.0f, outRmsR = 0.0f;
        float sparkGR = 0.0f;
        float clipHoldIn = 0.0f, clipHoldOut = 0.0f;
        float holdDecay = 0.995f;
        float rmsCoeff = 0.08f;
    };

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    BTZMeterState meters;
    MeterBallistics meterBallistics;

    SmoothParam sPunch, sWarmth, sBoom, sGlue, sAir, sWidth;
    SmoothParam sDensity, sMotion, sEra, sMix, sDrive;
    SmoothParam sMaster, sSparkCeil, sSparkMix, sShine, sShineMix;

    SafetyLayer safetyPre, safetyPost;
    SlewLimiter slewL, slewR;
    EnvFollower peakEnvL, peakEnvR, rmsEnvL, rmsEnvR;
    EnvFollower glueEnv;

    float glueGain = 1.0f;
    float xoverLowL = 0.0f, xoverLowR = 0.0f, xoverCoeff = 0.0f;
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f, sideLowCoeff = 0.0f;
    float sparkGrEnvelope = 0.0f;
    float sparkAttackCoeff = 0.2f, sparkReleaseCoeff = 0.01f;

    double currentSampleRate = 44100.0;
    int maxPreparedBlockSize = 0;
    uint32_t noiseSeed = 12345u;

    juce::AudioBuffer<float> dryBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> os2x;
    std::unique_ptr<juce::dsp::Oversampling<float>> os4x;
    int activeQualityMode = 1;

    void initSmoothers(double sampleRate);
    void updateTargetsFromAPVTS();
    void processCore(float* dataL, float* dataR, int numSamples, float osFactor);
    void updateMeters(const float* inL, const float* inR, const float* outL, const float* outR, int n, float sparkGRDb);
    int getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};

