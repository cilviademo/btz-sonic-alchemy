/*
  Box Tone Zone (BTZ) — PluginEditor.cpp  v1.0 Ivory System
  ──────────────────────────────────────────────────────────────────────────
  Calm horizontal mastering-console architecture.
  All colours from btz::palette (Ivory System).
  All dimensions from btz::space/layout. Zero hardcoded hex values.
  ──────────────────────────────────────────────────────────────────────────
*/
#include "PluginEditor.h"

using namespace btz;

// ═══════════════════════════════════════════════════════════════════════════
// BTZLookAndFeel
// ═══════════════════════════════════════════════════════════════════════════

BTZLookAndFeel::BTZLookAndFeel() {
    setColour(juce::Slider::thumbColourId,              juce::Colour(palette::orange));
    setColour(juce::Slider::rotarySliderFillColourId,   juce::Colour(palette::orange));
    setColour(juce::Slider::trackColourId,              juce::Colour(palette::knobTrack));
    setColour(juce::Label::textColourId,                juce::Colour(palette::charcoal));
    setColour(juce::TextButton::buttonColourId,         juce::Colour(palette::porcelain));
    setColour(juce::TextButton::textColourOffId,        juce::Colour(palette::charcoal));
    setColour(juce::ComboBox::backgroundColourId,       juce::Colour(palette::porcelain));
    setColour(juce::ComboBox::textColourId,             juce::Colour(palette::charcoal));
    setColour(juce::ComboBox::outlineColourId,          juce::Colour(palette::panelBorder));
    setColour(juce::PopupMenu::backgroundColourId,      juce::Colour(palette::porcelain));
    setColour(juce::PopupMenu::textColourId,            juce::Colour(palette::charcoal));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(palette::sand));
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
    // Thin rounded track (warm gray inactive)
    auto track = juce::Rectangle<float>((float)x, (float)y + h * 0.45f, (float)w, 3.0f);
    g.setColour(juce::Colour(palette::knobTrack));
    g.fillRoundedRectangle(track, 1.5f);

    // Active fill (orange)
    const float fillW = sliderPos - (float)x;
    if (fillW > 0.5f) {
        g.setColour(juce::Colour(palette::orange));
        g.fillRoundedRectangle(track.withWidth(fillW), 1.5f);
    }

    // Small rounded handle with subtle shadow
    const float handleR = 5.0f;
    g.setColour(juce::Colour(palette::charcoal).withAlpha(0.15f));
    g.fillEllipse(sliderPos - handleR + 0.5f, track.getCentreY() - handleR + 0.5f,
                  handleR * 2.0f, handleR * 2.0f);
    g.setColour(juce::Colour(palette::ivory));
    g.fillEllipse(sliderPos - handleR, track.getCentreY() - handleR, handleR * 2.0f, handleR * 2.0f);
    g.setColour(juce::Colour(palette::panelBorder));
    g.drawEllipse(sliderPos - handleR, track.getCentreY() - handleR, handleR * 2.0f, handleR * 2.0f, 0.5f);
}

void BTZLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                            const juce::Colour&, bool highlighted, bool down) {
    auto bounds = b.getLocalBounds().toFloat().reduced(0.5f);
    if (down || b.getToggleState()) {
        g.setColour(juce::Colour(palette::orange));
    } else if (highlighted) {
        g.setColour(juce::Colour(palette::sand));
    } else {
        g.setColour(juce::Colour(palette::porcelain));
    }
    g.fillRoundedRectangle(bounds, radius::sm);
    g.setColour(juce::Colour(palette::panelBorder));
    g.drawRoundedRectangle(bounds, radius::sm, 0.5f);
}

void BTZLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool down) {
    g.setFont(juce::Font(type::sans(), type::labelSize, juce::Font::bold));
    g.setColour((down || b.getToggleState()) ? juce::Colour(palette::ivory)
                                              : juce::Colour(palette::charcoal));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

void BTZLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                                       bool highlighted, bool) {
    auto bounds = b.getLocalBounds().toFloat().reduced(1.0f);
    if (b.getToggleState()) {
        g.setColour(juce::Colour(palette::orange));
        g.fillRoundedRectangle(bounds, radius::sm);
        g.setColour(juce::Colour(palette::ivory));
    } else {
        g.setColour(juce::Colour(palette::porcelain));
        g.fillRoundedRectangle(bounds, radius::sm);
        g.setColour(juce::Colour(palette::panelBorder));
        g.drawRoundedRectangle(bounds, radius::sm, 0.5f);
        g.setColour(highlighted ? juce::Colour(palette::charcoal) : juce::Colour(palette::warmGray));
    }
    g.setFont(juce::Font(type::sans(), type::labelSize, juce::Font::bold));
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
    g.setColour(juce::Colour(palette::porcelain));
    g.fillRoundedRectangle(bounds, radius::sm);
    g.setColour(juce::Colour(palette::panelBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius::sm, 0.5f);

    // Dropdown arrow
    const float arrowX = (float)w - 16.0f;
    const float arrowY = (float)h * 0.5f - 2.0f;
    juce::Path arrow;
    arrow.addTriangle(arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour(juce::Colour(palette::warmGray));
    g.fillPath(arrow);
}

void BTZLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool, bool isHighlighted,
                                        bool isTicked, bool, const juce::String& text,
                                        const juce::String&, const juce::Drawable*, const juce::Colour*) {
    if (isSeparator) {
        g.setColour(juce::Colour(palette::panelBorder));
        g.fillRect(area.reduced(space::sm, 0).withHeight(1).withCentre(area.getCentre()));
        return;
    }
    if (isHighlighted) {
        g.setColour(juce::Colour(palette::sand));
        g.fillRect(area);
    }
    g.setColour(isTicked ? juce::Colour(palette::orange) : juce::Colour(palette::charcoal));
    g.setFont(juce::Font(type::sans(), type::bodySize, juce::Font::plain));
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

    // ── View tabs (Simple / Standard / Advanced) ──
    viewTabs.setTabs({ "SIMPLE", "STANDARD", "ADVANCED" });
    viewTabs.setActive(1);
    viewTabs.onTabChanged = [this](int idx) { setViewMode(static_cast<ViewMode>(idx)); };
    addAndMakeVisible(viewTabs);

    // ── Custom visualizer components ──
    addAndMakeVisible(harmonicViz);
    addAndMakeVisible(grRibbon);
    addAndMakeVisible(spectrumDisplay);
    addAndMakeVisible(spectrumAdvanced);
    addAndMakeVisible(satIndicator);
    addAndMakeVisible(compIndicator);
    addAndMakeVisible(limIndicator);
    addAndMakeVisible(presetBrowser);
    addAndMakeVisible(safetyTruePeak);
    addAndMakeVisible(safetyCorrelation);
    addAndMakeVisible(safetyGR);

    // ── Simple Mode knobs ──
    addAndMakeVisible(simpleKnobDrive);
    addAndMakeVisible(simpleKnobTone);
    addAndMakeVisible(simpleKnobOutput);

    // ── Header toolbar buttons ──
    for (auto* btn : { &btnUndo, &btnRedo, &btnAB, &btnCopyAB,
                       &btnPresetPrev, &btnPresetNext, &btnPresetSave, &btnDelta })
        addAndMakeVisible(btn);
    addAndMakeVisible(btnBypass);
    addAndMakeVisible(btnAutoGain);
    addAndMakeVisible(lblPresetName);
    lblPresetName.setJustificationType(juce::Justification::centred);
    lblPresetName.setFont(juce::Font(type::sans(), type::bodySize, juce::Font::plain));

    // Toolbar callbacks
    btnUndo.onClick = [this] { proc.undo(); };
    btnRedo.onClick = [this] { proc.redo(); };
    btnAB.onClick = [this] { proc.toggleAB(); };
    btnCopyAB.onClick = [this] { proc.copyAtoB(); };
    btnPresetPrev.onClick = [this] { loadCombinedPreset(presetBrowser.getSelected() - 1); };
    btnPresetNext.onClick = [this] { loadCombinedPreset(presetBrowser.getSelected() + 1); };
    btnDelta.onClick = [this] {
        deltaMode = !deltaMode;
        btnDelta.setToggleState(deltaMode, juce::dontSendNotification);
        proc.deltaMonitoring.store(deltaMode, std::memory_order_relaxed);
    };

    // ── Standard Mode: Character knobs (left) ──
    setupKnob(kPunch, lPunch, "PUNCH", id::punch,
              "Adds transient energy and low-mid compression timing. Use on drums and mix bus.");
    setupKnob(kWarmth, lWarmth, "WARMTH", id::warmth,
              "Low-mid harmonic density with transformer/tape saturation blend.");
    setupKnob(kBoom, lBoom, "BOOM", id::boom,
              "Low-frequency harmonic content. Subtle sub-bass enhancement.");
    setupKnob(kGlue, lGlue, "GLUE", id::glue,
              "Adds compression-like cohesion and harmonic binding. Use lightly on mix bus or vocals.");
    setupKnob(kAir, lAir, "AIR", id::air,
              "High-frequency harmonic lift with subtle exciter and phase-safe widening.");
    setupKnob(kWidth, lWidth, "WIDTH", id::width,
              "Stereo image control. Center = natural, right = wider, left = narrower.");

    // ── Standard Mode: Center (Drive/Mix/Master) ──
    setupKnob(kDrive, lDrive, "DRIVE", id::drive,
              "Main saturation amount. Controls harmonic intensity across all models.");
    setupKnob(kMix, lMix, "MIX", id::mix,
              "Wet/dry blend. Use for parallel saturation processing.");
    setupKnob(kMaster, lMaster, "MASTER", id::output,
              "Output level trim. Adjust after processing to match input level.");

    // ── Standard Mode: Bottom row ──
    setupSmallKnob(kDensity, lDensity, "DENSITY", id::density,
                   "Parallel compression character. Adds body and sustain.");
    setupSmallKnob(kMotion, lMotion, "MOTION", id::motion,
                   "Subtle LFO modulation on drive. Adds organic movement.");
    setupSmallKnob(kEra, lEra, "ERA", id::era,
                   "Blends between vintage (warm/dark) and modern (clean/bright) character.");
    setupSmallKnob(kIntensity, lIntensity, "INTENSITY", id::intensity,
                   "Overall processing intensity. Scales all active modules proportionally.");

    // ── Advanced Mode controls ──
    setupSmallKnob(kResTame, lResTame, "RES TAME", id::resTame,
                   "Resonance taming sensitivity. Detects and reduces harsh resonances.");
    setupSmallKnob(kTransSens, lTransSens, "TRANSIENT", id::transient,
                   "Transient detection sensitivity. Protects transients from over-saturation.");
    setupSmallKnob(kCeiling, lCeiling, "CEILING", id::ceiling,
                   "True peak limiter ceiling. Set to -0.3 dB for streaming, -1 dB for mastering.");
    setupSmallKnob(kShine, lShine, "SHINE", id::shine,
                   "High-frequency presence EQ. Adds air and sparkle above 4kHz.");
    setupSmallKnob(kGlueAttack, lGlueAttack, "ATTACK", id::glue,
                   "Glue compressor attack time in ms.");
    setupSmallKnob(kGlueRelease, lGlueRelease, "RELEASE", id::glue,
                   "Glue compressor release time in ms.");
    setupSmallKnob(kGlueRatio, lGlueRatio, "RATIO", id::glue,
                   "Glue compressor ratio. 2-4 for glue, higher for limiting.");

    addAndMakeVisible(btnMidSide);
    addAndMakeVisible(cSatModel);
    addAndMakeVisible(cGlueScHpf);
    addAndMakeVisible(cMultiband);
    addAndMakeVisible(cQuality);

    cSatModel.addItemList({ "Tanh", "Tube", "Tape", "Transistor", "Transformer",
                            "Neural Neve", "Neural API", "Neural SSL", "Neural Custom",
                            "WDF Tube", "WDF Transformer" }, 1);
    // Honesty pass: the four Neural slots have no trained weights yet and fall
    // back to fastTanh internally — they currently sound identical to "Tanh".
    // Greyed-out until real weights ship, so users aren't misled. Index→value
    // mapping is preserved (preset compatibility kept).
    for (int neuralId : { 6, 7, 8, 9 }) cSatModel.setItemEnabled(neuralId, false);

    cGlueScHpf.addItemList({ "Off", "60 Hz", "120 Hz", "250 Hz" }, 1);
    cMultiband.addItemList({ "Full Range", "2 Bands", "3 Bands", "4 Bands", "5 Bands", "6 Bands" }, 1);
    // Honesty pass: MultibandEngine is prepared but its split/recombine are not
    // yet routed in processNonlinear — only "Full Range" produces audible change.
    // Greyed-out until the audio path is wired (planned v1.1).
    for (int mbId : { 2, 3, 4, 5, 6 }) cMultiband.setItemEnabled(mbId, false);

    cQuality.addItemList({ "Eco (1x)", "Standard (2x)", "High (4x)", "Ultra (8x)" }, 1);

    // ── APVTS Attachments ──
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
    aIntensity = std::make_unique<SliderAttachment>(apvts, "intensity", kIntensity);
    aCeiling  = std::make_unique<SliderAttachment>(apvts, "ceiling", kCeiling);
    aShine    = std::make_unique<SliderAttachment>(apvts, "shine", kShine);
    aResTame  = std::make_unique<SliderAttachment>(apvts, "resSens", kResTame);
    aTransSens = std::make_unique<SliderAttachment>(apvts, "transSens", kTransSens);
    aGlueAttack = std::make_unique<SliderAttachment>(apvts, "glueAttack", kGlueAttack);
    aGlueRelease = std::make_unique<SliderAttachment>(apvts, "glueRelease", kGlueRelease);
    aGlueRatio = std::make_unique<SliderAttachment>(apvts, "glueRatio", kGlueRatio);
    aBypass   = std::make_unique<ButtonAttachment>(apvts, "bypass", btnBypass);
    aMidSide  = std::make_unique<ButtonAttachment>(apvts, "midSide", btnMidSide);
    aAutoGain = std::make_unique<ButtonAttachment>(apvts, "autoGain", btnAutoGain);
    aSatModel = std::make_unique<ComboAttachment>(apvts, "satModel", cSatModel);
    aGlueScHpf = std::make_unique<ComboAttachment>(apvts, "glueScHpf", cGlueScHpf);
    aMultiband = std::make_unique<ComboAttachment>(apvts, "multibandCount", cMultiband);
    aQuality  = std::make_unique<ComboAttachment>(apvts, "quality", cQuality);

    // ── Preset save button (async — no modal loop, plugin-safe) ──
    btnPresetSave.onClick = [this] {
        auto dir = proc.getPresetDirectory();
        if (!dir.exists()) dir.createDirectory();
        fileChooser = std::make_unique<juce::FileChooser>("Save Preset", dir, "*.btzpreset");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc) {
                const auto result = fc.getResult();
                if (result != juce::File{}) {
                    proc.savePreset(result);
                    populatePresetBrowser();
                }
            });
    };

    // ── Simple mode knob callbacks ──
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

    // ── Preset browser (factory presets first, then user files) ──
    presetBrowser.onSelect = [this](int idx) { loadCombinedPreset(idx); };
    populatePresetBrowser();

    // ── v1.0.1: Target Lock UI setup ──
    setupTargetLockUI();

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
                                          const juce::String& text, int compID,
                                          const juce::String& tooltip) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setComponentID(juce::String(compID));
    s.setPopupDisplayEnabled(true, true, this);
    if (tooltip.isNotEmpty())
        s.setTooltip(tooltip);
    addAndMakeVisible(s);

    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(type::sans(), type::labelSize, juce::Font::bold));
    l.setColour(juce::Label::textColourId, juce::Colour(palette::warmGray));
    addAndMakeVisible(l);
}

