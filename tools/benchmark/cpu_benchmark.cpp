/*
  ==============================================================================

  BTZ CPU Benchmark Harness

  Purpose: Deterministic CPU performance measurement for Ship Gate #4

  Requirements:
  - N plugin instances (default 10)
  - Fixed sample rate (default 48kHz)
  - Fixed buffer size (default 128)
  - Offline processing loop (no realtime constraints)
  - Wall-clock + CPU timing
  - JSON output with avg/peak CPU metrics

  Pass Criteria: 10 instances @ 48kHz/128 samples < 60% CPU

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cmath>

using namespace juce;

// Include plugin processor implementation
#include "../../BTZ_JUCE/Source/PluginProcessor.h"

//==============================================================================
struct BenchmarkConfig {
    int numInstances = 10;
    double sampleRate = 48000.0;
    int bufferSize = 128;
    int numBuffersToProcess = 10000;  // ~27 seconds at 48kHz/128
    int warmupBuffers = 100;
    std::string outputPath = "benchmark_results.json";
};

struct BenchmarkResults {
    double totalWallTimeMs = 0.0;
    double totalCpuTimeMs = 0.0;
    double avgCpuPercent = 0.0;
    double peakCpuPercent = 0.0;
    std::vector<double> perBufferCpuPercent;
    int numInstances = 0;
    double sampleRate = 0.0;
    int bufferSize = 0;
    int buffersProcessed = 0;

    // Calculate CPU as: (processing_time / available_realtime) * 100
    // available_realtime = bufferSize / sampleRate
    double calculateCpuPercent(double processingTimeMs, double sampleRate, int bufferSize) const {
        double bufferDurationMs = (bufferSize / sampleRate) * 1000.0;
        return (processingTimeMs / bufferDurationMs) * 100.0;
    }
};

//==============================================================================
class CPUBenchmark {
public:
    CPUBenchmark(const BenchmarkConfig& config)
        : config_(config) {}

    BenchmarkResults run() {
        std::cout << "========================================\n";
        std::cout << "BTZ CPU Benchmark Harness\n";
        std::cout << "========================================\n";
        std::cout << "Configuration:\n";
        std::cout << "  Instances: " << config_.numInstances << "\n";
        std::cout << "  Sample Rate: " << config_.sampleRate << " Hz\n";
        std::cout << "  Buffer Size: " << config_.bufferSize << " samples\n";
        std::cout << "  Buffers to Process: " << config_.numBuffersToProcess << "\n";
        std::cout << "  Warmup Buffers: " << config_.warmupBuffers << "\n";
        std::cout << "========================================\n\n";

        // Initialize instances
        std::cout << "Initializing " << config_.numInstances << " plugin instances...\n";
        std::vector<std::unique_ptr<BTZAudioProcessor>> instances;
        for (int i = 0; i < config_.numInstances; ++i) {
            auto processor = std::make_unique<BTZAudioProcessor>();
            processor->setRateAndBufferSizeDetails(config_.sampleRate, config_.bufferSize);
            processor->prepareToPlay(config_.sampleRate, config_.bufferSize);
            instances.push_back(std::move(processor));
        }
        std::cout << "✓ All instances initialized\n\n";

        // Prepare audio buffers
        AudioBuffer<float> inputBuffer(2, config_.bufferSize);
        AudioBuffer<float> outputBuffer(2, config_.bufferSize);
        MidiBuffer midiBuffer;

        // Fill input with pink noise
        Random random;
        for (int ch = 0; ch < inputBuffer.getNumChannels(); ++ch) {
            for (int i = 0; i < inputBuffer.getNumSamples(); ++i) {
                inputBuffer.setSample(ch, i, random.nextFloat() * 2.0f - 1.0f);
            }
        }

        // Warmup
        std::cout << "Running warmup (" << config_.warmupBuffers << " buffers)...\n";
        for (int i = 0; i < config_.warmupBuffers; ++i) {
            for (auto& instance : instances) {
                outputBuffer.makeCopyOf(inputBuffer);
                instance->processBlock(outputBuffer, midiBuffer);
            }
        }
        std::cout << "✓ Warmup complete\n\n";

        // Benchmark
        std::cout << "Running benchmark (" << config_.numBuffersToProcess << " buffers)...\n";
        BenchmarkResults results;
        results.numInstances = config_.numInstances;
        results.sampleRate = config_.sampleRate;
        results.bufferSize = config_.bufferSize;
        results.perBufferCpuPercent.reserve(config_.numBuffersToProcess);

        auto wallStart = std::chrono::high_resolution_clock::now();
        auto cpuStart = std::clock();

        for (int bufferIdx = 0; bufferIdx < config_.numBuffersToProcess; ++bufferIdx) {
            auto bufferCpuStart = std::clock();

            for (auto& instance : instances) {
                outputBuffer.makeCopyOf(inputBuffer);
                instance->processBlock(outputBuffer, midiBuffer);
            }

            auto bufferCpuEnd = std::clock();
            double bufferCpuTimeMs = ((double)(bufferCpuEnd - bufferCpuStart) / CLOCKS_PER_SEC) * 1000.0;
            double cpuPercent = results.calculateCpuPercent(bufferCpuTimeMs, config_.sampleRate, config_.bufferSize);
            results.perBufferCpuPercent.push_back(cpuPercent);

            // Progress indicator
            if ((bufferIdx + 1) % 1000 == 0) {
                std::cout << "  Processed " << (bufferIdx + 1) << " / " << config_.numBuffersToProcess
                          << " buffers (" << std::fixed << std::setprecision(1)
                          << ((bufferIdx + 1) * 100.0 / config_.numBuffersToProcess) << "%)\n";
            }
        }

        auto cpuEnd = std::clock();
        auto wallEnd = std::chrono::high_resolution_clock::now();

        results.buffersProcessed = config_.numBuffersToProcess;
        results.totalCpuTimeMs = ((double)(cpuEnd - cpuStart) / CLOCKS_PER_SEC) * 1000.0;
        results.totalWallTimeMs = std::chrono::duration<double, std::milli>(wallEnd - wallStart).count();

        // Calculate average and peak CPU
        double sum = 0.0;
        results.peakCpuPercent = 0.0;
        for (double cpu : results.perBufferCpuPercent) {
            sum += cpu;
            results.peakCpuPercent = std::max(results.peakCpuPercent, cpu);
        }
        results.avgCpuPercent = sum / results.perBufferCpuPercent.size();

        std::cout << "✓ Benchmark complete\n\n";

        return results;
    }

private:
    BenchmarkConfig config_;
};

//==============================================================================
void printResults(const BenchmarkResults& results) {
    std::cout << "========================================\n";
    std::cout << "Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total Wall Time: " << results.totalWallTimeMs << " ms\n";
    std::cout << "Total CPU Time: " << results.totalCpuTimeMs << " ms\n";
    std::cout << "Average CPU: " << results.avgCpuPercent << "%\n";
    std::cout << "Peak CPU: " << results.peakCpuPercent << "%\n";
    std::cout << "Buffers Processed: " << results.buffersProcessed << "\n";
    std::cout << "========================================\n\n";

    // Ship Gate pass/fail
    bool passed = results.avgCpuPercent < 60.0;
    std::cout << "Ship Gate #4 Status: ";
    if (passed) {
        std::cout << "✅ PASS (< 60% CPU)\n";
    } else {
        std::cout << "❌ FAIL (>= 60% CPU)\n";
    }
    std::cout << "========================================\n";
}

void writeJsonResults(const BenchmarkResults& results, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open output file: " << path << "\n";
        return;
    }

    file << "{\n";
    file << "  \"benchmark_version\": \"1.0.0\",\n";
    file << "  \"timestamp\": \"" << Time::getCurrentTime().toString(true, true, true, true).toStdString() << "\",\n";
    file << "  \"configuration\": {\n";
    file << "    \"num_instances\": " << results.numInstances << ",\n";
    file << "    \"sample_rate\": " << results.sampleRate << ",\n";
    file << "    \"buffer_size\": " << results.bufferSize << ",\n";
    file << "    \"buffers_processed\": " << results.buffersProcessed << "\n";
    file << "  },\n";
    file << "  \"results\": {\n";
    file << "    \"total_wall_time_ms\": " << results.totalWallTimeMs << ",\n";
    file << "    \"total_cpu_time_ms\": " << results.totalCpuTimeMs << ",\n";
    file << "    \"avg_cpu_percent\": " << results.avgCpuPercent << ",\n";
    file << "    \"peak_cpu_percent\": " << results.peakCpuPercent << "\n";
    file << "  },\n";
    file << "  \"ship_gate_4\": {\n";
    file << "    \"requirement\": \"< 60% CPU\",\n";
    file << "    \"status\": \"" << (results.avgCpuPercent < 60.0 ? "PASS" : "FAIL") << "\"\n";
    file << "  }\n";
    file << "}\n";

    file.close();
    std::cout << "✓ Results written to: " << path << "\n";
}

//==============================================================================
void printUsage() {
    std::cout << "BTZ CPU Benchmark Harness\n\n";
    std::cout << "Usage: cpu_benchmark [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --instances N       Number of plugin instances (default: 10)\n";
    std::cout << "  --sample-rate SR    Sample rate in Hz (default: 48000)\n";
    std::cout << "  --buffer-size N     Buffer size in samples (default: 128)\n";
    std::cout << "  --buffers N         Number of buffers to process (default: 10000)\n";
    std::cout << "  --warmup N          Number of warmup buffers (default: 100)\n";
    std::cout << "  --output PATH       Output JSON path (default: benchmark_results.json)\n";
    std::cout << "  --help              Show this help\n";
}

//==============================================================================
int main(int argc, char* argv[]) {
    ScopedJuceInitialiser_GUI juceInitialiser;

    BenchmarkConfig config;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        String arg(argv[i]);

        if (arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "--instances" && i + 1 < argc) {
            config.numInstances = String(argv[++i]).getIntValue();
        } else if (arg == "--sample-rate" && i + 1 < argc) {
            config.sampleRate = String(argv[++i]).getDoubleValue();
        } else if (arg == "--buffer-size" && i + 1 < argc) {
            config.bufferSize = String(argv[++i]).getIntValue();
        } else if (arg == "--buffers" && i + 1 < argc) {
            config.numBuffersToProcess = String(argv[++i]).getIntValue();
        } else if (arg == "--warmup" && i + 1 < argc) {
            config.warmupBuffers = String(argv[++i]).getIntValue();
        } else if (arg == "--output" && i + 1 < argc) {
            config.outputPath = String(argv[++i]).toStdString();
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage();
            return 1;
        }
    }

    // Validate config
    if (config.numInstances < 1 || config.numInstances > 100) {
        std::cerr << "ERROR: Invalid number of instances (must be 1-100)\n";
        return 1;
    }

    // Run benchmark
    CPUBenchmark benchmark(config);
    BenchmarkResults results = benchmark.run();

    // Output results
    printResults(results);
    writeJsonResults(results, config.outputPath);

    return (results.avgCpuPercent < 60.0) ? 0 : 1;
}
