/*
  ShineEQ.cpp
  NOW USES: Professional RBJ biquad filters
*/

#include "ShineEQ.h"

ShineEQ::ShineEQ()
{
    // Initialize both filters as high-shelf
    for (auto& filter : highShelfFilter)
    {
        filter.setType(RBJBiquad::FilterType::HighShelf);
    }
}

void ShineEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare all filters
    for (auto& filter : highShelfFilter)
    {
        filter.prepare(sampleRate);
    }

    reset();
    updateCoefficients();
}

void ShineEQ::reset()
{
    for (auto& filter : highShelfFilter)
    {
        filter.reset();
    }
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
    // Update all RBJ filters with new parameters
    for (auto& filter : highShelfFilter)
    {
        filter.setFrequency(frequencyHz);
        filter.setQ(qValue);
        filter.setGainDB(gainDb);
    }
}
