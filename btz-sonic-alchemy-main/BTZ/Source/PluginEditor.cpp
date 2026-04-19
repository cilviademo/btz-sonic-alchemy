/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v9
  ────────────────────────────────────────────────────────────────────────
  v9: Resizable UI, preset browser, A/B, undo/redo, saturation model
      selector, mid/side toggle, multiband selector, spectrum area,
      GR history, MIDI learn context menu, EBU R128 LUFS readout.
  v3: 3-page luxury UI, module accents, premium macro knobs.
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

void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float, float, juce::Slider&) {
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

    // Knob body
    const float bodyR = radius * KnobStyle::bodyRadiusRatio;
    juce::ColourGradient bodyGrad(charcoal, cx, cy - bodyR, panel, cx, cy + bodyR, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour(hairline);
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.5f);

    // Indicator line
    g.setColour(accent);
    const float ix1 = cx + radius * KnobStyle::indicatorStart * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + radius * KnobStyle::indicatorStart * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + radius * KnobStyle::indicatorEnd * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + radius * KnobStyle::indicatorEnd * std::sin(angle - juce::MathConstants<float>::halfPi);
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

    if (button.getToggleState()) {
        auto bounds = button.getLocalBounds().toFloat();
        auto accent = amber;
        if (button.getButtonText() == "SPARK") accent = coral;
        else if (button.getButtonText() == "ADVANCED") accent = amber;
        g.setColour(accent);
        g.fillRect(bounds.removeFromBottom(2.0f));
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
                                   int, int, int, int, juce::ComboBox&) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)w, (float)h);
    g.setColour(panel);
    g.fillRoundedRectangle(bounds, 0.0f);
    g.setColour(hairline);
    g.drawRoundedRectangle(bounds, 0.0f, 0.5f);

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

