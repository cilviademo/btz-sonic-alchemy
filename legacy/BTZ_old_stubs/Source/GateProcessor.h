#pragma once
#include <JuceHeader.h>

class GateProcessor {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setThreshold(float) {}
    void setRatio(float) {}
};
