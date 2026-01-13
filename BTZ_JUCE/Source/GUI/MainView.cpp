/*
  ==============================================================================

  MainView.cpp

  ==============================================================================
*/

#include "MainView.h"

MainView::MainView(juce::AudioProcessorValueTreeState& apvts)
    : audioProcessorValueTreeState(apvts)
{
    createControls();
    setSize(BTZTheme::UI::windowWidth, BTZTheme::UI::windowHeight);
}

void MainView::createControls()
{
    // =========================================================================
    // HERO CONTROLS (Top Row)
    // =========================================================================

    punchKnob = std::make_unique<BTZKnob>("PUNCH");
    punchKnob->setParameter(audioProcessorValueTreeState.getParameter("punch"));
    addAndMakeVisible(punchKnob.get());

    warmthKnob = std::make_unique<BTZKnob>("WARMTH");
    warmthKnob->setParameter(audioProcessorValueTreeState.getParameter("warmth"));
    addAndMakeVisible(warmthKnob.get());

    boomKnob = std::make_unique<BTZKnob>("BOOM");
    boomKnob->setParameter(audioProcessorValueTreeState.getParameter("boom"));
    addAndMakeVisible(boomKnob.get());

    shineKnob = std::make_unique<BTZKnob>("SHINE");
    shineKnob->setParameter(audioProcessorValueTreeState.getParameter("shine"));
    addAndMakeVisible(shineKnob.get());

    driveKnob = std::make_unique<BTZKnob>("DRIVE");
    driveKnob->setParameter(audioProcessorValueTreeState.getParameter("drive"));
    addAndMakeVisible(driveKnob.get());

    // =========================================================================
    // UTILITY CONTROLS (Bottom Row)
    // =========================================================================

    mixKnob = std::make_unique<BTZKnob>("MIX");
    mixKnob->setParameter(audioProcessorValueTreeState.getParameter("mix"));
    mixKnob->setValueSuffix("%");
    addAndMakeVisible(mixKnob.get());

    inputGainKnob = std::make_unique<BTZKnob>("INPUT");
    inputGainKnob->setParameter(audioProcessorValueTreeState.getParameter("inputGain"));
    inputGainKnob->setValueSuffix(" dB");
    addAndMakeVisible(inputGainKnob.get());

    outputGainKnob = std::make_unique<BTZKnob>("OUTPUT");
    outputGainKnob->setParameter(audioProcessorValueTreeState.getParameter("outputGain"));
    outputGainKnob->setValueSuffix(" dB");
    addAndMakeVisible(outputGainKnob.get());

    // =========================================================================
    // SPARK LIMITER SECTION
    // =========================================================================

    sparkEnabledButton = std::make_unique<BTZButton>("SPARK");
    sparkEnabledButton->setClickingTogglesState(true);
    sparkEnabledButton->setToggleColors(BTZTheme::Colors::secondary, BTZTheme::Colors::buttonDisabled);
    addAndMakeVisible(sparkEnabledButton.get());

    sparkCeilingKnob = std::make_unique<BTZKnob>("CEILING");
    sparkCeilingKnob->setParameter(audioProcessorValueTreeState.getParameter("sparkCeiling"));
    sparkCeilingKnob->setValueSuffix(" dB");
    addAndMakeVisible(sparkCeilingKnob.get());

    // =========================================================================
    // PRESET LADDER (A/B/C)
    // =========================================================================

    presetAButton = std::make_unique<BTZButton>("A");
    presetAButton->setClickingTogglesState(true);
    presetAButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    addAndMakeVisible(presetAButton.get());

    presetBButton = std::make_unique<BTZButton>("B");
    presetBButton->setClickingTogglesState(true);
    presetBButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    addAndMakeVisible(presetBButton.get());

    presetCButton = std::make_unique<BTZButton>("C");
    presetCButton->setClickingTogglesState(true);
    presetCButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    addAndMakeVisible(presetCButton.get());

    // =========================================================================
    // MASTER CONTROLS
    // =========================================================================

    activeButton = std::make_unique<BTZButton>("ACTIVE");
    activeButton->setClickingTogglesState(true);
    activeButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::buttonDisabled);
    addAndMakeVisible(activeButton.get());

    bypassButton = std::make_unique<BTZButton>("BYPASS");
    bypassButton->setClickingTogglesState(true);
    bypassButton->setToggleColors(BTZTheme::Colors::meterHigh, BTZTheme::Colors::buttonDisabled);
    addAndMakeVisible(bypassButton.get());

    // Create parameter attachments
    createParameterAttachments();
}

void MainView::createParameterAttachments()
{
    // Note: Button attachments would be created here if the parameters exist in APVTS
    // For now, buttons are interactive but not yet connected to actual parameters
}

