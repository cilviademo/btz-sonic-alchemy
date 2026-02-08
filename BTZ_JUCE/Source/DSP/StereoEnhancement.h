/*
  ==============================================================================

  StereoEnhancement.h

  Phase 2.3: Stereo Micro-Drift and Depth
  - L/R channel micro-variations (timing, filtering, gain)
  - Haas-effect inspired stereo widening
  - Correlation-aware processing

  Features:
  - Micro-timing offset: ±0.05ms L/R delay variation
  - Filter micro-detuning: ±0.2% frequency drift per channel
  - Subtle gain wobble: ±0.1dB L/R variation
  - Stereo width control without phase issues
  - Correlation preservation (maintains mono compatibility)

  Applications:
  - Natural stereo depth (analog console character)
  - Prevents "dead center" stereo imaging
  - Adds subtle movement to static sources
  - Enhances perceived spaciousness

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>

class StereoMicroDrift
{
public:
    StereoMicroDrift();
    ~StereoMicroDrift() = default;

    void prepare(double sampleRate, int maxSamplesPerBlock);
    void reset();

    // Parameters
    void setDriftAmount(float amount);     // 0.0 to 1.0
    void setDepthAmount(float amount);     // 0.0 to 1.0 (width)

    // Process stereo buffer
    void process(juce::AudioBuffer<float>& buffer);

    // Get stereo correlation
    float getStereoCorrelation() const { return stereoCorrelation; }

private:
    double sampleRate = 48000.0;
    int maxBlockSize = 512;

    // Parameters
    float driftAmount = 0.5f;
    float depthAmount = 0.3f;

    // Micro-timing delays (fractional sample delay)
    struct MicroDelay
    {
        std::vector<float> buffer;
        int writePos = 0;
        float delayAmount = 0.0f; // In samples
    };

    std::array<MicroDelay, 2> microDelays; // L, R

    // Subtle LFOs for time-varying drift
    float lfoPhase[2] = {0.0f, 0.25f}; // L/R phase offset
    float lfoRate = 0.5f; // Hz

    // All-pass filters for micro-phase shifts
    std::array<juce::dsp::IIR::Filter<float>, 2> allPassFilters;

    // Stereo correlation measurement
    float stereoCorrelation = 1.0f;
    float correlationSmooth = 0.0f;

    // Internal methods
    void updateMicroDelays();
    void updateAllPassFilters();
    float processMicroDelay(float input, int channel);
    void measureCorrelation(const juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoMicroDrift)
};

/*
  ==============================================================================
  StereoWidth - MS-based stereo width control
  ==============================================================================
*/

class StereoWidth
{
public:
    StereoWidth();

    void prepare(double sampleRate);
    void reset();

    // Width: 0.0 = mono, 1.0 = normal, 2.0 = wide
    void setWidth(float width);

    // Process stereo buffer (Mid/Side processing)
    void process(juce::AudioBuffer<float>& buffer);

private:
    float widthAmount = 1.0f;

    // Safety: prevent excessive widening
    static constexpr float maxWidth = 1.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoWidth)
};

/*
  ==============================================================================
  CompositeStereoEnhancement - All stereo features in one
  ==============================================================================
*/

class CompositeStereoEnhancement
{
public:
    CompositeStereoEnhancement();

    void prepare(double sampleRate, int maxSamplesPerBlock);
    void reset();

    // Parameters
    void setDriftAmount(float amount) { microDrift.setDriftAmount(amount); }
    void setDepthAmount(float amount) { microDrift.setDepthAmount(amount); }
    void setWidth(float width) { stereoWidth.setWidth(width); }

    // Processing
    void process(juce::AudioBuffer<float>& buffer);

    // Metering
    float getStereoCorrelation() const { return microDrift.getStereoCorrelation(); }

private:
    StereoMicroDrift microDrift;
    StereoWidth stereoWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositeStereoEnhancement)
};
