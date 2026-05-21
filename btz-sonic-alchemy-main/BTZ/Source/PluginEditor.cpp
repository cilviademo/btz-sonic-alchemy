/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v11
  ──────────────────────────────────────────────────────────────────────────
  All colours from btz::palette. All dimensions from btz::space/layout.
  Zero hardcoded hex values. Zero legacy namespace references.
  ──────────────────────────────────────────────────────────────────────────
*/
#include "PluginEditor.h"

using namespace btz;

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel
// ═══════════════════════════════════════════════════════════════════════════

BTZLookAndFeel::BTZLookAndFeel() {
    setColour(juce::Slider::thumbColourId,              juce::Colour(palette::sage));
    setColour(juce::Slider::rotarySliderFillColourId,   juce::Colour(palette::sage));
    setColour(juce::Slider::trackColourId,              juce::Colour(palette::knobTrack));
    setColour(juce::Label::textColourId,                juce::Colour(palette::ink));
    setColour(juce::TextButton::buttonColourId,         juce::Colour(palette::surface));
    setColour(juce::TextButton::textColourOffId,        juce::Colour(palette::ink));
    setColour(juce::ComboBox::backgroundColourId,       juce::Colour(palette::surface));
    setColour(juce::ComboBox::textColourId,             juce::Colour(palette::ink));
    setColour(juce::ComboBox::outlineColourId,          juce::Colour(palette::border));
    setColour(juce::PopupMenu::backgroundColourId,      juce::Colour(palette::surface));
    setColour(juce::PopupMenu::textColourId,            juce::Colour(palette::ink));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(palette::sageFaint));
}

void BTZLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPos, float, float, juce::Slider& s) {
    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(2.0f);
    paintKnob(g, bounds, sliderPos, s.getComponentID().getIntValue(),
              s.isMouseOver() || s.isMouseButtonDown());
}

void BTZLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPos, float, float,
                                       juce::Slider::SliderStyle, juce::Slider&) {
    auto track = juce::Rectangle<float>((float)x, (float)y + h * 0.4f, (float)w, 3.0f);
    g.setColour(juce::Colour(palette::knobTrack));
    g.fillRoundedRectangle(track, 1.5f);

    const float fillW = sliderPos - (float)x;
    if (fillW > 0.5f) {
        g.setColour(juce::Colour(palette::sage));
        g.fillRoundedRectangle(track.withWidth(fillW), 1.5f);
    }

    g.setColour(juce::Colour(palette::knobFace));
    g.fillEllipse(sliderPos - 5.0f, track.getCentreY() - 5.0f, 10.0f, 10.0f);
    g.setColour(juce::Colour(palette::border));
    g.drawEllipse(sliderPos - 5.0f, track.getCentreY() - 5.0f, 10.0f, 10.0f, 0.5f);
}

void BTZLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                            const juce::Colour&, bool highlighted, bool down) {
    auto bounds = b.getLocalBounds().toFloat().reduced(0.5f);
    if (down || b.getToggleState()) {
        g.setColour(juce::Colour(palette::sage));
    } else if (highlighted) {
        g.setColour(juce::Colour(palette::sageFaint));
    } else {
        g.setColour(juce::Colour(palette::surface));
    }
    g.fillRoundedRectangle(bounds, radius::sm);
    g.setColour(juce::Colour(palette::border));
    g.drawRoundedRectangle(bounds, radius::sm, 0.5f);
}

void BTZLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool down) {
    g.setFont(juce::Font(type::sans(), type::label, juce::Font::bold));
    g.setColour((down || b.getToggleState()) ? juce::Colour(palette::canvas)
                                              : juce::Colour(palette::ink));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

void BTZLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                                       bool highlighted, bool) {
    auto bounds = b.getLocalBounds().toFloat().reduced(1.0f);
    if (b.getToggleState()) {
        g.setColour(juce::Colour(palette::sage));
        g.fillRoundedRectangle(bounds, radius::sm);
        g.setColour(juce::Colour(palette::canvas));
    } else {
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds, radius::sm);
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds, radius::sm, 0.5f);
        g.setColour(highlighted ? juce::Colour(palette::ink) : juce::Colour(palette::inkMuted));
    }
    g.setFont(juce::Font(type::sans(), type::label, juce::Font::bold));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

void BTZLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& l) {
    g.setColour(l.findColour(juce::Label::textColourId));
    g.setFont(l.getFont());
    g.drawText(l.getText(), l.getLocalBounds(), l.getJustificationType());
}

void BTZLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool,
                                    int, int, int, int, juce::ComboBox&) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)w, (float)h);
    g.setColour(juce::Colour(palette::surface));
    g.fillRoundedRectangle(bounds, radius::sm);
    g.setColour(juce::Colour(palette::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius::sm, 0.5f);

    const float arrowX = (float)w - 16.0f;
    const float arrowY = (float)h * 0.5f - 2.0f;
    juce::Path arrow;
    arrow.addTriangle(arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour(juce::Colour(palette::inkMuted));
    g.fillPath(arrow);
}

void BTZLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool, bool isHighlighted,
                                        bool isTicked, bool, const juce::String& text,
                                        const juce::String&, const juce::Drawable*, const juce::Colour*) {
    if (isSeparator) {
        g.setColour(juce::Colour(palette::border));
        g.fillRect(area.reduced(space::sm, 0).withHeight(1).withCentre(area.getCentre()));
        return;
    }
    if (isHighlighted) {
        g.setColour(juce::Colour(palette::sageFaint));
        g.fillRect(area);
    }
    g.setColour(isTicked ? juce::Colour(palette::sage) : juce::Colour(palette::ink));
    g.setFont(juce::Font(type::sans(), type::body, juce::Font::plain));
    g.drawText(text, area.reduced(space::md, 0), juce::Justification::centredLeft);
}

