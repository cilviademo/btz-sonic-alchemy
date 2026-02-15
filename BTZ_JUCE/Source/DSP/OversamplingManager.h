/*
  ==============================================================================

  OversamplingManager.h

  Centralized oversampling management for BTZ

  Features:
  - Configurable oversampling factors (1x, 2x, 4x, 8x, 16x)
  - Per-module enable/disable
  - Adaptive oversampling based on CPU load
  - Quality modes (draft, good, best)
  - Zero-latency mode for real-time use

  ==============================================================================
*/

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <array>

class OversamplingManager
{
public:
    enum class Quality
    {
        Draft,  // Fast, lower quality (Butterworth 2-pole)
        Good,   // Balanced (Butterworth 4-pole)
        Best    // Highest quality (Elliptic 12-pole)
    };

    enum class Factor
    {
        x1 = 0,   // No oversampling
        x2 = 1,   // 2x oversampling
        x4 = 2,   // 4x oversampling
        x8 = 3,   // 8x oversampling
        x16 = 4   // 16x oversampling
    };

    OversamplingManager();
    ~OversamplingManager() = default;

    // Setup
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void setQuality(Quality quality);
    void setFactor(Factor factor);
    void setAdaptive(bool enabled) { adaptiveEnabled = enabled; }

    // Processing
    juce::dsp::AudioBlock<float> processUp(juce::dsp::AudioBlock<float>& input);
    void processDown(juce::dsp::AudioBlock<float>& output);

    // Information
    int getOversamplingFactor() const;
    int getLatencySamples() const;
    double getOversampledSampleRate() const { return oversampledSampleRate; }
    bool isEnabled() const { return currentFactor != Factor::x1; }

    // Bypass
    void reset();

private:
    void updateOversampler();
    int getFactorValue(Factor f) const;

    Quality currentQuality = Quality::Good;
    Factor currentFactor = Factor::x1;
    bool adaptiveEnabled = false;

    double baseSampleRate = 48000.0;
    double oversampledSampleRate = 48000.0;
    int maxSamplesPerBlock = 512;
    int numChannels = 2;

    std::array<std::unique_ptr<juce::dsp::Oversampling<float>>, 5> oversamplers;
    juce::dsp::Oversampling<float>* activeOversampler = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OversamplingManager)
};
