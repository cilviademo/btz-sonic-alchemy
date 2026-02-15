#pragma once
#include <JuceHeader.h>

class LimiterNo6 {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setThreshold(float) {}
    void setCeiling(float) {}
};
