/*
  PluginProcessor.cpp
  BTZ - The Box Tone Zone Enhancer
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

BTZAudioProcessor::BTZAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
       apvts(*this, nullptr, "Parameters", BTZParams::createParameterLayout())
{
}

BTZAudioProcessor::~BTZAudioProcessor()
{
}

const juce::String BTZAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BTZAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BTZAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BTZAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BTZAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BTZAudioProcessor::getNumPrograms()
{
    return 1;
}

int BTZAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BTZAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BTZAudioProcessor::getProgramName (int index)
{
    return {};
}

void BTZAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void BTZAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Mark as prepared (prevents crashes if host calls processBlock before prepare)
    callOrderGuard.markPrepared(sampleRate, samplesPerBlock);

    // FIX #1: CRITICAL - Denormal protection (prevents 10-100x CPU spikes)
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Prepare all DSP modules
    transientShaper.prepare(spec);
    saturation.prepare(spec);
    subHarmonic.prepare(spec);
    sparkLimiter.prepare(spec);
    shineEQ.prepare(spec);
    consoleEmulator.prepare(spec);
    oversampler.prepare(spec);

    inputGainProcessor.prepare(spec);
    outputGainProcessor.prepare(spec);

    // P2-3: Initialize parameter smoothing using constants
    smoothedPunch.reset(sampleRate, BTZConstants::parameterSmoothingTime);
    smoothedWarmth.reset(sampleRate, BTZConstants::parameterSmoothingTime);
    smoothedBoom.reset(sampleRate, BTZConstants::parameterSmoothingTime);
    smoothedMix.reset(sampleRate, BTZConstants::parameterSmoothingTime);
    smoothedDrive.reset(sampleRate, BTZConstants::parameterSmoothingTime);
    smoothedInputGain.reset(sampleRate, BTZConstants::gainSmoothingTime);
    smoothedOutputGain.reset(sampleRate, BTZConstants::gainSmoothingTime);

    // NEW: Initialize TPT DC blockers (removes DC offset from saturation)
    for (auto& blocker : dcBlockerInput)
        blocker.prepare(sampleRate);
    for (auto& blocker : dcBlockerOutput)
        blocker.prepare(sampleRate);

    // P1-5 & P2-3: Report TOTAL latency to DAW (oversampling + lookahead)
    int sparkOSIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkOS)->load();
    int oversamplingFactor = 1 << sparkOSIndex;  // 1x, 2x, 4x, 8x, 16x
    int oversamplingLatency = (oversamplingFactor - 1) * samplesPerBlock / 2;

    int totalLatency = oversamplingLatency + BTZConstants::sparkLimiterLookahead;
    setLatencySamples(totalLatency);
}

void BTZAudioProcessor::releaseResources()
{
    // Mark as released (prevents stale processing if host re-uses instance)
    callOrderGuard.markReleased();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BTZAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BTZAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // P2-6 FIX: Denormal protection at block level (some hosts reset FTZ flags)
    juce::ScopedNoDenormals noDenormals;
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();

    // P0 FIX: Host call order guard (some hosts call process before prepare!)
    if (!callOrderGuard.safeToProcess())
    {
        buffer.clear();
        return; // Skip processing if not prepared
    }

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // FIX #5: Silence optimization (skip processing if buffer is silent)
    if (isBufferSilent(buffer))
    {
        consecutiveSilentBuffers++;
        if (consecutiveSilentBuffers > BTZConstants::maxSilentBuffersBeforeSkip)
        {
            updateMetering(buffer); // Still update meters
            return; // Skip DSP entirely
        }
    }
    else
    {
        consecutiveSilentBuffers = 0;
    }

    // Check if plugin is active
    bool isActive = apvts.getRawParameterValue(BTZParams::IDs::active)->load() > 0.5f;
    if (!isActive)
    {
        updateMetering(buffer);
        return; // Bypass
    }

    // P0-4 FIX: Set parameter targets once at start of block
    smoothedPunch.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::punch)->load());
    smoothedWarmth.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::warmth)->load());
    smoothedBoom.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::boom)->load());
    smoothedMix.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::mix)->load());
    smoothedDrive.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::drive)->load());
    smoothedInputGain.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::inputGain)->load());
    smoothedOutputGain.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::outputGain)->load());

    // P0-4 FIX: Advance smoothers by buffer length and get smoothed values
    // This provides block-rate smoothing (better than single-sample but not perfect)
    // Future: Implement sub-block processing for true sample-accurate smoothing
    const int numSamples = buffer.getNumSamples();
    smoothedPunch.skip(numSamples);
    smoothedWarmth.skip(numSamples);
    smoothedBoom.skip(numSamples);
    smoothedMix.skip(numSamples);
    smoothedDrive.skip(numSamples);
    smoothedInputGain.skip(numSamples);
    smoothedOutputGain.skip(numSamples);

    float punchAmount = smoothedPunch.getCurrentValue();
    float warmthAmount = smoothedWarmth.getCurrentValue();
    float boomAmount = smoothedBoom.getCurrentValue();
    float mixAmount = smoothedMix.getCurrentValue();
    float driveAmount = smoothedDrive.getCurrentValue();
    float inputGainDb = smoothedInputGain.getCurrentValue();
    float outputGainDb = smoothedOutputGain.getCurrentValue();

    // Read non-smoothed parameters once (these don't cause zipper noise - they're on/off or discrete)
    bool sparkEnabled = apvts.getRawParameterValue(BTZParams::IDs::sparkEnabled)->load() > 0.5f;
    float sparkLUFS = apvts.getRawParameterValue(BTZParams::IDs::sparkLUFS)->load();
    float sparkCeiling = apvts.getRawParameterValue(BTZParams::IDs::sparkCeiling)->load();
    float sparkMix = apvts.getRawParameterValue(BTZParams::IDs::sparkMix)->load();
    int sparkOSIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkOS)->load();
    int sparkModeIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkMode)->load();

    bool shineEnabled = apvts.getRawParameterValue(BTZParams::IDs::shineEnabled)->load() > 0.5f;
    float shineFreqHz = apvts.getRawParameterValue(BTZParams::IDs::shineFreqHz)->load();
    float shineGainDb = apvts.getRawParameterValue(BTZParams::IDs::shineGainDb)->load();
    float shineQ = apvts.getRawParameterValue(BTZParams::IDs::shineQ)->load();
    float shineMix = apvts.getRawParameterValue(BTZParams::IDs::shineMix)->load();

    int masterBlendIndex = apvts.getRawParameterValue(BTZParams::IDs::masterBlend)->load();
    bool masterEnabled = apvts.getRawParameterValue(BTZParams::IDs::masterEnabled)->load() > 0.5f;

    // Update DSP parameters
    transientShaper.setPunch(punchAmount);
    saturation.setWarmth(warmthAmount);
    subHarmonic.setBoom(boomAmount);

    sparkLimiter.setTargetLUFS(sparkLUFS);
    sparkLimiter.setCeiling(sparkCeiling);
    sparkLimiter.setMix(sparkMix);
    // P0 FIX: Detect OS factor change and defer to message thread (prevents allocation)
    int newOSFactor = 1 << sparkOSIndex; // 1x, 2x, 4x, 8x, 16x
    if (newOSFactor != pendingOSFactor.load(std::memory_order_relaxed))
    {
        pendingOSFactor.store(newOSFactor, std::memory_order_relaxed);
        osFactorNeedsUpdate.store(true, std::memory_order_release);
        triggerAsyncUpdate(); // Defer to message thread
    }
    sparkLimiter.setMode(sparkModeIndex == 0 ? SparkLimiter::Mode::Soft : SparkLimiter::Mode::Hard);

    shineEQ.setFrequency(shineFreqHz);
    shineEQ.setGain(shineGainDb);
    shineEQ.setQ(shineQ);
    shineEQ.setMix(shineMix);

    ConsoleEmulator::Type consoleType;
    switch (masterBlendIndex)
    {
        case 0: consoleType = ConsoleEmulator::Type::Transparent; break;
        case 1: consoleType = ConsoleEmulator::Type::Glue; break;
        case 2: consoleType = ConsoleEmulator::Type::Vintage; break;
        default: consoleType = ConsoleEmulator::Type::Transparent; break;
    }
    consoleEmulator.setType(consoleType);
    consoleEmulator.setMix(mixAmount);

    // Set I/O gains
    inputGainProcessor.setGainDecibels(inputGainDb);
    outputGainProcessor.setGainDecibels(outputGainDb);

    // Create audio block
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // === DSP CHAIN ===

    // 1. Input gain
    inputGainProcessor.process(context);

    // NEW: DC blocking at input (removes any DC offset from source)
    for (size_t ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = dcBlockerInput[ch].process(data[i]);
    }

    // P1-1 FIX: Include TransientShaper in oversampling for anti-aliasing
    // TransientShaper applies up to 3x gain changes (nonlinear) → needs oversampling
    bool needsOversampling = (punchAmount > 0.01f || warmthAmount > 0.01f || sparkEnabled);

    if (needsOversampling)
    {
        // Upsample to 2x/4x/8x/16x sample rate for artifact-free nonlinear processing
        int oversamplingFactor = 1 << sparkOSIndex;
        auto oversampledBlock = oversampler.processUp(block);
        juce::dsp::ProcessContextReplacing<float> oversampledContext(oversampledBlock);

        // 2. Punch (transient shaping) - at high SR, no aliasing!
        if (punchAmount > 0.01f)
            transientShaper.process(oversampledContext);

        // 3. Warmth (saturation) - at high sample rate, no aliasing!
        if (warmthAmount > 0.01f)
            saturation.process(oversampledContext);

        // 4. SPARK (advanced clipping/limiting) - at high SR
        if (sparkEnabled)
            sparkLimiter.process(oversampledContext);

        // Downsample with anti-aliasing filter
        oversampler.processDown(block);
    }
    else
    {
        // No oversampling needed, process normally
        if (punchAmount > 0.01f)
            transientShaper.process(context);
        if (warmthAmount > 0.01f)
            saturation.process(context);
        if (sparkEnabled)
            sparkLimiter.process(context);
    }

    // NEW: DC blocking after non-linear processing (removes DC offset from saturation)
    for (size_t ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = dcBlockerOutput[ch].process(data[i]);
    }

    // 4. Boom (subharmonic synthesis)
    if (boomAmount > 0.01f)
        subHarmonic.process(context);

    // 6. SHINE (ultra-high frequency air) - now uses professional RBJ filters
    if (shineEnabled)
        shineEQ.process(context);

    // 7. Console emulation (mix glue)
    if (masterEnabled || mixAmount < 0.99f)
        consoleEmulator.process(context);

    // 8. Output gain
    outputGainProcessor.process(context);

    // P2-4 FIX: DSP validation in ALL builds (not just DEBUG)
    // NaN/Inf must be caught in release mode too - silent failures are worse than crashes
    if (!BTZValidation::validateBuffer(buffer))
    {
        rtLogger.logRT("BTZ: Invalid samples detected - sanitizing");
        BTZValidation::sanitizeBuffer(buffer); // Replace NaN/Inf with silence
    }

    #if JUCE_DEBUG
    // Additional validation in DEBUG builds only
    if (BTZValidation::hasDCOffset(buffer, BTZConstants::dcOffsetThreshold))
    {
        rtLogger.logRT("BTZ: DC offset detected");
        // Note: Cannot log numeric value in RT context (would require sprintf)
    }
    #endif

    // Update metering for GUI
    updateMetering(buffer);
}

void BTZAudioProcessor::updateMetering(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Peak detection
    float peakLevel = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float channelPeak = buffer.getMagnitude(ch, 0, numSamples);
        peakLevel = std::max(peakLevel, channelPeak);
    }
    currentPeak.store(juce::Decibels::gainToDecibels(peakLevel, BTZConstants::minMeteringLevel));

    // Simplified LUFS (RMS-based approximation)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float rms = buffer.getRMSLevel(ch, 0, numSamples);
        lufsAccumulator += rms * rms;
        lufsSampleCount++;
    }

    if (lufsSampleCount > BTZConstants::lufsSampleCountThreshold)
    {
        float averageRMS = std::sqrt(lufsAccumulator / lufsSampleCount);
        float lufsEstimate = juce::Decibels::gainToDecibels(averageRMS, BTZConstants::minMeteringLevel) + BTZConstants::lufsKWeightingOffset;
        currentLUFS.store(lufsEstimate);
        lufsAccumulator = 0.0f;
        lufsSampleCount = 0;
    }

    // Stereo correlation (if stereo)
    if (numChannels == 2)
    {
        const float* left = buffer.getReadPointer(0);
        const float* right = buffer.getReadPointer(1);
        float correlation = 0.0f;
        for (int i = 0; i < numSamples; ++i)
            correlation += left[i] * right[i];
        correlation /= numSamples;
        stereoCorrelation.store(juce::jlimit(-1.0f, 1.0f, correlation));
    }

    // Gain reduction (if SPARK is active)
    bool sparkEnabled = apvts.getRawParameterValue(BTZParams::IDs::sparkEnabled)->load() > 0.5f;
    if (sparkEnabled)
    {
        float targetGain = 1.0f - (peakLevel - 0.95f) * 2.0f;
        gainReduction.store(juce::jlimit(0.7f, 1.0f, targetGain));
    }
    else
    {
        gainReduction.store(1.0f);
    }
}

bool BTZAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void BTZAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // FIX #6: Add version for preset backward compatibility
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());

    // Add version attributes for future migration logic
    xml->setAttribute("pluginVersion", juce::String(pluginVersionMajor) + "." +
                                        juce::String(pluginVersionMinor) + "." +
                                        juce::String(pluginVersionPatch));
    xml->setAttribute("pluginName", "BTZ");

    copyXmlToBinary (*xml, destData);
}

void BTZAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        // P1-6 FIX: Version-aware parameter migration
        juce::String loadedVersion = xmlState->getStringAttribute("pluginVersion", "0.0.0");
        juce::String currentVersion = juce::String(pluginVersionMajor) + "." +
                                      juce::String(pluginVersionMinor) + "." +
                                      juce::String(pluginVersionPatch);

        // Migration logic for future parameter changes
        // This ensures backward compatibility when parameters are added/removed/renamed

        if (loadedVersion == "0.0.0")
        {
            // Legacy state (no version) → v1.0.0 migration
            // All existing parameters should load with defaults
            rtLogger.logRT("BTZ: Loading legacy state (no version)");
        }

        // Example for future v1.0.0 → v1.1.0 migration:
        // if (loadedVersion == "1.0.0" && currentVersion >= "1.1.0")
        // {
        //     // Add new parameters with sensible defaults
        //     if (!xmlState->getChildByAttribute("PARAM", "id", "newFeature"))
        //     {
        //         auto* newParam = xmlState->createNewChildElement("PARAM");
        //         newParam->setAttribute("id", "newFeature");
        //         newParam->setAttribute("value", "0.5");
        //     }
        // }

        // Apply state (APVTS handles missing parameters gracefully with defaults)
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));

            #if JUCE_DEBUG
            // Verify all critical parameters loaded successfully
            if (apvts.getParameter("punch") == nullptr)
                rtLogger.logRT("BTZ: WARNING - Critical parameter 'punch' missing from state");
            #endif
        }
        else
        {
            // State XML has wrong tag name - corrupted or incompatible
            rtLogger.logRT("BTZ: State load failed - incompatible format");
        }
    }
    else
    {
        // Failed to parse XML - corrupted data
        rtLogger.logRT("BTZ: State load failed - corrupted XML");
    }
}

// P0 FIX: Async oversampling factor update (message thread only - safe to allocate)
void BTZAudioProcessor::handleAsyncUpdate()
{
    // Check if update is needed (atomic flag prevents race)
    if (!osFactorNeedsUpdate.exchange(false, std::memory_order_acquire))
        return;

    int newFactor = pendingOSFactor.load(std::memory_order_relaxed);

    // Update oversampler (allocation happens here, but we're on message thread - SAFE!)
    oversampler.setOversamplingFactor(newFactor);

    // Update SparkLimiter OS factor
    sparkLimiter.setOversamplingFactor(newFactor);

    // P1-5 & P2-3: Recalculate TOTAL latency (oversampling + lookahead)
    int sparkOSIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkOS)->load();
    int oversamplingFactor = 1 << sparkOSIndex;
    int samplesPerBlock = getBlockSize();
    int oversamplingLatency = (oversamplingFactor - 1) * samplesPerBlock / 2;

    int totalLatency = oversamplingLatency + BTZConstants::sparkLimiterLookahead;
    setLatencySamples(totalLatency);

    DBG("BTZ: Oversampling factor updated to " + juce::String(newFactor) + "x on message thread");
}

// FIX #5: Silence detection implementation
bool BTZAudioProcessor::isBufferSilent(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        if (buffer.getMagnitude(ch, 0, numSamples) > BTZConstants::silenceThreshold)
            return false;
    }
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BTZAudioProcessor();
}
