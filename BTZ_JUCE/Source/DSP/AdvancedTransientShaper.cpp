/*
  AdvancedTransientShaper.cpp

  Implementation of world-class transient shaping algorithms
  Based on open-source research and public algorithm descriptions
*/

#include "AdvancedTransientShaper.h"

AdvancedTransientShaper::AdvancedTransientShaper() = default;

void AdvancedTransientShaper::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    updateCoefficients();
}

void AdvancedTransientShaper::reset()
{
    peakEnvelope.fill(0.0f);
    rmsEnvelope.fill(0.0f);
    previousEnvelope.fill(0.0f);
    sustainEnvelope.fill(0.0f);
    adaptiveThreshold.fill(0.0f);

    for (auto& window : rmsWindow)
        window.fill(0.0f);

    rmsWriteIndex.fill(0);
}

void AdvancedTransientShaper::setPunch(float punchAmount)
{
    punchIntensity = juce::jlimit(0.0f, 1.0f, punchAmount);
    updateCoefficients();
}

void AdvancedTransientShaper::setAttackTime(float timeMs)
{
    attackTimeMs = juce::jlimit(0.1f, 50.0f, timeMs);
    updateCoefficients();
}

void AdvancedTransientShaper::setSustainTime(float timeMs)
{
    sustainTimeMs = juce::jlimit(10.0f, 500.0f, timeMs);
    updateCoefficients();
}

void AdvancedTransientShaper::setDetectionMode(DetectionMode mode)
{
    currentMode = mode;
}

void AdvancedTransientShaper::setFrequencyDependent(bool enabled)
{
    multiband = enabled;
}

void AdvancedTransientShaper::updateCoefficients()
{
    // Attack envelope coefficients (fast attack for transient detection)
    float attackTime = attackTimeMs * (1.0f - punchIntensity * 0.8f); // Faster with more punch
    attackCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * attackTime / 1000.0f));

    // Release envelope coefficients
    float releaseTime = attackTime * 5.0f; // Release longer than attack
    releaseCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * releaseTime / 1000.0f));

    // Sustain envelope (independent timing)
    float sustainAttack = sustainTimeMs * 0.1f;
    sustainAttackCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * sustainAttack / 1000.0f));

    float sustainRelease = sustainTimeMs;
    sustainReleaseCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * sustainRelease / 1000.0f));

    // UPDATE: Configure TPT filters (more stable than exponential)
    // Convert time constants to TPT cutoff frequencies
    for (size_t ch = 0; ch < 2; ++ch)
    {
        attackEnvFilter[ch].setCutoff(1000.0f / attackTime, sampleRate);
        releaseEnvFilter[ch].setCutoff(1000.0f / releaseTime, sampleRate);
        sustainAttackFilter[ch].setCutoff(1000.0f / sustainAttack, sampleRate);
        sustainReleaseFilter[ch].setCutoff(1000.0f / sustainRelease, sampleRate);
        adaptiveThresholdFilter[ch].setCutoff(0.1f, sampleRate); // Very slow (10s time constant)
    }
}

//=============================================================================
// PEAK ENVELOPE DETECTION
// Fast response, ideal for drum transients
// NOW USES: TPT filters for no frequency warping
//=============================================================================
float AdvancedTransientShaper::detectPeakEnvelope(float sample, size_t channel)
{
    float absSample = std::abs(sample);

    // Use TPT one-pole for attack/release (more stable than exponential)
    float envelope = (absSample > peakEnvelope[channel])
        ? attackEnvFilter[channel].process(absSample)
        : releaseEnvFilter[channel].process(absSample);

    peakEnvelope[channel] = envelope;
    return envelope;
}

//=============================================================================
// RMS ENVELOPE DETECTION
// Smoother, program-dependent detection
// NOW USES: TPT filters for smoothing
//=============================================================================
float AdvancedTransientShaper::detectRMSEnvelope(float sample, size_t channel)
{
    // Add sample to circular buffer
    auto& window = rmsWindow[channel];
    int& writeIdx = rmsWriteIndex[channel];

    window[writeIdx] = sample * sample; // Square
    writeIdx = (writeIdx + 1) % rmsWindowSize;

    // Calculate RMS
    float sumSquares = 0.0f;
    for (float square : window)
        sumSquares += square;

    float rms = std::sqrt(sumSquares / rmsWindowSize);

    // Smooth with TPT envelope follower (no frequency warping)
    float envelope = (rms > rmsEnvelope[channel])
        ? attackEnvFilter[channel].process(rms)
        : releaseEnvFilter[channel].process(rms);

    rmsEnvelope[channel] = envelope;
    return envelope;
}

