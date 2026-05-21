/*
  BTZ — CPU benchmark harness.
  Drives the real BTZAudioProcessor::processBlock and reports per-setting cost:
    ns/block, ns/sample, realtime factor (× faster than real time),
    and single-core CPU% for one instance.
  Build: -DBTZ_BUILD_BENCH=ON ; run: BTZBench
*/
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <chrono>
#include <cstdio>
#include <vector>
#include <random>

namespace {
struct Scenario { const char* name; int quality; bool heavy; };

void setVal(BTZAudioProcessor& p, const char* id, float v) {
    if (auto* param = p.getAPVTS().getParameter(id))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
}

double runScenario(const Scenario& s, double sr, int blockSize, int iters) {
    BTZAudioProcessor proc;
    proc.prepareToPlay(sr, blockSize);

    // Baseline musical settings (so the wet path actually runs).
    setVal(proc, "mix", 1.0f);
    setVal(proc, "drive", 0.55f);
    setVal(proc, "warmth", 0.4f);
    setVal(proc, "punch", 0.5f);
    setVal(proc, "density", 0.4f);
    setVal(proc, "master", 0.7f);
    setVal(proc, "width", 0.7f);
    setVal(proc, "glue", 0.5f);
    setVal(proc, "shine", 0.4f);
    setVal(proc, "quality", (float)s.quality);

    if (s.heavy) {
        setVal(proc, "resEnabled", 1.0f);
        setVal(proc, "transEnabled", 1.0f);
        setVal(proc, "midSide", 1.0f);
        setVal(proc, "targetLUFSLock", 1.0f);
        setVal(proc, "satModel", 1.0f);   // Tube
    }

    juce::AudioBuffer<float> buf(2, blockSize);
    juce::MidiBuffer midi;
    std::mt19937 rng(1);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
    auto fill = [&] {
        for (int ch = 0; ch < 2; ++ch) {
            auto* d = buf.getWritePointer(ch);
            for (int i = 0; i < blockSize; ++i) d[i] = dist(rng);
        }
    };

    // Warm up (let smoothers settle, OS init, branch predictors).
    for (int i = 0; i < 200; ++i) { fill(); proc.processBlock(buf, midi); }

    double best = 1e30;  // best (min) avg over repeats — least noisy estimate
    for (int rep = 0; rep < 5; ++rep) {
        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < iters; ++i) { fill(); proc.processBlock(buf, midi); }
        const auto t1 = std::chrono::steady_clock::now();
        const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
        best = std::min(best, ns / iters);   // ns per block (incl. fill overhead)
    }
    return best;
}
}

int main() {
    juce::ScopedJuceInitialiser_GUI juceInit;

    const double sr = 48000.0;
    const int block = 512;
    const int iters = 2000;
    const double blockSeconds = (double)block / sr;

    std::printf("BTZ CPU benchmark — %.0f Hz, %d-sample blocks (%.3f ms/block)\n",
                sr, block, blockSeconds * 1000.0);
    std::printf("%-26s %12s %12s %10s %9s\n",
                "scenario", "ns/block", "ns/sample", "RT factor", "CPU% (1 core)");
    std::printf("%s\n", std::string(74, '-').c_str());

    const Scenario scenarios[] = {
        { "Eco (1x), default",       0, false },
        { "Standard (2x), default",  1, false },
        { "High (4x), default",      2, false },
        { "Ultra (8x), default",     3, false },
        { "Standard (2x), ALL ON",   1, true  },
        { "Ultra (8x), ALL ON",      3, true  },
    };

    for (const auto& s : scenarios) {
        const double nsBlock = runScenario(s, sr, block, iters);
        const double nsSample = nsBlock / block;
        const double wallSec = nsBlock * 1e-9;
        const double rtFactor = blockSeconds / wallSec;
        const double cpuPct = wallSec / blockSeconds * 100.0;
        std::printf("%-26s %12.0f %12.2f %10.1fx %8.2f%%\n",
                    s.name, nsBlock, nsSample, rtFactor, cpuPct);
    }
    return 0;
}