void BTZAudioProcessorEditor::setupSmallKnob(juce::Slider& s, juce::Label& l,
                                               const juce::String& text, int compID,
                                               const juce::String& tooltip) {
    setupKnob(s, l, text, compID, tooltip);
}

// ═══════════════════════════════════════════════════════════════════════════
// Timer — update meters and visualizers
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::timerCallback() {
    // GR ribbon
    const float gr = proc.meters.grDb.load(std::memory_order_relaxed);
    grRibbon.push(gr);

    // Processing indicators (color per design brief)
    satIndicator.setActive(gr < -0.1f, juce::Colour(palette::orange));
    compIndicator.setActive(gr < -1.0f, juce::Colour(palette::sage));
    limIndicator.setActive(gr < -3.0f, juce::Colour(palette::clay));

    // Preset name
    lblPresetName.setText(proc.getCurrentPresetName(), juce::dontSendNotification);

    // Simple mode knob sync
    if (viewMode == ViewMode::Simple) {
        simpleKnobDrive.setValue(kDrive.getValue() / kDrive.getMaximum(), "");
        simpleKnobTone.setValue(kShine.getValue() / kShine.getMaximum(), "");
        simpleKnobOutput.setValue(kMaster.getValue() / kMaster.getMaximum(), "");
    }

    // Spectrum + harmonic visualizers (FFT on the UI thread)
    updateVisualizers();

    // Safety indicators
    updateSafetyIndicators();

    repaint();
}

