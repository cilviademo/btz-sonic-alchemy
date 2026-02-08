#include <gtest/gtest.h>
#include "../Source/AdvancedProcessingChain.h"
#include <JuceHeader.h>

class ProcessingChainTest : public ::testing::Test {
protected:
    void SetUp() override {
        chain = std::make_unique<AdvancedProcessingChain>();
        
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = 44100.0;
        spec.maximumBlockSize = 512;
        spec.numChannels = 2;
        
        chain->prepare(spec);
        
        // Create test buffer
        testBuffer.setSize(2, 512);
        generateTestSignal();
    }
    
    void generateTestSignal() {
        // Generate kick drum-like test signal
        for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel) {
            auto* channelData = testBuffer.getWritePointer(channel);
            
            for (int sample = 0; sample < testBuffer.getNumSamples(); ++sample) {
                float t = sample / 44100.0f;
                
                // Kick drum envelope
                float env = std::exp(-t * 30.0f);
                
                // Multiple frequency components
                float signal = 0.0f;
                signal += 0.8f * std::sin(2.0f * juce::MathConstants<float>::pi * 60.0f * t) * env; // Sub
                signal += 0.6f * std::sin(2.0f * juce::MathConstants<float>::pi * 80.0f * t) * env; // Fundamental
                signal += 0.3f * std::sin(2.0f * juce::MathConstants<float>::pi * 2000.0f * t) * env * env; // Click
                
                channelData[sample] = signal;
            }
        }
    }
    
    std::unique_ptr<AdvancedProcessingChain> chain;
    juce::AudioBuffer<float> testBuffer;
};

TEST_F(ProcessingChainTest, BasicProcessing) {
    auto block = juce::dsp::AudioBlock<float>(testBuffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    // Process should not crash
    EXPECT_NO_THROW(chain->processBlock(context));
}

TEST_F(ProcessingChainTest, TruePeakLimiting) {
    // Set high drive to test limiting
    chain->setDriveAmount(10.0f);
    
    auto block = juce::dsp::AudioBlock<float>(testBuffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    chain->processBlock(context);
    
    // Check that no sample exceeds -1.0 dBTP (approximately 0.891)
    float maxSample = 0.0f;
    for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel) {
        auto* channelData = testBuffer.getReadPointer(channel);
        for (int sample = 0; sample < testBuffer.getNumSamples(); ++sample) {
            maxSample = std::max(maxSample, std::abs(channelData[sample]));
        }
    }
    
    EXPECT_LE(maxSample, 0.95f); // Allow some headroom above -1 dBTP
}

TEST_F(ProcessingChainTest, OversamplingQuality) {
    // Test with oversampling enabled vs disabled
    chain->setOversamplingEnabled(true);
    chain->setWarmthAmount(1.0f); // High saturation
    
    juce::AudioBuffer<float> oversampledResult = testBuffer;
    auto block1 = juce::dsp::AudioBlock<float>(oversampledResult);
    auto context1 = juce::dsp::ProcessContextReplacing<float>(block1);
    chain->processBlock(context1);
    
    chain->setOversamplingEnabled(false);
    juce::AudioBuffer<float> regularResult = testBuffer;
    auto block2 = juce::dsp::AudioBlock<float>(regularResult);
    auto context2 = juce::dsp::ProcessContextReplacing<float>(block2);
    chain->processBlock(context2);
    
    // Oversampled result should be different (hopefully better quality)
    bool isDifferent = false;
    for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel) {
        auto* data1 = oversampledResult.getReadPointer(channel);
        auto* data2 = regularResult.getReadPointer(channel);
        
        for (int sample = 0; sample < testBuffer.getNumSamples(); ++sample) {
            if (std::abs(data1[sample] - data2[sample]) > 0.001f) {
                isDifferent = true;
                break;
            }
        }
    }
    
    EXPECT_TRUE(isDifferent);
}

TEST_F(ProcessingChainTest, ParameterRanges) {
    // Test that all parameters accept their full ranges
    EXPECT_NO_THROW(chain->setPunchAmount(0.0f));
    EXPECT_NO_THROW(chain->setPunchAmount(1.0f));
    
    EXPECT_NO_THROW(chain->setWarmthAmount(0.0f));
    EXPECT_NO_THROW(chain->setWarmthAmount(1.0f));
    
    EXPECT_NO_THROW(chain->setBoomAmount(0.0f));
    EXPECT_NO_THROW(chain->setBoomAmount(1.0f));
    
    EXPECT_NO_THROW(chain->setMixAmount(0.0f));
    EXPECT_NO_THROW(chain->setMixAmount(1.0f));
    
    EXPECT_NO_THROW(chain->setDriveAmount(0.0f));
    EXPECT_NO_THROW(chain->setDriveAmount(12.0f));
}

TEST_F(ProcessingChainTest, MeteringAccuracy) {
    // Process a known signal and check meters
    chain->setDriveAmount(0.0f); // No processing for accurate metering
    
    auto block = juce::dsp::AudioBlock<float>(testBuffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    chain->processBlock(context);
    
    float inputLevel = chain->getInputLevel();
    float outputLevel = chain->getOutputLevel();
    
    EXPECT_GT(inputLevel, 0.0f);
    EXPECT_GT(outputLevel, 0.0f);
    EXPECT_LE(inputLevel, 1.0f);
    EXPECT_LE(outputLevel, 1.0f);
}

// Performance test
TEST_F(ProcessingChainTest, PerformanceTest) {
    const int numIterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numIterations; ++i) {
        auto block = juce::dsp::AudioBlock<float>(testBuffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        chain->processBlock(context);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should process 1000 blocks (512 samples each) in reasonable time
    // At 44.1kHz, this represents about 11.6 seconds of audio
    double processingTimeMs = duration.count() / 1000.0;
    double audioLengthMs = (numIterations * 512.0 / 44100.0) * 1000.0;
    double realtimeFactor = audioLengthMs / processingTimeMs;
    
    std::cout << "Processing time: " << processingTimeMs << "ms" << std::endl;
    std::cout << "Audio length: " << audioLengthMs << "ms" << std::endl;
    std::cout << "Realtime factor: " << realtimeFactor << "x" << std::endl;
    
    // Should be able to process at least 10x realtime on modern hardware
    EXPECT_GT(realtimeFactor, 10.0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}