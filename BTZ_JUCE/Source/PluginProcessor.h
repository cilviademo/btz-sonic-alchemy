/*
  PluginProcessor.h
  BTZ - The Box Tone Zone Enhancer
  Main audio processor integrating all DSP modules
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

class BTZAudioProcessor : public juce::AudioProcessor
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

private:
    // Parameter state
    juce::AudioProcessorValueTreeState apvts;

    // DSP modules
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

    // Parameter smoothing (prevents zipper noise)
    juce::SmoothedValue<float> smoothedPunch, smoothedWarmth, smoothedBoom;
    juce::SmoothedValue<float> smoothedMix, smoothedDrive;
    juce::SmoothedValue<float> smoothedInputGain, smoothedOutputGain;

    // Metering (atomic for thread-safe GUI access)
    std::atomic<float> currentLUFS { -14.0f };
    std::atomic<float> currentPeak { -6.0f };
    std::atomic<float> gainReduction { 1.0f };
    std::atomic<float> stereoCorrelation { 1.0f };

    // LUFS measurement (simplified running average)
    float lufsAccumulator = 0.0f;
    int lufsSampleCount = 0;

    // Silence detection (optimization)
    float silenceThreshold = 1e-6f;
    int consecutiveSilentBuffers = 0;
    static constexpr int maxSilentBuffersBeforeSkip = 10;

    // Plugin version for preset compatibility
    static constexpr int pluginVersionMajor = 1;
    static constexpr int pluginVersionMinor = 0;
    static constexpr int pluginVersionPatch = 0;

    void updateMetering(const juce::AudioBuffer<float>& buffer);
    bool isBufferSilent(const juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTZAudioProcessor)
};