void BTZAudioProcessorEditor::updateVisualizers() {
    constexpr int N = BTZDsp::kSpectrumFFTSize;
    if (!proc.spectrumBuffer.ready) return;   // no full frame captured yet

    // Build a Hann window once.
    if (!fftWindowReady) {
        for (int i = 0; i < N; ++i)
            fftWindow[(size_t) i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (float) i / (float) (N - 1)));
        fftWindowReady = true;
    }

    // Snapshot the processor's circular buffer (single-writer/single-reader),
    // unwrapped from writePos, windowed. A torn read is cosmetically harmless.
    const int wp = proc.spectrumBuffer.writePos;
    for (int i = 0; i < N; ++i) {
        const int idx = (wp + i) % N;
        fftData[(size_t) i] = proc.spectrumBuffer.buffer[(size_t) idx] * fftWindow[(size_t) i];
    }
    std::fill(fftData.begin() + N, fftData.end(), 0.0f);

    fft.performFrequencyOnlyForwardTransform(fftData.data(), true);

    // Normalise magnitudes (≈ full-scale sine → ~1.0) into specMags.
    const float norm = 2.0f / (float) N;
    for (int i = 0; i < N / 2; ++i)
        specMags[(size_t) i] = fftData[(size_t) i] * norm;

    const float sr = (float) juce::jmax(1.0, proc.getSampleRate());
    spectrumDisplay.setSpectrum(specMags.data(), N / 2, sr);
    spectrumAdvanced.setSpectrum(specMags.data(), N / 2, sr);

    // 16 log-spaced bands (20 Hz – 20 kHz) → harmonic visualizer, mapped −60..0 dB → 0..1.
    const float binHz = sr / (float) N;
    for (int b = 0; b < 16; ++b) {
        const float f0 = 20.0f * std::pow(1000.0f, (float) b / 16.0f);
        const float f1 = 20.0f * std::pow(1000.0f, (float) (b + 1) / 16.0f);
        int lo = juce::jlimit(0, N / 2 - 1, (int) (f0 / binHz));
        int hi = juce::jlimit(lo + 1, N / 2, (int) (f1 / binHz) + 1);
        float peak = 0.0f;
        for (int k = lo; k < hi; ++k) peak = juce::jmax(peak, specMags[(size_t) k]);
        const float db = juce::Decibels::gainToDecibels(peak, -60.0f);
        harmonicMags[(size_t) b] = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
    }
    harmonicViz.setMagnitudes(harmonicMags.data(), 16);
}

void BTZAudioProcessorEditor::updateSafetyIndicators() {
    const float tp = proc.meters.truePeak.load(std::memory_order_relaxed);
    const float gr = proc.meters.grDb.load(std::memory_order_relaxed);

    // True peak warning (above -0.5 dBTP)
    safetyTruePeak.setLevel(tp > -0.5f ? SafetyIndicator::Level::Warning :
                            tp > -1.0f ? SafetyIndicator::Level::Caution :
                                         SafetyIndicator::Level::Safe);

    // Excessive GR warning (more than 6 dB)
    safetyGR.setLevel(gr < -6.0f ? SafetyIndicator::Level::Warning :
                      gr < -3.0f ? SafetyIndicator::Level::Caution :
                                   SafetyIndicator::Level::Safe);

    // Stereo correlation: +1 mono-safe, ~0 wide, <0 phase/mono-cancellation risk.
    const float corr = proc.meters.correlation.load(std::memory_order_relaxed);
    safetyCorrelation.setLevel(corr < 0.0f ? SafetyIndicator::Level::Warning :
                               corr < 0.3f ? SafetyIndicator::Level::Caution :
                                             SafetyIndicator::Level::Safe);
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
            safetyTruePeak.setVisible(true);
            safetyCorrelation.setVisible(true);
            break;

        case ViewMode::Standard:
            // Character knobs (left)
            kPunch.setVisible(true); lPunch.setVisible(true);
            kWarmth.setVisible(true); lWarmth.setVisible(true);
            kBoom.setVisible(true); lBoom.setVisible(true);
            kGlue.setVisible(true); lGlue.setVisible(true);
            kAir.setVisible(true); lAir.setVisible(true);
            kWidth.setVisible(true); lWidth.setVisible(true);
            // Center
            kDrive.setVisible(true); lDrive.setVisible(true);
            kMix.setVisible(true); lMix.setVisible(true);
            kMaster.setVisible(true); lMaster.setVisible(true);
            // Bottom
            kDensity.setVisible(true); lDensity.setVisible(true);
            kMotion.setVisible(true); lMotion.setVisible(true);
            kEra.setVisible(true); lEra.setVisible(true);
            kIntensity.setVisible(true); lIntensity.setVisible(true);
            // Visualizers
            harmonicViz.setVisible(true);
            grRibbon.setVisible(true);
            spectrumDisplay.setVisible(true);
            satIndicator.setVisible(true);
            compIndicator.setVisible(true);
            limIndicator.setVisible(true);
            safetyTruePeak.setVisible(true);
            safetyCorrelation.setVisible(true);
            safetyGR.setVisible(true);
            break;

        case ViewMode::Advanced:
            // All Standard knobs
            kDrive.setVisible(true); lDrive.setVisible(true);
            kMix.setVisible(true); lMix.setVisible(true);
            kMaster.setVisible(true); lMaster.setVisible(true);
            // Advanced-specific
            kResTame.setVisible(true); lResTame.setVisible(true);
            kTransSens.setVisible(true); lTransSens.setVisible(true);
            kCeiling.setVisible(true); lCeiling.setVisible(true);
            kShine.setVisible(true); lShine.setVisible(true);
            kGlueAttack.setVisible(true); lGlueAttack.setVisible(true);
            kGlueRelease.setVisible(true); lGlueRelease.setVisible(true);
            kGlueRatio.setVisible(true); lGlueRatio.setVisible(true);
            btnMidSide.setVisible(true);
            cSatModel.setVisible(true);
            cGlueScHpf.setVisible(true);
            cMultiband.setVisible(true);
            cQuality.setVisible(true);
            // Visualizers
            spectrumAdvanced.setVisible(true);
            grRibbon.setVisible(true);
            harmonicViz.setVisible(true);
            presetBrowser.setVisible(true);
            satIndicator.setVisible(true);
            compIndicator.setVisible(true);
            limIndicator.setVisible(true);
            safetyTruePeak.setVisible(true);
            safetyCorrelation.setVisible(true);
            safetyGR.setVisible(true);
            break;
    }

    resized();
}

void BTZAudioProcessorEditor::hideAllControls() {
    for (auto* child : getChildren())
        if (child != &viewTabs && child != resizer.get() &&
            child != &btnUndo && child != &btnRedo && child != &btnAB &&
            child != &btnCopyAB && child != &btnPresetPrev && child != &btnPresetNext &&
            child != &btnPresetSave && child != &btnDelta && child != &btnBypass &&
            child != &btnAutoGain && child != &lblPresetName)
            child->setVisible(false);
}

