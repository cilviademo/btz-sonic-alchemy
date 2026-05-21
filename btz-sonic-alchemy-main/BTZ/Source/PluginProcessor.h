/*
  Box Tone Zone (BTZ) — PluginProcessor.h  v1.0 Ivory System
  ──────────────────────────────────────────────────────────────────────────
  Thread model:
    • Audio thread: processBlock only — no allocation, no locks.
      Writes atomic meter values and single-writer buffers.
    • Message thread: prepareToPlay, releaseResources, state serialization,
      preset/undo/AB operations, neural model loading.
    • Timer thread (UI): reads atomic meter values, reads spectrum/GR
      history (single-writer/single-reader safe).

  Signal flow:
    Input → Safety → M/S Encode → Resonance Tame → Glue Comp → Shine EQ →
    Auto Gain In → [Oversampling Up] → Saturation (transient-aware, model-select) →
    [Oversampling Down] → Auto Gain Out → Limiter → M/S Decode → Width →
    Mix → Master → Safety → Bypass Crossfade → Output
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>
#include "BTZDsp.h"
#include <atomic>
#include <memory>

class BTZAudioProcessor : public juce::AudioProcessor {
public:
    BTZAudioProcessor();
    ~BTZAudioProcessor() override = default;

    // ── AudioProcessor overrides ─────────────────────────────────────────
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ── Public accessors ─────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // ── Atomic meters (written by audio thread, read by UI timer) ────────
    struct MeterValues {
        std::atomic<float> inputPeakL  { 0.0f };
        std::atomic<float> inputPeakR  { 0.0f };
        std::atomic<float> outputPeakL { 0.0f };
        std::atomic<float> outputPeakR { 0.0f };
        std::atomic<float> grDb        { 0.0f };
        std::atomic<float> lufs        { -24.0f };
        std::atomic<float> truePeak    { -100.0f };
        std::atomic<float> correlation { 1.0f };
    };
    MeterValues meters;

    // Delta (difference) monitoring toggle — UI writes, audio thread reads.
    std::atomic<bool> deltaMonitoring { false };

    // ── Single-writer (audio) / single-reader (UI) buffers ───────────────
    BTZDsp::SpectrumBuffer       spectrumBuffer;
    BTZDsp::GainReductionHistory grHistory;

    // ── Message-thread state (UI may read, only message thread writes) ───
    BTZDsp::UndoStack            undoStack;
    BTZDsp::ABState              abState;
    BTZDsp::MIDILearnState       midiLearn;
    BTZDsp::SimpleModeState      simpleMode;
    BTZDsp::PresetIntelligence   presetIntelligence;
    BTZDsp::ReferenceToneMatcher toneMatcher;
    BTZDsp::LoudnessMatchedAB   loudnessMatchedAB;

    juce::String currentPresetName;
    int          currentPresetIndex = 0;

    BTZDsp::SaturationModel currentSatModel = BTZDsp::SaturationModel::Tanh;

    // ── Public API (message thread only) ─────────────────────────────────
    void pushUndoState(const juce::String& description = {});
    void undo();
    void redo();
    void storeA();
    void storeB();
    void toggleAB();
    void copyAtoB();
    void loadPreset(const juce::File& file);
    void loadPresetByIndex(int index);
    void savePreset(const juce::File& file);
    juce::Array<juce::File> getPresetFiles() const;
    juce::File getPresetDirectory() const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }
    void loadNeuralModel(const juce::File& modelFile, BTZDsp::SaturationModel slot);

private:
    // ── Parameter layout ─────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Parameter smoothers (audio thread only) ──────────────────────────
    BTZDsp::SmoothParam sPunch, sWarmth, sBoom, sGlue, sAir, sWidth;
    BTZDsp::SmoothParam sDensity, sMotion, sEra;
    BTZDsp::SmoothParam sMix, sDrive, sMaster;
    BTZDsp::SmoothParam sShine, sShineMix, sShineFreq, sShineQ;
    BTZDsp::SmoothParam sMacro0, sMacro1, sMacro2, sMacro3;
    BTZDsp::SmoothParam sResonanceSens, sResonanceDepth;
    BTZDsp::SmoothParam sTransientSens, sTransientMix;
    BTZDsp::SmoothParam sToneMatchAmount;

    // ── DSP modules (audio thread only) ──────────────────────────────────
    BTZDsp::SafetyLayer              safetyPre, safetyPost;
    BTZDsp::EnvFollower              peakEnvL, peakEnvR, rmsEnvL, rmsEnvR, glueEnv;
    BTZDsp::BypassCrossfader         bypassCrossfader;
    BTZDsp::SidechainHPF             glueScHpf;
    BTZDsp::GlueCompressor           glueComp;
    BTZDsp::LinkwitzRileyCrossover   crossover;
    BTZDsp::TruePeakLimiter          truePeakLimiter;
    BTZDsp::ShineProcessor           shineProcessor;
    BTZDsp::MultibandEngine          multibandEngine;
    BTZDsp::MidSideEncoder           midSideEncoder;
    BTZDsp::AutoGainSmoother         autoGainSmoother;
    BTZDsp::MeterBallistics          inputMeterBallistics, outputMeterBallistics;
    BTZDsp::LoudnessMeter            loudnessMeter;
    BTZDsp::LFO                      lfoModSources[BTZDsp::kMaxLFOs];
    BTZDsp::MacroInterpreter         macroInterpreter;

    // v1.0.1: Target Lock
    BTZDsp::TargetLockEngine         targetLockEngine;
    BTZDsp::LinkwitzRileyCrossover   targetLockXO1, targetLockXO2; // 3-band split for target lock

    // v10 modules
    BTZDsp::ResonanceTamer           resonanceTamer;
    BTZDsp::TransientSplitter        transientSplitter;
    BTZDsp::OversamplingEngine       oversamplingEngine;
    BTZDsp::NeuralSaturationModel    neuralNeve, neuralAPI, neuralSSL, neuralCustom;
    BTZDsp::WDFTubeStage             wdfTube;
    BTZDsp::WDFTransformerStage      wdfTransformer;

    // ── Oversampling (JUCE built-in, lazy-initialized) ───────────────────
    std::unique_ptr<juce::dsp::Oversampling<float>> os2x, os4x, os8x, os16x;

    // ── Per-channel filter state (audio thread only) ─────────────────────
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f;
    float tapeStateL = 0.0f, tapeStateR = 0.0f;

    // ── Cached runtime state ─────────────────────────────────────────────
    double currentSampleRate      = 44100.0;
    double lastPreparedSR         = 0.0;
    int    currentBlockSize       = 512;
    int    maxPreparedBlockSize   = 512;
    int    lastPreparedBlockSize  = 0;
    int    activeQualityMode      = 1;
    float  motionPhase            = 0.0f;
    float  sideLowCoeff           = 0.0f;
    double glueScHpfSampleRate    = 44100.0;
    bool   prepared               = false;

    // ── RT-safe deferred latency update ──────────────────────────────────
    std::atomic<int>  pendingLatency { -1 };  // -1 = no pending change

    // ── RT-safe multiband config (avoid calling prepare on audio thread) ─
    int lastMultibandCount = 0;  // track changes, defer to message thread

    // ── Dry buffer for wet/dry mix ───────────────────────────────────────
    juce::AudioBuffer<float> dryBuffer;

    // ── Internal signal flow methods (audio thread only) ─────────────────
    void initSmoothers(double sampleRate);
    void updateTargetsFromAPVTS();
    void resetAll();
    int  getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);
    void processLinearPre(float* dataL, float* dataR, int numSamples);
    void processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor);
    void processLinearPost(float* dataL, float* dataR, int numSamples);
    void processMIDILearn(juce::MidiBuffer& midi);
    void updateMeters(const float* inL, const float* inR,
                      const float* outL, const float* outR,
                      int numSamples, float sparkGRDb);

    // ── State migration ──────────────────────────────────────────────────
    void migrateState(juce::ValueTree& state, int fromVersion);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