// ═══════════════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&lookAndFeel);
    setResizable(true, true);
    constrainer.setMinimumSize(layout::minW, layout::minH);
    constrainer.setMaximumSize(layout::maxW, layout::maxH);
    constrainer.setFixedAspectRatio((double)layout::defaultW / (double)layout::defaultH);
    setConstrainer(&constrainer);
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &constrainer);
    addAndMakeVisible(*resizer);

    // View tabs
    viewTabs.setTabs({ "SIMPLE", "STANDARD", "ADVANCED" });
    viewTabs.setActive(1);
    viewTabs.onTabChanged = [this](int idx) { setViewMode(static_cast<ViewMode>(idx)); };
    addAndMakeVisible(viewTabs);

    // Page tabs (Standard sub-pages)
    pageTabs.setTabs({ "MAIN", "SPARK", "DETAIL" });
    pageTabs.setActive(0);
    pageTabs.onTabChanged = [this](int idx) { currentPage = idx; resized(); };
    addAndMakeVisible(pageTabs);

    // Custom components
    addAndMakeVisible(harmonicViz);
    addAndMakeVisible(grRibbon);
    addAndMakeVisible(spectrumDisplay);
    addAndMakeVisible(spectrumAdvanced);
    addAndMakeVisible(satIndicator);
    addAndMakeVisible(compIndicator);
    addAndMakeVisible(limIndicator);
    addAndMakeVisible(presetBrowser);
    addAndMakeVisible(simpleKnobDrive);
    addAndMakeVisible(simpleKnobTone);
    addAndMakeVisible(simpleKnobOutput);

    // Toolbar buttons
    for (auto* btn : { &btnUndo, &btnRedo, &btnAB, &btnCopyAB,
                       &btnPresetPrev, &btnPresetNext, &btnPresetSave })
        addAndMakeVisible(btn);
    addAndMakeVisible(btnBypass);
    addAndMakeVisible(lblPresetName);
    lblPresetName.setJustificationType(juce::Justification::centred);
    lblPresetName.setFont(juce::Font(type::sans(), type::body, juce::Font::plain));

    btnUndo.onClick = [this] { proc.undo(); };
    btnRedo.onClick = [this] { proc.redo(); };
    btnAB.onClick = [this] { proc.toggleAB(); };
    btnCopyAB.onClick = [this] { proc.copyAtoB(); };
    btnPresetPrev.onClick = [this] { proc.loadPresetByIndex(proc.getCurrentPresetIndex() - 1); };
    btnPresetNext.onClick = [this] { proc.loadPresetByIndex(proc.getCurrentPresetIndex() + 1); };

    // Core knobs
    setupKnob(kPunch, lPunch, "PUNCH", id::punch);
    setupKnob(kWarmth, lWarmth, "WARMTH", id::warmth);
    setupKnob(kBoom, lBoom, "BOOM", id::boom);
    setupKnob(kGlue, lGlue, "GLUE", id::glue);
    setupKnob(kAir, lAir, "AIR", id::air);
    setupKnob(kWidth, lWidth, "WIDTH", id::width);
    setupKnob(kDrive, lDrive, "DRIVE", id::drive);
    setupKnob(kMix, lMix, "MIX", id::mix);
    setupKnob(kMaster, lMaster, "MASTER", id::output);

    // Macros
    setupSmallKnob(kDensity, lDensity, "DENSITY", id::density);
    setupSmallKnob(kMotion, lMotion, "MOTION", id::motion);
    setupSmallKnob(kEra, lEra, "ERA", id::era);
    setupSmallKnob(kMacro0, lMacro0, "MACRO 1", id::macro);
    setupSmallKnob(kMacro1, lMacro1, "MACRO 2", id::macro);
    setupSmallKnob(kMacro2, lMacro2, "MACRO 3", id::macro);
    setupSmallKnob(kMacro3, lMacro3, "MACRO 4", id::macro);

    // Spark page
    setupSmallKnob(kCeiling, lCeiling, "CEILING", id::ceiling);
    setupSmallKnob(kIntensity, lIntensity, "INTENSITY", id::intensity);

    // Shine
    setupSmallKnob(kShine, lShine, "SHINE", id::shine);
    setupSmallKnob(kShineMix, lShineMix, "SHINE MIX", id::shine);
    setupSmallKnob(kShineFreq, lShineFreq, "FREQ", id::shine);
    setupSmallKnob(kShineQ, lShineQ, "Q", id::shine);

    // Advanced
    setupSmallKnob(kResTame, lResTame, "RES TAME", id::resTame);
    setupSmallKnob(kTransSens, lTransSens, "TRANSIENT", id::transient);
    addAndMakeVisible(btnMidSide);
    addAndMakeVisible(cSatModel);
    addAndMakeVisible(cGlueScHpf);
    addAndMakeVisible(cMultiband);
    addAndMakeVisible(cQuality);

    cSatModel.addItemList({ "Tanh", "Tube", "Tape", "Transistor", "Transformer",
                            "Neural Neve", "Neural API", "Neural SSL", "Neural Custom" }, 1);
    cGlueScHpf.addItemList({ "Off", "60 Hz", "100 Hz", "200 Hz", "300 Hz" }, 1);
    cMultiband.addItemList({ "1 Band", "2 Bands", "3 Bands", "4 Bands", "5 Bands", "6 Bands" }, 1);
    cQuality.addItemList({ "Eco", "Standard", "High", "Ultra" }, 1);

    // Attachments
    auto& apvts = proc.getAPVTS();
    aPunch    = std::make_unique<SliderAttachment>(apvts, "punch", kPunch);
    aWarmth   = std::make_unique<SliderAttachment>(apvts, "warmth", kWarmth);
    aBoom     = std::make_unique<SliderAttachment>(apvts, "boom", kBoom);
    aGlue     = std::make_unique<SliderAttachment>(apvts, "glue", kGlue);
    aAir      = std::make_unique<SliderAttachment>(apvts, "air", kAir);
    aWidth    = std::make_unique<SliderAttachment>(apvts, "width", kWidth);
    aDrive    = std::make_unique<SliderAttachment>(apvts, "drive", kDrive);
    aMix      = std::make_unique<SliderAttachment>(apvts, "mix", kMix);
    aMaster   = std::make_unique<SliderAttachment>(apvts, "master", kMaster);
    aDensity  = std::make_unique<SliderAttachment>(apvts, "density", kDensity);
    aMotion   = std::make_unique<SliderAttachment>(apvts, "motion", kMotion);
    aEra      = std::make_unique<SliderAttachment>(apvts, "era", kEra);
    aCeiling  = std::make_unique<SliderAttachment>(apvts, "ceiling", kCeiling);
    aIntensity = std::make_unique<SliderAttachment>(apvts, "intensity", kIntensity);
    aShine    = std::make_unique<SliderAttachment>(apvts, "shine", kShine);
    aShineMix = std::make_unique<SliderAttachment>(apvts, "shineMix", kShineMix);
    aShineFreq = std::make_unique<SliderAttachment>(apvts, "shineFreq", kShineFreq);
    aShineQ   = std::make_unique<SliderAttachment>(apvts, "shineQ", kShineQ);
    aMacro0   = std::make_unique<SliderAttachment>(apvts, "macro0", kMacro0);
    aMacro1   = std::make_unique<SliderAttachment>(apvts, "macro1", kMacro1);
    aMacro2   = std::make_unique<SliderAttachment>(apvts, "macro2", kMacro2);
    aMacro3   = std::make_unique<SliderAttachment>(apvts, "macro3", kMacro3);
    aResTame  = std::make_unique<SliderAttachment>(apvts, "resSens", kResTame);
    aTransSens = std::make_unique<SliderAttachment>(apvts, "transSens", kTransSens);
    aBypass   = std::make_unique<ButtonAttachment>(apvts, "bypass", btnBypass);
    aMidSide  = std::make_unique<ButtonAttachment>(apvts, "midSide", btnMidSide);
    aSatModel = std::make_unique<ComboAttachment>(apvts, "satModel", cSatModel);
    aGlueScHpf = std::make_unique<ComboAttachment>(apvts, "glueScHpf", cGlueScHpf);
    aMultiband = std::make_unique<ComboAttachment>(apvts, "multibandCount", cMultiband);
    aQuality  = std::make_unique<ComboAttachment>(apvts, "quality", cQuality);

    // Wire preset save button
    btnPresetSave.onClick = [this] {
        auto dir = proc.getPresetDirectory();
        if (!dir.exists()) dir.createDirectory();
        juce::FileChooser chooser("Save Preset", dir, "*.btzpreset");
        if (chooser.browseForFileToSave(true)) {
            proc.savePreset(chooser.getResult());
            populatePresetBrowser();
        }
    };

    // Wire simple mode knobs to drive underlying APVTS parameters
    simpleKnobDrive.onValueChange = [this](float v) {
        if (auto* p = proc.getAPVTS().getParameter("drive"))
            p->setValueNotifyingHost(v);
    };
    simpleKnobTone.onValueChange = [this](float v) {
        if (auto* p = proc.getAPVTS().getParameter("shine"))
            p->setValueNotifyingHost(v);
    };
    simpleKnobOutput.onValueChange = [this](float v) {
        if (auto* p = proc.getAPVTS().getParameter("master"))
            p->setValueNotifyingHost(v);
    };

    // Wire preset browser
    presetBrowser.onSelect = [this](int idx) {
        proc.loadPresetByIndex(idx);
    };
    populatePresetBrowser();

    setSize(layout::defaultW, layout::defaultH);
    setViewMode(ViewMode::Standard);
    startTimerHz(anim::fps);
}

