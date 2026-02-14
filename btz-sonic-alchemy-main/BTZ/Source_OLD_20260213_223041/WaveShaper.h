#pragma once
#include <JuceHeader.h>

namespace chowdsp {
class WaveShaper {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setGain(float) {}
};
}
