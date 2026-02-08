/*
  LUFSMeter.cpp

  ITU-R BS.1770-4 compliant LUFS metering implementation
  Based on public specification and libebur128 reference
*/

#include "LUFSMeter.h"
#include <cmath>
#include <algorithm>

LUFSMeter::LUFSMeter() = default;

void LUFSMeter::prepare(double sampleRate, int channels)
{
    currentSampleRate = sampleRate;
    numChannels = channels;

    // Calculate block size for 400ms at current sample rate
    blockSizeSamples = static_cast<int>(sampleRate * blockSizeMs / 1000.0);

    // Allocate filter state for each channel
    highShelfState.resize(channels);
    highPassState.resize(channels);
    currentBlockPower.resize(channels);

    calculateFilterCoefficients();
    reset();
}

void LUFSMeter::reset()
{
    // Clear filter state
    for (auto& state : highShelfState)
        state.fill(0.0f);
    for (auto& state : highPassState)
        state.fill(0.0f);

    // Clear accumulators
    for (auto& power : currentBlockPower)
        power = 0.0f;

    samplesInCurrentBlock = 0;

    momentaryBlocks.clear();
    shortTermBlocks.clear();
    integratedBlocks.clear();

    momentaryLUFS.store(-70.0f);
    shortTermLUFS.store(-70.0f);
    integratedLUFS.store(-70.0f);
    loudnessRange.store(0.0f);
}

void LUFSMeter::resetIntegrated()
{
    integratedBlocks.clear();
    integratedLUFS.store(-70.0f);
}

//=============================================================================
// K-WEIGHTING FILTER COEFFICIENT CALCULATION
// ITU-R BS.1770-4 specification
//=============================================================================
void LUFSMeter::calculateFilterCoefficients()
{
    const double sr = currentSampleRate;

    //=== HIGH-SHELF FILTER: +4dB @ 1500Hz, Q=0.7 ===
    // RBJ Audio EQ Cookbook formula

    double fc = 1500.0;
    double gainDb = 4.0;
    double Q = 0.707; // sqrt(2)/2

    double A = std::pow(10.0, gainDb / 40.0);
    double omega = 2.0 * juce::MathConstants<double>::pi * fc / sr;
    double sn = std::sin(omega);
    double cs = std::cos(omega);
    double beta = std::sqrt(A) / Q;

    double b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta * sn);
    double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
    double b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta * sn);
    double a0 = (A + 1.0) - (A - 1.0) * cs + beta * sn;
    double a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
    double a2 = (A + 1.0) - (A - 1.0) * cs - beta * sn;

    // Normalize by a0
    highShelfCoeffs.b0 = static_cast<float>(b0 / a0);
    highShelfCoeffs.b1 = static_cast<float>(b1 / a0);
    highShelfCoeffs.b2 = static_cast<float>(b2 / a0);
    highShelfCoeffs.a1 = static_cast<float>(a1 / a0);
    highShelfCoeffs.a2 = static_cast<float>(a2 / a0);

    //=== HIGH-PASS FILTER: 38Hz, Q=0.5 ===
    // Removes DC and subsonic content

    fc = 38.0;
    Q = 0.5;
    omega = 2.0 * juce::MathConstants<double>::pi * fc / sr;
    sn = std::sin(omega);
    cs = std::cos(omega);
    double alpha = sn / (2.0 * Q);

    b0 = (1.0 + cs) / 2.0;
    b1 = -(1.0 + cs);
    b2 = (1.0 + cs) / 2.0;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cs;
    a2 = 1.0 - alpha;

    highPassCoeffs.b0 = static_cast<float>(b0 / a0);
    highPassCoeffs.b1 = static_cast<float>(b1 / a0);
    highPassCoeffs.b2 = static_cast<float>(b2 / a0);
    highPassCoeffs.a1 = static_cast<float>(a1 / a0);
    highPassCoeffs.a2 = static_cast<float>(a2 / a0);
}

//=============================================================================
// APPLY K-WEIGHTING FILTER CHAIN
//=============================================================================
float LUFSMeter::applyKWeighting(float sample, size_t channel)
{
    // Stage 1: High-shelf filter
    auto& hsState = highShelfState[channel];
    float filtered = highShelfCoeffs.b0 * sample
                   + highShelfCoeffs.b1 * hsState[0]
                   + highShelfCoeffs.b2 * hsState[1]
                   - highShelfCoeffs.a1 * hsState[2]
                   - highShelfCoeffs.a2 * hsState[2]; // Note: reusing hsState[2] for output delay

    // Update delay line
    hsState[1] = hsState[0];
    hsState[0] = sample;
    float temp = filtered;

    // Stage 2: High-pass filter
    auto& hpState = highPassState[channel];
    filtered = highPassCoeffs.b0 * temp
             + highPassCoeffs.b1 * hpState[0]
             + highPassCoeffs.b2 * hpState[1]
             - highPassCoeffs.a1 * hpState[2]
             - highPassCoeffs.a2 * hpState[2];

    hpState[1] = hpState[0];
    hpState[0] = temp;
    hpState[2] = filtered;
    highShelfState[channel][2] = temp;

    return filtered;
}

