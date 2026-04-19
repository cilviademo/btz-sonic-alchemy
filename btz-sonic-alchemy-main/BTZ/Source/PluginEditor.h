/*
  Box Tone Zone (BTZ) — PluginEditor.h  v9
  ────────────────────────────────────────────────────────────────────────
  v9 (industry-gap closure):
    • Resizable UI (75%–200% scaling, aspect ratio locked)
    • Preset browser (load/save/navigate)
    • A/B comparison toggle
    • Undo/Redo buttons
    • Saturation model selector
    • Mid/Side toggle
    • Multiband count selector
    • Spectrum analyzer display area
    • Gain reduction history display area
    • MIDI learn right-click context menu
    • EBU R128 LUFS readout (momentary/short-term/integrated)
  ────────────────────────────────────────────────────────────────────────
  v3: 3-page luxury UI, module accents, premium macro knobs
*/
#pragma once

#include "PluginProcessor.h"
#include "BTZTheme.h"
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════
// BTZLookAndFeel — rendering authority for all BTZ controls
// ═══════════════════════════════════════════════════════════════════════
class BTZLookAndFeel : public juce::LookAndFeel_V4 {
public:
    BTZLookAndFeel();

    BTZColours::Module currentModule = BTZColours::Module::BTZ;

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

    static void drawMacroKnob(juce::Graphics& g, juce::Rectangle<float> area,
                              float normValue, BTZColours::Module module,
                              bool isHovered = false);

    static void drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area);
    static void drawBTZSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                     const juce::String& text, BTZColours::Module module);
    static void drawBTZMeter(juce::Graphics& g, juce::Rectangle<float> area,
                             float valuePct, bool isGR = false);
    static void drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                           const juce::String& text, bool isActive,
                           BTZColours::Module module = BTZColours::Module::BTZ);
};

// ═══════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor
// ═══════════════════════════════════════════════════════════════════════
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
    void paintSpectrumArea(juce::Graphics& g, juce::Rectangle<float> area);
    void paintGRHistory(juce::Graphics& g, juce::Rectangle<float> area);

    // ── Layout helpers ──
    void layoutMainPage(juce::Rectangle<int> content);
    void layoutSparkPage(juce::Rectangle<int> content);
    void layoutAdvancedPage(juce::Rectangle<int> content);
    void hideAllControls();

    // ── v9: MIDI learn context menu ──
    void showMIDILearnMenu(juce::Slider& slider, const juce::String& paramID);

    BTZAudioProcessor& proc;
    BTZLookAndFeel lookAndFeel;
    int currentPage = 0;

    // v9: Resizable UI
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<juce::ResizableCornerComponent> resizer;

    // ── Tab navigation ──
    juce::TextButton tabMain { "MAIN" }, tabSpark { "SPARK" }, tabAdvanced { "ADVANCED" };
    juce::ToggleButton btnBypass { "BYPASS" };

    // ── v9: Toolbar buttons ──
    juce::TextButton btnUndo { "UNDO" }, btnRedo { "REDO" };
    juce::TextButton btnAB { "A" };
    juce::TextButton btnCopyAB { "A>B" };
    juce::TextButton btnPresetPrev { "<" }, btnPresetNext { ">" };
    juce::TextButton btnPresetSave { "SAVE" };
    juce::Label lblPresetName;

    // ── Core knobs (Main page) ──
    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth;
    juce::Slider kDensity, kMotion, kEra;
    juce::Slider kDrive, kMix, kMaster;
    juce::Label lPunch, lWarmth, lBoom, lGlue, lAir, lWidth;
    juce::Label lDensity, lMotion, lEra, lDrive, lMix, lMaster;

    // ── Macro knobs (Main page) ──
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

    // ── Advanced page controls ──
    juce::ComboBox cGlueScHpf;
    juce::Label lGlueScHpf;

    // ── v9: New controls ──
    juce::ComboBox cSatModel;
    juce::Label lSatModel;
    juce::ToggleButton btnMidSide { "M/S" };
    juce::ComboBox cMultiband;
    juce::Label lMultiband;

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
    std::unique_ptr<ComboAttachment>  aSatModel;
    std::unique_ptr<ButtonAttachment> aMidSide;
    std::unique_ptr<ComboAttachment>  aMultiband;

    // ── Meter display state ──
    float inPeakL = -100.0f, inPeakR = -100.0f, inRmsL = -100.0f, inRmsR = -100.0f;
    float outPeakL = -100.0f, outPeakR = -100.0f, outRmsL = -100.0f, outRmsR = -100.0f;
    float sparkGR = 0.0f, lufs = -24.0f, lufsShort = -24.0f, lufsInt = -24.0f;
    float corr = 1.0f;
    float inClip = 0.0f, outClip = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};
