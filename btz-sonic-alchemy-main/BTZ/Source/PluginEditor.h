/*
  Box Tone Zone (BTZ) - PluginEditor.h
*/
#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

namespace BTZColors {
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

    juce::TextButton tabMain { "MAIN" }, tabSpark { "SPARK" }, tabAdvanced { "ADVANCED" };
    juce::ToggleButton btnBypass { "BYPASS" };

    juce::Slider kPunch, kWarmth, kBoom, kGlue, kAir, kWidth, kDensity, kMotion, kEra;
    juce::Slider kDrive, kMix, kMaster;
    juce::Label lPunch{ "", "Punch" }, lWarmth{ "", "Warmth" }, lBoom{ "", "Boom" };
    juce::Label lGlue{ "", "Glue" }, lAir{ "", "Air" }, lWidth{ "", "Width" };
    juce::Label lDensity{ "", "Density" }, lMotion{ "", "Motion" }, lEra{ "", "Era" };
    juce::Label lDrive{ "", "Drive" }, lMix{ "", "Mix" }, lMaster{ "", "Master" };

    juce::Slider sCeiling, sSparkMix, sShine, sShineMix, sIntensity;
    juce::Slider sQualityMode, sCharacter, sAutoGain;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> aPunch, aWarmth, aBoom, aGlue, aAir, aWidth;
    std::unique_ptr<SliderAttachment> aDensity, aMotion, aEra, aMix, aDrive, aMaster;
    std::unique_ptr<SliderAttachment> aCeiling, aSparkMix, aShine, aShineMix, aIntensity;
    std::unique_ptr<SliderAttachment> aQualityMode, aCharacter, aAutoGain;
    std::unique_ptr<ButtonAttachment> aBypass;

    float inPeakL = -100.0f, inPeakR = -100.0f, inRmsL = -100.0f, inRmsR = -100.0f;
    float outPeakL = -100.0f, outPeakR = -100.0f, outRmsL = -100.0f, outRmsR = -100.0f;
    float sparkGR = 0.0f, lufs = -24.0f, corr = 1.0f;
    float inClip = 0.0f, outClip = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BTZAudioProcessorEditor)
};

