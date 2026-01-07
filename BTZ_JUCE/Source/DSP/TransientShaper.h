/*
  TransientShaper.h
  Transient shaping for "Punch" control
  Based on Waves Smack Attack, CLA-76, SPL Transient Designer
*/

#pragma once
#include <JuceHeader.h>

class TransientShaper
{
public:
    TransientShaper();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setPunch(float punchAmount); // 0.0 to 1.0

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

                // Envelope follower
                float envelope = std::abs(sample);
                envelope = envelope > envelopeState[channel]
                    ? attackCoeff * envelope + (1.0f - attackCoeff) * envelopeState[channel]
                    : releaseCoeff * envelope + (1.0f - releaseCoeff) * envelopeState[channel];
                envelopeState[channel] = envelope;

                // Transient detection
                float delta = envelope - previousEnvelope[channel];
                previousEnvelope[channel] = envelope;

                // Amplify transients
                float transientGain = 1.0f + (delta > 0.0f ? delta * punchIntensity * 10.0f : 0.0f);
                transientGain = juce::jlimit(1.0f, 3.0f, transientGain);

                output[i] = sample * transientGain;
            }
        }
    }

private:
    float punchIntensity = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    double sampleRate = 44100.0;

    std::array<float, 2> envelopeState = {0.0f, 0.0f};
    std::array<float, 2> previousEnvelope = {0.0f, 0.0f};

    void updateCoefficients();
};
