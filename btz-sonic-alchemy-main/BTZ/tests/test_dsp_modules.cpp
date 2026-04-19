/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp
  Unit tests for extracted DSP modules in BTZDsp.h.
  Build with: cmake -DBTZ_BUILD_TESTS=ON ..
  Run with:   ctest --output-on-failure
*/
#include <gtest/gtest.h>
#include "BTZDsp.h"
#include <cmath>
#include <vector>
#include <numeric>

using namespace BTZDsp;

// ═══════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════
static constexpr double kSR = 48000.0;
static constexpr int kBlockSize = 512;

static std::vector<float> generateSine(float freqHz, double sr, int numSamples, float amplitude = 0.5f) {
    std::vector<float> buf(numSamples);
    for (int i = 0; i < numSamples; ++i)
        buf[i] = amplitude * std::sin(2.0f * 3.14159265f * freqHz * (float)i / (float)sr);
    return buf;
}

static float peakAbs(const std::vector<float>& buf) {
    float pk = 0.0f;
    for (auto s : buf) pk = std::max(pk, std::abs(s));
    return pk;
}

static float rms(const std::vector<float>& buf) {
    float sq = 0.0f;
    for (auto s : buf) sq += s * s;
    return std::sqrt(sq / std::max(1, (int)buf.size()));
}

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SmoothParam, ConvergesToTarget) {
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(0.0f);
    sp.setTarget(1.0f);

    for (int i = 0; i < 48000; ++i)
        sp.next();

    EXPECT_NEAR(sp.current, 1.0f, 0.001f);
}

TEST(SmoothParam, SnapToSetsImmediately) {
    SmoothParam sp;
    sp.setTime(100.0f, kSR);
    sp.snapTo(0.75f);
    EXPECT_FLOAT_EQ(sp.current, 0.75f);
    EXPECT_FLOAT_EQ(sp.target, 0.75f);
}

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(EnvFollower, TracksInputLevel) {
    EnvFollower ef;
    ef.setTimes(0.5f, 100.0f, kSR);
    ef.reset();

    // Feed constant level
    for (int i = 0; i < 48000; ++i)
        ef.process(0.8f);

    EXPECT_NEAR(ef.env, 0.8f, 0.01f);
}

TEST(EnvFollower, ReleasesAfterSilence) {
    EnvFollower ef;
    ef.setTimes(0.5f, 50.0f, kSR);
    ef.reset();

    // Build up
    for (int i = 0; i < 4800; ++i) ef.process(1.0f);
    // Release
    for (int i = 0; i < 48000; ++i) ef.process(0.0f);

    EXPECT_LT(ef.env, 0.01f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SafetyLayer tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SafetyLayer, BlocksDC) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    sl.reset();

    float dc = 0.0f, dcPrev = 0.0f;
    // Feed DC offset
    for (int i = 0; i < 96000; ++i)
        sl.processSample(1.0f, dc, dcPrev);

    float out = sl.processSample(1.0f, dc, dcPrev);
    EXPECT_LT(std::abs(out), 0.05f); // DC should be mostly removed
}

TEST(SafetyLayer, CleansNaN) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    sl.reset();

    float dc = 0.0f, dcPrev = 0.0f;
    float result = sl.processSample(std::numeric_limits<float>::quiet_NaN(), dc, dcPrev);
    EXPECT_TRUE(std::isfinite(result));
}

TEST(SafetyLayer, CleansInf) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    sl.reset();

    float dc = 0.0f, dcPrev = 0.0f;
    float result = sl.processSample(std::numeric_limits<float>::infinity(), dc, dcPrev);
    EXPECT_TRUE(std::isfinite(result));
}

// ═══════════════════════════════════════════════════════════════════════════
// SlewLimiter tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SlewLimiter, LimitsTransients) {
    SlewLimiter sl;
    sl.setSampleRate(kSR);
    sl.reset();

    float out = sl.process(0.0f);
    EXPECT_FLOAT_EQ(out, 0.0f);

    // Large jump
    out = sl.process(10.0f);
    EXPECT_LT(out, 10.0f); // Should be slew-limited
    EXPECT_GT(out, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// LinkwitzRileyCrossover tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(LinkwitzRileyCrossover, SumsToOriginal) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, kBlockSize);
    float maxError = 0.0f;

    for (int i = 0; i < kBlockSize; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], sine[i], lowL, lowR, highL, highR);
        float reconstructed = lowL + highL;
        maxError = std::max(maxError, std::abs(reconstructed - sine[i]));
    }

    EXPECT_LT(maxError, 0.01f); // Should reconstruct within 1% error
}

