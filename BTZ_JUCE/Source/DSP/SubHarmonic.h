/*
  SubHarmonic.h
  Subharmonic synthesis for "Boom" control
  Based on Plugin Alliance bx_subsynth, Unfiltered Audio Bass-Mint
*/

#pragma once
#include <JuceHeader.h>

class SubHarmonic
{
public:
    SubHarmonic();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setBoom(float boomAmount); // 0.0 to 1.0

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

                // Low-pass filter to isolate bass (below 150Hz)
                float filtered = lowpassCoeff * sample + (1.0f - lowpassCoeff) * lowpassState[channel];
                lowpassState[channel] = filtered;

                // Generate subharmonic (octave down via sine oscillator)
                // Track fundamental with zero-crossing detection
                if ((previousSample[channel] < 0.0f && filtered >= 0.0f) ||
                    (previousSample[channel] >= 0.0f && filtered < 0.0f))
                {
                    // Zero crossing detected - reset phase
                    subPhase[channel] = 0.0f;
                }
                previousSample[channel] = filtered;

                // Generate subharmonic sine wave (octave down)
                float subharmonic = std::sin(subPhase[channel]);
                subPhase[channel] += juce::MathConstants<float>::twoPi * 0.5f / sampleRate; // Half frequency
                if (subPhase[channel] > juce::MathConstants<float>::twoPi)
                    subPhase[channel] -= juce::MathConstants<float>::twoPi;

                // Mix subharmonic with original, scaled by boom amount
                float subGain = boomIntensity * std::abs(filtered) * 0.5f; // Amplitude follows original
                output[i] = sample + (subharmonic * subGain);
            }
        }
    }

private:
    float boomIntensity = 0.0f;
    float lowpassCoeff = 0.0f;
    double sampleRate = 44100.0;

    std::array<float, 2> lowpassState = {0.0f, 0.0f};
    std::array<float, 2> previousSample = {0.0f, 0.0f};
    std::array<float, 2> subPhase = {0.0f, 0.0f};

    void updateCoefficients();
};
