/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v3
  ────────────────────────────────────────────────────────────────────────
  Ecosystem-aligned luxury UI.  1280×800.
  Module accents: BTZ=amber, SPARK=coral, SHINE=cyan.
  Premium MacroKnob rendering: cream gradient body, halo arc, tick dots.
  Reduced glow (~50%), stronger hierarchy, brand presence.
  Glue SC HPF combo exposed on Advanced page.
  ALL colors from BTZColours / BTZTheme::Color.
  ALL fonts from BTZTokens / BTZTheme::Font.
*/
#include "PluginEditor.h"
#include "BTZTheme.h"

using namespace BTZTheme;
using namespace BTZColours;
using namespace BTZTokens;

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel
// ═══════════════════════════════════════════════════════════════════════════

BTZLookAndFeel::BTZLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, obsidian);
    setColour(juce::Slider::rotarySliderFillColourId, amber);
    setColour(juce::Slider::rotarySliderOutlineColourId, hairline);
    setColour(juce::Slider::thumbColourId, cream);
    setColour(juce::Slider::textBoxTextColourId, bone);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF101218));
    setColour(juce::Slider::textBoxOutlineColourId, hairline);
    setColour(juce::Label::textColourId, mute);
    setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::textColourOffId, bone);
    setColour(juce::TextButton::textColourOnId, amber);
    setColour(juce::ToggleButton::textColourId, bone);
    setColour(juce::ToggleButton::tickColourId, amber);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, cream);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, stone);
    setColour(juce::ComboBox::backgroundColourId, panel);
    setColour(juce::ComboBox::textColourId, cream);
    setColour(juce::ComboBox::outlineColourId, hairline);
    setColour(juce::ComboBox::arrowColourId, mute);
}

// ── Utility knob: dark body, module-accent arc ──
void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float, juce::Slider&) {
    const auto accent = BTZColours::accentFor(currentModule);
    const auto accentDim = BTZColours::accentDimFor(currentModule);

    const float diameter = (float)juce::jmin(w, h) * 0.85f;
    const float radius = diameter * 0.5f;
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float startAngle = juce::MathConstants<float>::pi * KnobStyle::startAngle;
    const float endAngle   = juce::MathConstants<float>::pi * KnobStyle::endAngle;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float arcRadius = radius * 0.9f;
    const float arcThickness = diameter * KnobStyle::arcThicknessRatio;

    // ── Track arc ──
    juce::Path track;
    track.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(rule);
    g.strokePath(track, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // ── Fill arc (module accent) ──
    if (sliderPos > 0.001f) {
        juce::Path fill;
        fill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(accent);
        g.strokePath(fill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // ── Knob body (dark brushed-metal) ──
    const float bodyR = radius * KnobStyle::bodyRadiusRatio;
    juce::ColourGradient bodyGrad(charcoal, cx, cy - bodyR, panel, cx, cy + bodyR, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // ── Body border ──
    g.setColour(hairline);
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.5f);

    // ── Indicator line (module accent) ──
    g.setColour(accent);
    const float ix1 = cx + radius * KnobStyle::indicatorStart * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + radius * KnobStyle::indicatorStart * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + radius * KnobStyle::indicatorEnd * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + radius * KnobStyle::indicatorEnd * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.0f);

    // ── NO hover glow fill (v3: removed for restraint) ──
}

void BTZLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float,
                                      juce::Slider::SliderStyle, juce::Slider&) {
    const auto accent = BTZColours::accentFor(currentModule);
    const float trackY = (float)y + (float)h * 0.5f;
    const float trackH = (float)Geometry::sliderTrackH;

    g.setColour(juce::Colour(0xFF101218));
    g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)w, trackH, 2.0f);

    g.setColour(hairline);
    g.drawRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)w, trackH, 2.0f, 0.5f);

    const float fillW = juce::jlimit(0.0f, (float)w, sliderPos - (float)x);
    if (fillW > 1.0f) {
        g.setColour(accent);
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, fillW, trackH, 2.0f);
    }

    const float thumbR = 5.0f;
    const float thumbX = juce::jlimit((float)x + thumbR, (float)(x + w) - thumbR, sliderPos);
    g.setColour(cream);
    g.fillEllipse(thumbX - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f);
}

void BTZLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour&, bool isHighlighted, bool isDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    if (isDown)
        g.setColour(charcoal);
    else if (isHighlighted)
        g.setColour(stone);
    else
        g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(bounds, 0.0f);
}

void BTZLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) {
    g.setFont(Font::tab());
    g.setColour(button.getToggleState() ? amber : bone);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);

    // ── Active tab underline (module accent) ──
    if (button.getToggleState()) {
        auto bounds = button.getLocalBounds().toFloat();
        auto accent = amber;
        if (button.getButtonText() == "SPARK") accent = coral;
        else if (button.getButtonText() == "ADVANCED") accent = amber;
        g.setColour(accent);
        g.fillRect(bounds.removeFromBottom(2.0f));
    }
}

void BTZLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                       bool, bool) {
    auto bounds = button.getLocalBounds().toFloat();
    auto toggleArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);

    g.setColour(button.getToggleState() ? amber : stone);
    g.fillRoundedRectangle(toggleArea, Geometry::pillRadius);

    float dotR = toggleArea.getHeight() * 0.35f;
    float dotX = button.getToggleState()
        ? toggleArea.getRight() - dotR - 3.0f
        : toggleArea.getX() + dotR + 3.0f;
    float dotY = toggleArea.getCentreY();
    g.setColour(cream);
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);

    g.setFont(Font::label());
    g.setColour(button.getToggleState() ? amber : bone);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft);
}

void BTZLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.setFont(Font::label());
    g.setColour(label.findColour(juce::Label::textColourId));
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
}

void BTZLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool,
                                   int, int, int, int, juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)w, (float)h);
    g.setColour(panel);
    g.fillRoundedRectangle(bounds, 0.0f);
    g.setColour(hairline);
    g.drawRoundedRectangle(bounds, 0.0f, 0.5f);

    // ── Arrow ──
    auto arrowArea = bounds.removeFromRight(24.0f).reduced(6.0f);
    juce::Path arrow;
    arrow.addTriangle(arrowArea.getX(), arrowArea.getCentreY() - 3.0f,
                      arrowArea.getRight(), arrowArea.getCentreY() - 3.0f,
                      arrowArea.getCentreX(), arrowArea.getCentreY() + 3.0f);
    g.setColour(mute);
    g.fillPath(arrow);
}

void BTZLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted,
                                        bool isTicked, bool, const juce::String& text,
                                        const juce::String&, const juce::Drawable*, const juce::Colour*) {
    if (isSeparator) {
        g.setColour(rule);
        g.fillRect(area.reduced(8, 0).withHeight(1).withCentre(area.getCentre()));
        return;
    }

    if (isHighlighted) {
        g.setColour(stone);
        g.fillRect(area);
    }

    g.setFont(Font::label());
    g.setColour(isActive ? (isTicked ? amber : cream) : mute);
    g.drawText(text, area.reduced(12, 0), juce::Justification::centredLeft);
}

// ── Premium MacroKnob: cream gradient body, halo arc, tick dots ──
void BTZLookAndFeel::drawMacroKnob(juce::Graphics& g, juce::Rectangle<float> area,
                                    float normValue, BTZColours::Module module, bool isHovered) {
    const auto accent = BTZColours::accentFor(module);
    const float diameter = juce::jmin(area.getWidth(), area.getHeight());
    const float radius = diameter * 0.5f;
    const float cx = area.getCentreX();
    const float cy = area.getCentreY();
    const float startAngle = juce::MathConstants<float>::pi * 1.25f;
    const float endAngle   = juce::MathConstants<float>::pi * 2.75f;
    const float angle = startAngle + normValue * (endAngle - startAngle);
    const float arcRadius = radius * 0.92f;
    const float arcThickness = diameter * 0.045f;

    // ── Tick dots ──
    const int numTicks = BTZTokens::Dim::tickDotCount;
    for (int i = 0; i < numTicks; ++i) {
        float t = (float)i / (float)(numTicks - 1);
        float tickAngle = startAngle + t * (endAngle - startAngle);
        float dx = cx + (arcRadius + 8.0f) * std::cos(tickAngle - juce::MathConstants<float>::halfPi);
        float dy = cy + (arcRadius + 8.0f) * std::sin(tickAngle - juce::MathConstants<float>::halfPi);
        bool active = (tickAngle <= angle);
        g.setColour(active ? accent : deepMute);
        g.fillEllipse(dx - 2.0f, dy - 2.0f, 4.0f, 4.0f);
    }

    // ── Halo arc (background) ──
    juce::Path haloTrack;
    haloTrack.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(rule);
    g.strokePath(haloTrack, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // ── Halo arc (value fill — module accent) ──
    if (normValue > 0.001f) {
        juce::Path haloFill;
        haloFill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(accent);
        g.strokePath(haloFill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // ── Cream gradient body (premium) ──
    const float bodyR = radius * 0.72f;
    juce::ColourGradient bodyGrad(knobHighlight, cx, cy - bodyR, knobShadow, cx, cy + bodyR, false);
    bodyGrad.addColour(0.5, knobMid);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // ── Body border ──
    g.setColour(paper.withAlpha(0.3f));
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.5f);

    // ── Indicator notch (dark line on cream body) ──
    g.setColour(juce::Colour(0xFF3A3530));
    const float ix1 = cx + bodyR * 0.25f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + bodyR * 0.25f * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + bodyR * 0.75f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + bodyR * 0.75f * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.0f);

    // ── Subtle glow on hover only (reduced intensity) ──
    if (isHovered) {
        g.setColour(accent.withAlpha(BTZColours::amberSoftGlowAlpha));
        g.drawEllipse(cx - arcRadius - 4.0f, cy - arcRadius - 4.0f,
                      (arcRadius + 4.0f) * 2.0f, (arcRadius + 4.0f) * 2.0f, 3.0f);
    }
}

// ── Static draw methods ──

void BTZLookAndFeel::drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(panel);
    g.fillRoundedRectangle(area, 0.0f);
    g.setColour(rule);
    g.drawRoundedRectangle(area, 0.0f, 0.5f);
}

