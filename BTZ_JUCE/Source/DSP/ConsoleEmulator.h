/*
  ConsoleEmulator.h
  Console emulation for "Mix" knob - adds glue and cohesion
  Based on The Glue, Plugin Alliance bx_console SSL/Neve
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ConsoleEmulator
{
public:
    enum class Type
    {
        Transparent,
        Glue,
        Vintage
    };

    ConsoleEmulator();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setType(Type type);
    void setMix(float mix); // 0.0 to 1.0

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

                // Apply console characteristics based on type
                float processed = sample;
                switch (currentType)
                {
                    case Type::Transparent:
                        // Minimal coloration, just subtle glue
                        processed = sample * (1.0f + 0.01f * std::tanh(sample * 5.0f));
                        break;

                    case Type::Glue:
                        // SSL-style bus compression glue
                        processed = std::tanh(sample * 1.2f) * 0.9f;
                        // Add subtle crosstalk (channel bleed)
                        if (numChannels == 2)
                        {
                            size_t otherChannel = 1 - channel;
                            processed += 0.02f * input[i] * (channel == 0 ? 1.0f : -1.0f);
                        }
                        break;

                    case Type::Vintage:
                        // Neve-style warmth and character
                        processed = std::tanh(sample * 1.5f);
                        // Add even harmonics for warmth
                        processed += 0.08f * processed * processed;
                        break;
                }

                // Wet/dry mix
                output[i] = drySample + mixAmount * (processed - drySample);
            }
        }
    }

private:
    Type currentType = Type::Transparent;
    float mixAmount = 1.0f;
};
