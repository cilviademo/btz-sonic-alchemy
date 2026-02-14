#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class BTZAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit BTZAudioProcessorEditor(BTZAudioProcessor&);

    void paint(juce::Graphics&) override;

    void resized() override;

private:
    juce::String getFrontendIndexFilePath() const;

    BTZAudioProcessor& processor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    juce::String statusMessage;
};