// ═══════════════════════════════════════════════════════════════════════════
// Paint — Ivory System background
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::paint(juce::Graphics& g) {
    // Warm ivory canvas
    g.fillAll(juce::Colour(palette::ivory));

    // Subtle warm gradient (ivory → linen at bottom)
    juce::ColourGradient grad(juce::Colour(palette::ivory), 0, 0,
                              juce::Colour(palette::linen), 0, (float)getHeight(), false);
    g.setGradientFill(grad);
    g.fillRect(getLocalBounds());

    // Header bar (porcelain)
    auto headerArea = getLocalBounds().toFloat().removeFromTop((float)layout::headerH);
    g.setColour(juce::Colour(palette::porcelain));
    g.fillRect(headerArea);
    g.setColour(juce::Colour(palette::panelBorder));
    g.fillRect(headerArea.removeFromBottom(1.0f));

    // Brand wordmark — wide, spaced, confident
    g.setColour(juce::Colour(palette::softBlack));
    g.setFont(juce::Font(type::display(), type::brandSize, juce::Font::bold));
    auto brandArea = getLocalBounds().toFloat().removeFromTop((float)layout::headerH);
    g.drawText("B O X   T O N E   Z O N E", brandArea.reduced((float)space::md, 0),
               juce::Justification::centredLeft);

    // Footer bar
    auto footerArea = getLocalBounds().toFloat().removeFromBottom((float)layout::footerH);
    g.setColour(juce::Colour(palette::porcelain));
    g.fillRect(footerArea);
    g.setColour(juce::Colour(palette::panelBorder));
    g.fillRect(footerArea.removeFromTop(1.0f));

    // Footer metering text
    g.setFont(juce::Font(type::mono(), type::microSize, juce::Font::plain));
    g.setColour(juce::Colour(palette::warmGray));
    float lufs = proc.meters.lufs.load(std::memory_order_relaxed);
    float tp = proc.meters.truePeak.load(std::memory_order_relaxed);
    float grVal = proc.meters.grDb.load(std::memory_order_relaxed);
    juce::String footerText = juce::String::formatted(
        "LUFS: %.1f   |   TRUE PEAK: %.1f dBTP   |   GR: %.1f dB   |   %s",
        lufs, tp, grVal,
        deltaMode ? "DELTA ON" : "");
    g.drawText(footerText, footerArea.reduced((float)space::md, 0),
               juce::Justification::centredLeft);

    // Target Lock readout (flagship) — show current vs typed target + status.
    if (proc.meters.targetActive.load(std::memory_order_relaxed)) {
        const float tgt = proc.meters.targetValue.load(std::memory_order_relaxed);
        const bool isLufs = proc.meters.targetIsLufs.load(std::memory_order_relaxed);
        const bool onTgt = proc.meters.targetOnTarget.load(std::memory_order_relaxed);
        juce::String tl = juce::String::formatted("TARGET LOCK  %.1f %s  %s",
            tgt, isLufs ? "LUFS" : "dB", onTgt ? "LOCKED" : "approaching");
        g.setColour(juce::Colour(onTgt ? palette::sage : palette::gold));
        g.setFont(juce::Font(type::mono(), type::microSize, juce::Font::bold));
        g.drawText(tl, footerArea.reduced((float)space::md, 0),
                   juce::Justification::centred);
    }

    // Version badge
    g.setColour(juce::Colour(palette::warmGray));
    g.drawText("v1.0", footerArea.reduced((float)space::md, 0), juce::Justification::centredRight);
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
    toolbarArea.removeFromLeft(220); // Brand space

    // View tabs in header
    viewTabs.setBounds(toolbarArea.removeFromLeft(260).reduced(0, space::xs));

    // Toolbar buttons (right side)
    const int btnW = 32;
    const int btnH = 24;
    auto rightTools = toolbarArea;
    rightTools.removeFromLeft(space::lg);

    btnBypass.setBounds(rightTools.removeFromRight(56).withSizeKeepingCentre(56, btnH));
    rightTools.removeFromRight(space::xs);
    btnAutoGain.setBounds(rightTools.removeFromRight(44).withSizeKeepingCentre(44, btnH));
    rightTools.removeFromRight(space::sm);
    btnPresetNext.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    lblPresetName.setBounds(rightTools.removeFromRight(130).withSizeKeepingCentre(130, btnH));
    btnPresetPrev.setBounds(rightTools.removeFromRight(btnW).withSizeKeepingCentre(btnW, btnH));
    rightTools.removeFromRight(space::md);
    btnDelta.setBounds(rightTools.removeFromRight(48).withSizeKeepingCentre(48, btnH));
    rightTools.removeFromRight(space::xs);
    btnCopyAB.setBounds(rightTools.removeFromRight(36).withSizeKeepingCentre(36, btnH));
    btnAB.setBounds(rightTools.removeFromRight(28).withSizeKeepingCentre(28, btnH));
    rightTools.removeFromRight(space::sm);
    btnRedo.setBounds(rightTools.removeFromRight(40).withSizeKeepingCentre(40, btnH));
    btnUndo.setBounds(rightTools.removeFromRight(40).withSizeKeepingCentre(40, btnH));

    // Footer
    area.removeFromBottom(layout::footerH);

    // Content area
    auto content = area.reduced(space::lg, space::md);

    switch (viewMode) {
        case ViewMode::Simple:   layoutSimple(content); break;
        case ViewMode::Standard: layoutStandard(content); break;
        case ViewMode::Advanced: layoutAdvanced(content); break;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Layout: Simple Mode
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::layoutSimple(juce::Rectangle<int> area) {
    // Harmonic visualizer (circular bloom) — top half
    auto vizArea = area.removeFromTop(area.getHeight() * 55 / 100);
    harmonicViz.setBounds(vizArea.reduced(space::xl, space::md));

    // Three large knobs centered — bottom half
    const int knobSize = juce::jmin(area.getWidth() / 4, area.getHeight() - 40);
    const int totalW = knobSize * 3 + space::xl * 2;
    auto knobRow = area.withSizeKeepingCentre(totalW, knobSize + 28);

    auto dArea = knobRow.removeFromLeft(knobSize);
    simpleKnobDrive.setBounds(dArea);
    knobRow.removeFromLeft(space::xl);

    auto tArea = knobRow.removeFromLeft(knobSize);
    simpleKnobTone.setBounds(tArea);
    knobRow.removeFromLeft(space::xl);

    auto oArea = knobRow.removeFromLeft(knobSize);
    simpleKnobOutput.setBounds(oArea);

    // Safety indicators at bottom
    auto safetyRow = area.removeFromBottom(16);
    safetyTruePeak.setBounds(safetyRow.removeFromLeft(80));
    safetyRow.removeFromLeft(space::sm);
    safetyCorrelation.setBounds(safetyRow.removeFromLeft(80));
}

// ═══════════════════════════════════════════════════════════════════════════
// Layout: Standard Mode
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::layoutStandard(juce::Rectangle<int> area) {
    // Processing indicators (top row)
    auto indicatorRow = area.removeFromTop(14);
    satIndicator.setBounds(indicatorRow.removeFromLeft(14));
    indicatorRow.removeFromLeft(space::xs);
    compIndicator.setBounds(indicatorRow.removeFromLeft(14));
    indicatorRow.removeFromLeft(space::xs);
    limIndicator.setBounds(indicatorRow.removeFromLeft(14));
    area.removeFromTop(space::sm);

    // Visualizer row (harmonic + spectrum side by side)
    auto vizRow = area.removeFromTop(area.getHeight() * 28 / 100);
    harmonicViz.setBounds(vizRow.removeFromLeft(vizRow.getWidth() / 2).reduced(space::xs));
    spectrumDisplay.setBounds(vizRow.reduced(space::xs));

    // GR ribbon
    grRibbon.setBounds(area.removeFromTop(18).reduced(space::md, 0));
    area.removeFromTop(space::sm);

    // Main knob area — 3 columns
    auto mainArea = area.removeFromTop(area.getHeight() * 60 / 100);
    const int knobSize = juce::jmin(mainArea.getWidth() / 8, (mainArea.getHeight() - 20) / 2);

    // Left column: Character knobs (2x3 grid)
    auto leftCol = mainArea.removeFromLeft(mainArea.getWidth() * 38 / 100);
    auto charRow1 = leftCol.removeFromTop(knobSize + 16);
    auto charRow2 = leftCol.removeFromTop(knobSize + 16);

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int>& row) {
        auto slot = row.removeFromLeft(knobSize);
        s.setBounds(slot.removeFromTop(knobSize));
        l.setBounds(slot);
        row.removeFromLeft(space::sm);
    };

    placeKnob(kPunch, lPunch, charRow1);
    placeKnob(kWarmth, lWarmth, charRow1);
    placeKnob(kBoom, lBoom, charRow1);
    placeKnob(kGlue, lGlue, charRow2);
    placeKnob(kAir, lAir, charRow2);
    placeKnob(kWidth, lWidth, charRow2);

    // Center column: Drive / Mix / Master (large)
    auto centerCol = mainArea.removeFromLeft(mainArea.getWidth() * 50 / 100);
    const int bigKnob = (int)(knobSize * 1.3f);
    auto centerKnobRow = centerCol.withSizeKeepingCentre(bigKnob * 3 + space::lg * 2, bigKnob + 16);
    auto driveSlot = centerKnobRow.removeFromLeft(bigKnob);
    kDrive.setBounds(driveSlot.removeFromTop(bigKnob));
    lDrive.setBounds(driveSlot);
    centerKnobRow.removeFromLeft(space::lg);
    auto mixSlot = centerKnobRow.removeFromLeft(bigKnob);
    kMix.setBounds(mixSlot.removeFromTop(bigKnob));
    lMix.setBounds(mixSlot);
    centerKnobRow.removeFromLeft(space::lg);
    auto masterSlot = centerKnobRow.removeFromLeft(bigKnob);
    kMaster.setBounds(masterSlot.removeFromTop(bigKnob));
    lMaster.setBounds(masterSlot);

    // Right column: meters/safety (handled by indicators already placed)

    // Bottom row: Density / Motion / Era / Intensity
    area.removeFromTop(space::sm);
    auto bottomRow = area;
    const int smallKnob = (int)(knobSize * 0.8f);
    auto bottomCenter = bottomRow.withSizeKeepingCentre(smallKnob * 4 + space::md * 3, smallKnob + 16);

    auto placeSmall = [&](juce::Slider& s, juce::Label& l) {
        auto slot = bottomCenter.removeFromLeft(smallKnob);
        s.setBounds(slot.removeFromTop(smallKnob));
        l.setBounds(slot);
        bottomCenter.removeFromLeft(space::md);
    };
    placeSmall(kDensity, lDensity);
    placeSmall(kMotion, lMotion);
    placeSmall(kEra, lEra);
    placeSmall(kIntensity, lIntensity);

    // Safety indicators (bottom-right)
    auto safetyArea = area.removeFromBottom(16);
    safetyArea = safetyArea.removeFromRight(280);
    safetyTruePeak.setBounds(safetyArea.removeFromLeft(80));
    safetyArea.removeFromLeft(space::sm);
    safetyCorrelation.setBounds(safetyArea.removeFromLeft(80));
    safetyArea.removeFromLeft(space::sm);
    safetyGR.setBounds(safetyArea.removeFromLeft(80));
}

// ═══════════════════════════════════════════════════════════════════════════
// Layout: Advanced Mode
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::layoutAdvanced(juce::Rectangle<int> area) {
    // Top: Spectrum analyzer + harmonic visualizer
    auto vizRow = area.removeFromTop(area.getHeight() * 30 / 100);
    spectrumAdvanced.setBounds(vizRow.removeFromLeft(vizRow.getWidth() * 60 / 100).reduced(space::xs));
    harmonicViz.setBounds(vizRow.reduced(space::xs));

    // GR ribbon + indicators
    auto indicatorRow = area.removeFromTop(14);
    satIndicator.setBounds(indicatorRow.removeFromLeft(14));
    indicatorRow.removeFromLeft(space::xs);
    compIndicator.setBounds(indicatorRow.removeFromLeft(14));
    indicatorRow.removeFromLeft(space::xs);
    limIndicator.setBounds(indicatorRow.removeFromLeft(14));
    area.removeFromTop(space::xs);
    grRibbon.setBounds(area.removeFromTop(18).reduced(space::md, 0));
    area.removeFromTop(space::sm);

    // Three-column layout per design brief
    auto mainArea = area.removeFromTop(area.getHeight() * 65 / 100);

    // LEFT: Saturation model, dynamics, multiband, quality
    auto leftCol = mainArea.removeFromLeft(mainArea.getWidth() / 3);
    leftCol = leftCol.reduced(space::sm, 0);
    cSatModel.setBounds(leftCol.removeFromTop(26));
    leftCol.removeFromTop(space::xs);
    cQuality.setBounds(leftCol.removeFromTop(26));
    leftCol.removeFromTop(space::xs);
    cMultiband.setBounds(leftCol.removeFromTop(26));
    leftCol.removeFromTop(space::xs);
    cGlueScHpf.setBounds(leftCol.removeFromTop(26));
    leftCol.removeFromTop(space::sm);
    btnMidSide.setBounds(leftCol.removeFromTop(26).removeFromLeft(80));
    leftCol.removeFromTop(space::sm);

    // Glue compressor detail knobs
    const int advKnob = 48;
    auto glueRow = leftCol.removeFromTop(advKnob + 14);
    auto placeAdv = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int>& row) {
        auto slot = row.removeFromLeft(advKnob);
        s.setBounds(slot.removeFromTop(advKnob));
        l.setBounds(slot);
        row.removeFromLeft(space::xs);
    };
    placeAdv(kGlueAttack, lGlueAttack, glueRow);
    placeAdv(kGlueRelease, lGlueRelease, glueRow);
    placeAdv(kGlueRatio, lGlueRatio, glueRow);

    // CENTER: Drive/Mix/Master + Res/Trans/Ceiling/Shine
    auto centerCol = mainArea.removeFromLeft(mainArea.getWidth() / 2);
    centerCol = centerCol.reduced(space::sm, 0);
    const int ctrKnob = 56;
    auto ctrRow1 = centerCol.removeFromTop(ctrKnob + 14);
    auto placeCenter = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int>& row) {
        auto slot = row.removeFromLeft(ctrKnob);
        s.setBounds(slot.removeFromTop(ctrKnob));
        l.setBounds(slot);
        row.removeFromLeft(space::sm);
    };
    placeCenter(kDrive, lDrive, ctrRow1);
    placeCenter(kMix, lMix, ctrRow1);
    placeCenter(kMaster, lMaster, ctrRow1);

    centerCol.removeFromTop(space::sm);
    auto ctrRow2 = centerCol.removeFromTop(ctrKnob + 14);
    placeCenter(kResTame, lResTame, ctrRow2);
    placeCenter(kTransSens, lTransSens, ctrRow2);
    placeCenter(kCeiling, lCeiling, ctrRow2);
    placeCenter(kShine, lShine, ctrRow2);

    // CENTER continued: Target Lock section
    centerCol.removeFromTop(space::md);
    lblTargetSection.setBounds(centerCol.removeFromTop(16));
    centerCol.removeFromTop(space::xs);

    // Target Lock row: LUFS | RMS | DYN RANGE knob
    auto tlRow1 = centerCol.removeFromTop(36);
    const int tlFieldW = 60;
    const int tlLockW = 40;
    {
        auto lufsArea = tlRow1.removeFromLeft(tlFieldW + tlLockW + space::xs);
        lblTargetLUFS.setBounds(lufsArea.removeFromTop(12));
        auto lufsRow = lufsArea.removeFromTop(24);
        txtTargetLUFS.setBounds(lufsRow.removeFromLeft(tlFieldW));
        lufsRow.removeFromLeft(space::xs);
        btnLUFSLock.setBounds(lufsRow.removeFromLeft(tlLockW));
    }
    tlRow1.removeFromLeft(space::sm);
    {
        auto rmsArea = tlRow1.removeFromLeft(tlFieldW + tlLockW + space::xs);
        lblTargetRMS.setBounds(rmsArea.removeFromTop(12));
        auto rmsRow = rmsArea.removeFromTop(24);
        txtTargetRMS.setBounds(rmsRow.removeFromLeft(tlFieldW));
        rmsRow.removeFromLeft(space::xs);
        btnRMSLock.setBounds(rmsRow.removeFromLeft(tlLockW));
    }
    tlRow1.removeFromLeft(space::sm);
    {
        auto dynArea = tlRow1.removeFromLeft(56);
        kDynThreshold.setBounds(dynArea.removeFromTop(40));
        lDynThreshold.setBounds(dynArea);
    }

    // Per-band targets row: LOW | MID | HIGH
    centerCol.removeFromTop(space::xs);
    auto tlRow2 = centerCol.removeFromTop(36);
    {
        auto lowArea = tlRow2.removeFromLeft(tlFieldW + tlLockW + space::xs);
        lblTargetLow.setBounds(lowArea.removeFromTop(12));
        auto lowRow = lowArea.removeFromTop(24);
        txtTargetLow.setBounds(lowRow.removeFromLeft(tlFieldW));
        lowRow.removeFromLeft(space::xs);
        btnLowLock.setBounds(lowRow.removeFromLeft(tlLockW));
    }
    tlRow2.removeFromLeft(space::sm);
    {
        auto midArea = tlRow2.removeFromLeft(tlFieldW + tlLockW + space::xs);
        lblTargetMid.setBounds(midArea.removeFromTop(12));
        auto midRow = midArea.removeFromTop(24);
        txtTargetMid.setBounds(midRow.removeFromLeft(tlFieldW));
        midRow.removeFromLeft(space::xs);
        btnMidLock.setBounds(midRow.removeFromLeft(tlLockW));
    }
    tlRow2.removeFromLeft(space::sm);
    {
        auto highArea = tlRow2.removeFromLeft(tlFieldW + tlLockW + space::xs);
        lblTargetHigh.setBounds(highArea.removeFromTop(12));
        auto highRow = highArea.removeFromTop(24);
        txtTargetHigh.setBounds(highRow.removeFromLeft(tlFieldW));
        highRow.removeFromLeft(space::xs);
        btnHighLock.setBounds(highRow.removeFromLeft(tlLockW));
    }

    // RIGHT: Neural model browser / preset browser
    auto rightCol = mainArea;
    rightCol = rightCol.reduced(space::sm, 0);
    presetBrowser.setBounds(rightCol);

    // Bottom: Safety indicators
    auto safetyArea = area.removeFromBottom(16);
    safetyTruePeak.setBounds(safetyArea.removeFromLeft(80));
    safetyArea.removeFromLeft(space::sm);
    safetyCorrelation.setBounds(safetyArea.removeFromLeft(80));
    safetyArea.removeFromLeft(space::sm);
    safetyGR.setBounds(safetyArea.removeFromLeft(80));
}

