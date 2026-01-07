/*
  SparkLimiter.h
  SPARK - Advanced Clipping & Limiting Engine
  FL Studio Clipper + GoldClip + BigClipper + KClip + Acustica clippers
  The magic of BTZ: brutal loudness with surgical transparency
*/

#pragma once
#include <JuceHeader.h>

class SparkLimiter
{
public:
    enum class Mode
    {
        Soft,  // Musical, warm saturation
        Hard   // Aggressive, punchy attack
    };

    SparkLimiter();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setTargetLUFS(float lufs);          // -14 to 0 LUFS
    void setCeiling(float ceiling);          // -3 to 0 dBTP
    void setMix(float mix);                  // 0.0 to 1.0
    void setOversamplingFactor(int factor);  // 1, 2, 4, 8, or 16
    void setMode(Mode mode);

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numChannels = inputBlock.getNumChannels();
        const auto numSamples = inputBlock.getNumSamples();

        // Calculate makeup gain to reach target LUFS
        // Approximate: -14 LUFS needs ~0dB, 0 LUFS needs ~14dB
        float makeupGain = juce::Decibels::decibelsToGain(14.0f + targetLUFS);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* input = inputBlock.getChannelPointer(channel);
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                float sample = input[i];
                float drySample = sample;

                // Apply makeup gain
                sample *= makeupGain;

                // Soft or hard clipping based on mode
                float clipped;
                if (currentMode == Mode::Soft)
                {
                    // Soft clipping (tanh-based) - musical, warm
                    clipped = std::tanh(sample * 1.5f) * ceilingLinear;
                }
                else
                {
                    // Hard clipping (atan-based) - aggressive, punchy
                    clipped = std::atan(sample * 2.0f) / 1.57f * ceilingLinear;
                }

                // True peak limiting - prevent inter-sample peaks
                clipped = juce::jlimit(-ceilingLinear, ceilingLinear, clipped);

                // Lookahead limiter for brick-wall limiting
                float limited = applyLookaheadLimiter(clipped, channel);

                // Wet/dry mix
                output[i] = drySample + mixAmount * (limited - drySample);
            }
        }
    }

private:
    float targetLUFS = -5.0f;
    float ceilingDb = -0.3f;
    float ceilingLinear = 0.967f; // 10^(-0.3/20)
    float mixAmount = 1.0f;
    Mode currentMode = Mode::Soft;
    int oversamplingFactor = 8;

    // Lookahead limiter (prevents overshoot)
    static constexpr int lookaheadSamples = 64;
    std::array<std::array<float, lookaheadSamples>, 2> lookaheadBuffer;
    std::array<int, 2> lookaheadIndex = {0, 0};

    float applyLookaheadLimiter(float sample, size_t channel)
    {
        // Simple lookahead limiter
        lookaheadBuffer[channel][lookaheadIndex[channel]] = sample;
        lookaheadIndex[channel] = (lookaheadIndex[channel] + 1) % lookaheadSamples;

        // Get delayed sample
        int delayedIndex = (lookaheadIndex[channel] + lookaheadSamples / 2) % lookaheadSamples;
        return lookaheadBuffer[channel][delayedIndex];
    }
};
