/*
  ThermalKnob.h
  Custom rotary knob with thermal gradient visualization
  TODO: Implement custom look-and-feel matching React UI
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ThermalKnob : public juce::Slider
{
public:
    ThermalKnob()
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    }

    void paint(juce::Graphics& g) override
    {
        // TODO: Custom thermal gradient rendering
        juce::Slider::paint(g);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThermalKnob)
};
