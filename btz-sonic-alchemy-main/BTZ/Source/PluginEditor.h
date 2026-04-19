/*
  Box Tone Zone (BTZ) — PluginEditor.h  v3
  ────────────────────────────────────────────────────────────────────────
  Ecosystem-aligned luxury UI.  1280×800 window.
  Module-specific accents: BTZ=amber, SPARK=coral, SHINE=cyan.
  Premium MacroKnob rendering (cream gradient body, halo arc, tick dots).
  Glue SC HPF toggle exposed on Advanced page.
  All styling from BTZTheme.h v3 (BTZColours + BTZTokens).
*/
#pragma once

#include "PluginProcessor.h"
#include "BTZTheme.h"
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel — Single rendering authority for all BTZ controls
// ═══════════════════════════════════════════════════════════════════════════
class BTZLookAndFeel : public juce::LookAndFeel_V4 {
public:
    BTZLookAndFeel();

    // ── Current module accent (set per page) ──
    BTZColours::Module currentModule = BTZColours::Module::BTZ;

    // ── Standard JUCE overrides ──
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

    // ── Premium MacroKnob draw (called from paint for macro knobs) ──
    static void drawMacroKnob(juce::Graphics& g, juce::Rectangle<float> area,
                              float normValue, BTZColours::Module module,
                              bool isHovered = false);

    // ── BTZ custom draw methods ──
    static void drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area);
    static void drawBTZSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                     const juce::String& text, BTZColours::Module module);
    static void drawBTZMeter(juce::Graphics& g, juce::Rectangle<float> area,
                             float valuePct, bool isGR = false);
    static void drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                           const juce::String& text, bool isActive,
                           BTZColours::Module module = BTZColours::Module::BTZ);
};

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor
// ═══════════════════════════════════════════════════════════════════════════
class BTZAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit BTZAudioProcessorEditor(BTZAudioProcessor&);
    ~BTZAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // ── Setup helpers ──
    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText);
    void setupSmallKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText);
    void setupMacroKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText);

    // ── Paint helpers ──
    void paintHeader(juce::Graphics& g, juce::Rectangle<float> area);
    void paintMeterStrip(juce::Graphics& g, juce::Rectangle<float> area);
    void paintFooter(juce::Graphics& g, juce::Rectangle<float> area);
    void paintMainPage(juce::Graphics& g, juce::Rectangle<float> content);
    void paintSparkPage(juce::Graphics& g, juce::Rectangle<float> content);
    void paintAdvancedPage(juce::Graphics& g, juce::Rectangle<float> content);

    // ── Layout helpers ──
    void layoutMainPage(juce::Rectangle<int> content);
    void layoutSparkPage(juce::Rectangle<int> content);
    void layoutAdvancedPage(juce::Rectangle<int> content);
    void hideAllControls();

    BTZAudioProcessor& proc;
    BTZLookAndFeel lookAndFeel;
    int currentPage = 0;

    // ── Tab navigation ──
    juce::TextButton tabMain { "MAIN" }, tabSpark { "SPARK" }, tabAdvanced { "ADVANCED" };
    juce::ToggleButton btnBypass { "BYPASS" };

    // ── Core knobs (Main page) ──
    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth;
    juce::Slider kDensity, kMotion, kEra;
    juce::Slider kDrive, kMix, kMaster;
    juce::Label lPunch, lWarmth, lBoom, lGlue, lAir, lWidth;
    juce::Label lDensity, lMotion, lEra, lDrive, lMix, lMaster;

    // ── Macro knobs (Main page — 110px premium rendering) ──
    juce::Slider kMacro0, kMacro1, kMacro2, kMacro3;
    juce::Label lMacro0, lMacro1, lMacro2, lMacro3;

    // ── SPARK controls ──
    juce::Slider kCeiling;
    juce::Label lCeiling;

    // ── SHINE controls ──
    juce::Slider kShine, kShineMix, kShineFreq, kShineQ;
    juce::Label lShine, lShineMix, lShineFreq, lShineQ;

    // ── Intensity (shared) ──
    juce::Slider kIntensity;
    juce::Label lIntensity;

    // ── Advanced page: Glue SC HPF toggle ──
    juce::ComboBox cGlueScHpf;
    juce::Label lGlueScHpf;

    // ── Attachments ──
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> aPunch, aWarmth, aBoom, aGlue, aAir, aWidth;
    std::unique_ptr<SliderAttachment> aDensity, aMotion, aEra, aMix, aDrive, aMaster;
    std::unique_ptr<SliderAttachment> aCeiling, aShine, aShineMix, aIntensity;
    std::unique_ptr<SliderAttachment> aShineFreq, aShineQ;
    std::unique_ptr<SliderAttachment> aMacro0, aMacro1, aMacro2, aMacro3;
    std::unique_ptr<ButtonAttachment> aBypass;
    std::unique_ptr<ComboAttachment>  aGlueScHpf;

    // ── Meter display state ──
    float inPeakL = -100.0f, inPeakR = -100.0f, inRmsL = -100.0f, inRmsR = -100.0f;
    float outPeakL = -100.0f, outPeakR = -100.0f, outRmsL = -100.0f, outRmsR = -100.0f;
    float sparkGR = 0.0f, lufs = -24.0f, corr = 1.0f;
    float inClip = 0.0f, outClip = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};
