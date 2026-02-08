/*
  AdvancedSaturation.h

  World-class saturation combining the best open-source algorithms:
  - Airwindows Spiral (2024 - smoothest distortion, fixes zero-crossing discontinuities)
  - Airwindows Density (sine-based saturation, infinitely smooth)
  - Airwindows PurestDrive (hugely popular, dedicated fans)
  - ChowDSP analog tape modeling techniques

  Sources:
  - https://www.airwindows.com/spiral/
  - https://www.airwindows.com/density-vst/
  - https://www.airwindows.com/purestdrive-vst/
  - https://github.com/jatinchowdhury18/AnalogTapeModel

  All algorithms implemented from public descriptions under MIT license.
  NO proprietary code - all techniques from open-source analysis.
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AdvancedSaturation
{
public:
    enum class Mode
    {
        Spiral,       // Smoothest, fixes zero-crossing discontinuities (Airwindows 2024)
        Density,      // Sine-based, infinitely smooth transfer function
        PurestDrive,  // Musical drive with natural harmonics
        Tape,         // Analog tape saturation with hysteresis
        Transformer,  // Transformer saturation (even harmonics)
        Tube          // Tube-style (2nd + 3rd harmonics)
    };

    AdvancedSaturation();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setWarmth(float warmthAmount);  // 0.0 to 1.0
    void setMode(Mode mode);
    void setDrive(float drive);          // Additional gain staging

    // P1.1: Adaptive behavior
    void applyComponentVariance(const class ComponentVariance& variance);  // Apply curve/harmonic variance
    void applyAdaptiveDrive(float programLoudness);  // Reduce drive when already hot

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

                // Apply mode-specific saturation
                float saturated;
                switch (currentMode)
                {
                    case Mode::Spiral:
                        saturated = spiralSaturation(sample, channel);
                        break;
                    case Mode::Density:
                        saturated = densitySaturation(sample);
                        break;
                    case Mode::PurestDrive:
                        saturated = purestDriveSaturation(sample, channel);
                        break;
                    case Mode::Tape:
                        saturated = tapeSaturation(sample, channel);
                        break;
                    case Mode::Transformer:
                        saturated = transformerSaturation(sample);
                        break;
                    case Mode::Tube:
                        saturated = tubeSaturation(sample);
                        break;
                }

                // DC blocker (essential for any saturation)
                dcBlockerOutput[channel] = saturated - dcBlockerInput[channel] + 0.995f * dcBlockerOutput[channel];
                dcBlockerInput[channel] = saturated;

                output[i] = dcBlockerOutput[channel];
            }
        }
    }

private:
    Mode currentMode = Mode::Spiral;
    float warmthIntensity = 0.0f;
    float driveAmount = 1.0f;
    double sampleRate = 44100.0;

    // P1.1: Adaptive state
    float curveVariance = 1.0f;        // Drive curve variance (0.985 to 1.015)
    float harmonicVariance = 1.0f;     // Harmonic balance variance
    float adaptiveDriveScale = 1.0f;   // Loudness-based drive reduction (1.0 = no reduction)

    // DC blocker state
    std::array<float, 2> dcBlockerInput = {0.0f, 0.0f};
    std::array<float, 2> dcBlockerOutput = {0.0f, 0.0f};

    // Tape hysteresis state (for tape mode)
    std::array<float, 2> hysteresisState = {0.0f, 0.0f};

    // PurestDrive state
    std::array<float, 2> purestDriveState = {0.0f, 0.0f};

    //=== AIRWINDOWS SPIRAL (2024) ===
    // Smoothest distortion, fixes sine-based discontinuities at zero crossing
    // Source: https://www.airwindows.com/spiral/
    float spiralSaturation(float input, size_t channel);

    //=== AIRWINDOWS DENSITY ===
    // Sine-based transfer function - infinitely smooth
    // Source: https://www.airwindows.com/density-vst/
    float densitySaturation(float input);

    //=== AIRWINDOWS PURESTDRIVE ===
    // Hugely popular, natural harmonic generation
    // Source: https://www.airwindows.com/purestdrive-vst/
    float purestDriveSaturation(float input, size_t channel);

    //=== ANALOG TAPE SATURATION ===
    // Hysteresis modeling inspired by ChowDSP AnalogTapeModel
    // Source: https://github.com/jatinchowdhury18/AnalogTapeModel
    float tapeSaturation(float input, size_t channel);

    //=== TRANSFORMER SATURATION ===
    // Even-harmonic rich (2nd, 4th harmonics)
    float transformerSaturation(float input);

    //=== TUBE SATURATION ===
    // Classic tube distortion (2nd + 3rd harmonics)
    float tubeSaturation(float input);
};
