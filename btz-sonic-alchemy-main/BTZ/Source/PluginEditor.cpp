/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v10
  ────────────────────────────────────────────────────────────────────────
  v10: Complete UI/UX overhaul — Simple/Standard/Advanced view modes,
       glassmorphism panels, harmonic visualizer, direct manipulation,
       micro-interactions, startup reveal, processing indicators,
       gain reduction ribbon, tooltip overlay.
*/
#include "PluginEditor.h"

using namespace BTZTheme;
using namespace BTZColours;
using namespace BTZTokens;

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel — v4 with 3D knobs, hover glow, glassmorphism
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

void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float, juce::Slider& slider) {
    const auto accent = BTZColours::accentFor(currentModule);
    const float diameter = (float)juce::jmin(w, h) * 0.85f;
    const float radius = diameter * 0.5f;
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float startAngle = juce::MathConstants<float>::pi * KnobStyle::startAngle;
    const float endAngle   = juce::MathConstants<float>::pi * KnobStyle::endAngle;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float arcRadius = radius * 0.9f;
    const float arcThickness = diameter * KnobStyle::arcThicknessRatio;

    // v4: Hover glow micro-interaction
    if (slider.isMouseOver() || slider.isMouseButtonDown()) {
        float glowAlpha = slider.isMouseButtonDown() ? activeGlowAlpha : hoverGlowAlpha;
        g.setColour(accent.withAlpha(glowAlpha));
        g.fillEllipse(cx - radius - 4, cy - radius - 4, (radius + 4) * 2, (radius + 4) * 2);
    }

    // Track arc
    juce::Path track;
    track.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(rule);
    g.strokePath(track, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Fill arc
    if (sliderPos > 0.001f) {
        juce::Path fill;
        fill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(accent);
        g.strokePath(fill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // Tick dots
    for (int i = 0; i < KnobStyle::tickDotCount; ++i) {
        float t = (float)i / (float)(KnobStyle::tickDotCount - 1);
        float dotAngle = startAngle + t * (endAngle - startAngle);
        float dotR = arcRadius + 6.0f;
        float dx = cx + dotR * std::cos(dotAngle - juce::MathConstants<float>::halfPi);
        float dy = cy + dotR * std::sin(dotAngle - juce::MathConstants<float>::halfPi);
        g.setColour(t <= sliderPos ? accent.withAlpha(0.6f) : hairline);
        g.fillEllipse(dx - KnobStyle::tickDotRadius, dy - KnobStyle::tickDotRadius,
                      KnobStyle::tickDotRadius * 2, KnobStyle::tickDotRadius * 2);
    }

    // v4: 3D knob body with convexity
    const float bodyR = radius * KnobStyle::bodyRadiusRatio;
    juce::ColourGradient bodyGrad(
        knobRimLight, cx - bodyR * 0.3f, cy - bodyR * 0.5f,
        knobShadow, cx + bodyR * 0.2f, cy + bodyR * 0.7f, false
    );
    bodyGrad.addColour(0.3, knobHighlight);
    bodyGrad.addColour(0.6, knobMid);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Inner shadow for depth
    juce::ColourGradient innerShadow(
        juce::Colours::transparentBlack, cx, cy - bodyR * 0.3f,
        juce::Colour(0x18000000), cx, cy + bodyR, false
    );
    g.setGradientFill(innerShadow);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Rim highlight
    g.setColour(knobRimLight.withAlpha(KnobStyle::rimLightOpacity));
    juce::Path rimArc;
    rimArc.addCentredArc(cx, cy, bodyR - 1, bodyR - 1, 0.0f, -2.5f, -0.6f, true);
    g.strokePath(rimArc, juce::PathStrokeType(0.8f));

    // Indicator line
    g.setColour(accent);
    const float ix1 = cx + bodyR * KnobStyle::indicatorStart * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + bodyR * KnobStyle::indicatorStart * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + bodyR * KnobStyle::indicatorEnd * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + bodyR * KnobStyle::indicatorEnd * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.0f);
}

void BTZLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float,
                                      juce::Slider::SliderStyle, juce::Slider&) {
    const auto accent = BTZColours::accentFor(currentModule);
    const float trackY = (float)y + (float)h * 0.5f;
    const float trackH = (float)Geometry::sliderTrackH;

    g.setColour(juce::Colour(0xFF101218));
    g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)w, trackH, 2.0f);

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
    juce::Colour bg = isDown ? stone : (isHighlighted ? charcoal : juce::Colours::transparentBlack);
    if (button.getToggleState()) bg = amber.withAlpha(0.15f);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, Radius::sm);
    g.setColour(glassBorder);
    g.drawRoundedRectangle(bounds, Radius::sm, Border::glass);
}

void BTZLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) {
    g.setFont(Font::tab());
    g.setColour(button.getToggleState() ? amber : bone);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
    if (button.getToggleState()) {
        g.setColour(amber);
        g.fillRect(button.getLocalBounds().toFloat().removeFromBottom(2.0f));
    }
}

void BTZLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) {
    auto bounds = button.getLocalBounds().toFloat();
    auto toggleArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);
    g.setColour(button.getToggleState() ? amber : stone);
    g.fillRoundedRectangle(toggleArea, Geometry::pillRadius);
    float dotR = toggleArea.getHeight() * 0.35f;
    float dotX = button.getToggleState()
        ? toggleArea.getRight() - dotR - 3.0f
        : toggleArea.getX() + dotR + 3.0f;
    g.setColour(cream);
    g.fillEllipse(dotX - dotR, toggleArea.getCentreY() - dotR, dotR * 2.0f, dotR * 2.0f);
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
                                   int, int, int, int, juce::ComboBox&) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)w, (float)h);
    g.setColour(panel);
    g.fillRoundedRectangle(bounds, Radius::sm);
    g.setColour(glassBorder);
    g.drawRoundedRectangle(bounds, Radius::sm, Border::glass);
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
    if (isSeparator) { g.setColour(rule); g.fillRect(area.reduced(8, 0).withHeight(1).withCentre(area.getCentre())); return; }
    if (isHighlighted) { g.setColour(stone); g.fillRect(area); }
    g.setFont(Font::label());
    g.setColour(isActive ? (isTicked ? amber : cream) : mute);
    g.drawText(text, area.reduced(12, 0), juce::Justification::centredLeft);
}

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

    // Tick dots
    for (int i = 0; i < Dim::tickDotCount; ++i) {
        float t = (float)i / (float)(Dim::tickDotCount - 1);
        float tickAngle = startAngle + t * (endAngle - startAngle);
        float tx = cx + (arcRadius + 6.0f) * std::cos(tickAngle - juce::MathConstants<float>::halfPi);
        float ty = cy + (arcRadius + 6.0f) * std::sin(tickAngle - juce::MathConstants<float>::halfPi);
        g.setColour(t <= normValue ? accent.withAlpha(0.7f) : rule);
        g.fillEllipse(tx - 1.5f, ty - 1.5f, 3.0f, 3.0f);
    }

    // Halo arc
    juce::Path haloTrack;
    haloTrack.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(rule);
    g.strokePath(haloTrack, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (normValue > 0.001f) {
        juce::Path haloFill;
        haloFill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(accent);
        g.strokePath(haloFill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3D body
    const float bodyR = radius * 0.72f;
    juce::ColourGradient bodyGrad(knobRimLight, cx, cy - bodyR * 0.5f, knobShadow, cx, cy + bodyR * 0.7f, false);
    bodyGrad.addColour(0.35, knobHighlight);
    bodyGrad.addColour(0.65, knobMid);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour(bone.withAlpha(0.2f));
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.5f);

    // Indicator
    g.setColour(accent);
    const float ix1 = cx + bodyR * 0.3f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + bodyR * 0.3f * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + bodyR * 0.85f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + bodyR * 0.85f * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.5f);

    if (isHovered) {
        g.setColour(accent.withAlpha(hoverGlowAlpha));
        g.fillEllipse(cx - radius, cy - radius, diameter, diameter);
    }
}

void BTZLookAndFeel::drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area) {
    BTZTheme::drawGlassPanel(g, area);
}

void BTZLookAndFeel::drawBTZSectionHeader(juce::Graphics& g, juce::Rectangle<float> area,
                                           const juce::String& text, BTZColours::Module module) {
    g.setFont(Font::sectionHeader());
    g.setColour(accentFor(module));
    g.drawText(text, area, juce::Justification::centredLeft);
    auto lineArea = area;
    lineArea.removeFromLeft(g.getCurrentFont().getStringWidthFloat(text) + 12.0f);
    g.setColour(rule);
    g.fillRect(lineArea.withHeight(1.0f).withCentre(area.getCentre()));
}

void BTZLookAndFeel::drawBTZMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                   float valuePct, bool isGR) {
    g.setColour(juce::Colour(0xFF101218));
    g.fillRoundedRectangle(area, Effects::meterCornerRadius);
    const float fillW = area.getWidth() * juce::jlimit(0.0f, 1.0f, valuePct);
    if (fillW > 0.5f) {
        juce::Colour fillColour = isGR ? amber :
            (valuePct > 0.95f ? meterClip : valuePct > 0.75f ? meterHot :
             valuePct > 0.5f ? meterOptimal : meterSafe);
        g.setColour(fillColour);
        g.fillRoundedRectangle(area.withWidth(fillW), Effects::meterCornerRadius);
    }
}