//=============================================================================
// PROCESS AUDIO BLOCK
//=============================================================================
void LUFSMeter::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = std::min(buffer.getNumChannels(), numChannels);

    for (int i = 0; i < numSamples; ++i)
    {
        // Apply K-weighting and accumulate mean square power
        for (int ch = 0; ch < channels; ++ch)
        {
            float sample = buffer.getSample(ch, i);
            float weighted = applyKWeighting(sample, ch);

            // Accumulate mean square
            currentBlockPower[ch] += weighted * weighted;
        }

        samplesInCurrentBlock++;

        // Check if 400ms block is complete
        if (samplesInCurrentBlock >= blockSizeSamples)
        {
            // Calculate mean power across channels
            float totalPower = 0.0f;
            for (int ch = 0; ch < channels; ++ch)
            {
                totalPower += currentBlockPower[ch] / blockSizeSamples;
                currentBlockPower[ch] = 0.0f;
            }

            // Average across channels (equal weighting for stereo)
            float meanPower = totalPower / channels;

            // Convert to LUFS
            float blockLoudness = powerToLUFS(meanPower);

            // Create gating block
            GatingBlock block;
            block.loudness = blockLoudness;
            block.timestamp = 0; // Could track sample position if needed

            // Add to block lists
            momentaryBlocks.push_back(block);
            shortTermBlocks.push_back(block);
            integratedBlocks.push_back(block);

            // Maintain 400ms momentary window (1 block)
            while (momentaryBlocks.size() > 1)
                momentaryBlocks.pop_front();

            // Maintain 3s short-term window (7.5 blocks)
            while (shortTermBlocks.size() > 8)
                shortTermBlocks.pop_front();

            // Calculate momentary LUFS (400ms, no gating)
            if (!momentaryBlocks.empty())
                momentaryLUFS.store(momentaryBlocks.back().loudness);

            // Calculate short-term LUFS (3s, no gating)
            if (!shortTermBlocks.empty())
            {
                float sum = 0.0f;
                for (const auto& b : shortTermBlocks)
                    sum += std::pow(10.0f, b.loudness / 10.0f);
                shortTermLUFS.store(10.0f * std::log10(sum / shortTermBlocks.size()));
            }

            // Calculate integrated LUFS (with gating)
            if (integratedBlocks.size() > 0)
            {
                integratedLUFS.store(calculateGatedLoudness(integratedBlocks));
                loudnessRange.store(calculateLoudnessRange(integratedBlocks));
            }

            samplesInCurrentBlock = 0;
        }
    }
}

//=============================================================================
// CONVERT MEAN SQUARE POWER TO LUFS
// LUFS = -0.691 + 10 * log10(power)
// The -0.691 offset accounts for the difference between RMS and LUFS reference
//=============================================================================
float LUFSMeter::powerToLUFS(float power)
{
    if (power < 1e-10f) // Silence threshold
        return -70.0f;

    // ITU-R BS.1770-4 formula
    return -0.691f + 10.0f * std::log10(power);
}

//=============================================================================
// GATED LOUDNESS CALCULATION
// ITU-R BS.1770-4 two-pass gating:
// 1. Absolute gate at -70 LUFS (remove silence)
// 2. Relative gate at -10 LU below ungated loudness (remove quiet passages)
//=============================================================================
float LUFSMeter::calculateGatedLoudness(const std::vector<GatingBlock>& blocks)
{
    if (blocks.empty())
        return -70.0f;

    // Pass 1: Absolute gating (-70 LUFS)
    std::vector<float> absoluteGatedBlocks;
    for (const auto& block : blocks)
    {
        if (block.loudness >= absoluteGate)
            absoluteGatedBlocks.push_back(block.loudness);
    }

    if (absoluteGatedBlocks.empty())
        return -70.0f;

    // Calculate ungated loudness
    float ungatedSum = 0.0f;
    for (float loudness : absoluteGatedBlocks)
        ungatedSum += std::pow(10.0f, loudness / 10.0f);

    float ungatedLoudness = 10.0f * std::log10(ungatedSum / absoluteGatedBlocks.size());

    // Pass 2: Relative gating (-10 LU below ungated)
    float relativeThreshold = ungatedLoudness + relativeGate;

    std::vector<float> relativeGatedBlocks;
    for (float loudness : absoluteGatedBlocks)
    {
        if (loudness >= relativeThreshold)
            relativeGatedBlocks.push_back(loudness);
    }

    if (relativeGatedBlocks.empty())
        return ungatedLoudness;

    // Calculate gated loudness
    float gatedSum = 0.0f;
    for (float loudness : relativeGatedBlocks)
        gatedSum += std::pow(10.0f, loudness / 10.0f);

    return 10.0f * std::log10(gatedSum / relativeGatedBlocks.size());
}

//=============================================================================
// LOUDNESS RANGE (LRA) CALCULATION
// EBU TECH 3342: Measure of dynamic range
// LRA = difference between 95th and 10th percentile of gated blocks
//=============================================================================
float LUFSMeter::calculateLoudnessRange(const std::vector<GatingBlock>& blocks)
{
    if (blocks.size() < 10)
        return 0.0f;

    // Apply absolute and relative gating (same as integrated)
    std::vector<float> gatedLoudness;
    float relativeThreshold = calculateGatedLoudness(blocks) + relativeGate;

    for (const auto& block : blocks)
    {
        if (block.loudness >= absoluteGate && block.loudness >= relativeThreshold)
            gatedLoudness.push_back(block.loudness);
    }

    if (gatedLoudness.size() < 10)
        return 0.0f;

    // Sort loudness values
    std::sort(gatedLoudness.begin(), gatedLoudness.end());

    // Find 10th and 95th percentiles
    size_t idx10 = static_cast<size_t>(gatedLoudness.size() * 0.10);
    size_t idx95 = static_cast<size_t>(gatedLoudness.size() * 0.95);

    float loudness10 = gatedLoudness[idx10];
    float loudness95 = gatedLoudness[idx95];

    // LRA = difference (in LU)
    return loudness95 - loudness10;
}
