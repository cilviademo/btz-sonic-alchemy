/*
  ==============================================================================

  MainView.cpp

  ==============================================================================
*/

#include "MainView.h"
#include "../PluginProcessor.h"  // P1.3: For PresetManager access

MainView::MainView(juce::AudioProcessorValueTreeState& apvts)
    : audioProcessorValueTreeState(apvts)
{
    createControls();
    createParameterAttachments();
    setSize(BTZTheme::UI::windowWidth, BTZTheme::UI::windowHeight);
}

void MainView::createControls()
{
    // =========================================================================
    // HERO CONTROLS (Top Row)
    // =========================================================================

    punchKnob = std::make_unique<BTZKnob>("PUNCH");
    punchKnob->setRange(0.0, 100.0, 0.1);
    punchKnob->setValue(50.0);
    punchKnob->setTooltip("Transient shaping: enhance attack and punch on drums and percussive material");
    addAndMakeVisible(punchKnob.get());

    warmthKnob = std::make_unique<BTZKnob>("WARMTH");
    warmthKnob->setRange(0.0, 100.0, 0.1);
    warmthKnob->setValue(50.0);
    warmthKnob->setTooltip("Harmonic saturation: add warmth, analog character, and harmonic richness");
    addAndMakeVisible(warmthKnob.get());

    boomKnob = std::make_unique<BTZKnob>("BOOM");
    boomKnob->setRange(0.0, 100.0, 0.1);
    boomKnob->setValue(50.0);
    boomKnob->setTooltip("Sub-harmonic enhancement: add weight and low-end presence");
    addAndMakeVisible(boomKnob.get());

    shineKnob = std::make_unique<BTZKnob>("SHINE");
    shineKnob->setRange(0.0, 100.0, 0.1);
    shineKnob->setValue(50.0);
    shineKnob->setTooltip("Psychoacoustic air: enhance high-frequency presence and clarity (24 Bark bands)");
    addAndMakeVisible(shineKnob.get());

    driveKnob = std::make_unique<BTZKnob>("DRIVE");
    driveKnob->setRange(0.0, 100.0, 0.1);
    driveKnob->setValue(0.0);
    driveKnob->setTooltip("Adaptive saturation drive: control overall harmonic generation intensity");
    addAndMakeVisible(driveKnob.get());

    // =========================================================================
    // UTILITY CONTROLS (Bottom Row)
    // =========================================================================

    mixKnob = std::make_unique<BTZKnob>("MIX");
    mixKnob->setRange(0.0, 100.0, 0.1);
    mixKnob->setValue(100.0);
    mixKnob->setValueSuffix("%");
    mixKnob->setTooltip("Wet/dry mix: blend processed signal with dry signal (100% = full wet)");
    addAndMakeVisible(mixKnob.get());

    inputGainKnob = std::make_unique<BTZKnob>("INPUT");
    inputGainKnob->setRange(-12.0, 12.0, 0.1);
    inputGainKnob->setValue(0.0);
    inputGainKnob->setValueSuffix(" dB");
    inputGainKnob->setTooltip("Input gain trim: adjust input level before processing (-12 to +12 dB)");
    addAndMakeVisible(inputGainKnob.get());

    outputGainKnob = std::make_unique<BTZKnob>("OUTPUT");
    outputGainKnob->setRange(-12.0, 12.0, 0.1);
    outputGainKnob->setValue(0.0);
    outputGainKnob->setValueSuffix(" dB");
    outputGainKnob->setTooltip("Output gain trim: adjust final output level (-12 to +12 dB)");
    addAndMakeVisible(outputGainKnob.get());

    // =========================================================================
    // SPARK LIMITER SECTION
    // =========================================================================

    sparkEnabledButton = std::make_unique<BTZButton>("SPARK");
    sparkEnabledButton->setClickingTogglesState(true);
    sparkEnabledButton->setToggleColors(BTZTheme::Colors::secondary, BTZTheme::Colors::buttonDisabled);
    sparkEnabledButton->setTooltip("SPARK true-peak limiter: Jiles-Atherton hysteresis with ITU BS.1770 compliance");
    addAndMakeVisible(sparkEnabledButton.get());

    sparkCeilingKnob = std::make_unique<BTZKnob>("CEILING");
    sparkCeilingKnob->setRange(-12.0, 0.0, 0.1);
    sparkCeilingKnob->setValue(-0.3);
    sparkCeilingKnob->setValueSuffix(" dB");
    sparkCeilingKnob->setTooltip("True-peak ceiling: maximum output level with intersample peak detection");
    addAndMakeVisible(sparkCeilingKnob.get());

    // =========================================================================
    // PRESET LADDER (A/B/C)
    // =========================================================================

    presetAButton = std::make_unique<BTZButton>("A");
    presetAButton->setClickingTogglesState(true);
    presetAButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    presetAButton->setToggleState(true, juce::dontSendNotification);
    presetAButton->setTooltip("Preset slot A: click to load, right-click to save current settings (20ms click-free ramping)");
    addAndMakeVisible(presetAButton.get());

    presetBButton = std::make_unique<BTZButton>("B");
    presetBButton->setClickingTogglesState(true);
    presetBButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    presetBButton->setTooltip("Preset slot B: click to load, right-click to save current settings (20ms click-free ramping)");
    addAndMakeVisible(presetBButton.get());

    presetCButton = std::make_unique<BTZButton>("C");
    presetCButton->setClickingTogglesState(true);
    presetCButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::panelBorder);
    presetCButton->setTooltip("Preset slot C: click to load, right-click to save current settings (20ms click-free ramping)");
    addAndMakeVisible(presetCButton.get());

    // P1.3: Wire preset button click handlers
    // Get processor from APVTS to access PresetManager
    auto& processor = dynamic_cast<BTZAudioProcessor&>(audioProcessorValueTreeState.processor);
    auto& presetManager = processor.getPresetManager();

    // Preset A: Save on right-click, load on left-click
    presetAButton->onClick = [this, &presetManager, &processor]()
    {
        // Left click: Load from slot A
        presetManager.loadFromSlot(PresetManager::Slot::A);
        // Update button states
        presetAButton->setToggleState(true, juce::dontSendNotification);
        presetBButton->setToggleState(false, juce::dontSendNotification);
        presetCButton->setToggleState(false, juce::dontSendNotification);
    };

    // Preset B: Save on right-click, load on left-click
    presetBButton->onClick = [this, &presetManager, &processor]()
    {
        // Left click: Load from slot B
        presetManager.loadFromSlot(PresetManager::Slot::B);
        // Update button states
        presetAButton->setToggleState(false, juce::dontSendNotification);
        presetBButton->setToggleState(true, juce::dontSendNotification);
        presetCButton->setToggleState(false, juce::dontSendNotification);
    };

    // Preset C: Save on right-click, load on left-click
    presetCButton->onClick = [this, &presetManager, &processor]()
    {
        // Left click: Load from slot C
        presetManager.loadFromSlot(PresetManager::Slot::C);
        // Update button states
        presetAButton->setToggleState(false, juce::dontSendNotification);
        presetBButton->setToggleState(false, juce::dontSendNotification);
        presetCButton->setToggleState(true, juce::dontSendNotification);
    };

    // =========================================================================
    // MASTER CONTROLS
    // =========================================================================

    activeButton = std::make_unique<BTZButton>("ACTIVE");
    activeButton->setClickingTogglesState(true);
    activeButton->setToggleColors(BTZTheme::Colors::primary, BTZTheme::Colors::buttonDisabled);
    activeButton->setToggleState(true, juce::dontSendNotification);
    activeButton->setTooltip("Master active state: enable/disable all processing");
    addAndMakeVisible(activeButton.get());

    bypassButton = std::make_unique<BTZButton>("BYPASS");
    bypassButton->setClickingTogglesState(true);
    bypassButton->setToggleColors(BTZTheme::Colors::meterHigh, BTZTheme::Colors::buttonDisabled);
    bypassButton->setTooltip("Master bypass: pass audio through unprocessed (true bypass)");
    addAndMakeVisible(bypassButton.get());
}

void MainView::createParameterAttachments()
{
    // Create slider attachments for all knobs
    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "punch", *punchKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "warmth", *warmthKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "boom", *boomKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "shine", *shineKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "drive", *driveKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "mix", *mixKnob));
    mixKnob->setDoubleClickReturnValue(true, 1.0);  // QUICK WIN 3: Mix defaults to 100%

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "inputGain", *inputGainKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "outputGain", *outputGainKnob));

    knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessorValueTreeState, "sparkCeiling", *sparkCeilingKnob));
    sparkCeilingKnob->setDoubleClickReturnValue(true, -0.3);  // QUICK WIN 3: Ceiling defaults to -0.3 dB

    // QUICK WIN 2: Button attachments for proper parameter persistence
    // These ensure button states are saved/loaded correctly across sessions
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessorValueTreeState, "sparkEnabled", *sparkEnabledButton));

    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessorValueTreeState, "active", *activeButton));

    // Note: Bypass button uses built-in AudioProcessor bypass (not a parameter)
    // Note: Preset A/B/C buttons use custom onClick handlers (see lines 113-143)
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