void BTZLookAndFeel::drawBTZSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                           const juce::String& text, BTZColours::Module module) {
    g.setFont(Font::sectionHeader());
    g.setColour(BTZColours::accentFor(module));
    g.drawText(text, area, juce::Justification::centredLeft);
    auto line = area.removeFromBottom(1.0f);
    g.setColour(rule);
    g.fillRect(line);
}

void BTZLookAndFeel::drawBTZMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                   float valuePct, bool isGR) {
    g.setColour(juce::Colour(0xFF101218));
    g.fillRoundedRectangle(area, Effects::meterCornerRadius);

    float pct = juce::jlimit(0.0f, 1.0f, valuePct);
    if (pct > 0.001f) {
        juce::Colour fillColour;
        if (isGR) {
            fillColour = amber;
        } else if (pct < 0.6f) {
            fillColour = meterSafe;
        } else if (pct < 0.8f) {
            fillColour = meterOptimal;
        } else if (pct < 0.95f) {
            fillColour = meterHot;
        } else {
            fillColour = meterClip;
        }
        auto fillArea = area;
        fillArea.setWidth(area.getWidth() * pct);
        g.setColour(fillColour);
        g.fillRoundedRectangle(fillArea, Effects::meterCornerRadius);
    }
}

void BTZLookAndFeel::drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                                 const juce::String& text, bool isActive,
                                 BTZColours::Module module) {
    g.setFont(Font::tab());
    g.setColour(isActive ? BTZColours::accentFor(module) : bone);
    g.drawText(text, area, juce::Justification::centred);
    if (isActive) {
        g.setColour(BTZColours::accentFor(module));
        g.fillRect(area.removeFromBottom(2.0f));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor — Constructor
// ═══════════════════════════════════════════════════════════════════════════

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&lookAndFeel);
    setSize(Geometry::windowWidth, Geometry::windowHeight);

    // ── Tab navigation ──
    auto setupTab = [this](juce::TextButton& btn, int page) {
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1);
        btn.onClick = [this, page, &btn] {
            currentPage = page;
            resized();
            repaint();
        };
        addAndMakeVisible(btn);
    };
    setupTab(tabMain, 0);
    setupTab(tabSpark, 1);
    setupTab(tabAdvanced, 2);
    tabMain.setToggleState(true, juce::dontSendNotification);

    // ── Bypass ──
    addAndMakeVisible(btnBypass);
    aBypass = std::make_unique<ButtonAttachment>(proc.apvts, "bypass", btnBypass);

    // ── Core knobs ──
    setupKnob(kPunch, lPunch, "PUNCH");       aPunch   = std::make_unique<SliderAttachment>(proc.apvts, "punch", kPunch);
    setupKnob(kWarmth, lWarmth, "WARMTH");     aWarmth  = std::make_unique<SliderAttachment>(proc.apvts, "warmth", kWarmth);
    setupKnob(kBoom, lBoom, "BOOM");           aBoom    = std::make_unique<SliderAttachment>(proc.apvts, "boom", kBoom);
    setupKnob(kGlue, lGlue, "GLUE");          aGlue    = std::make_unique<SliderAttachment>(proc.apvts, "glue", kGlue);
    setupKnob(kAir, lAir, "AIR");             aAir     = std::make_unique<SliderAttachment>(proc.apvts, "air", kAir);
    setupKnob(kWidth, lWidth, "WIDTH");        aWidth   = std::make_unique<SliderAttachment>(proc.apvts, "width", kWidth);
    setupKnob(kDensity, lDensity, "DENSITY");  aDensity = std::make_unique<SliderAttachment>(proc.apvts, "density", kDensity);
    setupKnob(kMotion, lMotion, "MOTION");     aMotion  = std::make_unique<SliderAttachment>(proc.apvts, "motion", kMotion);
    setupKnob(kEra, lEra, "ERA");             aEra     = std::make_unique<SliderAttachment>(proc.apvts, "era", kEra);
    setupKnob(kDrive, lDrive, "DRIVE");        aDrive   = std::make_unique<SliderAttachment>(proc.apvts, "drive", kDrive);
    setupKnob(kMix, lMix, "MIX");             aMix     = std::make_unique<SliderAttachment>(proc.apvts, "mix", kMix);
    setupKnob(kMaster, lMaster, "MASTER");     aMaster  = std::make_unique<SliderAttachment>(proc.apvts, "master", kMaster);

    // ── Macro knobs (premium 110px) ──
    setupMacroKnob(kMacro0, lMacro0, "PUNCH");   aMacro0 = std::make_unique<SliderAttachment>(proc.apvts, "macro0", kMacro0);
    setupMacroKnob(kMacro1, lMacro1, "WARMTH");   aMacro1 = std::make_unique<SliderAttachment>(proc.apvts, "macro1", kMacro1);
    setupMacroKnob(kMacro2, lMacro2, "BOOM");     aMacro2 = std::make_unique<SliderAttachment>(proc.apvts, "macro2", kMacro2);
    setupMacroKnob(kMacro3, lMacro3, "GLUE");     aMacro3 = std::make_unique<SliderAttachment>(proc.apvts, "macro3", kMacro3);

    // ── SPARK controls ──
    setupKnob(kCeiling, lCeiling, "CEILING");      aCeiling   = std::make_unique<SliderAttachment>(proc.apvts, "sparkCeiling", kCeiling);
    setupKnob(kIntensity, lIntensity, "INTENSITY"); aIntensity = std::make_unique<SliderAttachment>(proc.apvts, "intensity", kIntensity);

    // ── SHINE controls ──
    setupKnob(kShine, lShine, "SHINE");           aShine    = std::make_unique<SliderAttachment>(proc.apvts, "shine", kShine);
    setupKnob(kShineMix, lShineMix, "SHINE MIX"); aShineMix = std::make_unique<SliderAttachment>(proc.apvts, "shineMix", kShineMix);
    setupSmallKnob(kShineFreq, lShineFreq, "FREQ");  aShineFreq = std::make_unique<SliderAttachment>(proc.apvts, "shineFreq", kShineFreq);
    setupSmallKnob(kShineQ, lShineQ, "Q");            aShineQ    = std::make_unique<SliderAttachment>(proc.apvts, "shineQ", kShineQ);

    // ── Glue SC HPF combo (Advanced page) ──
    lGlueScHpf.setText("SC HPF", juce::dontSendNotification);
    lGlueScHpf.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lGlueScHpf);
    cGlueScHpf.addItem("OFF", 1);
    cGlueScHpf.addItem("60 Hz", 2);
    cGlueScHpf.addItem("90 Hz", 3);
    cGlueScHpf.addItem("150 Hz", 4);
    addAndMakeVisible(cGlueScHpf);
    aGlueScHpf = std::make_unique<ComboAttachment>(proc.apvts, "glueScHpf", cGlueScHpf);

    startTimerHz(24);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

