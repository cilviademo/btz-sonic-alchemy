/*
  MeterStrip.h
  Enhanced metering: LUFS, True Peak, Gain Reduction, Stereo Correlation
  TODO: Implement custom meter rendering matching React UI
*/

#pragma once
#include <JuceHeader.h>

class MeterStrip : public juce::Component,
                   public juce::Timer
{
public:
    MeterStrip()
    {
        startTimerHz(30); // 30fps update
    }

    void setProcessor(class BTZAudioProcessor* proc)
    {
        processor = proc;
    }

    void paint(juce::Graphics& g) override
    {
        // TODO: Custom meter rendering with target zones
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.drawText("LUFS | Peak | GR | Stereo", getLocalBounds(), juce::Justification::centred);
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    class BTZAudioProcessor* processor = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterStrip)
};