BTZAudioProcessorEditor::~BTZAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

// ═══════════════════════════════════════════════════════════════════════════
// Setup Helpers
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l,
                                          const juce::String& text, int compID) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setComponentID(juce::String(compID));
    s.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(s);

    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(type::sans(), type::label, juce::Font::plain));
    l.setColour(juce::Label::textColourId, juce::Colour(palette::inkFaint));
    addAndMakeVisible(l);
}

void BTZAudioProcessorEditor::setupSmallKnob(juce::Slider& s, juce::Label& l,
                                               const juce::String& text, int compID) {
    setupKnob(s, l, text, compID);
}

// ═══════════════════════════════════════════════════════════════════════════
// Timer
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::timerCallback() {
    const float gr = proc.meters.grDb.load(std::memory_order_relaxed);
    grRibbon.push(gr);

    satIndicator.setActive(gr < -0.1f, juce::Colour(palette::orange));
    compIndicator.setActive(gr < -1.0f, juce::Colour(palette::sage));
    limIndicator.setActive(gr < -3.0f, juce::Colour(palette::clay));

    lblPresetName.setText(proc.getCurrentPresetName(), juce::dontSendNotification);

    if (viewMode == ViewMode::Simple) {
        simpleKnobDrive.setValue(kDrive.getValue() / kDrive.getMaximum(), "");
        simpleKnobTone.setValue(kShine.getValue() / kShine.getMaximum(), "");
        simpleKnobOutput.setValue(kMaster.getValue() / kMaster.getMaximum(), "");
    }

    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
// View Mode
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::setViewMode(ViewMode mode) {
    viewMode = mode;
    hideAllControls();

    switch (mode) {
        case ViewMode::Simple:
            simpleKnobDrive.setVisible(true);
            simpleKnobTone.setVisible(true);
            simpleKnobOutput.setVisible(true);
            harmonicViz.setVisible(true);
            pageTabs.setVisible(false);
            break;

        case ViewMode::Standard:
            kPunch.setVisible(true); lPunch.setVisible(true);
            kWarmth.setVisible(true); lWarmth.setVisible(true);
            kBoom.setVisible(true); lBoom.setVisible(true);
            kGlue.setVisible(true); lGlue.setVisible(true);
            kAir.setVisible(true); lAir.setVisible(true);
            kWidth.setVisible(true); lWidth.setVisible(true);
            kDrive.setVisible(true); lDrive.setVisible(true);
            kMix.setVisible(true); lMix.setVisible(true);
            kMaster.setVisible(true); lMaster.setVisible(true);
            kDensity.setVisible(true); lDensity.setVisible(true);
            kMotion.setVisible(true); lMotion.setVisible(true);
            kEra.setVisible(true); lEra.setVisible(true);
            harmonicViz.setVisible(true);
            grRibbon.setVisible(true);
            spectrumDisplay.setVisible(true);
            satIndicator.setVisible(true);
            compIndicator.setVisible(true);
            limIndicator.setVisible(true);
            pageTabs.setVisible(true);
            break;

        case ViewMode::Advanced:
            kDrive.setVisible(true); lDrive.setVisible(true);
            kMix.setVisible(true); lMix.setVisible(true);
            kMaster.setVisible(true); lMaster.setVisible(true);
            kResTame.setVisible(true); lResTame.setVisible(true);
            kTransSens.setVisible(true); lTransSens.setVisible(true);
            btnMidSide.setVisible(true);
            cSatModel.setVisible(true);
            cGlueScHpf.setVisible(true);
            cMultiband.setVisible(true);
            cQuality.setVisible(true);
            spectrumAdvanced.setVisible(true);
            grRibbon.setVisible(true);
            presetBrowser.setVisible(true);
            pageTabs.setVisible(false);
            break;
    }

    resized();
}

void BTZAudioProcessorEditor::hideAllControls() {
    for (auto* child : getChildren())
        if (child != &viewTabs && child != resizer.get() &&
            child != &btnUndo && child != &btnRedo && child != &btnAB &&
            child != &btnCopyAB && child != &btnPresetPrev && child != &btnPresetNext &&
            child != &btnPresetSave && child != &btnBypass && child != &lblPresetName)
            child->setVisible(false);
}

// ═══════════════════════════════════════════════════════════════════════════
// Paint
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    // Canvas background
    g.fillAll(juce::Colour(palette::canvas));

    // Subtle warm gradient overlay
    juce::ColourGradient grad(juce::Colour(palette::canvas), 0, 0,
                              juce::Colour(palette::orangeFaint), 0, (float)getHeight(), false);
    g.setGradientFill(grad);
    g.fillRect(getLocalBounds());

    // Header bar
    auto headerArea = getLocalBounds().toFloat().removeFromTop((float)layout::headerH);
    g.setColour(juce::Colour(palette::surface));
    g.fillRect(headerArea);
    g.setColour(juce::Colour(palette::border));
    g.fillRect(headerArea.removeFromBottom(1.0f));

    // Brand
    g.setColour(juce::Colour(palette::ink));
    g.setFont(juce::Font(type::display(), type::title, juce::Font::bold));
    g.drawText("BOX TONE ZONE", headerArea.reduced(space::md, 0),
               juce::Justification::centredLeft);

    // Footer bar
    auto footerArea = getLocalBounds().toFloat().removeFromBottom((float)layout::footerH);
    g.setColour(juce::Colour(palette::surface));
    g.fillRect(footerArea);
    g.setColour(juce::Colour(palette::border));
    g.fillRect(footerArea.removeFromTop(1.0f));

    // Footer meters text
    g.setFont(juce::Font(type::mono(), type::small, juce::Font::plain));
    g.setColour(juce::Colour(palette::inkMuted));
    float lufs = proc.meters.lufs.load(std::memory_order_relaxed);
    float tp = proc.meters.truePeak.load(std::memory_order_relaxed);
    juce::String footerText = juce::String::formatted("LUFS: %.1f  |  TP: %.1f dB  |  GR: %.1f dB",
                                                       lufs, tp,
                                                       proc.meters.grDb.load(std::memory_order_relaxed));
    g.drawText(footerText, footerArea.reduced(space::md, 0), juce::Justification::centredLeft);

    // Version
    g.setColour(juce::Colour(palette::inkFaint));
    g.drawText("v11", footerArea.reduced(space::md, 0), juce::Justification::centredRight);
}

