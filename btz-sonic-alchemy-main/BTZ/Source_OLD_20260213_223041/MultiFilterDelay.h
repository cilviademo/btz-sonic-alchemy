#pragma once
#include <JuceHeader.h>

class MultiFilterDelay {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setDelayTime(float) {}
};
