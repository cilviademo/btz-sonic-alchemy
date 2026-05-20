/*
  Box Tone Zone (BTZ) — PluginProcessor.h  v10
  ────────────────────────────────────────────────────────────────────────
  v10 (industry-transcending features):
    • RTNeural learned saturation models (Neural_Neve, Neural_API, Neural_SSL, Neural_Custom)
    • WDF circuit models (WDF_Tube, WDF_Transformer)
    • Dynamic Resonance Taming (pre-saturation spectral peak suppression)
    • Transient-Aware Saturation (envelope-split transient/sustain processing)
    • Reference Tone Matching (spectral comparison + auto-EQ)
    • SIMD Oversampling Engine (2x/4x/8x FIR antialiasing)
    • Simple Mode (3-knob interface: Drive, Tone, Output)
    • Preset Intelligence (input analysis + contextual preset suggestion)
    • Loudness-Matched A/B (auto-compensated comparison)
    • All v9 features retained: 5 original sat models, multiband, M/S, LFO,
      EBU R128, undo/redo, A/B, MIDI learn, spectrum, GR history
  ────────────────────────────────────────────────────────────────────────
  v9: 5 sat models, multiband, M/S, LFO, EBU R128, undo/redo, A/B, MIDI learn
  v8: removed ADAA, 1 Hz DC blocker, tightened TruePeakLimiter
*/
#pragma once

#include <JuceHeader.h>
#include "BTZDsp.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>

// ═══════════════════════════════════════════════════════════════════════
// Meter state — atomic bridge from audio thread to UI thread
// ═══════════════════════════════════════════════════════════════════════
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
    std::atomic<float> lufsShortTerm { -24.0f };
    std::atomic<float> lufsIntegrated { -24.0f };
    std::atomic<float> inputClip   { 0.0f };
    std::atomic<float> outputClip  { 0.0f };
    std::atomic<float> correlation { 1.0f };
};

// ═══════════════════════════════════════════════════════════════════════
// BTZAudioProcessor — main plugin processor
// ═══════════════════════════════════════════════════════════════════════
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
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    BTZMeterState& getMeters() { return meters; }

    // ── Undo/Redo ──
    void pushUndoState(const juce::String& description = {});
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // ── A/B Comparison ──
    void storeToSlotA();
    void storeToSlotB();
    void toggleAB();
    bool isSlotA() const;
    void copyAtoB();

    // ── Preset System ──
    bool loadPreset(const juce::File& file);
    bool savePreset(const juce::File& file, const juce::String& name,
                    const juce::String& category = {});
    juce::File getPresetsDirectory() const;
    juce::Array<BTZDsp::PresetInfo> scanPresets() const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }

    // ── MIDI Learn ──
    BTZDsp::MIDILearnState& getMIDILearn() { return midiLearn; }

    // ── Spectrum + GR History (UI reads these) ──
    BTZDsp::SpectrumBuffer& getSpectrumBuffer() { return spectrumBuffer; }
    BTZDsp::GainReductionHistory& getGRHistory() { return grHistory; }

    // ── v10: Neural Model Loading ──
    bool loadNeuralModel(BTZDsp::SaturationModel slot, const juce::File& jsonFile);
    bool isNeuralModelLoaded(BTZDsp::SaturationModel slot) const;

    // ── v10: Reference Tone Matching ──
    BTZDsp::ReferenceToneMatcher& getToneMatcher() { return toneMatcher; }
    void captureReferenceFromBuffer(const juce::AudioBuffer<float>& refBuffer, double refSR);

    // ── v10: Preset Intelligence ──
    BTZDsp::PresetIntelligence& getPresetIntelligence() { return presetIntelligence; }
    void runPresetIntelligence();

    // ── v10: Simple Mode ──
    BTZDsp::SimpleModeState& getSimpleMode() { return simpleMode; }
    void setSimpleModeEnabled(bool enabled);

    // ── v10: Loudness-Matched A/B ──
    BTZDsp::LoudnessMatchedAB& getLoudnessMatchedAB() { return loudnessMatchedAB; }

