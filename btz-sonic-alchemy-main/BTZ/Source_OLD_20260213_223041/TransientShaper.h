#pragma once
#include <JuceHeader.h>

namespace chowdsp {
class TransientShaper {
public:
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setAttackGain(float) {}
    void setSustainGain(float) {}
};
}
