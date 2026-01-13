/*
  ==============================================================================

  BTZKnob.cpp

  ==============================================================================
*/

#include "BTZKnob.h"

BTZKnob::BTZKnob(const juce::String& labelText)
    : label(labelText)
{
    juce::Component::setSize(BTZTheme::Layout::knobSize, BTZTheme::Layout::knobSize + 40);
}

void BTZKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto knobBounds = bounds.removeFromTop((int)knobSize).toFloat();

    // Draw knob background
    g.setColour(BTZTheme::Colors::knobBackground);
    g.fillEllipse(knobBounds);

    // Draw filled arc representing value
    auto center = knobBounds.getCentre();
    auto radius = knobSize / 2.0f - 4.0f;
    auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * currentValue;

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
    g.drawText(getDisplayValue(), valueBounds, juce::Justification::centred);
}

void BTZKnob::resized()
{
    // Layout handled in paint
}

void BTZKnob::mouseDown(const juce::MouseEvent& event)
{
    dragStartY = event.y;
    dragStartValue = currentValue;
}

void BTZKnob::mouseDrag(const juce::MouseEvent& event)
{
    auto dragDistance = dragStartY - event.y;
    auto sensitivity = 0.005f;

    // Shift for fine control
    if (event.mods.isShiftDown())
        sensitivity *= 0.1f;

    auto newValue = juce::jlimit(0.0f, 1.0f, dragStartValue + dragDistance * sensitivity);

    if (parameter != nullptr)
    {
        parameter->setValueNotifyingHost(newValue);
        currentValue = parameter->getValue();
    }
    else
    {
        currentValue = newValue;
    }

    repaint();
}

void BTZKnob::mouseUp(const juce::MouseEvent&)
{
    // Drag complete
}

void BTZKnob::mouseDoubleClick(const juce::MouseEvent&)
{
    // Reset to default
    if (parameter != nullptr)
    {
        parameter->setValueNotifyingHost(parameter->getDefaultValue());
        currentValue = parameter->getValue();
    }
    else
    {
        currentValue = defaultValue;
    }

    repaint();
}

void BTZKnob::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    auto delta = wheel.deltaY * 0.05f;

    // Shift for fine control
    if (event.mods.isShiftDown())
        delta *= 0.1f;

    auto newValue = juce::jlimit(0.0f, 1.0f, currentValue + delta);

    if (parameter != nullptr)
    {
        parameter->setValueNotifyingHost(newValue);
        currentValue = parameter->getValue();
    }
    else
    {
        currentValue = newValue;
    }

    repaint();
}

void BTZKnob::setParameter(juce::RangedAudioParameter* param)
{
    parameter = param;
    if (parameter != nullptr)
    {
        currentValue = parameter->getValue();
        defaultValue = parameter->getDefaultValue();
        repaint();
    }
}

juce::String BTZKnob::getDisplayValue() const
{
    if (valueFormatter)
    {
        return valueFormatter(currentValue);
    }

    if (parameter != nullptr)
    {
        return parameter->getText(currentValue, 50) + valueSuffix;
    }

    // Default: show percentage
    return juce::String((int)(currentValue * 100)) + "%";
}