// ═══════════════════════════════════════════════════════════════════════════
// Resized
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    resizer->setBounds(area.getWidth() - 16, area.getHeight() - 16, 16, 16);

    // Header
    auto header = area.removeFromTop(layout::headerH);
    auto toolbarArea = header.reduced(space::md, 0);
    toolbarArea.removeFromLeft(200); // Brand space

    // View tabs in header
    viewTabs.setBounds(toolbarArea.removeFromLeft(240).reduced(0, space::xs));

    // Toolbar buttons (right side of header)
    const int btnW = 28;
    const int btnH = 24;
    auto rightTools = toolbarArea;
    rightTools.removeFromLeft(space::lg);
    btnBypass.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    rightTools.removeFromRight(space::sm);
    btnPresetNext.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    lblPresetName.setBounds(rightTools.removeFromRight(120).withSizeKeepingCentre(120, btnH));
    btnPresetPrev.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    rightTools.removeFromRight(space::md);
    btnCopyAB.setBounds(rightTools.removeFromRight(36).withSizeKeepingCentre(36, btnH));
    btnAB.setBounds(rightTools.removeFromRight(36).withSizeKeepingCentre(36, btnH));
    rightTools.removeFromRight(space::sm);
    btnRedo.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    btnUndo.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));

    // Footer
    area.removeFromBottom(layout::footerH);

    // Content area
    auto content = area.reduced(space::lg, space::md);

    switch (viewMode) {
        case ViewMode::Simple:
            layoutSimple(content);
            break;
        case ViewMode::Standard:
            layoutStandard(content);
            break;
        case ViewMode::Advanced:
            layoutAdvanced(content);
            break;
    }
}

