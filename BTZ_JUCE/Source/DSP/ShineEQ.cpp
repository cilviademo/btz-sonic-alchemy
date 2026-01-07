/*
  ShineEQ.cpp
*/

#include "ShineEQ.h"

ShineEQ::ShineEQ() = default;

void ShineEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    updateCoefficients();
}

void ShineEQ::reset()
{
    z1.fill(0.0f);
    z2.fill(0.0f);
    z1_out.fill(0.0f);
    z2_out.fill(0.0f);
}

void ShineEQ::setFrequency(float freqHz)
{
    frequencyHz = juce::jlimit(10000.0f, 80000.0f, freqHz);
    updateCoefficients();
}

void ShineEQ::setGain(float gainDbVal)
{
    gainDb = juce::jlimit(-12.0f, 12.0f, gainDbVal);
    updateCoefficients();
}

void ShineEQ::setQ(float q)
{
    qValue = juce::jlimit(0.1f, 2.0f, q);
    updateCoefficients();
}

void ShineEQ::setMix(float mix)
{
    mixAmount = juce::jlimit(0.0f, 1.0f, mix);
}

void ShineEQ::updateCoefficients()
{
    // High-shelf biquad filter design (RBJ Audio EQ Cookbook)
    float A = std::pow(10.0f, gainDb / 40.0f);
    float omega = juce::MathConstants<float>::twoPi * frequencyHz / static_cast<float>(sampleRate);
    float sn = std::sin(omega);
    float cs = std::cos(omega);
    float alpha = sn / (2.0f * qValue);

    float beta = std::sqrt(A + A);

    b0 = A * ((A + 1.0f) + (A - 1.0f) * cs + beta * alpha);
    b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs);
    b2 = A * ((A + 1.0f) + (A - 1.0f) * cs - beta * alpha);
    float a0 = (A + 1.0f) - (A - 1.0f) * cs + beta * alpha;
    a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cs);
    a2 = (A + 1.0f) - (A - 1.0f) * cs - beta * alpha;

    // Normalize by a0
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}