// ═══════════════════════════════════════════════════════════════════════════
// Preset Browser Population
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::populatePresetBrowser() {
    std::vector<btz::PresetBrowser::Entry> entries;

    // Factory presets first (always available).
    numFactoryPresets_ = proc.getNumFactoryPresets();
    for (const auto& fp : proc.getFactoryPresets())
        entries.push_back({ fp.name, fp.category });

    // Then user presets from disk.
    for (const auto& f : proc.getPresetFiles())
        entries.push_back({ f.getFileNameWithoutExtension(), "User" });

    presetBrowser.setPresets(entries);
}

void BTZAudioProcessorEditor::loadCombinedPreset(int combinedIndex) {
    const int total = numFactoryPresets_ + proc.getPresetFiles().size();
    if (total <= 0) return;
    const int idx = juce::jlimit(0, total - 1, combinedIndex);
    if (idx < numFactoryPresets_)
        proc.loadFactoryPreset(idx);
    else
        proc.loadPresetByIndex(idx - numFactoryPresets_);
    presetBrowser.setSelected(idx);
}

// ═══════════════════════════════════════════════════════════════════════════
// v1.0.1: Target Lock UI Setup
// ═══════════════════════════════════════════════════════════════════════════

void BTZAudioProcessorEditor::setupTargetLockUI() {
    auto& apvts = proc.getAPVTS();

    // Section label
    lblTargetSection.setText("TARGET LOCK", juce::dontSendNotification);
    lblTargetSection.setFont(juce::Font(type::sans(), 11.0f, juce::Font::bold));
    lblTargetSection.setColour(juce::Label::textColourId, juce::Colour(palette::charcoal));
    addAndMakeVisible(lblTargetSection);

    // Helper: setup a text editor for numeric input
    auto setupTextInput = [this](juce::TextEditor& txt, juce::Label& lbl,
                                 const juce::String& labelText, const juce::String& paramID) {
        lbl.setText(labelText, juce::dontSendNotification);
        lbl.setFont(juce::Font(type::sans(), 10.0f, juce::Font::plain));
        lbl.setColour(juce::Label::textColourId, juce::Colour(palette::warmGray));
        addAndMakeVisible(lbl);

        txt.setFont(juce::Font(type::mono(), 12.0f, juce::Font::plain));
        txt.setJustification(juce::Justification::centred);
        txt.setColour(juce::TextEditor::backgroundColourId, juce::Colour(palette::porcelain));
        txt.setColour(juce::TextEditor::outlineColourId, juce::Colour(palette::panelBorder));
        txt.setColour(juce::TextEditor::textColourId, juce::Colour(palette::charcoal));
        txt.setInputRestrictions(6, "-0123456789.");
        addAndMakeVisible(txt);

        // Push value to APVTS on return key or focus lost
        txt.onReturnKey = [this, paramID, &txt] {
            const float val = txt.getText().getFloatValue();
            if (auto* p = proc.getAPVTS().getParameter(paramID))
                p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(val));
        };
        txt.onFocusLost = txt.onReturnKey;
    };

    // LUFS target
    setupTextInput(txtTargetLUFS, lblTargetLUFS, "LUFS", "targetLUFS");
    addAndMakeVisible(btnLUFSLock);
    aLUFSLock = std::make_unique<ButtonAttachment>(apvts, "targetLUFSLock", btnLUFSLock);

    // RMS target
    setupTextInput(txtTargetRMS, lblTargetRMS, "RMS", "targetRMS");
    addAndMakeVisible(btnRMSLock);
    aRMSLock = std::make_unique<ButtonAttachment>(apvts, "targetRMSLock", btnRMSLock);

    // Per-band targets
    setupTextInput(txtTargetLow, lblTargetLow, "LOW", "targetLowDb");
    addAndMakeVisible(btnLowLock);
    aLowLock = std::make_unique<ButtonAttachment>(apvts, "targetLowLock", btnLowLock);

    setupTextInput(txtTargetMid, lblTargetMid, "MID", "targetMidDb");
    addAndMakeVisible(btnMidLock);
    aMidLock = std::make_unique<ButtonAttachment>(apvts, "targetMidLock", btnMidLock);

    setupTextInput(txtTargetHigh, lblTargetHigh, "HIGH", "targetHighDb");
    addAndMakeVisible(btnHighLock);
    aHighLock = std::make_unique<ButtonAttachment>(apvts, "targetHighLock", btnHighLock);

    // Dynamics threshold knob
    kDynThreshold.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kDynThreshold.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    kDynThreshold.setComponentID(juce::String(id::density));
    kDynThreshold.setPopupDisplayEnabled(true, true, this);
    kDynThreshold.setTooltip("Dynamics Threshold: how much dynamic range to preserve around the target. "
                             "0 = hard lock (brick-wall). 24 = loose (gentle correction only).");
    addAndMakeVisible(kDynThreshold);
    lDynThreshold.setText("DYN RANGE", juce::dontSendNotification);
    lDynThreshold.setFont(juce::Font(type::sans(), 9.0f, juce::Font::plain));
    lDynThreshold.setJustificationType(juce::Justification::centred);
    lDynThreshold.setColour(juce::Label::textColourId, juce::Colour(palette::warmGray));
    addAndMakeVisible(lDynThreshold);
    aDynThreshold = std::make_unique<SliderAttachment>(apvts, "targetDynThresh", kDynThreshold);

    // Sync initial values from APVTS into text editors
    syncTargetLockFromAPVTS();
}

void BTZAudioProcessorEditor::syncTargetLockFromAPVTS() {
    auto& apvts = proc.getAPVTS();

    auto syncText = [&](juce::TextEditor& txt, const juce::String& paramID) {
        const float val = *apvts.getRawParameterValue(paramID);
        txt.setText(juce::String(val, 1), juce::dontSendNotification);
    };

    syncText(txtTargetLUFS, "targetLUFS");
    syncText(txtTargetRMS, "targetRMS");
    syncText(txtTargetLow, "targetLowDb");
    syncText(txtTargetMid, "targetMidDb");
    syncText(txtTargetHigh, "targetHighDb");
}
