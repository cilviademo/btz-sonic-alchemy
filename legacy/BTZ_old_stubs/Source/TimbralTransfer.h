#pragma once
#include <JuceHeader.h>

class TimbralTransfer {
public:
    explicit TimbralTransfer(const char*) {}
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    float getTimbralRichness() { return 0.5f; }
};
