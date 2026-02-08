/*
  ==============================================================================

  EnhancedSPARK.cpp

  ==============================================================================
*/

#include "EnhancedSPARK.h"

EnhancedSPARK::EnhancedSPARK()
{
}

void EnhancedSPARK::prepare(double sr, int samplesPerBlock, int channels)
{
    sampleRate = sr;
    maxBlockSize = samplesPerBlock;
    numChannels = channels;

    // Prepare oversampling manager
    oversamplingManager.prepare(sampleRate, samplesPerBlock, channels);
    oversamplingManager.setQuality(OversamplingManager::Quality::Good);

    // Set initial oversampling factor based on quality tier
    switch (currentTier)
    {
        case QualityTier::Eco:
            baseOSFactor = 1;
            oversamplingManager.setFactor(OversamplingManager::Factor::x1);
            break;
        case QualityTier::Normal:
            baseOSFactor = 2;
            oversamplingManager.setFactor(OversamplingManager::Factor::x2);
            break;
        case QualityTier::High:
            baseOSFactor = 4;
            oversamplingManager.setFactor(OversamplingManager::Factor::x4);
            break;
    }

    // Prepare parameter smoother
    ceilingLinearSmooth.prepare(sampleRate, 0.02f); // 20ms ramp
    ceilingLinearSmooth.reset(ceilingLinear);

    // Initialize lookahead buffers
    for (int ch = 0; ch < 2; ++ch)
    {
        lookaheadBuffer[ch].resize(lookaheadSamples);
        std::fill(lookaheadBuffer[ch].begin(), lookaheadBuffer[ch].end(), 0.0f);
        lookaheadWritePos[ch] = 0;
    }

    reset();
}

void EnhancedSPARK::reset()
{
    // Reset hysteresis state
    for (auto& state : hysteresisState)
    {
        state.magnetization = 0.0f;
        state.lastInput = 0.0f;
        state.anhystereticMag = 0.0f;
    }

    // Reset lookahead buffers
    for (auto& buffer : lookaheadBuffer)
        std::fill(buffer.begin(), buffer.end(), 0.0f);

    lookaheadWritePos.fill(0);

    // Reset oversampling
    oversamplingManager.reset();

    // Reset metering
    truePeakLinear = 0.0f;
    truePeakDb = -96.0f;
    gainReductionDb = 0.0f;
    peakEnvelope = 0.0f;
    rmsEnvelope = 0.0f;
}

void EnhancedSPARK::setCeiling(float ceilingDbValue)
{
    ceilingDb = juce::jlimit(-3.0f, 0.0f, ceilingDbValue);
    ceilingLinear = juce::Decibels::decibelsToGain(ceilingDb);
    ceilingLinearSmooth.setTarget(ceilingLinear);
}

void EnhancedSPARK::setEnabled(bool enabled)
{
    isEnabled = enabled;
}

void EnhancedSPARK::setQualityTier(QualityTier tier)
{
    if (currentTier == tier)
        return;

    currentTier = tier;

    // Update base oversampling factor
    switch (tier)
    {
        case QualityTier::Eco:
            baseOSFactor = 1;
            oversamplingManager.setFactor(OversamplingManager::Factor::x1);
            break;
        case QualityTier::Normal:
            baseOSFactor = 2;
            oversamplingManager.setFactor(OversamplingManager::Factor::x2);
            break;
        case QualityTier::High:
            baseOSFactor = 4;
            oversamplingManager.setFactor(OversamplingManager::Factor::x4);
            break;
    }
}

void EnhancedSPARK::setAdaptiveOS(bool enabled)
{
    adaptiveOSEnabled = enabled;
}

int EnhancedSPARK::getCurrentOSFactor() const
{
    return oversamplingManager.getOversamplingFactor();
}

void EnhancedSPARK::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled)
        return;

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (numSamples == 0 || channels == 0)
        return;

    // Update adaptive oversampling decision
    if (adaptiveOSEnabled)
        updateAdaptiveOS(buffer);

    // Process with or without oversampling
    if (oversamplingManager.isEnabled())
        processWithOversampling(buffer);
    else
        processDirect(buffer);

    // Update true-peak metering (on oversampled signal)
    updateTruePeak(buffer);
}

