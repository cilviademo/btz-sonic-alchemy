/*
  ==============================================================================

  DeterministicProcessing.cpp

  ==============================================================================
*/

#include "DeterministicProcessing.h"

// ============================================================================
// DeterministicRandom Implementation
// ============================================================================

DeterministicRandom::DeterministicRandom()
{
    // Initialize with default seed
    setSeed(12345);
}

void DeterministicRandom::setSeed(uint64_t seed)
{
    currentSeed = seed;
    rng.seed(seed);
}

void DeterministicRandom::reset()
{
    rng.seed(currentSeed); // Reset to initial seed
}

float DeterministicRandom::nextFloat()
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng);
}

float DeterministicRandom::nextFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int DeterministicRandom::nextInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// ============================================================================
// RenderModeDetector Implementation
// ============================================================================

RenderModeDetector::RenderModeDetector() = default;

void RenderModeDetector::prepare(double sampleRate)
{
    lastSampleRate = sampleRate;
    lastTimeInSamples = -1;
    consecutiveNonRealtimeBlocks = 0;
    currentMode = Mode::Unknown;
}

void RenderModeDetector::update(const juce::AudioPlayHead::PositionInfo& posInfo)
{
    // Heuristic 1: Check if timeInSamples is advancing consistently
    // In offline mode, time advances predictably without gaps

    bool isPlaying = posInfo.getIsPlaying();
    auto timeInSamples = posInfo.getTimeInSamples();

    if (!isPlaying || !timeInSamples.hasValue())
    {
        // Not playing or no time info - remain in current mode
        return;
    }

    int64_t currentTime = *timeInSamples;

    if (lastTimeInSamples >= 0)
    {
        int64_t timeDelta = currentTime - lastTimeInSamples;

        // In realtime mode, there may be gaps, jitter, or irregular advancement
        // In offline mode, time advances exactly by blockSize each processBlock

        // Check if advancing very consistently (offline render characteristic)
        bool consistentAdvancement = (timeDelta > 0 && timeDelta < 8192); // Reasonable block size

        if (consistentAdvancement)
        {
            consecutiveNonRealtimeBlocks++;

            if (consecutiveNonRealtimeBlocks >= offlineDetectionThreshold)
            {
                // Likely offline render
                if (currentMode != Mode::Offline)
                    currentMode = Mode::Offline;
            }
        }
        else
        {
            // Irregular advancement suggests realtime
            consecutiveNonRealtimeBlocks = 0;
            if (currentMode != Mode::Realtime)
                currentMode = Mode::Realtime;
        }
    }

    lastTimeInSamples = currentTime;
}

// ============================================================================
// StateCaptureSystem Implementation
// ============================================================================

StateCaptureSystem::StateCaptureSystem() = default;

void StateCaptureSystem::captureState(const juce::String& label, const juce::AudioBuffer<float>& buffer)
{
    if (!captureEnabled)
        return;

    auto state = analyzeBuffer(label, buffer);
    capturedStates.add(state);
}

juce::String StateCaptureSystem::exportStates() const
{
    juce::String output = "=== BTZ State Capture Report ===\n\n";

    for (const auto& state : capturedStates)
    {
        output += "Label: " + state.label + "\n";
        output += "Timestamp: " + juce::String(state.timestamp) + "\n";
        output += "RMS: " + juce::String(state.rmsLevel, 6) + "\n";
        output += "Peak: " + juce::String(state.peakLevel, 6) + "\n";
        output += "DC Offset: " + juce::String(state.dcOffset, 6) + "\n";
        output += "Spectral Fingerprint: [";

        for (int i = 0; i < state.spectralFingerprint.size(); ++i)
        {
            output += juce::String(state.spectralFingerprint[i], 4);
            if (i < state.spectralFingerprint.size() - 1)
                output += ", ";
        }

        output += "]\n\n";
    }

    return output;
}

void StateCaptureSystem::clear()
{
    capturedStates.clear();
}

StateCaptureSystem::CapturedState StateCaptureSystem::analyzeBuffer(
    const juce::String& label, const juce::AudioBuffer<float>& buffer)
{
    CapturedState state;
    state.label = label;
    state.timestamp = juce::Time::currentTimeMillis();

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (numSamples == 0 || channels == 0)
        return state;

    // Calculate RMS and peak
    float sumSquares = 0.0f;
    float peak = 0.0f;
    float dcSum = 0.0f;

    for (int ch = 0; ch < channels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = data[i];
            sumSquares += sample * sample;
            peak = std::max(peak, std::abs(sample));
            dcSum += sample;
        }
    }

    state.rmsLevel = std::sqrt(sumSquares / (numSamples * channels));
    state.peakLevel = peak;
    state.dcOffset = dcSum / (numSamples * channels);

    // Simple spectral fingerprint (4 frequency bands)
    // This is a simplified analysis - real FFT would be better
    state.spectralFingerprint.clear();

    // For now, just store some placeholder values
    // In production, you'd do proper FFT analysis here
    state.spectralFingerprint.add(state.rmsLevel * 0.8f); // "Low"
    state.spectralFingerprint.add(state.rmsLevel * 1.0f); // "Mid-low"
    state.spectralFingerprint.add(state.rmsLevel * 0.9f); // "Mid-high"
    state.spectralFingerprint.add(state.rmsLevel * 0.6f); // "High"

    return state;
}

// ============================================================================
// CompositeDeterministicProcessing Implementation
// ============================================================================

CompositeDeterministicProcessing::CompositeDeterministicProcessing() = default;

void CompositeDeterministicProcessing::prepare(double sampleRate)
{
    modeDetector.prepare(sampleRate);
}

void CompositeDeterministicProcessing::update(const juce::AudioPlayHead::PositionInfo& posInfo)
{
    modeDetector.update(posInfo);
}

void CompositeDeterministicProcessing::setGlobalSeed(uint64_t seed)
{
    random.setSeed(seed);
}
