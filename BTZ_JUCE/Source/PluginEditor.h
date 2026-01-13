/*
  PluginEditor.h
  BTZ - The Box Tone Zone Enhancer
  Modern UI with BTZ theme and custom components
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"
#include "GUI/MainView.h"
#include "GUI/BTZTheme.h"

class BTZAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BTZAudioProcessorEditor (BTZAudioProcessor&);
    ~BTZAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BTZAudioProcessor& audioProcessor;

    std::unique_ptr<MainView> mainView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTZAudioProcessorEditor)
};
