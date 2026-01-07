/*
  TPTFilters.h

  Topology-Preserving Transform (TPT) filters
  Based on Vadim Zavalishin's "The Art of VA Filter Design" (2012)

  WHY TPT > Traditional Bilinear:
  - No frequency warping at high frequencies
  - More stable at parameter modulation
  - Better for virtual analog modeling
  - Smooth parameter changes without artifacts

  Used by: U-He, Arturia, Native Instruments for VA synthesis

  Source: https://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_2.1.0.pdf
*/

#pragma once
#include <JuceHeader.h>

//=============================================================================
// TPT ONE-POLE FILTER
// Perfect for envelope followers, smoothing, DC blocking
//=============================================================================
class TPTOnePole
{
public:
    enum class Type
    {
        LowPass,
        HighPass
    };

    TPTOnePole() = default;

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        s = 0.0f;
    }

    void setType(Type newType)
    {
        type = newType;
    }

    void setCutoff(float cutoffHz)
    {
        cutoffHz = juce::jlimit(1.0f, static_cast<float>(sampleRate * 0.49), cutoffHz);

        // TPT embedded integrator gain
        g = std::tan(juce::MathConstants<float>::pi * cutoffHz / static_cast<float>(sampleRate));
    }

    inline float process(float input)
    {
        // TPT one-pole processing
        float v = (input - s) * g / (1.0f + g);
        float lp = v + s;  // Low-pass output
        s = lp + v;        // State update (critical: after reading!)

        return (type == Type::LowPass) ? lp : (input - lp);  // HP = input - LP
    }

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numSamples = inputBlock.getNumSamples();

        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            auto* input = inputBlock.getChannelPointer(channel);
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                output[i] = process(input[i]);
            }
        }
    }

private:
    Type type = Type::LowPass;
    double sampleRate = 44100.0;
    float g = 0.0f;  // Embedded integrator gain
    float s = 0.0f;  // State variable
};

//=============================================================================
// TPT SVF (State Variable Filter)
// Simultaneously outputs LP, HP, BP, Notch, All-Pass
// Perfect for multi-mode filters
//=============================================================================
class TPTSVF
{
public:
    enum class Type
    {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass
    };

    TPTSVF() = default;

    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        s1 = s2 = 0.0f;
    }

    void setType(Type newType)
    {
        type = newType;
    }

    void setFrequency(float freqHz)
    {
        freqHz = juce::jlimit(10.0f, static_cast<float>(sampleRate * 0.49), freqHz);

        // TPT embedded integrator gain
        g = std::tan(juce::MathConstants<float>::pi * freqHz / static_cast<float>(sampleRate));
    }

    void setQ(float newQ)
    {
        Q = juce::jlimit(0.1f, 20.0f, newQ);

        // Damping = 1 / (2 * Q)
        k = 1.0f / (2.0f * Q);
    }

    inline float process(float input)
    {
        // TPT SVF equations (Zavalishin's optimized form)
        float hp = (input - (2.0f * k + g) * s1 - s2) / (1.0f + 2.0f * k * g + g * g);
        float bp = g * hp + s1;
        float lp = g * bp + s2;

        // State update (critical: after reading!)
        s1 = g * hp + bp;
        s2 = g * bp + lp;

        // Select output based on filter type
        switch (type)
        {
            case Type::LowPass:  return lp;
            case Type::HighPass: return hp;
            case Type::BandPass: return bp;
            case Type::Notch:    return lp + hp;
            case Type::AllPass:  return lp - 2.0f * k * bp + hp;
            default:             return lp;
        }
    }

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numSamples = inputBlock.getNumSamples();

        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            auto* input = inputBlock.getChannelPointer(channel);
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                output[i] = process(input[i]);
            }
        }
    }

private:
    Type type = Type::LowPass;
    double sampleRate = 44100.0;
    float g = 0.0f;    // Embedded integrator gain
    float k = 0.707f;  // Damping (1 / 2Q)
    float Q = 0.707f;  // Resonance

    // State variables (two integrators)
    float s1 = 0.0f;
    float s2 = 0.0f;
};

//=============================================================================
// DC BLOCKER (TPT-based)
// Removes DC offset without affecting low-frequency content
// Essential after any non-linear processing
//=============================================================================
class TPTDCBlocker
{
public:
    void prepare(double sampleRate)
    {
        // High-pass at 5Hz (removes DC, keeps bass)
        filter.prepare(sampleRate);
        filter.setType(TPTOnePole::Type::HighPass);
        filter.setCutoff(5.0f);
    }

    void reset()
    {
        filter.reset();
    }

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        filter.process(context);

        // Additional denormal killer
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Kill denormals (numbers near zero)
                if (std::abs(output[i]) < 1e-15f)
                    output[i] = 0.0f;
            }
        }
    }

private:
    TPTOnePole filter;
};
