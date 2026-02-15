/*
  TransientShaper.cpp
*/

#include "TransientShaper.h"

TransientShaper::TransientShaper() = default;

void TransientShaper::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    updateCoefficients();
}

void TransientShaper::reset()
{
    envelopeState.fill(0.0f);
    previousEnvelope.fill(0.0f);
}

void TransientShaper::setPunch(float punchAmount)
{
    punchIntensity = punchAmount;
    updateCoefficients();
}

void TransientShaper::updateCoefficients()
{
    // Fast attack for transient detection (0.1ms)
    float attackTimeMs = 0.1f;
    attackCoeff = 1.0f - std::exp(-1.0f / (sampleRate * attackTimeMs / 1000.0f));

    // Slower release (10ms base, faster with more punch)
    float releaseTimeMs = 10.0f - (punchIntensity * 8.0f);
    releaseCoeff = 1.0f - std::exp(-1.0f / (sampleRate * releaseTimeMs / 1000.0f));
}
