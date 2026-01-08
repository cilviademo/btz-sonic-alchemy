/*
  PluginEditor.h
  BTZ - The Box Tone Zone Enhancer
  Custom GUI (placeholder - will be developed)
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

class BTZAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BTZAudioProcessorEditor (BTZAudioProcessor&);
    ~BTZAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BTZAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BTZAudioProcessorEditor)
};
