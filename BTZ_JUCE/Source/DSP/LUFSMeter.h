/*
  LUFSMeter.h

  ITU-R BS.1770-4 compliant LUFS metering
  Based on libebur128 reference implementation

  Sources:
  - https://github.com/jiixyj/libebur128 (MIT License)
  - https://github.com/klangfreund/LUFSMeter (GPLv3, reference implementation)
  - ITU-R BS.1770-4 specification (public standard)
  - EBU R 128 specification

  Implements:
  1. K-weighting filter (high-shelf +4dB @ 1.5kHz + high-pass @ 38Hz)
  2. Mean square calculation with 400ms integration
  3. Absolute gating (-70 LUFS) and relative gating (-10 LU)
  4. Integrated loudness over time
  5. Loudness range (LRA)

  This is a PROPER implementation, not the fake RMS-based one in the original code!
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <deque>

class LUFSMeter
{
public:
    LUFSMeter();

    void prepare(double sampleRate, int numChannels);
    void reset();

    // Process audio and update metering
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Get current LUFS values
    float getMomentaryLUFS() const { return momentaryLUFS.load(); }    // 400ms
    float getShortTermLUFS() const { return shortTermLUFS.load(); }   // 3s
    float getIntegratedLUFS() const { return integratedLUFS.load(); } // Since start/reset

    // Loudness range (dynamic range in LU)
    float getLoudnessRange() const { return loudnessRange.load(); }

    // Reset integrated measurement
    void resetIntegrated();

private:
    double currentSampleRate = 44100.0;
    int numChannels = 2;

    //=== K-WEIGHTING FILTERS (ITU-R BS.1770-4) ===

    // Stage 1: High-shelf filter (+4dB @ 1.5kHz, Q=0.7)
    struct BiquadCoeffs
    {
        float b0, b1, b2, a1, a2;
    };

    BiquadCoeffs highShelfCoeffs;
    std::vector<std::array<float, 3>> highShelfState; // z^-1, z^-2 per channel

    // Stage 2: High-pass filter (38Hz, Q=0.5) - removes DC and subsonic
    BiquadCoeffs highPassCoeffs;
    std::vector<std::array<float, 3>> highPassState;

    //=== GATING BLOCKS (400ms integration) ===

    static constexpr int blockSizeMs = 400;
    static constexpr float absoluteGate = -70.0f;    // LUFS
    static constexpr float relativeGate = -10.0f;    // LU

    struct GatingBlock
    {
        float loudness;    // Block loudness in LUFS
        int64_t timestamp; // Sample position
    };

    std::deque<GatingBlock> momentaryBlocks;  // 400ms blocks
    std::deque<GatingBlock> shortTermBlocks;  // 3s blocks
    std::vector<GatingBlock> integratedBlocks; // All blocks for integrated

    int blockSizeSamples = 0;
    int samplesInCurrentBlock = 0;
    std::vector<float> currentBlockPower; // Mean square accumulator per channel

    //=== OUTPUT VALUES (atomic for thread-safe GUI access) ===

    std::atomic<float> momentaryLUFS { -70.0f };
    std::atomic<float> shortTermLUFS { -70.0f };
    std::atomic<float> integratedLUFS { -70.0f };
    std::atomic<float> loudnessRange { 0.0f };

    //=== HELPER FUNCTIONS ===

    void calculateFilterCoefficients();

    // Apply K-weighting to sample
    float applyKWeighting(float sample, size_t channel);

    // Calculate loudness from mean square power
    float powerToLUFS(float power);

    // Gating algorithms
    float calculateGatedLoudness(const std::vector<GatingBlock>& blocks);
    float calculateLoudnessRange(const std::vector<GatingBlock>& blocks);
};
