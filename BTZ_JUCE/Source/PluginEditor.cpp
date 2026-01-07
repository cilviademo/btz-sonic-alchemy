/*
  PluginEditor.cpp
  BTZ - The Box Tone Zone Enhancer
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

BTZAudioProcessorEditor::BTZAudioProcessorEditor (BTZAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor()
{
}

void BTZAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background matching React UI
    g.fillAll (juce::Colour(0xFF171717)); // neutral-950

    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("BTZ - The Box Tone Zone", getLocalBounds().removeFromTop(60),
                      juce::Justification::centred, 1);

    g.setFont (14.0f);
    g.setColour (juce::Colours::lightgrey);
    g.drawFittedText ("Precision drum tone sculptor", getLocalBounds().removeFromTop(90).removeFromBottom(30),
                      juce::Justification::centred, 1);

    g.setFont (12.0f);
    g.setColour (juce::Colours::grey);
    g.drawFittedText ("Custom GUI coming soon - using generic editor below", getLocalBounds().removeFromTop(120).removeFromBottom(30),
                      juce::Justification::centred, 1);
}

void BTZAudioProcessorEditor::resized()
{
    // Reserve space for future custom components
}
