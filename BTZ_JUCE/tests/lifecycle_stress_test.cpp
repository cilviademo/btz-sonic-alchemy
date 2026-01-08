/*
  BTZ Lifecycle Stress Test
  Tests plugin stability under rapid create/destroy, automation, and state changes

  Ship-Ready Gate: This test must pass 100 iterations with 0 crashes
*/

#include "../Source/PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <iostream>
#include <random>
#include <chrono>
#include <fstream>

class LifecycleStressTest
{
public:
    LifecycleStressTest()
        : rng(std::random_device{}())
        , paramDist(0.0f, 1.0f)
    {
    }

    struct TestConfig
    {
        int numIterations = 100;
        int numInstances = 10;
        double sampleRate = 48000.0;
        int samplesPerBlock = 512;
        bool verbose = true;
    };

    struct TestResults
    {
        int iterations = 0;
        int crashes = 0;
        int warnings = 0;
        std::vector<std::string> errors;
        double totalTimeSeconds = 0.0;
        size_t peakMemoryMB = 0;
    };

    TestResults runTest(const TestConfig& config)
    {
        TestResults results;
        auto startTime = std::chrono::steady_clock::now();

        std::cout << "========================================"  << std::endl;
        std::cout << "BTZ Lifecycle Stress Test" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Iterations: " << config.numIterations << std::endl;
        std::cout << "Instances per iteration: " << config.numInstances << std::endl;
        std::cout << "Sample Rate: " << config.sampleRate << " Hz" << std::endl;
        std::cout << "Buffer Size: " << config.samplesPerBlock << " samples" << std::endl;
        std::cout << std::endl;

        for (int iteration = 0; iteration < config.numIterations; ++iteration)
        {
            results.iterations = iteration + 1;

            if (config.verbose)
            {
                std::cout << "Iteration " << (iteration + 1) << " / "
                          << config.numIterations << "..." << std::flush;
            }

            try
            {
                // Test 1: Create/Destroy Stress
                testCreateDestroy(config);

                // Test 2: Parameter Automation Stress
                testParameterAutomation(config);

                // Test 3: State Save/Load Stress
                testStateSaveLoad(config);

                // Test 4: Processing Under Automation
                testProcessingWithAutomation(config);

                if (config.verbose)
                    std::cout << " ✓" << std::endl;
            }
            catch (const std::exception& e)
            {
                results.crashes++;
                results.errors.push_back(std::string("Iteration ") + std::to_string(iteration + 1) +
                                        ": " + e.what());
                std::cerr << " ❌ CRASH: " << e.what() << std::endl;
            }

            // Log memory usage every 10 iterations
            if ((iteration + 1) % 10 == 0)
            {
                size_t currentMemMB = getMemoryUsageMB();
                if (currentMemMB > results.peakMemoryMB)
                    results.peakMemoryMB = currentMemMB;

                if (config.verbose)
                {
                    std::cout << "  Memory: " << currentMemMB << " MB" << std::endl;
                }
            }
        }

        auto endTime = std::chrono::steady_clock::now();
        results.totalTimeSeconds = std::chrono::duration<double>(endTime - startTime).count();

        printResults(results);
        return results;
    }

private:
    void testCreateDestroy(const TestConfig& config)
    {
        std::vector<std::unique_ptr<BTZAudioProcessor>> processors;

        // Rapid creation
        for (int i = 0; i < config.numInstances; ++i)
        {
            auto processor = std::make_unique<BTZAudioProcessor>();
            processor->prepareToPlay(config.sampleRate, config.samplesPerBlock);
            processors.push_back(std::move(processor));
        }

        // Rapid destruction (in reverse order to test different patterns)
        while (!processors.empty())
        {
            processors.pop_back();
        }
    }

    void testParameterAutomation(const TestConfig& config)
    {
        BTZAudioProcessor processor;
        processor.prepareToPlay(config.sampleRate, config.samplesPerBlock);

        // Rapid parameter changes
        for (int i = 0; i < 100; ++i)
        {
            auto& params = processor.getParameters();
            for (auto* param : params)
            {
                if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
                {
                    float randomValue = paramDist(rng);
                    rangedParam->setValueNotifyingHost(randomValue);
                }
            }
        }

        processor.releaseResources();
    }