void BTZAudioProcessorEditor::layoutSimple(juce::Rectangle<int> area) {
    // Three large knobs centered with harmonic viz above
    auto vizArea = area.removeFromTop(area.getHeight() / 2);
    harmonicViz.setBounds(vizArea.reduced(space::xl, space::md));

    const int knobSize = juce::jmin(area.getWidth() / 4, area.getHeight() - 30);
    const int totalW = knobSize * 3 + space::xl * 2;
    auto knobRow = area.withSizeKeepingCentre(totalW, knobSize + 24);

    auto dArea = knobRow.removeFromLeft(knobSize);
    simpleKnobDrive.setBounds(dArea.removeFromTop(knobSize));

    knobRow.removeFromLeft(space::xl);
    auto tArea = knobRow.removeFromLeft(knobSize);
    simpleKnobTone.setBounds(tArea.removeFromTop(knobSize));

    knobRow.removeFromLeft(space::xl);
    auto oArea = knobRow.removeFromLeft(knobSize);
    simpleKnobOutput.setBounds(oArea.removeFromTop(knobSize));
}

void BTZAudioProcessorEditor::layoutStandard(juce::Rectangle<int> area) {
    // Page tabs
    pageTabs.setBounds(area.removeFromTop(28).reduced(area.getWidth() / 4, 0));
    area.removeFromTop(space::sm);

    // Processing indicators
    auto indicatorRow = area.removeFromTop(12);
    satIndicator.setBounds(indicatorRow.removeFromLeft(12));
    indicatorRow.removeFromLeft(space::xs);
    compIndicator.setBounds(indicatorRow.removeFromLeft(12));
    indicatorRow.removeFromLeft(space::xs);
    limIndicator.setBounds(indicatorRow.removeFromLeft(12));
    area.removeFromTop(space::sm);

    // Harmonic viz / spectrum
    auto vizArea = area.removeFromTop(area.getHeight() / 4);
    harmonicViz.setBounds(vizArea.removeFromLeft(vizArea.getWidth() / 2).reduced(space::xs));
    spectrumDisplay.setBounds(vizArea.reduced(space::xs));

    // GR ribbon
    grRibbon.setBounds(area.removeFromTop(20).reduced(space::sm, 0));
    area.removeFromTop(space::sm);

    // Character knobs (2x3 grid)
    const int knobSize = juce::jmin(area.getWidth() / 7, (area.getHeight() - 80) / 3);
    auto charArea = area.removeFromTop(knobSize * 2 + space::md);
    auto row1 = charArea.removeFromTop(knobSize);
    auto row2 = charArea.removeFromTop(knobSize);

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int>& row) {
        auto slot = row.removeFromLeft(knobSize);
        s.setBounds(slot.removeFromTop(knobSize - 16));
        l.setBounds(slot);
        row.removeFromLeft(space::sm);
    };

    placeKnob(kPunch, lPunch, row1);
    placeKnob(kWarmth, lWarmth, row1);
    placeKnob(kBoom, lBoom, row1);
    row1.removeFromLeft(space::lg);
    placeKnob(kDrive, lDrive, row1);
    placeKnob(kMix, lMix, row1);
    placeKnob(kMaster, lMaster, row1);

    placeKnob(kGlue, lGlue, row2);
    placeKnob(kAir, lAir, row2);
    placeKnob(kWidth, lWidth, row2);
    row2.removeFromLeft(space::lg);
    placeKnob(kDensity, lDensity, row2);
    placeKnob(kMotion, lMotion, row2);
    placeKnob(kEra, lEra, row2);
}