void EnhancedSPARK::updateAdaptiveOS(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    // Calculate crest factor to detect transient-rich content
    float peakLevel = 0.0f;
    float rmsLevel = 0.0f;

    for (int ch = 0; ch < channels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float absSample = std::abs(data[i]);
            peakLevel = std::max(peakLevel, absSample);
            rmsLevel += data[i] * data[i];
        }
    }

    rmsLevel = std::sqrt(rmsLevel / (numSamples * channels));

    // Smooth with envelope followers
    peakEnvelope += envelopeTC * (peakLevel - peakEnvelope);
    rmsEnvelope += envelopeTC * (rmsLevel - rmsEnvelope);

    // Calculate crest factor
    if (rmsEnvelope > 0.0001f)
        crestFactor = peakEnvelope / rmsEnvelope;
    else
        crestFactor = 1.0f;

    // Engage higher oversampling for transient-rich content
    if (currentTier != QualityTier::Eco)
    {
        bool shouldEngageHighOS = crestFactor > crestThreshold;

        if (shouldEngageHighOS && !osCurrentlyEngaged)
        {
            // Engage 4x for high transient content
            oversamplingManager.setFactor(OversamplingManager::Factor::x4);
            osCurrentlyEngaged = true;
        }
        else if (!shouldEngageHighOS && osCurrentlyEngaged)
        {
            // Return to base factor
            if (currentTier == QualityTier::Normal)
                oversamplingManager.setFactor(OversamplingManager::Factor::x2);
            else // High
                oversamplingManager.setFactor(OversamplingManager::Factor::x4);
            osCurrentlyEngaged = false;
        }
    }
}

void EnhancedSPARK::processWithOversampling(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    // Create audio block for input
    juce::dsp::AudioBlock<float> inputBlock(buffer);

    // Upsample
    auto oversampledBlock = oversamplingManager.processUp(inputBlock);
    const int osNumSamples = (int)oversampledBlock.getNumSamples();

    // Process oversampled signal
    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = oversampledBlock.getChannelPointer(ch);

        for (int i = 0; i < osNumSamples; ++i)
        {
            float sample = data[i];

            // Apply hysteresis-based saturation
            float saturated = processHysteresis(sample, ch);

            // Apply true-peak limiting
            float ceiling = ceilingLinearSmooth.getNext();
            float limited = applyTruePeakLimiting(saturated);
            limited = juce::jlimit(-ceiling, ceiling, limited);

            data[i] = limited;
        }
    }

    // Downsample back to original rate
    oversamplingManager.processDown(inputBlock);
}

void EnhancedSPARK::processDirect(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = data[i];

            // Apply hysteresis-based saturation
            float saturated = processHysteresis(sample, ch);

            // Apply basic limiting (not true-peak without oversampling)
            float ceiling = ceilingLinearSmooth.getNext();
            float limited = juce::jlimit(-ceiling, ceiling, saturated);

            data[i] = limited;
        }
    }
}

float EnhancedSPARK::processHysteresis(float input, int channel)
{
    auto& state = hysteresisState[channel];

    // Detect input direction (ascending or descending)
    float inputDelta = input - state.lastInput;
    state.lastInput = input;

    // Langevin function (anhysteretic magnetization)
    // L(x) = coth(x) - 1/x, approximated as tanh for efficiency
    float scaledInput = input * 3.0f / Ms; // Scale to domain
    state.anhystereticMag = Ms * std::tanh(scaledInput);

    // Jiles-Atherton differential equation (simplified)
    // dM/dH = (Man - M) / (k * delta) + c * dMan/dH
    float manDiff = state.anhystereticMag - state.magnetization;
    float delta = (inputDelta >= 0.0f) ? 1.0f : -1.0f;

    // Update magnetization with hysteresis
    float dMdH = (manDiff / (k * delta + alpha * manDiff)) +
                 c * (state.anhystereticMag - state.magnetization);

    state.magnetization += dMdH * std::abs(inputDelta) * 0.5f;

    // Clamp to saturation limits
    state.magnetization = juce::jlimit(-Ms, Ms, state.magnetization);

    // Output is the magnetization (with subtle asymmetry)
    return state.magnetization * (1.0f + 0.02f * std::abs(state.magnetization));
}

float EnhancedSPARK::applyTruePeakLimiting(float sample)
{
    // Simple soft-knee limiter with musical character
    float threshold = ceilingLinear * 0.9f; // Start soft-knee at -1dB below ceiling

    if (std::abs(sample) <= threshold)
    {
        return sample; // Below threshold - no limiting
    }
    else
    {
        // Soft-knee compression/limiting
        float excess = std::abs(sample) - threshold;
        float knee = (ceilingLinear - threshold);
        float ratio = 0.25f; // 4:1 ratio in knee

        float compressed = threshold + excess * ratio;
        compressed = juce::jlimit(0.0f, ceilingLinear, compressed);

        return (sample > 0.0f) ? compressed : -compressed;
    }
}

void EnhancedSPARK::updateTruePeak(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    float maxPeak = 0.0f;

    for (int ch = 0; ch < channels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float absSample = std::abs(data[i]);
            maxPeak = std::max(maxPeak, absSample);
        }
    }

    // Update true-peak measurement
    truePeakLinear = std::max(truePeakLinear * 0.999f, maxPeak); // Slow decay
    truePeakDb = juce::Decibels::gainToDecibels(truePeakLinear, -96.0f);

    // Calculate gain reduction
    if (truePeakLinear > ceilingLinear)
    {
        gainReductionDb = juce::Decibels::gainToDecibels(ceilingLinear / truePeakLinear);
    }
    else
    {
        gainReductionDb *= 0.95f; // Decay to 0
    }
}
