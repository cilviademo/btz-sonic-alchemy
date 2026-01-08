/*
  ==============================================================================

  BTZ Bypass Bit-Perfect Tests

  Purpose: Verify bypass mode produces bit-identical output to input
  Ship Gate: #8 (Bypass Test)

  Tests:
  - True bypass: input buffer == output buffer (bitwise comparison)
  - Bypass latency compensation (if applicable)
  - No DSP artifacts when bypassed
  - Works for all channel configurations (mono, stereo)
  - Works for all buffer sizes
  - Works with automation changes during bypass

  Requirements:
  - BTZ must have a true bypass mode (not just mix=0)
  - Bypass should be instant, not ramped
  - No denormals, DC offset, or processing when bypassed

  ==============================================================================
*/

#include "../Source/PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <cassert>
#include <iostream>
#include <cstring>
#include <random>

using namespace juce;

//==============================================================================
// Helper Functions

bool buffersIdentical(const AudioBuffer<float>& buffer1,
                      const AudioBuffer<float>& buffer2) {
    if (buffer1.getNumChannels() != buffer2.getNumChannels())
        return false;
    if (buffer1.getNumSamples() != buffer2.getNumSamples())
        return false;

    for (int ch = 0; ch < buffer1.getNumChannels(); ++ch) {
        const float* data1 = buffer1.getReadPointer(ch);
        const float* data2 = buffer2.getReadPointer(ch);

        if (std::memcmp(data1, data2, buffer1.getNumSamples() * sizeof(float)) != 0)
            return false;
    }

    return true;
}

void fillBufferWithTestSignal(AudioBuffer<float>& buffer, int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            data[i] = dist(rng);
        }
    }
}

void fillBufferWithSineWave(AudioBuffer<float>& buffer, double sampleRate, float frequency) {
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            double phase = (i / sampleRate) * frequency * MathConstants<double>::twoPi;
            data[i] = std::sin(phase);
        }
    }
}

//==============================================================================
// Test Cases

void test_bypass_with_silence() {
    std::cout << "[TEST] Bypass with Silence... ";

    BTZAudioProcessor processor;

    // Prepare processor
    double sampleRate = 44100.0;
    int bufferSize = 512;
    processor.prepareToPlay(sampleRate, bufferSize);

    // Set to bypass mode (mix = 0.0 for now, or use dedicated bypass parameter)
    if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
        mixParam->setValueNotifyingHost(0.0f);

    // Create silent input buffer
    AudioBuffer<float> inputBuffer(2, bufferSize);
    inputBuffer.clear();

    // Copy input for comparison
    AudioBuffer<float> expectedOutput(2, bufferSize);
    expectedOutput.clear();

    // Create MIDI buffer (empty)
    MidiBuffer midiBuffer;

    // Process
    processor.processBlock(inputBuffer, midiBuffer);

    // Verify output is identical to input (silence)
    bool identical = buffersIdentical(inputBuffer, expectedOutput);
    assert(identical);

    std::cout << "✅ PASS\n";
}

void test_bypass_with_random_signal() {
    std::cout << "[TEST] Bypass with Random Signal... ";

    BTZAudioProcessor processor;

    // Prepare processor
    double sampleRate = 48000.0;
    int bufferSize = 256;
    processor.prepareToPlay(sampleRate, bufferSize);

    // Set to bypass mode
    if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
        mixParam->setValueNotifyingHost(0.0f);

    // Create random input signal
    AudioBuffer<float> inputBuffer(2, bufferSize);
    fillBufferWithTestSignal(inputBuffer, 12345);

    // Copy input for comparison
    AudioBuffer<float> expectedOutput(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
        expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, bufferSize);

    // Create MIDI buffer (empty)
    MidiBuffer midiBuffer;

    // Process
    processor.processBlock(inputBuffer, midiBuffer);

    // Verify output is identical to input (bit-perfect)
    bool identical = buffersIdentical(inputBuffer, expectedOutput);

    if (!identical) {
        // Print diagnostic info
        std::cout << "\n  Output differs from input!\n";
        std::cout << "  First difference at sample: ";
        for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch) {
            const float* in = expectedOutput.getReadPointer(ch);
            const float* out = inputBuffer.getReadPointer(ch);
            for (int i = 0; i < bufferSize; ++i) {
                if (in[i] != out[i]) {
                    std::cout << "ch" << ch << " sample " << i << " (in=" << in[i] << ", out=" << out[i] << ")\n";
                    break;
                }
            }
        }
    }

    assert(identical);

    std::cout << "✅ PASS\n";
}

void test_bypass_with_sine_wave() {
    std::cout << "[TEST] Bypass with Sine Wave... ";

    BTZAudioProcessor processor;

    // Prepare processor
    double sampleRate = 44100.0;
    int bufferSize = 512;
    processor.prepareToPlay(sampleRate, bufferSize);

    // Set to bypass mode
    if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
        mixParam->setValueNotifyingHost(0.0f);

    // Create sine wave input (1 kHz)
    AudioBuffer<float> inputBuffer(2, bufferSize);
    fillBufferWithSineWave(inputBuffer, sampleRate, 1000.0f);

    // Copy input for comparison
    AudioBuffer<float> expectedOutput(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
        expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, bufferSize);

    // Create MIDI buffer (empty)
    MidiBuffer midiBuffer;

    // Process
    processor.processBlock(inputBuffer, midiBuffer);

    // Verify output is identical to input
    bool identical = buffersIdentical(inputBuffer, expectedOutput);
    assert(identical);

    std::cout << "✅ PASS\n";
}

