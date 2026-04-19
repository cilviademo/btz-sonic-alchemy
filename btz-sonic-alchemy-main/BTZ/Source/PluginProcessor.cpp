/*
  Box Tone Zone (BTZ) — PluginProcessor.cpp  v2
  ────────────────────────────────────────────────────────────────────────
  v2 changes (Claude review + /dsp-engineering skill):
    • ADAATanh in ALL saturation stages (preamp, band sat, punch, density)
    • TruePeakLimiter replaces SparkLimiter — ISP-aware, block-level API
    • sparkMix REMOVED from limiter (causes overshoots)
    • sparkCeiling held per block, NOT smoothed at audio rate
    • FTZ/DAZ enabled at prepareToPlay()
    • SVF-based ShineProcessor (modulation-safe)
    • Pre-allocated sidechain buffer in TruePeakLimiter
    • Per-channel per-stage ADAA instances (no sharing)
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace BTZDsp;

// ═══════════════════════════════════════════════════════════════════════════
// Parameter Layout — grouped for DAW organization
// ═══════════════════════════════════════════════════════════════════════════
juce::AudioProcessorValueTreeState::ParameterLayout BTZAudioProcessor::createParameterLayout() {
    auto pct = [](const juce::String& id, const juce::String& name, float def) {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), def);
    };

    // ── Core group ──
    auto core = std::make_unique<juce::AudioProcessorParameterGroup>("core", "Core", "|");
    core->addChild(pct("punch",   "Punch",   0.18f));
    core->addChild(pct("warmth",  "Warmth",  0.22f));
    core->addChild(pct("boom",    "Boom",    0.10f));
    core->addChild(pct("glue",    "Glue",    0.25f));
    core->addChild(pct("air",     "Air",     0.12f));
    core->addChild(pct("width",   "Width",   0.50f));
    core->addChild(pct("density", "Density", 0.16f));
    core->addChild(pct("motion",  "Motion",  0.04f));
    core->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vintageModern", 1), "Era",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    core->addChild(pct("mix",  "Mix",  1.0f));
    core->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("drive", 1), "Drive",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f));

    // ── SPARK group ──
    // NOTE: sparkMix removed — limiter must not have dry/wet blend
    auto spark = std::make_unique<juce::AudioProcessorParameterGroup>("spark", "SPARK", "|");
    spark->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sparkCeiling", 1), "TP Ceil",
        juce::NormalisableRange<float>(-3.0f, 0.0f, 0.01f), -0.3f));

    // ── SHINE group ──
    auto shine = std::make_unique<juce::AudioProcessorParameterGroup>("shine", "SHINE", "|");
    shine->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("shineAmount", 1), "Shine",
        juce::NormalisableRange<float>(0.0f, 6.0f, 0.1f), 1.2f));
    shine->addChild(pct("shineMix", "Shine Mix", 0.30f));
    shine->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("shineFreq", 1), "Shine Freq",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 10.0f, 0.3f), 12000.0f));
    shine->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("shineQ", 1), "Shine Q",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 0.7f));

    // ── Macro group ──
    auto macros = std::make_unique<juce::AudioProcessorParameterGroup>("macros", "Macros", "|");
    macros->addChild(pct("macro0", "Macro 1", 0.5f));
    macros->addChild(pct("macro1", "Macro 2", 0.5f));
    macros->addChild(pct("macro2", "Macro 3", 0.5f));
    macros->addChild(pct("macro3", "Macro 4", 0.5f));

    // ── Global group ──
    auto global = std::make_unique<juce::AudioProcessorParameterGroup>("global", "Global", "|");
    global->addChild(pct("masterIntensity", "Master", 0.42f));
    global->addChild(pct("autogain", "AutoGain", 1.0f));
    global->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("qualityMode", 1), "Quality",
        juce::NormalisableRange<float>(0.0f, 2.0f, 1.0f), 1.0f));
    global->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stabilityMode", 1), "Character",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f));
    global->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bypass", 1), "Bypass",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    return { std::move(core), std::move(spark), std::move(shine),
             std::move(macros), std::move(global) };
}

// ═══════════════════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════════════════
BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BTZParams", createParameterLayout())
{
    macroInterpreter.setupDefaults();
}

bool BTZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

// ═══════════════════════════════════════════════════════════════════════════
// ADAA reset helper — must call in prepareToPlay and after state recall
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::resetAllADAA() {
    adaaPreampL.reset();   adaaPreampR.reset();
    adaaBandLowL.reset();  adaaBandLowR.reset();
    adaaBandHighL.reset(); adaaBandHighR.reset();
    adaaPunchOddL.reset(); adaaPunchOddR.reset();
    adaaPunchEvenL.reset();adaaPunchEvenR.reset();
    adaaDensityL.reset();  adaaDensityR.reset();
}

// ═══════════════════════════════════════════════════════════════════════════
// Smoother initialization — all times in ms, all SR-aware
// Per parameter-smoothing.md: per-sample for gain/filter/freq
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::initSmoothers(double sampleRate) {
    auto init = [sampleRate](SmoothParam& s, float ms) { s.setTime(ms, sampleRate); };
    init(sPunch, 5.0f);      init(sWarmth, 6.0f);
    init(sBoom, 8.0f);       init(sGlue, 20.0f);
    init(sAir, 6.0f);        init(sWidth, 20.0f);
    init(sDensity, 6.0f);    init(sMotion, 40.0f);
    init(sEra, 25.0f);       init(sMix, 12.0f);
    init(sDrive, 10.0f);     init(sMaster, 25.0f);
    init(sShine, 5.0f);      init(sShineMix, 5.0f);
    init(sMacro0, 15.0f);    init(sMacro1, 15.0f);
    init(sMacro2, 15.0f);    init(sMacro3, 15.0f);
    // NOTE: no smoother for sparkCeiling — held per block

    sPunch.snapTo(*apvts.getRawParameterValue("punch"));
    sWarmth.snapTo(*apvts.getRawParameterValue("warmth"));
    sBoom.snapTo(*apvts.getRawParameterValue("boom"));
    sGlue.snapTo(*apvts.getRawParameterValue("glue"));
    sAir.snapTo(*apvts.getRawParameterValue("air"));
    sWidth.snapTo(*apvts.getRawParameterValue("width"));
    sDensity.snapTo(*apvts.getRawParameterValue("density"));
    sMotion.snapTo(*apvts.getRawParameterValue("motion"));
    sEra.snapTo(*apvts.getRawParameterValue("vintageModern"));
    sMix.snapTo(*apvts.getRawParameterValue("mix"));
    sDrive.snapTo(*apvts.getRawParameterValue("drive"));
    sMaster.snapTo(*apvts.getRawParameterValue("masterIntensity"));
    sShine.snapTo(*apvts.getRawParameterValue("shineAmount"));
    sShineMix.snapTo(*apvts.getRawParameterValue("shineMix"));
    sMacro0.snapTo(*apvts.getRawParameterValue("macro0"));
    sMacro1.snapTo(*apvts.getRawParameterValue("macro1"));
    sMacro2.snapTo(*apvts.getRawParameterValue("macro2"));
    sMacro3.snapTo(*apvts.getRawParameterValue("macro3"));
}

// ═══════════════════════════════════════════════════════════════════════════
// prepareToPlay — initialize all DSP modules
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;
    maxPreparedBlockSize = juce::jmax(samplesPerBlock, 32768);

    // ── FTZ/DAZ — per dsp-engineering/realtime-safety.md ──
    BTZDsp::enableFlushToZero();

    // Safety layers
    safetyPre.setSampleRate(sampleRate);
    safetyPost.setSampleRate(sampleRate);
    safetyPre.reset();
    safetyPost.reset();

    // Slew limiters (safety net — ADAA is now primary AA)
    slewL.setSampleRate(sampleRate);
    slewR.setSampleRate(sampleRate);
    slewL.reset();
    slewR.reset();

    // Envelope followers
    peakEnvL.setTimes(0.2f, 220.0f, sampleRate);
    peakEnvR.setTimes(0.2f, 220.0f, sampleRate);
    rmsEnvL.setTimes(25.0f, 300.0f, sampleRate);
    rmsEnvR.setTimes(25.0f, 300.0f, sampleRate);
    glueEnv.setTimes(5.0f, 80.0f, sampleRate);
    glueEnv.reset();

    // Glue compressor
    glueComp.prepare(sampleRate);
    glueComp.reset();

    // Crossover (LR2 at 250 Hz)
    crossover.prepare(sampleRate, 250.0f);
    crossover.reset();

    // TruePeakLimiter (replaces SparkLimiter)
    // 2 ms lookahead, pre-allocates sidechain buffer and ISP oversampler
    truePeakLimiter.prepare(sampleRate, samplesPerBlock, 2.0f);
    truePeakLimiter.reset();

    // SHINE processor (now SVF-based)
    shineProcessor.prepare(sampleRate);
    shineProcessor.reset();

    // Auto-gain smoother
    autoGainSmoother.prepare(sampleRate);
    autoGainSmoother.reset();

    // Meter ballistics
    meterBallistics.prepare(sampleRate, samplesPerBlock);
    meterBallistics.reset();

    // Width state
    hpStateL = hpStateR = 0.0f;
    sideLowState = 0.0f;
    const float sideOmega = 6.2831853f * 120.0f / (float)sampleRate;
    sideLowCoeff = sideOmega / (1.0f + sideOmega);

    noiseSeed = 12345u;

    // ADAA saturators — reset all instances
    resetAllADAA();

    // Smoothers
    initSmoothers(sampleRate);

    // Dry buffer
    dryBuffer.setSize(2, maxPreparedBlockSize, false, false, true);
    dryBuffer.clear();

    // Oversampling (for nonlinear stages — ADAA reduces the need, but OS still helps)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax(1, samplesPerBlock);
    spec.numChannels = 2;

    os2x = std::make_unique<juce::dsp::Oversampling<float>>(
        2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    os4x = std::make_unique<juce::dsp::Oversampling<float>>(
        2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    os2x->initProcessing(spec.maximumBlockSize);
    os4x->initProcessing(spec.maximumBlockSize);
    os2x->reset();
    os4x->reset();

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

void BTZAudioProcessor::releaseResources() {
    dryBuffer.setSize(0, 0);
}

int BTZAudioProcessor::getRequestedQualityMode() const {
    const float quality = apvts.getRawParameterValue("qualityMode")->load(std::memory_order_relaxed);
    return (int) juce::jlimit(0.0f, 2.0f, quality);
}

void BTZAudioProcessor::updateLatencyFromQuality(int mode) {
    int latency = truePeakLimiter.getLatencySamples();
    if (mode == 1 && os2x != nullptr)
        latency += (int) std::ceil(os2x->getLatencyInSamples());
    else if (mode >= 2 && os4x != nullptr)
        latency += (int) std::ceil(os4x->getLatencyInSamples());

    if (getLatencySamples() != latency)
        setLatencySamples(latency);
}

// ═══════════════════════════════════════════════════════════════════════════
// updateTargetsFromAPVTS — read all parameter values into smoothers
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
    sEra.setTarget(*apvts.getRawParameterValue("vintageModern"));
    sMix.setTarget(*apvts.getRawParameterValue("mix"));
    sDrive.setTarget(*apvts.getRawParameterValue("drive"));
    sMaster.setTarget(*apvts.getRawParameterValue("masterIntensity"));
    sShine.setTarget(*apvts.getRawParameterValue("shineAmount"));
    sShineMix.setTarget(*apvts.getRawParameterValue("shineMix"));
    sMacro0.setTarget(*apvts.getRawParameterValue("macro0"));
    sMacro1.setTarget(*apvts.getRawParameterValue("macro1"));
    sMacro2.setTarget(*apvts.getRawParameterValue("macro2"));
    sMacro3.setTarget(*apvts.getRawParameterValue("macro3"));

    // Update SHINE filter coefficients from parameters
    const float shineFreq = *apvts.getRawParameterValue("shineFreq");
    const float shineQ    = *apvts.getRawParameterValue("shineQ");
    const float shineAmt  = *apvts.getRawParameterValue("shineAmount");
    shineProcessor.setParameters(shineFreq, shineAmt, shineQ);
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPre — runs at BASE sample rate, before oversampling
// Handles: safety pre, drive, master scaling, glue, width
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPre(float* dataL, float* dataR, int numSamples) {
    for (int n = 0; n < numSamples; ++n) {
        float punch   = sPunch.next();
        float warmth  = sWarmth.next();
        float boom    = sBoom.next();
        float glue    = sGlue.next();
        float air     = sAir.next();
        float width   = sWidth.next();
        float density = sDensity.next();
        float drive   = sDrive.next();
        float master  = sMaster.next();
        float era     = sEra.next();
        (void)sDensity.current;
        (void)sMotion.next();

        float L = dataL[n];
        float R = dataR[n];

        // ── Safety Pre ──
        L = safetyPre.processSample(L, safetyPre.dcL, safetyPre.dcPrevL);
        R = safetyPre.processSample(R, safetyPre.dcR, safetyPre.dcPrevR);

        // ── Drive (input gain) ──
        if (drive > 0.0f) {
            const float inGain = std::pow(10.0f, drive / 20.0f);
            L *= inGain;
            R *= inGain;
        }

        // ── Master intensity scaling ──
        const float masterScale = juce::jlimit(0.25f, 1.25f, 0.7f + master * 0.6f);
        punch   *= masterScale;
        warmth  *= masterScale;
        boom    *= masterScale;
        glue    *= masterScale;
        air     *= masterScale;
        density *= masterScale;

        // ── Glue compressor (linear, SR-dependent) ──
        {
            const float sidechain = juce::jmax(std::abs(L), std::abs(R));
            const float envVal = glueEnv.process(sidechain);
            glueComp.processStereo(L, R, glue, envVal);
        }

        // ── Width (M/S with mono-safe low-band) ──
        {
            const float mid  = 0.5f * (L + R);
            const float side = 0.5f * (L - R);
            const float widthScale = width * 2.0f;

            sideLowState += sideLowCoeff * (side - sideLowState);
            const float sideLow  = sideLowState;
            const float sideHigh = side - sideLow;
            const float lowBandWidth = juce::jmin(widthScale, 1.0f);
            const float sideOut = sideLow * lowBandWidth + sideHigh * widthScale;

            L = mid + sideOut;
            R = mid - sideOut;
        }

        dataL[n] = L;
        dataR[n] = R;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processNonlinear — runs at OVERSAMPLED rate (or 1x in Eco mode)
// ────────────────────────────────────────────────────────────────────────
// ALL saturation stages now use ADAATanh instead of fastTanh.
// Per Claude review: "one ADAA instance per channel per saturation stage"
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor) {
    (void) osFactor;

    for (int n = 0; n < numSamples; ++n) {
        const float warmth  = sWarmth.current;
        const float boom    = sBoom.current;
        const float punch   = sPunch.current;
        const float density = sDensity.current;
        const float era     = sEra.current;

        float L = dataL[n];
        float R = dataR[n];

        // ── Preamp / Color (warmth-driven ADAA tanh saturation) ──
        if (warmth > 0.001f) {
            const float drv = 1.0f + warmth * 2.8f;
            const float bias = warmth * 0.05f;
            const float eraScale = juce::jmax(0.55f, 1.0f + era * 0.30f);

            // ADAA tanh: drive the input, process through ADAA, remove bias
            const float biasComp = fastTanh(bias * drv / eraScale);
            float yL = adaaPreampL.process((L + bias) * drv / eraScale) - biasComp;
            float yR = adaaPreampR.process((R + bias) * drv / eraScale) - biasComp;
            L = L + (yL - L) * warmth;
            R = R + (yR - R) * warmth;
        }

        // ── Slew limiter (safety net — ADAA is primary AA now) ──
        L = slewL.process(L);
        R = slewR.process(R);

        // ── Band split (LR2 crossover at 250 Hz) ──
        float lowL, lowR, highL, highR;
        crossover.process(L, R, lowL, lowR, highL, highR);

        // ── Band saturation (ADAA tanh per band per channel) ──
        {
            const float lowDrv  = 1.0f + boom * 1.25f;
            const float highDrv = 1.0f + warmth * 1.75f;
            const float satAmt  = juce::jlimit(0.0f, 1.0f, warmth * 0.65f + density * 0.35f);

            if (satAmt > 0.001f) {
                const float satLowL = adaaBandLowL.process(lowL * lowDrv) / lowDrv;
                const float satLowR = adaaBandLowR.process(lowR * lowDrv) / lowDrv;
                const float satHiL  = adaaBandHighL.process(highL * highDrv) / highDrv;
                const float satHiR  = adaaBandHighR.process(highR * highDrv) / highDrv;

                L = lowL + (satLowL - lowL) * satAmt + highL + (satHiL - highL) * satAmt;
                R = lowR + (satLowR - lowR) * satAmt + highR + (satHiR - highR) * satAmt;
            } else {
                L = lowL + highL;
                R = lowR + highR;
            }
        }

        // ── Punch (crest-aware harmonic injection via ADAA) ──
        if (punch > 0.002f) {
            const float peakL = peakEnvL.process(std::abs(L));
            const float rmsL  = std::sqrt(rmsEnvL.process(L * L) + 1.0e-12f);
            const float crest = peakL / juce::jmax(1.0e-5f, rmsL);
            const float harmonicBias = juce::jlimit(0.8f, 1.3f, 1.0f + (crest - 3.0f) * 0.06f);
            const float amount = punch * 0.25f;
            const float drv = 1.0f + punch * 2.0f;

            const float oddL  = adaaPunchOddL.process(drv * L);
            const float evenL = adaaPunchEvenL.process(drv * L + 0.25f) - fastTanh(0.25f);
            const float oddR  = adaaPunchOddR.process(drv * R);
            const float evenR = adaaPunchEvenR.process(drv * R + 0.25f) - fastTanh(0.25f);
            L = L + ((oddL * harmonicBias + evenL * (2.0f - harmonicBias)) - L) * amount;
            R = R + ((oddR * harmonicBias + evenR * (2.0f - harmonicBias)) - R) * amount;
        }

        // ── Boom (low-band additive boost) ──
        if (boom > 0.01f) {
            L += lowL * boom * 0.28f;
            R += lowR * boom * 0.28f;
        }

        // ── Density (additional ADAA tanh saturation) ──
        if (density > 0.001f) {
            const float drv = 1.0f + density * 3.0f;
            L = adaaDensityL.process(L * drv) / drv;
            R = adaaDensityR.process(R * drv) / drv;
        }

        dataL[n] = L;
        dataR[n] = R;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPost — runs at BASE sample rate, after downsampling
// Handles: SHINE, motion, safety post, neutral comp, dry/wet
// NOTE: SPARK (TruePeakLimiter) is now called at block level in processBlock
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPost(float* dataL, float* dataR, int numSamples) {
    for (int n = 0; n < numSamples; ++n) {
        const float warmth  = sWarmth.current;
        const float density = sDensity.current;
        const float boom    = sBoom.current;
        const float motion  = sMotion.current;
        const float mix     = sMix.next();
        const float shineMix = sShineMix.next();

        float L = dataL[n];
        float R = dataR[n];

        // ── SHINE (SVF high-shelf EQ — modulation-safe) ──
        if (shineMix > 0.001f) {
            float shineL = L, shineR = R;
            shineProcessor.processStereo(shineL, shineR);
            L = L + (shineL - L) * shineMix;
            R = R + (shineR - R) * shineMix;
        }

        // ── Motion (noise injection) ──
        if (motion > 0.01f) {
            noiseSeed = 1664525u * noiseSeed + 1013904223u;
            float white = (float)((noiseSeed >> 9) & 0x7FFFFF) / 8388608.0f - 0.5f;
            const float noiseLevel = 1.0e-6f * motion * 8.0f;
            L += white * noiseLevel;
            noiseSeed = 1664525u * noiseSeed + 1013904223u;
            white = (float)((noiseSeed >> 9) & 0x7FFFFF) / 8388608.0f - 0.5f;
            R += white * noiseLevel;
        }

        // ── Safety Post ──
        L = safetyPost.processSample(L, safetyPost.dcL, safetyPost.dcPrevL);
        R = safetyPost.processSample(R, safetyPost.dcR, safetyPost.dcPrevR);

        // ── Neutral compensation ──
        const float neutralComp = 1.0f / juce::jlimit(0.75f, 1.5f,
            1.0f + 0.20f * (warmth + density + boom));
        L *= neutralComp;
        R *= neutralComp;

        // ── Dry/Wet mix (at base SR — correct buffer alignment) ──
        if (mix < 0.999f && n < maxPreparedBlockSize) {
            const float dryL = dryBuffer.getSample(0, n);
            const float dryR = dryBuffer.getSample(1, n);
            L = dryL + (L - dryL) * mix;
            R = dryR + (R - dryR) * mix;
        }

        dataL[n] = L;
        dataR[n] = R;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// updateMeters — SR-dependent ballistics
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::updateMeters(const float* inL, const float* inR,
                                      const float* outL, const float* outR,
                                      int n, float sparkGRDb)
{
    float inPkL = 0.0f, inPkR = 0.0f, outPkL = 0.0f, outPkR = 0.0f;
    float inSqL = 0.0f, inSqR = 0.0f, outSqL = 0.0f, outSqR = 0.0f;
    float corrNum = 0.0f, corrDenL = 0.0f, corrDenR = 0.0f;
    float lufsSq = 0.0f;
    bool clipIn = false, clipOut = false;

    for (int i = 0; i < n; ++i) {
        const float iL = inL[i], iR = inR[i], oL = outL[i], oR = outR[i];
        inPkL  = juce::jmax(inPkL,  std::abs(iL));
        inPkR  = juce::jmax(inPkR,  std::abs(iR));
        outPkL = juce::jmax(outPkL, std::abs(oL));
        outPkR = juce::jmax(outPkR, std::abs(oR));
        inSqL  += iL * iL;  inSqR  += iR * iR;
        outSqL += oL * oL;  outSqR += oR * oR;
        corrNum  += oL * oR;
        corrDenL += oL * oL;
        corrDenR += oR * oR;
        lufsSq += oL * oL + oR * oR;
        clipIn  = clipIn  || (std::abs(iL) >= 0.999f || std::abs(iR) >= 0.999f);
        clipOut = clipOut || (std::abs(oL) >= 0.999f || std::abs(oR) >= 0.999f);
    }

    const float invN = 1.0f / juce::jmax(1, n);
    const float inRmsL  = std::sqrt(inSqL  * invN + 1.0e-20f);
    const float inRmsR  = std::sqrt(inSqR  * invN + 1.0e-20f);
    const float outRmsL = std::sqrt(outSqL * invN + 1.0e-20f);
    const float outRmsR = std::sqrt(outSqR * invN + 1.0e-20f);

    auto& mb = meterBallistics;
    mb.inPeakHoldL  = juce::jmax(inPkL,  mb.inPeakHoldL  * mb.holdDecay);
    mb.inPeakHoldR  = juce::jmax(inPkR,  mb.inPeakHoldR  * mb.holdDecay);
    mb.outPeakHoldL = juce::jmax(outPkL, mb.outPeakHoldL * mb.holdDecay);
    mb.outPeakHoldR = juce::jmax(outPkR, mb.outPeakHoldR * mb.holdDecay);
    mb.inRmsL  += mb.rmsCoeff * (inRmsL  - mb.inRmsL);
    mb.inRmsR  += mb.rmsCoeff * (inRmsR  - mb.inRmsR);
    mb.outRmsL += mb.rmsCoeff * (outRmsL - mb.outRmsL);
    mb.outRmsR += mb.rmsCoeff * (outRmsR - mb.outRmsR);
    mb.sparkGR += 0.2f * (sparkGRDb - mb.sparkGR);
    mb.clipHoldIn  = juce::jmax(clipIn  ? 1.0f : 0.0f, mb.clipHoldIn  * mb.clipDecay);
    mb.clipHoldOut = juce::jmax(clipOut ? 1.0f : 0.0f, mb.clipHoldOut * mb.clipDecay);

    const float corrDen = std::sqrt(corrDenL * corrDenR) + 1.0e-12f;
    const float correlation = juce::jlimit(-1.0f, 1.0f, corrNum / corrDen);
    const float lufsRms = std::sqrt((lufsSq * 0.5f) * invN + 1.0e-20f);

    meters.inputPeakL.store(juce::Decibels::gainToDecibels(mb.inPeakHoldL, -100.0f), std::memory_order_relaxed);
    meters.inputPeakR.store(juce::Decibels::gainToDecibels(mb.inPeakHoldR, -100.0f), std::memory_order_relaxed);
    meters.inputRmsL.store(juce::Decibels::gainToDecibels(mb.inRmsL, -100.0f), std::memory_order_relaxed);
    meters.inputRmsR.store(juce::Decibels::gainToDecibels(mb.inRmsR, -100.0f), std::memory_order_relaxed);
    meters.outputPeakL.store(juce::Decibels::gainToDecibels(mb.outPeakHoldL, -100.0f), std::memory_order_relaxed);
    meters.outputPeakR.store(juce::Decibels::gainToDecibels(mb.outPeakHoldR, -100.0f), std::memory_order_relaxed);
    meters.outputRmsL.store(juce::Decibels::gainToDecibels(mb.outRmsL, -100.0f), std::memory_order_relaxed);
    meters.outputRmsR.store(juce::Decibels::gainToDecibels(mb.outRmsR, -100.0f), std::memory_order_relaxed);
    meters.sparkGainReductionDb.store(juce::jmax(0.0f, mb.sparkGR), std::memory_order_relaxed);
    meters.lufs.store(juce::Decibels::gainToDecibels(lufsRms, -100.0f), std::memory_order_relaxed);
    meters.inputClip.store(mb.clipHoldIn, std::memory_order_relaxed);
    meters.outputClip.store(mb.clipHoldOut, std::memory_order_relaxed);
    meters.correlation.store(correlation, std::memory_order_relaxed);
}

// ═══════════════════════════════════════════════════════════════════════════
// processBlock — main entry point with isolated oversampling architecture
// ────────────────────────────────────────────────────────────────────────
// Signal chain:
//   1. processLinearPre (base SR)
//   2. processNonlinear (oversampled — ADAA in all saturation stages)
//   3. processLinearPost (base SR — SHINE, motion, safety, dry/wet)
//   4. TruePeakLimiter (block-level, ISP-aware, held ceiling)
//   5. AutoGain (block-level)
//   6. Metering
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    if (numSamples <= 0 || buffer.getNumChannels() < 2)
        return;

    // ── Store dry buffer (at base SR) ──
    const int copyCount = juce::jmin(numSamples, maxPreparedBlockSize);
    dryBuffer.copyFrom(0, 0, buffer, 0, 0, copyCount);
    dryBuffer.copyFrom(1, 0, buffer, 1, 0, copyCount);

    const float* dryReadL = dryBuffer.getReadPointer(0);
    const float* dryReadR = dryBuffer.getReadPointer(1);

    updateTargetsFromAPVTS();
    const bool bypassed   = *apvts.getRawParameterValue("bypass") > 0.5f;
    const float autoGain  = *apvts.getRawParameterValue("autogain");
    const int requestedQuality = getRequestedQualityMode();

    // ── sparkCeiling: held per block, NOT smoothed ──
    // Per Claude review: "ceiling is a held value — do NOT smooth it into the limiter"
    const float sparkCeilDb = *apvts.getRawParameterValue("sparkCeiling");
    const float sparkCeilLin = juce::Decibels::decibelsToGain(sparkCeilDb);

    if (requestedQuality != activeQualityMode) {
        activeQualityMode = requestedQuality;
        updateLatencyFromQuality(activeQualityMode);
    }

    float* dataL = buffer.getWritePointer(0);
    float* dataR = buffer.getWritePointer(1);

    float sparkGRDb = 0.0f;

    if (! bypassed) {
        // ── Phase 1: Linear pre-processing (always at base SR) ──
        processLinearPre(dataL, dataR, numSamples);

        // ── Phase 2: Nonlinear processing (oversampled if quality > 0) ──
        if (activeQualityMode == 1 && os2x != nullptr) {
            juce::dsp::AudioBlock<float> block(buffer);
            auto stereoBlock = block.getSubsetChannelBlock(0, 2).getSubBlock(0, (size_t) numSamples);
            auto upBlock = os2x->processSamplesUp(stereoBlock);
            processNonlinear(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1),
                             (int) upBlock.getNumSamples(), 2.0f);
            os2x->processSamplesDown(stereoBlock);
        } else if (activeQualityMode >= 2 && os4x != nullptr) {
            juce::dsp::AudioBlock<float> block(buffer);
            auto stereoBlock = block.getSubsetChannelBlock(0, 2).getSubBlock(0, (size_t) numSamples);
            auto upBlock = os4x->processSamplesUp(stereoBlock);
            processNonlinear(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1),
                             (int) upBlock.getNumSamples(), 4.0f);
            os4x->processSamplesDown(stereoBlock);
        } else {
            processNonlinear(dataL, dataR, numSamples, 1.0f);
        }

        // ── Phase 3: Linear post-processing (always at base SR) ──
        processLinearPost(dataL, dataR, numSamples);

        // ── Phase 4: TruePeakLimiter (block-level, ISP-aware) ──
        // No sparkMix — limiter is always fully engaged when active.
        // Ceiling is held per block (read once above, not smoothed per sample).
        truePeakLimiter.processBlock(buffer, sparkCeilLin);
        sparkGRDb = truePeakLimiter.getGainReductionDb();

        // ── Phase 5: Smoothed auto-gain ──
        if (autoGain > 0.5f) {
            autoGainSmoother.processBlock(dataL, dataR, numSamples, dryReadL, dryReadR);
        }
    }

    if (bypassed) {
        meterBallistics.sparkGR *= 0.9f;
    }

    updateMeters(dryReadL, dryReadR, dataL, dataR, numSamples, sparkGRDb);
}

// ═══════════════════════════════════════════════════════════════════════════
// State serialization
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    const auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));

    // Re-initialize smoothers and ADAA after state recall to prevent jumps
    initSmoothers(currentSampleRate);
    resetAllADAA();

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}
