#pragma once
#include <JuceHeader.h>

class FuzzedDistortion {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setDrive(float) {}
};
