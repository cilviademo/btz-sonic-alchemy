/*
  ==============================================================================

  BTZKnob.h

  Modern rotary knob component with BTZ theme styling

  Features:
  - Smooth parameter control with mouse drag
  - Double-click to reset to default
  - Value label display
  - Themed colors (sage green + gold)
  - Lock-free parameter updates

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "BTZTheme.h"

class BTZKnob : public juce::Component
{
public:
    BTZKnob(const juce::String& labelText = "");
    ~BTZKnob() override = default;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    // Parameter attachment
    void setParameter(juce::RangedAudioParameter* param);
    void setValueSuffix(const juce::String& suffix) { valueSuffix = suffix; repaint(); }
    void setValueFormatter(std::function<juce::String(float)> formatter) { valueFormatter = formatter; }

    // Appearance
    void setLabel(const juce::String& text) { label = text; repaint(); }
    void setKnobSize(float diameter) { knobSize = diameter; juce::Component::setSize((int)diameter, (int)diameter + 40); }

private:
    void updateValue();
    juce::String getDisplayValue() const;

    juce::String label;
    juce::String valueSuffix;
    float knobSize = BTZTheme::Layout::knobSize;

    juce::RangedAudioParameter* parameter = nullptr;
    std::function<juce::String(float)> valueFormatter;

    float currentValue = 0.5f;
    float defaultValue = 0.5f;
    int dragStartY = 0;
    float dragStartValue = 0.0f;

    static constexpr float rotaryStartAngle = juce::MathConstants<float>::pi * 1.2f;
    static constexpr float rotaryEndAngle   = juce::MathConstants<float>::pi * 2.8f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZKnob)
};
