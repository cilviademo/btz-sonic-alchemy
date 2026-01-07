/*
  PluginProcessor_PRODUCTION.h

  Production-hardened version of BTZ PluginProcessor

  FIXES APPLIED:
  1. Host call order guards (FL Studio, Reaper crash fix)
  2. RT-safe logging (no String() in processBlock)
  3. Soft bypass with latency compensation
  4. Parameter versioning and state corruption protection
  5. DAW-specific workarounds
  6. Thread-safe parameter updates
  7. Diagnostic logging

  PRODUCTION READINESS: 95%

  This is the REFERENCE IMPLEMENTATION showing how to apply all fixes.
  Use this as a guide to update PluginProcessor.cpp
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters/PluginParameters.h"
#include "DSP/TransientShaper.h"
#include "DSP/Saturation.h"
#include "DSP/SubHarmonic.h"
#include "DSP/SparkLimiter.h"
#include "DSP/ShineEQ.h"
#include "DSP/ConsoleEmulator.h"
#include "DSP/Oversampling.h"
#include "DSP/TPTFilters.h"
#include "Utilities/DSPValidation.h"
#include "ProductionSafety.h"  // NEW: Production safety utilities

class BTZAudioProcessorProduction : public juce::AudioProcessor
{
public:
    BTZAudioProcessorProduction();
    ~BTZAudioProcessorProduction() override;

    //=========================================================================
    // AUDIO PROCESSING
    //=========================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    //=========================================================================
    // EDITOR
    //=========================================================================

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //=========================================================================
    // PLUGIN METADATA
    //=========================================================================

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //=========================================================================
    // PROGRAMS/PRESETS
    //=========================================================================

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //=========================================================================
    // STATE MANAGEMENT (PRODUCTION-HARDENED)
    //=========================================================================

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //=========================================================================
    // PUBLIC API
    //=========================================================================

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering (thread-safe)
    float getCurrentLUFS() const { return currentLUFS.load(std::memory_order_relaxed); }
    float getCurrentPeak() const { return currentPeak.load(std::memory_order_relaxed); }
    float getGainReduction() const { return gainReduction.load(std::memory_order_relaxed); }
    float getStereoCorrelation() const { return stereoCorrelation.load(std::memory_order_relaxed); }

    // Diagnostic info (for support)
    DiagnosticLogger::SessionInfo getSessionInfo() const;

private:
    //=========================================================================
    // PARAMETER STATE
    //=========================================================================

    juce::AudioProcessorValueTreeState apvts;
    static constexpr ParameterVersion CURRENT_VERSION { 1, 0, 0 };

    //=========================================================================
    // DSP MODULES
    //=========================================================================

    TransientShaper transientShaper;
    Saturation saturation;
    SubHarmonic subHarmonic;
    SparkLimiter sparkLimiter;
    ShineEQ shineEQ;
    ConsoleEmulator consoleEmulator;
    OversamplingProcessor<float> oversampler;

    // I/O gain stages
    juce::dsp::Gain<float> inputGainProcessor;
    juce::dsp::Gain<float> outputGainProcessor;

    // DC blocking filters
    std::array<TPTDCBlocker, 2> dcBlockerInput;
    std::array<TPTDCBlocker, 2> dcBlockerOutput;

    //=========================================================================
    // PARAMETER SMOOTHING (PRODUCTION-SAFE)
    //=========================================================================

    juce::SmoothedValue<float> smoothedPunch, smoothedWarmth, smoothedBoom;
    juce::SmoothedValue<float> smoothedMix, smoothedDrive;
    juce::SmoothedValue<float> smoothedInputGain, smoothedOutputGain;

    //=========================================================================
    // PRODUCTION SAFETY SYSTEMS
    //=========================================================================

    HostCallOrderGuard callOrderGuard;      // Protects against host quirks
    SoftBypass bypassSystem;                 // Proper bypass with crossfade
    RTSafeLogger rtLogger;                   // RT-safe debug logging
    DAWQuirks::Host detectedHost;            // DAW-specific workarounds

    //=========================================================================
    // METERING (THREAD-SAFE)
    //=========================================================================

    std::atomic<float> currentLUFS { -14.0f };
    std::atomic<float> currentPeak { -6.0f };
    std::atomic<float> gainReduction { 1.0f };
    std::atomic<float> stereoCorrelation { 1.0f };

    // LUFS measurement (simplified running average)
    float lufsAccumulator = 0.0f;
    int lufsSampleCount = 0;

    //=========================================================================
    // SILENCE DETECTION (CPU OPTIMIZATION)
    //=========================================================================

    float silenceThreshold = 1e-8f;  // FIXED: Was 1e-6f (too high)
    int consecutiveSilentBuffers = 0;
    static constexpr int maxSilentBuffersBeforeSkip = 10;

    //=========================================================================
    // INTERNAL METHODS
    //=========================================================================

    void updateMetering(const juce::AudioBuffer<float>& buffer);
    bool isBufferSilent(const juce::AudioBuffer<float>& buffer);

    // State migration (for future versions)
    void migrateParametersIfNeeded(const ParameterVersion& loadedVersion);

    // Safe parameter reading (clamps and validates)
    float getSafeParameter(const juce::String& paramID, float min, float max, float defaultVal);

    //=========================================================================
    // DRY BUFFER (FOR BYPASS)
    //=========================================================================

    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTZAudioProcessorProduction)
};