// Premium MacroKnob rendering
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
    const int numTicks = BTZTokens::Dim::tickDotCount;
    for (int i = 0; i < numTicks; ++i) {
        float t = (float)i / (float)(numTicks - 1);
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
    g.strokePath(haloTrack, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    if (normValue > 0.001f) {
        juce::Path haloFill;
        haloFill.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour(accent);
        g.strokePath(haloFill, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // Cream gradient body
    const float bodyR = radius * 0.72f;
    juce::ColourGradient bodyGrad(knobHighlight, cx, cy - bodyR, knobShadow, cx, cy + bodyR, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour(bone.withAlpha(0.3f));
    g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.5f);

    // Indicator
    g.setColour(accent);
    const float ix1 = cx + bodyR * 0.3f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy1 = cy + bodyR * 0.3f * std::sin(angle - juce::MathConstants<float>::halfPi);
    const float ix2 = cx + bodyR * 0.85f * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float iy2 = cy + bodyR * 0.85f * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.drawLine(ix1, iy1, ix2, iy2, 2.5f);

    // Hover glow (subtle)
    if (isHovered) {
        g.setColour(accent.withAlpha(0.08f));
        g.fillEllipse(cx - radius, cy - radius, diameter, diameter);
    }
}

void BTZLookAndFeel::drawBTZPanelBackground(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(charcoal);
    g.fillRoundedRectangle(area, Geometry::panelRadius);
    g.setColour(hairline);
    g.drawRoundedRectangle(area, Geometry::panelRadius, 0.5f);
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
        juce::Colour fillColour;
        if (isGR) {
            fillColour = amber;
        } else if (valuePct > 0.95f) {
            fillColour = meterClip;
        } else if (valuePct > 0.75f) {
            fillColour = meterHot;
        } else if (valuePct > 0.5f) {
            fillColour = meterOptimal;
        } else {
            fillColour = meterSafe;
        }
        g.setColour(fillColour);
        g.fillRoundedRectangle(area.withWidth(fillW), Effects::meterCornerRadius);
    }
}

void BTZLookAndFeel::drawBTZTab(juce::Graphics& g, juce::Rectangle<float> area,
                                 const juce::String& text, bool isActive,
                                 BTZColours::Module module) {
    g.setFont(Font::tab());
    g.setColour(isActive ? accentFor(module) : bone);
    g.drawText(text, area, juce::Justification::centred);
    if (isActive) {
        g.setColour(accentFor(module));
        g.fillRect(area.removeFromBottom(2.0f));
    }
}


// ═══════════════════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════════════════
BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&lookAndFeel);

    // v9: Resizable UI
    constrainer.setFixedAspectRatio((double)Geometry::windowWidth / (double)Geometry::windowHeight);
    constrainer.setMinimumSize(Window::minWidth, Window::minHeight);
    constrainer.setMaximumSize(Geometry::windowWidth * 2, Geometry::windowHeight * 2);
    setConstrainer(&constrainer);
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &constrainer);
    addAndMakeVisible(*resizer);

    setSize(Geometry::windowWidth, Geometry::windowHeight);

    // ── Tab navigation ──
    auto setupTab = [&](juce::TextButton& btn, int page) {
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1001);
        btn.onClick = [this, page] { currentPage = page; resized(); repaint(); };
        addAndMakeVisible(btn);
    };
    setupTab(tabMain, 0);
    setupTab(tabSpark, 1);
    setupTab(tabAdvanced, 2);
    tabMain.setToggleState(true, juce::dontSendNotification);

    // ── Bypass ──
    addAndMakeVisible(btnBypass);
    aBypass = std::make_unique<ButtonAttachment>(proc.getAPVTS(), "bypass", btnBypass);

    // ── v9: Undo/Redo buttons ──
    btnUndo.onClick = [this] { proc.undo(); repaint(); };
    btnRedo.onClick = [this] { proc.redo(); repaint(); };
    addAndMakeVisible(btnUndo);
    addAndMakeVisible(btnRedo);

    // ── v9: A/B buttons ──
    btnAB.onClick = [this] {
        proc.toggleAB();
        btnAB.setButtonText(proc.isSlotA() ? "A" : "B");
        repaint();
    };
    btnCopyAB.onClick = [this] { proc.copyAtoB(); };
    addAndMakeVisible(btnAB);
    addAndMakeVisible(btnCopyAB);

    // ── v9: Preset navigation ──
    lblPresetName.setText(proc.getCurrentPresetName().isEmpty() ? "Init" : proc.getCurrentPresetName(),
                          juce::dontSendNotification);
    lblPresetName.setJustificationType(juce::Justification::centred);
    lblPresetName.setColour(juce::Label::textColourId, cream);
    addAndMakeVisible(lblPresetName);

    btnPresetPrev.onClick = [this] {
        int idx = proc.getCurrentPresetIndex();
        if (idx > 0) proc.setCurrentProgram(idx - 1);
        lblPresetName.setText(proc.getCurrentPresetName().isEmpty() ? "Init" : proc.getCurrentPresetName(),
                              juce::dontSendNotification);
        repaint();
    };
    btnPresetNext.onClick = [this] {
        int idx = proc.getCurrentPresetIndex();
        proc.setCurrentProgram(idx + 1);
        lblPresetName.setText(proc.getCurrentPresetName().isEmpty() ? "Init" : proc.getCurrentPresetName(),
                              juce::dontSendNotification);
        repaint();
    };
    btnPresetSave.onClick = [this] {
        auto dir = proc.getPresetsDirectory();
        juce::FileChooser chooser("Save Preset", dir, "*.btzpreset");
        if (chooser.browseForFileToSave(true)) {
            auto file = chooser.getResult();
            proc.savePreset(file, file.getFileNameWithoutExtension());
            lblPresetName.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
        }
    };
    addAndMakeVisible(btnPresetPrev);
    addAndMakeVisible(btnPresetNext);
    addAndMakeVisible(btnPresetSave);

    // ── Core knobs ──
    setupKnob(kPunch, lPunch, "PUNCH");
    setupKnob(kWarmth, lWarmth, "WARMTH");
    setupKnob(kBoom, lBoom, "BOOM");
    setupKnob(kGlue, lGlue, "GLUE");
    setupKnob(kAir, lAir, "AIR");
    setupKnob(kWidth, lWidth, "WIDTH");
    setupKnob(kDensity, lDensity, "DENSITY");
    setupKnob(kMotion, lMotion, "MOTION");
    setupKnob(kEra, lEra, "ERA");
    setupKnob(kDrive, lDrive, "DRIVE");
    setupKnob(kMix, lMix, "MIX");
    setupKnob(kMaster, lMaster, "MASTER");

    // ── Macro knobs ──
    setupMacroKnob(kMacro0, lMacro0, "MACRO A");
    setupMacroKnob(kMacro1, lMacro1, "MACRO B");
    setupMacroKnob(kMacro2, lMacro2, "MACRO C");
    setupMacroKnob(kMacro3, lMacro3, "MACRO D");

    // ── SPARK/SHINE knobs ──
    setupKnob(kCeiling, lCeiling, "CEILING");
    setupKnob(kIntensity, lIntensity, "INTENSITY");
    setupKnob(kShine, lShine, "SHINE");
    setupKnob(kShineMix, lShineMix, "SHINE MIX");
    setupSmallKnob(kShineFreq, lShineFreq, "FREQ");
    setupSmallKnob(kShineQ, lShineQ, "Q");

    // ── Glue SC HPF combo ──
    lGlueScHpf.setText("SC HPF", juce::dontSendNotification);
    lGlueScHpf.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lGlueScHpf);
    addAndMakeVisible(cGlueScHpf);

    // ── v9: Saturation model combo ──
    lSatModel.setText("SAT MODEL", juce::dontSendNotification);
    lSatModel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lSatModel);
    addAndMakeVisible(cSatModel);

    // ── v9: Mid/Side toggle ──
    addAndMakeVisible(btnMidSide);

    // ── v9: Multiband combo ──
    lMultiband.setText("MULTIBAND", juce::dontSendNotification);
    lMultiband.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lMultiband);
    addAndMakeVisible(cMultiband);

    // ── Attachments ──
    auto& apvts = proc.getAPVTS();
    aPunch   = std::make_unique<SliderAttachment>(apvts, "punch",    kPunch);
    aWarmth  = std::make_unique<SliderAttachment>(apvts, "warmth",   kWarmth);
    aBoom    = std::make_unique<SliderAttachment>(apvts, "boom",     kBoom);
    aGlue    = std::make_unique<SliderAttachment>(apvts, "glue",     kGlue);
    aAir     = std::make_unique<SliderAttachment>(apvts, "air",      kAir);
    aWidth   = std::make_unique<SliderAttachment>(apvts, "width",    kWidth);
    aDensity = std::make_unique<SliderAttachment>(apvts, "density",  kDensity);
    aMotion  = std::make_unique<SliderAttachment>(apvts, "motion",   kMotion);
    aEra     = std::make_unique<SliderAttachment>(apvts, "era",      kEra);
    aDrive   = std::make_unique<SliderAttachment>(apvts, "drive",    kDrive);
    aMix     = std::make_unique<SliderAttachment>(apvts, "mix",      kMix);
    aMaster  = std::make_unique<SliderAttachment>(apvts, "master",   kMaster);
    aCeiling = std::make_unique<SliderAttachment>(apvts, "ceiling",  kCeiling);
    aShine   = std::make_unique<SliderAttachment>(apvts, "shine",    kShine);
    aShineMix= std::make_unique<SliderAttachment>(apvts, "shineMix", kShineMix);
    aIntensity = std::make_unique<SliderAttachment>(apvts, "intensity", kIntensity);
    aShineFreq = std::make_unique<SliderAttachment>(apvts, "shineFreq", kShineFreq);
    aShineQ    = std::make_unique<SliderAttachment>(apvts, "shineQ",    kShineQ);
    aMacro0  = std::make_unique<SliderAttachment>(apvts, "macro0",   kMacro0);
    aMacro1  = std::make_unique<SliderAttachment>(apvts, "macro1",   kMacro1);
    aMacro2  = std::make_unique<SliderAttachment>(apvts, "macro2",   kMacro2);
    aMacro3  = std::make_unique<SliderAttachment>(apvts, "macro3",   kMacro3);
    aBypass  = std::make_unique<ButtonAttachment>(apvts, "bypass",   btnBypass);
    aGlueScHpf = std::make_unique<ComboAttachment>(apvts, "glueScHpf", cGlueScHpf);
    aSatModel  = std::make_unique<ComboAttachment>(apvts, "satModel",  cSatModel);
    aMidSide   = std::make_unique<ButtonAttachment>(apvts, "midSide",  btnMidSide);
    aMultiband = std::make_unique<ComboAttachment>(apvts, "multibandCount", cMultiband);

    startTimerHz(30);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    setLookAndFeel(nullptr);
    stopTimer();
}

