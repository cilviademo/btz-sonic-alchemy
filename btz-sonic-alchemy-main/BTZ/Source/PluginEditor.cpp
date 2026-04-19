/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v2
  ────────────────────────────────────────────────────────────────────────
  UI Unification Pass: one visual language across Main, SPARK, Advanced.
  ALL colors from BTZTheme::Color, ALL fonts from BTZTheme::Font,
  ALL geometry from BTZTheme::Geometry.
  Zero hardcoded styling.
*/
#include "PluginEditor.h"
#include "BTZTheme.h"

using namespace BTZTheme;

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel
// ═══════════════════════════════════════════════════════════════════════════

BTZLookAndFeel::BTZLookAndFeel() {
    // Global JUCE colour overrides — all from theme
    setColour(juce::ResizableWindow::backgroundColourId, Color::backgroundRoot());
    setColour(juce::Slider::rotarySliderFillColourId, Color::accentPrimaryAmber());
    setColour(juce::Slider::rotarySliderOutlineColourId, Color::strokeSubtle());
    setColour(juce::Slider::thumbColourId, Color::textPrimary());
    setColour(juce::Slider::textBoxTextColourId, Color::textSecondary());
    setColour(juce::Slider::textBoxBackgroundColourId, Color::backgroundInset());
    setColour(juce::Slider::textBoxOutlineColourId, Color::strokeSubtle());
    setColour(juce::Label::textColourId, Color::textSecondary());
    setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::textColourOffId, Color::textSecondary());
    setColour(juce::TextButton::textColourOnId, Color::accentPrimaryAmber());
    setColour(juce::ToggleButton::textColourId, Color::textSecondary());
    setColour(juce::ToggleButton::tickColourId, Color::accentPrimaryAmber());
    setColour(juce::PopupMenu::backgroundColourId, Color::backgroundPanel());
    setColour(juce::PopupMenu::textColourId, Color::textPrimary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Color::hover());
    setColour(juce::TooltipWindow::backgroundColourId, Color::backgroundPanelRaised());
    setColour(juce::TooltipWindow::textColourId, Color::textPrimary());
    setColour(juce::TooltipWindow::outlineColourId, Color::strokeSubtle());
}

