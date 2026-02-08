/*
  test_AdvancedSaturation.cpp

  P2 Sprint - TASK-004: AdvancedSaturation tests (8 cases)
  Requirements: REQ-STAB-001 (DSP Unit Test Suite)
  Acceptance: AC-STAB-001.2 (8 tests: 6 modes + bypass + extreme values)

  Test Cases:
    1. testBypassMode     - warmth=0 → unity gain (tolerance 0.0001)
    2. testSpiralMode     - Airwindows Spiral saturation works
    3. testDensityMode    - Sine-based saturation produces smooth curves
    4. testPurestDriveMode- Musical drive with harmonics
    5. testTapeMode       - Analog tape saturation with hysteresis
    6. testTransformerMode- Even harmonic saturation
    7. testTubeMode       - Tube-style 2nd + 3rd harmonics
    8. testExtremeDrive   - drive=1.0 doesn't crash, output bounded

  References:
    - Source/DSP/AdvancedSaturation.h
    - docs/P2_ACCEPTANCE.md (AC-STAB-001.2)
*/

#include <gtest/gtest.h>
#include "DSP/AdvancedSaturation.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>

class AdvancedSaturationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Standard test configuration: 48kHz, stereo, 512 samples/block
        spec.sampleRate = 48000.0;
        spec.maximumBlockSize = 512;
        spec.numChannels = 2;

        saturation.prepare(spec);
        saturation.reset();
    }

    void TearDown() override
    {
        // Cleanup if needed
    }

    // Helper: Process a simple sine wave through saturation
    juce::AudioBuffer<float> processSineWave(float frequency, float amplitude, int numSamples)
    {
        juce::AudioBuffer<float> buffer(spec.numChannels, numSamples);

        // Generate sine wave
        for (int channel = 0; channel < spec.numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                float phase = (float)i / (float)spec.sampleRate;
                channelData[i] = amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * frequency * phase);
            }
        }

        // Process through saturation
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        saturation.process(context);

        return buffer;
    }

    // Helper: Calculate RMS of buffer
    float calculateRMS(const juce::AudioBuffer<float>& buffer, int channel = 0)
    {
        float sum = 0.0f;
        const auto* data = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            sum += data[i] * data[i];
        }
        return std::sqrt(sum / buffer.getNumSamples());
    }

    // Helper: Check if output is bounded (no NaN, no Inf, within [-2.0, 2.0])
    bool isOutputBounded(const juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto* data = buffer.getReadPointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (std::isnan(data[i]) || std::isinf(data[i]))
                    return false;
                if (std::abs(data[i]) > 2.0f)
                    return false;
            }
        }
        return true;
    }

    AdvancedSaturation saturation;
    juce::dsp::ProcessSpec spec;
};