// ── Setup helpers ──

void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setDoubleClickReturnValue(true, s.getRange().getStart());
    addAndMakeVisible(s);

    l.setText(labelText, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, mute);
    addAndMakeVisible(l);
}

void BTZAudioProcessorEditor::setupSmallKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText) {
    setupKnob(s, l, labelText);
}

void BTZAudioProcessorEditor::setupMacroKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setDoubleClickReturnValue(true, 0.0);
    s.setLookAndFeel(nullptr);  // MacroKnobs use custom paint, not LookAndFeel
    addAndMakeVisible(s);

    l.setText(labelText, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, bone);
    l.setFont(Font::macroLabel());
    addAndMakeVisible(l);
}

// ═══════════════════════════════════════════════════════════════════════════
// Timer — Meter updates
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::timerCallback() {
    auto& m = proc.meterBallistics;
    inPeakL  = m.getInPeakL();   inPeakR  = m.getInPeakR();
    inRmsL   = m.getInRmsL();    inRmsR   = m.getInRmsR();
    outPeakL = m.getOutPeakL();  outPeakR = m.getOutPeakR();
    outRmsL  = m.getOutRmsL();   outRmsR  = m.getOutRmsR();
    sparkGR  = m.getGR();        lufs     = m.getLufs();
    corr     = m.getCorrelation();
    inClip   = m.getInClip();    outClip  = m.getOutClip();
    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
// Paint
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    // ── Full background ──
    g.fillAll(obsidian);

    auto bounds = getLocalBounds().toFloat();

    // ── Header ──
    auto headerArea = bounds.removeFromTop((float)Geometry::headerHeight);
    paintHeader(g, headerArea);

    // ── Hairline under header ──
    g.setColour(rule);
    g.fillRect(bounds.removeFromTop(1.0f));

    // ── Meter strip ──
    auto meterArea = bounds.removeFromTop((float)Geometry::meterStripHeight);
    paintMeterStrip(g, meterArea);

    // ── Hairline under meters ──
    g.setColour(rule);
    g.fillRect(bounds.removeFromTop(1.0f));

    // ── Content area ──
    auto content = bounds.reduced((float)Geometry::padContent, (float)Space::lg);

    // ── Page-specific paint (section headers, panel backgrounds) ──
    if (currentPage == 0)
        paintMainPage(g, content);
    else if (currentPage == 1)
        paintSparkPage(g, content);
    else
        paintAdvancedPage(g, content);

    // ── Footer ──
    auto footerArea = getLocalBounds().toFloat().removeFromBottom(24.0f);
    paintFooter(g, footerArea);

    // ── Paint macro knobs manually (they bypass LookAndFeel) ──
    if (currentPage == 0) {
        auto drawMacro = [&](juce::Slider& s) {
            if (s.isVisible()) {
                auto b = s.getBounds().toFloat();
                float norm = (float)((s.getValue() - s.getMinimum()) / (s.getMaximum() - s.getMinimum()));
                BTZLookAndFeel::drawMacroKnob(g, b, norm, BTZColours::Module::BTZ, s.isMouseOver());
            }
        };
        drawMacro(kMacro0);
        drawMacro(kMacro1);
        drawMacro(kMacro2);
        drawMacro(kMacro3);
    }
}