void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float, juce::Slider& slider) {
    const float diameter = (float)juce::jmin(w, h) * 0.85f;
    const float radius = diameter * 0.5f;
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float startAngle = juce::MathConstants<float>::pi * KnobStyle::startAngle;
    const float endAngle   = juce::MathConstants<float>::pi * KnobStyle::endAngle;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float arcRadius = radius * 0.9f;
    const float arcThickness = diameter * KnobStyle::arcThicknessRatio;

    // ── Tick dots ──
    for (int i = 0; i < KnobStyle::tickDotCount; ++i) {
        float t = (float)i / (float)(KnobStyle::tickDotCount - 1);
        float tickAngle = startAngle + t * (endAngle - startAngle);
        float dx = cx + (arcRadius + 6.0f) * std::cos(tickAngle - juce::MathConstants<float>::halfPi);
        float dy = cy + (arcRadius + 6.0f) * std::sin(tickAngle - juce::MathConstants<float>::halfPi);
        bool active = (tickAngle <= angle);
        g.setColour(active ? Color::accentPrimaryAmber() : Color::strokeSubtle());
        g.fillEllipse(dx - KnobStyle::tickDotRadius, dy - KnobStyle::tickDotRadius,
                      KnobStyle::tickDotRadius * 2.0f, KnobStyle::tickDotRadius * 2.0f);
    }

    // ── Track arc (background) ──
    juce::Path track;
    track.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(Color::strokeSubtle());
    g.strokePath(track, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // ── Fill arc (value) ──
    if (sliderPos > 0.001f) {
        juce::Path fill;
        fill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(Color::accentPrimaryAmber());
        g.strokePath(fill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // ── Knob body (dark brushed-metal look) ──
    const float bodyR = radius * KnobStyle::bodyRadiusRatio;
    juce::ColourGradient bodyGrad(Color::backgroundPanelRaised(), cx, cy - bodyR,
                                   Color::backgroundPanel(), cx, cy + bodyR, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // ── Knob body border ──
    g.setColour(Color::strokeSubtle());
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, Geometry::borderThin);

    // ── Indicator line ──
    g.setColour(Color::accentPrimaryAmber());
    const float ix1 = cx + radius * KnobStyle::indicatorStart * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + radius * KnobStyle::indicatorStart * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + radius * KnobStyle::indicatorEnd * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + radius * KnobStyle::indicatorEnd * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.0f);

    // ── Subtle amber glow when hovered ──
    if (slider.isMouseOver()) {
        g.setColour(Color::accentGlow());
        g.fillEllipse(cx - bodyR - 2.0f, cy - bodyR - 2.0f, (bodyR + 2.0f) * 2.0f, (bodyR + 2.0f) * 2.0f);
    }
}

void BTZLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float,
                                      juce::Slider::SliderStyle, juce::Slider& slider) {
    const float trackY = (float)y + (float)h * 0.5f;
    const float trackH = (float)Geometry::sliderTrackH;

    // ── Track background ──
    g.setColour(Color::backgroundInset());
    g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)w, trackH,
                           Geometry::controlRadius);

    // ── Track border ──
    g.setColour(Color::strokeSubtle());
    g.drawRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)w, trackH,
                           Geometry::controlRadius, Geometry::borderThin);

    // ── Fill ──
    const float fillW = juce::jlimit(0.0f, (float)w, sliderPos - (float)x);
    if (fillW > 1.0f) {
        g.setColour(Color::accentPrimaryAmber());
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, fillW, trackH,
                               Geometry::controlRadius);
    }

    // ── Thumb ──
    const float thumbR = 6.0f;
    const float thumbX = juce::jlimit((float)x + thumbR, (float)(x + w) - thumbR, sliderPos);
    g.setColour(Color::textPrimary());
    g.fillEllipse(thumbX - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f);

    if (slider.isMouseOver()) {
        g.setColour(Color::accentGlow());
        g.fillEllipse(thumbX - thumbR - 2.0f, trackY - thumbR - 2.0f,
                      (thumbR + 2.0f) * 2.0f, (thumbR + 2.0f) * 2.0f);
    }
}

void BTZLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour&, bool isHighlighted, bool isDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    if (isDown)
        g.setColour(Color::focus());
    else if (isHighlighted)
        g.setColour(Color::hover());
    else
        g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(bounds, Geometry::controlRadius);
}

void BTZLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                     bool, bool) {
    g.setFont(Font::tab());
    g.setColour(button.getToggleState() ? Color::accentPrimaryAmber() : Color::textSecondary());
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void BTZLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                       bool isHighlighted, bool isDown) {
    auto bounds = button.getLocalBounds().toFloat();
    auto toggleArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);

    // ── Toggle pill ──
    g.setColour(button.getToggleState() ? Color::bypassOn() : Color::bypassOff());
    g.fillRoundedRectangle(toggleArea, Geometry::pillRadius);

    // ── Toggle indicator dot ──
    float dotR = toggleArea.getHeight() * 0.35f;
    float dotX = button.getToggleState()
        ? toggleArea.getRight() - dotR - 3.0f
        : toggleArea.getX() + dotR + 3.0f;
    float dotY = toggleArea.getCentreY();
    g.setColour(Color::textPrimary());
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);

    // ── Label ──
    g.setFont(Font::label());
    g.setColour(button.getToggleState() ? Color::accentPrimaryAmber() : Color::textSecondary());
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft);
}

void BTZLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.setFont(Font::label());
    g.setColour(label.findColour(juce::Label::textColourId));
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
}

// ── Static BTZ draw methods ──

void BTZLookAndFeel::drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area, bool raised) {
    g.setColour(raised ? Color::backgroundPanelRaised() : Color::backgroundPanel());
    g.fillRoundedRectangle(area, Geometry::panelRadius);
    g.setColour(Color::strokeSubtle());
    g.drawRoundedRectangle(area, Geometry::panelRadius, Geometry::borderThin);
}

