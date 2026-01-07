/*
  ShineEQ.h
  SHINE - Ultra-High Frequency Air Enhancement
  SSL Fusion Air + Maag EQ Air Band emulation
  10kHz-80kHz+ ultrasonic magic - ethereal highs, crystalline crispness
*/

#pragma once
#include <JuceHeader.h>

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

                // Biquad high-shelf filter
                float filtered = b0 * sample + b1 * z1[channel] + b2 * z2[channel]
                               - a1 * z1_out[channel] - a2 * z2_out[channel];

                // Update delay line
                z2[channel] = z1[channel];
                z1[channel] = sample;
                z2_out[channel] = z1_out[channel];
                z1_out[channel] = filtered;

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

    // Biquad coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // Delay line (z^-1 and z^-2)
    std::array<float, 2> z1 = {0.0f, 0.0f};
    std::array<float, 2> z2 = {0.0f, 0.0f};
    std::array<float, 2> z1_out = {0.0f, 0.0f};
    std::array<float, 2> z2_out = {0.0f, 0.0f};

    void updateCoefficients();
};
