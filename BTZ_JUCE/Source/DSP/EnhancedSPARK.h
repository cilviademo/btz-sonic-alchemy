/*
  ==============================================================================

  EnhancedSPARK.h

  Phase 1.1: Enhanced SPARK Limiter
  - True-peak detection with oversampling
  - Hysteresis-based saturation (Jiles-Atherton inspired)
  - Adaptive oversampling based on content
  - Program-dependent harmonic rotation

  Features:
  - True-peak limiting prevents inter-sample peaks
  - Hysteresis adds analog-like memory and character
  - Adaptive OS engages only when needed (CPU efficient)
  - Quality tiers: Eco (1x), Normal (2x), High (4x)

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "OversamplingManager.h"
#include "ParameterSmoother.h"
#include <array>
#include <cmath>

class EnhancedSPARK
{
public:
    enum class QualityTier
    {
        Eco,     // 1x OS, light processing (8-12% CPU)
        Normal,  // 2x OS, full features (15-25% CPU)
        High     // 4x OS, maximum quality (30-50% CPU)
    };

    EnhancedSPARK();
    ~EnhancedSPARK() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    // Parameters
    void setCeiling(float ceilingDb);           // -3.0 to 0.0 dBTP (default: -0.3)
    void setEnabled(bool enabled);              // Enable/disable SPARK
    void setQualityTier(QualityTier tier);      // Eco/Normal/High
    void setAdaptiveOS(bool enabled);           // Auto-engage oversampling based on content

    // Processing
    void process(juce::AudioBuffer<float>& buffer);

    // Metering
    float getGainReduction() const { return gainReductionDb; }
    float getTruePeak() const { return truePeakDb; }
    int getCurrentOSFactor() const;

private:
    // State
    bool isEnabled = true;
    QualityTier currentTier = QualityTier::Normal;
    bool adaptiveOSEnabled = true;

    double sampleRate = 48000.0;
    int maxBlockSize = 512;
    int numChannels = 2;

    // Parameters
    ParameterSmootherFloat ceilingLinearSmooth;
    float ceilingDb = -0.3f;
    float ceilingLinear = 0.967f; // 10^(-0.3/20)

    // Oversampling
    OversamplingManager oversamplingManager;
    int baseOSFactor = 2; // Base factor from quality tier
    bool osCurrentlyEngaged = false;

    // Hysteresis state (simplified Jiles-Atherton model)
    struct HysteresisState
    {
        float magnetization = 0.0f;      // Current magnetic state (-1 to 1)
        float lastInput = 0.0f;          // Previous input for direction detection
        float anhystereticMag = 0.0f;    // Reversible magnetization
    };
    std::array<HysteresisState, 2> hysteresisState;

    // Hysteresis parameters (tuned for subtle analog character)
    static constexpr float Ms = 0.95f;    // Saturation magnetization
    static constexpr float a = 0.12f;     // Domain density (higher = more hysteresis)
    static constexpr float alpha = 0.08f; // Inter-domain coupling
    static constexpr float k = 0.25f;     // Coercivity (affects hysteresis width)
    static constexpr float c = 0.15f;     // Reversibility coefficient

    // True-peak detection
    float truePeakLinear = 0.0f;
    float truePeakDb = -96.0f;
    float gainReductionDb = 0.0f;

    // Adaptive OS detection
    float crestFactor = 1.0f;
    float peakEnvelope = 0.0f;
    float rmsEnvelope = 0.0f;
    static constexpr float crestThreshold = 3.0f; // Engage OS when crest > 3
    static constexpr float envelopeTC = 0.01f;    // 10ms time constant

    // Lookahead buffer for transient-aware limiting
    static constexpr int lookaheadSamples = 128;
    std::array<std::vector<float>, 2> lookaheadBuffer;
    std::array<int, 2> lookaheadWritePos = {0, 0};

    // Internal processing methods
    float processHysteresis(float input, int channel);
    float applyTruePeakLimiting(float sample);
    void updateAdaptiveOS(const juce::AudioBuffer<float>& buffer);
    void updateTruePeak(const juce::AudioBuffer<float>& buffer);
    void processWithOversampling(juce::AudioBuffer<float>& buffer);
    void processDirect(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnhancedSPARK)
};
