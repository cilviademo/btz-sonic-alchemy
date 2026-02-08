#pragma once
#include <JuceHeader.h>

class GranularProcessor {
public:
    GranularProcessor() {}
    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
    }
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setGrainSize(float) {}
private:
    float sampleRate = 44100.0f;
};
