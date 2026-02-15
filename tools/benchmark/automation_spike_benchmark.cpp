/*
  ==============================================================================

  BTZ Automation Spike Benchmark

  Purpose: Measure CPU spikes during rapid parameter automation

  Requirements:
  - No audio glitches during automation
  - CPU spikes < 2x average during parameter changes
  - Smooth parameter transitions (no zipper noise)

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>

using namespace juce;

// Include plugin processor implementation
#include "../../BTZ_JUCE/Source/PluginProcessor.h"

//==============================================================================
struct AutomationSpikeResults {
    std::vector<double> baselineCpuPercent;
    std::vector<double> automationCpuPercent;

    double avgBaselineCpu = 0.0;
    double avgAutomationCpu = 0.0;
    double peakAutomationCpu = 0.0;
    double spikeRatio = 0.0;  // peak_automation / avg_baseline

    int numBuffers = 0;
    double sampleRate = 0.0;
    int bufferSize = 0;
};

//==============================================================================
class AutomationSpikeBenchmark {
public:
    AutomationSpikeBenchmark(double sampleRate, int bufferSize)
        : sampleRate_(sampleRate), bufferSize_(bufferSize) {}

    AutomationSpikeResults run() {
        std::cout << "========================================\n";
        std::cout << "BTZ Automation Spike Benchmark\n";
        std::cout << "========================================\n";
        std::cout << "Sample Rate: " << sampleRate_ << " Hz\n";
        std::cout << "Buffer Size: " << bufferSize_ << " samples\n";
        std::cout << "========================================\n\n";

        AutomationSpikeResults results;
        results.sampleRate = sampleRate_;
        results.bufferSize = bufferSize_;

        // Create processor
        auto processor = std::make_unique<BTZAudioProcessor>();
        processor->setRateAndBufferSizeDetails(sampleRate_, bufferSize_);
        processor->prepareToPlay(sampleRate_, bufferSize_);

        // Prepare buffers
        AudioBuffer<float> buffer(2, bufferSize_);
        MidiBuffer midiBuffer;

        // Fill with pink noise
        Random random;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                buffer.setSample(ch, i, random.nextFloat() * 2.0f - 1.0f);
            }
        }

        // Phase 1: Baseline measurement (no automation)
        std::cout << "Phase 1: Measuring baseline CPU (1000 buffers, no automation)...\n";
        for (int i = 0; i < 1000; ++i) {
            auto start = std::clock();
            processor->processBlock(buffer, midiBuffer);
            auto end = std::clock();

            double cpuTimeMs = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            double bufferDurationMs = (bufferSize_ / sampleRate_) * 1000.0;
            double cpuPercent = (cpuTimeMs / bufferDurationMs) * 100.0;

            results.baselineCpuPercent.push_back(cpuPercent);
        }
        std::cout << "✓ Baseline complete\n\n";

        // Phase 2: Automation measurement (rapid parameter changes)
        std::cout << "Phase 2: Measuring automation CPU (1000 buffers, rapid changes)...\n";

        auto& apvts = processor->getAPVTS();
        std::vector<String> paramIds = {"punch", "warmth", "boom", "drive", "mix",
                                        "inputGain", "outputGain", "shineFreqHz"};

        for (int i = 0; i < 1000; ++i) {
            // Change multiple parameters every buffer
            for (const auto& paramId : paramIds) {
                if (auto* param = apvts.getParameter(paramId)) {
                    float newValue = random.nextFloat();
                    param->setValueNotifyingHost(newValue);
                }
            }

            auto start = std::clock();
            processor->processBlock(buffer, midiBuffer);
            auto end = std::clock();

            double cpuTimeMs = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            double bufferDurationMs = (bufferSize_ / sampleRate_) * 1000.0;
            double cpuPercent = (cpuTimeMs / bufferDurationMs) * 100.0;

            results.automationCpuPercent.push_back(cpuPercent);
        }
        std::cout << "✓ Automation complete\n\n";

        // Calculate statistics
        calculateStats(results);

        processor->releaseResources();
        return results;
    }

private:
    void calculateStats(AutomationSpikeResults& results) {
        // Baseline stats
        double sumBaseline = 0.0;
        for (double cpu : results.baselineCpuPercent) {
            sumBaseline += cpu;
        }
        results.avgBaselineCpu = sumBaseline / results.baselineCpuPercent.size();

        // Automation stats
        double sumAutomation = 0.0;
        results.peakAutomationCpu = 0.0;
        for (double cpu : results.automationCpuPercent) {
            sumAutomation += cpu;
            results.peakAutomationCpu = std::max(results.peakAutomationCpu, cpu);
        }
        results.avgAutomationCpu = sumAutomation / results.automationCpuPercent.size();

        // Spike ratio
        if (results.avgBaselineCpu > 0.0) {
            results.spikeRatio = results.peakAutomationCpu / results.avgBaselineCpu;
        }

        results.numBuffers = static_cast<int>(results.automationCpuPercent.size());
    }

    double sampleRate_;
    int bufferSize_;
};

//==============================================================================
void printResults(const AutomationSpikeResults& results) {
    std::cout << "========================================\n";
    std::cout << "Automation Spike Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Baseline (No Automation):\n";
    std::cout << "  Average CPU: " << results.avgBaselineCpu << "%\n";
    std::cout << "\n";
    std::cout << "Automation (Rapid Changes):\n";
    std::cout << "  Average CPU: " << results.avgAutomationCpu << "%\n";
    std::cout << "  Peak CPU: " << results.peakAutomationCpu << "%\n";
    std::cout << "  Spike Ratio: " << results.spikeRatio << "x\n";
    std::cout << "========================================\n\n";

    // Performance evaluation
    bool spikePass = results.spikeRatio < 2.0;
    bool avgPass = results.avgAutomationCpu < results.avgBaselineCpu * 1.5;

    std::cout << "Performance Targets:\n";
    std::cout << "  CPU spike < 2x baseline: "
              << (spikePass ? "✅ PASS" : "❌ FAIL")
              << " (" << results.spikeRatio << "x)\n";
    std::cout << "  Avg automation < 1.5x baseline: "
              << (avgPass ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "========================================\n";
}

void writeJsonResults(const AutomationSpikeResults& results, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open output file: " << path << "\n";
        return;
    }

    bool spikePass = results.spikeRatio < 2.0;
    bool avgPass = results.avgAutomationCpu < results.avgBaselineCpu * 1.5;

    file << "{\n";
    file << "  \"benchmark_version\": \"1.0.0\",\n";
    file << "  \"benchmark_type\": \"automation_spike\",\n";
    file << "  \"timestamp\": \"" << Time::getCurrentTime().toString(true, true, true, true).toStdString() << "\",\n";
    file << "  \"configuration\": {\n";
    file << "    \"sample_rate\": " << results.sampleRate << ",\n";
    file << "    \"buffer_size\": " << results.bufferSize << ",\n";
    file << "    \"buffers_per_phase\": " << results.numBuffers << "\n";
    file << "  },\n";
    file << "  \"results\": {\n";
    file << "    \"baseline\": {\n";
    file << "      \"avg_cpu_percent\": " << results.avgBaselineCpu << "\n";
    file << "    },\n";
    file << "    \"automation\": {\n";
    file << "      \"avg_cpu_percent\": " << results.avgAutomationCpu << ",\n";
    file << "      \"peak_cpu_percent\": " << results.peakAutomationCpu << ",\n";
    file << "      \"spike_ratio\": " << results.spikeRatio << "\n";
    file << "    }\n";
    file << "  },\n";
    file << "  \"performance_targets\": {\n";
    file << "    \"spike_ratio_2x\": {\n";
    file << "      \"target\": \"< 2.0x\",\n";
    file << "      \"actual\": " << results.spikeRatio << ",\n";
    file << "      \"status\": \"" << (spikePass ? "PASS" : "FAIL") << "\"\n";
    file << "    },\n";
    file << "    \"avg_automation_1_5x\": {\n";
    file << "      \"target\": \"< 1.5x baseline\",\n";
    file << "      \"actual\": " << (results.avgAutomationCpu / results.avgBaselineCpu) << ",\n";
    file << "      \"status\": \"" << (avgPass ? "PASS" : "FAIL") << "\"\n";
    file << "    }\n";
    file << "  }\n";
    file << "}\n";

    file.close();
    std::cout << "✓ Results written to: " << path << "\n";
}

//==============================================================================
int main(int argc, char* argv[]) {
    ScopedJuceInitialiser_GUI juceInitialiser;

    double sampleRate = 48000.0;
    int bufferSize = 128;
    std::string outputPath = "automation_spike_results.json";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        String arg(argv[i]);

        if (arg == "--sample-rate" && i + 1 < argc) {
            sampleRate = String(argv[++i]).getDoubleValue();
        } else if (arg == "--buffer-size" && i + 1 < argc) {
            bufferSize = String(argv[++i]).getIntValue();
        } else if (arg == "--output" && i + 1 < argc) {
            outputPath = String(argv[++i]).toStdString();
        } else if (arg == "--help") {
            std::cout << "BTZ Automation Spike Benchmark\n\n";
            std::cout << "Usage: automation_spike_benchmark [options]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --sample-rate SR    Sample rate in Hz (default: 48000)\n";
            std::cout << "  --buffer-size N     Buffer size in samples (default: 128)\n";
            std::cout << "  --output PATH       Output JSON path\n";
            return 0;
        }
    }

    // Run benchmark
    AutomationSpikeBenchmark benchmark(sampleRate, bufferSize);
    AutomationSpikeResults results = benchmark.run();

    // Output results
    printResults(results);
    writeJsonResults(results, outputPath);

    bool passed = results.spikeRatio < 2.0;
    return passed ? 0 : 1;
}