void BTZLookAndFeel::drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                                 const juce::String& text, bool isActive, BTZColours::Module module) {
    if (isActive) {
        g.setColour(accentFor(module).withAlpha(0.12f));
        g.fillRoundedRectangle(area, Radius::sm);
    }
    g.setFont(Font::tab());
    g.setColour(isActive ? accentFor(module) : bone);
    g.drawText(text, area, juce::Justification::centred);
    if (isActive) {
        g.setColour(accentFor(module));
        g.fillRect(area.removeFromBottom(2.0f));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// BTZAudioProcessorEditor — Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&lookAndFeel);

    // Window setup: resizable with constraints
    constrainer.setMinimumSize(Window::minWidth, Window::minHeight);
    constrainer.setMaximumSize(Window::maxWidth, Window::maxHeight);
    constrainer.setFixedAspectRatio((double)Window::defaultWidth / (double)Window::defaultHeight);
    setConstrainer(&constrainer);
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &constrainer);
    addAndMakeVisible(*resizer);
    setResizable(true, false);
    setSize(Window::defaultWidth, Window::defaultHeight);

    // View mode buttons
    auto setupViewBtn = [this](juce::TextButton& btn) {
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(100);
        addAndMakeVisible(btn);
    };
    setupViewBtn(btnSimple);
    setupViewBtn(btnStandard);
    setupViewBtn(btnAdvanced);
    btnStandard.setToggleState(true, juce::dontSendNotification);

    btnSimple.onClick = [this] { setViewMode(ViewMode::Simple); };
    btnStandard.onClick = [this] { setViewMode(ViewMode::Standard); };
    btnAdvanced.onClick = [this] { setViewMode(ViewMode::Advanced); };

    // Tab navigation (within Standard mode)
    auto setupTab = [this](juce::TextButton& btn) {
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(200);
        addAndMakeVisible(btn);
    };
    setupTab(tabMain); setupTab(tabSpark); setupTab(tabDetail);
    tabMain.setToggleState(true, juce::dontSendNotification);
    tabMain.onClick = [this] { currentPage = 0; resized(); repaint(); };
    tabSpark.onClick = [this] { currentPage = 1; resized(); repaint(); };
    tabDetail.onClick = [this] { currentPage = 2; resized(); repaint(); };

    // Bypass
    addAndMakeVisible(btnBypass);
    aBypass = std::make_unique<ButtonAttachment>(proc.getAPVTS(), "bypass", btnBypass);

    // Toolbar buttons
    addAndMakeVisible(btnUndo); addAndMakeVisible(btnRedo);
    addAndMakeVisible(btnAB); addAndMakeVisible(btnCopyAB);
    addAndMakeVisible(btnPresetPrev); addAndMakeVisible(btnPresetNext);
    addAndMakeVisible(btnPresetSave); addAndMakeVisible(lblPresetName);
    lblPresetName.setText("Default", juce::dontSendNotification);
    lblPresetName.setJustificationType(juce::Justification::centred);

    btnUndo.onClick = [this] { proc.performUndo(); };
    btnRedo.onClick = [this] { proc.performRedo(); };
    btnAB.onClick = [this] { proc.toggleAB(); btnAB.setButtonText(proc.isOnStateB() ? "B" : "A"); };
    btnCopyAB.onClick = [this] { proc.copyAtoB(); };
    btnPresetPrev.onClick = [this] { proc.loadPreviousPreset(); };
    btnPresetNext.onClick = [this] { proc.loadNextPreset(); };
    btnPresetSave.onClick = [this] { proc.saveCurrentPreset("User Preset"); };

    // Core knobs
    setupKnob(kPunch, lPunch, "PUNCH"); setupKnob(kWarmth, lWarmth, "WARMTH");
    setupKnob(kBoom, lBoom, "BOOM"); setupKnob(kGlue, lGlue, "GLUE");
    setupKnob(kAir, lAir, "AIR"); setupKnob(kWidth, lWidth, "WIDTH");
    setupKnob(kDensity, lDensity, "DENSITY"); setupKnob(kMotion, lMotion, "MOTION");
    setupKnob(kEra, lEra, "ERA"); setupKnob(kDrive, lDrive, "DRIVE");
    setupKnob(kMix, lMix, "MIX"); setupKnob(kMaster, lMaster, "MASTER");
    setupMacroKnob(kMacro0, lMacro0, "MACRO 1"); setupMacroKnob(kMacro1, lMacro1, "MACRO 2");
    setupMacroKnob(kMacro2, lMacro2, "MACRO 3"); setupMacroKnob(kMacro3, lMacro3, "MACRO 4");
    setupKnob(kCeiling, lCeiling, "CEILING");
    setupKnob(kShine, lShine, "SHINE"); setupKnob(kShineMix, lShineMix, "SHINE MIX");
    setupSmallKnob(kShineFreq, lShineFreq, "FREQ"); setupSmallKnob(kShineQ, lShineQ, "Q");
    setupKnob(kIntensity, lIntensity, "INTENSITY");
    setupSmallKnob(kResTame, lResTame, "RES TAME"); setupSmallKnob(kTransSens, lTransSens, "TRANSIENT");

    // Combos
    cGlueScHpf.addItemList({"Off", "80 Hz", "120 Hz", "200 Hz", "300 Hz"}, 1);
    addAndMakeVisible(cGlueScHpf);
    lGlueScHpf.setText("SC HPF", juce::dontSendNotification);
    addAndMakeVisible(lGlueScHpf);

    cSatModel.addItemList({"Tanh", "Tube", "Tape", "Transistor", "Transformer",
                           "Neural Neve", "Neural API", "Neural SSL", "Neural Custom",
                           "WDF Tube", "WDF Transformer"}, 1);
    addAndMakeVisible(cSatModel);
    lSatModel.setText("SAT MODEL", juce::dontSendNotification);
    addAndMakeVisible(lSatModel);

    addAndMakeVisible(btnMidSide);
    cMultiband.addItemList({"1 Band", "2 Bands", "3 Bands", "4 Bands", "5 Bands", "6 Bands"}, 1);
    addAndMakeVisible(cMultiband);
    lMultiband.setText("BANDS", juce::dontSendNotification);
    addAndMakeVisible(lMultiband);

    cQuality.addItemList({"Eco (1x)", "Standard (2x)", "High (4x)", "Ultra (8x)"}, 1);
    addAndMakeVisible(cQuality);
    lQuality.setText("QUALITY", juce::dontSendNotification);
    addAndMakeVisible(lQuality);

    // Attachments
    auto& apvts = proc.getAPVTS();
    aPunch = std::make_unique<SliderAttachment>(apvts, "punch", kPunch);
    aWarmth = std::make_unique<SliderAttachment>(apvts, "warmth", kWarmth);
    aBoom = std::make_unique<SliderAttachment>(apvts, "boom", kBoom);
    aGlue = std::make_unique<SliderAttachment>(apvts, "glue", kGlue);
    aAir = std::make_unique<SliderAttachment>(apvts, "air", kAir);
    aWidth = std::make_unique<SliderAttachment>(apvts, "width", kWidth);
    aDensity = std::make_unique<SliderAttachment>(apvts, "density", kDensity);
    aMotion = std::make_unique<SliderAttachment>(apvts, "motion", kMotion);
    aEra = std::make_unique<SliderAttachment>(apvts, "era", kEra);
    aDrive = std::make_unique<SliderAttachment>(apvts, "drive", kDrive);
    aMix = std::make_unique<SliderAttachment>(apvts, "mix", kMix);
    aMaster = std::make_unique<SliderAttachment>(apvts, "master", kMaster);
    aCeiling = std::make_unique<SliderAttachment>(apvts, "ceiling", kCeiling);
    aShine = std::make_unique<SliderAttachment>(apvts, "shine", kShine);
    aShineMix = std::make_unique<SliderAttachment>(apvts, "shineMix", kShineMix);
    aIntensity = std::make_unique<SliderAttachment>(apvts, "intensity", kIntensity);
    aShineFreq = std::make_unique<SliderAttachment>(apvts, "shineFreq", kShineFreq);
    aShineQ = std::make_unique<SliderAttachment>(apvts, "shineQ", kShineQ);
    aMacro0 = std::make_unique<SliderAttachment>(apvts, "macro0", kMacro0);
    aMacro1 = std::make_unique<SliderAttachment>(apvts, "macro1", kMacro1);
    aMacro2 = std::make_unique<SliderAttachment>(apvts, "macro2", kMacro2);
    aMacro3 = std::make_unique<SliderAttachment>(apvts, "macro3", kMacro3);
    aResTame = std::make_unique<SliderAttachment>(apvts, "resTame", kResTame);
    aTransSens = std::make_unique<SliderAttachment>(apvts, "transSens", kTransSens);
    aBypass = std::make_unique<ButtonAttachment>(apvts, "bypass", btnBypass);
    aGlueScHpf = std::make_unique<ComboAttachment>(apvts, "glueScHpf", cGlueScHpf);
    aSatModel = std::make_unique<ComboAttachment>(apvts, "satModel", cSatModel);
    aMidSide = std::make_unique<ButtonAttachment>(apvts, "midSide", btnMidSide);
    aMultiband = std::make_unique<ComboAttachment>(apvts, "numBands", cMultiband);
    aQuality = std::make_unique<ComboAttachment>(apvts, "quality", cQuality);

    // v10: Custom components
    addAndMakeVisible(harmonicViz);
    addAndMakeVisible(glassPresetPanel);
    addAndMakeVisible(glassControlPanel);
    addAndMakeVisible(glassVisualizerPanel);
    addAndMakeVisible(tooltipOverlay);
    addAndMakeVisible(satIndicator);
    addAndMakeVisible(compIndicator);
    addAndMakeVisible(limiterIndicator);
    addAndMakeVisible(spectrumDisplay);
    addAndMakeVisible(grRibbon);

    // Simple Mode knobs (hidden by default)
    addAndMakeVisible(simpleKnobDrive);
    addAndMakeVisible(simpleKnobTone);
    addAndMakeVisible(simpleKnobOutput);
    simpleKnobDrive.setVisible(false);
    simpleKnobTone.setVisible(false);
    simpleKnobOutput.setVisible(false);

    // Startup reveal
    addAndMakeVisible(startupReveal);
    startupReveal.trigger();

    // MIDI learn: right-click on any knob
    auto addRightClick = [this](juce::Slider& s, const juce::String& paramID) {
        s.setPopupMenuEnabled(false);
        s.onDragStart = [this, &s, paramID] { showTooltipForSlider(s); };
        s.onDragEnd = [this] { tooltipOverlay.fadeOut(); };
    };
    addRightClick(kDrive, "drive");
    addRightClick(kMix, "mix");
    addRightClick(kMaster, "master");

    // Timer for meter updates
    startTimerHz(Animation::meterFps);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

// ═══════════════════════════════════════════════════════════════════════════
// Timer — meter and visualizer updates
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::timerCallback() {
    auto& m = proc.getMeterState();
    inPeakL  = m.inputPeakL.load();  inPeakR  = m.inputPeakR.load();
    inRmsL   = m.inputRmsL.load();   inRmsR   = m.inputRmsR.load();
    outPeakL = m.outputPeakL.load(); outPeakR = m.outputPeakR.load();
    outRmsL  = m.outputRmsL.load();  outRmsR  = m.outputRmsR.load();
    sparkGR  = m.sparkGR.load();
    lufs     = m.lufs.load();        lufsShort = m.lufsShort.load();
    lufsInt  = m.lufsIntegrated.load();
    corr     = m.correlation.load();
    inClip   = m.inputClip.load();   outClip  = m.outputClip.load();

    // Update processing indicators
    satIndicator.setActive(proc.getAPVTS().getRawParameterValue("drive")->load() > 0.05f);
    compIndicator.setActive(proc.getAPVTS().getRawParameterValue("glue")->load() > 0.05f);
    limiterIndicator.setActive(sparkGR > 0.5f);

    // Push GR to ribbon
    grRibbon.pushGR(-sparkGR);

    repaint();
}

void BTZAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    showTooltipForSlider(*slider);
}

