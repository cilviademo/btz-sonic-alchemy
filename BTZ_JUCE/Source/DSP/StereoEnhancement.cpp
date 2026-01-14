/*
  ==============================================================================

  StereoEnhancement.cpp

  ==============================================================================
*/

#include "StereoEnhancement.h"

// ============================================================================
// StereoMicroDrift Implementation
// ============================================================================

StereoMicroDrift::StereoMicroDrift() = default;

void StereoMicroDrift::prepare(double sr, int maxSamples)
{
    sampleRate = sr;
    maxBlockSize = maxSamples;

    // Allocate micro-delay buffers (max 1ms delay = SR/1000 samples)
    int maxDelaySamples = static_cast<int>(sampleRate / 1000.0) + 1;

    for (auto& delay : microDelays)
    {
        delay.buffer.resize(maxDelaySamples, 0.0f);
        delay.writePos = 0;
        delay.delayAmount = 0.0f;
    }

    // Prepare all-pass filters for subtle phase shifts
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxSamples);
    spec.numChannels = 1;

    for (auto& filter : allPassFilters)
    {
        filter.prepare(spec);
        filter.reset();
    }

    updateAllPassFilters();

    // Calculate correlation smoothing coefficient
    correlationSmooth = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.1f));

    reset();
}

void StereoMicroDrift::reset()
{
    for (auto& delay : microDelays)
    {
        std::fill(delay.buffer.begin(), delay.buffer.end(), 0.0f);
        delay.writePos = 0;
    }

    for (auto& filter : allPassFilters)
        filter.reset();

    lfoPhase[0] = 0.0f;
    lfoPhase[1] = 0.25f; // 90° phase offset
    stereoCorrelation = 1.0f;
}

void StereoMicroDrift::setDriftAmount(float amount)
{
    driftAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void StereoMicroDrift::setDepthAmount(float amount)
{
    depthAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void StereoMicroDrift::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (channels < 2 || numSamples == 0)
        return;

    // Update LFOs for time-varying drift
    updateMicroDelays();

    // Process each channel with micro-delays and all-pass filtering
    for (int ch = 0; ch < 2; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = data[i];

            // Apply micro-delay
            float delayed = processMicroDelay(sample, ch);

            // Apply all-pass filtering for phase variation
            float filtered = allPassFilters[ch].processSample(delayed);

            // Mix dry/wet based on drift amount
            data[i] = sample + driftAmount * (filtered - sample);
        }
    }

    // Apply stereo width adjustment
    // Simple M/S processing
    if (depthAmount != 0.5f) // 0.5 = neutral
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float left = buffer.getSample(0, i);
            float right = buffer.getSample(1, i);

            // Convert to M/S
            float mid = (left + right) * 0.5f;
            float side = (left - right) * 0.5f;

            // Scale side signal (depth control)
            float widthFactor = 1.0f + (depthAmount - 0.5f) * 2.0f;
            side *= juce::jlimit(0.0f, 2.0f, widthFactor);

            // Convert back to L/R
            buffer.setSample(0, i, mid + side);
            buffer.setSample(1, i, mid - side);
        }
    }

    // Measure stereo correlation
    measureCorrelation(buffer);
}

void StereoMicroDrift::updateMicroDelays()
{
    // Update LFO phases
    float lfoIncrement = lfoRate / static_cast<float>(sampleRate);

    for (int ch = 0; ch < 2; ++ch)
    {
        lfoPhase[ch] += lfoIncrement;
        if (lfoPhase[ch] >= 1.0f)
            lfoPhase[ch] -= 1.0f;

        // Calculate delay amount with slow LFO modulation
        // Max delay: ±0.05ms = ±(SR * 0.00005) samples
        float maxDelaySamples = static_cast<float>(sampleRate) * 0.00005f;

        float lfoValue = std::sin(juce::MathConstants<float>::twoPi * lfoPhase[ch]);
        microDelays[ch].delayAmount = lfoValue * maxDelaySamples * driftAmount;
    }
}