void BTZAudioProcessorEditor::timerCallback() {
    auto& m = proc.getMeters();
    inPeakL  = m.inputPeakL.load();   inPeakR  = m.inputPeakR.load();
    inRmsL   = m.inputRmsL.load();    inRmsR   = m.inputRmsR.load();
    outPeakL = m.outputPeakL.load();  outPeakR = m.outputPeakR.load();
    outRmsL  = m.outputRmsL.load();   outRmsR  = m.outputRmsR.load();
    sparkGR  = m.sparkGainReductionDb.load();
    lufs     = m.lufs.load();
    lufsShort = m.lufsShortTerm.load();
    lufsInt  = m.lufsIntegrated.load();
    corr     = m.correlation.load();
    inClip   = m.inputClip.load();
    outClip  = m.outputClip.load();
    repaint();
}

void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
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
    g.fillAll(obsidian);

    auto bounds = getLocalBounds().toFloat();

    // Header
    auto headerArea = bounds.removeFromTop((float)Geometry::headerHeight);
    paintHeader(g, headerArea);

    // Hairline
    g.setColour(hairline);
    g.fillRect(bounds.removeFromTop(1.0f));

    // Meter strip
    auto meterArea = bounds.removeFromTop((float)Geometry::meterStripHeight);
    paintMeterStrip(g, meterArea);

    // Hairline
    g.setColour(hairline);
    g.fillRect(bounds.removeFromTop(1.0f));

    // Content
    auto content = bounds;
    content.removeFromBottom(24.0f);  // Reserve footer

    if (currentPage == 0)
        paintMainPage(g, content);
    else if (currentPage == 1)
        paintSparkPage(g, content);
    else
        paintAdvancedPage(g, content);

    // Footer
    auto footerArea = getLocalBounds().toFloat().removeFromBottom(24.0f);
    paintFooter(g, footerArea);

    // Paint macro knobs manually
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

    auto brandArea = area.reduced(20.0f, 0.0f);
    g.setFont(Font::title());
    g.setColour(amber);
    g.drawText("BTZ", brandArea.removeFromLeft(60.0f), juce::Justification::centredLeft);

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

    // Status row with LUFS
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
    g.drawText("v1.1  |  " + juce::String(proc.getSampleRate(), 0) + " Hz  |  BTZ Sonic Alchemy  |  BTZ Audio",
               area, juce::Justification::centred);
}

