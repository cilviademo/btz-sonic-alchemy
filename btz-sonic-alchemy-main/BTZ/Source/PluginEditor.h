/*
  Box Tone Zone (BTZ) — PluginEditor.h
  Overhauled: moodboard-aligned color system, macro knobs,
  SHINE freq/Q controls, structured layout zones.
*/
#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// Color system — moodboard v0.2 aligned (near-black chassis, amber accent)
// Legacy palette retained as fallback; new palette is primary.
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColors {
    // ── Moodboard v0.2 primary palette ──
    const juce::Colour obsidian  { 0xFF0A0A0D };  // deepest background
    const juce::Colour panelDark { 0xFF13141A };  // panel background
    const juce::Colour charcoal  { 0xFF1A1C23 };  // raised surfaces
    const juce::Colour amber     { 0xFFE8A94A };  // primary accent
    const juce::Colour cream     { 0xFFEAE0CC };  // primary text
    const juce::Colour bone      { 0xFFD8CFB8 };  // secondary text

    // ── Legacy palette (kept for compatibility during transition) ──
    const juce::Colour canvas { 0xFFF1EFEA };
    const juce::Colour panel  { 0xFFE8E3D9 };
    const juce::Colour well   { 0xFFD4CEC2 };
    const juce::Colour text   { 0xFF1A1A18 };
    const juce::Colour text2  { 0xFF4A4640 };
    const juce::Colour text3  { 0xFF918B82 };
    const juce::Colour sage   { 0xFF7E9B8E };
    const juce::Colour oak    { 0xFFB08D57 };
    const juce::Colour red    { 0xFFC0543E };
}

class BTZLookAndFeel : public juce::LookAndFeel_V4 {
public:
    BTZLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                          float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider&) override;
};

class BTZAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit BTZAudioProcessorEditor(BTZAudioProcessor&);
    ~BTZAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob(juce::Slider& s, juce::Label& l);
    void setupSlider(juce::Slider& s);
    void paintMeter(juce::Graphics& g, juce::Rectangle<float> area, float db, float minDb = -60.0f, float maxDb = 6.0f);
    void paintGrMeter(juce::Graphics& g, juce::Rectangle<float> area, float grDb);

    BTZAudioProcessor& proc;
    BTZLookAndFeel lookAndFeel;
    int currentPage = 0;

    // ── Tab navigation ──
    juce::TextButton tabMain { "MAIN" }, tabSpark { "SPARK" }, tabAdvanced { "ADVANCED" };
    juce::ToggleButton btnBypass { "BYPASS" };

    // ── Core knobs ──
    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth, kDensity, kMotion, kEra;
    juce::Slider kDrive, kMix, kMaster;
    juce::Label lPunch{ "", "Punch" }, lWarmth{ "", "Warmth" }, lBoom{ "", "Boom" };
    juce::Label lGlue{ "", "Glue" }, lAir{ "", "Air" }, lWidth{ "", "Width" };
    juce::Label lDensity{ "", "Density" }, lMotion{ "", "Motion" }, lEra{ "", "Era" };
    juce::Label lDrive{ "", "Drive" }, lMix{ "", "Mix" }, lMaster{ "", "Master" };

    // ── SPARK / SHINE sliders ──
    juce::Slider sCeiling, sSparkMix, sShine, sShineMix, sIntensity;
    juce::Slider sShineFreq, sShineQ;

    // ── Macro knobs ──
    juce::Slider kMacro0, kMacro1, kMacro2, kMacro3;
    juce::Label lMacro0{ "", "Macro 1" }, lMacro1{ "", "Macro 2" };
    juce::Label lMacro2{ "", "Macro 3" }, lMacro3{ "", "Macro 4" };

    // ── Attachments ──
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> aPunch, aWarmth, aBoom, aGlue, aAir, aWidth;
    std::unique_ptr<SliderAttachment> aDensity, aMotion, aEra, aMix, aDrive, aMaster;
    std::unique_ptr<SliderAttachment> aCeiling, aSparkMix, aShine, aShineMix, aIntensity;
    std::unique_ptr<SliderAttachment> aShineFreq, aShineQ;
    std::unique_ptr<SliderAttachment> aMacro0, aMacro1, aMacro2, aMacro3;
    std::unique_ptr<ButtonAttachment> aBypass;

    // ── Meter display state ──
    float inPeakL = -100.0f, inPeakR = -100.0f, inRmsL = -100.0f, inRmsR = -100.0f;
    float outPeakL = -100.0f, outPeakR = -100.0f, outRmsL = -100.0f, outRmsR = -100.0f;
    float sparkGR = 0.0f, lufs = -24.0f, corr = 1.0f;
    float inClip = 0.0f, outClip = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};