void StereoMicroDrift::updateAllPassFilters()
{
    // Design subtle all-pass filters with slightly different frequencies
    // Creates micro-phase shifts without affecting amplitude

    auto designAllPass = [this](int channel, float centerFreq)
    {
        // First-order all-pass for subtle phase shift
        float freq = centerFreq * (1.0f + (channel == 0 ? 0.002f : -0.002f)); // ±0.2%

        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, freq);
        *allPassFilters[channel].coefficients = *coeffs;
    };

    designAllPass(0, 1000.0f); // Left: 1002 Hz
    designAllPass(1, 1000.0f); // Right: 998 Hz
}

float StereoMicroDrift::processMicroDelay(float input, int channel)
{
    auto& delay = microDelays[channel];

    // Write input
    delay.buffer[delay.writePos] = input;

    // Calculate read position with fractional delay
    float readPosFloat = static_cast<float>(delay.writePos) - std::abs(delay.delayAmount);

    // Wrap read position
    while (readPosFloat < 0.0f)
        readPosFloat += static_cast<float>(delay.buffer.size());

    int readPosInt = static_cast<int>(readPosFloat);
    float frac = readPosFloat - static_cast<float>(readPosInt);

    // Linear interpolation for fractional delay
    int nextPos = (readPosInt + 1) % static_cast<int>(delay.buffer.size());
    float output = delay.buffer[readPosInt] * (1.0f - frac) +
                   delay.buffer[nextPos] * frac;

    // Advance write position
    delay.writePos = (delay.writePos + 1) % static_cast<int>(delay.buffer.size());

    return output;
}

void StereoMicroDrift::measureCorrelation(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2)
        return;

    const int numSamples = buffer.getNumSamples();
    const float* left = buffer.getReadPointer(0);
    const float* right = buffer.getReadPointer(1);

    // Calculate correlation coefficient
    float sumLR = 0.0f;
    float sumLL = 0.0f;
    float sumRR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        sumLR += left[i] * right[i];
        sumLL += left[i] * left[i];
        sumRR += right[i] * right[i];
    }

    float correlation = 0.0f;
    float denominator = std::sqrt(sumLL * sumRR);
    if (denominator > 1.0e-6f)
        correlation = sumLR / denominator;

    // Smooth correlation measurement
    stereoCorrelation += correlationSmooth * (correlation - stereoCorrelation);
}

// ============================================================================
// StereoWidth Implementation
// ============================================================================

StereoWidth::StereoWidth() = default;

void StereoWidth::prepare(double sr)
{
    // No sample-rate dependent initialization needed
    reset();
}

void StereoWidth::reset()
{
    // No state to reset
}

void StereoWidth::setWidth(float width)
{
    widthAmount = juce::jlimit(0.0f, maxWidth, width);
}

void StereoWidth::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (channels < 2 || numSamples == 0)
        return;

    if (std::abs(widthAmount - 1.0f) < 0.01f)
        return; // No processing needed

    for (int i = 0; i < numSamples; ++i)
    {
        float left = buffer.getSample(0, i);
        float right = buffer.getSample(1, i);

        // M/S processing
        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;

        // Scale side signal
        side *= widthAmount;

        // Convert back to L/R
        buffer.setSample(0, i, mid + side);
        buffer.setSample(1, i, mid - side);
    }
}

// ============================================================================
// CompositeStereoEnhancement Implementation
// ============================================================================

CompositeStereoEnhancement::CompositeStereoEnhancement() = default;

void CompositeStereoEnhancement::prepare(double sampleRate, int maxSamplesPerBlock)
{
    microDrift.prepare(sampleRate, maxSamplesPerBlock);
    stereoWidth.prepare(sampleRate);
}

void CompositeStereoEnhancement::reset()
{
    microDrift.reset();
    stereoWidth.reset();
}

void CompositeStereoEnhancement::process(juce::AudioBuffer<float>& buffer)
{
    // Apply in order: micro-drift, then width
    microDrift.process(buffer);
    stereoWidth.process(buffer);
}
