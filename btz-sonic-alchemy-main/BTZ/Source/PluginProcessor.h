/*
  Box Tone Zone (BTZ) — PluginProcessor.h
  Overhauled: modular DSP, isolated oversampling, macro system,
  SR-dependent meters, parameter groups, SparkLimiter, ShineProcessor.
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
    BTZDsp::SmoothParam sDensity, sMotion, sEra, sMix, sDrive;
    BTZDsp::SmoothParam sMaster, sSparkCeil, sSparkMix, sShine, sShineMix;
    BTZDsp::SmoothParam sMacro0, sMacro1, sMacro2, sMacro3;

    // ── DSP modules ──
    BTZDsp::SafetyLayer safetyPre, safetyPost;
    BTZDsp::SlewLimiter slewL, slewR;
    BTZDsp::EnvFollower peakEnvL, peakEnvR, rmsEnvL, rmsEnvR;
    BTZDsp::EnvFollower glueEnv;
    BTZDsp::GlueCompressor glueComp;
    BTZDsp::LinkwitzRileyCrossover crossover;
    BTZDsp::SparkLimiter sparkLimiter;
    BTZDsp::ShineProcessor shineProcessor;
    BTZDsp::AutoGainSmoother autoGainSmoother;
    BTZDsp::MacroInterpreter macroInterpreter;

    // ── DSP state ──
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f, sideLowCoeff = 0.0f;
    float sparkGrEnvelope = 0.0f;

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

    // Linear pre-processing (runs at base SR)
    void processLinearPre(float* dataL, float* dataR, int numSamples);
    // Nonlinear processing (runs at oversampled SR)
    void processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor);
    // Linear post-processing (runs at base SR)
    void processLinearPost(float* dataL, float* dataR, int numSamples);

    void updateMeters(const float* inL, const float* inR,
                      const float* outL, const float* outR,
                      int n, float sparkGRDb);
    int getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
