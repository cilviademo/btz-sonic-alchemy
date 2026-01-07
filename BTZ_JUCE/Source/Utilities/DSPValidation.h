/*
  DSPValidation.h

  Professional DSP validation utilities
  Catches bugs BEFORE pluginval, BEFORE users hear them

  Based on best practices from:
  - ChowDSP testing framework
  - KVR DSP forum debugging techniques
  - Real-world plugin QA from FabFilter, Waves, etc.

  Use in DEBUG builds to catch:
  - NaN/Inf propagation
  - DC offset buildup
  - Denormals
  - Clipping/distortion
  - Instability under modulation
*/

#pragma once
#include <JuceHeader.h>
#include <cmath>

namespace BTZValidation
{
    //=========================================================================
    // SAMPLE VALIDATION
    //=========================================================================

    /**
     * Check if a sample is valid (finite and within reasonable range)
     * NaN/Inf samples will crash some hosts or cause silent output
     */
    inline bool isValidSample(float sample)
    {
        return std::isfinite(sample) && std::abs(sample) < 100.0f;
    }

    /**
     * Check entire buffer for invalid samples
     * Call this after each DSP module in DEBUG builds
     */
    inline bool validateBuffer(const juce::AudioBuffer<float>& buffer)
    {
        #if JUCE_DEBUG
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (!isValidSample(data[i]))
                {
                    DBG("INVALID SAMPLE in channel " + juce::String(ch)
                        + " sample " + juce::String(i) + ": " + juce::String(data[i]));
                    jassertfalse;  // Break in debugger
                    return false;
                }
            }
        }
        #endif
        return true;
    }

    /**
     * Sanitize buffer (replace NaN/Inf with silence)
     * Use as safety net in RELEASE builds
     */
    inline void sanitizeBuffer(juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (!std::isfinite(data[i]))
                    data[i] = 0.0f;
            }
        }
    }

    //=========================================================================
    // DC OFFSET DETECTION
    //=========================================================================

    /**
     * Measure DC offset (average value over buffer)
     * DC > 0.01 is audible as "thump" when plugin bypassed
     */
    inline float measureDCOffset(const juce::AudioBuffer<float>& buffer, int channel = 0)
    {
        float sum = 0.0f;
        const float* data = buffer.getReadPointer(channel);
        const int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
            sum += data[i];

        return sum / numSamples;
    }

    /**
     * Check if DC offset is within acceptable range
     */
    inline bool hasDCOffset(const juce::AudioBuffer<float>& buffer, float threshold = 0.01f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            if (std::abs(measureDCOffset(buffer, ch)) > threshold)
                return true;
        }
        return false;
    }

    //=========================================================================
    // DENORMAL DETECTION
    //=========================================================================

    /**
     * Check if buffer contains denormals
     * Denormals cause massive CPU usage on Intel/AMD
     */
    inline bool hasDenormals(const juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float abs_val = std::abs(data[i]);
                if (abs_val > 0.0f && abs_val < 1e-15f)  // Denormal range
                    return true;
            }
        }
        return false;
    }

    //=========================================================================
    // DSP MODULE STABILITY TESTING
    //=========================================================================

    /**
     * Test if a DSP module is stable with impulse input
     * Unstable filters will ring forever or produce NaN
     */
    template<typename DSPModule>
    bool testImpulseResponse(DSPModule& module, int numSamples = 1024)
    {
        juce::AudioBuffer<float> testBuffer(2, numSamples);
        testBuffer.clear();
        testBuffer.setSample(0, 0, 1.0f);  // Unit impulse
        testBuffer.setSample(1, 0, 1.0f);

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        // Check output validity
        if (!validateBuffer(testBuffer))
            return false;

        // Check if output decays (not ringing forever)
        float finalLevel = testBuffer.getMagnitude(0, numSamples - 100, 100);
        if (finalLevel > 0.001f)  // Should decay to near-zero
        {
            DBG("Module not decaying: final level = " + juce::String(finalLevel));
            return false;
        }

        return true;
    }

    /**
     * Test DSP module with DC input
     * Some filters produce DC offset when fed DC
     */
    template<typename DSPModule>
    bool testDCResponse(DSPModule& module, float dcLevel = 1.0f)
    {
        juce::AudioBuffer<float> testBuffer(2, 512);
        testBuffer.clear();

        // Fill with DC
        for (int ch = 0; ch < 2; ++ch)
            juce::FloatVectorOperations::fill(testBuffer.getWritePointer(ch), dcLevel, 512);

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        // Validate output
        return validateBuffer(testBuffer);
    }

    /**
     * Test DSP module with swept sine (frequency response check)
     */
    template<typename DSPModule>
    bool testSweptSine(DSPModule& module, double sampleRate = 44100.0)
    {
        const int numSamples = 4410;  // 100ms
        juce::AudioBuffer<float> testBuffer(2, numSamples);

        // Generate swept sine 20Hz-20kHz
        for (int i = 0; i < numSamples; ++i)
        {
            float t = i / static_cast<float>(numSamples);
            float freq = 20.0f * std::pow(1000.0f, t);  // 20Hz to 20kHz
            float phase = 2.0f * juce::MathConstants<float>::pi * freq * t;
            float sample = std::sin(phase);

            testBuffer.setSample(0, i, sample);
            testBuffer.setSample(1, i, sample);
        }

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        return validateBuffer(testBuffer);
    }

    /**
     * Test DSP module with silence
     * Should output silence (no noise/DC)
     */
    template<typename DSPModule>
    bool testSilence(DSPModule& module)
    {
        juce::AudioBuffer<float> testBuffer(2, 512);
        testBuffer.clear();  // All zeros

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        // Check output is still silent (or very quiet)
        float outputLevel = testBuffer.getMagnitude(0, 0, 512);
        if (outputLevel > 1e-6f)
        {
            DBG("Module producing noise from silence: " + juce::String(outputLevel));
            return false;
        }

        return true;
    }

    /**
     * Test DSP module with extreme input levels
     * Should not explode or produce NaN
     */
    template<typename DSPModule>
    bool testExtremeInputs(DSPModule& module)
    {
        juce::AudioBuffer<float> testBuffer(2, 512);

        // Test 1: Very loud input (+20dB)
        testBuffer.clear();
        juce::FloatVectorOperations::fill(testBuffer.getWritePointer(0), 10.0f, 512);
        juce::FloatVectorOperations::fill(testBuffer.getWritePointer(1), 10.0f, 512);

        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        module.process(context);

        if (!validateBuffer(testBuffer))
            return false;

        // Test 2: Very quiet input (-100dB)
        testBuffer.clear();
        juce::FloatVectorOperations::fill(testBuffer.getWritePointer(0), 0.00001f, 512);
        juce::FloatVectorOperations::fill(testBuffer.getWritePointer(1), 0.00001f, 512);

        module.process(context);

        return validateBuffer(testBuffer);
    }

    //=========================================================================
    // COMPREHENSIVE MODULE TEST SUITE
    //=========================================================================

    /**
     * Run all stability tests on a DSP module
     * Call this in unit tests or during development
     */
    template<typename DSPModule>
    bool runAllTests(DSPModule& module, const juce::String& moduleName = "DSP Module")
    {
        DBG("Testing: " + moduleName);

        bool allPassed = true;

        #define RUN_TEST(test, name) \
            if (!test(module)) { \
                DBG("  FAILED: " + juce::String(name)); \
                allPassed = false; \
            } else { \
                DBG("  PASSED: " + juce::String(name)); \
            }

        RUN_TEST(testImpulseResponse, "Impulse Response");
        RUN_TEST(testDCResponse, "DC Response");
        RUN_TEST(testSweptSine, "Swept Sine");
        RUN_TEST(testSilence, "Silence");
        RUN_TEST(testExtremeInputs, "Extreme Inputs");

        #undef RUN_TEST

        if (allPassed)
            DBG(moduleName + ": ALL TESTS PASSED ✅");
        else
            DBG(moduleName + ": SOME TESTS FAILED ❌");

        return allPassed;
    }

    //=========================================================================
    // PARAMETER MODULATION TESTING
    //=========================================================================

    /**
     * Test if rapid parameter changes cause instability
     * Some filters "blow up" when cutoff is swept rapidly
     */
    template<typename DSPModule>
    bool testParameterModulation(DSPModule& module,
                                 std::function<void(DSPModule&, float)> setParameter,
                                 float minValue, float maxValue)
    {
        juce::AudioBuffer<float> testBuffer(2, 512);

        // Generate test signal
        for (int i = 0; i < 512; ++i)
        {
            float t = i / 512.0f;
            float sample = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * t);
            testBuffer.setSample(0, i, sample);
            testBuffer.setSample(1, i, sample);
        }

        // Sweep parameter while processing
        for (int i = 0; i < 512; ++i)
        {
            float t = i / 512.0f;
            float value = minValue + t * (maxValue - minValue);
            setParameter(module, value);

            juce::dsp::AudioBlock<float> block(testBuffer.getArrayOfWritePointers(), 2, i, 1);
            juce::dsp::ProcessContextReplacing<float> context(block);
            module.process(context);
        }

        return validateBuffer(testBuffer);
    }

} // namespace BTZValidation
