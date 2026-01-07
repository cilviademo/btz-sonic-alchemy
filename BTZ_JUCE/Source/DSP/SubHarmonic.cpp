/*
  SubHarmonic.cpp
*/

#include "SubHarmonic.h"

SubHarmonic::SubHarmonic() = default;

void SubHarmonic::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    updateCoefficients();
}

void SubHarmonic::reset()
{
    lowpassState.fill(0.0f);
    previousSample.fill(0.0f);
    subPhase.fill(0.0f);
}

void SubHarmonic::setBoom(float boomAmount)
{
    boomIntensity = boomAmount;
}

void SubHarmonic::updateCoefficients()
{
    // Lowpass filter at 150Hz
    float cutoffFreq = 150.0f;
    float rc = 1.0f / (juce::MathConstants<float>::twoPi * cutoffFreq);
    float dt = 1.0f / static_cast<float>(sampleRate);
    lowpassCoeff = dt / (rc + dt);
}
