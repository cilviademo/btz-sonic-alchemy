/*
  Box Tone Zone (BTZ) — PluginProcessor.cpp  v9
  ────────────────────────────────────────────────────────────────────────
  v9: 5 saturation models, multiband, mid/side, undo/redo, A/B, presets,
      MIDI learn, EBU R128, spectrum, GR history, 8x/16x OS
  v8: removed ADAA, 1 Hz DC blocker, tightened limiter
  v7: click-free bypass, full reset, state migration, silence detection
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ═══════════════════════════════════════════════════════════════════════
// Parameter layout — defines all automatable parameters
// ═══════════════════════════════════════════════════════════════════════
juce::AudioProcessorValueTreeState::ParameterLayout
BTZAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pct = juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f);
    auto uni = juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f);
    auto db12 = juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f);
    auto db24 = juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f);

    // ── Core tone controls ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("punch",    "Punch",    pct, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("warmth",   "Warmth",   pct, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("boom",     "Boom",     pct, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glue",     "Glue",     pct, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("air",      "Air",      pct, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("width",    "Width",    pct, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("density",  "Density",  pct, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("motion",   "Motion",   pct, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("era",      "Era",      uni, 0.5f));

    // ── Drive / Mix / Master ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive",    "Drive",    db24, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix",      "Mix",      pct, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master",   "Master",   db12, 0.0f));

    // ── SPARK (limiter) ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling",  "Ceiling",
        juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), -0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("intensity","Intensity", pct, 50.0f));

    // ── SHINE (high-shelf EQ) ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shine",    "Shine",    db12, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineMix", "Shine Mix",pct, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineFreq","Shine Freq",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("shineQ",   "Shine Q",
        juce::NormalisableRange<float>(0.1f, 4.0f, 0.01f), 0.707f));

    // ── Macros ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro0",   "Macro A",  uni, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro1",   "Macro B",  uni, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro2",   "Macro C",  uni, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro3",   "Macro D",  uni, 0.0f));

    // ── Bypass ──
    params.push_back(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    // ── Quality (oversampling) ──
    params.push_back(std::make_unique<juce::AudioParameterChoice>("quality", "Quality",
        juce::StringArray { "Off", "2x", "4x", "8x", "16x" }, 1));

    // ── Glue SC HPF ──
    params.push_back(std::make_unique<juce::AudioParameterChoice>("glueScHpf", "Glue SC HPF",
        juce::StringArray { "Off", "60 Hz", "90 Hz", "150 Hz" }, 1));

    // ── v9: Saturation model ──
    params.push_back(std::make_unique<juce::AudioParameterChoice>("satModel", "Saturation Model",
        juce::StringArray { "Tanh", "Tube", "Tape", "Transistor", "Transformer" }, 0));

    // ── v9: Mid/Side mode ──
    params.push_back(std::make_unique<juce::AudioParameterBool>("midSide", "Mid/Side", false));

    // ── v9: Multiband count ──
    params.push_back(std::make_unique<juce::AudioParameterChoice>("multibandCount", "Multiband",
        juce::StringArray { "Off", "2 Bands", "3 Bands", "4 Bands", "5 Bands", "6 Bands" }, 0));

    return { params.begin(), params.end() };
}

// ═══════════════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════════════
BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BTZ_PARAMS", createParameterLayout())
{
    BTZDsp::enableFlushToZero();

    // Initialize A/B with default state
    abState.storeA(apvts.copyState());
    abState.storeB(apvts.copyState());

    // Push initial undo state
    undoStack.pushState(apvts.copyState(), "Initial state");
}

// ═══════════════════════════════════════════════════════════════════════
// prepareToPlay — allocate all buffers, configure DSP modules
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    maxPreparedBlockSize = samplesPerBlock;
    currentBlockSize = samplesPerBlock;

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

    // Configure DSP modules
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

    // v9: new modules
    multibandEngine.prepare(sampleRate);
    loudnessMeter.prepare(sampleRate);
    for (auto& lfo : lfoModSources) lfo.prepare(sampleRate);

    // Sidechain HPF
    const int scHpfMode = (int)*apvts.getRawParameterValue("glueScHpf");
    glueScHpf.prepare(sampleRate, scHpfMode);
    glueScHpfSampleRate = sampleRate;

    // Side low-pass for stereo width
    sideLowCoeff = std::exp(-BTZDsp::kTwoPi * 200.0f / (float)sampleRate);

    initSmoothers(sampleRate);

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);

    prepared = true;
}

void BTZAudioProcessor::releaseResources() {
    // Keep dryBuffer and OS objects alive — avoid reallocation on resume
}

bool BTZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}


// ═══════════════════════════════════════════════════════════════════════
// Smoother initialization — 20ms default for all parameters
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::initSmoothers(double sampleRate) {
    auto init = [&](BTZDsp::SmoothParam& s, float ms = 20.0f) {
        s.setTime(ms, sampleRate);
    };
    init(sPunch); init(sWarmth); init(sBoom); init(sGlue);
    init(sAir); init(sWidth); init(sDensity); init(sMotion); init(sEra);
    init(sMix); init(sDrive); init(sMaster);
    init(sShine); init(sShineMix); init(sShineFreq); init(sShineQ);
    init(sMacro0); init(sMacro1); init(sMacro2); init(sMacro3);
}

// ═══════════════════════════════════════════════════════════════════════
// Read APVTS values and push to smoother targets
// ═══════════════════════════════════════════════════════════════════════
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

    // v9: Update saturation model for multiband engine band 0
    const int satModelIdx = (int)*apvts.getRawParameterValue("satModel");
    multibandEngine.bands[0].satModel = static_cast<BTZDsp::SaturationModel>(
        juce::jlimit(0, (int)BTZDsp::SaturationModel::NumModels - 1, satModelIdx));

    // v9: Update multiband count
    const int mbCount = (int)*apvts.getRawParameterValue("multibandCount");
    multibandEngine.numBands = juce::jlimit(1, BTZDsp::kMaxBands, mbCount + 1);

    // v9: Update mid/side
    midSideEncoder.enabled = *apvts.getRawParameterValue("midSide") > 0.5f;
}

// ═══════════════════════════════════════════════════════════════════════
// Full DSP state reset — transport-safe
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::resetAll() {
    safetyPre.reset();
    safetyPost.reset();
    peakEnvL.reset(); peakEnvR.reset();
    rmsEnvL.reset();  rmsEnvR.reset();
    glueEnv.reset();
    glueScHpf.reset();
    glueComp.reset();
    crossover.reset();
    truePeakLimiter.reset();
    shineProcessor.reset();
    autoGainSmoother.reset();
    bypassCrossfader.reset();
    multibandEngine.reset();
    loudnessMeter.reset();
    spectrumBuffer.reset();
    grHistory.reset();
    for (auto& lfo : lfoModSources) lfo.reset();

    hpStateL = hpStateR = 0.0f;
    sideLowState = 0.0f;
    tapeStateL = tapeStateR = 0.0f;
    silentFrameCount = 0;
    cachedDriveGain = 1.0f;
    lastDriveDb = 0.0f;
    lastCrestRatio = 3.0f;
    motionPhase = 0.0f;
}

int BTZAudioProcessor::getRequestedQualityMode() const {
    return (int)*apvts.getRawParameterValue("quality");
}

void BTZAudioProcessor::updateLatencyFromQuality(int mode) {
    int latency = 0;
    switch (mode) {
        case 1: if (os2x)  latency = (int)os2x->getLatencyInSamples();  break;
        case 2: if (os4x)  latency = (int)os4x->getLatencyInSamples();  break;
        case 3: if (os8x)  latency = (int)os8x->getLatencyInSamples();  break;
        case 4: if (os16x) latency = (int)os16x->getLatencyInSamples(); break;
        default: break;
    }
    // Add limiter lookahead
    latency += truePeakLimiter.delaySamples;
    setLatencySamples(latency);
}


// ═══════════════════════════════════════════════════════════════════════
// processBlock — main audio processing entry point
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midiMessages) {
    if (!prepared) return;
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    // v9: Process MIDI learn
    processMIDILearn(midiMessages);

    updateTargetsFromAPVTS();

    // Check quality mode change
    const int requestedQuality = getRequestedQualityMode();
    if (requestedQuality != activeQualityMode) {
        activeQualityMode = requestedQuality;
        updateLatencyFromQuality(activeQualityMode);
    }

    // Bypass handling
    const bool bypassed = *apvts.getRawParameterValue("bypass") > 0.5f;
    bypassCrossfader.setBypassState(bypassed);

    float* dataL = buffer.getWritePointer(0);
    float* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : dataL;

    // Store dry signal for mix and bypass
    dryBuffer.copyFrom(0, 0, dataL, numSamples);
    dryBuffer.copyFrom(1, 0, dataR, numSamples);
    const float* dryReadL = dryBuffer.getReadPointer(0);
    const float* dryReadR = dryBuffer.getReadPointer(1);

    // Silence detection
    float maxAbs = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        maxAbs = juce::jmax(maxAbs, std::abs(dataL[i]), std::abs(dataR[i]));

    if (maxAbs < kSilenceThresholdProc) {
        ++silentFrameCount;
        if (silentFrameCount > kSilentFrameThreshold && !bypassCrossfader.isCrossfading()) {
            buffer.clear();
            return;
        }
    } else {
        silentFrameCount = 0;
    }

    float sparkGRDb = 0.0f;

    if (!bypassCrossfader.isBypassed() || bypassCrossfader.isCrossfading()) {
        // v9: Mid/Side encode (L/R → M/S before processing)
        for (int i = 0; i < numSamples; ++i)
            midSideEncoder.encode(dataL[i], dataR[i]);

        // ── Linear pre-processing ──
        processLinearPre(dataL, dataR, numSamples);

        // ── Nonlinear processing (with optional oversampling) ──
        switch (activeQualityMode) {
            case 0: {
                // No oversampling
                processNonlinear(dataL, dataR, numSamples, 1.0f);
                break;
            }
            case 1: {
                auto osBlock = os2x->processSamplesUp(juce::dsp::AudioBlock<float>(buffer));
                float* osL = osBlock.getChannelPointer(0);
                float* osR = osBlock.getChannelPointer(1);
                processNonlinear(osL, osR, (int)osBlock.getNumSamples(), 2.0f);
                os2x->processSamplesDown(juce::dsp::AudioBlock<float>(buffer));
                dataL = buffer.getWritePointer(0);
                dataR = buffer.getWritePointer(1);
                break;
            }
            case 2: {
                auto osBlock = os4x->processSamplesUp(juce::dsp::AudioBlock<float>(buffer));
                float* osL = osBlock.getChannelPointer(0);
                float* osR = osBlock.getChannelPointer(1);
                processNonlinear(osL, osR, (int)osBlock.getNumSamples(), 4.0f);
                os4x->processSamplesDown(juce::dsp::AudioBlock<float>(buffer));
                dataL = buffer.getWritePointer(0);
                dataR = buffer.getWritePointer(1);
                break;
            }
            case 3: {
                auto osBlock = os8x->processSamplesUp(juce::dsp::AudioBlock<float>(buffer));
                float* osL = osBlock.getChannelPointer(0);
                float* osR = osBlock.getChannelPointer(1);
                processNonlinear(osL, osR, (int)osBlock.getNumSamples(), 8.0f);
                os8x->processSamplesDown(juce::dsp::AudioBlock<float>(buffer));
                dataL = buffer.getWritePointer(0);
                dataR = buffer.getWritePointer(1);
                break;
            }
            case 4: {
                auto osBlock = os16x->processSamplesUp(juce::dsp::AudioBlock<float>(buffer));
                float* osL = osBlock.getChannelPointer(0);
                float* osR = osBlock.getChannelPointer(1);
                processNonlinear(osL, osR, (int)osBlock.getNumSamples(), 16.0f);
                os16x->processSamplesDown(juce::dsp::AudioBlock<float>(buffer));
                dataL = buffer.getWritePointer(0);
                dataR = buffer.getWritePointer(1);
                break;
            }
        }

        // ── Linear post-processing ──
        processLinearPost(dataL, dataR, numSamples);

        // SPARK limiter (ceiling updated per-block, coefficients set in prepareToPlay)
        truePeakLimiter.ceiling = *apvts.getRawParameterValue("ceiling");
        for (int i = 0; i < numSamples; ++i) {
            sparkGRDb = truePeakLimiter.processStereo(dataL[i], dataR[i]);
        }

        // v9: Mid/Side decode
        for (int i = 0; i < numSamples; ++i)
            midSideEncoder.decode(dataL[i], dataR[i]);

        // Dry/wet mix
        const float mixPct = sMix.current / 100.0f;
        if (mixPct < 0.999f) {
            const float dryGain = 1.0f - mixPct;
            for (int i = 0; i < numSamples; ++i) {
                dataL[i] = dryReadL[i] * dryGain + dataL[i] * mixPct;
                dataR[i] = dryReadR[i] * dryGain + dataR[i] * mixPct;
            }
        }

        // Master gain
        const float masterGain = std::pow(10.0f, sMaster.current / 20.0f);
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] *= masterGain;
            dataR[i] *= masterGain;
        }

        // Post safety
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] = safetyPost.processSample(dataL[i], safetyPost.dcL, safetyPost.dcPrevL);
            dataR[i] = safetyPost.processSample(dataR[i], safetyPost.dcR, safetyPost.dcPrevR);
        }

        // Bypass crossfade
        for (int i = 0; i < numSamples; ++i) {
            bypassCrossfader.processStereo(dryReadL[i], dryReadR[i], dataL[i], dataR[i]);
        }

    } else {
        // Fully bypassed — output dry
        buffer.copyFrom(0, 0, dryReadL, numSamples);
        buffer.copyFrom(1, 0, dryReadR, numSamples);
        // Fully bypassed — no metering update needed
    }

    // v9: Feed spectrum buffer and loudness meter
    for (int i = 0; i < numSamples; ++i) {
        const float inMono  = (dryReadL[i] + dryReadR[i]) * 0.5f;
        const float outMono = (dataL[i] + dataR[i]) * 0.5f;
        spectrumBuffer.pushSample(inMono, outMono);
        loudnessMeter.processSample(dataL[i], dataR[i]);
    }

    updateMeters(dryReadL, dryReadR, dataL, dataR, numSamples, sparkGRDb);
}

// ═══════════════════════════════════════════════════════════════════════
// processLinearPre — DC blocking, sidechain HPF, glue compression
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPre(float* dataL, float* dataR, int numSamples) {
    // Pre-safety (DC block + NaN guard)
    for (int i = 0; i < numSamples; ++i) {
        dataL[i] = safetyPre.processSample(dataL[i], safetyPre.dcL, safetyPre.dcPrevL);
        dataR[i] = safetyPre.processSample(dataR[i], safetyPre.dcR, safetyPre.dcPrevR);
    }

    // Glue compression
    const float glueAmt = sGlue.next() / 100.0f;
    if (glueAmt > 0.001f) {
        // Update sidechain HPF
        const int scHpfMode = (int)*apvts.getRawParameterValue("glueScHpf");
        glueScHpf.prepare(currentSampleRate, scHpfMode);

        for (int i = 0; i < numSamples; ++i) {
            float scL = dataL[i], scR = dataR[i];
            glueScHpf.processStereo(scL, scR);
            const float gain = glueComp.processStereo(scL, scR);
            const float blendedGain = 1.0f + (gain - 1.0f) * glueAmt;
            dataL[i] *= blendedGain;
            dataR[i] *= blendedGain;
        }
    }

    // Auto gain: measure input RMS before saturation
    for (int i = 0; i < numSamples; ++i) {
        autoGainSmoother.updateInput(std::abs(dataL[i]), std::abs(dataR[i]));
    }

    // SHINE EQ
    const float shineGain = sShine.next();
    if (std::abs(shineGain) > 0.01f) {
        shineProcessor.freq = sShineFreq.next();
        shineProcessor.gainDb = shineGain;
        shineProcessor.q = sShineQ.next();
        shineProcessor.prepare(currentSampleRate);
        const float shineMixAmt = sShineMix.next() / 100.0f;
        for (int i = 0; i < numSamples; ++i) {
            float origL = dataL[i], origR = dataR[i];
            shineProcessor.processStereo(dataL[i], dataR[i]);
            dataL[i] = origL + (dataL[i] - origL) * shineMixAmt;
            dataR[i] = origR + (dataR[i] - origR) * shineMixAmt;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════
// processNonlinear — saturation stages (runs at oversampled rate)
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processNonlinear(float* dataL, float* dataR,
                                          int numSamples, float osFactor) {
    // Read smoothed parameters (all advanced once per block)
    const float punch   = sPunch.next() / 100.0f;
    const float warmth  = sWarmth.next() / 100.0f;
    const float density = sDensity.next() / 100.0f;
    const float driveDb = sDrive.next();
    const float era     = sEra.next();
    const float boom    = sBoom.next() / 100.0f;
    const float air     = sAir.next() / 100.0f;
    const float width   = sWidth.next() / 100.0f;
    const float motion  = sMotion.next() / 100.0f;

    // Cache drive gain (avoid per-sample pow)
    if (std::abs(driveDb - lastDriveDb) > 0.01f) {
        cachedDriveGain = std::pow(10.0f, driveDb / 20.0f);
        lastDriveDb = driveDb;
    }

    // Get saturation model
    const int satModelIdx = (int)*apvts.getRawParameterValue("satModel");
    const auto satModel = static_cast<BTZDsp::SaturationModel>(
        juce::jlimit(0, (int)BTZDsp::SaturationModel::NumModels - 1, satModelIdx));

    // Motion: modulates drive depth via a slow LFO-like wobble
    const float motionMod = motion > 0.001f ? (1.0f + motion * 0.2f * std::sin(BTZDsp::kTwoPi * motionPhase)) : 1.0f;
    if (motion > 0.001f) {
        motionPhase += motion * 0.5f * (float)numSamples / (float)juce::jmax(1.0, currentSampleRate);
        if (motionPhase >= 1.0f) motionPhase -= 1.0f;
    }
    const float effectiveDriveGain = cachedDriveGain * motionMod;

    // Multiband path: when numBands > 1, route through MultibandEngine
    if (multibandEngine.numBands > 1) {
        for (int i = 0; i < numSamples; ++i) {
            float l = dataL[i] * effectiveDriveGain;
            float r = dataR[i] * effectiveDriveGain;
            multibandEngine.processStereo(l, r, boom);

            // Era tilt
            const float vintageCoeff = 0.85f + era * 0.1f;
            hpStateL = vintageCoeff * hpStateL + (1.0f - vintageCoeff) * l;
            hpStateR = vintageCoeff * hpStateR + (1.0f - vintageCoeff) * r;
            l = hpStateL + (l - hpStateL) * (0.5f + era * 0.5f);
            r = hpStateR + (r - hpStateR) * (0.5f + era * 0.5f);

            // Width
            const float mid  = (l + r) * 0.5f;
            const float side = (l - r) * 0.5f;
            l = mid + side * width * 2.0f;
            r = mid - side * width * 2.0f;

            const float invDrive = 1.0f / juce::jmax(0.001f, effectiveDriveGain);
            dataL[i] = l * invDrive;
            dataR[i] = r * invDrive;
        }
        return;
    }

    // Fullband path (numBands == 1)
    for (int i = 0; i < numSamples; ++i) {
        float l = dataL[i] * effectiveDriveGain;
        float r = dataR[i] * effectiveDriveGain;

        // Boom: crossover-based low-end enhancement
        if (boom > 0.001f) {
            float lowL, lowR, highL, highR;
            crossover.processStereo(l, r, lowL, lowR, highL, highR);
            lowL *= (1.0f + boom * 0.5f);
            lowR *= (1.0f + boom * 0.5f);
            l = lowL + highL;
            r = lowR + highR;
        }

        // Punch: bias-compensated saturation (even harmonics)
        if (punch > 0.001f) {
            const float punchDrive = 1.0f + punch * 2.0f;
            const float pL = BTZDsp::fastTanh(l * punchDrive + 0.25f) - BTZDsp::kTanhBias025;
            const float pR = BTZDsp::fastTanh(r * punchDrive + 0.25f) - BTZDsp::kTanhBias025;
            l += (pL - l) * punch;
            r += (pR - r) * punch;
        }

        // Warmth: main saturation stage using selected model
        if (warmth > 0.001f) {
            const float warmDrive = 1.0f + warmth * 1.5f;
            const float wL = BTZDsp::Waveshaper::process(satModel, l * warmDrive, tapeStateL, boom);
            const float wR = BTZDsp::Waveshaper::process(satModel, r * warmDrive, tapeStateR, boom);
            l += (wL - l) * warmth;
            r += (wR - r) * warmth;
        }

        // Density: additional harmonic density
        if (density > 0.001f) {
            const float dDrive = 1.0f + density * 3.0f;
            const float dL = BTZDsp::Waveshaper::process(satModel, l * dDrive, tapeStateL, boom);
            const float dR = BTZDsp::Waveshaper::process(satModel, r * dDrive, tapeStateR, boom);
            l += (dL - l) * density * 0.5f;
            r += (dR - r) * density * 0.5f;
        }

        // Era: vintage (darker) vs modern (brighter) character
        {
            const float vintageCoeff = 0.85f + era * 0.1f;
            hpStateL = vintageCoeff * hpStateL + (1.0f - vintageCoeff) * l;
            hpStateR = vintageCoeff * hpStateR + (1.0f - vintageCoeff) * r;
            const float bright = l - hpStateL;
            const float brightR = r - hpStateR;
            l = hpStateL + bright * (0.5f + era * 0.5f);
            r = hpStateR + brightR * (0.5f + era * 0.5f);
        }

        // Air: high-frequency presence boost
        if (air > 0.001f) {
            l += air * 0.3f * (l - hpStateL);
            r += air * 0.3f * (r - hpStateR);
        }

        // Stereo width
        {
            const float mid  = (l + r) * 0.5f;
            const float side = (l - r) * 0.5f;
            l = mid + side * width * 2.0f;
            r = mid - side * width * 2.0f;
        }

        // Undo drive gain to maintain unity
        const float invDrive = 1.0f / juce::jmax(0.001f, effectiveDriveGain);
        dataL[i] = l * invDrive;
        dataR[i] = r * invDrive;
    }
}

// ═══════════════════════════════════════════════════════════════════════
// processLinearPost — auto gain, final safety
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPost(float* dataL, float* dataR, int numSamples) {
    // Auto gain compensation: measure output RMS and apply correction
    for (int i = 0; i < numSamples; ++i) {
        autoGainSmoother.updateOutput(std::abs(dataL[i]), std::abs(dataR[i]));
    }
    const float compGain = autoGainSmoother.getCompensationGain();
    if (std::abs(compGain - 1.0f) > 0.001f) {
        for (int i = 0; i < numSamples; ++i) {
            dataL[i] *= compGain;
            dataR[i] *= compGain;
        }
    }
}


// ═══════════════════════════════════════════════════════════════════════
// v9: MIDI Learn processing
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processMIDILearn(juce::MidiBuffer& midi) {
    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (!msg.isController()) continue;

        const int cc = msg.getControllerNumber();
        const float ccValue = msg.getControllerValue() / 127.0f;

        // Learning mode: capture CC and assign
        if (midiLearn.isLearning && !midiLearn.learningParameterID.isEmpty()) {
            midiLearn.addMapping(cc, midiLearn.learningParameterID);
            midiLearn.stopLearning();
        }

        // Apply existing mappings
        const auto* mapping = midiLearn.findMapping(cc);
        if (mapping && !mapping->parameterID.isEmpty()) {
            if (auto* param = apvts.getParameter(mapping->parameterID)) {
                const float mapped = mapping->minValue +
                    (mapping->maxValue - mapping->minValue) * ccValue;
                param->setValueNotifyingHost(param->convertTo0to1(mapped));
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════
// Meter update — called at end of processBlock
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::updateMeters(const float* inL, const float* inR,
                                      const float* outL, const float* outR,
                                      int n, float sparkGRDb) {
    float inPkL = 0.0f, inPkR = 0.0f, outPkL = 0.0f, outPkR = 0.0f;
    float inSqL = 0.0f, inSqR = 0.0f, outSqL = 0.0f, outSqR = 0.0f;
    bool inClip = false, outClip = false;

    for (int i = 0; i < n; ++i) {
        const float aiL = std::abs(inL[i]),  aiR = std::abs(inR[i]);
        const float aoL = std::abs(outL[i]), aoR = std::abs(outR[i]);

        inPkL = juce::jmax(inPkL, aiL);  inPkR = juce::jmax(inPkR, aiR);
        outPkL = juce::jmax(outPkL, aoL); outPkR = juce::jmax(outPkR, aoR);

        inSqL += inL[i] * inL[i];   inSqR += inR[i] * inR[i];
        outSqL += outL[i] * outL[i]; outSqR += outR[i] * outR[i];

        if (aiL >= 1.0f || aiR >= 1.0f) inClip = true;
        if (aoL >= 1.0f || aoR >= 1.0f) outClip = true;
    }

    const float nf = juce::jmax(1.0f, (float)n);
    auto toDb = [](float x) { return x > 1.0e-8f ? 20.0f * std::log10(x) : -100.0f; };

    meters.inputPeakL.store(toDb(inPkL));
    meters.inputPeakR.store(toDb(inPkR));
    meters.outputPeakL.store(toDb(outPkL));
    meters.outputPeakR.store(toDb(outPkR));
    meters.inputRmsL.store(toDb(std::sqrt(inSqL / nf)));
    meters.inputRmsR.store(toDb(std::sqrt(inSqR / nf)));
    meters.outputRmsL.store(toDb(std::sqrt(outSqL / nf)));
    meters.outputRmsR.store(toDb(std::sqrt(outSqR / nf)));
    meters.sparkGainReductionDb.store(sparkGRDb);
    meters.inputClip.store(inClip ? 1.0f : meters.inputClip.load() * 0.95f);
    meters.outputClip.store(outClip ? 1.0f : meters.outputClip.load() * 0.95f);

    // v9: EBU R128 LUFS
    meters.lufs.store(loudnessMeter.momentaryLufs);
    meters.lufsShortTerm.store(loudnessMeter.shortTermLufs);
    meters.lufsIntegrated.store(loudnessMeter.integratedLufs);

    // Stereo correlation
    float corrSum = 0.0f, lSum = 0.0f, rSum = 0.0f;
    for (int i = 0; i < n; ++i) {
        corrSum += outL[i] * outR[i];
        lSum += outL[i] * outL[i];
        rSum += outR[i] * outR[i];
    }
    const float denom = std::sqrt(lSum * rSum);
    meters.correlation.store(denom > 1.0e-8f ? corrSum / denom : 1.0f);

    // GR history
    grHistory.pushGR(sparkGRDb);
}

// ═══════════════════════════════════════════════════════════════════════
// v9: Undo/Redo
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::pushUndoState(const juce::String& description) {
    undoStack.pushState(apvts.copyState(), description);
}

bool BTZAudioProcessor::canUndo() const { return undoStack.canUndo(); }
bool BTZAudioProcessor::canRedo() const { return undoStack.canRedo(); }

void BTZAudioProcessor::undo() {
    auto state = undoStack.undo();
    if (state.isValid()) {
        apvts.replaceState(state);
        initSmoothers(currentSampleRate);
    }
}

void BTZAudioProcessor::redo() {
    auto state = undoStack.redo();
    if (state.isValid()) {
        apvts.replaceState(state);
        initSmoothers(currentSampleRate);
    }
}

// ═══════════════════════════════════════════════════════════════════════
// v9: A/B Comparison
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::storeToSlotA() { abState.storeA(apvts.copyState()); }
void BTZAudioProcessor::storeToSlotB() { abState.storeB(apvts.copyState()); }

void BTZAudioProcessor::toggleAB() {
    // Store current to active slot before switching
    if (abState.isSlotA)
        abState.storeA(apvts.copyState());
    else
        abState.storeB(apvts.copyState());

    abState.toggle();
    auto state = abState.getActive();
    if (state.isValid()) {
        apvts.replaceState(state);
        initSmoothers(currentSampleRate);
    }
}

bool BTZAudioProcessor::isSlotA() const { return abState.isSlotA; }
void BTZAudioProcessor::copyAtoB() { abState.copyAtoB(); }

// ═══════════════════════════════════════════════════════════════════════
// v9: Preset System
// ═══════════════════════════════════════════════════════════════════════
juce::File BTZAudioProcessor::getPresetsDirectory() const {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("BTZ Sonic Alchemy")
                   .getChildFile("Presets");
    dir.createDirectory();
    return dir;
}

bool BTZAudioProcessor::loadPreset(const juce::File& file) {
    if (!file.existsAsFile()) return false;

    auto xml = juce::XmlDocument::parse(file);
    if (!xml || !xml->hasTagName("BTZPreset")) return false;

    auto* paramsXml = xml->getChildByName("Parameters");
    if (!paramsXml) return false;

    auto newState = juce::ValueTree::fromXml(*paramsXml);
    if (!newState.isValid()) return false;

    pushUndoState("Load preset: " + file.getFileNameWithoutExtension());
    apvts.replaceState(newState);
    initSmoothers(currentSampleRate);
    resetAll();

    currentPresetName = xml->getStringAttribute("name", file.getFileNameWithoutExtension());
    return true;
}

bool BTZAudioProcessor::savePreset(const juce::File& file, const juce::String& name,
                                    const juce::String& category) {
    auto xml = std::make_unique<juce::XmlElement>("BTZPreset");
    xml->setAttribute("name", name);
    xml->setAttribute("category", category);
    xml->setAttribute("version", BTZDsp::kStateVersion);

    auto state = apvts.copyState();
    auto* paramsXml = state.createXml().release();
    if (paramsXml) {
        xml->addChildElement(paramsXml);
    }

    return xml->writeTo(file);
}

juce::Array<BTZDsp::PresetInfo> BTZAudioProcessor::scanPresets() const {
    juce::Array<BTZDsp::PresetInfo> presets;
    auto dir = getPresetsDirectory();

    for (const auto& entry : juce::RangedDirectoryIterator(dir, true, "*.btzpreset")) {
        auto file = entry.getFile();
        auto xml = juce::XmlDocument::parse(file);
        if (xml && xml->hasTagName("BTZPreset")) {
            BTZDsp::PresetInfo info;
            info.name = xml->getStringAttribute("name", file.getFileNameWithoutExtension());
            info.category = xml->getStringAttribute("category", "Uncategorized");
            info.file = file;
            info.isFactory = false;
            presets.add(info);
        }
    }
    return presets;
}

// ═══════════════════════════════════════════════════════════════════════
// Program interface (preset navigation)
// ═══════════════════════════════════════════════════════════════════════
int BTZAudioProcessor::getNumPrograms() {
    auto presets = scanPresets();
    return juce::jmax(1, presets.size());
}

int BTZAudioProcessor::getCurrentProgram() { return currentPresetIndex; }

void BTZAudioProcessor::setCurrentProgram(int index) {
    auto presets = scanPresets();
    if (index >= 0 && index < presets.size()) {
        loadPreset(presets[index].file);
        currentPresetIndex = index;
    }
}

const juce::String BTZAudioProcessor::getProgramName(int index) {
    auto presets = scanPresets();
    if (index >= 0 && index < presets.size())
        return presets[index].name;
    return {};
}

void BTZAudioProcessor::changeProgramName(int, const juce::String&) {}

// ═══════════════════════════════════════════════════════════════════════
// State serialization
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    const auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml) {
        xml->setAttribute("btzStateVersion", BTZDsp::kStateVersion);

        // v9: Save MIDI learn mappings
        auto* midiXml = xml->createNewChildElement("MIDILearn");
        for (int i = 0; i < midiLearn.numMappings; ++i) {
            auto* m = midiXml->createNewChildElement("Mapping");
            m->setAttribute("cc", midiLearn.mappings[(size_t)i].ccNumber);
            m->setAttribute("param", midiLearn.mappings[(size_t)i].parameterID);
            m->setAttribute("min", (double)midiLearn.mappings[(size_t)i].minValue);
            m->setAttribute("max", (double)midiLearn.mappings[(size_t)i].maxValue);
        }

        copyXmlToBinary(*xml, destData);
    }
}

void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType())) {
        const int version = xml->getIntAttribute("btzStateVersion", 1);

        if (version > BTZDsp::kStateVersion) return;

        auto newState = juce::ValueTree::fromXml(*xml);

        if (version < BTZDsp::kStateVersion) {
            migrateState(newState, version);
        }

        apvts.replaceState(newState);

        // v9: Restore MIDI learn mappings
        if (auto* midiXml = xml->getChildByName("MIDILearn")) {
            midiLearn.clearAll();
            for (auto* m : midiXml->getChildIterator()) {
                if (m->hasTagName("Mapping")) {
                    midiLearn.addMapping(
                        m->getIntAttribute("cc", -1),
                        m->getStringAttribute("param"),
                        (float)m->getDoubleAttribute("min", 0.0),
                        (float)m->getDoubleAttribute("max", 1.0)
                    );
                }
            }
        }
    }

    initSmoothers(currentSampleRate);
    resetAll();
    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

// ═══════════════════════════════════════════════════════════════════════
// State migration — handle preset loading from older versions
// ═══════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::migrateState(juce::ValueTree& state, int fromVersion) {
    // v4→v5: no parameter changes
    // v5→v6: added glueScHpf (APVTS uses default)
    // v6→v7: no new parameters
    // v7→v8: removed ADAA (internal DSP change)
    // v8→v9: added satModel, midSide, multibandCount (APVTS uses defaults)
    (void)state;
    (void)fromVersion;
}

// ═══════════════════════════════════════════════════════════════════════
// Editor + factory
// ═══════════════════════════════════════════════════════════════════════
juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}
