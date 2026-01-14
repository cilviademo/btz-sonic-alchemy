/*
  PluginProcessor.h
  BTZ - The Box Tone Zone Enhancer
  Main audio processor integrating all DSP modules

  90-95% QUALITY IMPROVEMENTS:
  - TPT DC blocking filters (removes DC offset)
  - DSP validation in DEBUG builds (catches NaN/Inf)
  - Professional RBJ biquad filters
  - TPT envelope followers (no frequency warping)
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters/PluginParameters.h"
#include "DSP/TransientShaper.h"
#include "DSP/Saturation.h"
#include "DSP/SubHarmonic.h"
#include "DSP/SparkLimiter.h"
#include "DSP/ShineEQ.h"
#include "DSP/ConsoleEmulator.h"
#include "DSP/Oversampling.h"
#include "DSP/TPTFilters.h"

// Phase 1-3: Enhanced DSP modules
#include "DSP/EnhancedSPARK.h"
#include "DSP/EnhancedSHINE.h"
#include "DSP/ComponentVariance.h"
#include "DSP/SafetyLayer.h"
#include "DSP/LongTermMemory.h"
#include "DSP/StereoEnhancement.h"
#include "DSP/PerformanceGuardrails.h"
#include "DSP/DeterministicProcessing.h"
#include "DSP/OversamplingManager.h"

#include "Utilities/DSPValidation.h"
#include "Utilities/DSPConstants.h"
#include "ProductionSafety.h"

class BTZAudioProcessor : public juce::AudioProcessor,
                           private juce::AsyncUpdater
{
public:
    BTZAudioProcessor();
    ~BTZAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Access to parameters
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering (for GUI)
    float getCurrentLUFS() const { return currentLUFS.load(); }
    float getCurrentPeak() const { return currentPeak.load(); }
    float getGainReduction() const { return gainReduction.load(); }
    float getStereoCorrelation() const { return stereoCorrelation.load(); }

    // RT-safe logging (call from editor timer to flush messages)
    void processRTLogMessages() { rtLogger.processMessages(); }

private:
    // Parameter state
    juce::AudioProcessorValueTreeState apvts;

    // DSP modules (legacy - will be replaced by enhanced versions)
    TransientShaper transientShaper;
    Saturation saturation;
    SubHarmonic subHarmonic;
    SparkLimiter sparkLimiter;          // Will be replaced by EnhancedSPARK
    ShineEQ shineEQ;                    // Will be replaced by EnhancedSHINE
    ConsoleEmulator consoleEmulator;
    OversamplingProcessor<float> oversampler;  // Will be replaced by OversamplingManager

    // Phase 1-3: Enhanced DSP modules
    EnhancedSPARK enhancedSpark;
    EnhancedSHINE enhancedShine;
    ComponentVariance componentVariance;
    CompositeSafetyLayer safetyLayer;
    LongTermMemory longTermMemory;
    CompositeStereoEnhancement stereoEnhancement;
    CompositePerformanceGuardrails performanceGuardrails;
    CompositeDeterministicProcessing deterministicProcessing;
    OversamplingManager oversamplingManager;

    // I/O gain stages
    juce::dsp::Gain<float> inputGainProcessor;
    juce::dsp::Gain<float> outputGainProcessor;

    // DC blocking filters (TPT - removes DC offset after saturation)
    // DEPRECATED: Replaced by SafetyLayer
    // std::array<TPTDCBlocker, 2> dcBlockerInput;
    // std::array<TPTDCBlocker, 2> dcBlockerOutput;

    // Production safety (prevents crashes from host call order issues)
    HostCallOrderGuard callOrderGuard;
    RTSafeLogger rtLogger;

    // Parameter smoothing (prevents zipper noise)
    juce::SmoothedValue<float> smoothedPunch, smoothedWarmth, smoothedBoom;
    juce::SmoothedValue<float> smoothedMix, smoothedDrive;
    juce::SmoothedValue<float> smoothedInputGain, smoothedOutputGain;

    // P2-3: Metering (atomic for thread-safe GUI access) - using constants
    std::atomic<float> currentLUFS { BTZConstants::defaultLUFS };
    std::atomic<float> currentPeak { BTZConstants::defaultPeak };
    std::atomic<float> gainReduction { 1.0f };
    std::atomic<float> stereoCorrelation { 1.0f };

    // LUFS measurement (simplified running average)
    float lufsAccumulator = 0.0f;
    int lufsSampleCount = 0;

    // Silence detection (optimization) - using constants
    int consecutiveSilentBuffers = 0;

    // Plugin version for preset compatibility
    static constexpr int pluginVersionMajor = 1;
    static constexpr int pluginVersionMinor = 0;
    static constexpr int pluginVersionPatch = 0;

    void updateMetering(const juce::AudioBuffer<float>& buffer);
    bool isBufferSilent(const juce::AudioBuffer<float>& buffer);

    // P0 FIX: Async oversampling factor update (prevents allocation in audio thread)
    void handleAsyncUpdate() override;
    std::atomic<int> pendingOSFactor { 8 };  // Default 8x (matches sparkOS index 3)
    std::atomic<bool> osFactorNeedsUpdate { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTZAudioProcessor)
};
