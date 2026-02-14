#include "PluginEditor.h"

BTZLookAndFeel::BTZLookAndFeel() {
    setColour(juce::Slider::rotarySliderFillColourId, BTZColors::sage);
    setColour(juce::Slider::rotarySliderOutlineColourId, BTZColors::well);
    setColour(juce::Slider::thumbColourId, BTZColors::text);
}

void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPosProportional, float, float, juce::Slider&) {
    const float diameter = (float) juce::jmin(w, h) * 0.85f;
    const float radius = diameter * 0.5f;
    const float cx = (float) x + (float) w * 0.5f;
    const float cy = (float) y + (float) h * 0.5f;
    const float startAngle = juce::MathConstants<float>::pi * 1.25f;
    const float endAngle = juce::MathConstants<float>::pi * 2.75f;
    const float angle = startAngle + sliderPosProportional * (endAngle - startAngle);

    juce::Path track;
    track.addCentredArc(cx, cy, radius * 0.9f, radius * 0.9f, 0.0f, startAngle, endAngle, true);
    g.setColour(BTZColors::well);
    g.strokePath(track, juce::PathStrokeType(diameter * 0.065f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (sliderPosProportional > 0.001f) {
        juce::Path fill;
        fill.addCentredArc(cx, cy, radius * 0.9f, radius * 0.9f, 0.0f, startAngle, angle, true);
        juce::ColourGradient grad(BTZColors::oak, cx - radius, cy, BTZColors::sage, cx + radius, cy, false);
        g.setGradientFill(grad);
        g.strokePath(fill, juce::PathStrokeType(diameter * 0.065f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour(juce::Colour(0xFFF0ECE4));
    g.fillEllipse(cx - radius * 0.74f, cy - radius * 0.74f, radius * 1.48f, radius * 1.48f);
    g.setColour(BTZColors::text);
    const float ix1 = cx + radius * 0.22f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + radius * 0.22f * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + radius * 0.62f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + radius * 0.62f * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.0f);
}

void BTZLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                      float, float, juce::Slider::SliderStyle, juce::Slider&) {
    const float trackY = (float) y + (float) h * 0.5f;
    g.setColour(BTZColors::well);
    g.fillRoundedRectangle((float) x, trackY - 2.0f, (float) w, 4.0f, 2.0f);

    const float fillW = juce::jlimit(0.0f, (float) w, sliderPos - (float) x);
    juce::ColourGradient grad(BTZColors::oak, (float) x, trackY, BTZColors::sage, (float) x + (float) w, trackY, false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle((float) x, trackY - 2.0f, fillW, 4.0f, 2.0f);
}

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p) : AudioProcessorEditor(p), proc(p) {
    setLookAndFeel(&lookAndFeel);
    setSize(980, 610);

    auto styleTab = [&](juce::TextButton& b, int pageIdx) {
        addAndMakeVisible(b);
        b.onClick = [this, pageIdx] { currentPage = pageIdx; resized(); repaint(); };
    };
    styleTab(tabMain, 0);
    styleTab(tabSpark, 1);
    styleTab(tabAdvanced, 2);

    addAndMakeVisible(btnBypass);

    auto initKnob = [&](juce::Slider& s, juce::Label& l) { setupKnob(s, l); };
    initKnob(kPunch, lPunch); initKnob(kWarmth, lWarmth); initKnob(kBoom, lBoom);
    initKnob(kGlue, lGlue); initKnob(kAir, lAir); initKnob(kWidth, lWidth);
    initKnob(kDensity, lDensity); initKnob(kMotion, lMotion); initKnob(kEra, lEra);
    initKnob(kDrive, lDrive); initKnob(kMix, lMix); initKnob(kMaster, lMaster);

    setupSlider(sCeiling); setupSlider(sSparkMix); setupSlider(sShine);
    setupSlider(sShineMix); setupSlider(sIntensity);

    auto& apvts = proc.getAPVTS();
    aPunch    = std::make_unique<SliderAttachment>(apvts, "punch", kPunch);
    aWarmth   = std::make_unique<SliderAttachment>(apvts, "warmth", kWarmth);
    aBoom     = std::make_unique<SliderAttachment>(apvts, "boom", kBoom);
    aGlue     = std::make_unique<SliderAttachment>(apvts, "glue", kGlue);
    aAir      = std::make_unique<SliderAttachment>(apvts, "air", kAir);
    aWidth    = std::make_unique<SliderAttachment>(apvts, "width", kWidth);
    aDensity  = std::make_unique<SliderAttachment>(apvts, "density", kDensity);
    aMotion   = std::make_unique<SliderAttachment>(apvts, "motion", kMotion);
    aEra      = std::make_unique<SliderAttachment>(apvts, "vintageModern", kEra);
    aMix      = std::make_unique<SliderAttachment>(apvts, "mix", kMix);
    aDrive    = std::make_unique<SliderAttachment>(apvts, "drive", kDrive);
    aMaster   = std::make_unique<SliderAttachment>(apvts, "masterIntensity", kMaster);
    aCeiling  = std::make_unique<SliderAttachment>(apvts, "sparkCeiling", sCeiling);
    aSparkMix = std::make_unique<SliderAttachment>(apvts, "sparkMix", sSparkMix);
    aShine    = std::make_unique<SliderAttachment>(apvts, "shineAmount", sShine);
    aShineMix = std::make_unique<SliderAttachment>(apvts, "shineMix", sShineMix);
    aIntensity = std::make_unique<SliderAttachment>(apvts, "masterIntensity", sIntensity);
    aBypass = std::make_unique<ButtonAttachment>(apvts, "bypass", btnBypass);

    startTimerHz(45);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setPopupDisplayEnabled(true, true, this);

    addAndMakeVisible(l);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(9.0f));
    l.setColour(juce::Label::textColourId, BTZColors::text2);
}

void BTZAudioProcessorEditor::setupSlider(juce::Slider& s) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 18);
    s.setColour(juce::Slider::textBoxTextColourId, BTZColors::text3);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void BTZAudioProcessorEditor::timerCallback() {
    auto& m = proc.getMeters();
    auto lerp = [](float& d, float t, float c) { d += c * (t - d); };
    lerp(inPeakL, m.inputPeakL.load(std::memory_order_relaxed), 0.3f);
    lerp(inPeakR, m.inputPeakR.load(std::memory_order_relaxed), 0.3f);
    lerp(inRmsL,  m.inputRmsL.load(std::memory_order_relaxed), 0.2f);
    lerp(inRmsR,  m.inputRmsR.load(std::memory_order_relaxed), 0.2f);
    lerp(outPeakL, m.outputPeakL.load(std::memory_order_relaxed), 0.3f);
    lerp(outPeakR, m.outputPeakR.load(std::memory_order_relaxed), 0.3f);
    lerp(outRmsL,  m.outputRmsL.load(std::memory_order_relaxed), 0.2f);
    lerp(outRmsR,  m.outputRmsR.load(std::memory_order_relaxed), 0.2f);
    lerp(sparkGR, m.sparkGainReductionDb.load(std::memory_order_relaxed), 0.25f);
    lerp(lufs, m.lufs.load(std::memory_order_relaxed), 0.15f);
    lerp(corr, m.correlation.load(std::memory_order_relaxed), 0.2f);
    lerp(inClip, m.inputClip.load(std::memory_order_relaxed), 0.3f);
    lerp(outClip, m.outputClip.load(std::memory_order_relaxed), 0.3f);
    repaint();
}

void BTZAudioProcessorEditor::paintMeter(juce::Graphics& g, juce::Rectangle<float> area, float db, float minDb, float maxDb) {
    const float pct = juce::jlimit(0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
    g.setColour(BTZColors::well);
    g.fillRoundedRectangle(area, 2.0f);
    g.setColour(BTZColors::sage);
    g.fillRoundedRectangle(area.withWidth(area.getWidth() * pct), 2.0f);
}

void BTZAudioProcessorEditor::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> area, float grDb) {
    const float pct = juce::jlimit(0.0f, 1.0f, grDb / 18.0f);
    g.setColour(BTZColors::well);
    g.fillRoundedRectangle(area, 2.0f);
    g.setColour(BTZColors::oak);
    g.fillRoundedRectangle(area.withWidth(area.getWidth() * pct), 2.0f);
}

void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(BTZColors::canvas);
    g.fillRoundedRectangle(bounds, 10.0f);

    auto header = bounds.removeFromTop(54.0f);
    g.setColour(BTZColors::text);
    g.setFont(juce::Font(14.0f).boldened());
    g.drawText("BOX TONE ZONE (BTZ)", header.removeFromLeft(250.0f), juce::Justification::centredLeft);
    g.setFont(juce::Font(8.5f));
    g.setColour(BTZColors::text3);
    g.drawText("BTZ Audio", header.removeFromLeft(120.0f), juce::Justification::centredLeft);

    auto meterStrip = bounds.removeFromTop(78.0f).reduced(14.0f, 4.0f);
    g.setColour(BTZColors::panel);
    g.fillRoundedRectangle(meterStrip, 8.0f);
    auto meterBody = meterStrip.reduced(10.0f, 8.0f);

    auto drawMeterRow = [&](juce::String label, float a, float b, bool gr = false) {
        auto row = meterBody.removeFromTop(12.0f);
        g.setColour(BTZColors::text3);
        g.setFont(juce::Font(8.0f));
        g.drawText(label, row.removeFromLeft(80.0f), juce::Justification::centredLeft);
        auto m1 = row.removeFromLeft(180.0f).reduced(3.0f, 2.0f);
        auto m2 = row.removeFromLeft(180.0f).reduced(3.0f, 2.0f);
        if (! gr) { paintMeter(g, m1, a); paintMeter(g, m2, b); }
        else { paintGrMeter(g, m1, a); paintGrMeter(g, m2, b); }
        g.setColour(BTZColors::text2);
        g.drawText(juce::String(a, 1), row.removeFromLeft(48.0f), juce::Justification::centredLeft);
        g.drawText(juce::String(b, 1), row.removeFromLeft(48.0f), juce::Justification::centredLeft);
    };

    drawMeterRow("IN  PEAK L/R", inPeakL, inPeakR);
    drawMeterRow("IN  RMS  L/R", inRmsL, inRmsR);
    drawMeterRow("OUT PEAK L/R", outPeakL, outPeakR);
    drawMeterRow("OUT RMS  L/R", outRmsL, outRmsR);
    drawMeterRow("SPARK GR dB", sparkGR, sparkGR, true);

    auto statusRow = meterBody.removeFromTop(14.0f);
    g.setColour(BTZColors::text3);
    g.drawText("LUFS: " + juce::String(lufs, 1), statusRow.removeFromLeft(140.0f), juce::Justification::centredLeft);
    g.drawText("CORR: " + juce::String(corr, 2), statusRow.removeFromLeft(120.0f), juce::Justification::centredLeft);
    g.setColour(inClip > 0.2f ? BTZColors::red : BTZColors::text3);
    g.drawText("IN CLIP", statusRow.removeFromLeft(80.0f), juce::Justification::centredLeft);
    g.setColour(outClip > 0.2f ? BTZColors::red : BTZColors::text3);
    g.drawText("OUT CLIP", statusRow.removeFromLeft(90.0f), juce::Justification::centredLeft);

    auto content = bounds.reduced(16.0f, 4.0f);
    g.setColour(BTZColors::panel);
    g.fillRoundedRectangle(content, 10.0f);
}

void BTZAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(54);

    auto tabArea = header.reduced(300, 10);
    constexpr int tabW = 90, gap = 12;
    const int startX = tabArea.getCentreX() - (tabW * 3 + gap * 2) / 2;
    tabMain.setBounds(startX, tabArea.getY(), tabW, tabArea.getHeight());
    tabSpark.setBounds(startX + tabW + gap, tabArea.getY(), tabW, tabArea.getHeight());
    tabAdvanced.setBounds(startX + (tabW + gap) * 2, tabArea.getY(), tabW, tabArea.getHeight());
    btnBypass.setBounds(header.getRight() - 120, header.getY() + 14, 100, header.getHeight() - 24);

    auto styleTab = [&](juce::TextButton& b, int idx) {
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::textColourOffId, currentPage == idx ? BTZColors::text : BTZColors::text3);
        b.setColour(juce::TextButton::textColourOnId, BTZColors::text);
    };
    styleTab(tabMain, 0); styleTab(tabSpark, 1); styleTab(tabAdvanced, 2);

    bounds.removeFromTop(78);
    auto content = bounds.reduced(20, 16);

    auto hideKnob = [](juce::Slider& s, juce::Label& l) { s.setVisible(false); l.setVisible(false); };
    hideKnob(kPunch, lPunch); hideKnob(kWarmth, lWarmth); hideKnob(kBoom, lBoom);
    hideKnob(kGlue, lGlue); hideKnob(kAir, lAir); hideKnob(kWidth, lWidth);
    hideKnob(kDensity, lDensity); hideKnob(kMotion, lMotion); hideKnob(kEra, lEra);
    hideKnob(kDrive, lDrive); hideKnob(kMix, lMix); hideKnob(kMaster, lMaster);
    sCeiling.setVisible(false); sSparkMix.setVisible(false); sShine.setVisible(false); sShineMix.setVisible(false); sIntensity.setVisible(false);

    if (currentPage == 0) {
        const int knob = 74, label = 16;
        const int gapX = (content.getWidth() - knob * 6) / 5;
        const int y1 = content.getY();
        const int y2 = y1 + knob + label + 12;
        auto place = [&](juce::Slider& s, juce::Label& l, int i, int y) {
            const int x = content.getX() + i * (knob + gapX);
            s.setBounds(x, y, knob, knob);
            l.setBounds(x, y + knob, knob, label);
            s.setVisible(true); l.setVisible(true);
        };
        place(kPunch, lPunch, 0, y1); place(kWarmth, lWarmth, 1, y1); place(kBoom, lBoom, 2, y1);
        place(kGlue, lGlue, 3, y1); place(kAir, lAir, 4, y1); place(kWidth, lWidth, 5, y1);
        place(kDensity, lDensity, 0, y2); place(kMotion, lMotion, 1, y2); place(kEra, lEra, 2, y2);
        place(kDrive, lDrive, 3, y2); place(kMix, lMix, 4, y2); place(kMaster, lMaster, 5, y2);
    } else if (currentPage == 1) {
        auto left = content.removeFromLeft(content.getWidth() / 2).reduced(20, 24);
        auto right = content.reduced(20, 24);
        sCeiling.setBounds(left.removeFromTop(30)); left.removeFromTop(8);
        sSparkMix.setBounds(left.removeFromTop(30));
        sShine.setBounds(right.removeFromTop(30)); right.removeFromTop(8);
        sShineMix.setBounds(right.removeFromTop(30)); right.removeFromTop(24);
        sIntensity.setBounds(right.removeFromTop(30));
        sCeiling.setVisible(true); sSparkMix.setVisible(true); sShine.setVisible(true); sShineMix.setVisible(true); sIntensity.setVisible(true);
    }
}