//=============================================================================
// HALF-SPECTRAL ENVELOPE DETECTION
// Auburn Sounds Couture technique - frequency-aware transient detection
// Detects transients differently across frequency spectrum
//=============================================================================
float AdvancedTransientShaper::detectHalfSpectralEnvelope(float sample, size_t channel)
{
    // Simplified spectral detection (full implementation would use FFT)
    // This uses multiple envelope followers at different speeds to approximate

    float absSample = std::abs(sample);

    // Fast envelope (high frequencies / transients)
    static std::array<float, 2> fastEnv = {0.0f, 0.0f};
    float fastCoeff = 0.1f;
    fastEnv[channel] = fastCoeff * absSample + (1.0f - fastCoeff) * fastEnv[channel];

    // Slow envelope (low frequencies / body)
    static std::array<float, 2> slowEnv = {0.0f, 0.0f};
    float slowCoeff = 0.001f;
    slowEnv[channel] = slowCoeff * absSample + (1.0f - slowCoeff) * slowEnv[channel];

    // Combine: transients are where fast exceeds slow
    float spectralEnvelope = fastEnv[channel] * 0.7f + slowEnv[channel] * 0.3f;

    return spectralEnvelope;
}

//=============================================================================
// ADAPTIVE ENVELOPE DETECTION
// Flux BitterSweet technique - program-dependent, no internal thresholds
// Automatically adapts to material
// NOW USES: TPT filters for threshold adaptation and envelope following
//=============================================================================
float AdvancedTransientShaper::detectAdaptiveEnvelope(float sample, size_t channel)
{
    float absSample = std::abs(sample);

    // Update adaptive threshold using TPT filter (very slow, ~10s time constant)
    float threshold = adaptiveThresholdFilter[channel].process(absSample);
    adaptiveThreshold[channel] = threshold;

    // Normalize envelope relative to adaptive threshold
    float normalizedSample = (threshold > 0.0001f) ? (absSample / threshold) : absSample;

    // Peak detection with normalization using TPT
    float envelope = (normalizedSample > peakEnvelope[channel])
        ? attackEnvFilter[channel].process(normalizedSample)
        : releaseEnvFilter[channel].process(normalizedSample);

    peakEnvelope[channel] = envelope;
    return envelope;
}

//=============================================================================
// CALCULATE TRANSIENT GAIN
// Voxengo TransGainer approach: react on transients, not loudness
//=============================================================================
float AdvancedTransientShaper::calculateTransientGain(float envelope, size_t channel)
{
    // Detect transient delta (rising edge)
    float delta = envelope - previousEnvelope[channel];
    previousEnvelope[channel] = envelope;

    // Attack enhancement (transient boost)
    float attackGain = 1.0f;
    if (delta > 0.0f)
    {
        // Amplify rising edge (attack)
        attackGain = 1.0f + (delta * punchIntensity * 20.0f);
        attackGain = juce::jlimit(1.0f, 4.0f, attackGain);
    }

    // Sustain modification (separate envelope)
    float& sustain = sustainEnvelope[channel];
    if (envelope > 0.1f)
    {
        // Sustain is present
        sustain = sustainAttackCoeff * 1.0f + (1.0f - sustainAttackCoeff) * sustain;
    }
    else
    {
        // Decay
        sustain = (1.0f - sustainReleaseCoeff) * sustain;
    }

    // Sustain can reduce body while enhancing attack
    // BitterSweet "Sweet" mode reduces sustain, "Bitter" enhances attack
    float sustainGain = 1.0f - (sustain * punchIntensity * 0.3f);

    // Combine attack and sustain
    float totalGain = attackGain * sustainGain;

    return juce::jlimit(0.5f, 4.0f, totalGain);
}

//=============================================================================
// NON-LINEAR SCALING
// Transpire technique: smaller transients affected more than larger ones
// Brings out finer details without over-processing loud transients
//=============================================================================
float AdvancedTransientShaper::getNonLinearScale(float envelope)
{
    // Inverse relationship: quiet transients get more enhancement
    // Prevents over-processing of already loud transients

    if (envelope < 0.001f)
        return 1.0f; // Below noise floor

    // Logarithmic scaling
    float scale = 1.0f - (std::log10(envelope + 0.01f) / 2.0f);
    scale = juce::jlimit(0.5f, 1.5f, scale);

    // Apply punch intensity
    return 1.0f + (scale - 1.0f) * punchIntensity;
}
