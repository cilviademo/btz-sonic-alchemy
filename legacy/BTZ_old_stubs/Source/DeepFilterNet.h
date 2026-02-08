#pragma once
#include <JuceHeader.h>

class DeepFilterNet {
public:
    explicit DeepFilterNet(const char*) {}
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    float getTransientStrength() { return 0.5f; }
    float getLowEndEnergy() { return 0.5f; }
    float getLoudnessScore() { return 0.5f; }
};