void MainView::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(BTZTheme::Colors::background);

    // Title Bar
    auto titleArea = getLocalBounds().removeFromTop(60);
    g.setColour(BTZTheme::Colors::panelBackground);
    g.fillRect(titleArea);

    g.setFont(BTZTheme::Fonts::getTitle());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText("BTZ", titleArea.removeFromLeft(120), juce::Justification::centred);

    g.setFont(BTZTheme::Fonts::getBody());
    g.setColour(BTZTheme::Colors::textSecondary);
    g.drawText("The Box Tone Zone", titleArea.removeFromLeft(200), juce::Justification::left);

    // Hero Section Label
    auto heroLabelArea = getLocalBounds().withY(70).withHeight(30);
    g.setFont(BTZTheme::Fonts::getHeading());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText("HERO CONTROLS", heroLabelArea, juce::Justification::centred);

    // SPARK Section Background
    auto sparkArea = juce::Rectangle<int>(650, 120, 220, 200);
    g.setColour(BTZTheme::Colors::panelBackground);
    g.fillRoundedRectangle(sparkArea.toFloat(), BTZTheme::Layout::cornerRadius);
    g.setColour(BTZTheme::Colors::panelBorder);
    g.drawRoundedRectangle(sparkArea.toFloat(), BTZTheme::Layout::cornerRadius, 2.0f);

    g.setFont(BTZTheme::Fonts::getHeading());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText("SPARK LIMITER", sparkArea.removeFromTop(35), juce::Justification::centred);

    // Utility Section Label
    auto utilityLabelArea = getLocalBounds().withY(350).withHeight(30);
    g.setFont(BTZTheme::Fonts::getHeading());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText("UTILITY", utilityLabelArea, juce::Justification::centred);

    // Preset Ladder Background
    auto presetArea = juce::Rectangle<int>(650, 350, 220, 120);
    g.setColour(BTZTheme::Colors::panelBackground);
    g.fillRoundedRectangle(presetArea.toFloat(), BTZTheme::Layout::cornerRadius);
    g.setColour(BTZTheme::Colors::panelBorder);
    g.drawRoundedRectangle(presetArea.toFloat(), BTZTheme::Layout::cornerRadius, 2.0f);

    g.setFont(BTZTheme::Fonts::getHeading());
    g.setColour(BTZTheme::Colors::textPrimary);
    g.drawText("PRESETS", presetArea.removeFromTop(35), juce::Justification::centred);
}

void MainView::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(60); // Title bar

    // =========================================================================
    // HERO CONTROLS ROW (Top Center)
    // =========================================================================

    auto heroRow = bounds.removeFromTop(180).reduced(BTZTheme::Layout::marginM);
    heroRow.removeFromTop(40); // Label space

    auto knobSpacing = 20;
    auto heroKnobsWidth = (BTZTheme::Layout::knobSize * 5) + (knobSpacing * 4);
    auto heroArea = heroRow.withSizeKeepingCentre(heroKnobsWidth, BTZTheme::Layout::knobSize + 40);

    punchKnob->setBounds(heroArea.removeFromLeft(BTZTheme::Layout::knobSize));
    heroArea.removeFromLeft(knobSpacing);

    warmthKnob->setBounds(heroArea.removeFromLeft(BTZTheme::Layout::knobSize));
    heroArea.removeFromLeft(knobSpacing);

    boomKnob->setBounds(heroArea.removeFromLeft(BTZTheme::Layout::knobSize));
    heroArea.removeFromLeft(knobSpacing);

    shineKnob->setBounds(heroArea.removeFromLeft(BTZTheme::Layout::knobSize));
    heroArea.removeFromLeft(knobSpacing);

    driveKnob->setBounds(heroArea.removeFromLeft(BTZTheme::Layout::knobSize));

    // =========================================================================
    // SPARK SECTION (Right Side)
    // =========================================================================

    auto sparkArea = juce::Rectangle<int>(670, 165, 180, 140);

    auto sparkButtonArea = sparkArea.removeFromTop(40);
    sparkEnabledButton->setBounds(sparkButtonArea.withSizeKeepingCentre(120, BTZTheme::Layout::buttonHeight));

    sparkCeilingKnob->setBounds(sparkArea.withSizeKeepingCentre(BTZTheme::Layout::knobSize,
                                                                 BTZTheme::Layout::knobSize + 40));

    // =========================================================================
    // UTILITY ROW (Bottom Center)
    // =========================================================================

    bounds.removeFromTop(30); // Section spacing
    auto utilityRow = bounds.removeFromTop(180).reduced(BTZTheme::Layout::marginM);
    utilityRow.removeFromTop(40); // Label space

    auto utilityKnobsWidth = (BTZTheme::Layout::knobSize * 3) + (knobSpacing * 2);
    auto utilityArea = utilityRow.withSizeKeepingCentre(utilityKnobsWidth, BTZTheme::Layout::knobSize + 40);

    inputGainKnob->setBounds(utilityArea.removeFromLeft(BTZTheme::Layout::knobSize));
    utilityArea.removeFromLeft(knobSpacing);

    mixKnob->setBounds(utilityArea.removeFromLeft(BTZTheme::Layout::knobSize));
    utilityArea.removeFromLeft(knobSpacing);

    outputGainKnob->setBounds(utilityArea.removeFromLeft(BTZTheme::Layout::knobSize));

    // =========================================================================
    // PRESET LADDER (Right Side, Bottom)
    // =========================================================================

    auto presetArea = juce::Rectangle<int>(670, 395, 180, 60);

    auto presetButtonWidth = 50;
    auto presetButtonSpacing = 15;

    auto presetRow = presetArea.withSizeKeepingCentre(
        (presetButtonWidth * 3) + (presetButtonSpacing * 2),
        BTZTheme::Layout::buttonHeight
    );

    presetAButton->setBounds(presetRow.removeFromLeft(presetButtonWidth));
    presetRow.removeFromLeft(presetButtonSpacing);

    presetBButton->setBounds(presetRow.removeFromLeft(presetButtonWidth));
    presetRow.removeFromLeft(presetButtonSpacing);

    presetCButton->setBounds(presetRow.removeFromLeft(presetButtonWidth));

    // =========================================================================
    // MASTER CONTROLS (Bottom Right)
    // =========================================================================

    auto masterArea = juce::Rectangle<int>(670, 490, 180, 80);
    auto masterRow = masterArea.withSizeKeepingCentre(170, BTZTheme::Layout::buttonHeight);

    activeButton->setBounds(masterRow.removeFromLeft(80));
    masterRow.removeFromLeft(10);
    bypassButton->setBounds(masterRow.removeFromLeft(80));
}