private:
    // ── Parameter layout ──
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Meter state ──
    BTZMeterState meters;

    // ── Parameter smoothers ──
    BTZDsp::SmoothParam sPunch, sWarmth, sBoom, sGlue, sAir, sWidth;
    BTZDsp::SmoothParam sDensity, sMotion, sEra, sMix, sDrive, sMaster;
    BTZDsp::SmoothParam sShine, sShineMix, sShineFreq, sShineQ;
    BTZDsp::SmoothParam sMacro0, sMacro1, sMacro2, sMacro3;
    // v10: New parameter smoothers
    BTZDsp::SmoothParam sResonanceSens, sResonanceDepth;
    BTZDsp::SmoothParam sTransientSens, sTransientMix;
    BTZDsp::SmoothParam sToneMatchAmount;

    // ── DSP modules (core) ──
    BTZDsp::SafetyLayer safetyPre, safetyPost;
    BTZDsp::EnvFollower peakEnvL, peakEnvR, rmsEnvL, rmsEnvR;
    BTZDsp::EnvFollower glueEnv;
    BTZDsp::SidechainHPF glueScHpf;
    BTZDsp::GlueCompressor glueComp;
    BTZDsp::LinkwitzRileyCrossover crossover;
    BTZDsp::TruePeakLimiter truePeakLimiter;
    BTZDsp::ShineProcessor shineProcessor;
    BTZDsp::AutoGainSmoother autoGainSmoother;
    BTZDsp::MacroInterpreter macroInterpreter;
    BTZDsp::BypassCrossfader bypassCrossfader;

    // ── v9: DSP modules ──
    BTZDsp::MultibandEngine multibandEngine;
    BTZDsp::MidSideEncoder midSideEncoder;
    BTZDsp::LFO lfoModSources[4];
    BTZDsp::LoudnessMeter loudnessMeter;
    BTZDsp::SpectrumBuffer spectrumBuffer;
    BTZDsp::GainReductionHistory grHistory;

    // ── v10: New DSP modules ──
    BTZDsp::NeuralSaturationModel neuralNeve;
    BTZDsp::NeuralSaturationModel neuralAPI;
    BTZDsp::NeuralSaturationModel neuralSSL;
    BTZDsp::NeuralSaturationModel neuralCustom;
    BTZDsp::WDFTubeStage wdfTube;
    BTZDsp::WDFTransformerStage wdfTransformer;
    BTZDsp::ResonanceTamer resonanceTamer;
    BTZDsp::TransientSplitter transientSplitter;
    BTZDsp::OversamplingEngine oversamplingEngine;
    BTZDsp::ReferenceToneMatcher toneMatcher;
    BTZDsp::PresetIntelligence presetIntelligence;
    BTZDsp::SimpleModeState simpleMode;
    BTZDsp::LoudnessMatchedAB loudnessMatchedAB;
    BTZDsp::MeterBallistics inputMeterBallistics;
    BTZDsp::MeterBallistics outputMeterBallistics;

    // ── State management ──
    BTZDsp::UndoStack undoStack;
    BTZDsp::ABState abState;
    BTZDsp::MIDILearnState midiLearn;

    // ── Preset state ──
    int currentPresetIndex = -1;
    juce::String currentPresetName;

    // ── DSP state ──
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f, sideLowCoeff = 0.0f;
    float cachedDriveGain = 1.0f;
    float lastDriveDb = 0.0f;
    float lastCrestRatio = 3.0f;
    float motionPhase = 0.0f;
    float macroValues[BTZDsp::MacroInterpreter::kNumMacros] = {};
    float lastGlueScHpfFreq = 60.0f;
    double glueScHpfSampleRate = 44100.0;
    float tapeStateL = 0.0f, tapeStateR = 0.0f;

    // Silence detection
    int silentFrameCount = 0;
    static constexpr int kSilentFrameThreshold = 512;
    static constexpr float kSilenceThresholdProc = 1.0e-8f;

    bool prepared = false;
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
    std::unique_ptr<juce::dsp::Oversampling<float>> os8x;
    std::unique_ptr<juce::dsp::Oversampling<float>> os16x;
    int activeQualityMode = 1;

    // ── Internal methods ──
    void initSmoothers(double sampleRate);
    void updateTargetsFromAPVTS();
    void resetAll();

    void processLinearPre(float* dataL, float* dataR, int numSamples);
    void processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor);
    void processLinearPost(float* dataL, float* dataR, int numSamples);

    void processMIDILearn(juce::MidiBuffer& midi);
    void processNeuralSaturation(float& l, float& r, BTZDsp::SaturationModel model);

    void updateMeters(const float* inL, const float* inR,
                      const float* outL, const float* outR,
                      int n, float sparkGRDb);
    int getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);

    void migrateState(juce::ValueTree& state, int fromVersion);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