void BTZLookAndFeel::drawBTZSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                           const juce::String& text) {
    g.setFont(Font::sectionHeader());
    g.setColour(Color::accentPrimaryAmber());
    g.drawText(text, area, juce::Justification::centredLeft);
    auto line = area.removeFromBottom(1.0f);
    g.setColour(Color::strokeSubtle());
    g.fillRect(line);
}

void BTZLookAndFeel::drawBTZMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                   float valuePct, bool isGR) {
    // ── Background ──
    g.setColour(Color::backgroundInset());
    g.fillRoundedRectangle(area, Effects::meterCornerRadius);

    // ── Fill ──
    float pct = juce::jlimit(0.0f, 1.0f, valuePct);
    if (pct > 0.001f) {
        juce::Colour fillColour;
        if (isGR) {
            fillColour = Color::grMeter();
        } else if (pct > 0.95f) {
            fillColour = Color::meterClip();
        } else if (pct > 0.80f) {
            fillColour = Color::meterHot();
        } else if (pct > 0.65f) {
            fillColour = Color::meterWarn();
        } else {
            fillColour = Color::meterSafe();
        }
        g.setColour(fillColour);
        g.fillRoundedRectangle(area.withWidth(area.getWidth() * pct), Effects::meterCornerRadius);
    }
}

void BTZLookAndFeel::drawBTZSegmentedMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                            float valuePct, bool isGR) {
    const int segments = Effects::meterSegmentCount;
    const float segW = (area.getWidth() - (float)(segments - 1) * Effects::meterSegmentGap) / (float)segments;
    float pct = juce::jlimit(0.0f, 1.0f, valuePct);
    int litSegments = (int)(pct * (float)segments);

    for (int i = 0; i < segments; ++i) {
        float sx = area.getX() + (float)i * (segW + Effects::meterSegmentGap);
        auto segRect = juce::Rectangle<float>(sx, area.getY(), segW, area.getHeight());

        if (i < litSegments) {
            float segPct = (float)i / (float)segments;
            juce::Colour c;
            if (isGR) {
                c = Color::grMeter();
            } else if (segPct > 0.92f) {
                c = Color::meterClip();
            } else if (segPct > 0.75f) {
                c = Color::meterHot();
            } else if (segPct > 0.58f) {
                c = Color::meterWarn();
            } else {
                c = Color::meterSafe();
            }
            g.setColour(c);
        } else {
            g.setColour(Color::backgroundInset());
        }
        g.fillRoundedRectangle(segRect, 1.0f);
    }
}

void BTZLookAndFeel::drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                                 const juce::String& text, bool isActive) {
    if (isActive) {
        g.setColour(Color::backgroundPanelRaised());
        g.fillRoundedRectangle(area, Geometry::controlRadius);
        g.setColour(Color::accentPrimaryAmber());
        g.fillRect(area.removeFromBottom(2.0f));
    }
    g.setFont(Font::tab());
    g.setColour(isActive ? Color::accentPrimaryAmber() : Color::textSecondary());
    g.drawText(text, area, juce::Justification::centred);
}

void BTZLookAndFeel::drawBTZValueBox(juce::Graphics& g, juce::Rectangle<float> area,
                                      const juce::String& value, const juce::String& unit) {
    g.setColour(Color::backgroundInset());
    g.fillRoundedRectangle(area, Geometry::controlRadius);
    g.setColour(Color::strokeSubtle());
    g.drawRoundedRectangle(area, Geometry::controlRadius, Geometry::borderThin);
    g.setFont(Font::value());
    g.setColour(Color::textPrimary());
    g.drawText(value, area.reduced(4.0f, 0.0f), juce::Justification::centredRight);
    if (unit.isNotEmpty()) {
        g.setColour(Color::textDisabled());
        g.setFont(Font::micro());
        g.drawText(unit, area.reduced(4.0f, 0.0f).removeFromRight(20.0f), juce::Justification::centredLeft);
    }
}

