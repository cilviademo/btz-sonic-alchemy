/*
  RBJFilters.h

  Professional biquad filter implementation based on:
  Robert Bristow-Johnson's Audio EQ Cookbook
  http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

  This is THE industry-standard biquad reference.
  Used by: Pro Tools, Logic, Ableton, FabFilter, Waves, etc.

  Improvements over simplified biquad:
  - Correct Q/frequency response
  - Proper gain scaling
  - Numerically stable coefficient calculation
  - Multiple filter types with consistent behavior
*/

#pragma once
#include <JuceHeader.h>
#include <cmath>

class RBJBiquad
{
public:
    enum class FilterType
    {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        PeakingEQ,
        LowShelf,
        HighShelf
    };

    RBJBiquad() = default;

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        z1_L = z2_L = 0.0f;
        z1_R = z2_R = 0.0f;
    }

    //=== PARAMETER SETTERS ===

    void setType(FilterType newType)
    {
        if (type != newType)
        {
            type = newType;
            updateCoefficients();
        }
    }

    void setFrequency(float freqHz)
    {
        if (frequency != freqHz)
        {
            frequency = juce::jlimit(10.0f, static_cast<float>(sampleRate * 0.49), freqHz);
            updateCoefficients();
        }
    }

    void setQ(float newQ)
    {
        if (Q != newQ)
        {
            Q = juce::jlimit(0.1f, 20.0f, newQ);
            updateCoefficients();
        }
    }

    void setGainDB(float newGainDB)
    {
        if (gainDB != newGainDB)
        {
            gainDB = juce::jlimit(-48.0f, 48.0f, newGainDB);
            updateCoefficients();
        }
    }

    //=== PROCESSING ===

    inline float processSample(float input, bool rightChannel = false)
    {
        float& z1 = rightChannel ? z1_R : z1_L;
        float& z2 = rightChannel ? z2_R : z2_L;

        // Direct Form II (more numerically stable than Direct Form I)
        float output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;

        return output;
    }

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

            bool isRight = (channel == 1);
            for (size_t i = 0; i < numSamples; ++i)
            {
                output[i] = processSample(input[i], isRight);
            }
        }
    }

private:
    FilterType type = FilterType::LowPass;
    double sampleRate = 44100.0;
    float frequency = 1000.0f;
    float Q = 0.707f;  // Butterworth default
    float gainDB = 0.0f;

    // Coefficients (Direct Form II)
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // State (separate for L/R)
    float z1_L = 0.0f, z2_L = 0.0f;
    float z1_R = 0.0f, z2_R = 0.0f;

    //=== COEFFICIENT CALCULATION (RBJ Audio EQ Cookbook) ===

    void updateCoefficients()
    {
        const float A = std::pow(10.0f, gainDB / 40.0f);  // For shelf/peaking
        const float omega = 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        const float sn = std::sin(omega);
        const float cs = std::cos(omega);
        const float alpha = sn / (2.0f * Q);

        float b0_temp, b1_temp, b2_temp, a0_temp, a1_temp, a2_temp;

        switch (type)
        {
            case FilterType::LowPass:
                b0_temp = (1.0f - cs) / 2.0f;
                b1_temp = 1.0f - cs;
                b2_temp = (1.0f - cs) / 2.0f;
                a0_temp = 1.0f + alpha;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha;
                break;

            case FilterType::HighPass:
                b0_temp = (1.0f + cs) / 2.0f;
                b1_temp = -(1.0f + cs);
                b2_temp = (1.0f + cs) / 2.0f;
                a0_temp = 1.0f + alpha;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha;
                break;

            case FilterType::BandPass:  // Constant skirt gain
                b0_temp = Q * alpha;
                b1_temp = 0.0f;
                b2_temp = -Q * alpha;
                a0_temp = 1.0f + alpha;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha;
                break;

            case FilterType::Notch:
                b0_temp = 1.0f;
                b1_temp = -2.0f * cs;
                b2_temp = 1.0f;
                a0_temp = 1.0f + alpha;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha;
                break;

            case FilterType::AllPass:
                b0_temp = 1.0f - alpha;
                b1_temp = -2.0f * cs;
                b2_temp = 1.0f + alpha;
                a0_temp = 1.0f + alpha;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha;
                break;

            case FilterType::PeakingEQ:
                b0_temp = 1.0f + alpha * A;
                b1_temp = -2.0f * cs;
                b2_temp = 1.0f - alpha * A;
                a0_temp = 1.0f + alpha / A;
                a1_temp = -2.0f * cs;
                a2_temp = 1.0f - alpha / A;
                break;

            case FilterType::LowShelf:
            {
                float beta = std::sqrt(A) / Q;  // Use Q for shelf slope
                b0_temp = A * ((A + 1.0f) - (A - 1.0f) * cs + beta * sn);
                b1_temp = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cs);
                b2_temp = A * ((A + 1.0f) - (A - 1.0f) * cs - beta * sn);
                a0_temp = (A + 1.0f) + (A - 1.0f) * cs + beta * sn;
                a1_temp = -2.0f * ((A - 1.0f) + (A + 1.0f) * cs);
                a2_temp = (A + 1.0f) + (A - 1.0f) * cs - beta * sn;
                break;
            }

            case FilterType::HighShelf:
            {
                float beta = std::sqrt(A) / Q;
                b0_temp = A * ((A + 1.0f) + (A - 1.0f) * cs + beta * sn);
                b1_temp = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs);
                b2_temp = A * ((A + 1.0f) + (A - 1.0f) * cs - beta * sn);
                a0_temp = (A + 1.0f) - (A - 1.0f) * cs + beta * sn;
                a1_temp = 2.0f * ((A - 1.0f) - (A + 1.0f) * cs);
                a2_temp = (A + 1.0f) - (A - 1.0f) * cs - beta * sn;
                break;
            }
        }

        // Normalize by a0 (critical for numerical stability)
        b0 = b0_temp / a0_temp;
        b1 = b1_temp / a0_temp;
        b2 = b2_temp / a0_temp;
        a1 = a1_temp / a0_temp;
        a2 = a2_temp / a0_temp;
    }
};
