/*
  Saturation.cpp
*/

#include "Saturation.h"

Saturation::Saturation() = default;

void Saturation::prepare(const juce::dsp::ProcessSpec& spec)
{
    reset();
}

void Saturation::reset()
{
    dcBlockerInput.fill(0.0f);
    dcBlockerOutput.fill(0.0f);
}

void Saturation::setWarmth(float warmthAmount)
{
    warmthIntensity = warmthAmount;
    // Increase saturation drive with warmth
    saturationDrive = 1.5f + (warmthAmount * 2.5f);
}
