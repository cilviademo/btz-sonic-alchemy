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

    auto center = knobBounds.getCentre();
    auto radius = knobSize / 2.0f - 4.0f;

    // === PHASE 4: Output Thermal/Portal Inspired 3D Knob ===

    // 1. Bottom-right shadow (gentle depth)
    auto shadowBounds = knobBounds.translated(2.0f, 2.0f);
    g.setColour(BTZTheme::Colors::knobShadow.withAlpha(0.3f));
    g.fillEllipse(shadowBounds);

    // 2. Main knob background with subtle gradient (warm beige)
    auto mainKnobBounds = knobBounds.reduced(1.0f);
    juce::ColourGradient backgroundGradient(
        BTZTheme::Colors::knobHighlight.withAlpha(0.8f), center.x - radius * 0.4f, center.y - radius * 0.4f,
        BTZTheme::Colors::knobBackground, center.x + radius * 0.5f, center.y + radius * 0.5f,
        true);
    g.setGradientFill(backgroundGradient);
    g.fillEllipse(mainKnobBounds);

    // 3. Inner bevel shadow (recessed look)
    g.setColour(BTZTheme::Colors::knobShadow.withAlpha(0.2f));
    g.drawEllipse(mainKnobBounds.reduced(3.0f), 1.5f);

    // 4. Top-left highlight (warm lighting from upper-left)
    g.setColour(BTZTheme::Colors::knobHighlight.withAlpha(0.6f));
    juce::Path highlightPath;
    highlightPath.addCentredArc(center.x, center.y, radius - 5.0f, radius - 5.0f,
                                0.0f,
                                juce::MathConstants<float>::pi * 1.0f,   // Top-left start
                                juce::MathConstants<float>::pi * 1.5f,   // Top-left end
                                true);
    g.strokePath(highlightPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

    // Calculate current angle based on slider value
    auto value = getValue();
    auto normalizedValue = (value - getMinimum()) / (getMaximum() - getMinimum());
    auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * normalizedValue;

    // 5. Value arc (sage green, slightly thicker for prominence)
    juce::Path arcPath;
    arcPath.addCentredArc(center.x, center.y, radius - 2.0f, radius - 2.0f,
                         0.0f, rotaryStartAngle, angle, true);

    g.setColour(BTZTheme::Colors::knobFill);
    g.strokePath(arcPath, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // 6. Outer ring (dark sage outline for definition)
    g.setColour(BTZTheme::Colors::knobOutline);
    g.drawEllipse(knobBounds.reduced(1.0f), 2.0f);

    // 7. Pointer (natural oak, beveled for depth)
    auto pointerLength = radius * 0.65f;
    auto pointerThickness = 3.5f;

    juce::Point<float> pointerStart(center.x, center.y);
    juce::Point<float> pointerEnd(center.x + std::cos(angle - juce::MathConstants<float>::halfPi) * pointerLength,
                                 center.y + std::sin(angle - juce::MathConstants<float>::halfPi) * pointerLength);

    // Pointer shadow (subtle)
    g.setColour(BTZTheme::Colors::knobShadow.withAlpha(0.4f));
    g.drawLine(pointerStart.x + 1.0f, pointerStart.y + 1.0f,
               pointerEnd.x + 1.0f, pointerEnd.y + 1.0f, pointerThickness);

    // Pointer main (natural oak)
    g.setColour(BTZTheme::Colors::knobPointer);
    g.drawLine(pointerStart.x, pointerStart.y, pointerEnd.x, pointerEnd.y, pointerThickness);

    // Pointer highlight (warm lighting)
    g.setColour(BTZTheme::Colors::knobHighlight.withAlpha(0.5f));
    g.drawLine(pointerStart.x - 0.5f, pointerStart.y - 0.5f,
               pointerEnd.x - 0.5f, pointerEnd.y - 0.5f, 1.5f);

    // 8. Center cap (small circle for visual anchor)
    auto capRadius = 6.0f;
    juce::Rectangle<float> capBounds(center.x - capRadius, center.y - capRadius,
                                     capRadius * 2.0f, capRadius * 2.0f);

    juce::ColourGradient capGradient(
        BTZTheme::Colors::knobHighlight, center.x - capRadius * 0.3f, center.y - capRadius * 0.3f,
        BTZTheme::Colors::knobPointer, center.x + capRadius * 0.5f, center.y + capRadius * 0.5f,
        true);
    g.setGradientFill(capGradient);
    g.fillEllipse(capBounds);

    g.setColour(BTZTheme::Colors::knobOutline);
    g.drawEllipse(capBounds, 1.0f);

    // === Labels and Value Display ===

    // Draw label (charcoal black for contrast)
    auto labelBounds = bounds.removeFromTop(20);
    g.setFont(BTZTheme::Fonts::getLabel());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText(label, labelBounds, juce::Justification::centred);

    // Draw value (medium brown for hierarchy)
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
