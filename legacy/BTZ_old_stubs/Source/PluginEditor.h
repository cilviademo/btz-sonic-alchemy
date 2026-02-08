#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class BTZAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit BTZAudioProcessorEditor(BTZAudioProcessor& p) : juce::AudioProcessorEditor(&p), processor(p) {
        setSize(660, 360);
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::white);
        g.setFont(18.0f);
        g.drawFittedText("BTZ — The Box Tone Zone Enhancer", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);
    }
    void resized() override {}
private:
    BTZAudioProcessor& processor;
};
