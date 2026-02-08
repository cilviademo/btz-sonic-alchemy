/*
  WDFSaturation.cpp

  Implementation of WDF-based analog circuit saturation
*/

#include "WDFSaturation.h"

WDFSaturation::WDFSaturation()
{
    // Initialize WDF components with default circuit (Tube 12AX7)
    for (size_t ch = 0; ch < 2; ++ch)
    {
        inputResistor[ch].setResistanceValue(1.0e6f);   // 1MΩ grid resistor
        couplingCap[ch] = WDF::Capacitor(0.1e-6f);      // 0.1µF coupling cap
        outputResistor[ch].setResistanceValue(100.0e3f); // 100kΩ plate load
    }
}

void WDFSaturation::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare all WDF components
    for (size_t ch = 0; ch < 2; ++ch)
    {
        couplingCap[ch].prepare(sampleRate);
        inputSource[ch].calcImpedance();
        inputResistor[ch].calcImpedance();
        diodeClipper[ch].calcImpedance();
        outputResistor[ch].calcImpedance();
    }

    reset();
    updateCircuitParameters();
}

void WDFSaturation::reset()
{
    toneZ1.fill(0.0f);

    // Reset all WDF state
    for (size_t ch = 0; ch < 2; ++ch)
    {
        inputSource[ch].incident(0.0f);
        couplingCap[ch].incident(0.0f);
    }
}

void WDFSaturation::setCircuitType(CircuitType type)
{
    currentCircuit = type;
    updateCircuitParameters();
}

void WDFSaturation::setDrive(float drive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, drive);
    updateCircuitParameters();
}

void WDFSaturation::setTone(float tone)
{
    toneAmount = juce::jlimit(0.0f, 1.0f, tone);
}

void WDFSaturation::setMix(float mix)
{
    mixAmount = juce::jlimit(0.0f, 1.0f, mix);
}

void WDFSaturation::updateCircuitParameters()
{
    // Configure circuit based on type and drive
    switch (currentCircuit)
    {
        case CircuitType::Tube12AX7:
        {
            // 12AX7 triode configuration
            // Higher grid resistor for tube input impedance
            for (auto& r : inputResistor)
                r.setResistanceValue(1.0e6f);  // 1MΩ

            // Triode "clipping" via diode approximation
            // Tubes produce primarily 2nd harmonic (even harmonics)
            for (auto& d : diodeClipper)
                d.setDiodeParams(1.0e-12f, 0.026f);  // Standard diode params

            break;
        }

        case CircuitType::Transformer:
        {
            // Iron-core output transformer
            // Saturation curve is softer than tubes
            for (auto& r : inputResistor)
                r.setResistanceValue(600.0f);  // 600Ω typical transformer impedance

            // Transformer saturation is asymmetric (iron hysteresis)
            for (auto& d : diodeClipper)
                d.setDiodeParams(5.0e-12f, 0.04f);  // Softer knee

            break;
        }

        case CircuitType::TransistorSi:
        {
            // Silicon transistor clipper (e.g., 2N3904)
            // Harder clipping, more asymmetric
            for (auto& r : inputResistor)
                r.setResistanceValue(10.0e3f);  // 10kΩ

            // Silicon diodes have sharper knee
            for (auto& d : diodeClipper)
                d.setDiodeParams(1.0e-14f, 0.026f);  // Sharper clipping

            break;
        }

        case CircuitType::TransistorGe:
        {
            // Germanium transistor (e.g., OC71, vintage fuzz)
            // Softer, warmer clipping
            for (auto& r : inputResistor)
                r.setResistanceValue(10.0e3f);  // 10kΩ

            // Germanium has lower forward voltage (~0.3V vs 0.7V)
            for (auto& d : diodeClipper)
                d.setDiodeParams(1.0e-11f, 0.015f);  // Lower Vf

            break;
        }

        case CircuitType::OpAmpNE5534:
        {
            // Modern low-noise op-amp (clean saturation)
            for (auto& r : inputResistor)
                r.setResistanceValue(10.0e3f);  // 10kΩ feedback

            // Op-amp soft clipping near rails
            for (auto& d : diodeClipper)
                d.setDiodeParams(1.0e-13f, 0.03f);  // Soft knee

            break;
        }

        case CircuitType::OpAmpTL072:
        {
            // Vintage op-amp (colored, warmer)
            for (auto& r : inputResistor)
                r.setResistanceValue(10.0e3f);  // 10kΩ feedback

            // TL072 has slightly more character
            for (auto& d : diodeClipper)
                d.setDiodeParams(1.0e-12f, 0.028f);  // Vintage character

            break;
        }
    }
}

float WDFSaturation::processCircuit(float input, size_t channel)
{
    // Scale input by drive amount
    float scaledInput = input * (1.0f + driveAmount * 20.0f);  // Up to +26dB

    // Set input voltage source
    inputSource[channel].setVoltage(scaledInput);

    // WDF circuit processing (simplified - full WDF would use adaptor tree)
    // For production, use full ChowDSP WDF library

    // 1. Input voltage source
    inputSource[channel].calcImpedance();
    float vIn = inputSource[channel].reflected();

    // 2. Through input resistor
    inputResistor[channel].incident(vIn);
    float vR1 = inputResistor[channel].reflected();

    // 3. Through coupling capacitor (AC coupling)
    couplingCap[channel].incident(vR1);
    float vCap = couplingCap[channel].reflected();

    // 4. Through non-linear element (diode clipper)
    diodeClipper[channel].incident(vCap);
    float vClipped = diodeClipper[channel].reflected();

    // 5. Through output resistor
    outputResistor[channel].incident(vClipped);
    float vOut = outputResistor[channel].reflected();

    // Compensate for gain changes
    float output = vOut * 0.5f;  // Reduce output level

    return output;
}

float WDFSaturation::applyToneControl(float sample, size_t channel)
{
    // Simple one-pole low-pass filter for tone control
    // toneAmount: 0.0 = dark (low cutoff), 1.0 = bright (high cutoff)

    // Map tone to cutoff frequency (200Hz - 10kHz)
    float cutoffHz = 200.0f + toneAmount * 9800.0f;
    float omega = juce::MathConstants<float>::twoPi * cutoffHz / static_cast<float>(sampleRate);
    float coeff = 1.0f - std::exp(-omega);

    // One-pole IIR filter
    float& z1 = toneZ1[channel];
    z1 = coeff * sample + (1.0f - coeff) * z1;

    // Mix between filtered (dark) and unfiltered (bright)
    return z1 + toneAmount * (sample - z1);
}
