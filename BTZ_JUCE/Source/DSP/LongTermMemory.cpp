/*
  ==============================================================================

  LongTermMemory.cpp

  ==============================================================================
*/

#include "LongTermMemory.h"

// ============================================================================
// LongTermMemory Implementation
// ============================================================================

LongTermMemory::LongTermMemory() = default;

void LongTermMemory::prepare(double sr)
{
    sampleRate = sr;

    // Calculate time constant coefficients
    fastCoeff = calculateCoeff(sampleRate, 100.0f);     // 100ms
    mediumCoeff = calculateCoeff(sampleRate, 500.0f);   // 500ms
    slowCoeff = calculateCoeff(sampleRate, 2000.0f);    // 2s
    peakDecayCoeff = calculateCoeff(sampleRate, 1000.0f); // 1s decay

    reset();
}

void LongTermMemory::reset()
{
    if (!nonResetting)
    {
        fastRMS = 0.0f;
        mediumRMS = 0.0f;
        slowRMS = 0.0f;
        peakLevel = 0.0f;
        historicalPeak = 0.0f;
    }
    else
    {
        // Keep above floor for context continuity
        fastRMS = std::max(fastRMS, memoryFloor);
        mediumRMS = std::max(mediumRMS, memoryFloor);
        slowRMS = std::max(slowRMS, memoryFloor);
    }
}

void LongTermMemory::update(const float* samples, int numSamples)
{
    // Calculate block RMS
    float sumSquares = 0.0f;
    float blockPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float absSample = std::abs(samples[i]);
        sumSquares += samples[i] * samples[i];
        blockPeak = std::max(blockPeak, absSample);
    }

    float blockRMS = std::sqrt(sumSquares / numSamples);

    // Update multi-scale RMS with different time constants
    fastRMS += fastCoeff * (blockRMS - fastRMS);
    mediumRMS += mediumCoeff * (blockRMS - mediumRMS);
    slowRMS += slowCoeff * (blockRMS - slowRMS);

    // Update peak with hold and decay
    if (blockPeak > peakLevel)
    {
        peakLevel = blockPeak;
        historicalPeak = std::max(historicalPeak, blockPeak);
    }
    else
    {
        peakLevel += peakDecayCoeff * (blockPeak - peakLevel);
    }

    // Ensure minimum floor for non-resetting mode
    if (nonResetting)
    {
        fastRMS = std::max(fastRMS, memoryFloor);
        mediumRMS = std::max(mediumRMS, memoryFloor);
        slowRMS = std::max(slowRMS, memoryFloor);
    }
}

float LongTermMemory::getCrestFactor() const
{
    if (fastRMS < 1.0e-6f)
        return 1.0f;

    return peakLevel / fastRMS;
}

float LongTermMemory::getDynamicRange() const
{
    if (slowRMS < 1.0e-6f)
        return 1.0f;

    return historicalPeak / slowRMS;
}

float LongTermMemory::getProgramLoudness() const
{
    // Weighted average favoring medium-term energy
    return 0.2f * fastRMS + 0.5f * mediumRMS + 0.3f * slowRMS;
}

void LongTermMemory::applyDecay()
{
    // Gentle decay to prevent permanent state buildup
    // Decays to floor over ~10 seconds
    constexpr float decayFactor = 0.9999f;

    fastRMS *= decayFactor;
    mediumRMS *= decayFactor;
    slowRMS *= decayFactor;
    historicalPeak *= decayFactor;

    if (nonResetting)
    {
        fastRMS = std::max(fastRMS, memoryFloor);
        mediumRMS = std::max(mediumRMS, memoryFloor);
        slowRMS = std::max(slowRMS, memoryFloor);
    }
}

float LongTermMemory::calculateCoeff(double sr, float timeConstantMs)
{
    // One-pole filter coefficient for exponential smoothing
    // Time constant = time to reach ~63% of target
    float timeSeconds = timeConstantMs * 0.001f;
    return 1.0f - std::exp(-1.0f / (static_cast<float>(sr) * timeSeconds));
}

// ============================================================================
// AccelerationSensitiveDynamics Implementation
// ============================================================================

AccelerationSensitiveDynamics::AccelerationSensitiveDynamics() = default;

void AccelerationSensitiveDynamics::prepare(double sr)
{
    sampleRate = sr;

    // Fast smoothing for velocity (1ms)
    velocitySmooth = 1.0f - std::exp(-1.0f / (static_cast<float>(sr) * 0.001f));

    // Medium smoothing for acceleration (5ms)
    accelSmooth = 1.0f - std::exp(-1.0f / (static_cast<float>(sr) * 0.005f));

    reset();
}