    void testStateSaveLoad(const TestConfig& config)
    {
        BTZAudioProcessor processor;
        processor.prepareToPlay(config.sampleRate, config.samplesPerBlock);

        // Set random parameters
        auto& params = processor.getParameters();
        for (auto* param : params)
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                rangedParam->setValueNotifyingHost(paramDist(rng));
            }
        }

        // Save state
        juce::MemoryBlock stateBlock;
        processor.getStateInformation(stateBlock);

        // Create new instance and load state
        BTZAudioProcessor processor2;
        processor2.prepareToPlay(config.sampleRate, config.samplesPerBlock);
        processor2.setStateInformation(stateBlock.getData(), (int)stateBlock.getSize());

        // Repeat several times
        for (int i = 0; i < 10; ++i)
        {
            juce::MemoryBlock tempBlock;
            processor2.getStateInformation(tempBlock);
            processor2.setStateInformation(tempBlock.getData(), (int)tempBlock.getSize());
        }

        processor.releaseResources();
        processor2.releaseResources();
    }

    void testProcessingWithAutomation(const TestConfig& config)
    {
        BTZAudioProcessor processor;
        processor.prepareToPlay(config.sampleRate, config.samplesPerBlock);

        juce::AudioBuffer<float> buffer(2, config.samplesPerBlock);
        juce::MidiBuffer midiBuffer;

        // Fill with test signal
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(ch, i, std::sin(2.0f * juce::MathConstants<float>::pi *
                                                 440.0f * i / (float)config.sampleRate));
            }
        }

        // Process while changing parameters
        for (int block = 0; block < 100; ++block)
        {
            // Change some parameters
            auto& params = processor.getParameters();
            if (params.size() > 0)
            {
                int paramIndex = rng() % params.size();
                if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(params[paramIndex]))
                {
                    param->setValueNotifyingHost(paramDist(rng));
                }
            }

            // Process audio
            processor.processBlock(buffer, midiBuffer);

            // Validate output (no NaN/Inf)
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    float sample = buffer.getSample(ch, i);
                    if (std::isnan(sample) || std::isinf(sample))
                    {
                        throw std::runtime_error("NaN or Inf detected in audio output!");
                    }
                }
            }
        }

        processor.releaseResources();
    }

    size_t getMemoryUsageMB()
    {
        // Simple memory estimation (platform-specific implementations can be added)
        return 0; // TODO: Implement platform-specific memory queries
    }

    void printResults(const TestResults& results)
    {
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "TEST RESULTS" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Iterations: " << results.iterations << std::endl;
        std::cout << "Crashes: " << results.crashes << std::endl;
        std::cout << "Warnings: " << results.warnings << std::endl;
        std::cout << "Time: " << results.totalTimeSeconds << " seconds" << std::endl;
        std::cout << "Peak Memory: " << results.peakMemoryMB << " MB" << std::endl;
        std::cout << std::endl;

        if (results.crashes == 0 && results.errors.empty())
        {
            std::cout << "✓ TEST PASSED: No crashes detected" << std::endl;
        }
        else
        {
            std::cout << "❌ TEST FAILED: " << results.crashes << " crashes detected" << std::endl;
            std::cout << std::endl;
            std::cout << "Errors:" << std::endl;
            for (const auto& error : results.errors)
            {
                std::cout << "  - " << error << std::endl;
            }
        }
        std::cout << "========================================" << std::endl;
    }

    std::mt19937 rng;
    std::uniform_real_distribution<float> paramDist;
};

int main(int argc, char* argv[])
{
    // Initialize JUCE
    juce::ScopedJuceInitialiser_GUI juceInit;

    LifecycleStressTest test;
    LifecycleStressTest::TestConfig config;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--iterations" && i + 1 < argc)
        {
            config.numIterations = std::atoi(argv[++i]);
        }
        else if (arg == "--instances" && i + 1 < argc)
        {
            config.numInstances = std::atoi(argv[++i]);
        }
        else if (arg == "--quiet")
        {
            config.verbose = false;
        }
    }

    auto results = test.runTest(config);

    // Write results to JSON for CI
    std::ofstream jsonFile("lifecycle_stress_results.json");
    jsonFile << "{" << std::endl;
    jsonFile << "  \"iterations\": " << results.iterations << "," << std::endl;
    jsonFile << "  \"crashes\": " << results.crashes << "," << std::endl;
    jsonFile << "  \"warnings\": " << results.warnings << "," << std::endl;
    jsonFile << "  \"totalTimeSeconds\": " << results.totalTimeSeconds << "," << std::endl;
    jsonFile << "  \"peakMemoryMB\": " << results.peakMemoryMB << "," << std::endl;
    jsonFile << "  \"passed\": " << (results.crashes == 0 ? "true" : "false") << std::endl;
    jsonFile << "}" << std::endl;
    jsonFile.close();

    return (results.crashes == 0) ? 0 : 1;
}
