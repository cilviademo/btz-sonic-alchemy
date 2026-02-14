#pragma once
#include <JuceHeader.h>

class ConsoleEmulator {
public:
    ConsoleEmulator() {}
    void prepare(const juce::dsp::ProcessSpec&) {}
    void process(juce::dsp::ProcessContextReplacing<float>&) {}
    void setDrive(float) {}
    void setCrosstalk(float) {}
};
