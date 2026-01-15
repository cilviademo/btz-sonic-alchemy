/*
  MeterStrip.h
  Enhanced metering: LUFS, True Peak, Gain Reduction, Stereo Correlation

  QUICK WIN 1: Optimized timer - only repaints when meter values change
  QUICK WIN 5: Stops timer when component is hidden (CPU optimization)
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class MeterStrip : public juce::Component,
                   public juce::Timer
{
public:
    MeterStrip();
    ~MeterStrip() override = default;

    void setProcessor(class BTZAudioProcessor* proc);
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void visibilityChanged() override;

private:
    class BTZAudioProcessor* processor = nullptr;

    // Cached meter values to avoid unnecessary repaints
    float cachedLUFS = -23.0f;
    float cachedPeak = 0.0f;
    float cachedGR = 1.0f;
    float cachedStereo = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterStrip)
};
