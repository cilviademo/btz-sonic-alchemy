/*
  MeterStrip.cpp
*/

#include "MeterStrip.h"
#include "../PluginProcessor.h"  // Need full definition for meter access

MeterStrip::MeterStrip()
{
    // QUICK WIN 5: Don't start timer in constructor - wait until visible
}

void MeterStrip::setProcessor(BTZAudioProcessor* proc)
{
    processor = proc;
}

void MeterStrip::paint(juce::Graphics& g)
{
    // TODO: Custom meter rendering with target zones
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);

    // Display cached meter values
    juce::String meterText = juce::String::formatted(
        "LUFS: %.1f | Peak: %.1f | GR: %.1f | Stereo: %.2f",
        cachedLUFS, cachedPeak, cachedGR, cachedStereo
    );
    g.drawText(meterText, getLocalBounds(), juce::Justification::centred);
}

void MeterStrip::timerCallback()
{
    if (processor == nullptr)
        return;

    // Read atomic meter values from processor
    float newLUFS = processor->getCurrentLUFS();
    float newPeak = processor->getCurrentPeak();
    float newGR = processor->getGainReduction();
    float newStereo = processor->getStereoCorrelation();

    // QUICK WIN 1: Only repaint if values changed significantly
    // This reduces repaints by ~90% when meters are stable
    bool needsRepaint = false;

    if (std::abs(newLUFS - cachedLUFS) > 0.1f)
    {
        cachedLUFS = newLUFS;
        needsRepaint = true;
    }

    if (std::abs(newPeak - cachedPeak) > 0.01f)
    {
        cachedPeak = newPeak;
        needsRepaint = true;
    }

    if (std::abs(newGR - cachedGR) > 0.01f)
    {
        cachedGR = newGR;
        needsRepaint = true;
    }

    if (std::abs(newStereo - cachedStereo) > 0.01f)
    {
        cachedStereo = newStereo;
        needsRepaint = true;
    }

    if (needsRepaint)
        repaint();
}

// QUICK WIN 5: Start/stop timer based on visibility (CPU optimization)
void MeterStrip::visibilityChanged()
{
    if (isVisible())
    {
        startTimerHz(30);  // 30fps update when visible
    }
    else
    {
        stopTimer();  // Stop wasting CPU when hidden
    }
}