void BTZAudioProcessorEditor::paintMainPage(juce::Graphics& g, juce::Rectangle<float> content) {
    auto topHeader = content.removeFromTop(20.0f);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader.removeFromLeft(content.getWidth() * 0.5f),
                                          "TONE SHAPING", BTZColours::Module::BTZ);
    BTZLookAndFeel::drawBTZSectionHeader(g, topHeader,
                                          "CHARACTER", BTZColours::Module::BTZ);
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

    // v9: Spectrum analyzer area
    auto specArea = content.removeFromBottom(content.getHeight() * 0.3f);
    paintSpectrumArea(g, specArea.removeFromLeft(specArea.getWidth() * 0.6f));
    paintGRHistory(g, specArea);
}

void BTZAudioProcessorEditor::paintSpectrumArea(juce::Graphics& g, juce::Rectangle<float> area) {
    area = area.reduced(8.0f);
    g.setColour(juce::Colour(0xFF0D0E12));
    g.fillRoundedRectangle(area, 2.0f);
    g.setColour(hairline);
    g.drawRoundedRectangle(area, 2.0f, 0.5f);

    g.setFont(Font::meter());
    g.setColour(deepMute);
    g.drawText("SPECTRUM", area.reduced(8.0f, 4.0f), juce::Justification::topLeft);

    // Draw frequency grid lines
    const float w = area.getWidth();
    const float h = area.getHeight();
    g.setColour(rule.withAlpha(0.3f));
    for (float freq : { 100.0f, 1000.0f, 10000.0f }) {
        float x = area.getX() + w * std::log10(freq / 20.0f) / std::log10(20000.0f / 20.0f);
        g.drawVerticalLine((int)x, area.getY(), area.getBottom());
    }
    for (float db : { -48.0f, -24.0f, 0.0f }) {
        float y = area.getBottom() - h * (db + 60.0f) / 66.0f;
        g.drawHorizontalLine((int)y, area.getX(), area.getRight());
    }
}