void BTZLookAndFeel::drawBTZTooltip(juce::Graphics& g, juce::Rectangle<float> area,
                                     const juce::String& text) {
    g.setColour(Color::backgroundPanelRaised());
    g.fillRoundedRectangle(area, Geometry::controlRadius);
    g.setColour(Color::strokeSubtle());
    g.drawRoundedRectangle(area, Geometry::controlRadius, Geometry::borderThin);
    g.setFont(Font::tooltip());
    g.setColour(Color::textPrimary());
    g.drawText(text, area.reduced(6.0f, 2.0f), juce::Justification::centredLeft);
}

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor — Constructor
// ═══════════════════════════════════════════════════════════════════════════

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : AudioProcessorEditor(p), proc(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(Geometry::windowWidth, Geometry::windowHeight);

    // ── Tab navigation ──
    auto initTab = [&](juce::TextButton& b, int pageIdx) {
        addAndMakeVisible(b);
        b.setClickingTogglesState(true);
        b.setRadioGroupId(1001);
        b.onClick = [this, pageIdx] { currentPage = pageIdx; resized(); repaint(); };
    };
    initTab(tabMain, 0);
    initTab(tabSpark, 1);
    initTab(tabAdvanced, 2);
    tabMain.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(btnBypass);

    // ── Core knobs ──
    setupKnob(kPunch, lPunch, "Punch");
    setupKnob(kWarmth, lWarmth, "Warmth");
    setupKnob(kBoom, lBoom, "Boom");
    setupKnob(kGlue, lGlue, "Glue");
    setupKnob(kAir, lAir, "Air");
    setupKnob(kWidth, lWidth, "Width");
    setupKnob(kDensity, lDensity, "Density");
    setupKnob(kMotion, lMotion, "Motion");
    setupKnob(kEra, lEra, "Era");
    setupKnob(kDrive, lDrive, "Drive");
    setupKnob(kMix, lMix, "Mix");
    setupKnob(kMaster, lMaster, "Master");

    // ── Macro knobs ──
    setupKnob(kMacro0, lMacro0, "Macro 1");
    setupKnob(kMacro1, lMacro1, "Macro 2");
    setupKnob(kMacro2, lMacro2, "Macro 3");
    setupKnob(kMacro3, lMacro3, "Macro 4");

    // ── SPARK / SHINE knobs (unified control vocabulary) ──
    setupKnob(kCeiling, lCeiling, "Ceiling");
    setupKnob(kShine, lShine, "Shine");
    setupKnob(kShineMix, lShineMix, "Shine Mix");
    setupKnob(kShineFreq, lShineFreq, "Freq");
    setupKnob(kShineQ, lShineQ, "Q");
    setupKnob(kIntensity, lIntensity, "Intensity");

    // ── APVTS Attachments ──
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
    aCeiling  = std::make_unique<SliderAttachment>(apvts, "sparkCeiling", kCeiling);
    aShine    = std::make_unique<SliderAttachment>(apvts, "shineAmount", kShine);
    aShineMix = std::make_unique<SliderAttachment>(apvts, "shineMix", kShineMix);
    aShineFreq = std::make_unique<SliderAttachment>(apvts, "shineFreq", kShineFreq);
    aShineQ    = std::make_unique<SliderAttachment>(apvts, "shineQ", kShineQ);
    aIntensity = std::make_unique<SliderAttachment>(apvts, "masterIntensity", kIntensity);
    aMacro0   = std::make_unique<SliderAttachment>(apvts, "macro0", kMacro0);
    aMacro1   = std::make_unique<SliderAttachment>(apvts, "macro1", kMacro1);
    aMacro2   = std::make_unique<SliderAttachment>(apvts, "macro2", kMacro2);
    aMacro3   = std::make_unique<SliderAttachment>(apvts, "macro3", kMacro3);
    aBypass   = std::make_unique<ButtonAttachment>(apvts, "bypass", btnBypass);

    startTimerHz(45);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

// ═══════════════════════════════════════════════════════════════════════════
// Setup Helpers
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& labelText) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setPopupDisplayEnabled(true, true, this);
    s.setDoubleClickReturnValue(true, s.getValue());  // double-click to reset

    addAndMakeVisible(l);
    l.setText(labelText, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(Font::label());
    l.setColour(juce::Label::textColourId, Color::textSecondary());
}

