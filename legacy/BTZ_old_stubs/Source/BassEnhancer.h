#pragma once
#include <JuceHeader.h>

class BassEnhancer {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setIntensity(float) {}
    void setFrequency(float) {}
};
