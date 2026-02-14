/*
  ==============================================================================

  OversamplingManager.cpp

  ==============================================================================
*/

#include "OversamplingManager.h"

OversamplingManager::OversamplingManager()
{
    // Oversamplers will be created in prepare()
}

void OversamplingManager::prepare(double sampleRate, int samplesPerBlock, int channels)
{
    baseSampleRate = sampleRate;
    maxSamplesPerBlock = samplesPerBlock;
    numChannels = channels;

    updateOversampler();
}

void OversamplingManager::setQuality(Quality quality)
{
    if (currentQuality != quality)
    {
        currentQuality = quality;
        updateOversampler();
    }
}

void OversamplingManager::setFactor(Factor factor)
{
    if (currentFactor != factor)
    {
        currentFactor = factor;
        updateOversampler();
    }
}

juce::dsp::AudioBlock<float> OversamplingManager::processUp(juce::dsp::AudioBlock<float>& input)
{
    if (currentFactor == Factor::x1 || activeOversampler == nullptr)
    {
        // No oversampling - return input block unchanged
        return input;
    }

    // Upsample
    return activeOversampler->processSamplesUp(input);
}

void OversamplingManager::processDown(juce::dsp::AudioBlock<float>& output)
{
    if (currentFactor == Factor::x1 || activeOversampler == nullptr)
    {
        // No oversampling - nothing to downsample
        return;
    }

    // Downsample
    activeOversampler->processSamplesDown(output);
}

int OversamplingManager::getOversamplingFactor() const
{
    return getFactorValue(currentFactor);
}

int OversamplingManager::getLatencySamples() const
{
    if (activeOversampler == nullptr)
        return 0;

    return (int)activeOversampler->getLatencyInSamples();
}

void OversamplingManager::reset()
{
    if (activeOversampler != nullptr)
        activeOversampler->reset();
}

void OversamplingManager::updateOversampler()
{
    int factorIndex = static_cast<int>(currentFactor);

    // Determine filter type based on quality
    juce::dsp::Oversampling<float>::FilterType filterType;
    switch (currentQuality)
    {
        case Quality::Draft:
            filterType = juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR;
            break;
        case Quality::Good:
            filterType = juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple;
            break;
        case Quality::Best:
            filterType = juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple;
            break;
    }

    // Create or update oversampler for current factor
    if (currentFactor != Factor::x1)
    {
        int stages = factorIndex; // x2=1 stage, x4=2 stages, x8=3 stages, x16=4 stages

        oversamplers[factorIndex] = std::make_unique<juce::dsp::Oversampling<float>>(
            numChannels,
            stages,
            filterType,
            currentQuality == Quality::Best, // Use steep transition band for Best quality
            currentQuality != Quality::Draft // Normalize for Good and Best
        );

        oversamplers[factorIndex]->initProcessing((size_t)maxSamplesPerBlock);
        activeOversampler = oversamplers[factorIndex].get();
        oversampledSampleRate = baseSampleRate * getFactorValue(currentFactor);
    }
    else
    {
        activeOversampler = nullptr;
        oversampledSampleRate = baseSampleRate;
    }
}

int OversamplingManager::getFactorValue(Factor f) const
{
    switch (f)
    {
        case Factor::x1:  return 1;
        case Factor::x2:  return 2;
        case Factor::x4:  return 4;
        case Factor::x8:  return 8;
        case Factor::x16: return 16;
        default:          return 1;
    }
}
