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
}

void BTZAudioProcessor::releaseResources()
{
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check if plugin is active
    bool isActive = apvts.getRawParameterValue(BTZParams::IDs::active)->load() > 0.5f;
    if (!isActive)
    {
        updateMetering(buffer);
        return; // Bypass
    }

    // Get all parameters
    float punchAmount = apvts.getRawParameterValue(BTZParams::IDs::punch)->load();
    float warmthAmount = apvts.getRawParameterValue(BTZParams::IDs::warmth)->load();
    float boomAmount = apvts.getRawParameterValue(BTZParams::IDs::boom)->load();
    float mixAmount = apvts.getRawParameterValue(BTZParams::IDs::mix)->load();
    float driveAmount = apvts.getRawParameterValue(BTZParams::IDs::drive)->load();

    float inputGainDb = apvts.getRawParameterValue(BTZParams::IDs::inputGain)->load();
    float outputGainDb = apvts.getRawParameterValue(BTZParams::IDs::outputGain)->load();

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
    sparkLimiter.setOversamplingFactor(1 << sparkOSIndex); // 1x, 2x, 4x, 8x, 16x
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

    // 2. Punch (transient shaping)
    if (punchAmount > 0.01f)
        transientShaper.process(context);

    // 3. Warmth (saturation)
    if (warmthAmount > 0.01f)
        saturation.process(context);

    // 4. Boom (subharmonic synthesis)
    if (boomAmount > 0.01f)
        subHarmonic.process(context);

    // 5. SPARK (advanced clipping/limiting)
    if (sparkEnabled)
        sparkLimiter.process(context);

    // 6. SHINE (ultra-high frequency air)
    if (shineEnabled)
        shineEQ.process(context);

    // 7. Console emulation (mix glue)
    if (masterEnabled || mixAmount < 0.99f)
        consoleEmulator.process(context);

    // 8. Output gain
    outputGainProcessor.process(context);

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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BTZAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BTZAudioProcessor();
}
