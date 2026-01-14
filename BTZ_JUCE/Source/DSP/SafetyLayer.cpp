/*
  ==============================================================================

  SafetyLayer.cpp

  ==============================================================================
*/

#include "SafetyLayer.h"
#include <xmmintrin.h> // For SSE denormal control

// ============================================================================
// DCBlocker Implementation
// ============================================================================

void DCBlocker::prepare(double sr)
{
    sampleRate = sr;
    updateCoefficients();
    reset();
}

void DCBlocker::reset()
{
    for (auto& s : state)
        s.s = 0.0f;
}

void DCBlocker::updateCoefficients()
{
    // TPT coefficient for first-order highpass
    // g = tan(pi * fc / fs)
    g = std::tan(juce::MathConstants<float>::pi * cutoffHz / static_cast<float>(sampleRate));
}

float DCBlocker::processSample(float input, int channel)
{
    auto& s = state[channel];

    // TPT first-order highpass
    // v = (input - s) / (1 + g)
    // output = input - v
    // s = s + 2*g*v

    float v = (input - s.s) / (1.0f + g);
    float output = input - v;
    s.s = s.s + 2.0f * g * v;

    return output;
}

void DCBlocker::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
            data[i] = processSample(data[i], ch);
    }
}

// ============================================================================
// DenormalGuard Implementation
// ============================================================================

DenormalGuard::DenormalGuard()
{
    enableFlushToZero();
}

void DenormalGuard::enableFlushToZero()
{
#if JUCE_USE_SSE_INTRINSICS
    // Enable FTZ (Flush-To-Zero) and DAZ (Denormals-Are-Zero)
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
}

void DenormalGuard::disableFlushToZero()
{
#if JUCE_USE_SSE_INTRINSICS
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
#endif
}

void DenormalGuard::process(juce::AudioBuffer<float>& buffer)
{
    // Add ultra-quiet noise to prevent denormals
    // This is a backup to FTZ/DAZ modes

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    // Use a simple counter-based pseudo-random noise
    static int noiseCounter = 0;

    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            // Generate bipolar noise: -1 or +1
            float noise = ((noiseCounter++ & 1) == 0) ? denormalNoise : -denormalNoise;
            data[i] += noise;
        }
    }
}

// ============================================================================
// NaNInfHandler Implementation
// ============================================================================

NaNInfHandler::NaNInfHandler() = default;

bool NaNInfHandler::checkAndFix(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    bool foundError = false;

    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            if (std::isnan(data[i]))
            {
                data[i] = 0.0f; // Replace NaN with silence
                nanCount++;
                foundError = true;
            }
            else if (std::isinf(data[i]))
            {
                data[i] = 0.0f; // Replace Inf with silence
                infCount++;
                foundError = true;
            }
        }
    }

    if (foundError)
    {
        logError("NaN/Inf detected and corrected");
    }

    return foundError;
}

void NaNInfHandler::resetCounts()
{
    nanCount = 0;
    infCount = 0;
}

void NaNInfHandler::logError(const juce::String& message)
{
    // RT-safe logging: just set a flag, actual logging happens on message thread
    // For now, we just increment counters (already done above)
    // In production, you'd use a lock-free queue to send messages to GUI

    DBG("SafetyLayer: " << message << " (NaN: " << nanCount.load()
        << ", Inf: " << infCount.load() << ")");
}

// ============================================================================
// ClickFreeSwitch Implementation
// ============================================================================

ClickFreeSwitch::ClickFreeSwitch() = default;

void ClickFreeSwitch::prepare(double sampleRate)
{
    // Calculate ramp increment for smooth transitions
    float rampSamples = static_cast<float>(sampleRate) * rampTimeMs * 0.001f;
    rampIncrement = 1.0f / rampSamples;
}

void ClickFreeSwitch::reset()
{
    currentGain = targetActive ? 1.0f : 0.0f;
    targetGain = currentGain;
    ramping = false;
}

void ClickFreeSwitch::setActive(bool shouldBeActive)
{
    if (targetActive == shouldBeActive)
        return;

    targetActive = shouldBeActive;
    targetGain = shouldBeActive ? 1.0f : 0.0f;
    ramping = true;
}

void ClickFreeSwitch::process(juce::AudioBuffer<float>& dryBuffer,
                               const juce::AudioBuffer<float>& wetBuffer)
{
    const int numSamples = dryBuffer.getNumSamples();
    const int channels = dryBuffer.getNumChannels();

    if (!ramping && currentGain == targetGain)
    {
        // No ramping needed
        if (targetActive)
        {
            // Copy wet to output
            for (int ch = 0; ch < channels; ++ch)
                dryBuffer.copyFrom(ch, 0, wetBuffer, ch, 0, numSamples);
        }
        // else: keep dry (already in dryBuffer)

        return;
    }

    // Ramp between dry and wet
    for (int ch = 0; ch < channels; ++ch)
    {
        float* dry = dryBuffer.getWritePointer(ch);
        const float* wet = wetBuffer.getReadPointer(ch);

        float gain = currentGain;

        for (int i = 0; i < numSamples; ++i)
        {
            // Crossfade: output = dry * (1-gain) + wet * gain
            dry[i] = dry[i] * (1.0f - gain) + wet[i] * gain;

            // Update gain
            if (ramping)
            {
                if (targetGain > currentGain)
                {
                    gain = std::min(gain + rampIncrement, targetGain);
                }
                else
                {
                    gain = std::max(gain - rampIncrement, targetGain);
                }

                if (std::abs(gain - targetGain) < 0.001f)
                {
                    gain = targetGain;
                    ramping = false;
                }
            }
        }

        currentGain = gain;
    }
}

// ============================================================================
// CompositeSafetyLayer Implementation
// ============================================================================

CompositeSafetyLayer::CompositeSafetyLayer() = default;

void CompositeSafetyLayer::prepare(double sampleRate, int samplesPerBlock)
{
    dcBlocker.prepare(sampleRate);

    // Enable FTZ/DAZ modes globally
    if (denormalGuardEnabled)
        DenormalGuard::enableFlushToZero();
}

void CompositeSafetyLayer::reset()
{
    dcBlocker.reset();
    nanInfHandler.resetCounts();
}

void CompositeSafetyLayer::process(juce::AudioBuffer<float>& buffer)
{
    // Apply safety layers in order:
    // 1. NaN/Inf check (first, to prevent propagation)
    // 2. DC blocker
    // 3. Denormal guard (last, after all processing)

    if (nanInfCheckEnabled)
    {
        nanInfHandler.checkAndFix(buffer);
    }

    if (dcBlockEnabled)
    {
        dcBlocker.process(buffer);
    }

    if (denormalGuardEnabled)
    {
        DenormalGuard::process(buffer);
    }
}
