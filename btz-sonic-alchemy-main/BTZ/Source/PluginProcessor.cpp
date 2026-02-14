/*
  Box Tone Zone (BTZ) - PluginProcessor.cpp
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
static inline float fastTanh(float x) {
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}
}

juce::AudioProcessorValueTreeState::ParameterLayout BTZAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pct = [](const juce::String& id, const juce::String& name, float def) {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), def);
    };

    params.push_back(pct("punch", "Punch", 0.18f));
    params.push_back(pct("warmth", "Warmth", 0.22f));
    params.push_back(pct("boom", "Boom", 0.10f));
    params.push_back(pct("glue", "Glue", 0.25f));
    params.push_back(pct("air", "Air", 0.12f));
    params.push_back(pct("width", "Width", 0.50f));
    params.push_back(pct("density", "Density", 0.16f));
    params.push_back(pct("motion", "Motion", 0.04f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vintageModern", 1), "Era",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    params.push_back(pct("mix", "Mix", 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("drive", 1), "Drive",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sparkCeiling", 1), "TP Ceil",
        juce::NormalisableRange<float>(-3.0f, 0.0f, 0.01f), -0.3f));
    params.push_back(pct("sparkMix", "Spark Mix", 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("shineAmount", 1), "Shine",
        juce::NormalisableRange<float>(0.0f, 6.0f, 0.1f), 1.2f));
    params.push_back(pct("shineMix", "Shine Mix", 0.30f));

    params.push_back(pct("masterIntensity", "Master", 0.42f));
    params.push_back(pct("autogain", "AutoGain", 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("qualityMode", 1), "Quality",
        juce::NormalisableRange<float>(0.0f, 2.0f, 1.0f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stabilityMode", 1), "Character",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bypass", 1), "Bypass",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    return { params.begin(), params.end() };
}

BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BTZParams", createParameterLayout()) {}

bool BTZAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void BTZAudioProcessor::initSmoothers(double sampleRate) {
    auto initSmooth = [sampleRate](SmoothParam& s, float ms) { s.setTime(ms, sampleRate); };
    initSmooth(sPunch, 5.0f);      initSmooth(sWarmth, 6.0f);
    initSmooth(sBoom, 8.0f);       initSmooth(sGlue, 20.0f);
    initSmooth(sAir, 6.0f);        initSmooth(sWidth, 20.0f);
    initSmooth(sDensity, 6.0f);    initSmooth(sMotion, 40.0f);
    initSmooth(sEra, 25.0f);       initSmooth(sMix, 12.0f);
    initSmooth(sDrive, 5.0f);      initSmooth(sMaster, 25.0f);
    initSmooth(sSparkCeil, 5.0f);  initSmooth(sSparkMix, 5.0f);
    initSmooth(sShine, 5.0f);      initSmooth(sShineMix, 5.0f);

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
}

void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    maxPreparedBlockSize = juce::jmax(samplesPerBlock, 32768);

    safetyPre.setSampleRate(sampleRate);
    safetyPost.setSampleRate(sampleRate);
    safetyPre.reset();
    safetyPost.reset();
    slewL.setSampleRate(sampleRate);
    slewR.setSampleRate(sampleRate);
    slewL.reset();
    slewR.reset();
    peakEnvL.setTimes(0.2f, 220.0f, sampleRate);
    peakEnvR.setTimes(0.2f, 220.0f, sampleRate);
    rmsEnvL.setTimes(25.0f, 300.0f, sampleRate);
    rmsEnvR.setTimes(25.0f, 300.0f, sampleRate);
    glueEnv.setTimes(5.0f, 80.0f, sampleRate);
    glueEnv.reset();

    glueGain = 1.0f;
    sparkGrEnvelope = 0.0f;
    hpStateL = hpStateR = 0.0f;
    sideLowState = 0.0f;
    xoverLowL = xoverLowR = 0.0f;
    noiseSeed = 12345u;

    const float omega = 6.2831853f * 250.0f / (float) sampleRate;
    xoverCoeff = omega / (1.0f + omega);

    const float sideOmega = 6.2831853f * 120.0f / (float) sampleRate;
    sideLowCoeff = sideOmega / (1.0f + sideOmega);

    const float sparkAttackMs = 8.0f;
    const float sparkReleaseMs = 120.0f;
    sparkAttackCoeff = 1.0f - std::exp(-1.0f / ((float) sampleRate * sparkAttackMs * 0.001f));
    sparkReleaseCoeff = 1.0f - std::exp(-1.0f / ((float) sampleRate * sparkReleaseMs * 0.001f));

    initSmoothers(sampleRate);

    dryBuffer.setSize(2, maxPreparedBlockSize, false, false, true);
    dryBuffer.clear();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax(1, samplesPerBlock);
    spec.numChannels = 2;

    os2x = std::make_unique<juce::dsp::Oversampling<float>>(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    os4x = std::make_unique<juce::dsp::Oversampling<float>>(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
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
    return juce::jlimit(0, 2, (int) std::lround(quality));
}

void BTZAudioProcessor::updateLatencyFromQuality(int mode) {
    int latency = 0;
    if (mode == 1 && os2x != nullptr)
        latency = (int) std::ceil(os2x->getLatencyInSamples());
    else if (mode >= 2 && os4x != nullptr)
        latency = (int) std::ceil(os4x->getLatencyInSamples());

    if (latency != getLatencySamples())
        setLatencySamples(latency);
}

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
}

void BTZAudioProcessor::processCore(float* dataL, float* dataR, int numSamples, float osFactor) {
    for (int n = 0; n < numSamples; ++n) {
        float punch = sPunch.next();
        float warmth = sWarmth.next();
        float boom = sBoom.next();
        float glue = sGlue.next();
        float air = sAir.next();
        float width = sWidth.next();
        float density = sDensity.next();
        float motion = sMotion.next();
        float era = sEra.next();
        float drive = sDrive.next();
        float master = sMaster.next();
        float ceilDb = sSparkCeil.next();
        float sparkMix = sSparkMix.next();
        float shine = sShine.next();
        float shineMix = sShineMix.next();

        float L = dataL[n];
        float R = dataR[n];

        L = safetyPre.processSample(L, safetyPre.dcL, safetyPre.dcPrevL);
        R = safetyPre.processSample(R, safetyPre.dcR, safetyPre.dcPrevR);

        if (drive > 0.0f) {
            const float inGain = std::pow(10.0f, drive / 20.0f);
            L *= inGain;
            R *= inGain;
        }

        const float masterScale = juce::jlimit(0.25f, 1.25f, 0.7f + master * 0.6f);
        punch *= masterScale; warmth *= masterScale; boom *= masterScale;
        glue *= masterScale; air *= masterScale; density *= masterScale;

        {
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

        L = slewL.process(L);
        R = slewR.process(R);

        {
            xoverLowL += xoverCoeff * (L - xoverLowL);
            xoverLowR += xoverCoeff * (R - xoverLowR);
            const float highL = L - xoverLowL;
            const float highR = R - xoverLowR;

            const float lowDrv = 1.0f + boom * 1.25f;
            const float highDrv = 1.0f + warmth * 1.75f;
            const float satAmt = juce::jlimit(0.0f, 1.0f, warmth * 0.65f + density * 0.35f);

            const float satLowL = fastTanh(xoverLowL * lowDrv) / lowDrv;
            const float satLowR = fastTanh(xoverLowR * lowDrv) / lowDrv;
            const float satHiL = fastTanh(highL * highDrv) / highDrv;
            const float satHiR = fastTanh(highR * highDrv) / highDrv;

            L = xoverLowL + (satLowL - xoverLowL) * satAmt + highL + (satHiL - highL) * satAmt;
            R = xoverLowR + (satLowR - xoverLowR) * satAmt + highR + (satHiR - highR) * satAmt;
        }

        {
            const float peakL = peakEnvL.process(std::abs(L));
            const float rmsL = std::sqrt(rmsEnvL.process(L * L) + 1.0e-12f);
            const float crest = peakL / juce::jmax(1.0e-5f, rmsL);
            const float harmonicBias = juce::jlimit(0.8f, 1.3f, 1.0f + (crest - 3.0f) * 0.06f);
            const float amount = punch * 0.25f;
            if (amount > 0.0005f) {
                const float drv = 1.0f + punch * 2.0f;
                const float oddL = fastTanh(drv * L);
                const float evenL = fastTanh(drv * L + 0.25f) - fastTanh(0.25f);
                const float oddR = fastTanh(drv * R);
                const float evenR = fastTanh(drv * R + 0.25f) - fastTanh(0.25f);
                L = L + ((oddL * harmonicBias + evenL * (2.0f - harmonicBias)) - L) * amount;
                R = R + ((oddR * harmonicBias + evenR * (2.0f - harmonicBias)) - R) * amount;
            }
        }

        if (glue > 0.01f) {
            const float threshold = juce::Decibels::decibelsToGain(-8.0f - glue * 10.0f);
            const float ratio = 2.0f + glue * 5.0f;
            const float sidechain = juce::jmax(std::abs(L), std::abs(R));
            const float envVal = glueEnv.process(sidechain);

            float gainReduction = 1.0f;
            if (envVal > threshold) {
                const float overDb = juce::Decibels::gainToDecibels(envVal / threshold, -100.0f);
                const float reducedDb = overDb * (1.0f - 1.0f / ratio);
                gainReduction = juce::Decibels::decibelsToGain(-reducedDb);
            }

            const float smoothCoeff = gainReduction < glueGain ? 0.02f : 0.002f;
            glueGain += smoothCoeff * (gainReduction - glueGain);
            L *= glueGain;
            R *= glueGain;
        }

        {
            const float mid = 0.5f * (L + R);
            const float side = 0.5f * (L - R);
            const float widthScale = width * 2.0f;

            sideLowState += sideLowCoeff * (side - sideLowState);
            const float sideLow = sideLowState;
            const float sideHigh = side - sideLow;
            const float lowBandWidth = juce::jmin(widthScale, 1.0f); // Mono-safe low-end widening cap.
            const float sideOut = sideLow * lowBandWidth + sideHigh * widthScale;

            L = mid + sideOut;
            R = mid - sideOut;
        }

        {
            const float airAmount = air + shine * shineMix * 0.15f;
            if (airAmount > 0.001f) {
                const float hpCoeff = juce::jlimit(0.70f, 0.995f, 0.95f - airAmount * 0.12f);
                const float hfL = L - hpStateL; hpStateL = L * (1.0f - hpCoeff) + hpStateL * hpCoeff;
                const float hfR = R - hpStateR; hpStateR = R * (1.0f - hpCoeff) + hpStateR * hpCoeff;
                L += hfL * airAmount * 0.45f;
                R += hfR * airAmount * 0.45f;
            }
        }

        if (boom > 0.01f) {
            L += xoverLowL * boom * 0.28f;
            R += xoverLowR * boom * 0.28f;
        }

        if (density > 0.001f) {
            const float drv = 1.0f + density * 3.0f;
            L = fastTanh(L * drv) / drv;
            R = fastTanh(R * drv) / drv;
        }

        float sparkGrInst = 0.0f;
        {
            const float ceilLin = juce::Decibels::decibelsToGain(ceilDb);
            const float absL = std::abs(L);
            const float absR = std::abs(R);
            const float inAbsMax = juce::jmax(absL, absR);

            if (absL > ceilLin)
                L = ((L > 0.0f ? ceilLin : -ceilLin) * sparkMix) + L * (1.0f - sparkMix);
            if (absR > ceilLin)
                R = ((R > 0.0f ? ceilLin : -ceilLin) * sparkMix) + R * (1.0f - sparkMix);

            const float outAbsMax = juce::jmax(std::abs(L), std::abs(R));
            if (inAbsMax > 1.0e-6f && outAbsMax < inAbsMax)
                sparkGrInst = juce::jmax(0.0f, juce::Decibels::gainToDecibels(inAbsMax / outAbsMax, 0.0f));
        }

        const float sparkCoeff = sparkGrInst > sparkGrEnvelope ? sparkAttackCoeff : sparkReleaseCoeff;
        sparkGrEnvelope += sparkCoeff * (sparkGrInst - sparkGrEnvelope);

        if (motion > 0.01f) {
            noiseSeed = 1664525u * noiseSeed + 1013904223u;
            float white = (float) ((noiseSeed >> 9) & 0x7FFFFF) / 8388608.0f - 0.5f;
            const float noiseLevel = 1.0e-6f * motion * 8.0f / juce::jmax(1.0f, osFactor);
            L += white * noiseLevel;
            noiseSeed = 1664525u * noiseSeed + 1013904223u;
            white = (float) ((noiseSeed >> 9) & 0x7FFFFF) / 8388608.0f - 0.5f;
            R += white * noiseLevel;
        }

        L = safetyPost.processSample(L, safetyPost.dcL, safetyPost.dcPrevL);
        R = safetyPost.processSample(R, safetyPost.dcR, safetyPost.dcPrevR);

        const float neutralComp = 1.0f / juce::jlimit(0.75f, 1.5f, 1.0f + 0.20f * (warmth + density + boom));
        L *= neutralComp;
        R *= neutralComp;

        dataL[n] = L;
        dataR[n] = R;
    }
}

void BTZAudioProcessor::updateMeters(const float* inL, const float* inR, const float* outL, const float* outR, int n, float sparkGRDb) {
    float inPkL = 0.0f, inPkR = 0.0f, outPkL = 0.0f, outPkR = 0.0f;
    float inSqL = 0.0f, inSqR = 0.0f, outSqL = 0.0f, outSqR = 0.0f;
    float corrNum = 0.0f, corrDenL = 0.0f, corrDenR = 0.0f;
    float lufsSq = 0.0f;
    bool clipIn = false, clipOut = false;

    for (int i = 0; i < n; ++i) {
        const float iL = inL[i], iR = inR[i], oL = outL[i], oR = outR[i];
        inPkL = juce::jmax(inPkL, std::abs(iL));
        inPkR = juce::jmax(inPkR, std::abs(iR));
        outPkL = juce::jmax(outPkL, std::abs(oL));
        outPkR = juce::jmax(outPkR, std::abs(oR));
        inSqL += iL * iL; inSqR += iR * iR;
        outSqL += oL * oL; outSqR += oR * oR;
        corrNum += oL * oR;
        corrDenL += oL * oL;
        corrDenR += oR * oR;
        lufsSq += oL * oL + oR * oR;
        clipIn = clipIn || (std::abs(iL) >= 0.999f || std::abs(iR) >= 0.999f);
        clipOut = clipOut || (std::abs(oL) >= 0.999f || std::abs(oR) >= 0.999f);
    }

    const float invN = 1.0f / juce::jmax(1, n);
    const float inRmsL = std::sqrt(inSqL * invN + 1.0e-20f);
    const float inRmsR = std::sqrt(inSqR * invN + 1.0e-20f);
    const float outRmsL = std::sqrt(outSqL * invN + 1.0e-20f);
    const float outRmsR = std::sqrt(outSqR * invN + 1.0e-20f);

    meterBallistics.inPeakHoldL = juce::jmax(inPkL, meterBallistics.inPeakHoldL * meterBallistics.holdDecay);
    meterBallistics.inPeakHoldR = juce::jmax(inPkR, meterBallistics.inPeakHoldR * meterBallistics.holdDecay);
    meterBallistics.outPeakHoldL = juce::jmax(outPkL, meterBallistics.outPeakHoldL * meterBallistics.holdDecay);
    meterBallistics.outPeakHoldR = juce::jmax(outPkR, meterBallistics.outPeakHoldR * meterBallistics.holdDecay);
    meterBallistics.inRmsL += meterBallistics.rmsCoeff * (inRmsL - meterBallistics.inRmsL);
    meterBallistics.inRmsR += meterBallistics.rmsCoeff * (inRmsR - meterBallistics.inRmsR);
    meterBallistics.outRmsL += meterBallistics.rmsCoeff * (outRmsL - meterBallistics.outRmsL);
    meterBallistics.outRmsR += meterBallistics.rmsCoeff * (outRmsR - meterBallistics.outRmsR);
    meterBallistics.sparkGR += 0.2f * (sparkGRDb - meterBallistics.sparkGR);
    meterBallistics.clipHoldIn = juce::jmax(clipIn ? 1.0f : 0.0f, meterBallistics.clipHoldIn * 0.92f);
    meterBallistics.clipHoldOut = juce::jmax(clipOut ? 1.0f : 0.0f, meterBallistics.clipHoldOut * 0.92f);

    const float corrDen = std::sqrt(corrDenL * corrDenR) + 1.0e-12f;
    const float correlation = juce::jlimit(-1.0f, 1.0f, corrNum / corrDen);

    const float lufsRms = std::sqrt((lufsSq * 0.5f) * invN + 1.0e-20f);

    meters.inputPeakL.store(juce::Decibels::gainToDecibels(meterBallistics.inPeakHoldL, -100.0f), std::memory_order_relaxed);
    meters.inputPeakR.store(juce::Decibels::gainToDecibels(meterBallistics.inPeakHoldR, -100.0f), std::memory_order_relaxed);
    meters.inputRmsL.store(juce::Decibels::gainToDecibels(meterBallistics.inRmsL, -100.0f), std::memory_order_relaxed);
    meters.inputRmsR.store(juce::Decibels::gainToDecibels(meterBallistics.inRmsR, -100.0f), std::memory_order_relaxed);
    meters.outputPeakL.store(juce::Decibels::gainToDecibels(meterBallistics.outPeakHoldL, -100.0f), std::memory_order_relaxed);
    meters.outputPeakR.store(juce::Decibels::gainToDecibels(meterBallistics.outPeakHoldR, -100.0f), std::memory_order_relaxed);
    meters.outputRmsL.store(juce::Decibels::gainToDecibels(meterBallistics.outRmsL, -100.0f), std::memory_order_relaxed);
    meters.outputRmsR.store(juce::Decibels::gainToDecibels(meterBallistics.outRmsR, -100.0f), std::memory_order_relaxed);
    meters.sparkGainReductionDb.store(juce::jmax(0.0f, meterBallistics.sparkGR), std::memory_order_relaxed);
    meters.lufs.store(juce::Decibels::gainToDecibels(lufsRms, -100.0f), std::memory_order_relaxed);
    meters.inputClip.store(meterBallistics.clipHoldIn, std::memory_order_relaxed);
    meters.outputClip.store(meterBallistics.clipHoldOut, std::memory_order_relaxed);
    meters.correlation.store(correlation, std::memory_order_relaxed);
}

void BTZAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    if (numSamples <= 0 || buffer.getNumChannels() < 2)
        return;

    const int drySampleCount = juce::jmin(numSamples, dryBuffer.getNumSamples());
    if (drySampleCount > 0) {
        dryBuffer.copyFrom(0, 0, buffer, 0, 0, drySampleCount);
        dryBuffer.copyFrom(1, 0, buffer, 1, 0, drySampleCount);
    }

    const float* dryReadL = dryBuffer.getReadPointer(0);
    const float* dryReadR = dryBuffer.getReadPointer(1);

    updateTargetsFromAPVTS();
    const bool bypassed = *apvts.getRawParameterValue("bypass") > 0.5f;
    const float autoGain = *apvts.getRawParameterValue("autogain");
    const int requestedQuality = getRequestedQualityMode();

    if (requestedQuality != activeQualityMode) {
        activeQualityMode = requestedQuality;
        updateLatencyFromQuality(activeQualityMode);
    }

    float* dataL = buffer.getWritePointer(0);
    float* dataR = buffer.getWritePointer(1);

    if (! bypassed) {
        if (activeQualityMode == 1 && os2x != nullptr) {
            juce::dsp::AudioBlock<float> block(buffer);
            auto stereoBlock = block.getSubsetChannelBlock(0, 2).getSubBlock(0, (size_t) numSamples);
            auto upBlock = os2x->processSamplesUp(stereoBlock);
            processCore(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1), (int) upBlock.getNumSamples(), 2.0f);
            os2x->processSamplesDown(stereoBlock);
        } else if (activeQualityMode >= 2 && os4x != nullptr) {
            juce::dsp::AudioBlock<float> block(buffer);
            auto stereoBlock = block.getSubsetChannelBlock(0, 2).getSubBlock(0, (size_t) numSamples);
            auto upBlock = os4x->processSamplesUp(stereoBlock);
            processCore(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1), (int) upBlock.getNumSamples(), 4.0f);
            os4x->processSamplesDown(stereoBlock);
        } else {
            processCore(dataL, dataR, numSamples, 1.0f);
        }

        // Apply wet/dry at native rate after any oversampling up/down path.
        for (int n = 0; n < drySampleCount; ++n) {
            const float mix = sMix.next();
            dataL[n] = dryReadL[n] + (dataL[n] - dryReadL[n]) * mix;
            dataR[n] = dryReadR[n] + (dataR[n] - dryReadR[n]) * mix;
        }

        // Keep smoother state coherent even when host block exceeds prepared scratch size.
        for (int n = drySampleCount; n < numSamples; ++n)
            (void) sMix.next();
    }

    if (autoGain > 0.5f && ! bypassed && drySampleCount > 0) {
        float inRmsSq = 0.0f, outRmsSq = 0.0f;
        for (int n = 0; n < drySampleCount; ++n) {
            const float iL = dryReadL[n];
            const float iR = dryReadR[n];
            inRmsSq += iL * iL + iR * iR;
            outRmsSq += dataL[n] * dataL[n] + dataR[n] * dataR[n];
        }
        const float inRms = std::sqrt(inRmsSq / juce::jmax(1, drySampleCount * 2) + 1.0e-20f);
        const float outRms = std::sqrt(outRmsSq / juce::jmax(1, drySampleCount * 2) + 1.0e-20f);
        if (inRms > 1.0e-6f && outRms > 1.0e-6f) {
            const float gainDb = juce::jlimit(-4.0f, 4.0f, juce::Decibels::gainToDecibels(inRms / outRms, 0.0f));
            const float gain = juce::Decibels::decibelsToGain(gainDb);
            for (int n = 0; n < numSamples; ++n) {
                dataL[n] *= gain;
                dataR[n] *= gain;
            }
        }
    }

    if (bypassed) {
        meterBallistics.sparkGR *= 0.9f;
        sparkGrEnvelope *= 0.9f;
    }

    const float* meterInL = dryReadL;
    const float* meterInR = dryReadR;
    int meterSamples = drySampleCount;
    if (meterSamples <= 0) {
        meterInL = dataL;
        meterInR = dataR;
        meterSamples = numSamples;
    }

    updateMeters(meterInL, meterInR, dataL, dataR, meterSamples, sparkGrEnvelope);
}

void BTZAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    const auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));

    activeQualityMode = getRequestedQualityMode();
    updateLatencyFromQuality(activeQualityMode);
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() {
    return new BTZAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BTZAudioProcessor();
}

