/*
  Box Tone Zone (BTZ) — PluginEditor.h  v1.0 Ivory System
  ──────────────────────────────────────────────────────────────────────────
  Three view modes: Simple, Standard, Advanced.
  All colours from btz::palette (Ivory System).
  All components from btz:: namespace.
  Layout: calm horizontal mastering-console architecture.
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
    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text,
                   int compID, const juce::String& tooltip = {});
    void setupSmallKnob(juce::Slider& s, juce::Label& l, const juce::String& text,
                        int compID, const juce::String& tooltip = {});

    // ── Layout helpers ──
    void setViewMode(ViewMode mode);
    void layoutSimple(juce::Rectangle<int> content);
    void layoutStandard(juce::Rectangle<int> content);
    void layoutAdvanced(juce::Rectangle<int> content);
    void hideAllControls();
    void populatePresetBrowser();
    void loadCombinedPreset(int combinedIndex);   // factory (first) + user presets
    int  numFactoryPresets_ = 0;
    void updateSafetyIndicators();
    void updateVisualizers();   // FFT the processor's spectrum buffer → displays

    BTZAudioProcessor& proc;
    BTZLookAndFeel lookAndFeel;
    ViewMode viewMode = ViewMode::Standard;

    // ── Spectrum analysis (UI thread): FFT of the processor's spectrum buffer ──
    juce::dsp::FFT fft { BTZDsp::kSpectrumFFTOrder };
    std::array<float, (size_t) BTZDsp::kSpectrumFFTSize * 2> fftData {};
    std::array<float, (size_t) BTZDsp::kSpectrumFFTSize>     fftWindow {};
    std::array<float, (size_t) BTZDsp::kSpectrumFFTSize / 2> specMags {};
    std::array<float, 16> harmonicMags {};
    bool fftWindowReady = false;

    // ── Resizable ──
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<juce::ResizableCornerComponent> resizer;

    // ── Async file chooser (must outlive launchAsync callback) ───────────
    std::unique_ptr<juce::FileChooser> fileChooser;

    // ── Custom components (btz:: namespace) ──
    btz::HarmonicVisualizer     harmonicViz;
    btz::GainReductionRibbon    grRibbon;
    btz::SpectrumDisplay        spectrumDisplay;
    btz::SpectrumDisplay        spectrumAdvanced;  // full-width in Advanced
    btz::ProcessingIndicator    satIndicator;
    btz::ProcessingIndicator    compIndicator;
    btz::ProcessingIndicator    limIndicator;
    btz::TabBar                 viewTabs;
    btz::PresetBrowser          presetBrowser;
    btz::SafetyIndicator        safetyTruePeak;
    btz::SafetyIndicator        safetyCorrelation;
    btz::SafetyIndicator        safetyGR;

    // ── Simple Mode knobs ──
    btz::LabeledKnob simpleKnobDrive  { "DRIVE",  btz::id::drive };
    btz::LabeledKnob simpleKnobTone   { "CHARACTER", btz::id::shine };
    btz::LabeledKnob simpleKnobOutput { "MIX / OUTPUT", btz::id::output };

    // ── Header toolbar ──
    juce::TextButton btnUndo { "UNDO" }, btnRedo { "REDO" };
    juce::TextButton btnAB { "A" }, btnCopyAB { "A>B" };
    juce::TextButton btnPresetPrev { "<" }, btnPresetNext { ">" };
    juce::TextButton btnPresetSave { "SAVE" };
    juce::TextButton btnDelta { "DELTA" };
    juce::Label lblPresetName;
    juce::ToggleButton btnBypass { "BYPASS" };
    juce::ToggleButton btnAutoGain { "AUTO" };

    // ── Standard Mode: Character knobs (left column) ──
    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth;
    juce::Label lPunch, lWarmth, lBoom, lGlue, lAir, lWidth;

    // ── Standard Mode: Center (Drive/Mix/Master) ──
    juce::Slider kDrive, kMix, kMaster;
    juce::Label lDrive, lMix, lMaster;

    // ── Standard Mode: Bottom row (Density/Motion/Era/Intensity) ──
    juce::Slider kDensity, kMotion, kEra, kIntensity;
    juce::Label lDensity, lMotion, lEra, lIntensity;

    // ── Advanced Mode: Saturation/Dynamics/Multiband ──
    juce::ComboBox cSatModel, cGlueScHpf, cMultiband, cQuality;
    juce::Label lSatModel, lGlueScHpf, lMultiband, lQuality;
    juce::ToggleButton btnMidSide { "M/S" };
    juce::Slider kResTame, kTransSens, kCeiling, kShine;
    juce::Label lResTame, lTransSens, lCeiling, lShine;

    // ── Advanced Mode: Glue compressor detail ──
    juce::Slider kGlueAttack, kGlueRelease, kGlueRatio;
    juce::Label lGlueAttack, lGlueRelease, lGlueRatio;

    // ── Attachments ──
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> aPunch, aWarmth, aBoom, aGlue, aAir, aWidth;
    std::unique_ptr<SliderAttachment> aDrive, aMix, aMaster;
    std::unique_ptr<SliderAttachment> aDensity, aMotion, aEra, aIntensity;
    std::unique_ptr<SliderAttachment> aCeiling, aShine;
    std::unique_ptr<SliderAttachment> aResTame, aTransSens;
    std::unique_ptr<SliderAttachment> aGlueAttack, aGlueRelease, aGlueRatio;
    std::unique_ptr<ButtonAttachment> aBypass, aMidSide, aAutoGain;
    std::unique_ptr<ComboAttachment>  aSatModel, aGlueScHpf, aMultiband, aQuality;

    // ── v1.0.1: Target Lock UI ──
    juce::Label lblTargetLUFS, lblTargetRMS;
    juce::Label lblTargetLow, lblTargetMid, lblTargetHigh;
    juce::TextEditor txtTargetLUFS, txtTargetRMS;
    juce::TextEditor txtTargetLow, txtTargetMid, txtTargetHigh;
    juce::ToggleButton btnLUFSLock { "LOCK" }, btnRMSLock { "LOCK" };
    juce::ToggleButton btnLowLock { "LOCK" }, btnMidLock { "LOCK" }, btnHighLock { "LOCK" };
    juce::Slider kDynThreshold;
    juce::Label lDynThreshold;
    juce::Label lblTargetSection;

    std::unique_ptr<SliderAttachment> aDynThreshold;
    std::unique_ptr<ButtonAttachment> aLUFSLock, aRMSLock;
    std::unique_ptr<ButtonAttachment> aLowLock, aMidLock, aHighLock;
    // Note: TextEditor values are pushed to APVTS manually via onReturnKey/onFocusLost

    void setupTargetLockUI();
    void syncTargetLockFromAPVTS();

    // ── Delta monitoring state ──
    bool deltaMode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};
