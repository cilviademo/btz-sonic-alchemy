/*
  BTZ — Phase 0 measurement harness. No DSP changes; reads the live processor.
  Emits a Markdown report to stdout (captured by docs/measurements/BTZ_BASELINE.md).
  Build: -DBTZ_BUILD_BENCH=ON ; run: BTZMeasure > docs/measurements/BTZ_BASELINE.md
*/
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <cstdio>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace {
constexpr double SR = 48000.0;
constexpr int BLOCK = 512;
constexpr int FFT_ORDER = 14;          // 16384-point FFT
constexpr int FFT_SIZE  = 1 << FFT_ORDER;

void setVal(BTZAudioProcessor& p, const char* id, float v) {
    if (auto* prm = p.getAPVTS().getParameter(id))
        prm->setValueNotifyingHost(prm->getNormalisableRange().convertTo0to1(v));
}

float rms(const juce::AudioBuffer<float>& b) {
    double s = 0; int n = 0;
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i) { const float x = b.getSample(ch, i); s += x * x; ++n; }
    return (float) std::sqrt(s / std::max(1, n));
}
double rmsDb(float r) { return 20.0 * std::log10(std::max(1.0e-10f, r)); }

// Pump a stereo sine into the processor, fill `out` with the processed audio (chained over blocks).
void processSine(BTZAudioProcessor& p, float freq, float amp, juce::AudioBuffer<float>& out) {
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buf(2, BLOCK);
    const int total = out.getNumSamples();
    int written = 0, blk = 0;
    while (written < total) {
        const int n = std::min(BLOCK, total - written);
        for (int ch = 0; ch < 2; ++ch) {
            auto* d = buf.getWritePointer(ch);
            for (int i = 0; i < BLOCK; ++i)
                d[i] = amp * std::sin(2.0f * BTZDsp::kPi * freq * (float)(blk*BLOCK + i) / (float)SR);
        }
        p.processBlock(buf, midi);
        for (int ch = 0; ch < 2; ++ch) out.copyFrom(ch, written, buf, ch, 0, n);
        written += n; ++blk;
    }
}

// Single-channel real FFT → magnitude bins (size FFT_SIZE/2). Hann-windowed.
void fftMags(const float* x, int n, std::vector<float>& mags) {
    juce::dsp::FFT fft(FFT_ORDER);
    std::vector<float> buf((size_t)(2 * FFT_SIZE), 0.0f);
    const int N = std::min(n, FFT_SIZE);
    for (int i = 0; i < N; ++i) {
        const float w = 0.5f - 0.5f * std::cos(2.0f * BTZDsp::kPi * (float)i / (float)(N - 1));
        buf[(size_t) i] = x[i] * w;
    }
    fft.performFrequencyOnlyForwardTransform(buf.data(), true);
    mags.assign(buf.begin(), buf.begin() + FFT_SIZE / 2);
}

// THD% from FFT magnitudes around a fundamental frequency.
double computeTHD(const std::vector<float>& mags, double fundFreq, int maxHarm = 12) {
    const double binHz = SR / (double) FFT_SIZE;
    auto peakNear = [&](double f) {
        const int center = (int) std::round(f / binHz);
        float p = 0;
        for (int k = std::max(0, center-3); k <= std::min((int) mags.size()-1, center+3); ++k)
            p = std::max(p, mags[(size_t) k]);
        return p;
    };
    const double fund = peakNear(fundFreq);
    double harmEnergy = 0;
    for (int h = 2; h <= maxHarm; ++h) {
        const double hf = h * fundFreq;
        if (hf >= SR * 0.5) break;
        const double pk = peakNear(hf);
        harmEnergy += pk * pk;
    }
    return 100.0 * std::sqrt(harmEnergy) / std::max(1.0e-10, fund);
}

