/*
  ==============================================================================

  BTZButton.cpp

  ==============================================================================
*/

#include "BTZButton.h"

BTZButton::BTZButton(const juce::String& buttonText)
    : juce::TextButton(buttonText)
{
    juce::Component::setSize(BTZTheme::Layout::buttonWidth, BTZTheme::Layout::buttonHeight);
}

void BTZButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();

    // Determine button color based on state
    juce::Colour buttonColor;

    if (!isEnabled())
    {
        buttonColor = BTZTheme::Colors::buttonDisabled;
    }
    else if (getToggleState() && getClickingTogglesState())
    {
        buttonColor = toggleOnColor;
    }
    else if (shouldDrawButtonAsDown)
    {
        buttonColor = BTZTheme::Colors::buttonActive;
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        buttonColor = BTZTheme::Colors::buttonHover;
    }
    else if (getClickingTogglesState())
    {
        buttonColor = toggleOffColor;
    }
    else
    {
        buttonColor = BTZTheme::Colors::buttonNormal;
    }

    // Draw background with rounded corners
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds, BTZTheme::Layout::cornerRadius);

    // Draw border
    g.setColour(BTZTheme::darken(buttonColor, 0.2f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), BTZTheme::Layout::cornerRadius, 1.5f);

    // Draw text
    g.setFont(BTZTheme::Fonts::getBody());
    g.setColour(isEnabled() ? juce::Colours::white : BTZTheme::Colors::textDisabled);
    g.drawText(getButtonText(), bounds, juce::Justification::centred);
}

void BTZButton::setToggleColors(juce::Colour onColor, juce::Colour offColor)
{
    toggleOnColor = onColor;
    toggleOffColor = offColor;
    repaint();
}
