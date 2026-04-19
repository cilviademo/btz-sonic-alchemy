/*
  Box Tone Zone (BTZ) — PluginProcessor.h  v7
  ────────────────────────────────────────────────────────────────────────
  v7 (release-gate hardening):
    • Click-free bypass via BypassCrossfader (64-sample cosine ramp)
    • Full resetAll() method — transport stop/start safe
    • releaseResources() guarded — no deallocation of dryBuffer
    • OS objects created once, not recreated on every prepareToPlay
    • State migration with version validation (v4→v7 compat)
    • Silence-in-silence-out detection
    • glueScHpf crossfade on mode change (via SidechainHPF v7)
*/
#pragma once

#include <JuceHeader.h>
#include "BTZDsp.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>

// ═══════════════════════════════════════════════════════════════════════════
// Meter state — atomic bridge from audio thread to UI thread
// ═══════════════════════════════════════════════════════════════════════════
struct BTZMeterState {
    std::atomic<float> inputPeakL  { -100.0f };
    std::atomic<float> inputPeakR  { -100.0f };
    std::atomic<float> inputRmsL   { -100.0f };
    std::atomic<float> inputRmsR   { -100.0f };
    std::atomic<float> outputPeakL { -100.0f };
    std::atomic<float> outputPeakR { -100.0f };
    std::atomic<float> outputRmsL  { -100.0f };
    std::atomic<float> outputRmsR  { -100.0f };
    std::atomic<float> sparkGainReductionDb { 0.0f };
    std::atomic<float> lufs        { -24.0f };
    std::atomic<float> inputClip   { 0.0f };
    std::atomic<float> outputClip  { 0.0f };
    std::atomic<float> correlation { 1.0f };
};

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessor — main plugin processor
// ═══════════════════════════════════════════════════════════════════════════
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
    // ── Parameter layout ──
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Meter state ──
    BTZMeterState meters;
    BTZDsp::MeterBallistics meterBallistics;

    // ── Parameter smoothers ──
    BTZDsp::SmoothParam sPunch, sWarmth, sBoom, sGlue, sAir, sWidth;
    BTZDsp::SmoothParam sDensity, sMotion, sEra, sMix, sDrive, sMaster;
    BTZDsp::SmoothParam sShine, sShineMix, sShineFreq, sShineQ;
    BTZDsp::SmoothParam sMacro0, sMacro1, sMacro2, sMacro3;

    // ── ADAA saturators — one instance per channel per saturation stage ──
    BTZDsp::ADAATanh adaaPreampL, adaaPreampR;
    BTZDsp::ADAATanh adaaBandLowL, adaaBandLowR;
    BTZDsp::ADAATanh adaaBandHighL, adaaBandHighR;
    BTZDsp::ADAATanh adaaPunchOddL, adaaPunchOddR;
    BTZDsp::ADAATanh adaaPunchEvenL, adaaPunchEvenR;
    BTZDsp::ADAATanh adaaDensityL, adaaDensityR;

    // ── DSP modules ──
    BTZDsp::SafetyLayer safetyPre, safetyPost;
    BTZDsp::SlewLimiter slewL, slewR;
    BTZDsp::EnvFollower peakEnvL, peakEnvR, rmsEnvL, rmsEnvR;
    BTZDsp::EnvFollower glueEnv;
    BTZDsp::SidechainHPF glueScHpf;
    BTZDsp::GlueCompressor glueComp;
    BTZDsp::LinkwitzRileyCrossover crossover;
    BTZDsp::TruePeakLimiter truePeakLimiter;
    BTZDsp::ShineProcessor shineProcessor;
    BTZDsp::AutoGainSmoother autoGainSmoother;
    BTZDsp::MacroInterpreter macroInterpreter;

    // v7: Click-free bypass crossfader
    BTZDsp::BypassCrossfader bypassCrossfader;

    // ── DSP state ──
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f, sideLowCoeff = 0.0f;

    // v4: cached drive gain to avoid per-sample std::pow
    float cachedDriveGain = 1.0f;
    float lastDriveDb = 0.0f;

    // v5: crest ratio computed at base SR, held for processNonlinear
    float lastCrestRatio = 3.0f;

    // v6: macro value array for MacroInterpreter
    float macroValues[BTZDsp::MacroInterpreter::kNumMacros] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // v6: cached glue SC HPF frequency
    float lastGlueScHpfFreq = 60.0f;
    double glueScHpfSampleRate = 44100.0;

    // v7: silence detection — skip processing when input is silent
    int silentFrameCount = 0;
    static constexpr int kSilentFrameThreshold = 512;  // ~12ms at 44.1kHz
    static constexpr float kSilenceThreshold = 1.0e-8f;

    // v7: track whether prepareToPlay has been called (guard processBlock)
    bool prepared = false;

    // v7: track last prepared SR/blockSize to avoid OS recreation
    double lastPreparedSR = 0.0;
    int lastPreparedBlockSize = 0;

    double currentSampleRate = 44100.0;
    int maxPreparedBlockSize = 0;
    int currentBlockSize = 512;
    uint32_t noiseSeed = 12345u;

    // ── Dry buffer and oversampling ──
    juce::AudioBuffer<float> dryBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> os2x;
    std::unique_ptr<juce::dsp::Oversampling<float>> os4x;
    int activeQualityMode = 1;

    // ── Internal methods ──
    void initSmoothers(double sampleRate);
    void updateTargetsFromAPVTS();
    void resetAllADAA();
    void resetAll();  // v7: full DSP state reset (transport-safe)

    void processLinearPre(float* dataL, float* dataR, int numSamples);
    void processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor);
    void processLinearPost(float* dataL, float* dataR, int numSamples);

    void updateMeters(const float* inL, const float* inR,
                      const float* outL, const float* outR,
                      int n, float sparkGRDb);
    int getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);

    // v7: state migration
    void migrateState(juce::ValueTree& state, int fromVersion);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