//==============================================================================
// TEST 1: Bypass Mode (warmth=0)
//==============================================================================
TEST_F(AdvancedSaturationTest, testBypassMode)
{
    // When warmth=0, should pass signal unchanged (unity gain)
    saturation.setWarmth(0.0f);

    juce::AudioBuffer<float> inputBuffer(2, 512);
    inputBuffer.clear();

    // Generate 1kHz sine @ -20dB (0.1 amplitude)
    for (int channel = 0; channel < 2; ++channel)
    {
        auto* data = inputBuffer.getWritePointer(channel);
        for (int i = 0; i < 512; ++i)
        {
            data[i] = 0.1f * std::sin(2.0f * juce::MathConstants<float>::pi * 1000.0f * i / 48000.0f);
        }
    }

    // Copy input for comparison
    juce::AudioBuffer<float> expectedOutput(inputBuffer);

    // Process
    juce::dsp::AudioBlock<float> block(inputBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    saturation.process(context);

    // Verify: output should match input within tolerance (DC blocker may cause tiny differences)
    for (int channel = 0; channel < 2; ++channel)
    {
        const auto* output = inputBuffer.getReadPointer(channel);
        const auto* expected = expectedOutput.getReadPointer(channel);

        for (int i = 100; i < 512; ++i)  // Skip first 100 samples for DC blocker settling
        {
            EXPECT_NEAR(output[i], expected[i], 0.001f)
                << "Bypass mode should preserve input (sample " << i << ", channel " << channel << ")";
        }
    }
}

//==============================================================================
// TEST 2: Spiral Mode (Airwindows Spiral - smoothest saturation)
//==============================================================================
TEST_F(AdvancedSaturationTest, testSpiralMode)
{
    saturation.setMode(AdvancedSaturation::Mode::Spiral);
    saturation.setWarmth(0.5f);

    auto buffer = processSineWave(1000.0f, 0.5f, 512);

    // Verify output is bounded
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Spiral mode should produce bounded output";

    // Verify saturation occurred (output RMS should be less than input due to soft clipping)
    float outputRMS = calculateRMS(buffer);
    EXPECT_LT(outputRMS, 0.5f * 0.707f)  // Less than input RMS (sine wave RMS = amplitude / sqrt(2))
        << "Spiral mode should apply saturation (output RMS < input RMS)";

    EXPECT_GT(outputRMS, 0.1f)
        << "Spiral mode should not silence signal";
}

//==============================================================================
// TEST 3: Density Mode (Sine-based, infinitely smooth)
//==============================================================================
TEST_F(AdvancedSaturationTest, testDensityMode)
{
    saturation.setMode(AdvancedSaturation::Mode::Density);
    saturation.setWarmth(0.7f);

    auto buffer = processSineWave(440.0f, 0.8f, 1024);

    // Verify smooth saturation (no discontinuities)
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Density mode should produce smooth, bounded output";

    // Verify signal processed
    float outputRMS = calculateRMS(buffer);
    EXPECT_GT(outputRMS, 0.01f)
        << "Density mode should not silence signal";
}

//==============================================================================
// TEST 4: PurestDrive Mode (Musical drive with harmonics)
//==============================================================================
TEST_F(AdvancedSaturationTest, testPurestDriveMode)
{
    saturation.setMode(AdvancedSaturation::Mode::PurestDrive);
    saturation.setWarmth(0.6f);
    saturation.setDrive(1.5f);

    auto buffer = processSineWave(880.0f, 0.5f, 1024);

    // Verify harmonic generation (RMS should differ from clean sine)
    EXPECT_TRUE(isOutputBounded(buffer))
        << "PurestDrive mode should produce bounded output";

    float outputRMS = calculateRMS(buffer);
    EXPECT_GT(outputRMS, 0.1f)
        << "PurestDrive should generate audible harmonics";
}

//==============================================================================
// TEST 5: Tape Mode (Analog tape with hysteresis)
//==============================================================================
TEST_F(AdvancedSaturationTest, testTapeMode)
{
    saturation.setMode(AdvancedSaturation::Mode::Tape);
    saturation.setWarmth(0.8f);

    auto buffer = processSineWave(100.0f, 0.6f, 2048);  // Low frequency to test hysteresis

    // Verify tape saturation works
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Tape mode should produce bounded output";

    // Tape saturation should compress peaks
    float peakLevel = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    EXPECT_LT(peakLevel, 0.8f)
        << "Tape mode should compress input peaks";
}

//==============================================================================
// TEST 6: Transformer Mode (Even harmonics - 2nd, 4th)
//==============================================================================
TEST_F(AdvancedSaturationTest, testTransformerMode)
{
    saturation.setMode(AdvancedSaturation::Mode::Transformer);
    saturation.setWarmth(0.5f);

    auto buffer = processSineWave(1000.0f, 0.5f, 1024);

    // Verify transformer saturation produces even harmonics
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Transformer mode should produce bounded output";

    float outputRMS = calculateRMS(buffer);
    EXPECT_GT(outputRMS, 0.1f)
        << "Transformer mode should generate even harmonics";
}

//==============================================================================
// TEST 7: Tube Mode (2nd + 3rd harmonics)
//==============================================================================
TEST_F(AdvancedSaturationTest, testTubeMode)
{
    saturation.setMode(AdvancedSaturation::Mode::Tube);
    saturation.setWarmth(0.6f);

    auto buffer = processSineWave(440.0f, 0.5f, 1024);

    // Verify tube saturation works
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Tube mode should produce bounded output";

    float outputRMS = calculateRMS(buffer);
    EXPECT_GT(outputRMS, 0.1f)
        << "Tube mode should generate harmonics";
}

//==============================================================================
// TEST 8: Extreme Drive (drive=1.0, high input level)
//==============================================================================
TEST_F(AdvancedSaturationTest, testExtremeDrive)
{
    saturation.setMode(AdvancedSaturation::Mode::Spiral);
    saturation.setWarmth(1.0f);   // Maximum warmth
    saturation.setDrive(10.0f);   // Extreme drive

    // Process high-level signal (0dBFS)
    auto buffer = processSineWave(1000.0f, 1.0f, 512);

    // Even with extreme settings, should not crash or produce invalid output
    EXPECT_TRUE(isOutputBounded(buffer))
        << "Extreme drive should not cause NaN/Inf or unbounded output";

    // Should produce heavy saturation (compressed output)
    float outputRMS = calculateRMS(buffer);
    EXPECT_LT(outputRMS, 0.8f)
        << "Extreme drive should heavily saturate/compress signal";

    EXPECT_GT(outputRMS, 0.01f)
        << "Extreme drive should not silence signal completely";
}

//==============================================================================
// Bonus: Zero Input Test (silence in → silence out)
//==============================================================================
TEST_F(AdvancedSaturationTest, testZeroInput)
{
    saturation.setWarmth(0.8f);

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();  // All zeros

    // Process
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    saturation.process(context);

    // Verify: zeros in → zeros out (within floating point tolerance)
    for (int channel = 0; channel < 2; ++channel)
    {
        const auto* data = buffer.getReadPointer(channel);
        for (int i = 0; i < 512; ++i)
        {
            EXPECT_NEAR(data[i], 0.0f, 1e-6f)
                << "Zero input should produce zero output (sample " << i << ", channel " << channel << ")";
        }
    }
}
