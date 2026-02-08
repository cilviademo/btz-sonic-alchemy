/*
  ==============================================================================

  ComponentVariance.h

  Phase 1.3: Component Variance System
  - Per-instance deterministic randomization for analog character
  - ±2% variance on filters, saturation, L/R channel balance
  - Stored in preset state for recall consistency

  Features:
  - Deterministic variance: same seed = same character
  - Per-instance uniqueness: each plugin instance sounds slightly different
  - Subtle variances: ±0.5% to ±2% on key parameters
  - State persistence: variance values saved with presets
  - L/R asymmetry: subtle stereo depth from channel differences

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <random>
#include <cstdint>

class ComponentVariance
{
public:
    ComponentVariance();
    ~ComponentVariance() = default;

    // Variance control
    void setSeed(uint32_t seed);              // Set deterministic seed
    void randomizeSeed();                     // Generate new random seed
    uint32_t getSeed() const { return currentSeed; }

    void setVarianceAmount(float amount);     // 0.0 = no variance, 1.0 = full ±2%
    float getVarianceAmount() const { return varianceAmount; }

    // Get variance factors (1.0 ± variance)
    // All methods are deterministic based on seed

    // Filter variances (±2% max)
    float getFilterFrequencyVariance(int filterIndex) const;
    float getFilterQVariance(int filterIndex) const;

    // Saturation variances (±1.5% max)
    float getSaturationCurveVariance(int stageIndex) const;
    float getSaturationAsymmetryVariance(int stageIndex) const;

    // Channel balance variances (±0.5% max)
    float getLeftChannelGainVariance() const;
    float getRightChannelGainVariance() const;

    // Timing variances (±0.1ms max)
    float getChannelDelayVariance(int channel) const; // Returns delay in samples at 48kHz

    // Harmonic variances (±1% max)
    float getHarmonicGainVariance(int harmonic) const;

    // State management (for preset save/load)
    void getState(juce::MemoryBlock& destData) const;
    void setState(const void* data, int sizeInBytes);

private:
    uint32_t currentSeed = 0;
    float varianceAmount = 1.0f; // 0.0 to 1.0

    // Deterministic random number generation
    float getVariance(int index, float maxVariancePercent) const;
    mutable std::mt19937 rng; // Mersenne Twister RNG

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentVariance)
};

/*
  ==============================================================================

  VarianceState - Stored variance values for a single instance

  This structure holds all computed variance values so they remain
  consistent across prepare() calls and preset changes.

  ==============================================================================
*/

struct VarianceState
{
    // Filter variances
    std::array<float, 8> filterFreqVariances;
    std::array<float, 8> filterQVariances;

    // Saturation variances
    std::array<float, 4> saturationCurveVariances;
    std::array<float, 4> saturationAsymmetryVariances;

    // Channel balance
    float leftChannelGain;
    float rightChannelGain;

    // Timing
    float leftChannelDelay;
    float rightChannelDelay;

    // Harmonics
    std::array<float, 8> harmonicGainVariances;

    VarianceState()
    {
        filterFreqVariances.fill(1.0f);
        filterQVariances.fill(1.0f);
        saturationCurveVariances.fill(1.0f);
        saturationAsymmetryVariances.fill(1.0f);
        leftChannelGain = 1.0f;
        rightChannelGain = 1.0f;
        leftChannelDelay = 0.0f;
        rightChannelDelay = 0.0f;
        harmonicGainVariances.fill(1.0f);
    }
};
