/*
  ==============================================================================

  BTZButton.h

  Styled button component matching BTZ theme

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BTZTheme.h"

class BTZButton : public juce::TextButton
{
public:
    BTZButton(const juce::String& buttonText = "");
    ~BTZButton() override = default;

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void setToggleColors(juce::Colour onColor, juce::Colour offColor);

private:
    juce::Colour toggleOnColor = BTZTheme::Colors::primary;
    juce::Colour toggleOffColor = BTZTheme::Colors::buttonDisabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZButton)
};
