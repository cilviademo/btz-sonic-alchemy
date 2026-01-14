/*
  ==============================================================================

  SafetyLayer.h

  Phase 1.4: Safety Layer
  - DC blocker (5Hz HPF) to prevent DC buildup
  - Denormal guard (FTZ/DAZ + noise injection)
  - NaN/Inf handler with RT-safe logging
  - ClickFreeSwitch for bypass without pops

  Features:
  - Topology-Preserving Transform (TPT) DC blocker
  - Aggressive denormal prevention (FTZ mode + noise)
  - NaN/Inf detection and correction without crashes
  - RT-safe error logging for debugging
  - Sample-accurate bypass switching

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

/*
  ==============================================================================
  DCBlocker - High-pass filter at 5Hz to remove DC offset
  ==============================================================================
*/

class DCBlocker
{
public:
    DCBlocker() = default;

    void prepare(double sampleRate);
    void reset();

    // Process single sample
    float processSample(float input, int channel);

    // Process buffer in-place
    void process(juce::AudioBuffer<float>& buffer);

private:
    double sampleRate = 48000.0;
    static constexpr float cutoffHz = 5.0f;

    // TPT first-order highpass state (per channel)
    struct State
    {
        float s = 0.0f; // State variable
    };
    std::array<State, 2> state;

    float g = 0.0f; // TPT coefficient

    void updateCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DCBlocker)
};

/*
  ==============================================================================
  DenormalGuard - Prevents denormal numbers that kill CPU performance
  ==============================================================================
*/

class DenormalGuard
{
public:
    DenormalGuard();

    // Enable/disable FTZ and DAZ modes
    static void enableFlushToZero();
    static void disableFlushToZero();

    // Process buffer: adds ultra-quiet noise to prevent denormals
    static void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float denormalNoise = 1.0e-25f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DenormalGuard)
};

/*
  ==============================================================================
  NaNInfHandler - Detects and corrects NaN/Inf without crashing
  ==============================================================================
*/

class NaNInfHandler
{
public:
    NaNInfHandler();

    // Check and fix NaN/Inf in buffer
    // Returns true if any NaN/Inf were found
    bool checkAndFix(juce::AudioBuffer<float>& buffer);

    // Get diagnostic info
    int getNaNCount() const { return nanCount.load(); }
    int getInfCount() const { return infCount.load(); }
    void resetCounts();

private:
    std::atomic<int> nanCount{0};
    std::atomic<int> infCount{0};

    // Logging (RT-safe)
    void logError(const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaNInfHandler)
};

/*
  ==============================================================================
  ClickFreeSwitch - Sample-accurate bypass without clicks/pops
  ==============================================================================
*/

class ClickFreeSwitch
{
public:
    ClickFreeSwitch();

    void prepare(double sampleRate);
    void reset();

    // Set target state (true = active, false = bypassed)
    void setActive(bool shouldBeActive);

    // Get current state
    bool isActive() const { return targetActive; }
    bool isRamping() const { return ramping; }

    // Process: mix dry/wet based on ramp state
    void process(juce::AudioBuffer<float>& dryBuffer,
                 const juce::AudioBuffer<float>& wetBuffer);

private:
    bool targetActive = true;
    bool ramping = false;
    float currentGain = 1.0f;
    float targetGain = 1.0f;

    static constexpr float rampTimeMs = 10.0f; // 10ms crossfade
    float rampIncrement = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClickFreeSwitch)
};

/*
  ==============================================================================
  CompositeSafetyLayer - All safety features in one convenient class
  ==============================================================================
*/

class CompositeSafetyLayer
{
public:
    CompositeSafetyLayer();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Enable/disable individual layers
    void setDCBlockEnabled(bool enabled) { dcBlockEnabled = enabled; }
    void setDenormalGuardEnabled(bool enabled) { denormalGuardEnabled = enabled; }
    void setNaNInfCheckEnabled(bool enabled) { nanInfCheckEnabled = enabled; }

    // Process buffer through all enabled safety layers
    void process(juce::AudioBuffer<float>& buffer);

    // Get diagnostics
    int getNaNCount() const { return nanInfHandler.getNaNCount(); }
    int getInfCount() const { return nanInfHandler.getInfCount(); }
    void resetDiagnostics() { nanInfHandler.resetCounts(); }

private:
    bool dcBlockEnabled = true;
    bool denormalGuardEnabled = true;
    bool nanInfCheckEnabled = true;

    DCBlocker dcBlocker;
    DenormalGuard denormalGuard;
    NaNInfHandler nanInfHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositeSafetyLayer)
};
