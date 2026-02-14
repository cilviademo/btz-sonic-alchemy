#pragma once
#include <JuceHeader.h>

class TapeEmulator {
public:
    TapeEmulator() {}
    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
    }
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setSaturation(float) {}
    void setWowFlutter(float amount) { wowFlutterAmount = amount; }
private:
    float wowFlutterAmount = 0.0f;
    float sampleRate = 44100.0f;
};