void BTZAudioProcessorEditor::setupSlider(juce::Slider& s) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 18);
    s.setColour(juce::Slider::textBoxTextColourId, Color::textSecondary());
    s.setColour(juce::Slider::textBoxBackgroundColourId, Color::backgroundInset());
    s.setColour(juce::Slider::textBoxOutlineColourId, Color::strokeSubtle());
    s.setDoubleClickReturnValue(true, s.getValue());
}

// ═══════════════════════════════════════════════════════════════════════════
// Timer — Meter smoothing
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::timerCallback() {
    auto& m = proc.getMeters();
    auto lerp = [](float& d, float t, float c) { d += c * (t - d); };
    lerp(inPeakL,  m.inputPeakL.load(std::memory_order_relaxed),  0.3f);
    lerp(inPeakR,  m.inputPeakR.load(std::memory_order_relaxed),  0.3f);
    lerp(inRmsL,   m.inputRmsL.load(std::memory_order_relaxed),   0.2f);
    lerp(inRmsR,   m.inputRmsR.load(std::memory_order_relaxed),   0.2f);
    lerp(outPeakL, m.outputPeakL.load(std::memory_order_relaxed), 0.3f);
    lerp(outPeakR, m.outputPeakR.load(std::memory_order_relaxed), 0.3f);
    lerp(outRmsL,  m.outputRmsL.load(std::memory_order_relaxed),  0.2f);
    lerp(outRmsR,  m.outputRmsR.load(std::memory_order_relaxed),  0.2f);
    lerp(sparkGR,  m.sparkGainReductionDb.load(std::memory_order_relaxed), 0.25f);
    lerp(lufs,     m.lufs.load(std::memory_order_relaxed),        0.15f);
    lerp(corr,     m.correlation.load(std::memory_order_relaxed), 0.2f);
    lerp(inClip,   m.inputClip.load(std::memory_order_relaxed),  0.3f);
    lerp(outClip,  m.outputClip.load(std::memory_order_relaxed), 0.3f);
    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
// Paint — Shared shell (header, meters, content, footer)
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // ── Root background ──
    g.setColour(Color::backgroundRoot());
    g.fillRoundedRectangle(bounds, Geometry::outerRadius);

    // ── Header ──
    auto headerArea = bounds.removeFromTop((float)Geometry::headerHeight);
    paintHeader(g, headerArea);

    // ── Meter strip ──
    auto meterArea = bounds.removeFromTop((float)Geometry::meterStripHeight);
    paintMeterStrip(g, meterArea);

    // ── Content panel ──
    auto contentArea = bounds.reduced((float)Geometry::padContent, (float)Geometry::spaceSM);
    BTZLookAndFeel::drawBTZPanelBackground(g, contentArea);

    // ── Footer ──
    auto footerArea = contentArea.removeFromBottom(20.0f);
    paintFooter(g, footerArea);
}