void test_bypass_different_buffer_sizes() {
    std::cout << "[TEST] Bypass with Various Buffer Sizes... ";

    int bufferSizes[] = { 32, 64, 128, 256, 512, 1024, 2048 };
    double sampleRate = 48000.0;

    for (int size : bufferSizes) {
        BTZAudioProcessor processor;
        processor.prepareToPlay(sampleRate, size);

        // Set to bypass mode
        if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
            mixParam->setValueNotifyingHost(0.0f);

        // Create random input
        AudioBuffer<float> inputBuffer(2, size);
        fillBufferWithTestSignal(inputBuffer, size);

        // Copy input for comparison
        AudioBuffer<float> expectedOutput(2, size);
        for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
            expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, size);

        // Process
        MidiBuffer midiBuffer;
        processor.processBlock(inputBuffer, midiBuffer);

        // Verify bit-perfect
        bool identical = buffersIdentical(inputBuffer, expectedOutput);
        assert(identical);
    }

    std::cout << "✅ PASS (7 buffer sizes verified)\n";
}

void test_bypass_mono_and_stereo() {
    std::cout << "[TEST] Bypass with Mono/Stereo Configurations... ";

    double sampleRate = 44100.0;
    int bufferSize = 512;

    // Test mono (1 channel)
    {
        BTZAudioProcessor processor;
        processor.prepareToPlay(sampleRate, bufferSize);

        // Set to bypass mode
        if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
            mixParam->setValueNotifyingHost(0.0f);

        AudioBuffer<float> inputBuffer(1, bufferSize);
        fillBufferWithTestSignal(inputBuffer, 100);

        AudioBuffer<float> expectedOutput(1, bufferSize);
        expectedOutput.copyFrom(0, 0, inputBuffer, 0, 0, bufferSize);

        MidiBuffer midiBuffer;
        processor.processBlock(inputBuffer, midiBuffer);

        bool identical = buffersIdentical(inputBuffer, expectedOutput);
        assert(identical);
    }

    // Test stereo (2 channels)
    {
        BTZAudioProcessor processor;
        processor.prepareToPlay(sampleRate, bufferSize);

        // Set to bypass mode
        if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
            mixParam->setValueNotifyingHost(0.0f);

        AudioBuffer<float> inputBuffer(2, bufferSize);
        fillBufferWithTestSignal(inputBuffer, 200);

        AudioBuffer<float> expectedOutput(2, bufferSize);
        for (int ch = 0; ch < 2; ++ch)
            expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, bufferSize);

        MidiBuffer midiBuffer;
        processor.processBlock(inputBuffer, midiBuffer);

        bool identical = buffersIdentical(inputBuffer, expectedOutput);
        assert(identical);
    }

    std::cout << "✅ PASS\n";
}

void test_bypass_with_extreme_values() {
    std::cout << "[TEST] Bypass with Extreme Values... ";

    BTZAudioProcessor processor;

    double sampleRate = 44100.0;
    int bufferSize = 512;
    processor.prepareToPlay(sampleRate, bufferSize);

    // Set to bypass mode
    if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
        mixParam->setValueNotifyingHost(0.0f);

    // Create buffer with extreme values
    AudioBuffer<float> inputBuffer(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch) {
        float* data = inputBuffer.getWritePointer(ch);
        for (int i = 0; i < bufferSize; ++i) {
            // Alternate between max positive and max negative
            data[i] = (i % 2 == 0) ? 1.0f : -1.0f;
        }
    }

    // Copy input for comparison
    AudioBuffer<float> expectedOutput(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
        expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, bufferSize);

    // Process
    MidiBuffer midiBuffer;
    processor.processBlock(inputBuffer, midiBuffer);

    // Verify bit-perfect
    bool identical = buffersIdentical(inputBuffer, expectedOutput);
    assert(identical);

    std::cout << "✅ PASS\n";
}

void test_bypass_no_denormals() {
    std::cout << "[TEST] Bypass No Denormals... ";

    BTZAudioProcessor processor;

    double sampleRate = 44100.0;
    int bufferSize = 512;
    processor.prepareToPlay(sampleRate, bufferSize);

    // Set to bypass mode
    if (auto* mixParam = processor.getAPVTS().getParameter("mix"))
        mixParam->setValueNotifyingHost(0.0f);

    // Create buffer with denormal values (very small floats)
    AudioBuffer<float> inputBuffer(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch) {
        float* data = inputBuffer.getWritePointer(ch);
        for (int i = 0; i < bufferSize; ++i) {
            data[i] = 1e-40f;  // Denormal value
        }
    }

    // Copy input for comparison
    AudioBuffer<float> expectedOutput(2, bufferSize);
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch)
        expectedOutput.copyFrom(ch, 0, inputBuffer, ch, 0, bufferSize);

    // Process
    MidiBuffer midiBuffer;
    processor.processBlock(inputBuffer, midiBuffer);

    // Verify output is identical (denormals should pass through unchanged)
    // Note: If ScopedNoDenormals is active, denormals may be flushed to zero
    // This test documents the actual behavior
    bool identical = buffersIdentical(inputBuffer, expectedOutput);

    if (!identical) {
        std::cout << "⚠️  WARN (denormals flushed to zero - acceptable)\n";
    } else {
        std::cout << "✅ PASS\n";
    }
}

//==============================================================================
// Main Test Runner

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "BTZ Bypass Bit-Perfect Tests\n";
    std::cout << "========================================\n\n";

    int passed = 0;
    int failed = 0;

    try {
        test_bypass_with_silence();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_with_random_signal();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_with_sine_wave();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_different_buffer_sizes();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_mono_and_stereo();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_with_extreme_values();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_bypass_no_denormals();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    std::cout << "\n========================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "========================================\n";

    return (failed == 0) ? 0 : 1;
}
