/*
  Saturation.h
  Analog-style saturation for "Warmth" control
  Based on Soundtoys Decapitator, Plugin Alliance HG-2, Output Thermal
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class Saturation
{
public:
    Saturation();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setWarmth(float warmthAmount); // 0.0 to 1.0

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numChannels = inputBlock.getNumChannels();
        const auto numSamples = inputBlock.getNumSamples();

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* input = inputBlock.getChannelPointer(channel);
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                float sample = input[i];

                // Pre-gain
                sample *= (1.0f + warmthIntensity * 2.0f);

                // Soft clipper (tanh-based for musical saturation)
                float saturated = std::tanh(sample * saturationDrive);

                // Add 2nd and 3rd harmonics (even = warmth, odd = bite)
                float harmonic2 = 0.15f * warmthIntensity * saturated * saturated;
                float harmonic3 = 0.08f * warmthIntensity * saturated * saturated * saturated;

                saturated += harmonic2 + harmonic3;

                // DC blocker
                dcBlockerOutput[channel] = saturated - dcBlockerInput[channel] + 0.995f * dcBlockerOutput[channel];
                dcBlockerInput[channel] = saturated;

                // Compensate gain
                output[i] = dcBlockerOutput[channel] / (1.0f + warmthIntensity * 0.5f);
            }
        }
    }

private:
    float warmthIntensity = 0.0f;
    float saturationDrive = 1.5f;

    std::array<float, 2> dcBlockerInput = {0.0f, 0.0f};
    std::array<float, 2> dcBlockerOutput = {0.0f, 0.0f};
};