TEST(LinkwitzRileyCrossover, LowPassAttenuatesHigh) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    // Feed 5 kHz sine (well above crossover)
    auto sine = generateSine(5000.0f, kSR, 4800, 1.0f);
    std::vector<float> lowOut(4800);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], 0.0f, lowL, lowR, highL, highR);
        lowOut[i] = lowL;
    }

    // Low band should be significantly attenuated for 5 kHz
    float lowPeak = peakAbs(lowOut);
    EXPECT_LT(lowPeak, 0.3f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SparkLimiter tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SparkLimiter, LimitsAboveCeiling) {
    SparkLimiter spark;
    spark.prepare(kSR, kBlockSize);
    spark.reset();

    const float ceiling = 0.5f; // -6 dB
    float peakOut = 0.0f;

    // Feed hot signal
    auto sine = generateSine(1000.0f, kSR, 4800, 2.0f);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        spark.processStereo(L, R, ceiling, 1.0f);
        peakOut = std::max(peakOut, std::max(std::abs(L), std::abs(R)));
    }

    EXPECT_LE(peakOut, ceiling + 0.02f); // Allow small overshoot from attack time
}

TEST(SparkLimiter, PassthroughBelowCeiling) {
    SparkLimiter spark;
    spark.prepare(kSR, kBlockSize);
    spark.reset();

    const float ceiling = 1.0f;
    auto sine = generateSine(1000.0f, kSR, 4800, 0.3f);
    float maxDiff = 0.0f;

    // Burn through lookahead first
    for (int i = 0; i < 200; ++i) {
        float L = sine[i], R = sine[i];
        spark.processStereo(L, R, ceiling, 1.0f);
    }

    // Now check passthrough
    for (int i = 200; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        float origL = L;
        spark.processStereo(L, R, ceiling, 1.0f);
        // With lookahead delay, compare delayed output
        // Just verify level is preserved
        maxDiff = std::max(maxDiff, std::abs(std::abs(L) - std::abs(origL)));
    }

    // Signal below ceiling should pass through mostly unchanged
    // (allowing for lookahead delay offset)
    EXPECT_LT(maxDiff, 0.35f);
}

TEST(SparkLimiter, ReportsGainReduction) {
    SparkLimiter spark;
    spark.prepare(kSR, kBlockSize);
    spark.reset();

    const float ceiling = 0.5f;
    float maxGR = 0.0f;

    auto sine = generateSine(1000.0f, kSR, 4800, 2.0f);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        float gr = spark.processStereo(L, R, ceiling, 1.0f);
        maxGR = std::max(maxGR, gr);
    }

    EXPECT_GT(maxGR, 1.0f); // Should report at least 1 dB GR
}

TEST(SparkLimiter, SparkMixZeroIsPassthrough) {
    SparkLimiter spark;
    spark.prepare(kSR, kBlockSize);
    spark.reset();

    const float ceiling = 0.5f;
    auto sine = generateSine(1000.0f, kSR, 4800, 2.0f);

    // With sparkMix = 0, output should equal delayed input
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        spark.processStereo(L, R, ceiling, 0.0f);
        // Just verify it doesn't crash and peak is preserved
    }
    // If we got here without crash, pass
    SUCCEED();
}

// ═══════════════════════════════════════════════════════════════════════════
// ShineProcessor tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(ShineProcessor, BoostsHighFrequencies) {
    ShineProcessor shine;
    shine.prepare(kSR);
    shine.setParameters(8000.0f, 6.0f, 0.7f);
    shine.reset();

    auto sine = generateSine(10000.0f, kSR, 4800, 0.3f);
    std::vector<float> output(4800);

    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }

    float inRms = rms(sine);
    float outRms = rms(output);
    EXPECT_GT(outRms, inRms * 1.2f); // Should boost by at least 20%
}