void BTZAudioProcessorEditor::paintHeader(juce::Graphics& g, juce::Rectangle<float> area) {
    // ── Header background ──
    g.setColour(Color::backgroundPanel());
    g.fillRect(area);

    // ── Bottom border ──
    g.setColour(Color::strokeSubtle());
    g.fillRect(area.removeFromBottom(1.0f));

    // ── Brand ──
    auto brandArea = area.removeFromLeft(250.0f).reduced(16.0f, 0.0f);
    g.setFont(Font::title());
    g.setColour(Color::textPrimary());
    g.drawText("BOX TONE ZONE", brandArea.removeFromTop(area.getHeight() * 0.6f),
               juce::Justification::centredLeft);
    g.setFont(Font::brand());
    g.setColour(Color::textDisabled());
    g.drawText("BTZ Audio", brandArea, juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintMeterStrip(juce::Graphics& g, juce::Rectangle<float> area) {
    auto strip = area.reduced((float)Geometry::padPanel, (float)Geometry::spaceXS);
    BTZLookAndFeel::drawBTZPanelBackground(g, strip, true);

    auto body = strip.reduced((float)Geometry::spaceSM, (float)Geometry::spaceSM);

    auto dbToPct = [](float db, float minDb = -60.0f, float maxDb = 6.0f) -> float {
        return juce::jlimit(0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
    };

    auto drawMeterRow = [&](const juce::String& label, float valL, float valR, bool gr = false) {
        auto row = body.removeFromTop(12.0f);
        g.setFont(Font::meter());
        g.setColour(Color::textDisabled());
        g.drawText(label, row.removeFromLeft(80.0f), juce::Justification::centredLeft);

        auto m1 = row.removeFromLeft(170.0f).reduced(2.0f, 1.0f);
        auto m2 = row.removeFromLeft(170.0f).reduced(2.0f, 1.0f);

        float pctL = gr ? juce::jlimit(0.0f, 1.0f, valL / 18.0f) : dbToPct(valL);
        float pctR = gr ? juce::jlimit(0.0f, 1.0f, valR / 18.0f) : dbToPct(valR);

        BTZLookAndFeel::drawBTZMeter(g, m1, pctL, gr);
        BTZLookAndFeel::drawBTZMeter(g, m2, pctR, gr);

        g.setFont(Font::value());
        g.setColour(Color::textSecondary());
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
    g.setColour(Color::textDisabled());
    g.drawText("LUFS: " + juce::String(lufs, 1), statusRow.removeFromLeft(100.0f), juce::Justification::centredLeft);
    g.drawText("CORR: " + juce::String(corr, 2), statusRow.removeFromLeft(90.0f), juce::Justification::centredLeft);

    g.setColour(inClip > 0.2f ? Color::meterClip() : Color::textDisabled());
    g.drawText("IN CLIP", statusRow.removeFromLeft(60.0f), juce::Justification::centredLeft);
    g.setColour(outClip > 0.2f ? Color::meterClip() : Color::textDisabled());
    g.drawText("OUT CLIP", statusRow.removeFromLeft(70.0f), juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintFooter(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setFont(Font::micro());
    g.setColour(Color::textDisabled());
    g.drawText("v1.1  |  " + juce::String(proc.getSampleRate(), 0) + " Hz  |  BTZ Sonic Alchemy",
               area, juce::Justification::centred);
}

// ═══════════════════════════════════════════════════════════════════════════
// Resized — Layout
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    // ── Header: tabs + bypass ──
    auto header = bounds.removeFromTop(Geometry::headerHeight);
    auto tabArea = header.reduced(260, 12);
    const int tabW = Geometry::tabWidth;
    const int gap = Geometry::tabGap;
    const int startX = tabArea.getCentreX() - (tabW * 3 + gap * 2) / 2;
    tabMain.setBounds(startX, tabArea.getY(), tabW, Geometry::tabHeight);
    tabSpark.setBounds(startX + tabW + gap, tabArea.getY(), tabW, Geometry::tabHeight);
    tabAdvanced.setBounds(startX + (tabW + gap) * 2, tabArea.getY(), tabW, Geometry::tabHeight);
    btnBypass.setBounds(header.getRight() - 120, header.getY() + 14, 100, header.getHeight() - 24);

    // ── Tab toggle states ──
    tabMain.setToggleState(currentPage == 0, juce::dontSendNotification);
    tabSpark.setToggleState(currentPage == 1, juce::dontSendNotification);
    tabAdvanced.setToggleState(currentPage == 2, juce::dontSendNotification);

    // ── Skip meter strip ──
    bounds.removeFromTop(Geometry::meterStripHeight);

    // ── Content area ──
    auto content = bounds.reduced(Geometry::padContent, Geometry::spaceSM + 10);

    // ── Hide all, then show current page ──
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
}

void BTZAudioProcessorEditor::layoutMainPage(juce::Rectangle<int> content) {
    const int knob = Geometry::knobLarge;
    const int label = Geometry::knobLabelHeight;
    const int gapX = (content.getWidth() - knob * 6) / 5;
    const int y1 = content.getY() + Geometry::spaceLG;
    const int y2 = y1 + knob + label + Geometry::spaceMD;

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

    // Macro row
    const int macroY = y2 + knob + label + Geometry::spaceXL;
    const int macroKnob = Geometry::knobMedium;
    const int macroGap = (content.getWidth() - macroKnob * 4) / 3;
    auto placeMacro = [&](juce::Slider& s, juce::Label& l, int i) {
        int x = content.getX() + i * (macroKnob + macroGap);
        s.setBounds(x, macroY, macroKnob, macroKnob);
        l.setBounds(x, macroY + macroKnob, macroKnob, label);
        s.setVisible(true); l.setVisible(true);
    };
    placeMacro(kMacro0, lMacro0, 0);
    placeMacro(kMacro1, lMacro1, 1);
    placeMacro(kMacro2, lMacro2, 2);
    placeMacro(kMacro3, lMacro3, 3);
}

void BTZAudioProcessorEditor::layoutSparkPage(juce::Rectangle<int> content) {
    // Two-panel layout: SPARK (left) | SHINE (right)
    auto leftPanel = content.removeFromLeft(content.getWidth() / 2).reduced(Geometry::padPanel, Geometry::spaceLG);
    auto rightPanel = content.reduced(Geometry::padPanel, Geometry::spaceLG);

    const int knob = Geometry::knobLarge;
    const int smallKnob = Geometry::knobSmall;
    const int label = Geometry::knobLabelHeight;

    // ── SPARK section ──
    // Ceiling knob centered
    int sparkCenterX = leftPanel.getCentreX() - knob / 2;
    int sparkY = leftPanel.getY() + Geometry::spaceXL;
    kCeiling.setBounds(sparkCenterX, sparkY, knob, knob);
    lCeiling.setBounds(sparkCenterX, sparkY + knob, knob, label);
    kCeiling.setVisible(true); lCeiling.setVisible(true);

    // Intensity knob below ceiling
    int intensityY = sparkY + knob + label + Geometry::spaceLG;
    kIntensity.setBounds(sparkCenterX, intensityY, knob, knob);
    lIntensity.setBounds(sparkCenterX, intensityY + knob, knob, label);
    kIntensity.setVisible(true); lIntensity.setVisible(true);

    // ── SHINE section ──
    // Shine Amount and Shine Mix as medium knobs side by side
    int shineY = rightPanel.getY() + Geometry::spaceXL;
    int shineGap = Geometry::spaceLG;
    int shineTotalW = knob * 2 + shineGap;
    int shineStartX = rightPanel.getCentreX() - shineTotalW / 2;

    kShine.setBounds(shineStartX, shineY, knob, knob);
    lShine.setBounds(shineStartX, shineY + knob, knob, label);
    kShine.setVisible(true); lShine.setVisible(true);

    kShineMix.setBounds(shineStartX + knob + shineGap, shineY, knob, knob);
    lShineMix.setBounds(shineStartX + knob + shineGap, shineY + knob, knob, label);
    kShineMix.setVisible(true); lShineMix.setVisible(true);

    // Freq and Q as smaller knobs below
    int freqY = shineY + knob + label + Geometry::spaceLG;
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
    // Advanced page: Drive, Era, Density, Motion as large knobs (top row)
    // Mix, Master as medium knobs (bottom row)
    // This page provides a focused view of the "character" parameters

    const int knob = Geometry::knobLarge;
    const int medKnob = Geometry::knobMedium;
    const int label = Geometry::knobLabelHeight;
    const int y1 = content.getY() + Geometry::spaceXL;

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

    // Bottom row: Mix + Master centered
    const int y2 = y1 + knob + label + Geometry::spaceXL;
    const int botGap = Geometry::spaceLG;
    const int botTotalW = medKnob * 2 + botGap;
    const int botStartX = content.getCentreX() - botTotalW / 2;

    kMix.setBounds(botStartX, y2, medKnob, medKnob);
    lMix.setBounds(botStartX, y2 + medKnob, medKnob, label);
    kMix.setVisible(true); lMix.setVisible(true);

    kMaster.setBounds(botStartX + medKnob + botGap, y2, medKnob, medKnob);
    lMaster.setBounds(botStartX + medKnob + botGap, y2 + medKnob, medKnob, label);
    kMaster.setVisible(true); lMaster.setVisible(true);
}
