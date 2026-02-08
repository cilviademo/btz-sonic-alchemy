/*
  SparkLimiter.cpp
*/

#include "SparkLimiter.h"

SparkLimiter::SparkLimiter() = default;

void SparkLimiter::prepare(const juce::dsp::ProcessSpec& spec)
{
    reset();
}

void SparkLimiter::reset()
{
    for (auto& buffer : lookaheadBuffer)
        buffer.fill(0.0f);
    lookaheadIndex.fill(0);
}

void SparkLimiter::setTargetLUFS(float lufs)
{
    targetLUFS = juce::jlimit(-14.0f, 0.0f, lufs);
}

void SparkLimiter::setCeiling(float ceiling)
{
    ceilingDb = juce::jlimit(-3.0f, 0.0f, ceiling);
    ceilingLinear = juce::Decibels::decibelsToGain(ceilingDb);
}

void SparkLimiter::setMix(float mix)
{
    mixAmount = juce::jlimit(0.0f, 1.0f, mix);
}

void SparkLimiter::setOversamplingFactor(int factor)
{
    // Validate factor (1, 2, 4, 8, or 16)
    if (factor == 1 || factor == 2 || factor == 4 || factor == 8 || factor == 16)
        oversamplingFactor = factor;
}

void SparkLimiter::setMode(Mode mode)
{
    currentMode = mode;
}