void AccelerationSensitiveDynamics::reset()
{
    lastSample = 0.0f;
    lastVelocity = 0.0f;
    accelerationLevel = 0.0f;
}

float AccelerationSensitiveDynamics::processSample(float input)
{
    // Calculate velocity (first derivative)
    float velocity = input - lastSample;
    lastSample = input;

    // Smooth velocity
    float smoothedVelocity = lastVelocity + velocitySmooth * (velocity - lastVelocity);
    lastVelocity = smoothedVelocity;

    // Calculate acceleration (second derivative)
    float acceleration = std::abs(smoothedVelocity);

    // Smooth acceleration
    accelerationLevel += accelSmooth * (acceleration - accelerationLevel);

    // Return normalized acceleration (clamped to 0-1)
    return juce::jlimit(0.0f, 1.0f, accelerationLevel * 10.0f);
}

// ============================================================================
// AdaptiveHarmonics Implementation
// ============================================================================

AdaptiveHarmonics::AdaptiveHarmonics()
{
    // Initialize with BTZ's default harmonic profile
    // 2nd > 3rd > 5th (warm, musical)
    harmonicGains[0] = 1.0f;  // Fundamental
    harmonicGains[1] = 0.8f;  // 2nd
    harmonicGains[2] = 0.6f;  // 3rd
    harmonicGains[3] = 0.3f;  // 4th
    harmonicGains[4] = 0.5f;  // 5th
    harmonicGains[5] = 0.2f;  // 6th
    harmonicGains[6] = 0.3f;  // 7th
    harmonicGains[7] = 0.1f;  // 8th
}

void AdaptiveHarmonics::prepare(double sr)
{
    // No sample-rate dependent initialization needed
    reset();
}

void AdaptiveHarmonics::reset()
{
    programEnergy = 0.0f;
}

void AdaptiveHarmonics::updateFromMemory(const LongTermMemory& memory)
{
    // Get program loudness for adaptive rotation
    programEnergy = memory.getProgramLoudness();

    // Rotate harmonics based on energy
    rotateHarmonics();
}

void AdaptiveHarmonics::rotateHarmonics()
{
    // Map program energy to rotation bias
    // Low energy (quiet): emphasize higher harmonics (detail, air)
    // High energy (loud): emphasize lower harmonics (warmth, density)

    float energyDb = juce::Decibels::gainToDecibels(programEnergy, -96.0f);

    // Normalize to 0-1 range (-60dB to -10dB)
    float energyNorm = (energyDb + 60.0f) / 50.0f;
    energyNorm = juce::jlimit(0.0f, 1.0f, energyNorm);

    // Calculate rotation factor (0 = emphasize highs, 1 = emphasize lows)
    float rotationFactor = energyNorm * rotationAmount;

    // Update harmonic gains with rotation
    // Low rotation: 2nd, 3rd, 5th dominant (BTZ signature)
    // High rotation: shift emphasis to lower harmonics

    harmonicGains[0] = 1.0f; // Fundamental always 1.0

    // 2nd harmonic: increase with loudness
    harmonicGains[1] = 0.8f + 0.2f * rotationFactor;

    // 3rd harmonic: stays relatively constant (musicality)
    harmonicGains[2] = 0.6f + 0.1f * rotationFactor;

    // 4th harmonic: decrease slightly with loudness
    harmonicGains[3] = 0.3f - 0.1f * rotationFactor;

    // 5th harmonic: BTZ signature - moderate level
    harmonicGains[4] = 0.5f;

    // Higher harmonics: decrease with loudness (reduce harshness)
    harmonicGains[5] = 0.2f * (1.0f - rotationFactor * 0.5f);
    harmonicGains[6] = 0.3f * (1.0f - rotationFactor * 0.5f);
    harmonicGains[7] = 0.1f * (1.0f - rotationFactor * 0.5f);
}

float AdaptiveHarmonics::getHarmonicGain(int harmonic) const
{
    if (harmonic < 0 || harmonic >= maxHarmonics)
        return 0.0f;

    return harmonicGains[static_cast<size_t>(harmonic)];
}

void AdaptiveHarmonics::setRotationAmount(float amount)
{
    rotationAmount = juce::jlimit(0.0f, 1.0f, amount);
}
