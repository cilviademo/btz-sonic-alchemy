/*
  ==============================================================================

  BTZKnob.h

  Modern rotary knob component with BTZ theme styling

  Features:
  - Based on juce::Slider for proper APVTS integration
  - Smooth parameter control with mouse drag
  - Double-click to reset to default
  - Value label display
  - Themed colors (sage green + gold)
  - Lock-free parameter updates via APVTS attachments

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "BTZTheme.h"

class BTZKnob : public juce::Slider
{
public:
    BTZKnob(const juce::String& labelText = "");
    ~BTZKnob() override = default;

    void paint(juce::Graphics& g) override;

    // Appearance customization
    void setLabel(const juce::String& text) { label = text; repaint(); }
    void setValueSuffix(const juce::String& suffix) { valueSuffix = suffix; repaint(); }
    void setKnobSize(float diameter);

private:
    juce::String label;
    juce::String valueSuffix;
    float knobSize = BTZTheme::Layout::knobSize;

    static constexpr float rotaryStartAngle = juce::MathConstants<float>::pi * 1.2f;
    static constexpr float rotaryEndAngle   = juce::MathConstants<float>::pi * 2.8f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZKnob)
};
