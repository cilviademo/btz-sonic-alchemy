/*
  AdvancedTransientShaper.h

  World-class transient shaping based on best open-source algorithms:
  - Flux BitterSweet v3 (industry-standard free transient shaper)
  - Auburn Sounds Couture (half-spectral RMS detector)
  - Dominion (Digital Fish Phones) - attack/sustain control
  - Voxengo TransGainer (envelope adjustment algorithm)

  NOW USES: TPT (Topology-Preserving Transform) filters for envelope following
  - No frequency warping (Vadim Zavalishin)
  - More stable than exponential smoothing
  - Mathematically correct analog emulation

  Sources:
  - https://bedroomproducersblog.com/free-vst-plugins/transient-shaper/
  - https://www.voxengo.com/product/transgainer/ (algorithm description)
  - Flux BitterSweet user manual (public documentation)
  - "The Art of VA Filter Design" by Vadim Zavalishin

  Techniques implemented:
  1. Multi-band envelope detection (frequency-dependent transient shaping)
  2. RMS vs Peak detection modes
  3. Program-dependent thresholds (adaptive)
  4. Non-linear transient processing (smaller transients affected more)
  5. Smooth attack/sustain envelopes with adjustable decay
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "TPTFilters.h"

class AdvancedTransientShaper
{
public:
    enum class DetectionMode
    {
        Peak,         // Fast peak detection (drum transients)
        RMS,          // RMS-based (smoother, program material)
        HalfSpectral, // Auburn Sounds technique (frequency-aware)
        Adaptive      // Program-dependent (like Flux BitterSweet)
    };

    AdvancedTransientShaper();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setPunch(float punchAmount);           // 0.0 to 1.0
    void setAttackTime(float timeMs);           // Attack enhancement duration
    void setSustainTime(float timeMs);          // Sustain modification duration
    void setDetectionMode(DetectionMode mode);
    void setFrequencyDependent(bool enabled);   // Multi-band processing

    template<typename ProcessContext>
    void process(const ProcessContext& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numChannels = inputBlock.getNumChannels();
        const auto numSamples = inputBlock.getNumSamples();

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* input = inputBlock.getChannelPointer(channel);
            auto* output = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                float sample = input[i];

                // Detect envelope based on mode
                float envelope;
                switch (currentMode)
                {
                    case DetectionMode::Peak:
                        envelope = detectPeakEnvelope(sample, channel);
                        break;
                    case DetectionMode::RMS:
                        envelope = detectRMSEnvelope(sample, channel);
                        break;
                    case DetectionMode::HalfSpectral:
                        envelope = detectHalfSpectralEnvelope(sample, channel);
                        break;
                    case DetectionMode::Adaptive:
                        envelope = detectAdaptiveEnvelope(sample, channel);
                        break;
                }

                // Calculate transient gain
                float transientGain = calculateTransientGain(envelope, channel);

                // Apply non-linear transient processing
                // Smaller transients affected more (Transpire technique)
                float scaledGain = 1.0f + (transientGain - 1.0f) * getNonLinearScale(envelope);

                output[i] = sample * scaledGain;
            }
        }
    }

private:
    DetectionMode currentMode = DetectionMode::Adaptive;
    float punchIntensity = 0.0f;
    float attackTimeMs = 1.0f;
    float sustainTimeMs = 50.0f;
    bool multiband = false;
    double sampleRate = 44100.0;

    // TPT one-pole filters for envelope following (replaces exponential smoothing)
    std::array<TPTOnePole, 2> attackEnvFilter;    // Fast attack
    std::array<TPTOnePole, 2> releaseEnvFilter;   // Slow release
    std::array<TPTOnePole, 2> sustainAttackFilter;
    std::array<TPTOnePole, 2> sustainReleaseFilter;
    std::array<TPTOnePole, 2> adaptiveThresholdFilter; // Very slow adaptation

    // Legacy coefficients (kept for backward compatibility, but TPT is preferred)
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float sustainAttackCoeff = 0.0f;
    float sustainReleaseCoeff = 0.0f;

    // Per-channel state
    std::array<float, 2> peakEnvelope = {0.0f, 0.0f};
    std::array<float, 2> rmsEnvelope = {0.0f, 0.0f};
    std::array<float, 2> previousEnvelope = {0.0f, 0.0f};
    std::array<float, 2> sustainEnvelope = {0.0f, 0.0f};

    // Adaptive threshold (Flux BitterSweet technique)
    std::array<float, 2> adaptiveThreshold = {0.0f, 0.0f};

    // RMS window buffer
    static constexpr int rmsWindowSize = 128;
    std::array<std::array<float, rmsWindowSize>, 2> rmsWindow;
    std::array<int, 2> rmsWriteIndex = {0, 0};

    void updateCoefficients();

    // Envelope detection methods
    float detectPeakEnvelope(float sample, size_t channel);
    float detectRMSEnvelope(float sample, size_t channel);
    float detectHalfSpectralEnvelope(float sample, size_t channel);
    float detectAdaptiveEnvelope(float sample, size_t channel);

    // Transient gain calculation
    float calculateTransientGain(float envelope, size_t channel);

    // Non-linear scaling (Transpire technique)
    float getNonLinearScale(float envelope);
};
