/*
  ==============================================================================

  DeterministicProcessing.h

  Phase 3.3: Deterministic Processing Guarantee
  - Seeded randomness for component variance
  - Offline render detection
  - Consistent results between RT and offline
  - Reproducible processing

  Features:
  - Deterministic PRNG seeding
  - Offline vs realtime mode detection
  - Timestamp-independent processing
  - Bit-exact reproducibility
  - State capture for regression testing

  Applications:
  - Identical results in DAW offline bounce
  - Reproducible A/B comparisons
  - Regression testing with golden audio files
  - Consistent behavior across sessions

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>
#include <cstdint>

class DeterministicRandom
{
public:
    DeterministicRandom();

    // Set seed for reproducibility
    void setSeed(uint64_t seed);
    uint64_t getSeed() const { return currentSeed; }

    // Generate random values (always deterministic given same seed)
    float nextFloat();                      // 0.0 to 1.0
    float nextFloat(float min, float max);  // min to max
    int nextInt(int min, int max);          // min to max (inclusive)

    // Reset to initial state
    void reset();

private:
    uint64_t currentSeed = 0;
    std::mt19937_64 rng; // 64-bit Mersenne Twister

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeterministicRandom)
};

/*
  ==============================================================================
  RenderModeDetector - Detects offline vs realtime rendering
  ==============================================================================
*/

class RenderModeDetector
{
public:
    enum class Mode
    {
        Realtime,       // Normal DAW playback
        Offline,        // Bounce/export
        Unknown
    };

    RenderModeDetector();

    void prepare(double sampleRate);

    // Call once per processBlock
    void update(const juce::AudioPlayHead::PositionInfo& posInfo);

    Mode getCurrentMode() const { return currentMode; }
    bool isOffline() const { return currentMode == Mode::Offline; }
    bool isRealtime() const { return currentMode == Mode::Realtime; }

private:
    Mode currentMode = Mode::Unknown;

    // Heuristics for offline detection
    double lastSampleRate = 0.0;
    int64_t lastTimeInSamples = -1;
    int consecutiveNonRealtimeBlocks = 0;

    static constexpr int offlineDetectionThreshold = 3; // blocks

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderModeDetector)
};

/*
  ==============================================================================
  StateCaptureSystem - Captures processing state for regression testing
  ==============================================================================
*/

class StateCaptureSystem
{
public:
    StateCaptureSystem();

    // Enable/disable capture
    void setEnabled(bool enabled) { captureEnabled = enabled; }
    bool isEnabled() const { return captureEnabled; }

    // Capture state at current point
    void captureState(const juce::String& label, const juce::AudioBuffer<float>& buffer);

    // Export captured states for comparison
    juce::String exportStates() const;

    // Clear all captured states
    void clear();

private:
    bool captureEnabled = false;

    struct CapturedState
    {
        juce::String label;
        juce::int64 timestamp;
        float rmsLevel;
        float peakLevel;
        float dcOffset;
        juce::Array<float> spectralFingerprint; // Simple FFT bins
    };

    juce::Array<CapturedState> capturedStates;

    CapturedState analyzeBuffer(const juce::String& label, const juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StateCaptureSystem)
};

/*
  ==============================================================================
  CompositeDeterministicProcessing - All determinism features
  ==============================================================================
*/

class CompositeDeterministicProcessing
{
public:
    CompositeDeterministicProcessing();

    void prepare(double sampleRate);
    void update(const juce::AudioPlayHead::PositionInfo& posInfo);

    // Randomness
    DeterministicRandom& getRandom() { return random; }
    void setGlobalSeed(uint64_t seed);

    // Render mode
    bool isOfflineRender() const { return modeDetector.isOffline(); }
    RenderModeDetector::Mode getRenderMode() const { return modeDetector.getCurrentMode(); }

    // State capture (for testing)
    StateCaptureSystem& getStateCapture() { return stateCapture; }

private:
    DeterministicRandom random;
    RenderModeDetector modeDetector;
    StateCaptureSystem stateCapture;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositeDeterministicProcessing)
};
