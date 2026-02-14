#include "PluginProcessor.h"
#include "PluginEditor.h"

BTZAudioProcessor::BTZAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Soft clipper function
    softClipper.functionToUse = [](float x) { return std::tanh(x * 0.8f) / 0.8f; };

    compressor.setThreshold(-12.0f);
    compressor.setRatio(4.0f);
    compressor.setAttack(0.3f);
    compressor.setRelease(20.0f);

    exciter.state = juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100.0f, 8000.0f, 0.7f, 1.0f);

    multibandLimiter.setThreshold(-1.0f);
    multibandLimiter.setCeiling(-1.0f);

    gateProcessor.setThreshold(-40.0f);
    gateProcessor.setRatio(0.1f);

    juce::File irFile("Source/IRs/small_room.wav");
    if (irFile.existsAsFile())
        convolution.loadImpulseResponse(irFile, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0);
}

void BTZAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // #region agent log
    {
        juce::File logFile("c:\\Users\\marcm\\OneDrive\\Desktop\\btz-sonic-alchemy-main\\.cursor\\debug.log");
        if (logFile.getParentDirectory().exists()) {
            juce::String line = "{\"id\":\"btz_prep_1\",\"timestamp\":" + juce::String(juce::Time::getMillisecondCounter()) + ",\"location\":\"PluginProcessor.cpp:prepareToPlay\",\"message\":\"BTZ plugin prepared\",\"data\":{\"sampleRate\":"
                + juce::String(sampleRate) + ",\"samplesPerBlock\":" + juce::String(samplesPerBlock) + "},\"hypothesisId\":\"H4\"}\n";
            logFile.appendText(line);
        }
    }
    // #endregion
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() };

    transientShaper.prepare(spec);
    compressor.prepare(spec);
    gateProcessor.prepare(spec);
    waveShaper.prepare(spec);
    fuzzedDistortion.prepare(spec);
    tapeEmulator.prepare(spec);
    timbralTransfer.prepare(spec);
    eq.prepare(spec);
    bassEnhancer.prepare(spec);
    convolution.prepare(spec);
    delay.prepare(spec);
    granularProcessor.prepare(spec);
    exciter.prepare(spec);
    consoleEmulator.prepare(spec);
    softClipper.prepare(spec);
    multibandLimiter.prepare(spec);
    oversampler.initProcessing((juce::uint32) samplesPerBlock);
    oversampler.reset();
    deepFilter.prepare(spec);

    *exciter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 8000.0f, 0.7f, 1.0f);
}

void BTZAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float punch = *parameters.getRawParameterValue("punch");
    float warmth = *parameters.getRawParameterValue("warmth");
    float boom = *parameters.getRawParameterValue("boom");
    float mix = *parameters.getRawParameterValue("mix");
    bool texture = parameters.getParameter("texture")->getValue() > 0.5f;
    float drive = *parameters.getRawParameterValue("drive");

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // AI analysis (mocked)
    deepFilter.process(context);
    float transientStrength = deepFilter.getTransientStrength();
    float lowEndEnergy = deepFilter.getLowEndEnergy();
    float richness = timbralTransfer.getTimbralRichness();
    float loudnessScore = deepFilter.getLoudnessScore();

    aiTransientGain = 1.0f + transientStrength * punch * 0.7f;
    aiLowShelfGain = boom * 10.0f * (1.0f + lowEndEnergy * 0.5f);
    aiSaturationAmount = 1.0f + warmth * richness * 0.7f;
    aiLoudnessBoost = 1.0f + loudnessScore * 0.4f;
    aiExciterAmount = texture ? 1.0f + richness * 0.3f : 0.0f;
    aiGranularAmount = texture ? richness * 0.4f : 0.0f;

    // Dynamics
    transientShaper.setAttackGain(aiTransientGain * punch * 2.5f);
    transientShaper.setSustainGain(punch * 0.6f);
    transientShaper.process(context);

    compressor.setRatio(2.0f + punch * 8.0f);
    compressor.setThreshold(-12.0f - punch * 8.0f);
    compressor.process(context);

    gateProcessor.setThreshold(-40.0f + punch * 10.0f);
    gateProcessor.process(context);

    // Tone & color
    timbralTransfer.process(context);
    waveShaper.setGain(aiSaturationAmount * (1.0f + warmth * 3.0f));
    waveShaper.process(context);

    if (warmth > 0.5f) {
        fuzzedDistortion.setDrive(warmth * 3.0f);
        fuzzedDistortion.process(context);
    }

    tapeEmulator.setSaturation(warmth * 0.6f);
    tapeEmulator.setWowFlutter(warmth * 0.3f);
    tapeEmulator.process(context);

    // Low end
    bassEnhancer.setIntensity(boom * 0.7f);
    bassEnhancer.setFrequency(40.0f + boom * 30.0f);
    bassEnhancer.process(context);

    eq.setLowShelfGain(aiLowShelfGain);
    eq.setHighShelfGain(boom * 3.0f);
    eq.process(context);

    // Texture
    if (texture) {
        convolution.process(context);
        delay.setDelayTime(aiGranularAmount * 0.5f);
        delay.process(context);
        granularProcessor.setGrainSize(aiGranularAmount);
        granularProcessor.process(context);
        *exciter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), 8000.0f, 0.7f, aiExciterAmount);
        exciter.process(context);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n) data[n] *= 0.15f;
        }
    }

    // Console color & mix
    consoleEmulator.setDrive(mix * 0.5f);
    consoleEmulator.setCrosstalk(mix * 0.1f);
    consoleEmulator.process(context);

    // Simple wet/dry (here block already wet)
    // In a real plugin keep dry copy before processing

    // Drive stage w/ soft clip and limiter
    if (drive > 0.0f) {
        float gain = juce::Decibels::decibelsToGain(drive) * aiLoudnessBoost;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n) data[n] *= gain;
        }
        auto up = oversampler.processSamplesUp(block);
        juce::dsp::ProcessContextReplacing<float> upCtx(up);
        softClipper.process(upCtx);
        multibandLimiter.process(upCtx);
        oversampler.processSamplesDown(block);
    }
}

juce::AudioProcessorEditor* BTZAudioProcessor::createEditor() { return new BTZAudioProcessorEditor(*this); }

// Plugin host entry point (required for VST3/AU when using JUCE plugin client)
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BTZAudioProcessor();
}
