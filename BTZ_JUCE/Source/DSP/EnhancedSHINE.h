/*
  ==============================================================================

  EnhancedSHINE.h

  Phase 1.2: Enhanced SHINE Air Band EQ
  - Psychoacoustic critical band analysis
  - Temporal masking (post-transient HF reduction)
  - Multi-band air enhancement (10kHz, 20kHz, 40kHz)
  - Spectral masking awareness

  Features:
  - 24 critical bands (Bark scale) for psychoacoustic analysis
  - Temporal masking: reduces harsh HF after transients
  - Triple-band shelving: 10kHz (presence), 20kHz (air), 40kHz (ultra-air)
  - Adaptive gain reduction based on spectral content

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "ParameterSmoother.h"
#include <array>
#include <vector>
#include <cmath>

class EnhancedSHINE
{
public:
    EnhancedSHINE();
    ~EnhancedSHINE() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    // Parameters
    void setShineAmount(float amount);          // 0.0 to 1.0 (0 = off, 1 = full)
    void setFrequencyCenter(float freqHz);      // 10k, 20k, or 40k center
    void setEnabled(bool enabled);
    void setPsychoacousticMode(bool enabled);   // Enable temporal masking

    // P1.1: Adaptive behavior
    void applyComponentVariance(const class ComponentVariance& variance);  // Apply analog variance to filters
    void applyFatigueReduction(float slowEnergy);  // Reduce HF based on long-term energy

    // Processing
    void process(juce::AudioBuffer<float>& buffer);

    // Analysis
    float getHFEnergy() const { return hfEnergyDb; }

private:
    // State
    bool isEnabled = true;
    bool psychoacousticEnabled = true;
    double sampleRate = 48000.0;
    int numChannels = 2;

    // Parameters
    ParameterSmootherFloat shineAmountSmooth;
    float shineAmount = 0.5f;
    float frequencyCenter = 20000.0f;

    // Triple-band shelving filters (10kHz, 20kHz, 40kHz)
    struct ShelfBand
    {
        juce::dsp::IIR::Filter<float> filter[2]; // Stereo
        float centerFreq;
        float gain;
    };
    std::array<ShelfBand, 3> shelfBands; // Low-air, Mid-air, Ultra-air

    // Critical band analyzer (24 Bark bands)
    static constexpr int numBarkBands = 24;
    struct BarkBandState
    {
        float energy = 0.0f;              // Current energy in band
        float threshold = 0.0f;           // Masking threshold
        juce::dsp::IIR::Filter<float> bandpassFilter[2]; // Stereo bandpass
    };
    std::array<BarkBandState, numBarkBands> barkBands;

    // Temporal masking state
    struct TemporalMaskingState
    {
        float transientEnvelope = 0.0f;   // Transient detector
        float maskingReduction = 0.0f;    // HF reduction amount (0-1)
        float lastSample = 0.0f;          // For derivative detection
    };
    std::array<TemporalMaskingState, 2> temporalState;

    // Masking parameters
    static constexpr float maskingAttackMs = 1.0f;    // Fast attack to detect transients
    static constexpr float maskingReleaseMs = 50.0f;  // Slow release (50ms post-masking)
    static constexpr float maskingDepthDb = -6.0f;    // Max HF reduction
    float maskingAttackCoeff = 0.0f;
    float maskingReleaseCoeff = 0.0f;

    // P1.1: Adaptive state
    float filterFreqVariance[3] = {1.0f, 1.0f, 1.0f};  // Per-band frequency variance
    float filterQVariance[3] = {1.0f, 1.0f, 1.0f};     // Per-band Q variance
    float fatigueReduction = 1.0f;  // HF reduction factor (1.0 = none, 0.0 = full reduction)

    // Metering
    float hfEnergyDb = -96.0f;

    // Internal methods
    void updateShelfCoefficients();
    void initializeBarkBands();
    float hzToBark(float hz) const;
    float barkToHz(float bark) const;
    void analyzeCriticalBands(const juce::AudioBuffer<float>& buffer);
    void updateTemporalMasking(const juce::AudioBuffer<float>& buffer);
    float calculateSpectralMaskingFactor() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnhancedSHINE)
};
