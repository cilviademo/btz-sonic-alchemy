/*
  BTZ Plugin Parameters
  All parameter IDs, ranges, and default values
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace BTZParams
{
    // Parameter IDs
    namespace IDs
    {
        // Hero controls (5 main knobs)
        const juce::String punch     = "punch";
        const juce::String warmth    = "warmth";
        const juce::String boom      = "boom";
        const juce::String mix       = "mix";
        const juce::String drive     = "drive";

        // Texture toggle
        const juce::String texture   = "texture";

        // I/O Trim
        const juce::String inputGain  = "inputGain";
        const juce::String outputGain = "outputGain";
        const juce::String autoGain   = "autoGain";

        // SPARK - Advanced Clipping Engine
        const juce::String sparkEnabled = "sparkEnabled";
        const juce::String sparkLUFS    = "sparkLUFS";
        const juce::String sparkCeiling = "sparkCeiling";
        const juce::String sparkMix     = "sparkMix";
        const juce::String sparkOS      = "sparkOS";
        const juce::String sparkAutoOS  = "sparkAutoOS";
        const juce::String sparkMode    = "sparkMode";

        // SHINE - Ultra-High Frequency Air
        const juce::String shineEnabled = "shineEnabled";
        const juce::String shineFreqHz  = "shineFreqHz";
        const juce::String shineGainDb  = "shineGainDb";
        const juce::String shineQ       = "shineQ";
        const juce::String shineMix     = "shineMix";
        const juce::String shineAutoOS  = "shineAutoOS";

        // Master
        const juce::String masterEnabled = "masterEnabled";
        const juce::String masterMacro   = "masterMacro";
        const juce::String masterBlend   = "masterBlend";
        const juce::String masterMix     = "masterMix";

        // Precision mode
        const juce::String precisionMode = "precisionMode";

        // Plugin state
        const juce::String active        = "active";
        const juce::String oversampling  = "oversampling";
    }

    // Create parameter layout for AudioProcessorValueTreeState
    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        // Hero controls
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::punch, "Punch", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::warmth, "Warmth", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::boom, "Boom", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::mix, "Mix", 0.0f, 1.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::drive, "Drive", 0.0f, 1.0f, 0.0f));

        // Texture
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::texture, "Texture", false));

        // I/O Trim
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::inputGain, "Input Gain", -12.0f, 12.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::outputGain, "Output Gain", -12.0f, 12.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::autoGain, "Auto Gain", false));

        // SPARK - Advanced Clipping Engine
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::sparkEnabled, "Spark Enabled", true));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::sparkLUFS, "Spark Target LUFS", -14.0f, 0.0f, -5.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::sparkCeiling, "Spark Ceiling", -3.0f, 0.0f, -0.3f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::sparkMix, "Spark Mix", 0.0f, 1.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            IDs::sparkOS, "Spark Oversampling",
            juce::StringArray{"1x", "2x", "4x", "8x", "16x"}, 3)); // Default 8x
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::sparkAutoOS, "Spark Auto OS", true));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            IDs::sparkMode, "Spark Mode",
            juce::StringArray{"Soft", "Hard"}, 0)); // Default Soft

        // SHINE - Ultra-High Frequency Air
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::shineEnabled, "Shine Enabled", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::shineFreqHz, "Shine Frequency", 10000.0f, 80000.0f, 20000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::shineGainDb, "Shine Gain", -12.0f, 12.0f, 3.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::shineQ, "Shine Q", 0.1f, 2.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::shineMix, "Shine Mix", 0.0f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::shineAutoOS, "Shine Auto OS", true));

        // Master
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::masterEnabled, "Master Enabled", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::masterMacro, "Master Macro", 0.0f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            IDs::masterBlend, "Master Blend",
            juce::StringArray{"Transparent", "Glue", "Vintage"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            IDs::masterMix, "Master Mix", 0.0f, 1.0f, 1.0f));

        // Precision mode
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::precisionMode, "Precision Mode", false));

        // Plugin state
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::active, "Active", true));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            IDs::oversampling, "Oversampling", true));

        return layout;
    }
}
