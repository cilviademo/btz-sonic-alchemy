/*
  ConsoleEmulator.cpp
*/

#include "ConsoleEmulator.h"

ConsoleEmulator::ConsoleEmulator() = default;

void ConsoleEmulator::prepare(const juce::dsp::ProcessSpec& spec)
{
    reset();
}

void ConsoleEmulator::reset()
{
    // No state to reset for this simple implementation
}

void ConsoleEmulator::setType(Type type)
{
    currentType = type;
}

void ConsoleEmulator::setMix(float mix)
{
    mixAmount = juce::jlimit(0.0f, 1.0f, mix);
}
