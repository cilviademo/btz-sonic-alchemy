/*
  ==============================================================================

  EnhancedSHINE.cpp

  ==============================================================================
*/

#include "EnhancedSHINE.h"

EnhancedSHINE::EnhancedSHINE()
{
    // Initialize shelf bands with default frequencies
    shelfBands[0].centerFreq = 10000.0f;  // Presence
    shelfBands[1].centerFreq = 20000.0f;  // Air
    shelfBands[2].centerFreq = 40000.0f;  // Ultra-air

    for (auto& band : shelfBands)
        band.gain = 1.0f;
}

void EnhancedSHINE::prepare(double sr, int samplesPerBlock, int channels)
{
    sampleRate = sr;
    numChannels = juce::jmin(channels, 2);

    // Prepare parameter smoother
    shineAmountSmooth.prepare(sampleRate, 0.02f); // 20ms ramp
    shineAmountSmooth.reset(shineAmount);

    // Calculate masking coefficients
    maskingAttackCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * maskingAttackMs * 0.001f));
    maskingReleaseCoeff = 1.0f - std::exp(-1.0f / (float(sampleRate) * maskingReleaseMs * 0.001f));

    // Initialize Bark band filters
    initializeBarkBands();

    // Prepare shelf filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)channels;

    for (auto& band : shelfBands)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            band.filter[ch].prepare(spec);
            band.filter[ch].reset();
        }
    }

    updateShelfCoefficients();
    reset();
}

void EnhancedSHINE::reset()
{
    // Reset all filters
    for (auto& band : shelfBands)
    {
        for (int ch = 0; ch < 2; ++ch)
            band.filter[ch].reset();
    }

    for (auto& band : barkBands)
    {
        band.energy = 0.0f;
        band.threshold = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            band.bandpassFilter[ch].reset();
    }

    // Reset temporal masking
    for (auto& state : temporalState)
    {
        state.transientEnvelope = 0.0f;
        state.maskingReduction = 0.0f;
        state.lastSample = 0.0f;
    }

    hfEnergyDb = -96.0f;
}

void EnhancedSHINE::setShineAmount(float amount)
{
    shineAmount = juce::jlimit(0.0f, 1.0f, amount);
    shineAmountSmooth.setTarget(shineAmount);
    updateShelfCoefficients();
}

void EnhancedSHINE::setFrequencyCenter(float freqHz)
{
    frequencyCenter = juce::jlimit(10000.0f, 40000.0f, freqHz);
    updateShelfCoefficients();
}

void EnhancedSHINE::setEnabled(bool enabled)
{
    isEnabled = enabled;
}

void EnhancedSHINE::setPsychoacousticMode(bool enabled)
{
    psychoacousticEnabled = enabled;
}

void EnhancedSHINE::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled)
        return;

    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    if (numSamples == 0 || channels == 0)
        return;

    // Analyze critical bands for psychoacoustic processing
    if (psychoacousticEnabled)
    {
        analyzeCriticalBands(buffer);
        updateTemporalMasking(buffer);
    }

    // Calculate adaptive gain reduction based on spectral masking
    float spectralMaskingFactor = psychoacousticEnabled ? calculateSpectralMaskingFactor() : 1.0f;
    float temporalMaskingFactor = 1.0f;

    // Apply triple-band shelving with masking
    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        // Get temporal masking factor for this channel
        if (psychoacousticEnabled)
            temporalMaskingFactor = 1.0f - temporalState[ch].maskingReduction;

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = data[i];
            float processed = sample;

            // Apply each shelf band
            float shineGain = shineAmountSmooth.getNext();

            // Process through all three bands
            for (auto& band : shelfBands)
            {
                processed = band.filter[ch].processSample(processed);
            }

            // Apply psychoacoustic masking
            float masked = sample + (processed - sample) * shineGain * spectralMaskingFactor * temporalMaskingFactor;

            data[i] = masked;
        }
    }

    // Update HF energy metering
    float hfEnergy = 0.0f;
    for (int ch = 0; ch < channels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            hfEnergy += data[i] * data[i];
    }
    hfEnergy = std::sqrt(hfEnergy / (numSamples * channels));
    hfEnergyDb = juce::Decibels::gainToDecibels(hfEnergy, -96.0f);
}

