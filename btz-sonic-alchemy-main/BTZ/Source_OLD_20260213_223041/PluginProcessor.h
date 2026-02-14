#pragma once

#include <JuceHeader.h>
#include "WaveShaper.h"
#include "EQ.h"
#include "TransientShaper.h"
#include "MultiFilterDelay.h"
#include "DeepFilterNet.h"
#include "TimbralTransfer.h"
#include "Fuzzed.h"
#include "BassEnhancer.h"
#include "LimiterNo6.h"
#include "TapeEmulator.h"
#include "GranularProcessor.h"
#include "GateProcessor.h"
#include "ConsoleEmulator.h"

class BTZAudioProcessor : public juce::AudioProcessor {
public:
    BTZAudioProcessor();
    ~BTZAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BTZ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    void setTextureEnabled(bool enabled) { textureEnabled = enabled; }
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters {
        *this, nullptr, "PARAMETERS",
        {
            std::make_unique<juce::AudioParameterFloat>("punch", "Punch", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("warmth", "Warmth", 0.0f, 1.0f, 0.3f),
            std::make_unique<juce::AudioParameterFloat>("boom", "Boom", 0.0f, 1.0f, 0.2f),
            std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f),
            std::make_unique<juce::AudioParameterBool>("texture", "Texture", false),
            std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 12.0f, 0.0f)
        }
    };

    chowdsp::TransientShaper transientShaper;
    juce::dsp::Compressor<float> compressor;
    GateProcessor gateProcessor;
    chowdsp::WaveShaper waveShaper;
    FuzzedDistortion fuzzedDistortion;
    TapeEmulator tapeEmulator;
    TimbralTransfer timbralTransfer { "Source/Models/timbral_model.pt" };
    chowdsp::EQ eq;
    BassEnhancer bassEnhancer;
    juce::dsp::Convolution convolution;
    MultiFilterDelay delay;
    GranularProcessor granularProcessor;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> exciter;
    ConsoleEmulator consoleEmulator;
    juce::dsp::WaveShaper<float> softClipper;
    LimiterNo6 multibandLimiter;
    juce::dsp::Oversampling<float> oversampler { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    DeepFilterNet deepFilter { "Source/Models/DeepFilterNet2.tar.gz" };

    bool textureEnabled = false;

    float aiTransientGain = 1.0f;
    float aiLowShelfGain = 0.0f;
    float aiSaturationAmount = 1.0f;
    float aiLoudnessBoost = 1.0f;
    float aiExciterAmount = 0.0f;
    float aiGranularAmount = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessor)
};
