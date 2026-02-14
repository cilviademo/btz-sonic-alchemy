/*
  ==============================================================================

  PresetManager.cpp

  P1.3: A/B/C Preset System Implementation

  ==============================================================================
*/

#include "PresetManager.h"
#include "../Parameters/PluginParameters.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts)
{
    // Initialize factory presets on construction
    initializeFactoryPresets();

    // Load factory "Default" preset into slot A
    if (factoryPresets.count("Default") > 0)
    {
        slots[static_cast<int>(Slot::A)] = factoryPresets["Default"];
        currentSlot = Slot::A;
    }
}

void PresetManager::saveToSlot(Slot slot)
{
    auto& targetSlot = slots[static_cast<int>(slot)];
    captureCurrentState(targetSlot);
    targetSlot.populated = true;
    currentSlot = slot;
}

void PresetManager::loadFromSlot(Slot slot)
{
    const auto& sourceSlot = slots[static_cast<int>(slot)];

    if (!sourceSlot.populated)
    {
        // Slot is empty - do nothing
        return;
    }

    applyPresetState(sourceSlot, true);  // true = with ramping
    currentSlot = slot;
}

bool PresetManager::isSlotPopulated(Slot slot) const
{
    return slots[static_cast<int>(slot)].populated;
}

void PresetManager::loadFactoryPreset(const juce::String& presetName)
{
    if (factoryPresets.count(presetName) == 0)
    {
        jassertfalse;  // Invalid preset name
        return;
    }

    const auto& preset = factoryPresets[presetName];
    applyPresetState(preset, true);  // true = with ramping
}

juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& pair : factoryPresets)
        names.add(pair.first);
    return names;
}

void PresetManager::processRamping(int numSamples)
{
    if (!ramping)
        return;

    rampingSamplesRemaining -= numSamples;

    if (rampingSamplesRemaining <= 0)
    {
        // Ramping complete - snap to final values
        for (const auto& pair : rampTargetValues)
        {
            const auto& paramID = pair.first;
            float targetValue = pair.second;

            auto* param = apvts.getParameter(paramID);
            if (param != nullptr)
            {
                param->setValueNotifyingHost(targetValue);
            }
        }

        ramping = false;
        rampingSamplesRemaining = 0;
        rampStartValues.clear();
        rampTargetValues.clear();
    }
    else
    {
        // Apply interpolated values
        float progress = 1.0f - (static_cast<float>(rampingSamplesRemaining) / rampingTotalSamples);

        for (const auto& pair : rampTargetValues)
        {
            const auto& paramID = pair.first;
            float targetValue = pair.second;
            float startValue = rampStartValues[paramID];

            float interpolatedValue = startValue + (targetValue - startValue) * progress;

            auto* param = apvts.getParameter(paramID);
            if (param != nullptr)
            {
                param->setValueNotifyingHost(interpolatedValue);
            }
        }
    }
}

float PresetManager::getRampProgress() const
{
    if (!ramping || rampingTotalSamples == 0)
        return 1.0f;

    return 1.0f - (static_cast<float>(rampingSamplesRemaining) / rampingTotalSamples);
}

juce::ValueTree PresetManager::getSlotData(Slot slot) const
{
    const auto& sourceSlot = slots[static_cast<int>(slot)];

    juce::ValueTree data("PresetSlot");
    data.setProperty("populated", sourceSlot.populated, nullptr);

    for (const auto& pair : sourceSlot.parameterValues)
    {
        juce::ValueTree paramNode("Parameter");
        paramNode.setProperty("id", pair.first, nullptr);
        paramNode.setProperty("value", pair.second, nullptr);
        data.appendChild(paramNode, nullptr);
    }

    return data;
}

void PresetManager::setSlotData(Slot slot, const juce::ValueTree& data)
{
    auto& targetSlot = slots[static_cast<int>(slot)];
    targetSlot.parameterValues.clear();

    targetSlot.populated = data.getProperty("populated", false);

    for (int i = 0; i < data.getNumChildren(); ++i)
    {
        auto paramNode = data.getChild(i);
        juce::String paramID = paramNode.getProperty("id", "");
        float value = paramNode.getProperty("value", 0.0f);

        if (paramID.isNotEmpty())
        {
            targetSlot.parameterValues[paramID] = value;
        }
    }
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

void PresetManager::captureCurrentState(PresetSlot& slot)
{
    slot.parameterValues.clear();

    // Capture all parameters from APVTS
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            juce::String paramID = paramWithID->paramID;
            float normalizedValue = paramWithID->getValue();
            slot.parameterValues[paramID] = normalizedValue;
        }
    }
}

