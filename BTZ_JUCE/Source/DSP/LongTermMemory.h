/*
  ==============================================================================

  LongTermMemory.h

  Phase 2.1: Long-Term Energy Integration
  - Slow-moving RMS envelope (300ms-2s time constants)
  - Hysteres integrator with memory decay
  - Context-aware processing (adjust based on program material)

  Features:
  - Multiple time scales: fast (100ms), medium (500ms), slow (2s)
  - Non-resetting envelopes maintain context across silence
  - Program-adaptive gain staging
  - Musical memory for natural dynamics

  Applications:
  - Saturation amount adapts to program loudness
  - Limiter threshold adjusts to recent peak history
  - Harmonic content varies with long-term energy
  - Preserves mix context across processing

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

class LongTermMemory
{
public:
    LongTermMemory();
    ~LongTermMemory() = default;

    void prepare(double sampleRate);
    void reset();

    // Update with new samples (call per-block)
    void update(const float* samples, int numSamples);

    // Get energy estimates at different time scales
    float getFastEnergy() const { return fastRMS; }      // ~100ms
    float getMediumEnergy() const { return mediumRMS; }  // ~500ms
    float getSlowEnergy() const { return slowRMS; }      // ~2s

    // Get derived metrics
    float getCrestFactor() const;           // Peak / RMS ratio
    float getDynamicRange() const;          // Historical peak / RMS
    float getProgramLoudness() const;       // Long-term integrated loudness

    // Memory decay (prevents permanent state buildup)
    void applyDecay();

    // Reset on transport stop (optional)
    void setNonResetting(bool enabled) { nonResetting = enabled; }

private:
    // Multi-scale RMS tracking
    float fastRMS = 0.0f;
    float mediumRMS = 0.0f;
    float slowRMS = 0.0f;

    // Peak tracking with decay
    float peakLevel = 0.0f;
    float historicalPeak = 0.0f;

    // Time constants (coefficients)
    float fastCoeff = 0.0f;
    float mediumCoeff = 0.0f;
    float slowCoeff = 0.0f;
    float peakDecayCoeff = 0.0f;

    // Non-resetting behavior
    bool nonResetting = true;
    float memoryFloor = 1.0e-6f; // Minimum energy level to maintain

    // Sample rate
    double sampleRate = 48000.0;

    // Calculate time constant coefficient
    static float calculateCoeff(double sr, float timeConstantMs);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LongTermMemory)
};

/*
  ==============================================================================
  AccelerationSensitiveDynamics - Detects transient acceleration
  ==============================================================================
*/

class AccelerationSensitiveDynamics
{
public:
    AccelerationSensitiveDynamics();

    void prepare(double sampleRate);
    void reset();

    // Process sample and return acceleration factor (0-1)
    // High values indicate rapid transients
    float processSample(float input);

    // Get current acceleration level
    float getAcceleration() const { return accelerationLevel; }

private:
    float lastSample = 0.0f;
    float lastVelocity = 0.0f;
    float accelerationLevel = 0.0f;

    // Smoothing coefficients
    float velocitySmooth = 0.0f;
    float accelSmooth = 0.0f;

    double sampleRate = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccelerationSensitiveDynamics)
};

/*
  ==============================================================================
  AdaptiveHarmonics - Program-dependent harmonic rotation
  ==============================================================================
*/

class AdaptiveHarmonics
{
public:
    AdaptiveHarmonics();

    void prepare(double sampleRate);
    void reset();

    // Update harmonic weights based on program content
    void updateFromMemory(const LongTermMemory& memory);

    // Get harmonic gain multipliers (0 = fundamental, 1 = 2nd, 2 = 3rd, etc.)
    float getHarmonicGain(int harmonic) const;

    // Rotation parameters
    void setRotationAmount(float amount); // 0 = static, 1 = full rotation

private:
    static constexpr int maxHarmonics = 8;
    std::array<float, maxHarmonics> harmonicGains;

    float rotationAmount = 0.5f;
    float programEnergy = 0.0f;

    // Rotate harmonics based on program content
    // Loud content: emphasize lower harmonics (warmth)
    // Quiet content: emphasize higher harmonics (detail)
    void rotateHarmonics();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdaptiveHarmonics)
};
