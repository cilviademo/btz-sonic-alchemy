/*
  ==============================================================================

  PresetManager.h

  P1.3: A/B/C Preset System with Click-Free Switching

  Features:
  - 3 preset slots (A/B/C) for instant recall
  - Click-free switching via parameter ramping (10-30ms)
  - Factory presets (Default, Punchy Drums, Warm Glue, Bright Lift, Deep Weight)
  - RT-safe: no allocations in audio thread
  - APVTS integration for all parameters

  Architecture:
  - PresetSlot: Stores snapshot of all APVTS parameters
  - PresetManager: Manages 3 slots + ramping logic
  - RampedParameterSet: Handles click-free transitions

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <map>
#include <string>

class PresetManager
{
public:
    enum class Slot
    {
        A = 0,
        B = 1,
        C = 2
    };

    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;

    // Preset slot management
    void saveToSlot(Slot slot);                     // Save current state to A/B/C
    void loadFromSlot(Slot slot);                   // Load from A/B/C (with ramping)
    Slot getCurrentSlot() const { return currentSlot; }
    bool isSlotPopulated(Slot slot) const;

    // Factory presets
    void loadFactoryPreset(const juce::String& presetName);
    juce::StringArray getFactoryPresetNames() const;

    // Parameter ramping (call from processBlock for click-free switching)
    void processRamping(int numSamples);
    bool isRamping() const { return ramping; }
    float getRampProgress() const;  // 0.0 to 1.0

    // Preset data import/export (for future preset browser)
    juce::ValueTree getSlotData(Slot slot) const;
    void setSlotData(Slot slot, const juce::ValueTree& data);

private:
    juce::AudioProcessorValueTreeState& apvts;

    // Preset slots
    struct PresetSlot
    {
        bool populated = false;
        std::map<juce::String, float> parameterValues;  // parameter ID → value
    };
    std::array<PresetSlot, 3> slots;  // A, B, C
    Slot currentSlot = Slot::A;

    // Ramping state
    bool ramping = false;
    int rampingSamplesRemaining = 0;
    int rampingTotalSamples = 0;
    std::map<juce::String, float> rampStartValues;  // Starting values for current ramp
    std::map<juce::String, float> rampTargetValues; // Target values for current ramp

    // Factory presets
    void initializeFactoryPresets();
    std::map<juce::String, PresetSlot> factoryPresets;

    // Internal helpers
    void captureCurrentState(PresetSlot& slot);
    void applyPresetState(const PresetSlot& slot, bool withRamping);
    void startRamping(const std::map<juce::String, float>& targetValues);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