void BTZAudioProcessorEditor::paintGRHistory(juce::Graphics& g, juce::Rectangle<float> area) {
    area = area.reduced(8.0f);
    g.setColour(juce::Colour(0xFF0D0E12));
    g.fillRoundedRectangle(area, 2.0f);
    g.setColour(hairline);
    g.drawRoundedRectangle(area, 2.0f, 0.5f);

    g.setFont(Font::meter());
    g.setColour(deepMute);
    g.drawText("GR HISTORY", area.reduced(8.0f, 4.0f), juce::Justification::topLeft);

    // Draw GR history from ring buffer
    auto& grHist = proc.getGRHistory();
    const int histSize = BTZDsp::GainReductionHistory::kHistorySize;
    const int wp = grHist.writePos.load(std::memory_order_relaxed);
    const float w = area.getWidth();
    const float h = area.getHeight();

    juce::Path grPath;
    bool started = false;
    for (int i = 0; i < histSize; ++i) {
        int idx = (wp + i) % histSize;
        float grDb = grHist.history[(size_t)idx];
        float x = area.getX() + (float)i / (float)histSize * w;
        float y = area.getY() + h * 0.1f + juce::jlimit(0.0f, h * 0.8f, std::abs(grDb) / 18.0f * h * 0.8f);
        if (!started) { grPath.startNewSubPath(x, y); started = true; }
        else grPath.lineTo(x, y);
    }
    g.setColour(amber.withAlpha(0.6f));
    g.strokePath(grPath, juce::PathStrokeType(1.0f));
}