// ═══════════════════════════════════════════════════════════════════════════
// Setup helpers
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    s.setDoubleClickReturnValue(true, s.getValue());
    addAndMakeVisible(s);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(l);
}

void BTZAudioProcessorEditor::setupSmallKnob(juce::Slider& s, juce::Label& l, const juce::String& text) {
    setupKnob(s, l, text);
}

void BTZAudioProcessorEditor::setupMacroKnob(juce::Slider& s, juce::Label& l, const juce::String& text) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(s);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(Font::macroLabel());
    addAndMakeVisible(l);
}

// ═══════════════════════════════════════════════════════════════════════════
// Paint
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    paintBackground(g);

    auto bounds = getLocalBounds().toFloat();
    auto headerArea = bounds.removeFromTop((float)Geometry::headerHeight);
    paintHeader(g, headerArea);

    g.setColour(hairline);
    g.fillRect(bounds.removeFromTop(1.0f));

    if (viewMode == ViewMode::Simple) {
        paintSimpleMode(g, bounds);
    } else {
        auto meterArea = bounds.removeFromTop((float)Geometry::meterStripHeight);
        paintMeterStrip(g, meterArea);
        g.setColour(hairline);
        g.fillRect(bounds.removeFromTop(1.0f));

        auto content = bounds;
        content.removeFromBottom(24.0f);

        if (currentPage == 0) paintMainPage(g, content);
        else if (currentPage == 1) paintSparkPage(g, content);
        else paintAdvancedPage(g, content);
    }

    // Footer
    auto footerArea = getLocalBounds().toFloat().removeFromBottom(24.0f);
    paintFooter(g, footerArea);

    // Macro knobs (manual paint in Standard Main page)
    if (viewMode == ViewMode::Standard && currentPage == 0) {
        auto drawMacro = [&](juce::Slider& s) {
            if (s.isVisible()) {
                auto b = s.getBounds().toFloat();
                float norm = (float)((s.getValue() - s.getMinimum()) / (s.getMaximum() - s.getMinimum()));
                BTZLookAndFeel::drawMacroKnob(g, b, norm, BTZColours::Module::BTZ, s.isMouseOver());
            }
        };
        drawMacro(kMacro0); drawMacro(kMacro1); drawMacro(kMacro2); drawMacro(kMacro3);
    }
}

