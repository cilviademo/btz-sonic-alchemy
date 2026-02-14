/*
  PluginEditor.cpp
  BTZ - The Box Tone Zone Enhancer
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

BTZAudioProcessorEditor::BTZAudioProcessorEditor (BTZAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Create main view with modern UI
    mainView = std::make_unique<MainView>(audioProcessor.getAPVTS());
    addAndMakeVisible(mainView.get());

    setSize (BTZTheme::UI::windowWidth, BTZTheme::UI::windowHeight);
    setResizable(false, false);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor()
{
}

void BTZAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (BTZTheme::Colors::background);
}

void BTZAudioProcessorEditor::resized()
{
    if (mainView != nullptr)
    {
        mainView->setBounds(getLocalBounds());
    }
}