void BTZAudioProcessorEditor::paintHeader(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(panel);
    g.fillRect(area);

    // ── Brand: "BTZ" left-aligned ──
    auto brandArea = area.reduced(20.0f, 0.0f);
    g.setFont(Font::title());
    g.setColour(amber);
    g.drawText("BTZ", brandArea.removeFromLeft(60.0f), juce::Justification::centredLeft);

    // ── "SONIC ALCHEMY" subtitle ──
    g.setFont(Font::subtitle());
    g.setColour(mute);
    g.drawText("SONIC ALCHEMY", brandArea.removeFromLeft(140.0f), juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintMeterStrip(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(panel);
    g.fillRect(area);

    auto body = area.reduced(20.0f, 6.0f);

    auto dbToPct = [](float db, float minDb = -60.0f, float maxDb = 6.0f) -> float {
        return juce::jlimit(0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
    };

    auto drawMeterRow = [&](const juce::String& label, float valL, float valR, bool gr = false) {
        auto row = body.removeFromTop(12.0f);
        g.setFont(Font::meter());
        g.setColour(deepMute);
        g.drawText(label, row.removeFromLeft(80.0f), juce::Justification::centredLeft);

        auto m1 = row.removeFromLeft(170.0f).reduced(2.0f, 1.0f);
        auto m2 = row.removeFromLeft(170.0f).reduced(2.0f, 1.0f);

        float pctL = gr ? juce::jlimit(0.0f, 1.0f, valL / 18.0f) : dbToPct(valL);
        float pctR = gr ? juce::jlimit(0.0f, 1.0f, valR / 18.0f) : dbToPct(valR);

        BTZLookAndFeel::drawBTZMeter(g, m1, pctL, gr);
        BTZLookAndFeel::drawBTZMeter(g, m2, pctR, gr);

        g.setFont(Font::value());
        g.setColour(bone);
        g.drawText(juce::String(valL, 1), row.removeFromLeft(44.0f), juce::Justification::centredRight);
        g.drawText(juce::String(valR, 1), row.removeFromLeft(44.0f), juce::Justification::centredRight);
    };

    drawMeterRow("IN  PEAK", inPeakL, inPeakR);
    drawMeterRow("IN  RMS",  inRmsL, inRmsR);
    drawMeterRow("OUT PEAK", outPeakL, outPeakR);
    drawMeterRow("OUT RMS",  outRmsL, outRmsR);
    drawMeterRow("SPARK GR", sparkGR, sparkGR, true);

    // ── Status row ──
    auto statusRow = body.removeFromTop(14.0f);
    g.setFont(Font::meter());
    g.setColour(deepMute);
    g.drawText("LUFS: " + juce::String(lufs, 1), statusRow.removeFromLeft(100.0f), juce::Justification::centredLeft);
    g.drawText("CORR: " + juce::String(corr, 2), statusRow.removeFromLeft(90.0f), juce::Justification::centredLeft);

    g.setColour(inClip > 0.2f ? stateClip : deepMute);
    g.drawText("IN CLIP", statusRow.removeFromLeft(60.0f), juce::Justification::centredLeft);
    g.setColour(outClip > 0.2f ? stateClip : deepMute);
    g.drawText("OUT CLIP", statusRow.removeFromLeft(70.0f), juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintFooter(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setFont(Font::micro());
    g.setColour(deepMute);
    g.drawText("v1.1  |  " + juce::String(proc.getSampleRate(), 0) + " Hz  |  BTZ Sonic Alchemy  |  BTZ Audio",
               area, juce::Justification::centred);
}

void BTZAudioProcessorEditor::paintMainPage(juce::Graphics& g, juce::Rectangle<float> content) {
    // ── Section headers ──
    auto topHeader = content.removeFromTop(20.0f);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader.removeFromLeft(content.getWidth() * 0.5f),
                                          "TONE SHAPING", BTZColours::Module::BTZ);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader,
                                          "CHARACTER", BTZColours::Module::BTZ);
}

void BTZAudioProcessorEditor::paintSparkPage(juce::Graphics& g, juce::Rectangle<float> content) {
    auto leftPanel = content.removeFromLeft(content.getWidth() * 0.5f);
    auto rightPanel = content;

    // ── SPARK panel background ──
    BTZLookAndFeel::drawBTZPanelBackground(g, leftPanel.reduced(4.0f));
    BTZLookAndFeel::drawBTZSectionHeader(g, leftPanel.reduced(16.0f, 12.0f).removeFromTop(22.0f),
                                          "SPARK  —  LIMITER", BTZColours::Module::SPARK);

    // ── SHINE panel background ──
    BTZLookAndFeel::drawBTZPanelBackground(g, rightPanel.reduced(4.0f));
    BTZLookAndFeel::drawBTZSectionHeader(g, rightPanel.reduced(16.0f, 12.0f).removeFromTop(22.0f),
                                          "SHINE  —  EXCITER", BTZColours::Module::SHINE);
}

void BTZAudioProcessorEditor::paintAdvancedPage(juce::Graphics& g, juce::Rectangle<float> content) {
    BTZLookAndFeel::drawBTZSectionHeader(g, content.removeFromTop(22.0f),
                                          "CHARACTER  —  ADVANCED", BTZColours::Module::BTZ);
}

// ═══════════════════════════════════════════════════════════════════════════
// Resized — Layout
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    // ── Header: tabs + bypass ──
    auto header = bounds.removeFromTop(Geometry::headerHeight);
    auto tabArea = header.reduced(280, 10);
    const int tabW = Geometry::tabWidth;
    const int gap = Geometry::tabGap;
    const int startX = tabArea.getCentreX() - (tabW * 3 + gap * 2) / 2;
    tabMain.setBounds(startX, tabArea.getY(), tabW, Geometry::tabHeight);
    tabSpark.setBounds(startX + tabW + gap, tabArea.getY(), tabW, Geometry::tabHeight);
    tabAdvanced.setBounds(startX + (tabW + gap) * 2, tabArea.getY(), tabW, Geometry::tabHeight);
    btnBypass.setBounds(header.getRight() - 140, header.getY() + 10, 120, header.getHeight() - 20);

    tabMain.setToggleState(currentPage == 0, juce::dontSendNotification);
    tabSpark.setToggleState(currentPage == 1, juce::dontSendNotification);
    tabAdvanced.setToggleState(currentPage == 2, juce::dontSendNotification);

    // ── Set module accent for LookAndFeel ──
    if (currentPage == 1)
        lookAndFeel.currentModule = BTZColours::Module::SPARK;
    else if (currentPage == 2)
        lookAndFeel.currentModule = BTZColours::Module::BTZ;
    else
        lookAndFeel.currentModule = BTZColours::Module::BTZ;

    // ── Skip meter strip + hairlines ──
    bounds.removeFromTop(Geometry::meterStripHeight + 2);

    // ── Content area ──
    auto content = bounds.reduced(Geometry::padContent, Space::lg + 10);

    hideAllControls();

    if (currentPage == 0)
        layoutMainPage(content);
    else if (currentPage == 1)
        layoutSparkPage(content);
    else
        layoutAdvancedPage(content);
}

void BTZAudioProcessorEditor::hideAllControls() {
    auto hide = [](juce::Slider& s, juce::Label& l) { s.setVisible(false); l.setVisible(false); };
    hide(kPunch, lPunch); hide(kWarmth, lWarmth); hide(kBoom, lBoom);
    hide(kGlue, lGlue); hide(kAir, lAir); hide(kWidth, lWidth);
    hide(kDensity, lDensity); hide(kMotion, lMotion); hide(kEra, lEra);
    hide(kDrive, lDrive); hide(kMix, lMix); hide(kMaster, lMaster);
    hide(kMacro0, lMacro0); hide(kMacro1, lMacro1);
    hide(kMacro2, lMacro2); hide(kMacro3, lMacro3);
    hide(kCeiling, lCeiling);
    hide(kShine, lShine); hide(kShineMix, lShineMix);
    hide(kShineFreq, lShineFreq); hide(kShineQ, lShineQ);
    hide(kIntensity, lIntensity);
    cGlueScHpf.setVisible(false);
    lGlueScHpf.setVisible(false);
}

void BTZAudioProcessorEditor::layoutMainPage(juce::Rectangle<int> content) {
    // Skip section header area
    content.removeFromTop(24);

    const int knob = Geometry::knobLarge;
    const int label = Geometry::knobLabelHeight;
    const int gapX = (content.getWidth() - knob * 6) / 5;
    const int y1 = content.getY() + Space::lg;
    const int y2 = y1 + knob + label + Space::xl;

    auto place = [&](juce::Slider& s, juce::Label& l, int col, int row) {
        int x = content.getX() + col * (knob + gapX);
        int y = (row == 0) ? y1 : y2;
        s.setBounds(x, y, knob, knob);
        l.setBounds(x, y + knob, knob, label);
        s.setVisible(true); l.setVisible(true);
    };

    // Row 1: Punch, Warmth, Boom, Glue, Air, Width
    place(kPunch, lPunch, 0, 0); place(kWarmth, lWarmth, 1, 0); place(kBoom, lBoom, 2, 0);
    place(kGlue, lGlue, 3, 0); place(kAir, lAir, 4, 0); place(kWidth, lWidth, 5, 0);

    // Row 2: Density, Motion, Era, Drive, Mix, Master
    place(kDensity, lDensity, 0, 1); place(kMotion, lMotion, 1, 1); place(kEra, lEra, 2, 1);
    place(kDrive, lDrive, 3, 1); place(kMix, lMix, 4, 1); place(kMaster, lMaster, 5, 1);

    // ── Macro row (premium 110px knobs) ──
    const int macroY = y2 + knob + label + Space::secLg;
    const int macroKnob = Geometry::knobMacro;
    const int macroGap = (content.getWidth() - macroKnob * 4) / 3;
    auto placeMacro = [&](juce::Slider& s, juce::Label& l, int i) {
        int x = content.getX() + i * (macroKnob + macroGap);
        s.setBounds(x, macroY, macroKnob, macroKnob);
        l.setBounds(x, macroY + macroKnob + 4, macroKnob, label + 4);
        s.setVisible(true); l.setVisible(true);
    };
    placeMacro(kMacro0, lMacro0, 0);
    placeMacro(kMacro1, lMacro1, 1);
    placeMacro(kMacro2, lMacro2, 2);
    placeMacro(kMacro3, lMacro3, 3);
}

void BTZAudioProcessorEditor::layoutSparkPage(juce::Rectangle<int> content) {
    auto leftPanel = content.removeFromLeft(content.getWidth() / 2).reduced(Geometry::padPanel, Space::secLg);
    auto rightPanel = content.reduced(Geometry::padPanel, Space::secLg);

    const int knob = Geometry::knobLarge;
    const int smallKnob = Geometry::knobSmall;
    const int label = Geometry::knobLabelHeight;

    // ── SPARK: Ceiling + Intensity ──
    int sparkCenterX = leftPanel.getCentreX() - knob / 2;
    int sparkY = leftPanel.getY() + Space::secMd;
    kCeiling.setBounds(sparkCenterX, sparkY, knob, knob);
    lCeiling.setBounds(sparkCenterX, sparkY + knob, knob, label);
    kCeiling.setVisible(true); lCeiling.setVisible(true);

    int intensityY = sparkY + knob + label + Space::xl;
    kIntensity.setBounds(sparkCenterX, intensityY, knob, knob);
    lIntensity.setBounds(sparkCenterX, intensityY + knob, knob, label);
    kIntensity.setVisible(true); lIntensity.setVisible(true);

    // ── SHINE: Amount + Mix (top), Freq + Q (bottom) ──
    int shineY = rightPanel.getY() + Space::secMd;
    int shineGap = Space::xl;
    int shineTotalW = knob * 2 + shineGap;
    int shineStartX = rightPanel.getCentreX() - shineTotalW / 2;

    kShine.setBounds(shineStartX, shineY, knob, knob);
    lShine.setBounds(shineStartX, shineY + knob, knob, label);
    kShine.setVisible(true); lShine.setVisible(true);

    kShineMix.setBounds(shineStartX + knob + shineGap, shineY, knob, knob);
    lShineMix.setBounds(shineStartX + knob + shineGap, shineY + knob, knob, label);
    kShineMix.setVisible(true); lShineMix.setVisible(true);

    int freqY = shineY + knob + label + Space::xl;
    int freqTotalW = smallKnob * 2 + shineGap;
    int freqStartX = rightPanel.getCentreX() - freqTotalW / 2;

    kShineFreq.setBounds(freqStartX, freqY, smallKnob, smallKnob);
    lShineFreq.setBounds(freqStartX, freqY + smallKnob, smallKnob, label);
    kShineFreq.setVisible(true); lShineFreq.setVisible(true);

    kShineQ.setBounds(freqStartX + smallKnob + shineGap, freqY, smallKnob, smallKnob);
    lShineQ.setBounds(freqStartX + smallKnob + shineGap, freqY + smallKnob, smallKnob, label);
    kShineQ.setVisible(true); lShineQ.setVisible(true);
}

void BTZAudioProcessorEditor::layoutAdvancedPage(juce::Rectangle<int> content) {
    content.removeFromTop(26);

    const int knob = Geometry::knobLarge;
    const int medKnob = Geometry::knobMedium;
    const int label = Geometry::knobLabelHeight;
    const int y1 = content.getY() + Space::xl;

    // Top row: 4 character knobs
    const int topGap = (content.getWidth() - knob * 4) / 3;
    auto placeTop = [&](juce::Slider& s, juce::Label& l, int col) {
        int x = content.getX() + col * (knob + topGap);
        s.setBounds(x, y1, knob, knob);
        l.setBounds(x, y1 + knob, knob, label);
        s.setVisible(true); l.setVisible(true);
    };
    placeTop(kDrive, lDrive, 0);
    placeTop(kEra, lEra, 1);
    placeTop(kDensity, lDensity, 2);
    placeTop(kMotion, lMotion, 3);

    // Bottom row: Mix + Master + SC HPF
    const int y2 = y1 + knob + label + Space::secLg;
    const int botGap = Space::xl;
    const int botTotalW = medKnob * 2 + botGap + Space::xl + 120;
    const int botStartX = content.getCentreX() - botTotalW / 2;

    kMix.setBounds(botStartX, y2, medKnob, medKnob);
    lMix.setBounds(botStartX, y2 + medKnob, medKnob, label);
    kMix.setVisible(true); lMix.setVisible(true);

    kMaster.setBounds(botStartX + medKnob + botGap, y2, medKnob, medKnob);
    lMaster.setBounds(botStartX + medKnob + botGap, y2 + medKnob, medKnob, label);
    kMaster.setVisible(true); lMaster.setVisible(true);

    // ── Glue SC HPF combo ──
    int comboX = botStartX + medKnob * 2 + botGap + Space::xl;
    int comboY = y2 + (medKnob - 28) / 2;
    lGlueScHpf.setBounds(comboX, comboY - 18, 120, 16);
    lGlueScHpf.setVisible(true);
    cGlueScHpf.setBounds(comboX, comboY, 120, 28);
    cGlueScHpf.setVisible(true);
}
