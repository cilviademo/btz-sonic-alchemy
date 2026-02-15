/*
  ==============================================================================

  ComponentVariance.cpp

  ==============================================================================
*/

#include "ComponentVariance.h"

ComponentVariance::ComponentVariance()
{
    // Generate random seed on construction for per-instance uniqueness
    randomizeSeed();
}

void ComponentVariance::setSeed(uint32_t seed)
{
    currentSeed = seed;
    rng.seed(currentSeed);
}

void ComponentVariance::randomizeSeed()
{
    // Use high-resolution time + random device for unique seed
    std::random_device rd;
    uint32_t timeSeed = static_cast<uint32_t>(juce::Time::currentTimeMillis());
    currentSeed = rd() ^ timeSeed;
    rng.seed(currentSeed);
}

void ComponentVariance::setVarianceAmount(float amount)
{
    varianceAmount = juce::jlimit(0.0f, 1.0f, amount);
}

float ComponentVariance::getVariance(int index, float maxVariancePercent) const
{
    // Reset RNG to deterministic state based on seed and index
    std::mt19937 localRng(currentSeed + static_cast<uint32_t>(index));

    // Generate random value in range [-maxVariance, +maxVariance]
    std::uniform_real_distribution<float> dist(-maxVariancePercent, maxVariancePercent);
    float variancePercent = dist(localRng);

    // Scale by variance amount
    variancePercent *= varianceAmount;

    // Convert to multiplier: 0% = 1.0, +2% = 1.02, -2% = 0.98
    return 1.0f + (variancePercent / 100.0f);
}

// Filter variances (±2% max)
float ComponentVariance::getFilterFrequencyVariance(int filterIndex) const
{
    return getVariance(1000 + filterIndex, 2.0f);
}

float ComponentVariance::getFilterQVariance(int filterIndex) const
{
    return getVariance(2000 + filterIndex, 2.0f);
}

// Saturation variances (±1.5% max)
float ComponentVariance::getSaturationCurveVariance(int stageIndex) const
{
    return getVariance(3000 + stageIndex, 1.5f);
}

float ComponentVariance::getSaturationAsymmetryVariance(int stageIndex) const
{
    // Asymmetry variance: shifts the saturation curve slightly
    // Returns a value from -0.015 to +0.015
    std::mt19937 localRng(currentSeed + 4000 + static_cast<uint32_t>(stageIndex));
    std::uniform_real_distribution<float> dist(-0.015f, 0.015f);
    return dist(localRng) * varianceAmount;
}

// Channel balance variances (±0.5% max)
float ComponentVariance::getLeftChannelGainVariance() const
{
    return getVariance(5000, 0.5f);
}

float ComponentVariance::getRightChannelGainVariance() const
{
    return getVariance(5001, 0.5f);
}

// Timing variances (±0.1ms max at 48kHz = ±4.8 samples)
float ComponentVariance::getChannelDelayVariance(int channel) const
{
    std::mt19937 localRng(currentSeed + 6000 + static_cast<uint32_t>(channel));
    std::uniform_real_distribution<float> dist(-4.8f, 4.8f); // Samples at 48kHz
    return dist(localRng) * varianceAmount;
}

// Harmonic variances (±1% max)
float ComponentVariance::getHarmonicGainVariance(int harmonic) const
{
    return getVariance(7000 + harmonic, 1.0f);
}

void ComponentVariance::getState(juce::MemoryBlock& destData) const
{
    // Serialize state: seed + variance amount
    juce::MemoryOutputStream stream(destData, false);

    stream.writeInt(static_cast<int>(currentSeed));
    stream.writeFloat(varianceAmount);
}

void ComponentVariance::setState(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes < 8)
    {
        // Invalid state, randomize
        randomizeSeed();
        return;
    }

    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

    currentSeed = static_cast<uint32_t>(stream.readInt());
    varianceAmount = stream.readFloat();

    // Reseed RNG
    rng.seed(currentSeed);
}