void EnhancedSHINE::updateShelfCoefficients()
{
    // Design high-shelf filters for each band
    // Gain scales with shine amount: 0.0 = 0dB, 1.0 = +6dB

    auto designShelf = [this](ShelfBand& band)
    {
        float gainDb = shineAmount * 6.0f; // 0 to +6dB
        float gainLinear = juce::Decibels::decibelsToGain(gainDb);

        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate,
            band.centerFreq,
            0.707f, // Q = 1/sqrt(2) for Butterworth response
            gainLinear
        );

        for (int ch = 0; ch < 2; ++ch)
            *band.filter[ch].coefficients = *coeffs;
    };

    for (auto& band : shelfBands)
        designShelf(band);
}

void EnhancedSHINE::initializeBarkBands()
{
    // Initialize 24 critical bands on Bark scale
    // Bark bands span from ~20Hz to Nyquist

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 2;

    for (int i = 0; i < numBarkBands; ++i)
    {
        float bark = float(i + 1); // Bark 1-24
        float centerHz = barkToHz(bark);
        float bandwidth = 100.0f + 0.1f * centerHz; // Bark band bandwidth increases with frequency

        // Design bandpass filter for this Bark band
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate,
            centerHz,
            bandwidth / centerHz // Q from bandwidth
        );

        for (int ch = 0; ch < 2; ++ch)
        {
            barkBands[i].bandpassFilter[ch].prepare(spec);
            *barkBands[i].bandpassFilter[ch].coefficients = *coeffs;
        }
    }
}

float EnhancedSHINE::hzToBark(float hz) const
{
    // Traunmüller's formula for Hz to Bark conversion
    return 26.81f * hz / (1960.0f + hz) - 0.53f;
}

float EnhancedSHINE::barkToHz(float bark) const
{
    // Inverse of Traunmüller's formula
    return 1960.0f * (bark + 0.53f) / (26.81f - bark - 0.53f);
}

void EnhancedSHINE::analyzeCriticalBands(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    // Measure energy in each Bark band
    for (int bandIdx = 0; bandIdx < numBarkBands; ++bandIdx)
    {
        float bandEnergy = 0.0f;

        for (int ch = 0; ch < channels; ++ch)
        {
            const float* data = buffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float filtered = barkBands[bandIdx].bandpassFilter[ch].processSample(data[i]);
                bandEnergy += filtered * filtered;
            }
        }

        // Average and smooth
        bandEnergy = std::sqrt(bandEnergy / (numSamples * channels));
        barkBands[bandIdx].energy += 0.1f * (bandEnergy - barkBands[bandIdx].energy);
    }

    // Calculate masking thresholds based on spectral content
    // Higher energy in low bands masks higher bands
    for (int i = 0; i < numBarkBands; ++i)
    {
        float maskingSum = 0.0f;

        // Upward spread of masking (lower bands mask higher bands)
        for (int j = 0; j < i; ++j)
        {
            float distance = float(i - j);
            float spreadDb = -27.0f + distance * 3.0f; // Spread function
            maskingSum += barkBands[j].energy * juce::Decibels::decibelsToGain(spreadDb);
        }

        barkBands[i].threshold = maskingSum;
    }
}

void EnhancedSHINE::updateTemporalMasking(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), 2);

    for (int ch = 0; ch < channels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        auto& state = temporalState[ch];

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = data[i];

            // Detect transients using derivative (high-frequency content)
            float derivative = std::abs(sample - state.lastSample);
            state.lastSample = sample;

            // Update transient envelope
            float attack = derivative > state.transientEnvelope ? maskingAttackCoeff : maskingReleaseCoeff;
            state.transientEnvelope += attack * (derivative - state.transientEnvelope);

            // Map transient strength to HF reduction (0 = no reduction, 1 = full reduction)
            // Strong transients trigger masking
            float transientStrength = juce::jlimit(0.0f, 1.0f, state.transientEnvelope * 20.0f);
            state.maskingReduction += 0.01f * (transientStrength - state.maskingReduction);
        }
    }
}

float EnhancedSHINE::calculateSpectralMaskingFactor() const
{
    // Calculate adaptive HF gain reduction based on spectral masking
    // If high-frequency bands are already energetic, reduce SHINE effect

    // Find HF band energy (Bark bands 20-24 are > 10kHz)
    float hfBandEnergy = 0.0f;
    for (int i = 20; i < numBarkBands; ++i)
        hfBandEnergy += barkBands[i].energy;

    // Normalize and invert (high energy = less boost needed)
    float factor = 1.0f - juce::jlimit(0.0f, 0.5f, hfBandEnergy * 5.0f);

    return factor;
}
