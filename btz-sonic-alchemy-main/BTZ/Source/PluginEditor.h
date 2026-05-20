/*
  Box Tone Zone (BTZ) — PluginEditor.h  v11
  ──────────────────────────────────────────────────────────────────────────
  Three view modes: Simple, Standard, Advanced.
  All colours from btz::palette. All components from btz:: namespace.
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include "PluginProcessor.h"
#include "BTZTheme.h"
#include "BTZComponents.h"
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel — rendering authority for all JUCE controls
// ═══════════════════════════════════════════════════════════════════════════
class BTZLookAndFeel : public juce::LookAndFeel_V4 {
public:
    BTZLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float minPos, float maxPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool isHighlighted, bool isDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool isHighlighted, bool isDown) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool isHighlighted, bool isDown) override;

    void drawLabel(juce::Graphics&, juce::Label&) override;

    void drawComboBox(juce::Graphics&, int w, int h, bool isDown,
                      int bx, int by, int bw, int bh, juce::ComboBox&) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcut,
                           const juce::Drawable*, const juce::Colour*) override;
};

// ═══════════════════════════════════════════════════════════════════════════
// View Modes
// ═══════════════════════════════════════════════════════════════════════════
enum class ViewMode { Simple, Standard, Advanced };

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor
// ═══════════════════════════════════════════════════════════════════════════
class BTZAudioProcessorEditor : public juce::AudioProcessorEditor,
                                 private juce::Timer {
public:
    explicit BTZAudioProcessorEditor(BTZAudioProcessor&);
    ~BTZAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // ── Setup helpers ──
    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, int compID);
    void setupSmallKnob(juce::Slider& s, juce::Label& l, const juce::String& text, int compID);

    // ── Layout helpers ──
    void setViewMode(ViewMode mode);
    void layoutSimple(juce::Rectangle<int> content);
    void layoutStandard(juce::Rectangle<int> content);
    void layoutAdvanced(juce::Rectangle<int> content);
    void hideAllControls();

    BTZAudioProcessor& proc;
    BTZLookAndFeel lookAndFeel;
    ViewMode viewMode = ViewMode::Standard;
    int currentPage = 0;  // Standard sub-pages: 0=Main, 1=Spark, 2=Detail

    // ── Resizable ──
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<juce::ResizableCornerComponent> resizer;

    // ── Custom components (btz:: namespace) ──
    btz::HarmonicVisualizer     harmonicViz;
    btz::GainReductionRibbon    grRibbon;
    btz::SpectrumDisplay        spectrumDisplay;
    btz::DirectManipSpectrum    spectrumAdvanced;
    btz::ProcessingIndicator    satIndicator;
    btz::ProcessingIndicator    compIndicator;
    btz::ProcessingIndicator    limIndicator;
    btz::TabBar                 viewTabs;
    btz::TabBar                 pageTabs;
    btz::PresetBrowser          presetBrowser;

    // ── Simple Mode knobs ──
    btz::LabeledKnob simpleKnobDrive  { "DRIVE",  btz::id::drive };
    btz::LabeledKnob simpleKnobTone   { "TONE",   btz::id::tone };
    btz::LabeledKnob simpleKnobOutput { "OUTPUT", btz::id::output };

    // ── Toolbar buttons ──
    juce::TextButton btnUndo { "UNDO" }, btnRedo { "REDO" };
    juce::TextButton btnAB { "A" }, btnCopyAB { "A>B" };
    juce::TextButton btnPresetPrev { "<" }, btnPresetNext { ">" };
    juce::TextButton btnPresetSave { "SAVE" };
    juce::Label lblPresetName;
    juce::ToggleButton btnBypass { "BYPASS" };

    // ── Core knobs (Standard — Main page) ──
    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth;
    juce::Slider kDrive, kMix, kMaster;
    juce::Slider kDensity, kMotion, kEra;
    juce::Label lPunch, lWarmth, lBoom, lGlue, lAir, lWidth;
    juce::Label lDrive, lMix, lMaster;
    juce::Label lDensity, lMotion, lEra;

    // ── Macro knobs ──
    juce::Slider kMacro0, kMacro1, kMacro2, kMacro3;
    juce::Label lMacro0, lMacro1, lMacro2, lMacro3;

    // ── Spark page ──
    juce::Slider kCeiling, kIntensity;
    juce::Label lCeiling, lIntensity;

    // ── Shine controls ──
    juce::Slider kShine, kShineMix, kShineFreq, kShineQ;
    juce::Label lShine, lShineMix, lShineFreq, lShineQ;

    // ── Advanced controls ──
    juce::ComboBox cSatModel, cGlueScHpf, cMultiband, cQuality;
    juce::Label lSatModel, lGlueScHpf, lMultiband, lQuality;
    juce::ToggleButton btnMidSide { "M/S" };
    juce::Slider kResTame, kTransSens;
    juce::Label lResTame, lTransSens;

    // ── Attachments ──
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> aPunch, aWarmth, aBoom, aGlue, aAir, aWidth;
    std::unique_ptr<SliderAttachment> aDrive, aMix, aMaster;
    std::unique_ptr<SliderAttachment> aDensity, aMotion, aEra;
    std::unique_ptr<SliderAttachment> aCeiling, aIntensity;
    std::unique_ptr<SliderAttachment> aShine, aShineMix, aShineFreq, aShineQ;
    std::unique_ptr<SliderAttachment> aMacro0, aMacro1, aMacro2, aMacro3;
    std::unique_ptr<SliderAttachment> aResTame, aTransSens;
    std::unique_ptr<ButtonAttachment> aBypass, aMidSide;
    std::unique_ptr<ComboAttachment>  aSatModel, aGlueScHpf, aMultiband, aQuality;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};