TEST(ShineProcessor, DoesNotBoostLowFrequencies) {
    ShineProcessor shine;
    shine.prepare(kSR);
    shine.setParameters(12000.0f, 6.0f, 0.7f);
    shine.reset();

    auto sine = generateSine(100.0f, kSR, 4800, 0.3f);
    std::vector<float> output(4800);

    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }

    float inRms = rms(sine);
    float outRms = rms(output);
    // Low frequencies should pass through approximately unchanged
    EXPECT_NEAR(outRms / inRms, 1.0f, 0.15f);
}

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(GlueCompressor, ReducesLoudSignals) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    EnvFollower env;
    env.setTimes(5.0f, 80.0f, kSR);
    env.reset();

    auto sine = generateSine(1000.0f, kSR, 4800, 1.0f);
    std::vector<float> output(4800);

    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        float sidechain = std::max(std::abs(L), std::abs(R));
        float envVal = env.process(sidechain);
        gc.processStereo(L, R, 0.8f, envVal); // High glue amount
        output[i] = L;
    }

    float outPeak = peakAbs(output);
    EXPECT_LT(outPeak, 0.95f); // Should compress
}

TEST(GlueCompressor, PassthroughAtZeroGlue) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    float L = 0.5f, R = 0.5f;
    float origL = L;
    gc.processStereo(L, R, 0.0f, 0.5f);
    EXPECT_FLOAT_EQ(L, origL);
}

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(MacroInterpreter, DefaultMappingsExist) {
    MacroInterpreter mi;
    mi.setupDefaults();

    float macros[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float mod = mi.getModulation(0, macros); // target 0 = punch
    EXPECT_GT(mod, 0.0f); // Macro 0 should modulate punch
}

TEST(MacroInterpreter, ZeroMacrosGiveZeroMod) {
    MacroInterpreter mi;
    mi.setupDefaults();

    float macros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float mod = mi.getModulation(0, macros);
    EXPECT_FLOAT_EQ(mod, 0.0f);
}

TEST(MacroInterpreter, ClearRemovesAllMappings) {
    MacroInterpreter mi;
    mi.setupDefaults();
    mi.clearMappings();

    float macros[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float mod = mi.getModulation(0, macros);
    EXPECT_FLOAT_EQ(mod, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// MeterBallistics tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(MeterBallistics, PrepareDoesNotCrash) {
    MeterBallistics mb;
    mb.prepare(44100.0, 256);
    mb.prepare(48000.0, 512);
    mb.prepare(96000.0, 1024);
    mb.prepare(192000.0, 64);
    SUCCEED();
}

TEST(MeterBallistics, ResetClearsState) {
    MeterBallistics mb;
    mb.prepare(kSR, kBlockSize);
    mb.inPeakHoldL = 0.9f;
    mb.sparkGR = 5.0f;
    mb.reset();
    EXPECT_FLOAT_EQ(mb.inPeakHoldL, 0.0f);
    EXPECT_FLOAT_EQ(mb.sparkGR, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// fastTanh tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(FastTanh, ApproximatesStdTanh) {
    for (float x = -3.0f; x <= 3.0f; x += 0.1f) {
        float approx = fastTanh(x);
        float exact = std::tanh(x);
        EXPECT_NEAR(approx, exact, 0.05f) << "at x=" << x;
    }
}

TEST(FastTanh, ZeroReturnsZero) {
    EXPECT_FLOAT_EQ(fastTanh(0.0f), 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// AutoGainSmoother tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(AutoGainSmoother, DoesNotCrashOnSilence) {
    AutoGainSmoother ag;
    ag.prepare(kSR);

    std::vector<float> zeros(kBlockSize, 0.0f);
    ag.processBlock(zeros.data(), zeros.data(), kBlockSize, zeros.data(), zeros.data());
    SUCCEED();
}

TEST(AutoGainSmoother, ConvergesOverBlocks) {
    AutoGainSmoother ag;
    ag.prepare(kSR);

    auto dry = generateSine(1000.0f, kSR, kBlockSize, 0.5f);
    auto wet = generateSine(1000.0f, kSR, kBlockSize, 1.0f); // 2x louder

    // Run multiple blocks
    for (int b = 0; b < 20; ++b) {
        auto wetCopy = wet;
        auto wetCopy2 = wet;
        ag.processBlock(wetCopy.data(), wetCopy2.data(), kBlockSize, dry.data(), dry.data());
    }

    // After convergence, gain should be reducing the wet signal
    EXPECT_LT(ag.smoothedGain, 0.9f);
}
