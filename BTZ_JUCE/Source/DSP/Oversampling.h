/*
  Oversampling.h
  Oversampling utility wrapper for JUCE's dsp::Oversampling
  Up to 16x oversampling for artifact-free processing
*/

#pragma once
#include <JuceHeader.h>

template<typename SampleType>
class OversamplingProcessor
{
public:
    OversamplingProcessor()
    {
        // Initialize with 8x oversampling (3 stages: 2^3 = 8)
        oversampler = std::make_unique<juce::dsp::Oversampling<SampleType>>(
            2, 3, juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR);
    }

    void setOversamplingFactor(int factor)
    {
        int stages = 0;
        switch (factor)
        {
            case 1:  stages = 0; break; // No oversampling
            case 2:  stages = 1; break; // 2^1 = 2x
            case 4:  stages = 2; break; // 2^2 = 4x
            case 8:  stages = 3; break; // 2^3 = 8x
            case 16: stages = 4; break; // 2^4 = 16x
            default: stages = 3; break; // Default 8x
        }

        if (stages != currentStages)
        {
            currentStages = stages;
            oversampler = std::make_unique<juce::dsp::Oversampling<SampleType>>(
                2, stages, juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR);

            if (isPrepared)
                oversampler->initProcessing(maxBlockSize);
        }
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        maxBlockSize = spec.maximumBlockSize;
        oversampler->initProcessing(maxBlockSize);
        isPrepared = true;
    }

    void reset()
    {
        oversampler->reset();
    }

    juce::dsp::AudioBlock<SampleType> processUp(juce::dsp::AudioBlock<SampleType>& block)
    {
        return oversampler->processSamplesUp(block);
    }

    void processDown(juce::dsp::AudioBlock<SampleType>& block)
    {
        oversampler->processSamplesDown(block);
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<SampleType>> oversampler;
    int currentStages = 3; // Default 8x
    size_t maxBlockSize = 512;
    bool isPrepared = false;
};