void PresetManager::applyPresetState(const PresetSlot& slot, bool withRamping)
{
    if (withRamping)
    {
        startRamping(slot.parameterValues);
    }
    else
    {
        // Instant parameter change (no ramping)
        for (const auto& pair : slot.parameterValues)
        {
            const auto& paramID = pair.first;
            float value = pair.second;

            auto* param = apvts.getParameter(paramID);
            if (param != nullptr)
            {
                param->setValueNotifyingHost(value);
            }
        }
    }
}

void PresetManager::startRamping(const std::map<juce::String, float>& targetValues)
{
    // Capture current parameter values as ramp start
    rampStartValues.clear();
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            juce::String paramID = paramWithID->paramID;
            float currentValue = paramWithID->getValue();
            rampStartValues[paramID] = currentValue;
        }
    }

    rampTargetValues = targetValues;

    // Ramp duration: 20ms (click-free transition)
    double sampleRate = apvts.processor.getSampleRate();
    rampingTotalSamples = static_cast<int>(sampleRate * 0.020);  // 20ms
    rampingSamplesRemaining = rampingTotalSamples;
    ramping = true;
}

void PresetManager::initializeFactoryPresets()
{
    using namespace BTZParams::IDs;

    // =========================================================================
    // FACTORY PRESET 1: DEFAULT
    // =========================================================================
    {
        PresetSlot preset;
        preset.populated = true;

        // Hero controls - neutral
        preset.parameterValues[punch] = 0.0f;
        preset.parameterValues[warmth] = 0.0f;
        preset.parameterValues[boom] = 0.0f;
        preset.parameterValues[drive] = 0.0f;
        preset.parameterValues[mix] = 1.0f;  // 100% wet

        // I/O
        preset.parameterValues[inputGain] = 0.5f;   // 0 dB (normalized 0-1 range)
        preset.parameterValues[outputGain] = 0.5f;  // 0 dB
        preset.parameterValues[autoGain] = 0.0f;    // Off

        // SPARK - enabled with conservative settings
        preset.parameterValues[sparkEnabled] = 1.0f;   // On
        preset.parameterValues[sparkCeiling] = 0.9f;   // -0.3 dB (normalized)
        preset.parameterValues[sparkMix] = 1.0f;       // 100%
        preset.parameterValues[sparkOS] = 0.6f;        // 4x (normalized choice index)

        // SHINE - disabled
        preset.parameterValues[shineEnabled] = 0.0f;
        preset.parameterValues[shineGainDb] = 0.625f;  // 3 dB (normalized -12 to 12)

        // Master
        preset.parameterValues[active] = 1.0f;

        factoryPresets["Default"] = preset;
    }

    // =========================================================================
    // FACTORY PRESET 2: PUNCHY DRUMS
    // =========================================================================
    {
        PresetSlot preset;
        preset.populated = true;

        // Hero controls - emphasis on punch + warmth
        preset.parameterValues[punch] = 0.75f;    // 75% punch
        preset.parameterValues[warmth] = 0.45f;   // 45% warmth
        preset.parameterValues[boom] = 0.25f;     // 25% boom
        preset.parameterValues[drive] = 0.30f;    // 30% drive
        preset.parameterValues[mix] = 1.0f;       // 100% wet

        // I/O
        preset.parameterValues[inputGain] = 0.5f;   // 0 dB
        preset.parameterValues[outputGain] = 0.5f;  // 0 dB
        preset.parameterValues[autoGain] = 0.0f;

        // SPARK - enabled with aggressive settings
        preset.parameterValues[sparkEnabled] = 1.0f;
        preset.parameterValues[sparkCeiling] = 0.9f;   // -0.3 dB
        preset.parameterValues[sparkMix] = 1.0f;
        preset.parameterValues[sparkOS] = 0.6f;        // 4x

        // SHINE - moderate air
        preset.parameterValues[shineEnabled] = 1.0f;
        preset.parameterValues[shineGainDb] = 0.625f;  // 3 dB

        preset.parameterValues[active] = 1.0f;

        factoryPresets["Punchy Drums"] = preset;
    }

    // =========================================================================
    // FACTORY PRESET 3: WARM GLUE
    // =========================================================================
    {
        PresetSlot preset;
        preset.populated = true;

        // Hero controls - emphasis on warmth + saturation
        preset.parameterValues[punch] = 0.35f;    // 35% punch
        preset.parameterValues[warmth] = 0.80f;   // 80% warmth
        preset.parameterValues[boom] = 0.50f;     // 50% boom
        preset.parameterValues[drive] = 0.55f;    // 55% drive
        preset.parameterValues[mix] = 0.85f;      // 85% wet (blend with dry)

        // I/O
        preset.parameterValues[inputGain] = 0.5f;
        preset.parameterValues[outputGain] = 0.5f;
        preset.parameterValues[autoGain] = 0.0f;

        // SPARK - moderate limiting
        preset.parameterValues[sparkEnabled] = 1.0f;
        preset.parameterValues[sparkCeiling] = 0.85f;  // -0.5 dB
        preset.parameterValues[sparkMix] = 0.9f;       // 90%
        preset.parameterValues[sparkOS] = 0.4f;        // 2x

        // SHINE - disabled (focus on warmth)
        preset.parameterValues[shineEnabled] = 0.0f;
        preset.parameterValues[shineGainDb] = 0.5f;    // 0 dB

        preset.parameterValues[active] = 1.0f;

        factoryPresets["Warm Glue"] = preset;
    }

    // =========================================================================
    // FACTORY PRESET 4: BRIGHT LIFT
    // =========================================================================
    {
        PresetSlot preset;
        preset.populated = true;

        // Hero controls - emphasis on shine + punch
        preset.parameterValues[punch] = 0.60f;    // 60% punch
        preset.parameterValues[warmth] = 0.20f;   // 20% warmth
        preset.parameterValues[boom] = 0.15f;     // 15% boom
        preset.parameterValues[drive] = 0.25f;    // 25% drive
        preset.parameterValues[mix] = 1.0f;       // 100% wet

        // I/O
        preset.parameterValues[inputGain] = 0.5f;
        preset.parameterValues[outputGain] = 0.5f;
        preset.parameterValues[autoGain] = 0.0f;

        // SPARK - conservative
        preset.parameterValues[sparkEnabled] = 1.0f;
        preset.parameterValues[sparkCeiling] = 0.9f;   // -0.3 dB
        preset.parameterValues[sparkMix] = 1.0f;
        preset.parameterValues[sparkOS] = 0.6f;        // 4x

        // SHINE - high air boost
        preset.parameterValues[shineEnabled] = 1.0f;
        preset.parameterValues[shineGainDb] = 0.75f;   // 6 dB (normalized -12 to 12)

        preset.parameterValues[active] = 1.0f;

        factoryPresets["Bright Lift"] = preset;
    }

    // =========================================================================
    // FACTORY PRESET 5: DEEP WEIGHT
    // =========================================================================
    {
        PresetSlot preset;
        preset.populated = true;

        // Hero controls - emphasis on boom + warmth
        preset.parameterValues[punch] = 0.40f;    // 40% punch
        preset.parameterValues[warmth] = 0.70f;   // 70% warmth
        preset.parameterValues[boom] = 0.85f;     // 85% boom
        preset.parameterValues[drive] = 0.40f;    // 40% drive
        preset.parameterValues[mix] = 0.90f;      // 90% wet

        // I/O
        preset.parameterValues[inputGain] = 0.5f;
        preset.parameterValues[outputGain] = 0.45f;  // -0.6 dB (compensate for bass boost)
        preset.parameterValues[autoGain] = 0.0f;

        // SPARK - moderate
        preset.parameterValues[sparkEnabled] = 1.0f;
        preset.parameterValues[sparkCeiling] = 0.85f;  // -0.5 dB
        preset.parameterValues[sparkMix] = 0.95f;
        preset.parameterValues[sparkOS] = 0.4f;        // 2x

        // SHINE - disabled (focus on low end)
        preset.parameterValues[shineEnabled] = 0.0f;
        preset.parameterValues[shineGainDb] = 0.5f;    // 0 dB

        preset.parameterValues[active] = 1.0f;

        factoryPresets["Deep Weight"] = preset;
    }
}
