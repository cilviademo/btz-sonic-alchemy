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

    // FIX #3: Initialize parameter smoothing (prevents zipper noise)
    smoothedPunch.reset(sampleRate, 0.02);      // 20ms ramp
    smoothedWarmth.reset(sampleRate, 0.02);
    smoothedBoom.reset(sampleRate, 0.02);
    smoothedMix.reset(sampleRate, 0.02);
    smoothedDrive.reset(sampleRate, 0.02);
    smoothedInputGain.reset(sampleRate, 0.05);  // 50ms for gain changes
    smoothedOutputGain.reset(sampleRate, 0.05);

    // NEW: Initialize TPT DC blockers (removes DC offset from saturation)
    for (auto& blocker : dcBlockerInput)
        blocker.prepare(sampleRate);
    for (auto& blocker : dcBlockerOutput)
        blocker.prepare(sampleRate);

    // FIX #4: Report latency to DAW for automatic compensation
    int sparkOSIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkOS)->load();
    int oversamplingFactor = 1 << sparkOSIndex;  // 1x, 2x, 4x, 8x, 16x
    int estimatedLatency = (oversamplingFactor - 1) * samplesPerBlock / 2;
    setLatencySamples(estimatedLatency);
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
    juce::ScopedNoDenormals noDenormals;

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
        if (consecutiveSilentBuffers > maxSilentBuffersBeforeSkip)
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

    // FIX #3: Use smoothed parameters (prevents zipper noise during automation)
    smoothedPunch.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::punch)->load());
    smoothedWarmth.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::warmth)->load());
    smoothedBoom.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::boom)->load());
    smoothedMix.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::mix)->load());
    smoothedDrive.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::drive)->load());
    smoothedInputGain.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::inputGain)->load());
    smoothedOutputGain.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::outputGain)->load());

    float punchAmount = smoothedPunch.getNextValue();
    float warmthAmount = smoothedWarmth.getNextValue();
    float boomAmount = smoothedBoom.getNextValue();
    float mixAmount = smoothedMix.getNextValue();
    float driveAmount = smoothedDrive.getNextValue();
    float inputGainDb = smoothedInputGain.getNextValue();
    float outputGainDb = smoothedOutputGain.getNextValue();

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

    // 2. Punch (transient shaping)
    if (punchAmount > 0.01f)
        transientShaper.process(context);

    // FIX #2: OVERSAMPLING INTEGRATION (was never actually used!)
    // Non-linear modules (saturation, spark) need oversampling to prevent aliasing
    bool needsOversampling = (warmthAmount > 0.01f || sparkEnabled);

    if (needsOversampling)
    {
        // Upsample to 2x/4x/8x/16x sample rate
        int oversamplingFactor = 1 << sparkOSIndex;
        auto oversampledBlock = oversampler.processUp(block);
        juce::dsp::ProcessContextReplacing<float> oversampledContext(oversampledBlock);

        // 3. Warmth (saturation) - at high sample rate, no aliasing!
        if (warmthAmount > 0.01f)
            saturation.process(oversampledContext);

        // 5. SPARK (advanced clipping/limiting) - at high SR
        if (sparkEnabled)
            sparkLimiter.process(oversampledContext);

        // Downsample with anti-aliasing filter
        oversampler.processDown(block);
    }
    else
    {
        // No oversampling needed, process normally
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

    // P0 FIX: DSP validation with RT-safe logging (no DBG allocations!)
    #if JUCE_DEBUG
    if (!BTZValidation::validateBuffer(buffer))
    {
        rtLogger.logRT("BTZ: Invalid samples detected in output buffer!");
        BTZValidation::sanitizeBuffer(buffer); // Replace NaN/Inf with silence
    }

    // Check for DC offset (should be < 0.01 after DC blockers)
    if (BTZValidation::hasDCOffset(buffer, 0.01f))
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
    currentPeak.store(juce::Decibels::gainToDecibels(peakLevel, -60.0f));

    // Simplified LUFS (RMS-based approximation)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float rms = buffer.getRMSLevel(ch, 0, numSamples);
        lufsAccumulator += rms * rms;
        lufsSampleCount++;
    }

    if (lufsSampleCount > 100)
    {
        float averageRMS = std::sqrt(lufsAccumulator / lufsSampleCount);
        float lufsEstimate = juce::Decibels::gainToDecibels(averageRMS, -60.0f) - 23.0f; // K-weighting approximation
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
        // FIX #6: Check version for parameter migration
        juce::String loadedVersion = xmlState->getStringAttribute("pluginVersion", "0.0.0");

        // Future: Add parameter migration logic here if structure changes
        // Example:
        // if (loadedVersion == "1.0.0")
        // {
        //     // Migrate old parameters to new structure
        // }

        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
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

    // Recalculate latency
    int sparkOSIndex = apvts.getRawParameterValue(BTZParams::IDs::sparkOS)->load();
    int oversamplingFactor = 1 << sparkOSIndex;
    int samplesPerBlock = getBlockSize();
    int estimatedLatency = (oversamplingFactor - 1) * samplesPerBlock / 2;
    setLatencySamples(estimatedLatency);

    DBG("BTZ: Oversampling factor updated to " + juce::String(newFactor) + "x on message thread");
}

// FIX #5: Silence detection implementation
bool BTZAudioProcessor::isBufferSilent(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        if (buffer.getMagnitude(ch, 0, numSamples) > silenceThreshold)
            return false;
    }
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BTZAudioProcessor();
}