// Inharmonic (alias) floor in dB relative to fundamental — energy in bins that are
// NOT near a harmonic of the fundamental.
double computeAliasFloorDb(const std::vector<float>& mags, double fundFreq) {
    const double binHz = SR / (double) FFT_SIZE;
    auto isHarmonic = [&](double f) {
        for (int h = 1; h <= 40; ++h) {
            if (h * fundFreq >= SR * 0.5) break;
            if (std::abs(f - h * fundFreq) < 5 * binHz) return true;
        }
        return false;
    };
    double sumAlias = 0; int nAlias = 0;
    double peakFund = 0;
    for (int b = 1; b < (int) mags.size(); ++b) {
        const double f = b * binHz;
        if (std::abs(f - fundFreq) < 5 * binHz) peakFund = std::max(peakFund, (double) mags[(size_t) b]);
        if (!isHarmonic(f) && f > 50.0 && f < SR * 0.45) { sumAlias += mags[(size_t) b] * mags[(size_t) b]; ++nAlias; }
    }
    const double aliasRms = std::sqrt(sumAlias / std::max(1, nAlias));
    return 20.0 * std::log10(std::max(1.0e-12, aliasRms) / std::max(1.0e-12, peakFund));
}

// Inline K-weighted integrated loudness over a block of audio.
double computeKWeightedLUFS(const juce::AudioBuffer<float>& buf) {
    BTZDsp::LoudnessMeter lm; lm.prepare(SR);
    for (int i = 0; i < buf.getNumSamples(); ++i)
        lm.process(buf.getSample(0, i), buf.getSample(1, i));
    return lm.integrated;
}

// Sample-peak in dBFS (linear → dB).
double computeSamplePeakDb(const juce::AudioBuffer<float>& buf) {
    float pk = 0.0f;
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int i = 0; i < buf.getNumSamples(); ++i)
            pk = std::max(pk, std::abs(buf.getSample(ch, i)));
    return 20.0 * std::log10(std::max(1.0e-10f, pk));
}

// Compute inter-sample-peak via 4× zero-stuffed FIR (independent reference for honesty check).
double computeISPdBTP(const juce::AudioBuffer<float>& buf) {
    // 4× polyphase using a 32-tap windowed sinc — independent of the limiter's own ISP code.
    const int taps = 32, phases = 4;
    std::vector<float> h((size_t)(taps * phases), 0.0f);
    for (int n = 0; n < taps * phases; ++n) {
        const double t = (double) n / (double) phases - (double) taps * 0.5;
        const double sinc = (std::abs(t) < 1e-9) ? 1.0 : std::sin(BTZDsp::kPi * t) / (BTZDsp::kPi * t);
        const double w = 0.42 - 0.5 * std::cos(2.0 * BTZDsp::kPi * (double) n / (double)(taps * phases - 1))
                              + 0.08 * std::cos(4.0 * BTZDsp::kPi * (double) n / (double)(taps * phases - 1));
        h[(size_t) n] = (float)(sinc * w);
    }
    float pk = 0.0f;
    for (int ch = 0; ch < buf.getNumChannels(); ++ch) {
        const float* x = buf.getReadPointer(ch);
        const int N = buf.getNumSamples();
        for (int i = taps; i < N - taps; ++i)
            for (int ph = 0; ph < phases; ++ph) {
                float y = 0.0f;
                for (int k = 0; k < taps; ++k) y += x[i - k] * h[(size_t)(k * phases + ph)];
                pk = std::max(pk, std::abs(y));
            }
    }
    return 20.0 * std::log10(std::max(1.0e-10f, pk));
}
} // namespace

