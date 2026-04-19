/*
  Box Tone Zone (BTZ) — PluginProcessor.cpp
  Overhauled: modular DSP, isolated oversampling, macro system,
  SparkLimiter with lookahead, ShineProcessor high-shelf, LR2 crossover,
  SR-dependent meters, smoothed auto-gain, fixed dry/wet mix.
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
    auto spark = std::make_unique<juce::AudioProcessorParameterGroup>("spark", "SPARK", "|");
    spark->addChild(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sparkCeiling", 1), "TP Ceil",
        juce::NormalisableRange<float>(-3.0f, 0.0f, 0.01f), -0.3f));
    spark->addChild(pct("sparkMix", "Spark Mix", 1.0f));

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
// Smoother initialization — all times in ms, all SR-aware
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::initSmoothers(double sampleRate) {
    auto init = [sampleRate](SmoothParam& s, float ms) { s.setTime(ms, sampleRate); };
    init(sPunch, 5.0f);      init(sWarmth, 6.0f);
    init(sBoom, 8.0f);       init(sGlue, 20.0f);
    init(sAir, 6.0f);        init(sWidth, 20.0f);
    init(sDensity, 6.0f);    init(sMotion, 40.0f);
    init(sEra, 25.0f);       init(sMix, 12.0f);
    init(sDrive, 10.0f);     init(sMaster, 25.0f);
    init(sSparkCeil, 5.0f);  init(sSparkMix, 5.0f);
    init(sShine, 5.0f);      init(sShineMix, 5.0f);
    init(sMacro0, 15.0f);    init(sMacro1, 15.0f);
    init(sMacro2, 15.0f);    init(sMacro3, 15.0f);

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
    sSparkCeil.snapTo(*apvts.getRawParameterValue("sparkCeiling"));
    sSparkMix.snapTo(*apvts.getRawParameterValue("sparkMix"));
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

    // Safety layers
    safetyPre.setSampleRate(sampleRate);
    safetyPost.setSampleRate(sampleRate);
    safetyPre.reset();
    safetyPost.reset();

    // Slew limiters
    slewL.setSampleRate(sampleRate);
    slewR.setSampleRate(sampleRate);
    slewL.reset();
    slewR.reset();

    // Envelope followers (at base SR — they run in linear pre-processing)
    peakEnvL.setTimes(0.2f, 220.0f, sampleRate);
    peakEnvR.setTimes(0.2f, 220.0f, sampleRate);
    rmsEnvL.setTimes(25.0f, 300.0f, sampleRate);
    rmsEnvR.setTimes(25.0f, 300.0f, sampleRate);
    glueEnv.setTimes(5.0f, 80.0f, sampleRate);
    glueEnv.reset();

    // Glue compressor (SR-dependent smoothing)
    glueComp.prepare(sampleRate);
    glueComp.reset();

    // Crossover (LR2 at 250 Hz)
    crossover.prepare(sampleRate, 250.0f);
    crossover.reset();

    // SPARK limiter
    sparkLimiter.prepare(sampleRate, samplesPerBlock);
    sparkLimiter.reset();
    sparkGrEnvelope = 0.0f;

    // SHINE processor
    shineProcessor.prepare(sampleRate);
    shineProcessor.reset();

    // Auto-gain smoother
    autoGainSmoother.prepare(sampleRate);
    autoGainSmoother.reset();

    // Meter ballistics (SR-dependent)
    meterBallistics.prepare(sampleRate, samplesPerBlock);
    meterBallistics.reset();

    // Width state
    hpStateL = hpStateR = 0.0f;
    sideLowState = 0.0f;
    const float sideOmega = 6.2831853f * 120.0f / (float)sampleRate;
    sideLowCoeff = sideOmega / (1.0f + sideOmega);

    noiseSeed = 12345u;

    // Smoothers
    initSmoothers(sampleRate);

    // Dry buffer
    dryBuffer.setSize(2, maxPreparedBlockSize, false, false, true);
    dryBuffer.clear();

    // Oversampling
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
    int latency = sparkLimiter.getLatency(); // SPARK lookahead latency always present
    if (mode == 1 && os2x != nullptr)
        latency += (int) std::ceil(os2x->getLatencyInSamples());
    else if (mode >= 2 && os4x != nullptr)
        latency += (int) std::ceil(os4x->getLatencyInSamples());

    if (latency != getLatencySamples())
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
    sSparkCeil.setTarget(*apvts.getRawParameterValue("sparkCeiling"));
    sSparkMix.setTarget(*apvts.getRawParameterValue("sparkMix"));
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
// Handles: safety pre, drive, master scaling, glue, width, air (legacy HP)
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
        (void)sDensity.current; // density used in nonlinear
        (void)sMotion.next();   // motion used in nonlinear post

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
// Handles: preamp/color, slew, crossover, band saturation, punch, density
// These are the stages that generate harmonics and need anti-aliasing.
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor) {
    (void) osFactor; // Available for future per-sample SR adjustment

    for (int n = 0; n < numSamples; ++n) {
        // Read current smoothed values (smoothers tick at base rate in processLinearPre,
        // so here we just use the last smoothed value — no double-ticking)
        const float warmth  = sWarmth.current;
        const float boom    = sBoom.current;
        const float punch   = sPunch.current;
        const float density = sDensity.current;
        const float era     = sEra.current;

        float L = dataL[n];
        float R = dataR[n];

        // ── Preamp / Color (warmth-driven tanh saturation) ──
        if (warmth > 0.001f) {
            const float drv = 1.0f + warmth * 2.8f;
            const float bias = warmth * 0.05f;
            const float eraScale = juce::jmax(0.55f, 1.0f + era * 0.30f);

            auto preamp = [&](float x) {
                const float xb = x + bias;
                float y = fastTanh(xb * drv / eraScale);
                y -= fastTanh(bias * drv / eraScale);
                return x + (y - x) * warmth;
            };
            L = preamp(L);
            R = preamp(R);
        }

        // ── Slew limiter ──
        L = slewL.process(L);
        R = slewR.process(R);

        // ── Band split (LR2 crossover at 250 Hz) ──
        float lowL, lowR, highL, highR;
        crossover.process(L, R, lowL, lowR, highL, highR);

        // ── Band saturation ──
        {
            const float lowDrv  = 1.0f + boom * 1.25f;
            const float highDrv = 1.0f + warmth * 1.75f;
            const float satAmt  = juce::jlimit(0.0f, 1.0f, warmth * 0.65f + density * 0.35f);

            if (satAmt > 0.001f) {
                const float satLowL = fastTanh(lowL * lowDrv) / lowDrv;
                const float satLowR = fastTanh(lowR * lowDrv) / lowDrv;
                const float satHiL  = fastTanh(highL * highDrv) / highDrv;
                const float satHiR  = fastTanh(highR * highDrv) / highDrv;

                L = lowL + (satLowL - lowL) * satAmt + highL + (satHiL - highL) * satAmt;
                R = lowR + (satLowR - lowR) * satAmt + highR + (satHiR - highR) * satAmt;
            } else {
                L = lowL + highL;
                R = lowR + highR;
            }
        }

        // ── Punch (crest-aware harmonic injection) ──
        if (punch > 0.002f) {
            const float peakL = peakEnvL.process(std::abs(L));
            const float rmsL  = std::sqrt(rmsEnvL.process(L * L) + 1.0e-12f);
            const float crest = peakL / juce::jmax(1.0e-5f, rmsL);
            const float harmonicBias = juce::jlimit(0.8f, 1.3f, 1.0f + (crest - 3.0f) * 0.06f);
            const float amount = punch * 0.25f;
            const float drv = 1.0f + punch * 2.0f;

            const float oddL  = fastTanh(drv * L);
            const float evenL = fastTanh(drv * L + 0.25f) - fastTanh(0.25f);
            const float oddR  = fastTanh(drv * R);
            const float evenR = fastTanh(drv * R + 0.25f) - fastTanh(0.25f);
            L = L + ((oddL * harmonicBias + evenL * (2.0f - harmonicBias)) - L) * amount;
            R = R + ((oddR * harmonicBias + evenR * (2.0f - harmonicBias)) - R) * amount;
        }

        // ── Boom (low-band additive boost) ──
        if (boom > 0.01f) {
            L += lowL * boom * 0.28f;
            R += lowR * boom * 0.28f;
        }

        // ── Density (additional tanh saturation) ──
        if (density > 0.001f) {
            const float drv = 1.0f + density * 3.0f;
            L = fastTanh(L * drv) / drv;
            R = fastTanh(R * drv) / drv;
        }

        dataL[n] = L;
        dataR[n] = R;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPost — runs at BASE sample rate, after downsampling
// Handles: SHINE, SPARK, motion, safety post, neutral comp, dry/wet
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processLinearPost(float* dataL, float* dataR, int numSamples) {
    for (int n = 0; n < numSamples; ++n) {
        const float warmth  = sWarmth.current;
        const float density = sDensity.current;
        const float boom    = sBoom.current;
        const float motion  = sMotion.current;
        const float mix     = sMix.next();
        const float ceilDb  = sSparkCeil.next();
        const float sparkMix = sSparkMix.next();
        const float shineMix = sShineMix.next();

        float L = dataL[n];
        float R = dataR[n];

        // ── SHINE (proper high-shelf EQ) ──
        if (shineMix > 0.001f) {
            float shineL = L, shineR = R;
            shineProcessor.processStereo(shineL, shineR);
            L = L + (shineL - L) * shineMix;
            R = R + (shineR - R) * shineMix;
        }

        // ── SPARK (lookahead soft-clip limiter) ──
        {
            const float ceilLin = juce::Decibels::decibelsToGain(ceilDb);
            const float grDb = sparkLimiter.processStereo(L, R, ceilLin, sparkMix);
            // Smooth GR for metering
            const float sparkAttack  = 0.3f;
            const float sparkRelease = 0.02f;
            const float coeff = grDb > sparkGrEnvelope ? sparkAttack : sparkRelease;
            sparkGrEnvelope += coeff * (grDb - sparkGrEnvelope);
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

    // SR-dependent ballistics
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

    // Store to atomics
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

    if (requestedQuality != activeQualityMode) {
        activeQualityMode = requestedQuality;
        updateLatencyFromQuality(activeQualityMode);
    }

    float* dataL = buffer.getWritePointer(0);
    float* dataR = buffer.getWritePointer(1);

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

        // ── Phase 4: Smoothed auto-gain ──
        if (autoGain > 0.5f) {
            autoGainSmoother.processBlock(dataL, dataR, numSamples, dryReadL, dryReadR);
        }
    }

    if (bypassed) {
        meterBallistics.sparkGR *= 0.9f;
        sparkGrEnvelope *= 0.9f;
    }

    updateMeters(dryReadL, dryReadR, dataL, dataR, numSamples, sparkGrEnvelope);
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

    // Re-initialize smoothers after state recall to prevent parameter jumps
    initSmoothers(currentSampleRate);

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}