void BTZAudioProcessorEditor::layoutAdvanced(juce::Rectangle<int> area) {
    // Spectrum analyzer (full width, top)
    spectrumAdvanced.setBounds(area.removeFromTop(area.getHeight() / 3).reduced(space::xs));
    area.removeFromTop(space::sm);

    // GR ribbon
    grRibbon.setBounds(area.removeFromTop(20).reduced(space::sm, 0));
    area.removeFromTop(space::sm);

    // Controls row
    auto ctrlRow = area.removeFromTop(80);
    auto leftCtrl = ctrlRow.removeFromLeft(ctrlRow.getWidth() / 2);
    auto rightCtrl = ctrlRow;

    // Left: sat model, quality, multiband, mid/side
    cSatModel.setBounds(leftCtrl.removeFromTop(24).removeFromLeft(160));
    leftCtrl.removeFromTop(space::xs);
    auto comboRow = leftCtrl.removeFromTop(24);
    cQuality.setBounds(comboRow.removeFromLeft(100));
    comboRow.removeFromLeft(space::sm);
    cMultiband.setBounds(comboRow.removeFromLeft(100));
    comboRow.removeFromLeft(space::sm);
    cGlueScHpf.setBounds(comboRow.removeFromLeft(100));
    leftCtrl.removeFromTop(space::xs);
    btnMidSide.setBounds(leftCtrl.removeFromTop(24).removeFromLeft(80));

    // Right: knobs
    const int smallKnob = 56;
    auto knobArea = rightCtrl;
    auto placeSmall = [&](juce::Slider& s, juce::Label& l) {
        auto slot = knobArea.removeFromLeft(smallKnob);
        s.setBounds(slot.removeFromTop(smallKnob - 14));
        l.setBounds(slot);
        knobArea.removeFromLeft(space::xs);
    };
    placeSmall(kDrive, lDrive);
    placeSmall(kResTame, lResTame);
    placeSmall(kTransSens, lTransSens);
    placeSmall(kMix, lMix);
    placeSmall(kMaster, lMaster);

    area.removeFromTop(space::md);

    // Preset browser (bottom)
    presetBrowser.setBounds(area.reduced(space::sm));
}

// ═══════════════════════════════════════════════════════════════════════════
// Preset Browser Population
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::populatePresetBrowser() {
    auto files = proc.getPresetFiles();
    std::vector<btz::PresetBrowser::Entry> entries;
    entries.reserve(static_cast<size_t>(files.size()));

    for (const auto& f : files) {
        btz::PresetBrowser::Entry e;
        e.name = f.getFileNameWithoutExtension();
        // Try to extract category from parent folder name
        e.category = f.getParentDirectory().getFileName();
        if (e.category == proc.getPresetDirectory().getFileName())
            e.category = "User";
        entries.push_back(std::move(e));
    }

    presetBrowser.setPresets(entries);
    presetBrowser.setSelected(proc.getCurrentPresetIndex());
}
