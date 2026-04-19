/*
  Box Tone Zone (BTZ) — PluginProcessor.h  v9
  ────────────────────────────────────────────────────────────────────────
  v9 (industry-gap closure):
    • 5 saturation models (Tanh, Tube, Tape, Transistor, Transformer)
    • Configurable multiband engine (1–6 bands)
    • Mid/Side processing mode
    • Undo/Redo state stack (64 levels)
    • A/B comparison slots
    • Preset system (save/load/browse)
    • MIDI learn CC mapping
    • LFO modulation sources
    • EBU R128 loudness metering (momentary/short-term/integrated)
    • Spectrum analyzer ring buffer
    • Gain reduction history graph
    • 8x/16x oversampling options
    • State version bumped to 9
  ────────────────────────────────────────────────────────────────────────
  v8: removed ADAA, 1 Hz DC blocker, tightened TruePeakLimiter
  v7: BypassCrossfader, full resetAll(), state migration v4–v9
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
    bool acceptsMidi() const override { return true; }   // v9: MIDI learn
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

    // ── v9: Undo/Redo ──
    void pushUndoState(const juce::String& description = {});
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // ── v9: A/B Comparison ──
    void storeToSlotA();
    void storeToSlotB();
    void toggleAB();
    bool isSlotA() const;
    void copyAtoB();

    // ── v9: Preset System ──
    bool loadPreset(const juce::File& file);
    bool savePreset(const juce::File& file, const juce::String& name,
                    const juce::String& category = {});
    juce::File getPresetsDirectory() const;
    juce::Array<BTZDsp::PresetInfo> scanPresets() const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }

    // ── v9: MIDI Learn ──
    BTZDsp::MIDILearnState& getMIDILearn() { return midiLearn; }

    // ── v9: Spectrum + GR History (UI reads these) ──
    BTZDsp::SpectrumBuffer& getSpectrumBuffer() { return spectrumBuffer; }
    BTZDsp::GainReductionHistory& getGRHistory() { return grHistory; }

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

    // ── DSP modules ──
    BTZDsp::SafetyLayer safetyPre, safetyPost;
    BTZDsp::EnvFollower peakEnvL, peakEnvR, rmsEnvL, rmsEnvR;
    BTZDsp::EnvFollower glueEnv;
    BTZDsp::SidechainHPF glueScHpf;
    BTZDsp::GlueCompressor glueComp;
    BTZDsp::LinkwitzRileyCrossover crossover;  // Legacy single crossover for Boom
    BTZDsp::TruePeakLimiter truePeakLimiter;
    BTZDsp::ShineProcessor shineProcessor;
    BTZDsp::AutoGainSmoother autoGainSmoother;
    BTZDsp::MacroInterpreter macroInterpreter;
    BTZDsp::BypassCrossfader bypassCrossfader;

    // ── v9: New DSP modules ──
    BTZDsp::MultibandEngine multibandEngine;
    BTZDsp::MidSideEncoder midSideEncoder;
    BTZDsp::LFO lfoModSources[4];  // Up to 4 LFO sources
    BTZDsp::LoudnessMeter loudnessMeter;
    BTZDsp::SpectrumBuffer spectrumBuffer;
    BTZDsp::GainReductionHistory grHistory;

    // ── v9: State management ──
    BTZDsp::UndoStack undoStack;
    BTZDsp::ABState abState;
    BTZDsp::MIDILearnState midiLearn;

    // ── v9: Preset state ──
    int currentPresetIndex = -1;
    juce::String currentPresetName;

    // ── DSP state ──
    float hpStateL = 0.0f, hpStateR = 0.0f;
    float sideLowState = 0.0f, sideLowCoeff = 0.0f;

    float cachedDriveGain = 1.0f;
    float lastDriveDb = 0.0f;
    float lastCrestRatio = 3.0f;
    float motionPhase = 0.0f;  // v9: Motion LFO phase accumulator

    float macroValues[BTZDsp::MacroInterpreter::kNumMacros] = {};

    float lastGlueScHpfFreq = 60.0f;
    double glueScHpfSampleRate = 44100.0;

    // v9: Per-channel tape hysteresis state (for fullband mode)
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

    void updateMeters(const float* inL, const float* inR,
                      const float* outL, const float* outR,
                      int n, float sparkGRDb);
    int getRequestedQualityMode() const;
    void updateLatencyFromQuality(int mode);

    void migrateState(juce::ValueTree& state, int fromVersion);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
