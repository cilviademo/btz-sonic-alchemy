/*
  ==============================================================================

  MainView.h

  Main control panel with hero controls:
  - 5 Hero Knobs: Punch, Warmth, Boom, Shine, Drive
  - Mix control
  - Input/Output gains
  - A/B/C preset ladder
  - SPARK limiter section
  - Master bypass

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "BTZTheme.h"
#include "BTZKnob.h"
#include "BTZButton.h"

class MainView : public juce::Component
{
public:
    MainView(juce::AudioProcessorValueTreeState& apvts);
    ~MainView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& audioProcessorValueTreeState;

    // Hero Controls
    std::unique_ptr<BTZKnob> punchKnob;
    std::unique_ptr<BTZKnob> warmthKnob;
    std::unique_ptr<BTZKnob> boomKnob;
    std::unique_ptr<BTZKnob> shineKnob;
    std::unique_ptr<BTZKnob> driveKnob;

    // Utility Controls
    std::unique_ptr<BTZKnob> mixKnob;
    std::unique_ptr<BTZKnob> inputGainKnob;
    std::unique_ptr<BTZKnob> outputGainKnob;

    // SPARK Section
    std::unique_ptr<BTZButton> sparkEnabledButton;
    std::unique_ptr<BTZKnob> sparkCeilingKnob;

    // Preset Ladder (A/B/C)
    std::unique_ptr<BTZButton> presetAButton;
    std::unique_ptr<BTZButton> presetBButton;
    std::unique_ptr<BTZButton> presetCButton;

    // Master Controls
    std::unique_ptr<BTZButton> bypassButton;
    std::unique_ptr<BTZButton> activeButton;

    // Parameter attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    void createControls();
    void createParameterAttachments();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainView)
};