// ═══════════════════════════════════════════════════════════════════════════
// Resized — Layout
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    // v9: Resizer in bottom-right corner
    resizer->setBounds(bounds.getWidth() - 16, bounds.getHeight() - 16, 16, 16);

    // Header: tabs + bypass + toolbar
    auto header = bounds.removeFromTop(Geometry::headerHeight);
    auto tabArea = header.reduced(280, 10);
    const int tabW = Geometry::tabWidth;
    const int gap = Geometry::tabGap;
    const int startX = tabArea.getCentreX() - (tabW * 3 + gap * 2) / 2;
    tabMain.setBounds(startX, tabArea.getY(), tabW, Geometry::tabHeight);
    tabSpark.setBounds(startX + tabW + gap, tabArea.getY(), tabW, Geometry::tabHeight);
    tabAdvanced.setBounds(startX + (tabW + gap) * 2, tabArea.getY(), tabW, Geometry::tabHeight);
    btnBypass.setBounds(header.getRight() - 140, header.getY() + 10, 120, header.getHeight() - 20);

    // v9: Toolbar (undo/redo, A/B, preset nav) in header left
    int toolX = header.getX() + 220;
    int toolY = header.getY() + 12;
    int toolH = header.getHeight() - 24;
    int toolBtnW = 40;
    btnUndo.setBounds(toolX, toolY, toolBtnW, toolH);
    btnRedo.setBounds(toolX + toolBtnW + 4, toolY, toolBtnW, toolH);
    btnAB.setBounds(toolX + (toolBtnW + 4) * 2, toolY, 30, toolH);
    btnCopyAB.setBounds(toolX + (toolBtnW + 4) * 2 + 34, toolY, 36, toolH);

    // Preset nav in header right (before bypass)
    int presetRightEdge = header.getRight() - 150;
    int presetW = 200;
    int presetX = presetRightEdge - presetW;
    btnPresetPrev.setBounds(presetX, toolY, 24, toolH);
    lblPresetName.setBounds(presetX + 28, toolY, presetW - 100, toolH);
    btnPresetNext.setBounds(presetX + presetW - 68, toolY, 24, toolH);
    btnPresetSave.setBounds(presetX + presetW - 40, toolY, 40, toolH);

    tabMain.setToggleState(currentPage == 0, juce::dontSendNotification);
    tabSpark.setToggleState(currentPage == 1, juce::dontSendNotification);
    tabAdvanced.setToggleState(currentPage == 2, juce::dontSendNotification);

    if (currentPage == 1)
        lookAndFeel.currentModule = BTZColours::Module::SPARK;
    else
        lookAndFeel.currentModule = BTZColours::Module::BTZ;

    bounds.removeFromTop(Geometry::meterStripHeight + 2);

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
    cSatModel.setVisible(false);
    lSatModel.setVisible(false);
    btnMidSide.setVisible(false);
    cMultiband.setVisible(false);
    lMultiband.setVisible(false);
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

    // Row 1: Punch, Warmth, Boom, Glue, Air, Width
    place(kPunch, lPunch, 0, 0); place(kWarmth, lWarmth, 1, 0); place(kBoom, lBoom, 2, 0);
    place(kGlue, lGlue, 3, 0); place(kAir, lAir, 4, 0); place(kWidth, lWidth, 5, 0);

    // Row 2: Density, Motion, Era, Drive, Mix, Master
    place(kDensity, lDensity, 0, 1); place(kMotion, lMotion, 1, 1); place(kEra, lEra, 2, 1);
    place(kDrive, lDrive, 3, 1); place(kMix, lMix, 4, 1); place(kMaster, lMaster, 5, 1);

    // Macro row
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

    // SPARK: Ceiling + Intensity
    int sparkCenterX = leftPanel.getCentreX() - knob / 2;
    int sparkY = leftPanel.getY() + Space::secMd;
    kCeiling.setBounds(sparkCenterX, sparkY, knob, knob);
    lCeiling.setBounds(sparkCenterX, sparkY + knob, knob, label);
    kCeiling.setVisible(true); lCeiling.setVisible(true);

    int intensityY = sparkY + knob + label + Space::xl;
    kIntensity.setBounds(sparkCenterX, intensityY, knob, knob);
    lIntensity.setBounds(sparkCenterX, intensityY + knob, knob, label);
    kIntensity.setVisible(true); lIntensity.setVisible(true);

    // SHINE: Amount + Mix (top), Freq + Q (bottom)
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

    // Middle row: Mix + Master + SC HPF + Sat Model + M/S + Multiband
    const int y2 = y1 + knob + label + Space::secLg;
    const int botGap = Space::xl;

    kMix.setBounds(content.getX(), y2, medKnob, medKnob);
    lMix.setBounds(content.getX(), y2 + medKnob, medKnob, label);
    kMix.setVisible(true); lMix.setVisible(true);

    kMaster.setBounds(content.getX() + medKnob + botGap, y2, medKnob, medKnob);
    lMaster.setBounds(content.getX() + medKnob + botGap, y2 + medKnob, medKnob, label);
    kMaster.setVisible(true); lMaster.setVisible(true);

    // SC HPF combo
    int comboX = content.getX() + medKnob * 2 + botGap * 2;
    int comboY = y2 + (medKnob - 28) / 2;
    lGlueScHpf.setBounds(comboX, comboY - 18, 120, 16);
    lGlueScHpf.setVisible(true);
    cGlueScHpf.setBounds(comboX, comboY, 120, 28);
    cGlueScHpf.setVisible(true);

    // v9: Saturation model combo
    int satX = comboX + 130;
    lSatModel.setBounds(satX, comboY - 18, 120, 16);
    lSatModel.setVisible(true);
    cSatModel.setBounds(satX, comboY, 120, 28);
    cSatModel.setVisible(true);

    // v9: Mid/Side toggle
    int msX = satX + 130;
    btnMidSide.setBounds(msX, comboY, 80, 28);
    btnMidSide.setVisible(true);

    // v9: Multiband combo
    int mbX = msX + 90;
    lMultiband.setBounds(mbX, comboY - 18, 120, 16);
    lMultiband.setVisible(true);
    cMultiband.setBounds(mbX, comboY, 120, 28);
    cMultiband.setVisible(true);
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: MIDI Learn context menu
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessorEditor::showMIDILearnMenu(juce::Slider& slider, const juce::String& paramID) {
    juce::PopupMenu menu;
    auto& ml = proc.getMIDILearn();

    menu.addItem(1, "MIDI Learn");
    menu.addItem(2, "Clear MIDI Mapping");
    menu.addSeparator();
    menu.addItem(3, "Reset to Default");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&slider),
        [this, paramID, &ml](int result) {
            if (result == 1) {
                ml.startLearning(paramID);
            } else if (result == 2) {
                // Find and remove mapping for this param
                for (int i = 0; i < ml.numMappings; ++i) {
                    if (ml.mappings[(size_t)i].parameterID == paramID) {
                        ml.removeMapping(ml.mappings[(size_t)i].ccNumber);
                        break;
                    }
                }
            } else if (result == 3) {
                if (auto* param = proc.getAPVTS().getParameter(paramID)) {
                    param->setValueNotifyingHost(param->getDefaultValue());
                }
            }
        });
}
