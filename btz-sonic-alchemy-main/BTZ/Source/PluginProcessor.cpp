/*
  Box Tone Zone (BTZ) — PluginProcessor.cpp  v8
  ────────────────────────────────────────────────────────────────────────
  v8 (competitive-audit driven):
    • REMOVED all 12 ADAATanh instances — plain fastTanh + OS is strictly
      superior (-59 dB alias rejection vs -18.6 dB with ADAA at 4x OS)
    • REMOVED SlewLimiter — was only a safety net for ADAA
    • DC blocker cutoff lowered from 5 Hz to 1 Hz (SafetyLayer v8)
    • TruePeakLimiter: tightened release + added attack coeff for ISP
    • State version bumped to 8
  ────────────────────────────────────────────────────────────────────────
  v7 (release-gate hardening):
    • Click-free bypass via BypassCrossfader (64-sample cosine ramp)
    • Full resetAll() method — transport stop/start safe
    • releaseResources() guarded — no deallocation of dryBuffer
    • OS objects created once, not recreated on every prepareToPlay
    • State migration with version validation (v4→v8 compat)
    • Silence-in-silence-out detection
    • glueScHpf crossfade on mode change (via SidechainHPF v7)
  ────────────────────────────────────────────────────────────────────────
  v6: Macro wiring, glue sidechain HPF
  v5: Envelope followers at base SR, SlewLimiter OS factor
  v4: SVF LR4 crossover, soft-knee glue, Padé [5/5] fastTanh
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ═══════════════════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════════════════
BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BTZ_PARAMS", createParameterLayout())
{
    macroInterpreter.setupDefaults();
}

// ═══════════════════════════════════════════════════════════════════════════
// Parameter layout
// ═══════════════════════════════════════════════════════════════════════════
juce::AudioProcessorValueTreeState::ParameterLayout BTZAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ── Core group ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "punch", "Punch", 0.0f, 1.0f, 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "warmth", "Warmth", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "boom", "Boom", 0.0f, 1.0f, 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "glue", "Glue", 0.0f, 1.0f, 0.25f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "air", "Air", 0.0f, 1.0f, 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "width", "Width", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "density", "Density", 0.0f, 1.0f, 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "motion", "Motion", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vintageModern", "Era", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive", 0.0f, 12.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterIntensity", "Master Intensity", 0.0f, 1.0f, 0.7f));

    // ── SPARK group ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sparkCeiling", "SPARK Ceiling", -12.0f, 0.0f, -0.3f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bypass", "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "autogain", "Auto Gain", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "qualityMode", "Quality Mode", 0.0f, 2.0f, 1.0f));

    // ── SHINE group ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "shineAmount", "SHINE Amount", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "shineMix", "SHINE Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "shineFreq", "SHINE Frequency",
        juce::NormalisableRange<float>(1000.0f, 16000.0f, 1.0f, 0.3f), 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "shineQ", "SHINE Q",
        juce::NormalisableRange<float>(0.3f, 4.0f, 0.01f, 0.5f), 0.707f));

    // ── Macros group ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "macro0", "Macro A", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "macro1", "Macro B", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "macro2", "Macro C", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "macro3", "Macro D", 0.0f, 1.0f, 0.0f));

    // ── Global group ──
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "glueScHpf", "Glue SC HPF", 0.0f, 3.0f, 1.0f));

    return { params.begin(), params.end() };
}

// ═══════════════════════════════════════════════════════════════════════════
// initSmoothers
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::initSmoothers(double sampleRate) {
    const float ms5  = 5.0f;
    const float ms10 = 10.0f;
    const float ms20 = 20.0f;
    const float ms50 = 50.0f;

    sPunch.setTime(ms10, sampleRate);
    sWarmth.setTime(ms10, sampleRate);
    sBoom.setTime(ms20, sampleRate);
    sGlue.setTime(ms10, sampleRate);
    sAir.setTime(ms10, sampleRate);
    sWidth.setTime(ms20, sampleRate);
    sDensity.setTime(ms10, sampleRate);
    sMotion.setTime(ms50, sampleRate);
    sEra.setTime(ms20, sampleRate);
    sMix.setTime(ms5, sampleRate);
    sDrive.setTime(ms10, sampleRate);
    sMaster.setTime(ms20, sampleRate);
    sShine.setTime(ms10, sampleRate);
    sShineMix.setTime(ms10, sampleRate);
    sShineFreq.setTime(ms50, sampleRate);
    sShineQ.setTime(ms50, sampleRate);
    sMacro0.setTime(ms10, sampleRate);
    sMacro1.setTime(ms10, sampleRate);
    sMacro2.setTime(ms10, sampleRate);
    sMacro3.setTime(ms10, sampleRate);
}

// ═══════════════════════════════════════════════════════════════════════════
// resetAll — v7: full DSP state reset (transport-safe, zero allocation)
// v8: removed ADAA and SlewLimiter reset calls
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::resetAll() {
    safetyPre.reset();
    safetyPost.reset();
    peakEnvL.reset();
    peakEnvR.reset();
    rmsEnvL.reset();
    rmsEnvR.reset();
    glueEnv.reset();
    glueComp.reset();
    crossover.reset();
    truePeakLimiter.reset();
    shineProcessor.reset();
    autoGainSmoother.reset();
    bypassCrossfader.reset();
    meterBallistics.reset();

    // v8: no ADAA or SlewLimiter to reset

    // Reset analog state
    hpStateL = hpStateR = 0.0f;
    sideLowState = 0.0f;
    lastCrestRatio = 3.0f;
    noiseSeed = 12345u;
    silentFrameCount = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// isBusesLayoutSupported
// ═══════════════════════════════════════════════════════════════════════════
bool BTZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// prepareToPlay
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // v4: FTZ/DAZ at init
    BTZDsp::enableFlushToZero();

    // Call base class
    juce::AudioProcessor::prepareToPlay(sampleRate, samplesPerBlock);

    currentSampleRate = sampleRate;
    maxPreparedBlockSize = samplesPerBlock;
    currentBlockSize = samplesPerBlock;

    // Safety layers (v8: 1 Hz DC blocker)
    safetyPre.setSampleRate(sampleRate);
    safetyPost.setSampleRate(sampleRate);

    // Envelope followers
    peakEnvL.setTimes(0.1f, 50.0f, sampleRate);
    peakEnvR.setTimes(0.1f, 50.0f, sampleRate);
    rmsEnvL.setTimes(5.0f, 100.0f, sampleRate);
    rmsEnvR.setTimes(5.0f, 100.0f, sampleRate);
    glueEnv.setTimes(0.5f, 80.0f, sampleRate);

    // Glue compressor and sidechain HPF
    glueComp.prepare(sampleRate);
    glueScHpfSampleRate = sampleRate;
    glueScHpf.prepare(sampleRate, lastGlueScHpfFreq);

    // Crossover (v4: LR4 at 250 Hz)
    crossover.prepare(sampleRate, 250.0f);

    // TruePeakLimiter (v8: tightened attack/release for ISP compliance)
    truePeakLimiter.prepare(sampleRate, samplesPerBlock);

    // SHINE (v4: SVF high-shelf)
    shineProcessor.prepare(sampleRate);

    // Auto-gain smoother
    autoGainSmoother.prepare(sampleRate);

    // Meter ballistics
    meterBallistics.prepare(sampleRate, samplesPerBlock);

    // v7: Bypass crossfader
    bypassCrossfader.prepare();

    // Width state
    const float sideOmega = 6.2831853f * 120.0f / (float)sampleRate;
    sideLowCoeff = sideOmega / (1.0f + sideOmega);

    // v4: initialize cached drive gain
    cachedDriveGain = 1.0f;
    lastDriveDb = 0.0f;

    // Smoothers
    initSmoothers(sampleRate);

    // Dry buffer — v7: allocate once, keep across calls
    if (dryBuffer.getNumSamples() < maxPreparedBlockSize) {
        dryBuffer.setSize(2, maxPreparedBlockSize, false, true, true);
    }

    // v7: Oversampling — only recreate if SR or blockSize changed
    const bool osNeedsRecreation = (os2x == nullptr || os4x == nullptr
        || sampleRate != lastPreparedSR
        || samplesPerBlock != lastPreparedBlockSize);

    if (osNeedsRecreation) {
        os2x = std::make_unique<juce::dsp::Oversampling<float>>(
            2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
        os4x = std::make_unique<juce::dsp::Oversampling<float>>(
            2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);

        const auto maxBlock = (juce::uint32) juce::jmax(1, samplesPerBlock);
        os2x->initProcessing(maxBlock);
        os4x->initProcessing(maxBlock);
    }

    lastPreparedSR = sampleRate;
    lastPreparedBlockSize = samplesPerBlock;

    // Full reset of all DSP state
    resetAll();

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);

    // v7: mark as prepared
    prepared = true;
}

// ═══════════════════════════════════════════════════════════════════════════
// releaseResources — v7: do NOT deallocate dryBuffer (crash risk if
// processBlock is called after releaseResources by some hosts)
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::releaseResources() {
    // v7: intentionally keep dryBuffer allocated.
    // Some hosts call processBlock after releaseResources.
    // We just clear it to avoid stale data.
    dryBuffer.clear();

    // Mark as unprepared — processBlock will early-return
    prepared = false;
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
// updateTargetsFromAPVTS
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

    // ── v6: Wire macros through MacroInterpreter ──
    macroValues[0] = sMacro0.current;
    macroValues[1] = sMacro1.current;
    macroValues[2] = sMacro2.current;
    macroValues[3] = sMacro3.current;

    {
        const float modPunch   = macroInterpreter.getModulation(0,  macroValues);
        const float modWarmth  = macroInterpreter.getModulation(1,  macroValues);
        const float modBoom    = macroInterpreter.getModulation(2,  macroValues);
        const float modGlue    = macroInterpreter.getModulation(3,  macroValues);
        const float modAir     = macroInterpreter.getModulation(4,  macroValues);
        const float modDensity = macroInterpreter.getModulation(6,  macroValues);
        const float modDrive   = macroInterpreter.getModulation(10, macroValues);

        sPunch.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("punch") + modPunch));
        sWarmth.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("warmth") + modWarmth));
        sBoom.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("boom") + modBoom));
        sGlue.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("glue") + modGlue));
        sAir.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("air") + modAir));
        sDensity.setTarget(juce::jlimit(0.0f, 1.0f,
            *apvts.getRawParameterValue("density") + modDensity));
        sDrive.setTarget(juce::jlimit(0.0f, 12.0f,
            *apvts.getRawParameterValue("drive") + modDrive * 12.0f));
    }

    // ── v6/v7: Update glue sidechain HPF frequency (crossfade on change) ──
    {
        const float hpfMode = *apvts.getRawParameterValue("glueScHpf");
        float hpfFreq = 0.0f;
        if (hpfMode >= 2.5f)      hpfFreq = 150.0f;
        else if (hpfMode >= 1.5f) hpfFreq = 90.0f;
        else if (hpfMode >= 0.5f) hpfFreq = 60.0f;

        if (std::abs(hpfFreq - lastGlueScHpfFreq) > 0.1f) {
            lastGlueScHpfFreq = hpfFreq;
            // v7: SidechainHPF.prepare() uses internal crossfade
            glueScHpf.prepare(glueScHpfSampleRate, hpfFreq);
        }
    }

    // Update SHINE filter coefficients (v7: smoothed freq/Q to prevent zipper noise)
    sShineFreq.setTarget(*apvts.getRawParameterValue("shineFreq"));
    sShineQ.setTarget(*apvts.getRawParameterValue("shineQ"));
    const float shineFreq = sShineFreq.next();
    const float shineQ    = sShineQ.next();
    const float shineAmt  = *apvts.getRawParameterValue("shineAmount");
    shineProcessor.setParameters(shineFreq, shineAmt, shineQ);

    // v4: cache drive gain to avoid per-sample std::pow
    const float driveDb = *apvts.getRawParameterValue("drive");
    if (std::abs(driveDb - lastDriveDb) > 0.01f) {
        lastDriveDb = driveDb;
        cachedDriveGain = (driveDb > 0.01f) ? std::pow(10.0f, driveDb / 20.0f) : 1.0f;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPre — runs at BASE sample rate, before oversampling
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

        // ── Drive (input gain) — v4: uses cached gain ──
        if (drive > 0.01f) {
            L *= cachedDriveGain;
            R *= cachedDriveGain;
        }

        // ── Master intensity scaling ──
        const float masterScale = juce::jlimit(0.25f, 1.25f, 0.7f + master * 0.6f);
        punch   *= masterScale;
        warmth  *= masterScale;
        boom    *= masterScale;
        glue    *= masterScale;
        air     *= masterScale;
        density *= masterScale;

        // ── Glue compressor (v4: soft-knee, v6: sidechain HPF) ──
        {
            const float sidechain = glueScHpf.process(L, R);
            const float envVal = glueEnv.process(sidechain);
            glueComp.processStereo(L, R, glue, envVal);
        }

        // ── v5 FIX: Envelope followers at BASE sample rate ──
        {
            const float peakVal = peakEnvL.process(std::abs(L));
            const float rmsVal  = std::sqrt(rmsEnvL.process(L * L) + 1.0e-12f);
            lastCrestRatio = peakVal / juce::jmax(1.0e-5f, rmsVal);
        }
        peakEnvR.process(std::abs(R));
        rmsEnvR.process(R * R);

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
// v8: uses plain fastTanh instead of ADAA. No SlewLimiter.
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::processNonlinear(float* dataL, float* dataR, int numSamples, float osFactor) {
    (void) osFactor;  // v8: no longer needed (SlewLimiter removed)

    for (int n = 0; n < numSamples; ++n) {
        const float warmth  = sWarmth.current;
        const float boom    = sBoom.current;
        const float punch   = sPunch.current;
        const float density = sDensity.current;
        const float era     = sEra.current;

        float L = dataL[n];
        float R = dataR[n];

        // ── Preamp / Color (warmth-driven tanh saturation) ──
        // v8: plain fastTanh — measured -59 dB alias rejection at 4x OS
        //     vs -18.6 dB with ADAA. ADAA actively degrades quality.
        if (warmth > 0.001f) {
            const float drv = 1.0f + warmth * 2.8f;
            const float bias = warmth * 0.05f;
            const float eraScale = juce::jmax(0.55f, 1.0f + era * 0.30f);

            const float biasComp = std::tanh(bias * drv / eraScale);
            float yL = BTZDsp::fastTanh((L + bias) * drv / eraScale) - biasComp;
            float yR = BTZDsp::fastTanh((R + bias) * drv / eraScale) - biasComp;
            L = L + (yL - L) * warmth;
            R = R + (yR - R) * warmth;
        }

        // v8: SlewLimiter REMOVED — was only a safety net for ADAA

        // ── Band split (v4: TRUE LR4 crossover at 250 Hz — 24 dB/oct) ──
        float lowL, lowR, highL, highR;
        crossover.process(L, R, lowL, lowR, highL, highR);

        // ── Band saturation (plain fastTanh per band per channel) ──
        {
            const float lowDrv  = 1.0f + boom * 1.25f;
            const float highDrv = 1.0f + warmth * 1.75f;
            const float satAmt  = juce::jlimit(0.0f, 1.0f, warmth * 0.65f + density * 0.35f);

            if (satAmt > 0.001f) {
                const float satLowL = BTZDsp::fastTanh(lowL * lowDrv) / lowDrv;
                const float satLowR = BTZDsp::fastTanh(lowR * lowDrv) / lowDrv;
                const float satHiL  = BTZDsp::fastTanh(highL * highDrv) / highDrv;
                const float satHiR  = BTZDsp::fastTanh(highR * highDrv) / highDrv;

                L = lowL + (satLowL - lowL) * satAmt + highL + (satHiL - highL) * satAmt;
                R = lowR + (satLowR - lowR) * satAmt + highR + (satHiR - highR) * satAmt;
            } else {
                L = lowL + highL;
                R = lowR + highR;
            }
        }

        // ── Punch (crest-aware harmonic injection via fastTanh) ──
        if (punch > 0.002f) {
            const float crest = lastCrestRatio;
            const float harmonicBias = juce::jlimit(0.8f, 1.3f, 1.0f + (crest - 3.0f) * 0.06f);
            const float amount = punch * 0.25f;
            const float drv = 1.0f + punch * 2.0f;

            const float oddL  = BTZDsp::fastTanh(drv * L);
            const float evenL = BTZDsp::fastTanh(drv * L + 0.25f) - BTZDsp::kTanhBias025;
            const float oddR  = BTZDsp::fastTanh(drv * R);
            const float evenR = BTZDsp::fastTanh(drv * R + 0.25f) - BTZDsp::kTanhBias025;
            L = L + ((oddL * harmonicBias + evenL * (2.0f - harmonicBias)) - L) * amount;
            R = R + ((oddR * harmonicBias + evenR * (2.0f - harmonicBias)) - R) * amount;
        }

        // ── Boom (low-band additive boost) ──
        if (boom > 0.01f) {
            L += lowL * boom * 0.28f;
            R += lowR * boom * 0.28f;
        }

        // ── Density (additional fastTanh saturation) ──
        if (density > 0.001f) {
            const float drv = 1.0f + density * 3.0f;
            L = BTZDsp::fastTanh(L * drv) / drv;
            R = BTZDsp::fastTanh(R * drv) / drv;
        }

        dataL[n] = L;
        dataR[n] = R;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// processLinearPost — runs at BASE sample rate, after downsampling
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

        // ── SHINE (v4: sqrt(A)-prewarped SVF high-shelf) ──
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

        // ── Dry/Wet mix ──
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
// processBlock — main entry point
// v7: click-free bypass, silence detection, prepared guard
// v8: ADAA removed, plain fastTanh in processNonlinear
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

    // v7: Guard against processBlock called before prepareToPlay or after releaseResources
    if (!prepared)
        return;

    // ── Store dry buffer ──
    const int copyCount = juce::jmin(numSamples, maxPreparedBlockSize);
    dryBuffer.copyFrom(0, 0, buffer, 0, 0, copyCount);
    dryBuffer.copyFrom(1, 0, buffer, 1, 0, copyCount);

    const float* dryReadL = dryBuffer.getReadPointer(0);
    const float* dryReadR = dryBuffer.getReadPointer(1);

    updateTargetsFromAPVTS();
    const bool bypassed   = *apvts.getRawParameterValue("bypass") > 0.5f;
    const float autoGain  = *apvts.getRawParameterValue("autogain");
    const int requestedQuality = getRequestedQualityMode();

    // sparkCeiling: held per block, NOT smoothed
    const float sparkCeilDb = *apvts.getRawParameterValue("sparkCeiling");
    const float sparkCeilLin = juce::Decibels::decibelsToGain(sparkCeilDb);

    if (requestedQuality != activeQualityMode) {
        activeQualityMode = requestedQuality;
        updateLatencyFromQuality(activeQualityMode);
    }

    float* dataL = buffer.getWritePointer(0);
    float* dataR = buffer.getWritePointer(1);

    float sparkGRDb = 0.0f;

    // ── v7: Silence-in-silence-out detection ──
    {
        float blockPeak = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            blockPeak = juce::jmax(blockPeak, std::abs(dataL[i]), std::abs(dataR[i]));
        }
        if (blockPeak < kSilenceThreshold) {
            silentFrameCount += numSamples;
        } else {
            silentFrameCount = 0;
        }
    }

    const bool inputIsSilent = (silentFrameCount > kSilentFrameThreshold);

    // ── v7: Click-free bypass via BypassCrossfader ──
    bypassCrossfader.setBypassState(bypassed);

    if (!inputIsSilent) {
        // Process wet signal (even during bypass transition for crossfade)
        // ── Phase 1: Linear pre-processing ──
        processLinearPre(dataL, dataR, numSamples);

        // ── Phase 2: Nonlinear processing (oversampled) ──
        // v8: plain fastTanh in processNonlinear (no ADAA)
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

        // ── Phase 3: Linear post-processing ──
        processLinearPost(dataL, dataR, numSamples);

        // ── Phase 4: TruePeakLimiter ──
        truePeakLimiter.processBlock(buffer, sparkCeilLin);
        sparkGRDb = truePeakLimiter.getGainReductionDb();

        // ── Phase 5: Smoothed auto-gain ──
        if (autoGain > 0.5f) {
            autoGainSmoother.processBlock(dataL, dataR, numSamples, dryReadL, dryReadR);
        }

        // ── v7: Apply bypass crossfade ──
        for (int i = 0; i < numSamples; ++i) {
            bypassCrossfader.processStereo(dryReadL[i], dryReadR[i], dataL[i], dataR[i]);
        }

    } else {
        // Input is silent — output silence, keep meters decaying
        buffer.clear();
        meterBallistics.sparkGR *= 0.9f;
    }

    updateMeters(dryReadL, dryReadR, dataL, dataR, numSamples, sparkGRDb);
}

// ═══════════════════════════════════════════════════════════════════════════
// v7: State migration — handle preset loading from older versions
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::migrateState(juce::ValueTree& state, int fromVersion) {
    // v4→v5: no parameter changes, just internal DSP fixes
    // v5→v6: added glueScHpf parameter (default 1 = 60 Hz)
    if (fromVersion < 6) {
        // If glueScHpf doesn't exist in the state, APVTS will use default (1.0 = 60 Hz)
        // No explicit migration needed — APVTS handles missing params gracefully.
    }

    // v6→v7: no new parameters, just behavioral fixes
    // v7→v8: no new parameters, removed ADAA (internal DSP change only)

    (void)state;  // State tree is already handled by APVTS replaceState
}

// ═══════════════════════════════════════════════════════════════════════════
// State serialization
// ═══════════════════════════════════════════════════════════════════════════
void BTZAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    const auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml) {
        xml->setAttribute("btzStateVersion", BTZDsp::kStateVersion);
        copyXmlToBinary(*xml, destData);
    }
}

void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType())) {
        const int version = xml->getIntAttribute("btzStateVersion", 1);

        // v7: Validate version range — reject future versions
        if (version > BTZDsp::kStateVersion) {
            // State from a newer version — do not load, keep current state
            return;
        }

        auto newState = juce::ValueTree::fromXml(*xml);

        // v7: Run migration if loading from an older version
        if (version < BTZDsp::kStateVersion) {
            migrateState(newState, version);
        }

        apvts.replaceState(newState);
    }

    // Re-initialize smoothers and DSP state after loading
    initSmoothers(currentSampleRate);
    resetAll();

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}