int main() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    const double blockSec = (double) BLOCK / SR;

    std::printf("# BTZ — Phase 0 Baseline Measurements\n\n");
    std::printf("> Generated by `BTZMeasure` at %.0f Hz, %d-sample blocks. **No DSP changes** —\n", SR, BLOCK);
    std::printf("> this report is observational and records the current state of `main` HEAD.\n\n");

    // ════════════════════════════════════════════════════════════════════════
    // 1. CONTROL-TO-DSP TRUTH AUDIT
    // ════════════════════════════════════════════════════════════════════════
    std::printf("## 1. Control-to-DSP truth audit\n\n");
    std::printf("Method: load processor at defaults, set `mix=1, autoGain=on, master=0.7 (unity)`,\n");
    std::printf("warm 200 blocks, then sweep one parameter to its extremes with a 1 kHz / −12 dBFS\n");
    std::printf("stereo sine input. Measure output RMS at min vs max. `DEAD` = |Δ| < 0.2 dB,\n");
    std::printf("`DESTRUCTIVE` = collapses signal below −40 dBFS or drives above +6 dBFS.\n\n");
    std::printf("| Param ID | RMS @ 0.0 (dBFS) | RMS @ 1.0 (dBFS) | Δ (dB) | Status |\n");
    std::printf("|---|---:|---:|---:|---|\n");

    const char* sweepParams[] = {
        "punch","warmth","boom","glue","air","width","density","motion","era",
        "drive","mix","master","intensity",
        "shine","shineMix",
        "resSens","resDepth","transSens","transMix","toneMatchAmt",
        "macro0","macro1","macro2","macro3"
    };
    for (auto* id : sweepParams) {
        const auto sweep = [&](float v) {
            BTZAudioProcessor p; p.prepareToPlay(SR, BLOCK);
            setVal(p, "mix", 1.0f); setVal(p, "drive", 0.3f);
            setVal(p, "autoGain", 1.0f); setVal(p, "master", 0.7f);
            setVal(p, id, v);
            juce::AudioBuffer<float> out(2, FFT_SIZE);
            // warm-up + measurement: process FFT_SIZE samples (~340 ms)
            juce::AudioBuffer<float> warm(2, FFT_SIZE);
            processSine(p, 1000.0f, 0.25f, warm);                  // warm-up
            processSine(p, 1000.0f, 0.25f, out);                   // measure
            return rmsDb(rms(out));
        };
        const double lo = sweep(0.0f), hi = sweep(1.0f), delta = hi - lo;
        const char* status =
            (std::abs(delta) < 0.2)          ? "DEAD"
          : (hi < -40.0 || lo < -40.0)       ? "DESTRUCTIVE (silences)"
          : (hi >  6.0 || lo >  6.0)         ? "DESTRUCTIVE (drives hot)"
          : "active";
        std::printf("| `%s` | %+.2f | %+.2f | %+.2f | %s |\n", id, lo, hi, delta, status);
        std::fflush(stdout);
    }
    std::printf("\n");

    // ════════════════════════════════════════════════════════════════════════
    // 2. OVERSAMPLING REALITY CHECK
    // ════════════════════════════════════════════════════════════════════════
    std::printf("## 2. Oversampling reality (CPU + reported latency)\n\n");
    std::printf("Method: drive moderate saturation (`drive=0.5`, `glue=0.4`), measure ns/block and\n");
    std::printf("reported latency at each Quality. If `Ultra (8×)` doesn't cost meaningfully more\n");
    std::printf("CPU than `Eco (1×)`, the OS isn't wrapping the nonlinear stages.\n\n");
    std::printf("| Quality | ns/block | CPU%% (1 core) | Latency (samples) | Latency (ms) |\n");
    std::printf("|---|---:|---:|---:|---:|\n");
    const char* qualLabels[] = { "Eco (1×)", "Standard (2×)", "High (4×)", "Ultra (8×)" };
    for (int q = 0; q < 4; ++q) {
        BTZAudioProcessor p; p.prepareToPlay(SR, BLOCK);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.5f); setVal(p, "glue", 0.4f);
        setVal(p, "quality", (float) q);
        juce::AudioBuffer<float> warm(2, FFT_SIZE);
        processSine(p, 220.0f, 0.4f, warm);  // warm up smoothers + apply quality
        // Time it
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> buf(2, BLOCK);
        for (int ch = 0; ch < 2; ++ch) {
            auto* d = buf.getWritePointer(ch);
            for (int i = 0; i < BLOCK; ++i)
                d[i] = 0.4f * std::sin(2.0f * BTZDsp::kPi * 220.0f * (float) i / (float) SR);
        }
        double best = 1e30;
        for (int rep = 0; rep < 5; ++rep) {
            const auto t0 = std::chrono::steady_clock::now();
            for (int i = 0; i < 4000; ++i) {
                juce::AudioBuffer<float> work(2, BLOCK);
                work.makeCopyOf(buf);
                p.processBlock(work, midi);
            }
            const auto t1 = std::chrono::steady_clock::now();
            best = std::min(best, std::chrono::duration<double, std::nano>(t1 - t0).count() / 4000.0);
        }
        const double cpuPct = (best * 1e-9) / blockSec * 100.0;
        const int latSamples = p.getLatencySamples();
        const double latMs = (double) latSamples / SR * 1000.0;
        std::printf("| %s | %.0f | %.2f%% | %d | %.2f |\n",
                    qualLabels[q], best, cpuPct, latSamples, latMs);
        std::fflush(stdout);
    }
    std::printf("\n");

    // ════════════════════════════════════════════════════════════════════════
    // 3. ALIASING — high-drive sine fed at three frequencies; FFT output
    // ════════════════════════════════════════════════════════════════════════
    std::printf("## 3. Aliasing at high drive\n\n");
    std::printf("Method: feed a single-tone sine at −12 dBFS, set `drive=0.8`, `mix=1`,\n");
    std::printf("warm + measure. FFT the output (16384-point, Hann). THD%% = sum of harmonic\n");
    std::printf("magnitudes ÷ fundamental. Alias floor = inharmonic-bin RMS in dB relative to\n");
    std::printf("the fundamental (lower / more-negative = cleaner).\n\n");
    std::printf("| Quality | Test freq | THD%% (Tube) | Alias floor (dB ref fund) |\n");
    std::printf("|---|---:|---:|---:|\n");
    for (int q = 0; q < 4; ++q) {
        for (float testFreq : { 1000.0f, 5000.0f, 12000.0f }) {
            BTZAudioProcessor p; p.prepareToPlay(SR, BLOCK);
            setVal(p, "mix", 1.0f); setVal(p, "drive", 0.8f); setVal(p, "satModel", 1.0f);  // Tube
            setVal(p, "quality", (float) q); setVal(p, "autoGain", 0.0f); setVal(p, "master", 0.7f);
            juce::AudioBuffer<float> warm(2, FFT_SIZE), out(2, FFT_SIZE);
            processSine(p, testFreq, 0.25f, warm);
            processSine(p, testFreq, 0.25f, out);
            std::vector<float> mags;
            fftMags(out.getReadPointer(0), out.getNumSamples(), mags);
            const double thd  = computeTHD(mags, testFreq);
            const double alias = computeAliasFloorDb(mags, testFreq);
            std::printf("| %s | %.0f Hz | %.2f%% | %+.1f dB |\n",
                        qualLabels[q], testFreq, thd, alias);
            std::fflush(stdout);
        }
    }
    std::printf("\n");

    // ════════════════════════════════════════════════════════════════════════
    // 4. LOUDNESS HONESTY — drive/glue sweeps with autoGain on
    // ════════════════════════════════════════════════════════════════════════
    std::printf("## 4. Loudness honesty (drift across macro sweeps with autoGain ON)\n\n");
    std::printf("Method: feed pink-noise–shaped sine sum (multiple sines summed) at −20 dBFS RMS,\n");
    std::printf("warm + measure K-weighted integrated loudness in dB. With autoGain ON, output\n");
    std::printf("loudness should not drift as macros increase — drift > 1 LU is a dishonest A/B.\n\n");
    std::printf("| Macro | Value | Output K-weighted (LUFS) | Drift vs neutral (LU) |\n");
    std::printf("|---|---:|---:|---:|\n");
    auto measureLufsAt = [&](const char* macro, float v) {
        BTZAudioProcessor p; p.prepareToPlay(SR, BLOCK);
        setVal(p, "mix", 1.0f); setVal(p, "autoGain", 1.0f); setVal(p, "master", 0.7f);
        setVal(p, "drive", 0.3f);
        setVal(p, macro, v);
        juce::AudioBuffer<float> warm(2, FFT_SIZE), out(2, FFT_SIZE * 4);  // ~1.3 s for K-weighting to settle
        // Build a multi-sine "pinkish" stimulus
        for (int ch = 0; ch < 2; ++ch) {
            auto* d = warm.getWritePointer(ch);
            for (int i = 0; i < warm.getNumSamples(); ++i) {
                const float t = (float) i / (float) SR;
                d[i] = 0.10f * (std::sin(2.0f * BTZDsp::kPi * 110.0f * t)
                              + std::sin(2.0f * BTZDsp::kPi * 440.0f * t)
                              + std::sin(2.0f * BTZDsp::kPi * 1500.0f * t)
                              + std::sin(2.0f * BTZDsp::kPi * 4400.0f * t));
            }
        }
        // Replicate into out (4× the length)
        for (int blk = 0; blk < 4; ++blk)
            for (int ch = 0; ch < 2; ++ch) out.copyFrom(ch, blk * warm.getNumSamples(), warm, ch, 0, warm.getNumSamples());
        // Process the long buffer
        juce::AudioBuffer<float> piece(2, BLOCK);
        juce::MidiBuffer midi;
        int written = 0;
        while (written < out.getNumSamples()) {
            const int n = std::min(BLOCK, out.getNumSamples() - written);
            for (int ch = 0; ch < 2; ++ch) piece.copyFrom(ch, 0, out, ch, written, n);
            p.processBlock(piece, midi);
            for (int ch = 0; ch < 2; ++ch) out.copyFrom(ch, written, piece, ch, 0, n);
            written += n;
        }
        return computeKWeightedLUFS(out);
    };
    for (const char* macro : { "drive", "glue", "warmth", "punch", "density", "boom" }) {
        const double base = measureLufsAt(macro, 0.3f);  // neutral
        for (float v : { 0.0f, 0.3f, 0.6f, 1.0f }) {
            const double lufs = measureLufsAt(macro, v);
            std::printf("| `%s` | %.1f | %+.1f | %+.2f |\n", macro, v, lufs, lufs - base);
            std::fflush(stdout);
        }
    }
    std::printf("\n");

    // ════════════════════════════════════════════════════════════════════════
    // 5. TRUE-PEAK HONESTY
    // ════════════════════════════════════════════════════════════════════════
    std::printf("## 5. True-peak honesty (footer dBTP vs. measured 4× ISP)\n\n");
    std::printf("Method: feed a known ISP-heavy signal (alternating ±1 samples → a digital square\n");
    std::printf("wave that has very high inter-sample peaks). Process, then compare the meter's\n");
    std::printf("`truePeak` (from the limiter's hold) against an INDEPENDENT 4×-FIR ISP measurement\n");
    std::printf("of the output. Sample-peak is also shown for context. Agreement = honest meter.\n\n");
    {
        BTZAudioProcessor p; p.prepareToPlay(SR, BLOCK);
        setVal(p, "mix", 1.0f); setVal(p, "autoGain", 0.0f); setVal(p, "master", 0.7f);
        setVal(p, "drive", 0.0f);   // no drive — we want to see the limiter pass / clamp ISP
        setVal(p, "ceiling", -0.3f);
        juce::AudioBuffer<float> in(2, BLOCK * 80);
        for (int ch = 0; ch < 2; ++ch) {
            auto* d = in.getWritePointer(ch);
            for (int i = 0; i < in.getNumSamples(); ++i) d[i] = (i & 1) ? 0.99f : -0.99f;
        }
        // Process
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> piece(2, BLOCK);
        int written = 0;
        while (written < in.getNumSamples()) {
            const int n = std::min(BLOCK, in.getNumSamples() - written);
            for (int ch = 0; ch < 2; ++ch) piece.copyFrom(ch, 0, in, ch, written, n);
            p.processBlock(piece, midi);
            for (int ch = 0; ch < 2; ++ch) in.copyFrom(ch, written, piece, ch, 0, n);
            written += n;
        }
        const double samplePk = computeSamplePeakDb(in);
        const double ispMeasured = computeISPdBTP(in);
        const double meterReading = p.meters.truePeak.load(std::memory_order_relaxed);
        std::printf("| Signal | Sample peak (dBFS) | Independent 4× ISP (dBTP) | BTZ meter (dBTP) | Honest? |\n");
        std::printf("|---|---:|---:|---:|---|\n");
        const double diff = std::abs(meterReading - ispMeasured);
        const char* honest = diff < 1.5 ? "✓ within 1.5 dB"
                          : std::abs(meterReading - samplePk) < 1.5 ? "✗ reads sample-peak"
                          : "? mismatch";
        std::printf("| alternating ±0.99 | %+.2f | %+.2f | %+.2f | %s |\n",
                    samplePk, ispMeasured, meterReading, honest);
    }
    std::printf("\n");

    std::printf("---\n\n_End of automated measurements. DAW-listening validation still required._\n");
    return 0;
}