void BTZAudioProcessorEditor::paintBackground(juce::Graphics& g) {
    BTZTheme::drawRadialBackground(g, getLocalBounds());
    BTZTheme::drawNoiseTexture(g, getLocalBounds());
}

void BTZAudioProcessorEditor::paintHeader(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(panel.withAlpha(0.8f));
    g.fillRect(area);

    auto brandArea = area.reduced(20.0f, 0.0f);
    g.setFont(Font::title());
    g.setColour(amber);
    g.drawText("BTZ", brandArea.removeFromLeft(60.0f), juce::Justification::centredLeft);
    g.setFont(Font::subtitle());
    g.setColour(mute);
    g.drawText("SONIC ALCHEMY", brandArea.removeFromLeft(140.0f), juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintMeterStrip(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(panel.withAlpha(0.6f));
    g.fillRect(area);

    auto body = area.reduced(20.0f, 6.0f);
    auto dbToPct = [](float db) -> float {
        return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f);
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

    auto statusRow = body.removeFromTop(14.0f);
    g.setFont(Font::meter());
    g.setColour(deepMute);
    g.drawText("M:" + juce::String(lufs, 1) + " LUFS", statusRow.removeFromLeft(100.0f), juce::Justification::centredLeft);
    g.drawText("S:" + juce::String(lufsShort, 1), statusRow.removeFromLeft(70.0f), juce::Justification::centredLeft);
    g.drawText("I:" + juce::String(lufsInt, 1), statusRow.removeFromLeft(70.0f), juce::Justification::centredLeft);
    g.drawText("CORR:" + juce::String(corr, 2), statusRow.removeFromLeft(80.0f), juce::Justification::centredLeft);
    g.setColour(inClip > 0.2f ? stateClip : deepMute);
    g.drawText("IN CLIP", statusRow.removeFromLeft(60.0f), juce::Justification::centredLeft);
    g.setColour(outClip > 0.2f ? stateClip : deepMute);
    g.drawText("OUT CLIP", statusRow.removeFromLeft(70.0f), juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintFooter(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setFont(Font::micro());
    g.setColour(deepMute);
    g.drawText("v1.2  |  " + juce::String(proc.getSampleRate(), 0) + " Hz  |  BTZ Sonic Alchemy  |  BTZ Audio",
               area, juce::Justification::centred);
}

void BTZAudioProcessorEditor::paintSimpleMode(juce::Graphics& g, juce::Rectangle<float> content) {
    // Large centered harmonic visualizer label
    g.setFont(Font::sectionHeader());
    g.setColour(mute);
    g.drawText("HARMONIC SPECTRUM", content.removeFromTop(20.0f).reduced(20.0f, 0.0f),
               juce::Justification::centredLeft);
}

void BTZAudioProcessorEditor::paintMainPage(juce::Graphics& g, juce::Rectangle<float> content) {
    auto topHeader = content.removeFromTop(20.0f);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader.removeFromLeft(content.getWidth() * 0.5f),
                                          "TONE SHAPING", BTZColours::Module::BTZ);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader, "CHARACTER", BTZColours::Module::BTZ);
}

void BTZAudioProcessorEditor::paintSparkPage(juce::Graphics& g, juce::Rectangle<float> content) {
    auto leftPanel = content.removeFromLeft(content.getWidth() * 0.5f);
    auto rightPanel = content;
    BTZLookAndFeel::drawBTZPanelBackground(g, leftPanel.reduced(4.0f));
    BTZLookAndFeel::drawBTZSectionHeader(g, leftPanel.reduced(16.0f, 12.0f).removeFromTop(22.0f),
                                          "SPARK  —  LIMITER", BTZColours::Module::SPARK);
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

    // Resizer
    resizer->setBounds(bounds.getWidth() - 16, bounds.getHeight() - 16, 16, 16);

    // Startup reveal covers everything
    startupReveal.setBounds(bounds);

    // Tooltip overlay covers everything
    tooltipOverlay.setBounds(bounds);

    // Header
    auto header = bounds.removeFromTop(Geometry::headerHeight);

    // View mode buttons (left side of header)
    int vmX = header.getX() + 220;
    int vmY = header.getY() + 10;
    int vmH = header.getHeight() - 20;
    btnSimple.setBounds(vmX, vmY, 60, vmH);
    btnStandard.setBounds(vmX + 64, vmY, 76, vmH);
    btnAdvanced.setBounds(vmX + 144, vmY, 76, vmH);

    // Toolbar (undo/redo, A/B)
    int toolX = vmX + 240;
    int toolH = vmH;
    int toolBtnW = 40;
    btnUndo.setBounds(toolX, vmY, toolBtnW, toolH);
    btnRedo.setBounds(toolX + toolBtnW + 4, vmY, toolBtnW, toolH);
    btnAB.setBounds(toolX + (toolBtnW + 4) * 2, vmY, 30, toolH);
    btnCopyAB.setBounds(toolX + (toolBtnW + 4) * 2 + 34, vmY, 36, toolH);

    // Preset nav (right side)
    int presetRightEdge = header.getRight() - 150;
    int presetW = 200;
    int presetX = presetRightEdge - presetW;
    btnPresetPrev.setBounds(presetX, vmY, 24, toolH);
    lblPresetName.setBounds(presetX + 28, vmY, presetW - 100, toolH);
    btnPresetNext.setBounds(presetX + presetW - 68, vmY, 24, toolH);
    btnPresetSave.setBounds(presetX + presetW - 40, vmY, 40, toolH);

    // Bypass
    btnBypass.setBounds(header.getRight() - 140, vmY, 120, vmH);

    // Processing indicators (small dots near brand)
    satIndicator.setBounds(header.getX() + 80, vmY + 4, 12, 12);
    compIndicator.setBounds(header.getX() + 96, vmY + 4, 12, 12);
    limiterIndicator.setBounds(header.getX() + 112, vmY + 4, 12, 12);

    hideAllControls();

    if (viewMode == ViewMode::Simple) {
        // Hide standard tabs
        tabMain.setVisible(false); tabSpark.setVisible(false); tabDetail.setVisible(false);

        auto content = bounds;
        content.removeFromTop(Geometry::meterStripHeight + 2);
        layoutSimpleMode(content);
    } else {
        // Show tabs within header area
        auto tabArea = header;
        tabArea = tabArea.withX(header.getCentreX() - 170).withWidth(340);
        tabMain.setBounds(tabArea.getX(), vmY, 100, vmH);
        tabSpark.setBounds(tabArea.getX() + 108, vmY, 100, vmH);
        tabDetail.setBounds(tabArea.getX() + 216, vmY, 100, vmH);
        tabMain.setVisible(viewMode == ViewMode::Standard);
        tabSpark.setVisible(viewMode == ViewMode::Standard);
        tabDetail.setVisible(viewMode == ViewMode::Standard);

        bounds.removeFromTop(Geometry::meterStripHeight + 2);
        auto content = bounds.reduced(Geometry::padContent, Space::lg + 10);

        if (viewMode == ViewMode::Standard) {
            if (currentPage == 0) layoutMainPage(content);
            else if (currentPage == 1) layoutSparkPage(content);
            else layoutAdvancedPage(content);
        } else {
            // Advanced mode: show everything
            layoutAdvancedPage(content);
        }
    }

    // Update tab toggle states
    tabMain.setToggleState(currentPage == 0, juce::dontSendNotification);
    tabSpark.setToggleState(currentPage == 1, juce::dontSendNotification);
    tabDetail.setToggleState(currentPage == 2, juce::dontSendNotification);

    if (currentPage == 1) lookAndFeel.currentModule = BTZColours::Module::SPARK;
    else lookAndFeel.currentModule = BTZColours::Module::BTZ;
}

void BTZAudioProcessorEditor::setViewMode(ViewMode mode) {
    viewMode = mode;
    resized();
    repaint();
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
    hide(kResTame, lResTame); hide(kTransSens, lTransSens);
    cGlueScHpf.setVisible(false); lGlueScHpf.setVisible(false);
    cSatModel.setVisible(false); lSatModel.setVisible(false);
    btnMidSide.setVisible(false);
    cMultiband.setVisible(false); lMultiband.setVisible(false);
    cQuality.setVisible(false); lQuality.setVisible(false);
    simpleKnobDrive.setVisible(false);
    simpleKnobTone.setVisible(false);
    simpleKnobOutput.setVisible(false);
    harmonicViz.setVisible(false);
    spectrumDisplay.setVisible(false);
    grRibbon.setVisible(false);
}

void BTZAudioProcessorEditor::layoutSimpleMode(juce::Rectangle<int> content) {
    content.removeFromBottom(30);  // footer

    // Harmonic visualizer takes top half
    auto vizArea = content.removeFromTop(content.getHeight() / 2).reduced(40, 20);
    harmonicViz.setBounds(vizArea);
    harmonicViz.setVisible(true);

    // Three large knobs centered below
    auto knobArea = content.reduced(0, 20);
    int knobSize = Dim::simpleKnobSize;
    int gap = Dim::simpleKnobGap;
    int totalW = knobSize * 3 + gap * 2;
    int startX = knobArea.getCentreX() - totalW / 2;
    int knobY = knobArea.getCentreY() - knobSize / 2;

    simpleKnobDrive.setBounds(startX, knobY, knobSize, knobSize + 30);
    simpleKnobTone.setBounds(startX + knobSize + gap, knobY, knobSize, knobSize + 30);
    simpleKnobOutput.setBounds(startX + (knobSize + gap) * 2, knobY, knobSize, knobSize + 30);
    simpleKnobDrive.setVisible(true);
    simpleKnobTone.setVisible(true);
    simpleKnobOutput.setVisible(true);
}

void BTZAudioProcessorEditor::layoutMainPage(juce::Rectangle<int> content) {
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

    place(kPunch, lPunch, 0, 0); place(kWarmth, lWarmth, 1, 0); place(kBoom, lBoom, 2, 0);
    place(kGlue, lGlue, 3, 0); place(kAir, lAir, 4, 0); place(kWidth, lWidth, 5, 0);
    place(kDensity, lDensity, 0, 1); place(kMotion, lMotion, 1, 1); place(kEra, lEra, 2, 1);
    place(kDrive, lDrive, 3, 1); place(kMix, lMix, 4, 1); place(kMaster, lMaster, 5, 1);

    // Macros
    const int macroY = y2 + knob + label + Space::secLg;
    const int macroKnob = Geometry::knobMacro;
    const int macroGap = (content.getWidth() - macroKnob * 4) / 3;
    auto placeMacro = [&](juce::Slider& s, juce::Label& l, int i) {
        int x = content.getX() + i * (macroKnob + macroGap);
        s.setBounds(x, macroY, macroKnob, macroKnob);
        l.setBounds(x, macroY + macroKnob + 4, macroKnob, label + 4);
        s.setVisible(true); l.setVisible(true);
    };
    placeMacro(kMacro0, lMacro0, 0); placeMacro(kMacro1, lMacro1, 1);
    placeMacro(kMacro2, lMacro2, 2); placeMacro(kMacro3, lMacro3, 3);
}

void BTZAudioProcessorEditor::layoutSparkPage(juce::Rectangle<int> content) {
    auto leftPanel = content.removeFromLeft(content.getWidth() / 2).reduced(Geometry::padPanel, Space::secLg);
    auto rightPanel = content.reduced(Geometry::padPanel, Space::secLg);
    const int knob = Geometry::knobLarge;
    const int smallKnob = Geometry::knobSmall;
    const int label = Geometry::knobLabelHeight;

    int sparkCenterX = leftPanel.getCentreX() - knob / 2;
    int sparkY = leftPanel.getY() + Space::secMd;
    kCeiling.setBounds(sparkCenterX, sparkY, knob, knob);
    lCeiling.setBounds(sparkCenterX, sparkY + knob, knob, label);
    kCeiling.setVisible(true); lCeiling.setVisible(true);

    int intensityY = sparkY + knob + label + Space::xl;
    kIntensity.setBounds(sparkCenterX, intensityY, knob, knob);
    lIntensity.setBounds(sparkCenterX, intensityY + knob, knob, label);
    kIntensity.setVisible(true); lIntensity.setVisible(true);

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
    const int smallKnob = Geometry::knobSmall;
    const int label = Geometry::knobLabelHeight;
    const int y1 = content.getY() + Space::xl;

    // Top row: Drive, Era, Density, Motion
    const int topGap = (content.getWidth() - knob * 4) / 3;
    auto placeTop = [&](juce::Slider& s, juce::Label& l, int col) {
        int x = content.getX() + col * (knob + topGap);
        s.setBounds(x, y1, knob, knob);
        l.setBounds(x, y1 + knob, knob, label);
        s.setVisible(true); l.setVisible(true);
    };
    placeTop(kDrive, lDrive, 0); placeTop(kEra, lEra, 1);
    placeTop(kDensity, lDensity, 2); placeTop(kMotion, lMotion, 3);

    // Middle row: Mix, Master, ResTame, TransSens
    const int y2 = y1 + knob + label + Space::secLg;
    const int midGap = Space::xl;
    int mx = content.getX();
    kMix.setBounds(mx, y2, medKnob, medKnob);
    lMix.setBounds(mx, y2 + medKnob, medKnob, label);
    kMix.setVisible(true); lMix.setVisible(true);
    mx += medKnob + midGap;
    kMaster.setBounds(mx, y2, medKnob, medKnob);
    lMaster.setBounds(mx, y2 + medKnob, medKnob, label);
    kMaster.setVisible(true); lMaster.setVisible(true);
    mx += medKnob + midGap;
    kResTame.setBounds(mx, y2, smallKnob, smallKnob);
    lResTame.setBounds(mx, y2 + smallKnob, smallKnob, label);
    kResTame.setVisible(true); lResTame.setVisible(true);
    mx += smallKnob + midGap;
    kTransSens.setBounds(mx, y2, smallKnob, smallKnob);
    lTransSens.setBounds(mx, y2 + smallKnob, smallKnob, label);
    kTransSens.setVisible(true); lTransSens.setVisible(true);

    // Combos row
    int comboY = y2 + medKnob + label + Space::lg;
    int cx = content.getX();
    lGlueScHpf.setBounds(cx, comboY, 100, 16); lGlueScHpf.setVisible(true);
    cGlueScHpf.setBounds(cx, comboY + 18, 100, 28); cGlueScHpf.setVisible(true);
    cx += 110;
    lSatModel.setBounds(cx, comboY, 120, 16); lSatModel.setVisible(true);
    cSatModel.setBounds(cx, comboY + 18, 120, 28); cSatModel.setVisible(true);
    cx += 130;
    btnMidSide.setBounds(cx, comboY + 18, 70, 28); btnMidSide.setVisible(true);
    cx += 80;
    lMultiband.setBounds(cx, comboY, 100, 16); lMultiband.setVisible(true);
    cMultiband.setBounds(cx, comboY + 18, 100, 28); cMultiband.setVisible(true);
    cx += 110;
    lQuality.setBounds(cx, comboY, 100, 16); lQuality.setVisible(true);
    cQuality.setBounds(cx, comboY + 18, 120, 28); cQuality.setVisible(true);

    // Spectrum + GR ribbon at bottom
    int vizY = comboY + 56;
    int vizH = content.getBottom() - vizY - 10;
    if (vizH > 60) {
        int vizW = content.getWidth();
        spectrumDisplay.setBounds(content.getX(), vizY, (int)(vizW * 0.6f), vizH);
        spectrumDisplay.setVisible(true);
        grRibbon.setBounds(content.getX() + (int)(vizW * 0.62f), vizY, (int)(vizW * 0.38f), vizH);
        grRibbon.setVisible(true);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Tooltip + MIDI Learn
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::showTooltipForSlider(juce::Slider& slider) {
    auto pos = slider.getBounds().getCentre();
    juce::String name = slider.getName().isEmpty() ? "Parameter" : slider.getName();
    juce::String val = juce::String(slider.getValue(), 2);
    tooltipOverlay.showValue(name, val, pos);
}

void BTZAudioProcessorEditor::showMIDILearnMenu(juce::Slider& slider, const juce::String& paramID) {
    juce::PopupMenu menu;
    auto& ml = proc.getMIDILearn();
    menu.addItem(1, "MIDI Learn");
    menu.addItem(2, "Clear MIDI Mapping");
    menu.addSeparator();
    menu.addItem(3, "Reset to Default");
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&slider),
        [this, paramID, &ml](int result) {
            if (result == 1) { ml.startLearning(paramID); }
            else if (result == 2) {
                for (int i = 0; i < ml.numMappings; ++i) {
                    if (ml.mappings[(size_t)i].parameterID == paramID) {
                        ml.removeMapping(ml.mappings[(size_t)i].ccNumber);
                        break;
                    }
                }
            } else if (result == 3) {
                if (auto* param = proc.getAPVTS().getParameter(paramID))
                    param->setValueNotifyingHost(param->getDefaultValue());
            }
        });
}
