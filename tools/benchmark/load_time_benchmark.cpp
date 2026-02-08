/*
  ==============================================================================

  BTZ Load Time Benchmark

  Purpose: Measure plugin initialization time for Ship Gate performance targets

  Requirements:
  - Load time < 500ms (worst-case with all features)
  - Typical load time < 200ms
  - No allocations after prepareToPlay

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>

using namespace juce;

// Include plugin processor implementation
#include "../../BTZ_JUCE/Source/PluginProcessor.h"

//==============================================================================
struct LoadTimeBenchmarkResults {
    std::vector<double> constructorTimes;
    std::vector<double> prepareToPlayTimes;
    std::vector<double> totalLoadTimes;

    double avgConstructorMs = 0.0;
    double avgPrepareMs = 0.0;
    double avgTotalMs = 0.0;

    double maxConstructorMs = 0.0;
    double maxPrepareMs = 0.0;
    double maxTotalMs = 0.0;

    int numIterations = 0;
    double sampleRate = 0.0;
    int bufferSize = 0;
};

//==============================================================================
class LoadTimeBenchmark {
public:
    LoadTimeBenchmark(int iterations, double sampleRate, int bufferSize)
        : iterations_(iterations), sampleRate_(sampleRate), bufferSize_(bufferSize) {}

    LoadTimeBenchmarkResults run() {
        std::cout << "========================================\n";
        std::cout << "BTZ Load Time Benchmark\n";
        std::cout << "========================================\n";
        std::cout << "Iterations: " << iterations_ << "\n";
        std::cout << "Sample Rate: " << sampleRate_ << " Hz\n";
        std::cout << "Buffer Size: " << bufferSize_ << " samples\n";
        std::cout << "========================================\n\n";

        LoadTimeBenchmarkResults results;
        results.numIterations = iterations_;
        results.sampleRate = sampleRate_;
        results.bufferSize = bufferSize_;

        std::cout << "Running load time measurements...\n";

        for (int i = 0; i < iterations_; ++i) {
            // Measure constructor time
            auto constructorStart = std::chrono::high_resolution_clock::now();
            auto processor = std::make_unique<BTZAudioProcessor>();
            auto constructorEnd = std::chrono::high_resolution_clock::now();

            double constructorMs = std::chrono::duration<double, std::milli>(
                constructorEnd - constructorStart).count();

            // Measure prepareToPlay time
            auto prepareStart = std::chrono::high_resolution_clock::now();
            processor->setRateAndBufferSizeDetails(sampleRate_, bufferSize_);
            processor->prepareToPlay(sampleRate_, bufferSize_);
            auto prepareEnd = std::chrono::high_resolution_clock::now();

            double prepareMs = std::chrono::duration<double, std::milli>(
                prepareEnd - prepareStart).count();

            double totalMs = constructorMs + prepareMs;

            results.constructorTimes.push_back(constructorMs);
            results.prepareToPlayTimes.push_back(prepareMs);
            results.totalLoadTimes.push_back(totalMs);

            // Clean up
            processor->releaseResources();
            processor.reset();

            if ((i + 1) % 10 == 0) {
                std::cout << "  Completed " << (i + 1) << " / " << iterations_ << " iterations\n";
            }
        }

        std::cout << "✓ Load time measurements complete\n\n";

        // Calculate statistics
        calculateStats(results);

        return results;
    }

private:
    void calculateStats(LoadTimeBenchmarkResults& results) {
        double sumConstructor = 0.0, sumPrepare = 0.0, sumTotal = 0.0;

        for (size_t i = 0; i < results.constructorTimes.size(); ++i) {
            sumConstructor += results.constructorTimes[i];
            sumPrepare += results.prepareToPlayTimes[i];
            sumTotal += results.totalLoadTimes[i];

            results.maxConstructorMs = std::max(results.maxConstructorMs, results.constructorTimes[i]);
            results.maxPrepareMs = std::max(results.maxPrepareMs, results.prepareToPlayTimes[i]);
            results.maxTotalMs = std::max(results.maxTotalMs, results.totalLoadTimes[i]);
        }

        int n = results.numIterations;
        results.avgConstructorMs = sumConstructor / n;
        results.avgPrepareMs = sumPrepare / n;
        results.avgTotalMs = sumTotal / n;
    }

    int iterations_;
    double sampleRate_;
    int bufferSize_;
};

//==============================================================================
void printResults(const LoadTimeBenchmarkResults& results) {
    std::cout << "========================================\n";
    std::cout << "Load Time Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Constructor Time:\n";
    std::cout << "  Average: " << results.avgConstructorMs << " ms\n";
    std::cout << "  Maximum: " << results.maxConstructorMs << " ms\n";
    std::cout << "\n";
    std::cout << "prepareToPlay Time:\n";
    std::cout << "  Average: " << results.avgPrepareMs << " ms\n";
    std::cout << "  Maximum: " << results.maxPrepareMs << " ms\n";
    std::cout << "\n";
    std::cout << "Total Load Time:\n";
    std::cout << "  Average: " << results.avgTotalMs << " ms\n";
    std::cout << "  Maximum: " << results.maxTotalMs << " ms\n";
    std::cout << "========================================\n\n";

    // Ship Gate evaluation
    bool typicalPass = results.avgTotalMs < 200.0;
    bool worstCasePass = results.maxTotalMs < 500.0;

    std::cout << "Performance Targets:\n";
    std::cout << "  Typical load (< 200ms): "
              << (typicalPass ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "  Worst-case load (< 500ms): "
              << (worstCasePass ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "========================================\n";
}

void writeJsonResults(const LoadTimeBenchmarkResults& results, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open output file: " << path << "\n";
        return;
    }

    bool typicalPass = results.avgTotalMs < 200.0;
    bool worstCasePass = results.maxTotalMs < 500.0;

    file << "{\n";
    file << "  \"benchmark_version\": \"1.0.0\",\n";
    file << "  \"benchmark_type\": \"load_time\",\n";
    file << "  \"timestamp\": \"" << Time::getCurrentTime().toString(true, true, true, true).toStdString() << "\",\n";
    file << "  \"configuration\": {\n";
    file << "    \"iterations\": " << results.numIterations << ",\n";
    file << "    \"sample_rate\": " << results.sampleRate << ",\n";
    file << "    \"buffer_size\": " << results.bufferSize << "\n";
    file << "  },\n";
    file << "  \"results\": {\n";
    file << "    \"constructor\": {\n";
    file << "      \"avg_ms\": " << results.avgConstructorMs << ",\n";
    file << "      \"max_ms\": " << results.maxConstructorMs << "\n";
    file << "    },\n";
    file << "    \"prepare_to_play\": {\n";
    file << "      \"avg_ms\": " << results.avgPrepareMs << ",\n";
    file << "      \"max_ms\": " << results.maxPrepareMs << "\n";
    file << "    },\n";
    file << "    \"total_load\": {\n";
    file << "      \"avg_ms\": " << results.avgTotalMs << ",\n";
    file << "      \"max_ms\": " << results.maxTotalMs << "\n";
    file << "    }\n";
    file << "  },\n";
    file << "  \"performance_targets\": {\n";
    file << "    \"typical_load_200ms\": {\n";
    file << "      \"target\": \"< 200ms\",\n";
    file << "      \"actual\": " << results.avgTotalMs << ",\n";
    file << "      \"status\": \"" << (typicalPass ? "PASS" : "FAIL") << "\"\n";
    file << "    },\n";
    file << "    \"worst_case_load_500ms\": {\n";
    file << "      \"target\": \"< 500ms\",\n";
    file << "      \"actual\": " << results.maxTotalMs << ",\n";
    file << "      \"status\": \"" << (worstCasePass ? "PASS" : "FAIL") << "\"\n";
    file << "    }\n";
    file << "  }\n";
    file << "}\n";

    file.close();
    std::cout << "✓ Results written to: " << path << "\n";
}

//==============================================================================
int main(int argc, char* argv[]) {
    ScopedJuceInitialiser_GUI juceInitialiser;

    int iterations = 50;
    double sampleRate = 48000.0;
    int bufferSize = 512;
    std::string outputPath = "load_time_results.json";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        String arg(argv[i]);

        if (arg == "--iterations" && i + 1 < argc) {
            iterations = String(argv[++i]).getIntValue();
        } else if (arg == "--sample-rate" && i + 1 < argc) {
            sampleRate = String(argv[++i]).getDoubleValue();
        } else if (arg == "--buffer-size" && i + 1 < argc) {
            bufferSize = String(argv[++i]).getIntValue();
        } else if (arg == "--output" && i + 1 < argc) {
            outputPath = String(argv[++i]).toStdString();
        } else if (arg == "--help") {
            std::cout << "BTZ Load Time Benchmark\n\n";
            std::cout << "Usage: load_time_benchmark [options]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --iterations N      Number of load cycles (default: 50)\n";
            std::cout << "  --sample-rate SR    Sample rate in Hz (default: 48000)\n";
            std::cout << "  --buffer-size N     Buffer size in samples (default: 512)\n";
            std::cout << "  --output PATH       Output JSON path\n";
            return 0;
        }
    }

    // Run benchmark
    LoadTimeBenchmark benchmark(iterations, sampleRate, bufferSize);
    LoadTimeBenchmarkResults results = benchmark.run();

    // Output results
    printResults(results);
    writeJsonResults(results, outputPath);

    bool passed = results.avgTotalMs < 200.0 && results.maxTotalMs < 500.0;
    return passed ? 0 : 1;
}
