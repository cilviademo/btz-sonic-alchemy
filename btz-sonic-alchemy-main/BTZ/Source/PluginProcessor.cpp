/*
  Box Tone Zone (BTZ) — PluginProcessor.cpp  v1.0 Ivory System
  ──────────────────────────────────────────────────────────────────────────
  All APIs match BTZDsp.h v1.0. Thread model: processBlock = audio thread only.
  Delta monitoring, auto-gain, correlation metering, full signal chain.
  ──────────────────────────────────────────────────────────────────────────
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

// ═══════════════════════════════════════════════════════════════════════════
// Parameter Layout
// ═══════════════════════════════════════════════════════════════════════════
juce::AudioProcessorValueTreeState::ParameterLayout
BTZAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto uni = juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f);
    auto biRange = juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f);

    // ── Character knobs ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("punch",   "Punch",   uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("warmth",  "Warmth",  uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("boom",    "Boom",    uni, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glue",    "Glue",    uni, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("air",     "Air",     uni, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("width",   "Width",   uni, 0.5f));

    // ── Drive / Mix / Master ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive",   "Drive",   uni, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix",     "Mix",     uni, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master",  "Master",  uni, 0.7f));

    // ── Macros ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("density", "Density", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("motion",  "Motion",  uni, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("era",     "Era",     uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro0",  "Macro A", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro1",  "Macro B", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro2",  "Macro C", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro3",  "Macro D", uni, 0.5f));

    // ── Shine EQ ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shine",     "Shine",     uni, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineMix",  "Shine Mix", uni, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineFreq", "Shine Freq",
        juce::NormalisableRange<float>(1000.0f, 16000.0f, 1.0f, 0.5f), 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineQ",    "Shine Q",
        juce::NormalisableRange<float>(0.3f, 8.0f, 0.01f, 0.5f), 0.707f));

    // ── Saturation model ──
    params.push_back(std::make_unique<juce::AudioParameterInt>("satModel", "Saturation Model",
        0, (int)BTZDsp::SaturationModel::NumModels - 1, 0));

    // ── Dynamics ──
    params.push_back(std::make_unique<juce::AudioParameterInt>("glueScHpf", "Glue SC HPF", 0, 3, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling", "Limiter Ceiling",
        juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), -0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("intensity", "Intensity", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glueAttack", "Glue Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.5f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glueRelease", "Glue Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.5f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glueRatio", "Glue Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 3.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glueLink", "Glue Stereo Link", uni, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("autoGain", "Auto Gain", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("midSide", "Mid/Side", false));

    // ── Quality / Oversampling ──
    params.push_back(std::make_unique<juce::AudioParameterInt>("quality", "Quality", 0, 3, 1));

    // ── Multiband ──
    params.push_back(std::make_unique<juce::AudioParameterInt>("multibandCount", "Multiband Count", 0, 5, 0));

    // ── v10: Resonance Taming ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("resSens",  "Resonance Sensitivity", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("resDepth", "Resonance Depth",       uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("resEnabled", "Resonance Tame On", false));

    // ── v10: Transient-Aware Saturation ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("transSens", "Transient Sensitivity", uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("transMix",  "Transient Mix",         uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("transEnabled", "Transient Split On", false));

    // ── v10: Reference Tone Matching ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("toneMatchAmt", "Tone Match Amount", uni, 0.0f));

    // ── v10: Simple Mode ──
    params.push_back(std::make_unique<juce::AudioParameterBool>("simpleMode", "Simple Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("simpleDrive",  "Simple Drive",  uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("simpleTone",   "Simple Tone",   uni, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("simpleOutput", "Simple Output", uni, 0.5f));

    // ── v10: LFO ──
    params.push_back(std::make_unique<juce::AudioParameterInt>("lfoCount", "LFO Count", 0, 4, 0));

    // ── v1.0.1: Target Lock ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetLUFS", "Target LUFS",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -14.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetRMS", "Target RMS",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -14.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetDynThresh", "Dynamics Threshold",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 3.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("targetLUFSLock", "LUFS Lock", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("targetRMSLock", "RMS Lock", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetLowDb", "Target Low dB",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetMidDb", "Target Mid dB",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetHighDb", "Target High dB",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("targetLowLock", "Low Band Lock", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("targetMidLock", "Mid Band Lock", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("targetHighLock", "High Band Lock", false));

    return { params.begin(), params.end() };
}

// ═══════════════════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════════════════
BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BTZ_PARAMS", createParameterLayout())
{
    BTZDsp::enableFlushToZero();

    // Store initial state in both undo and A/B
    juce::MemoryBlock initialState;
    {
        juce::MemoryOutputStream stream(initialState, false);
        apvts.state.writeToStream(stream);
    }
    undoStack.push(initialState, "Initial state");
    abState.storeA(initialState);
    abState.storeB(initialState);
}

// ═══════════════════════════════════════════════════════════════════════════
// prepareToPlay
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    maxPreparedBlockSize = samplesPerBlock;
    currentBlockSize = samplesPerBlock;

    // Oversampling (JUCE built-in, lazy rebuild)
    const bool needsOSRebuild = (sampleRate != lastPreparedSR ||
                                  samplesPerBlock != lastPreparedBlockSize);
    if (needsOSRebuild) {
        os2x  = std::make_unique<juce::dsp::Oversampling<float>>(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
        os4x  = std::make_unique<juce::dsp::Oversampling<float>>(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
        os8x  = std::make_unique<juce::dsp::Oversampling<float>>(2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
        os16x = std::make_unique<juce::dsp::Oversampling<float>>(2, 4, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);

        os2x->initProcessing((size_t)samplesPerBlock);
        os4x->initProcessing((size_t)samplesPerBlock);
        os8x->initProcessing((size_t)samplesPerBlock);
        os16x->initProcessing((size_t)samplesPerBlock);

        lastPreparedSR = sampleRate;
        lastPreparedBlockSize = samplesPerBlock;
    }

    dryBuffer.setSize(2, samplesPerBlock, false, true, true);

    // Core DSP modules
    safetyPre.setSampleRate(sampleRate);
    safetyPost.setSampleRate(sampleRate);

    peakEnvL.setTimes(0.0f, 300.0f, sampleRate);
    peakEnvR.setTimes(0.0f, 300.0f, sampleRate);
    rmsEnvL.setTimes(5.0f, 50.0f, sampleRate);
    rmsEnvR.setTimes(5.0f, 50.0f, sampleRate);
    glueEnv.setTimes(10.0f, 100.0f, sampleRate);

    glueComp.prepare(sampleRate);
    crossover.prepare(sampleRate, 250.0f);
    truePeakLimiter.prepare(sampleRate);
    shineProcessor.prepare(sampleRate);
    autoGainSmoother.prepare(sampleRate);
    bypassCrossfader.prepare();

    // Multiband — prepare with default 1 band
    const float defaultFreqs[] = { 250.0f, 1000.0f, 4000.0f, 8000.0f, 12000.0f };
    multibandEngine.prepare(sampleRate, 1, defaultFreqs);

    // Loudness
    loudnessMeter.prepare(sampleRate);
    loudnessMatchedAB.prepare(sampleRate);

    // LFOs — default 1 Hz
    for (auto& lfo : lfoModSources)
        lfo.prepare(sampleRate, 1.0f);

    // v1.0.1: Target Lock
    targetLockEngine.prepare(sampleRate);
    targetLockXO1.prepare(sampleRate, 200.0f);   // Low/Mid split at 200Hz
    targetLockXO2.prepare(sampleRate, 4000.0f);  // Mid/High split at 4kHz

    // v10 modules
    resonanceTamer.prepare(sampleRate);
    transientSplitter.prepare(sampleRate);
    oversamplingEngine.prepare(1);  // default: no oversampling
    toneMatcher.reset();
    wdfTube.reset();
    wdfTransformer.reset();
    inputMeterBallistics.prepare(sampleRate);
    outputMeterBallistics.prepare(sampleRate);

    // Sidechain HPF — convert mode (0-3) to frequency
    const int scHpfMode = (int)*apvts.getRawParameterValue("glueScHpf");
    static constexpr float scHpfFreqs[] = { 20.0f, 60.0f, 120.0f, 250.0f };
    glueScHpf.prepare(sampleRate, scHpfFreqs[juce::jlimit(0, 3, scHpfMode)]);
    glueScHpfSampleRate = sampleRate;

    // Side low-pass for stereo width
    sideLowCoeff = std::exp(-BTZDsp::kTwoPi * 200.0f / (float)sampleRate);

    initSmoothers(sampleRate);

    // Initialise smoothers to the current parameter values so the first blocks
    // don't ramp up from zero (which would briefly mute/mono the output).
    updateTargetsFromAPVTS();
    for (auto* s : { &sPunch, &sWarmth, &sBoom, &sGlue, &sAir, &sWidth, &sDensity,
                     &sMotion, &sEra, &sMix, &sDrive, &sMaster, &sShine, &sShineMix,
                     &sShineFreq, &sShineQ, &sMacro0, &sMacro1, &sMacro2, &sMacro3,
                     &sResonanceSens, &sResonanceDepth, &sTransientSens, &sTransientMix,
                     &sToneMatchAmount })
        s->snap();

    activeQualityMode = getRequestedQualityMode();
    // In prepareToPlay (message thread), we can call setLatencySamples directly
    {
        int latency = BTZDsp::TruePeakLimiter::kLookahead;
        switch (activeQualityMode) {
            case 1: latency += (int)os2x->getLatencyInSamples(); break;
            case 2: latency += (int)os4x->getLatencyInSamples(); break;
            case 3: latency += (int)os8x->getLatencyInSamples(); break;
            default: break;
        }
        setLatencySamples(latency);
    }
    pendingLatency.store(-1, std::memory_order_relaxed);

    prepared = true;
}

void BTZAudioProcessor::releaseResources() {}

bool BTZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

// ═══════════════════════════════════════════════════════════════════════════
// Smoother initialization
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::initSmoothers(double sampleRate) {
    auto init = [&](BTZDsp::SmoothParam& s, float ms = 20.0f) {
        s.setTime(ms, sampleRate);
    };
    init(sPunch); init(sWarmth); init(sBoom); init(sGlue);
    init(sAir); init(sWidth); init(sDensity); init(sMotion); init(sEra);
    init(sMix); init(sDrive); init(sMaster);
    init(sShine); init(sShineMix); init(sShineFreq); init(sShineQ);
    init(sMacro0); init(sMacro1); init(sMacro2); init(sMacro3);
    init(sResonanceSens); init(sResonanceDepth);
    init(sTransientSens); init(sTransientMix);
    init(sToneMatchAmount);
}

// ═══════════════════════════════════════════════════════════════════════════
// Read APVTS values and push to smoother targets
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::updateTargetsFromAPVTS() {
    sPunch.setTarget(*apvts.getRawParameterValue("punch"));
    sWarmth.setTarget(*apvts.getRawParameterValue("warmth"));
    sBoom.setTarget(*apvts.getRawParameterValue("boom"));
    sGlue.setTarget(*apvts.getRawParameterValue("glue"));
    sAir.setTarget(*apvts.getRawParameterValue("air"));
    sWidth.setTarget(*apvts.getRawParameterValue("width"));
    sDensity.setTarget(*apvts.getRawParameterValue("density"));
    sMotion.setTarget(*apvts.getRawParameterValue("motion"));
    sEra.setTarget(*apvts.getRawParameterValue("era"));
    sMix.setTarget(*apvts.getRawParameterValue("mix"));
    sDrive.setTarget(*apvts.getRawParameterValue("drive"));
    sMaster.setTarget(*apvts.getRawParameterValue("master"));
    sShine.setTarget(*apvts.getRawParameterValue("shine"));
    sShineMix.setTarget(*apvts.getRawParameterValue("shineMix"));
    sShineFreq.setTarget(*apvts.getRawParameterValue("shineFreq"));
    sShineQ.setTarget(*apvts.getRawParameterValue("shineQ"));
    sMacro0.setTarget(*apvts.getRawParameterValue("macro0"));
    sMacro1.setTarget(*apvts.getRawParameterValue("macro1"));
    sMacro2.setTarget(*apvts.getRawParameterValue("macro2"));
    sMacro3.setTarget(*apvts.getRawParameterValue("macro3"));
    sResonanceSens.setTarget(*apvts.getRawParameterValue("resSens"));
    sResonanceDepth.setTarget(*apvts.getRawParameterValue("resDepth"));
    sTransientSens.setTarget(*apvts.getRawParameterValue("transSens"));
    sTransientMix.setTarget(*apvts.getRawParameterValue("transMix"));
    sToneMatchAmount.setTarget(*apvts.getRawParameterValue("toneMatchAmt"));

    // Saturation model
    const int satModelIdx = (int)*apvts.getRawParameterValue("satModel");
    currentSatModel = static_cast<BTZDsp::SaturationModel>(
        juce::jlimit(0, (int)BTZDsp::SaturationModel::NumModels - 1, satModelIdx));

    // Mid/Side
    midSideEncoder.enabled = (bool)*apvts.getRawParameterValue("midSide");

    // Resonance tamer
    resonanceTamer.enabled = (bool)*apvts.getRawParameterValue("resEnabled");

    // Transient splitter
    transientSplitter.enabled = (bool)*apvts.getRawParameterValue("transEnabled");
    transientSplitter.sensitivity = *apvts.getRawParameterValue("transSens");  // was inert

    // Multiband count — only update numBands (no allocation/prepare on audio thread)
    const int mbCount = (int)*apvts.getRawParameterValue("multibandCount");
    const int newBandCount = juce::jlimit(1, BTZDsp::kMaxBands, mbCount + 1);
    multibandEngine.numBands = newBandCount;
}

void BTZAudioProcessor::resetAll() {
    safetyPre.reset(); safetyPost.reset();
    peakEnvL.reset(); peakEnvR.reset(); rmsEnvL.reset(); rmsEnvR.reset(); glueEnv.reset();
    glueComp.reset(); crossover.reset(); truePeakLimiter.reset();
    shineProcessor.reset(); autoGainSmoother.reset(); bypassCrossfader.reset();
    multibandEngine.reset(); loudnessMeter.reset();
    for (auto& lfo : lfoModSources) lfo.reset();
    resonanceTamer.reset(); transientSplitter.reset(); oversamplingEngine.reset();
    wdfTube.reset(); wdfTransformer.reset();
    inputMeterBallistics.reset(); outputMeterBallistics.reset();
    glueScHpf.reset();
    targetLockEngine.reset(); targetLockXO1.reset(); targetLockXO2.reset();
    if (os2x)  os2x->reset();
    if (os4x)  os4x->reset();
    if (os8x)  os8x->reset();
    if (os16x) os16x->reset();
    hpStateL = hpStateR = sideLowState = tapeStateL = tapeStateR = 0.0f;
    motionPhase = 0.0f;
}

// JUCE lifecycle hook — hosts call this to clear state (transport stop, etc.)
void BTZAudioProcessor::reset() {
    resetAll();
}

int BTZAudioProcessor::getRequestedQualityMode() const {
    return (int)*apvts.getRawParameterValue("quality");
}

void BTZAudioProcessor::updateLatencyFromQuality(int mode) {
    int latency = BTZDsp::TruePeakLimiter::kLookahead;
    switch (mode) {
        case 1: latency += (int)os2x->getLatencyInSamples(); break;
        case 2: latency += (int)os4x->getLatencyInSamples(); break;
        case 3: latency += (int)os8x->getLatencyInSamples(); break;
        default: break;
    }
    // Defer latency reporting to message thread via async callback
    pendingLatency.store(latency, std::memory_order_relaxed);
}

// ═══════════════════════════════════════════════════════════════════════════
// processBlock — main audio callback (audio thread only)
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;

    if (!prepared || buffer.getNumChannels() < 2) return;

    // Robustness: JUCE's contract guarantees numSamples <= the prepared block
    // size, but a misbehaving host could exceed it and overrun dryBuffer / the
    // oversampling buffers. Re-prepare to the larger size rather than risk an
    // out-of-bounds write (rare, off the steady state).
    if (buffer.getNumSamples() > maxPreparedBlockSize)
        prepareToPlay(currentSampleRate, buffer.getNumSamples());

    // Apply deferred latency update (setLatencySamples is safe from any thread in JUCE)
    const int pending = pendingLatency.exchange(-1, std::memory_order_relaxed);
    if (pending >= 0)
        setLatencySamples(pending);

    const int numSamples = buffer.getNumSamples();
    auto* dataL = buffer.getWritePointer(0);
    auto* dataR = buffer.getWritePointer(1);

    // MIDI learn processing
    processMIDILearn(midi);

    // Update smoother targets from parameter values
    updateTargetsFromAPVTS();

    // Advance block-rate smoothers (consumed once per block as a constant).
    // Without this their .current would never leave 0 — e.g. Mix would stay at
    // 0 (fully dry) and Width would collapse to mono. The per-sample smoothers
    // (drive/warmth/punch/density/master/resonance) advance via next() instead.
    sMix.advanceBlock(numSamples);
    sGlue.advanceBlock(numSamples);
    sShine.advanceBlock(numSamples);
    sShineFreq.advanceBlock(numSamples);
    sShineQ.advanceBlock(numSamples);
    sWidth.advanceBlock(numSamples);
    sMotion.advanceBlock(numSamples);
    sTransientMix.advanceBlock(numSamples);
    sBoom.advanceBlock(numSamples);

    // Store dry signal for wet/dry mix
    dryBuffer.copyFrom(0, 0, dataL, numSamples);
    dryBuffer.copyFrom(1, 0, dataR, numSamples);

    // Bypass check
    const bool bypassed = (bool)*apvts.getRawParameterValue("bypass");
    bypassCrossfader.setBypassState(bypassed);

    if (bypassCrossfader.isBypassed()) {
        // Still update meters even when bypassed
        updateMeters(dataL, dataR, dataL, dataR, numSamples, 0.0f);
        return;
    }

    // Quality mode → oversampling selection
    const int qualityMode = getRequestedQualityMode();
    if (qualityMode != activeQualityMode) {
        activeQualityMode = qualityMode;
        updateLatencyFromQuality(qualityMode);
    }

    // ── Signal flow ──
    processLinearPre(dataL, dataR, numSamples);

    // Oversampling wrapper
    float sparkGR = 0.0f;
    if (qualityMode == 0) {
        // No oversampling
        processNonlinear(dataL, dataR, numSamples, 1.0f);
    } else {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::Oversampling<float>* os = nullptr;
        switch (qualityMode) {
            case 1: os = os2x.get(); break;
            case 2: os = os4x.get(); break;
            case 3: os = os8x.get(); break;
            default: os = os2x.get(); break;
        }

        auto osBlock = os->processSamplesUp(block);
        auto* osL = osBlock.getChannelPointer(0);
        auto* osR = osBlock.getChannelPointer(1);
        const int osNumSamples = (int)osBlock.getNumSamples();
        const float osFactor = (float)osNumSamples / (float)numSamples;

        processNonlinear(osL, osR, osNumSamples, osFactor);

        os->processSamplesDown(block);
    }

    processLinearPost(dataL, dataR, numSamples);

    // Wet/dry mix
    const float mixVal = sMix.current;
    if (mixVal < 0.999f) {
        const float* dryL = dryBuffer.getReadPointer(0);
        const float* dryR = dryBuffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] = dryL[i] + mixVal * (dataL[i] - dryL[i]);
            dataR[i] = dryR[i] + mixVal * (dataR[i] - dryR[i]);
        }
    }

    // Master gain — smoothed as a per-block linear ramp (2 dbToGain calls per
    // block instead of one std::pow per sample; audibly identical).
    {
        const float gStart = BTZDsp::dbToGain((sMaster.current - 0.7f) * 24.0f);
        sMaster.advanceBlock(numSamples);
        const float gEnd = BTZDsp::dbToGain((sMaster.current - 0.7f) * 24.0f);
        const float gStep = (numSamples > 1) ? (gEnd - gStart) / (float)(numSamples - 1) : 0.0f;
        float g = gStart;
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] *= g;
            dataR[i] *= g;
            g += gStep;
        }
    }

    // Final safety
    for (int i = 0; i < numSamples; ++i)
        safetyPost.processStereo(dataL[i], dataR[i]);

    // Bypass crossfade
    const float* dryL = dryBuffer.getReadPointer(0);
    const float* dryR = dryBuffer.getReadPointer(1);
    for (int i = 0; i < numSamples; ++i)
        bypassCrossfader.processStereo(dryL[i], dryR[i], dataL[i], dataR[i]);

    // Delta monitoring: output = wet - dry (what BTZ is adding)
    if (deltaMonitoring.load(std::memory_order_relaxed)) {
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] = dataL[i] - dryL[i];
            dataR[i] = dataR[i] - dryR[i];
        }
    }

    // Stereo correlation measurement
    float corrSum = 0.0f, energyL = 0.0f, energyR = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        corrSum += dataL[i] * dataR[i];
        energyL += dataL[i] * dataL[i];
        energyR += dataR[i] * dataR[i];
    }
    const float denom = std::sqrt(energyL * energyR);
    const float corr = (denom > 1e-8f) ? (corrSum / denom) : 1.0f;
    meters.correlation.store(corr, std::memory_order_relaxed);

    // Update meters
    sparkGR = glueComp.lastGainReductionDb;
    updateMeters(dryBuffer.getReadPointer(0), dryBuffer.getReadPointer(1),
                 dataL, dataR, numSamples, sparkGR);
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPre — M/S encode, resonance tame, sidechain HPF, glue comp, shine
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPre(float* dataL, float* dataR, int numSamples) {
    // Input safety (DC block + NaN guard)
    for (int i = 0; i < numSamples; ++i)
        safetyPre.processStereo(dataL[i], dataR[i]);

    // Auto-gain: measure input level
    const bool autoGainOn = (bool)*apvts.getRawParameterValue("autoGain");
    if (autoGainOn) {
        float peakL = 0.0f, peakR = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            peakL = juce::jmax(peakL, std::abs(dataL[i]));
            peakR = juce::jmax(peakR, std::abs(dataR[i]));
        }
        autoGainSmoother.updateInput(peakL, peakR);
    }

    // Mid/Side encode
    for (int i = 0; i < numSamples; ++i)
        midSideEncoder.encode(dataL[i], dataR[i]);

    // Resonance taming (per-sample, mono processing on L and R independently)
    for (int i = 0; i < numSamples; ++i) {
        const float sens = sResonanceSens.next();
        const float depth = sResonanceDepth.next();
        resonanceTamer.sensitivity = sens;
        resonanceTamer.depth = depth;
        dataL[i] = resonanceTamer.process(dataL[i]);
        dataR[i] = resonanceTamer.process(dataR[i]);
    }

    // Glue compressor
    const float glueAmt = sGlue.current;
    if (glueAmt > 0.01f) {
        // Configure compressor from APVTS parameters
        const float glueRatio = *apvts.getRawParameterValue("glueRatio");
        const float glueAttack = *apvts.getRawParameterValue("glueAttack");
        const float glueRelease = *apvts.getRawParameterValue("glueRelease");
        const float glueLink = *apvts.getRawParameterValue("glueLink");
        glueComp.setParameters(-6.0f - glueAmt * 12.0f,  // threshold
                               glueRatio,                  // ratio
                               glueAmt * 3.0f);            // makeup
        glueComp.setAttackRelease(glueAttack, glueRelease);
        glueComp.setStereoLink(glueLink);

        for (int i = 0; i < numSamples; ++i) {
            // Sidechain HPF
            float scL = dataL[i], scR = dataR[i];
            glueScHpf.processStereo(scL, scR);

            // Compress
            const float gain = glueComp.processStereo(scL, scR);
            dataL[i] *= gain;
            dataR[i] *= gain;
        }
    }

    // Shine EQ
    const float shineAmt = sShine.current;
    if (shineAmt > 0.01f) {
        shineProcessor.setParameters(sShineFreq.current, shineAmt * 12.0f, sShineQ.current);
        shineProcessor.recalcCoeffs(currentSampleRate);
        for (int i = 0; i < numSamples; ++i)
            shineProcessor.processStereo(dataL[i], dataR[i]);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processNonlinear — saturation (transient-aware, model-select, multiband)
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor) {
    // When oversampling, this loop runs osFactor× more samples per host block,
    // so all per-sample modulation must advance against the oversampled rate or
    // it runs osFactor× too fast.
    const double effectiveSR = currentSampleRate * (double)juce::jmax(1.0f, osFactor);

    // Tick LFOs (if any active)
    const int lfoCount = (int)*apvts.getRawParameterValue("lfoCount");
    for (int lfoIdx = 0; lfoIdx < juce::jmin(lfoCount, BTZDsp::kMaxLFOs); ++lfoIdx)
        lfoModSources[lfoIdx].prepare(effectiveSR, 1.0f + (float)lfoIdx * 0.5f);

    // Multiband split (if >1 band)
    const bool useMultiband = multibandEngine.numBands > 1;
    float bandL[BTZDsp::kMaxBands] = {};
    float bandR[BTZDsp::kMaxBands] = {};

    for (int i = 0; i < numSamples; ++i) {
        const float drive = sDrive.next();
        const float warmth = sWarmth.next();
        const float punch = sPunch.next();
        const float density = sDensity.next();

        // Motion LFO modulation on drive
        const float motionAmt = sMotion.current;
        float driveModulated = drive;
        if (motionAmt > 0.01f) {
            motionPhase += 2.0f / (float)effectiveSR;  // 2 Hz LFO (oversampling-aware)
            if (motionPhase >= 1.0f) motionPhase -= 1.0f;
            const float lfoVal = std::sin(BTZDsp::kTwoPi * motionPhase);
            driveModulated += lfoVal * motionAmt * 0.2f;
        }

        // LFO modulation (additive on drive)
        for (int lfoIdx = 0; lfoIdx < juce::jmin(lfoCount, BTZDsp::kMaxLFOs); ++lfoIdx) {
            const float lfoOut = lfoModSources[lfoIdx].tick();
            driveModulated += lfoOut * 0.05f;  // subtle modulation
        }

        // Calculate drive gain (dB mapping: 0→0dB, 0.5→12dB, 1→30dB)
        const float driveDb = driveModulated * 30.0f;
        const float driveGain = BTZDsp::dbToGain(driveDb);

        float l = dataL[i] * driveGain;
        float r = dataR[i] * driveGain;

        // Transient-aware processing
        float transientAmt = 0.0f;
        if (transientSplitter.enabled) {
            transientAmt = transientSplitter.getTransientAmount((l + r) * 0.5f);
            // Reduce drive on transients to preserve punch
            const float transientProtect = 1.0f - transientAmt * sTransientMix.current * 0.5f;
            l *= transientProtect;
            r *= transientProtect;
        }

        // Apply saturation based on selected model
        switch (currentSatModel) {
            case BTZDsp::SaturationModel::Tanh:
            case BTZDsp::SaturationModel::Tube:
            case BTZDsp::SaturationModel::Tape:
            case BTZDsp::SaturationModel::Transistor:
            case BTZDsp::SaturationModel::Transformer: {
                const float lowContent = sBoom.current;
                l = BTZDsp::Waveshaper::process(currentSatModel, l, tapeStateL, lowContent);
                r = BTZDsp::Waveshaper::process(currentSatModel, r, tapeStateR, lowContent);
                break;
            }
            case BTZDsp::SaturationModel::Neural_Neve:
                l = neuralNeve.process(l);
                r = neuralNeve.process(r);
                break;
            case BTZDsp::SaturationModel::Neural_API:
                l = neuralAPI.process(l);
                r = neuralAPI.process(r);
                break;
            case BTZDsp::SaturationModel::Neural_SSL:
                l = neuralSSL.process(l);
                r = neuralSSL.process(r);
                break;
            case BTZDsp::SaturationModel::Neural_Custom:
                l = neuralCustom.process(l);
                r = neuralCustom.process(r);
                break;
            case BTZDsp::SaturationModel::WDF_Tube:
                l = wdfTube.process(l);
                r = wdfTube.process(r);
                break;
            case BTZDsp::SaturationModel::WDF_Transformer:
                l = wdfTransformer.process(l);
                r = wdfTransformer.process(r);
                break;
            default:
                l = BTZDsp::fastTanh(l);
                r = BTZDsp::fastTanh(r);
                break;
        }

        // Warmth: blend in low-frequency content
        if (warmth > 0.01f) {
            hpStateL += (l - hpStateL) * (1.0f - warmth * 0.3f);
            hpStateR += (r - hpStateR) * (1.0f - warmth * 0.3f);
            l = BTZDsp::lerp(l, hpStateL, warmth * 0.3f);
            r = BTZDsp::lerp(r, hpStateR, warmth * 0.3f);
        }

        // Density: parallel compression character
        if (density > 0.01f) {
            const float compressed = BTZDsp::softClip(l * (1.0f + density));
            l = BTZDsp::lerp(l, compressed, density * 0.5f);
            const float compressedR = BTZDsp::softClip(r * (1.0f + density));
            r = BTZDsp::lerp(r, compressedR, density * 0.5f);
        }

        // Normalize back from drive gain
        const float invDrive = 1.0f / juce::jmax(0.1f, driveGain);
        dataL[i] = l * invDrive;
        dataR[i] = r * invDrive;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPost — auto-gain, limiter, M/S decode, width, spectrum/GR
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPost(float* dataL, float* dataR, int numSamples) {
    // Auto-gain compensation
    const bool autoGainOn = (bool)*apvts.getRawParameterValue("autoGain");
    if (autoGainOn) {
        float peakL = 0.0f, peakR = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            peakL = juce::jmax(peakL, std::abs(dataL[i]));
            peakR = juce::jmax(peakR, std::abs(dataR[i]));
        }
        autoGainSmoother.updateOutput(peakL, peakR);
        const float compGain = autoGainSmoother.getCompensationGain();
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] *= compGain;
            dataR[i] *= compGain;
        }
    }

    // ── v1.0.1: Target Lock processing ──
    // Read target lock parameters
    targetLockEngine.lufsLocked = (bool)*apvts.getRawParameterValue("targetLUFSLock");
    targetLockEngine.rmsLocked = (bool)*apvts.getRawParameterValue("targetRMSLock");
    targetLockEngine.setLUFSTarget(*apvts.getRawParameterValue("targetLUFS"));
    targetLockEngine.setRMSTarget(*apvts.getRawParameterValue("targetRMS"));
    targetLockEngine.setDynamicsThreshold(*apvts.getRawParameterValue("targetDynThresh"));

    // Per-band locks
    targetLockEngine.bands[0].locked = (bool)*apvts.getRawParameterValue("targetLowLock");
    targetLockEngine.bands[1].locked = (bool)*apvts.getRawParameterValue("targetMidLock");
    targetLockEngine.bands[2].locked = (bool)*apvts.getRawParameterValue("targetHighLock");
    targetLockEngine.bands[0].targetDb = *apvts.getRawParameterValue("targetLowDb");
    targetLockEngine.bands[1].targetDb = *apvts.getRawParameterValue("targetMidDb");
    targetLockEngine.bands[2].targetDb = *apvts.getRawParameterValue("targetHighDb");

    if (targetLockEngine.isActive()) {
        for (int i = 0; i < numSamples; ++i) {
            // Measure per-band peaks via 3-band crossover
            float lowL, lowR, midL, midR, highL, highR;
            float tempHighL, tempHighR;
            targetLockXO1.processStereo(dataL[i], dataR[i], lowL, lowR, tempHighL, tempHighR);
            targetLockXO2.processStereo(tempHighL, tempHighR, midL, midR, highL, highR);

            const float bandPeaks[3] = {
                juce::jmax(std::abs(lowL), std::abs(lowR)),
                juce::jmax(std::abs(midL), std::abs(midR)),
                juce::jmax(std::abs(highL), std::abs(highR))
            };

            float bandGains[3] = { 1.0f, 1.0f, 1.0f };
            const float masterGain = targetLockEngine.process(
                dataL[i], dataR[i], loudnessMeter.momentary, bandPeaks, bandGains);

            // Apply per-band spectral correction via crossover split
            lowL *= bandGains[0]; lowR *= bandGains[0];
            midL *= bandGains[1]; midR *= bandGains[1];
            highL *= bandGains[2]; highR *= bandGains[2];

            // Recombine bands and apply master correction
            dataL[i] = (lowL + midL + highL) * masterGain;
            dataR[i] = (lowR + midR + highR) * masterGain;
        }
    }

    // True peak limiter (ceiling from parameter)
    truePeakLimiter.ceiling = *apvts.getRawParameterValue("ceiling");
    for (int i = 0; i < numSamples; ++i)
        truePeakLimiter.processStereo(dataL[i], dataR[i]);

    // Mid/Side decode
    for (int i = 0; i < numSamples; ++i)
        midSideEncoder.decode(dataL[i], dataR[i]);

    // Stereo width
    const float width = sWidth.current;
    if (std::abs(width - 0.5f) > 0.01f) {
        const float widthFactor = width * 2.0f;  // 0→0 (mono), 0.5→1 (normal), 1→2 (wide)
        for (int i = 0; i < numSamples; ++i) {
            const float mid = (dataL[i] + dataR[i]) * 0.5f;
            const float side = (dataL[i] - dataR[i]) * 0.5f;
            // Low-pass the side signal to prevent harsh widening
            sideLowState += (1.0f - sideLowCoeff) * (side - sideLowState);
            const float wideSide = sideLowState * widthFactor;
            dataL[i] = mid + wideSide;
            dataR[i] = mid - wideSide;
        }
    }

    // Spectrum buffer (mono sum for FFT)
    for (int i = 0; i < numSamples; ++i)
        spectrumBuffer.pushSample((dataL[i] + dataR[i]) * 0.5f);

    // GR history
    grHistory.push(glueComp.lastGainReductionDb);

    // Loudness meter
    for (int i = 0; i < numSamples; ++i)
        loudnessMeter.process(dataL[i], dataR[i]);
}

// ═══════════════════════════════════════════════════════════════════════════
// MIDI Learn
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processMIDILearn(juce::MidiBuffer& midi) {
    if (midiLearn.numMappings == 0 && !midiLearn.isLearning) return;

    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (!msg.isController()) continue;

        const int cc = msg.getControllerNumber();
        const float normValue = msg.getControllerValue() / 127.0f;

        // Learning mode: assign this CC to the target parameter
        // Note: learningParamID is set from message thread before isLearning=true.
        // We only read it here (single-writer guarantee from message thread).
        // Setting isLearning=false is the signal to message thread that learning completed.
        if (midiLearn.isLearning && midiLearn.learningParamID.isNotEmpty()) {
            midiLearn.addMapping(cc, midiLearn.learningParamID, 0.0f, 1.0f);
            midiLearn.isLearning = false;
            // Don't clear learningParamID here (String dealloc not RT-safe)
            // Message thread will clear it when it sees isLearning == false
            continue;
        }

        // Apply existing mappings
        const auto* mapping = midiLearn.findMapping(cc);
        if (mapping != nullptr) {
            const float mapped = mapping->minValue + normValue * (mapping->maxValue - mapping->minValue);
            if (auto* param = apvts.getParameter(mapping->parameterID))
                param->setValueNotifyingHost(mapped);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Metering (called at end of processBlock)
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::updateMeters(const float* inL, const float* inR,
                                      const float* outL, const float* outR,
                                      int numSamples, float grDb) {
    // Input meters
    for (int i = 0; i < numSamples; ++i)
        inputMeterBallistics.process(inL[i], inR[i]);

    // Output meters
    for (int i = 0; i < numSamples; ++i)
        outputMeterBallistics.process(outL[i], outR[i]);

    // Store atomic meter values for UI thread
    meters.inputPeakL.store(inputMeterBallistics.peakL, std::memory_order_relaxed);
    meters.inputPeakR.store(inputMeterBallistics.peakR, std::memory_order_relaxed);
    meters.outputPeakL.store(outputMeterBallistics.peakL, std::memory_order_relaxed);
    meters.outputPeakR.store(outputMeterBallistics.peakR, std::memory_order_relaxed);
    meters.grDb.store(grDb, std::memory_order_relaxed);
    meters.lufs.store(loudnessMeter.momentary, std::memory_order_relaxed);
    meters.truePeak.store(loudnessMeter.truePeak, std::memory_order_relaxed);
}

// ═══════════════════════════════════════════════════════════════════════════
// Undo / Redo
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::pushUndoState(const juce::String& description) {
    juce::MemoryBlock state;
    juce::MemoryOutputStream stream(state, false);
    apvts.state.writeToStream(stream);
    undoStack.push(state, description);
}

void BTZAudioProcessor::undo() {
    const auto* snapshot = undoStack.undo();
    if (snapshot != nullptr) {
        auto tree = juce::ValueTree::readFromData(snapshot->data.getData(), snapshot->data.getSize());
        if (tree.isValid()) apvts.replaceState(tree);
    }
}

void BTZAudioProcessor::redo() {
    const auto* snapshot = undoStack.redo();
    if (snapshot != nullptr) {
        auto tree = juce::ValueTree::readFromData(snapshot->data.getData(), snapshot->data.getSize());
        if (tree.isValid()) apvts.replaceState(tree);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// A/B Comparison
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::storeA() {
    juce::MemoryBlock state;
    juce::MemoryOutputStream stream(state, false);
    apvts.state.writeToStream(stream);
    abState.storeA(state);
}

void BTZAudioProcessor::storeB() {
    juce::MemoryBlock state;
    juce::MemoryOutputStream stream(state, false);
    apvts.state.writeToStream(stream);
    abState.storeB(state);
}

void BTZAudioProcessor::toggleAB() {
    // Store current state in the active slot before toggling
    if (abState.isA) storeA(); else storeB();
    abState.toggle();

    // Restore the other slot
    const auto* active = abState.getActive();
    if (active != nullptr) {
        auto tree = juce::ValueTree::readFromData(active->getData(), active->getSize());
        if (tree.isValid()) apvts.replaceState(tree);
    }
}

void BTZAudioProcessor::copyAtoB() { abState.copyAtoB(); }

// ═══════════════════════════════════════════════════════════════════════════
// Preset System
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::loadPreset(const juce::File& file) {
    if (!file.existsAsFile()) return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return;

    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid()) return;

    // State migration
    const int version = tree.getProperty("stateVersion", 0);
    if (version < BTZDsp::kStateVersion)
        migrateState(tree, version);

    apvts.replaceState(tree);
    currentPresetName = file.getFileNameWithoutExtension();
    pushUndoState("Load preset: " + currentPresetName);
}

void BTZAudioProcessor::loadPresetByIndex(int index) {
    auto files = getPresetFiles();
    if (files.isEmpty()) return;
    index = juce::jlimit(0, files.size() - 1, index);
    currentPresetIndex = index;
    loadPreset(files[index]);
}

void BTZAudioProcessor::savePreset(const juce::File& file) {
    auto state = apvts.copyState();
    state.setProperty("stateVersion", BTZDsp::kStateVersion, nullptr);
    state.setProperty("presetName", currentPresetName, nullptr);

    if (auto xml = state.createXml())
        xml->writeTo(file);
}

juce::Array<juce::File> BTZAudioProcessor::getPresetFiles() const {
    juce::Array<juce::File> results;
    const auto presetDir = getPresetDirectory();
    if (presetDir.isDirectory())
        presetDir.findChildFiles(results, juce::File::findFiles, true, "*.btzpreset");
    return results;
}

juce::File BTZAudioProcessor::getPresetDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("BTZ").getChildFile("Presets");
}

// ═══════════════════════════════════════════════════════════════════════════
// State Serialization
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    state.setProperty("stateVersion", BTZDsp::kStateVersion, nullptr);
    state.setProperty("presetName", currentPresetName, nullptr);

    // Serialize MIDI mappings
    juce::ValueTree midiTree("MIDIMappings");
    for (int i = 0; i < midiLearn.numMappings; ++i) {
        juce::ValueTree m("Mapping");
        m.setProperty("cc", midiLearn.mappings[i].ccNumber, nullptr);
        m.setProperty("param", midiLearn.mappings[i].parameterID, nullptr);
        m.setProperty("min", midiLearn.mappings[i].minValue, nullptr);
        m.setProperty("max", midiLearn.mappings[i].maxValue, nullptr);
        midiTree.addChild(m, -1, nullptr);
    }
    state.addChild(midiTree, -1, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr) return;

    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid()) return;

    // State migration
    const int version = tree.getProperty("stateVersion", 0);
    if (version < BTZDsp::kStateVersion)
        migrateState(tree, version);

    // Restore MIDI mappings
    auto midiTree = tree.getChildWithName("MIDIMappings");
    if (midiTree.isValid()) {
        midiLearn.clearAll();
        for (int i = 0; i < midiTree.getNumChildren(); ++i) {
            auto m = midiTree.getChild(i);
            midiLearn.addMapping(
                (int)m.getProperty("cc", -1),
                m.getProperty("param", "").toString(),
                (float)m.getProperty("min", 0.0f),
                (float)m.getProperty("max", 1.0f));
        }
        tree.removeChild(midiTree, nullptr);
    }

    currentPresetName = tree.getProperty("presetName", "").toString();
    apvts.replaceState(tree);
}

void BTZAudioProcessor::migrateState(juce::ValueTree& tree, int fromVersion) {
    // v9 → v10: add resonance, transient, tone match params with defaults
    if (fromVersion < 10) {
        if (!tree.getChildWithProperty("id", "resSens").isValid()) {
            // Parameters will use their defaults from the layout
        }
    }
    tree.setProperty("stateVersion", BTZDsp::kStateVersion, nullptr);
}

// ═══════════════════════════════════════════════════════════════════════════
// Neural Model Loading (message thread only)
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::loadNeuralModel(const juce::File& modelFile, BTZDsp::SaturationModel slot) {
    if (!modelFile.existsAsFile()) return;

    // Load weights from JSON file
    auto json = juce::JSON::parse(modelFile);
    if (!json.isObject()) return;

    auto* weightsArray = json.getProperty("weights", juce::var()).getArray();
    if (weightsArray == nullptr || weightsArray->isEmpty()) return;

    // Convert to float array
    std::vector<float> weights;
    weights.reserve((size_t)weightsArray->size());
    for (const auto& val : *weightsArray)
        weights.push_back((float)(double)val);

    // Assign to the correct neural model
    BTZDsp::NeuralSaturationModel* target = nullptr;
    switch (slot) {
        case BTZDsp::SaturationModel::Neural_Neve:   target = &neuralNeve; break;
        case BTZDsp::SaturationModel::Neural_API:    target = &neuralAPI; break;
        case BTZDsp::SaturationModel::Neural_SSL:    target = &neuralSSL; break;
        case BTZDsp::SaturationModel::Neural_Custom: target = &neuralCustom; break;
        default: return;
    }

    target->loadWeights(weights.data(), (int)weights.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// Factory
// ═══════════════════════════════════════════════════════════════════════════
juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

bool BTZAudioProcessor::hasEditor() const { return true; }
const juce::String BTZAudioProcessor::getName() const { return "Box Tone Zone"; }
bool BTZAudioProcessor::acceptsMidi() const { return true; }
bool BTZAudioProcessor::producesMidi() const { return false; }
bool BTZAudioProcessor::isMidiEffect() const { return false; }
double BTZAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BTZAudioProcessor::getNumPrograms() { return 1; }
int BTZAudioProcessor::getCurrentProgram() { return 0; }
void BTZAudioProcessor::setCurrentProgram(int) {}
const juce::String BTZAudioProcessor::getProgramName(int) { return {}; }
void BTZAudioProcessor::changeProgramName(int, const juce::String&) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}
