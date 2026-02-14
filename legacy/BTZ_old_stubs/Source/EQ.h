#pragma once
#include <JuceHeader.h>

namespace chowdsp {
class EQ {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setLowShelfGain(float) {}
    void setHighShelfGain(float) {}
};
}
