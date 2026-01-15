/*
  ==============================================================================

  BTZKnob.cpp

  ==============================================================================
*/

#include "BTZKnob.h"

BTZKnob::BTZKnob(const juce::String& labelText)
    : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox),
      label(labelText)
{
    setRotaryParameters(rotaryStartAngle, rotaryEndAngle, true);
    setVelocityBasedMode(true);
    setVelocityModeParameters(1.0, 1, 0.0, false);

    // QUICK WIN 3: Double-click reset enabled
    // Note: Default value will be set from APVTS parameter after attachment
    // For now, use 0.0 as the most common default (punch, warmth, boom, drive, gains)
    setDoubleClickReturnValue(true, 0.0);

    juce::Component::setSize(BTZTheme::Layout::knobSize, BTZTheme::Layout::knobSize + 40);
}

void BTZKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto knobBounds = bounds.removeFromTop((int)knobSize).toFloat();

    // Draw knob background
    g.setColour(BTZTheme::Colors::knobBackground);
    g.fillEllipse(knobBounds);

    // Calculate current angle based on slider value
    auto value = getValue();
    auto normalizedValue = (value - getMinimum()) / (getMaximum() - getMinimum());
    auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * normalizedValue;

    // Draw filled arc representing value
    auto center = knobBounds.getCentre();
    auto radius = knobSize / 2.0f - 4.0f;

    juce::Path arcPath;
    arcPath.addCentredArc(center.x, center.y, radius, radius,
                         0.0f, rotaryStartAngle, angle, true);

    g.setColour(BTZTheme::Colors::knobFill);
    g.strokePath(arcPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Draw outline circle
    g.setColour(BTZTheme::Colors::knobOutline);
    g.drawEllipse(knobBounds.reduced(2.0f), 2.0f);

    // Draw pointer line
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 3.0f;

    juce::Point<float> pointerStart(center.x + std::cos(angle - juce::MathConstants<float>::halfPi) * pointerLength * 0.3f,
                                   center.y + std::sin(angle - juce::MathConstants<float>::halfPi) * pointerLength * 0.3f);
    juce::Point<float> pointerEnd(center.x + std::cos(angle - juce::MathConstants<float>::halfPi) * pointerLength,
                                 center.y + std::sin(angle - juce::MathConstants<float>::halfPi) * pointerLength);

    g.setColour(BTZTheme::Colors::knobPointer);
    g.drawLine(pointerStart.x, pointerStart.y, pointerEnd.x, pointerEnd.y, pointerThickness);

    // Draw label
    auto labelBounds = bounds.removeFromTop(20);
    g.setFont(BTZTheme::Fonts::getLabel());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText(label, labelBounds, juce::Justification::centred);

    // Draw value
    auto valueBounds = bounds;
    g.setFont(BTZTheme::Fonts::getValue());
    g.setColour(BTZTheme::Colors::textSecondary);

    // Format value display
    juce::String valueText;
    if (valueSuffix.isNotEmpty())
    {
        valueText = juce::String(value, 1) + valueSuffix;
    }
    else
    {
        valueText = juce::String((int)(normalizedValue * 100)) + "%";
    }

    g.drawText(valueText, valueBounds, juce::Justification::centred);
}

void BTZKnob::setKnobSize(float diameter)
{
    knobSize = diameter;
    juce::Component::setSize((int)diameter, (int)diameter + 40);
}
