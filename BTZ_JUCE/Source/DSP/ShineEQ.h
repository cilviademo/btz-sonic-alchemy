/*
  ShineEQ.h
  SHINE - Ultra-High Frequency Air Enhancement
  SSL Fusion Air + Maag EQ Air Band emulation
  10kHz-80kHz+ ultrasonic magic - ethereal highs, crystalline crispness

  NOW USES: Professional RBJ biquad filters for correct frequency response
*/

#pragma once
#include <JuceHeader.h>
#include "RBJFilters.h"

class ShineEQ
{
public:
    ShineEQ();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setFrequency(float freqHz);  // 10kHz to 80kHz
    void setGain(float gainDb);       // -12 to +12 dB
    void setQ(float q);               // 0.1 to 2.0
    void setMix(float mix);           // 0.0 to 1.0

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
                float drySample = sample;

                // Professional RBJ high-shelf filter
                float filtered = highShelfFilter[channel].processSample(sample);

                // Wet/dry mix
                output[i] = drySample + mixAmount * (filtered - drySample);
            }
        }
    }

private:
    float frequencyHz = 20000.0f;
    float gainDb = 3.0f;
    float qValue = 0.5f;
    float mixAmount = 0.5f;
    double sampleRate = 44100.0;

    // Professional RBJ biquad filters (one per channel)
    std::array<RBJBiquad, 2> highShelfFilter;

    void updateCoefficients();
};
