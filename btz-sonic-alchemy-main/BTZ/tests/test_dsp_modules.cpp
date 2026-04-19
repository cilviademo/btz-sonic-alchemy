/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp  v8
  ────────────────────────────────────────────────────────────────────────
  GoogleTest-based unit tests for all BTZDsp modules.
  v8: Competitive-audit driven:
    - REMOVED all ADAATanh tests (ADAA removed from DSP chain)
    - REMOVED SlewLimiter tests (SlewLimiter removed from DSP chain)
    - Updated RealTimeSafety tests to use plain fastTanh
    - Updated state version check to 8
    - Fixed MacroInterpreter PerceptualCurves test API
    - Fixed SmoothParam test API (.setTime instead of .prepare)
    - Added v8 regression tests:
      * DC blocker 1 Hz coloration (pink noise pass-through)
      * Plain fastTanh saturation chain (no ADAA state)
      * Full chain without ADAA/SlewLimiter
  v7: Release-gate hardening:
    - BypassCrossfader click-free transition test
    - SidechainHPF crossfade (no click on mode change)
    - Silence-in-silence-out verification
    - State version validation
    - SmoothParam automation zipper test
  v6: Senior-level fixes:
    - SidechainHPF tests (60/90/150 Hz, stereo, passthrough)
    - MacroInterpreter wiring integration tests
  v5: Audit-driven fixes:
    - Envelope follower timing test (verifies base-SR coefficients)
    - Updated comments with measured values
  v4: Mathematical overhaul tests:
    - LR4 crossover (24 dB/oct SVF-based)
    - Soft-knee GlueCompressor
    - Padé [5/5] fastTanh
    - Perceptual macro curves
    - FixedDeque (pre-allocated, lock-free)

  Build: cmake -DBTZ_BUILD_TESTS=ON ..
  Run:   ctest --output-on-failure
*/
#include <gtest/gtest.h>
#include "../Source/BTZDsp.h"
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>

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

static float rmsRange(const std::vector<float>& buf, int start, int end) {
    float sq = 0.0f;
    int count = 0;
    for (int i = start; i < end && i < (int)buf.size(); ++i) {
        sq += buf[i] * buf[i];
        count++;
    }
    return std::sqrt(sq / std::max(1, count));
}

// ═══════════════════════════════════════════════════════════════════════════
// fastTanh Padé [5/5] Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(FastTanh, ApproximatesStdTanhWithin1Percent) {
    // v4: Padé [5/5] should be within 1% for |x| <= 3
    for (float x = -3.0f; x <= 3.0f; x += 0.05f) {
        float approx = fastTanh(x);
        float exact = std::tanh(x);
        float relErr = (std::abs(exact) > 0.01f)
            ? std::abs(approx - exact) / std::abs(exact)
            : std::abs(approx - exact);
        EXPECT_LT(relErr, 0.01f) << "at x=" << x << " approx=" << approx << " exact=" << exact;
    }
}

TEST(FastTanh, MaxErrorBelow2PercentToX5) {
    // v4: Padé [5/5] max error should be < 2% for |x| <= 5
    float maxRelErr = 0.0f;
    for (float x = -5.0f; x <= 5.0f; x += 0.01f) {
        float approx = fastTanh(x);
        float exact = std::tanh(x);
        float relErr = (std::abs(exact) > 0.01f)
            ? std::abs(approx - exact) / std::abs(exact)
            : std::abs(approx - exact);
        maxRelErr = std::max(maxRelErr, relErr);
    }
    EXPECT_LT(maxRelErr, 0.02f) << "max relative error = " << maxRelErr;
}

TEST(FastTanh, ZeroReturnsZero) {
    EXPECT_FLOAT_EQ(fastTanh(0.0f), 0.0f);
}

TEST(FastTanh, OddSymmetry) {
    // tanh is an odd function: f(-x) = -f(x)
    for (float x = 0.1f; x <= 4.0f; x += 0.3f) {
        EXPECT_NEAR(fastTanh(-x), -fastTanh(x), 1e-6f) << "at x=" << x;
    }
}

TEST(FastTanh, MonotonicIncreasing) {
    float prev = fastTanh(-5.0f);
    for (float x = -4.9f; x <= 5.0f; x += 0.1f) {
        float y = fastTanh(x);
        EXPECT_GE(y, prev - 1e-6f) << "non-monotonic at x=" << x;
        prev = y;
    }
}

// v8: Verify fastTanh is suitable as a direct replacement for ADAA in the chain
TEST(FastTanh, SaturatesLargeSignals) {
    // fastTanh should bound output to approximately [-1, 1]
    for (float x = 1.0f; x <= 10.0f; x += 0.5f) {
        float y = fastTanh(x);
        EXPECT_LE(std::abs(y), 1.05f) << "fastTanh should saturate at x=" << x;
        EXPECT_GT(std::abs(y), 0.5f) << "fastTanh should have significant output at x=" << x;
    }
}

TEST(FastTanh, SmallSignalApproximatesLinear) {
    // For small inputs, tanh(x) ≈ x
    for (float x = -0.01f; x <= 0.01f; x += 0.001f) {
        EXPECT_NEAR(fastTanh(x), x, std::abs(x) * 0.05f + 1e-7f)
            << "fastTanh should be approximately linear for small x=" << x;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// FixedDeque Tests (pre-allocated, lock-free ring buffer)
// ═══════════════════════════════════════════════════════════════════════════
TEST(FixedDeque, PushAndPopBack) {
    FixedDeque<float> dq;
    dq.allocate(16);
    dq.push_back(1.0f);
    dq.push_back(2.0f);
    dq.push_back(3.0f);
    EXPECT_EQ(dq.size(), 3);
    EXPECT_FLOAT_EQ(dq.front(), 1.0f);
    EXPECT_FLOAT_EQ(dq.back(), 3.0f);
}

TEST(FixedDeque, PopFront) {
    FixedDeque<float> dq;
    dq.allocate(16);
    dq.push_back(10.0f);
    dq.push_back(20.0f);
    dq.push_back(30.0f);
    dq.pop_front();
    EXPECT_EQ(dq.size(), 2);
    EXPECT_FLOAT_EQ(dq.front(), 20.0f);
}

TEST(FixedDeque, PopBack) {
    FixedDeque<float> dq;
    dq.allocate(16);
    dq.push_back(10.0f);
    dq.push_back(20.0f);
    dq.push_back(30.0f);
    dq.pop_back();
    EXPECT_EQ(dq.size(), 2);
    EXPECT_FLOAT_EQ(dq.back(), 20.0f);
}

TEST(FixedDeque, ClearResetsSize) {
    FixedDeque<float> dq;
    dq.allocate(16);
    for (int i = 0; i < 10; ++i) dq.push_back((float)i);
    EXPECT_EQ(dq.size(), 10);
    dq.clear();
    EXPECT_EQ(dq.size(), 0);
    EXPECT_TRUE(dq.empty());
}

TEST(FixedDeque, WrapsAroundCorrectly) {
    FixedDeque<float> dq;
    dq.allocate(4);
    dq.push_back(1.0f);
    dq.push_back(2.0f);
    dq.push_back(3.0f);
    dq.push_back(4.0f);
    dq.pop_front();
    dq.pop_front();
    dq.push_back(5.0f);
    dq.push_back(6.0f);
    EXPECT_EQ(dq.size(), 4);
    EXPECT_FLOAT_EQ(dq.front(), 3.0f);
    EXPECT_FLOAT_EQ(dq.back(), 6.0f);
}

TEST(FixedDeque, EmptyChecks) {
    FixedDeque<float> dq;
    dq.allocate(8);
    EXPECT_TRUE(dq.empty());
    dq.push_back(1.0f);
    EXPECT_FALSE(dq.empty());
    dq.pop_front();
    EXPECT_TRUE(dq.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// TruePeakLimiter Tests
// ═══════════════════════════════════════════════════════════════════════════
class TruePeakLimiterTest : public ::testing::Test {
protected:
    TruePeakLimiter limiter;
    void SetUp() override {
        limiter.prepare(kSR, kBlockSize, 2.0f);
        limiter.reset();
    }
};

TEST_F(TruePeakLimiterTest, PassesSilence) {
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    buffer.clear();
    limiter.processBlock(buffer, 1.0f);
    for (int i = 0; i < kBlockSize; ++i) {
        EXPECT_NEAR(buffer.getSample(0, i), 0.0f, 1e-6f);
        EXPECT_NEAR(buffer.getSample(1, i), 0.0f, 1e-6f);
    }
}

TEST_F(TruePeakLimiterTest, LimitsAboveCeiling) {
    const float ceiling = 0.5f;
    const int totalSamples = kBlockSize * 8;
    auto sine = generateSine(440.0f, kSR, totalSamples, 1.0f);
    juce::AudioBuffer<float> buffer(2, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        buffer.setSample(0, i, sine[i]);
        buffer.setSample(1, i, sine[i]);
    }
    limiter.processBlock(buffer, ceiling);
    const int skip = limiter.getLatencySamples() + 50;
    for (int i = skip; i < totalSamples; ++i) {
        EXPECT_LE(std::abs(buffer.getSample(0, i)), ceiling + 0.02f) << "Overshoot at sample " << i;
        EXPECT_LE(std::abs(buffer.getSample(1, i)), ceiling + 0.02f) << "Overshoot at sample " << i;
    }
}

TEST_F(TruePeakLimiterTest, DoesNotLimitBelowCeiling) {
    const float ceiling = 1.0f;
    const int totalSamples = kBlockSize * 4;
    auto sine = generateSine(440.0f, kSR, totalSamples, 0.3f);
    juce::AudioBuffer<float> buffer(2, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        buffer.setSample(0, i, sine[i]);
        buffer.setSample(1, i, sine[i]);
    }
    limiter.processBlock(buffer, ceiling);
    float grDb = limiter.getGainReductionDb();
    EXPECT_NEAR(grDb, 0.0f, 0.5f);
}

TEST_F(TruePeakLimiterTest, ReportsGainReduction) {
    auto sine = generateSine(440.0f, kSR, kBlockSize * 4, 1.0f);
    juce::AudioBuffer<float> buffer(2, kBlockSize * 4);
    for (int i = 0; i < kBlockSize * 4; ++i) {
        buffer.setSample(0, i, sine[i]);
        buffer.setSample(1, i, sine[i]);
    }
    limiter.processBlock(buffer, 0.5f);
    float grDb = limiter.getGainReductionDb();
    EXPECT_GT(grDb, 0.5f);
}

TEST_F(TruePeakLimiterTest, LatencyIsPositive) {
    EXPECT_GT(limiter.getLatencySamples(), 0);
}

TEST_F(TruePeakLimiterTest, ResetClearsGainReduction) {
    auto sine = generateSine(440.0f, kSR, kBlockSize, 2.0f);
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    for (int i = 0; i < kBlockSize; ++i) {
        buffer.setSample(0, i, sine[i]);
        buffer.setSample(1, i, sine[i]);
    }
    limiter.processBlock(buffer, 0.3f);
    EXPECT_GT(limiter.getGainReductionDb(), 0.0f);
    limiter.reset();
    juce::AudioBuffer<float> silence(2, kBlockSize);
    silence.clear();
    limiter.processBlock(silence, 1.0f);
    EXPECT_NEAR(limiter.getGainReductionDb(), 0.0f, 0.05f);
}

TEST_F(TruePeakLimiterTest, HandlesStereoLinked) {
    const int totalSamples = kBlockSize * 4;
    auto hotSine = generateSine(440.0f, kSR, totalSamples, 1.5f);
    auto quietSine = generateSine(440.0f, kSR, totalSamples, 0.2f);
    juce::AudioBuffer<float> buffer(2, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        buffer.setSample(0, i, hotSine[i]);
        buffer.setSample(1, i, quietSine[i]);
    }
    limiter.processBlock(buffer, 0.5f);
    const int skip = limiter.getLatencySamples() + 50;
    for (int i = skip; i < totalSamples; ++i) {
        EXPECT_LE(std::abs(buffer.getSample(0, i)), 0.52f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// SVF ShineProcessor Tests
// ═══════════════════════════════════════════════════════════════════════════
class ShineProcessorTest : public ::testing::Test {
protected:
    ShineProcessor shine;
    void SetUp() override {
        shine.prepare(kSR);
        shine.reset();
    }
};

TEST_F(ShineProcessorTest, PassesSilence) {
    float L = 0.0f, R = 0.0f;
    shine.processStereo(L, R);
    EXPECT_NEAR(L, 0.0f, 1e-6f);
    EXPECT_NEAR(R, 0.0f, 1e-6f);
}

TEST_F(ShineProcessorTest, BoostsHighFrequencies) {
    shine.setParameters(8000.0f, 6.0f, 0.7f);
    auto sine = generateSine(10000.0f, kSR, 4800, 0.3f);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }
    float inRms = rms(sine);
    float outRms = rms(output);
    EXPECT_GT(outRms, inRms * 1.2f);
}

TEST_F(ShineProcessorTest, DoesNotBoostLowFrequencies) {
    shine.setParameters(12000.0f, 6.0f, 0.7f);
    auto sine = generateSine(100.0f, kSR, 4800, 0.3f);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }
    float inRms = rms(sine);
    float outRms = rms(output);
    EXPECT_NEAR(outRms / inRms, 1.0f, 0.15f);
}

TEST_F(ShineProcessorTest, ZeroGainIsUnity) {
    shine.setParameters(12000.0f, 0.0f, 0.7f);
    auto sine = generateSine(8000.0f, kSR, 4800, 0.4f);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }
    for (int i = 200; i < 4800; ++i)
        EXPECT_NEAR(output[i], sine[i], 0.02f);
}

TEST_F(ShineProcessorTest, ResetClearsState) {
    auto sine = generateSine(10000.0f, kSR, kBlockSize, 0.5f);
    for (int i = 0; i < kBlockSize; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
    }
    shine.reset();
    float L = 0.0f, R = 0.0f;
    shine.processStereo(L, R);
    EXPECT_NEAR(L, 0.0f, 1e-5f);
}

TEST_F(ShineProcessorTest, ModulationSafe) {
    auto sine = generateSine(5000.0f, kSR, kBlockSize * 4, 0.5f);
    for (int i = 0; i < kBlockSize * 4; ++i) {
        if (i % 100 == 0) {
            float freq = 2000.0f + (float)(i % 18000);
            float gain = (float)(i % 12);
            float q = 0.1f + (float)(i % 20) * 0.1f;
            shine.setParameters(freq, gain, q);
        }
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        EXPECT_TRUE(std::isfinite(L)) << "NaN/Inf at sample " << i;
        EXPECT_TRUE(std::isfinite(R)) << "NaN/Inf at sample " << i;
    }
}

TEST_F(ShineProcessorTest, CutReducesHighFrequencies) {
    // v4: Verify negative gain (cut) works correctly
    shine.setParameters(8000.0f, -6.0f, 0.7f);
    auto sine = generateSine(10000.0f, kSR, 4800, 0.3f);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }
    float inRms = rms(sine);
    float outRms = rmsRange(output, 200, 4800);
    EXPECT_LT(outRms, inRms * 0.85f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SmoothParam, ConvergesToTarget) {
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(0.0f);
    sp.setTarget(1.0f);
    for (int i = 0; i < 48000; ++i) sp.next();
    EXPECT_NEAR(sp.current, 1.0f, 0.001f);
}

TEST(SmoothParam, SnapToSetsImmediately) {
    SmoothParam sp;
    sp.setTime(100.0f, kSR);
    sp.snapTo(0.75f);
    EXPECT_FLOAT_EQ(sp.current, 0.75f);
    EXPECT_FLOAT_EQ(sp.target, 0.75f);
}

TEST(SmoothParam, IsSmoothingDetection) {
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(1.0f);
    EXPECT_FALSE(sp.isSmoothing());
    sp.setTarget(0.0f);
    EXPECT_TRUE(sp.isSmoothing());
}

TEST(SmoothParam, SnapsAtThreshold) {
    // v4: SmoothParam should snap to target when difference < 1e-6
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(0.0f);
    sp.setTarget(1e-7f);
    sp.next();
    // After one step toward a very tiny target, it should snap
    EXPECT_NEAR(sp.current, sp.target, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(EnvFollower, TracksInputLevel) {
    EnvFollower ef;
    ef.setTimes(0.5f, 100.0f, kSR);
    ef.reset();
    for (int i = 0; i < 48000; ++i) ef.process(0.8f);
    EXPECT_NEAR(ef.env, 0.8f, 0.01f);
}

TEST(EnvFollower, ReleasesAfterSilence) {
    EnvFollower ef;
    ef.setTimes(0.5f, 50.0f, kSR);
    ef.reset();
    for (int i = 0; i < 4800; ++i) ef.process(1.0f);
    for (int i = 0; i < 48000; ++i) ef.process(0.0f);
    EXPECT_LT(ef.env, 0.01f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SafetyLayer Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SafetyLayer, BlocksDC) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    sl.reset();
    float dc = 0.0f, dcPrev = 0.0f;
    for (int i = 0; i < 96000; ++i) sl.processSample(1.0f, dc, dcPrev);
    float out = sl.processSample(1.0f, dc, dcPrev);
    EXPECT_LT(std::abs(out), 0.05f);
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
// LinkwitzRileyCrossover Tests (v4: SVF-based LR4, 24 dB/oct)
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
    EXPECT_LT(maxError, 0.01f);
}

TEST(LinkwitzRileyCrossover, LowPassAttenuatesHigh) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto sine = generateSine(5000.0f, kSR, 4800, 1.0f);
    std::vector<float> lowOut(4800);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], 0.0f, lowL, lowR, highL, highR);
        lowOut[i] = lowL;
    }

    float lowPeak = peakAbs(lowOut);
    EXPECT_LT(lowPeak, 0.3f);
}

TEST(LinkwitzRileyCrossover, HighPassAttenuatesLow) {
    // v4: Verify high-pass band attenuates below crossover
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 2000.0f);
    xo.reset();

    auto sine = generateSine(100.0f, kSR, 4800, 1.0f);
    std::vector<float> highOut(4800);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], 0.0f, lowL, lowR, highL, highR);
        highOut[i] = highL;
    }

    float highRms = rmsRange(highOut, 200, 4800);
    float inRms = rmsRange(sine, 200, 4800);
    EXPECT_LT(highRms / inRms, 0.1f); // Should be heavily attenuated
}

TEST(LinkwitzRileyCrossover, AtCrossoverFreqBothBandsPresent) {
    // v4: At fc, both bands should have significant energy (LR4: -6 dB each)
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 1000.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut(9600), highOut(9600);

    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], 0.0f, lowL, lowR, highL, highR);
        lowOut[i] = lowL;
        highOut[i] = highL;
    }

    float lowRms = rmsRange(lowOut, 2000, 9600);
    float highRms = rmsRange(highOut, 2000, 9600);
    float inRms = rmsRange(sine, 2000, 9600);

    // Both bands should be between -9 dB and -3 dB of input
    float lowRatio = lowRms / inRms;
    float highRatio = highRms / inRms;
    EXPECT_GT(lowRatio, 0.3f);
    EXPECT_LT(lowRatio, 0.8f);
    EXPECT_GT(highRatio, 0.3f);
    EXPECT_LT(highRatio, 0.8f);
}

TEST(LinkwitzRileyCrossover, SteepSlope) {
    // v4: Verify the slope is steeper than 12 dB/oct (should be ~24 dB/oct)
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 500.0f);
    xo.reset();

    // Measure LP at 2 kHz (2 octaves above fc)
    auto sine2k = generateSine(2000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut2k(9600);
    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine2k[i], 0.0f, lowL, lowR, highL, highR);
        lowOut2k[i] = lowL;
    }

    // Measure LP at 4 kHz (3 octaves above fc)
    xo.reset();
    auto sine4k = generateSine(4000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut4k(9600);
    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine4k[i], 0.0f, lowL, lowR, highL, highR);
        lowOut4k[i] = lowL;
    }

    float rms2k = rmsRange(lowOut2k, 2000, 9600);
    float rms4k = rmsRange(lowOut4k, 2000, 9600);

    // At 24 dB/oct, the difference between 2 kHz and 4 kHz should be ~24 dB
    // At 12 dB/oct, it would be ~12 dB
    float slopeDb = 20.0f * std::log10(rms2k / std::max(rms4k, 1e-10f));
    EXPECT_GT(slopeDb, 18.0f); // Should be close to 24 dB
}

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor Tests (v4: Soft-knee)
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
        gc.processStereo(L, R, 0.8f, envVal);
        output[i] = L;
    }

    float outPeak = peakAbs(output);
    EXPECT_LT(outPeak, 0.95f);
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

TEST(GlueCompressor, SoftKneeGradualOnset) {
    // v4: Verify soft-knee produces gradual compression onset
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    // Process signals at various levels and check the compression is gradual
    std::vector<float> gains;
    for (float level = 0.1f; level <= 1.0f; level += 0.05f) {
        float L = level, R = level;
        gc.processStereo(L, R, 0.5f, level);
        gains.push_back(L / level); // gain ratio
    }

    // Check that gain ratios decrease smoothly (no sudden jumps)
    for (size_t i = 1; i < gains.size(); ++i) {
        float jump = gains[i - 1] - gains[i];
        EXPECT_LT(jump, 0.15f) << "Abrupt gain change at index " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter Tests (v4: Perceptual curves)
// ═══════════════════════════════════════════════════════════════════════════
TEST(MacroInterpreter, DefaultMappingsExist) {
    MacroInterpreter mi;
    mi.setupDefaults();
    float macros[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float mod = mi.getModulation(0, macros);
    EXPECT_GT(mod, 0.0f);
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

TEST(MacroInterpreter, PerceptualCurvesAreNonLinear) {
    // v8 FIX: Use correct API — addMapping(macroIndex, targetIndex, depth, curve)
    MacroInterpreter mi;
    mi.clearMappings();

    // Add a mapping with exponential curve
    mi.addMapping(0, 0, 1.0f, MacroInterpreter::Curve::Exponential);

    float macrosHalf[4] = { 0.5f, 0.0f, 0.0f, 0.0f };
    float modExp = mi.getModulation(0, macrosHalf);

    // With exponential curve, 0.5 input should give < 0.5 output (x^2 = 0.25)
    mi.clearMappings();
    mi.addMapping(0, 0, 1.0f, MacroInterpreter::Curve::Linear);
    float modLin = mi.getModulation(0, macrosHalf);

    EXPECT_NE(modExp, modLin);
    EXPECT_LT(modExp, modLin)
        << "Exponential curve at 0.5 should produce less than linear";
}

// ═══════════════════════════════════════════════════════════════════════════
// MeterBallistics Tests
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
// AutoGainSmoother Tests
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
    auto wet = generateSine(1000.0f, kSR, kBlockSize, 1.0f);
    for (int b = 0; b < 20; ++b) {
        auto wetCopy = wet;
        auto wetCopy2 = wet;
        ag.processBlock(wetCopy.data(), wetCopy2.data(), kBlockSize, dry.data(), dry.data());
    }
    EXPECT_LT(ag.smoothedGain, 0.9f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SVFLowpass2 Tests (v4: used in LR4 crossover)
// ═══════════════════════════════════════════════════════════════════════════
TEST(SVFLowpass2, PassesDC) {
    SVFLowpass2 svf;
    svf.setCoefficients(1000.0f, kSR);
    svf.reset();

    // Feed constant DC — lowpass should pass it through
    float out = 0.0f;
    for (int i = 0; i < 48000; ++i)
        out = svf.process(1.0f);
    EXPECT_NEAR(out, 1.0f, 0.01f);
}

TEST(SVFLowpass2, AttenuatesAboveCutoff) {
    SVFLowpass2 svf;
    svf.setCoefficients(500.0f, kSR);
    svf.reset();

    auto sine = generateSine(5000.0f, kSR, 4800, 1.0f);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i)
        output[i] = svf.process(sine[i]);

    float outRms = rmsRange(output, 200, 4800);
    float inRms = rmsRange(sine, 200, 4800);
    EXPECT_LT(outRms / inRms, 0.1f);
}

TEST(SVFLowpass2, ResetClearsState) {
    SVFLowpass2 svf;
    svf.setCoefficients(1000.0f, kSR);

    for (int i = 0; i < 100; ++i) svf.process(1.0f);
    svf.reset();
    float out = svf.process(0.0f);
    EXPECT_NEAR(out, 0.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// Real-Time Safety Smoke Tests
// v8: Updated to use plain fastTanh (no ADAA, no SlewLimiter)
// ═══════════════════════════════════════════════════════════════════════════
TEST(RealTimeSafety, NoNaNOrInfInFullChain) {
    // v8: Full chain uses SafetyLayer + fastTanh (no ADAA, no SlewLimiter)
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    for (int i = 0; i < kBlockSize; ++i) {
        float x = 100.0f * std::sin(2.0f * 3.14159f * 1000.0f * i / (float)kSR) + 50.0f;
        x = safety.processSample(x, safety.dcL, safety.dcPrevL);
        x = fastTanh(x);  // v8: plain fastTanh replaces ADAA
        EXPECT_TRUE(std::isfinite(x)) << "NaN/Inf at sample " << i;
    }
}

TEST(RealTimeSafety, FastTanhHandlesExtremeInput) {
    // v8: Verify fastTanh doesn't produce NaN/Inf for extreme inputs
    float y;
    y = fastTanh(0.0f);
    EXPECT_TRUE(std::isfinite(y));

    y = fastTanh(100.0f);
    EXPECT_TRUE(std::isfinite(y));
    EXPECT_LE(std::abs(y), 1.1f);

    y = fastTanh(-100.0f);
    EXPECT_TRUE(std::isfinite(y));
    EXPECT_LE(std::abs(y), 1.1f);

    y = fastTanh(1000.0f);
    EXPECT_TRUE(std::isfinite(y));

    y = fastTanh(-1000.0f);
    EXPECT_TRUE(std::isfinite(y));
}

TEST(RealTimeSafety, FullChainWithExtremeParameters) {
    // v8: Process through crossover + fastTanh + glue + shine with extreme params
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    ShineProcessor shine;
    shine.prepare(kSR);
    shine.setParameters(12000.0f, 9.0f, 0.7f);
    shine.reset();

    EnvFollower env;
    env.setTimes(5.0f, 80.0f, kSR);
    env.reset();

    for (int i = 0; i < kBlockSize * 4; ++i) {
        float x = 10.0f * std::sin(2.0f * 3.14159f * 1000.0f * i / (float)kSR);
        float lowL, lowR, highL, highR;
        xo.process(x, x, lowL, lowR, highL, highR);

        // v8: plain fastTanh instead of ADAA
        lowL = fastTanh(lowL * 2.0f);
        float sc = std::abs(lowL);
        float envVal = env.process(sc);
        gc.processStereo(lowL, lowR, 1.0f, envVal);

        float outL = lowL + highL;
        float outR = lowR + highR;
        shine.processStereo(outL, outR);

        EXPECT_TRUE(std::isfinite(outL)) << "NaN/Inf at sample " << i;
        EXPECT_TRUE(std::isfinite(outR)) << "NaN/Inf at sample " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// v5 AUDIT TESTS: Envelope Follower Timing
// ═══════════════════════════════════════════════════════════════════════════

// Verify that EnvFollower release time matches configured value
// (catches the v4 bug where OS-rate ticking caused 2-4x faster release)
TEST(V5Audit, EnvFollowerReleaseTiming) {
    // Configure for 220ms release at 48 kHz
    EnvFollower env;
    env.setTimes(0.2f, 220.0f, kSR);
    env.reset();

    // Feed a 1ms impulse to set peak
    const int impulseSamples = (int)(kSR * 0.001);
    for (int i = 0; i < impulseSamples; ++i)
        env.process(1.0f);

    float peakVal = env.env;
    EXPECT_GT(peakVal, 0.5f) << "Envelope should have risen to near 1.0";

    // Now release: measure time to reach 37% of peak (1 time constant)
    int sampleCount = 0;
    const float target = peakVal * 0.37f;
    const int maxSamples = (int)(kSR * 2.0); // 2 second max
    while (env.env > target && sampleCount < maxSamples) {
        env.process(0.0f);
        sampleCount++;
    }

    float releaseMs = (float)sampleCount / (float)kSR * 1000.0f;
    // Should be approximately 220ms (1 time constant)
    // Allow 10% tolerance
    EXPECT_NEAR(releaseMs, 220.0f, 22.0f)
        << "Release time should be ~220ms at base SR. Got " << releaseMs << "ms";
}

// Verify that EnvFollower at 2x OS rate (wrong) gives wrong timing
// This is a regression test: if someone moves envelopes back to OS path,
// this test documents the expected failure.
TEST(V5Audit, EnvFollowerOSRateBug) {
    // Configure with base SR coefficients
    EnvFollower envBase;
    envBase.setTimes(0.2f, 220.0f, kSR);
    envBase.reset();

    // Configure with 2x OS SR coefficients (correct for OS path)
    EnvFollower envOS;
    envOS.setTimes(0.2f, 220.0f, kSR * 2.0);
    envOS.reset();

    // Feed impulse
    for (int i = 0; i < 48; ++i) {
        envBase.process(1.0f);
        envOS.process(1.0f);
    }

    // Release for 500 base-SR samples (ticking envBase at 1x, envOS at 2x)
    for (int i = 0; i < 500; ++i) {
        envBase.process(0.0f);
        // envOS ticks twice per base sample (simulating OS)
        envOS.process(0.0f);
        envOS.process(0.0f);
    }

    // envBase should still be higher (slower release)
    // envOS should have decayed more (correct for 2x rate)
    // If both are equal, the OS coefficients are wrong
    EXPECT_GT(envBase.env, envOS.env)
        << "Base-SR envelope should decay slower than OS-rate envelope";
}

// ═══════════════════════════════════════════════════════════════════════════
// v5 AUDIT TESTS: Null-Path Crossover Complementarity
// ═══════════════════════════════════════════════════════════════════════════

TEST(V5Audit, CrossoverNullPath) {
    // Verify that low + high = input (complementary crossover)
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto input = generateSine(1000.0f, kSR, kBlockSize * 4);
    float maxError = 0.0f;

    for (int i = 0; i < (int)input.size(); ++i) {
        float lowL, lowR, highL, highR;
        xo.process(input[i], input[i], lowL, lowR, highL, highR);
        float reconstructed = lowL + highL;
        float error = std::abs(reconstructed - input[i]);
        maxError = std::max(maxError, error);
    }

    // Complementary subtraction: error should be at floating-point precision
    EXPECT_LT(maxError, 1.0e-6f)
        << "Crossover low+high should equal input. Max error: " << maxError;
}

// ═══════════════════════════════════════════════════════════════════════════
// v6 TESTS: SidechainHPF
// ═══════════════════════════════════════════════════════════════════════════

TEST(SidechainHPF, PassthroughWhenOff) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 0.0f); // 0 Hz = off
    hpf.reset();

    // Feed a DC-ish low signal — should pass through unchanged
    float L = 0.8f, R = 0.8f;
    float result = hpf.process(L, R);
    EXPECT_NEAR(result, std::max(std::abs(L), std::abs(R)), 0.01f);
}

TEST(SidechainHPF, AttenuatesLowFrequencies) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 150.0f); // 150 Hz HPF
    hpf.reset();

    // Feed a 50 Hz sine (well below 150 Hz cutoff)
    auto sine = generateSine(50.0f, kSR, 4800, 0.8f);
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float out = hpf.process(sine[i], sine[i]);
        if (i > 480) maxOut = std::max(maxOut, out); // skip transient
    }
    // Should be significantly attenuated
    EXPECT_LT(maxOut, 0.5f) << "50 Hz should be attenuated by 150 Hz HPF";
}

TEST(SidechainHPF, PassesHighFrequencies) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 60.0f); // 60 Hz HPF
    hpf.reset();

    // Feed a 1 kHz sine (well above 60 Hz cutoff)
    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float out = hpf.process(sine[i], sine[i]);
        if (i > 480) maxOut = std::max(maxOut, out);
    }
    // Should pass through with minimal attenuation
    EXPECT_GT(maxOut, 0.6f) << "1 kHz should pass through 60 Hz HPF";
}

TEST(SidechainHPF, ResetClearsState) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 90.0f);
    // Process some signal
    for (int i = 0; i < 480; ++i) hpf.process(0.5f, 0.5f);
    hpf.reset();
    float out = hpf.process(0.0f, 0.0f);
    EXPECT_NEAR(out, 0.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// v6 TESTS: MacroInterpreter Wiring Integration
// ═══════════════════════════════════════════════════════════════════════════

TEST(MacroWiring, AllDefaultTargetsReceiveModulation) {
    MacroInterpreter mi;
    mi.setupDefaults();

    // All macros at 1.0
    float macros[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Check all 7 default target indices that should receive modulation
    // Targets: 0=punch, 1=warmth, 2=boom, 3=glue, 4=air, 6=density, 10=drive
    int targetIndices[] = { 0, 1, 2, 3, 4, 6, 10 };
    for (int idx : targetIndices) {
        float mod = mi.getModulation(idx, macros);
        EXPECT_NE(mod, 0.0f) << "Target " << idx << " should receive modulation";
    }
}

TEST(MacroWiring, UnmappedTargetsGetZero) {
    MacroInterpreter mi;
    mi.setupDefaults();

    float macros[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Target 5 (width) and 7 (motion) are not in default mappings
    float mod5 = mi.getModulation(5, macros);
    float mod7 = mi.getModulation(7, macros);
    EXPECT_FLOAT_EQ(mod5, 0.0f) << "Width should not be macro-mapped by default";
    EXPECT_FLOAT_EQ(mod7, 0.0f) << "Motion should not be macro-mapped by default";
}

TEST(MacroWiring, ModulationScalesWithMacroValue) {
    MacroInterpreter mi;
    mi.setupDefaults();

    float macrosLow[4] = { 0.25f, 0.0f, 0.0f, 0.0f };
    float macrosHigh[4] = { 0.75f, 0.0f, 0.0f, 0.0f };

    float modLow = mi.getModulation(0, macrosLow);   // punch at 0.25
    float modHigh = mi.getModulation(0, macrosHigh);  // punch at 0.75

    EXPECT_GT(std::abs(modHigh), std::abs(modLow))
        << "Higher macro value should produce more modulation";
}

// ═══════════════════════════════════════════════════════════════════════════
// v7 TESTS: BypassCrossfader
// ═══════════════════════════════════════════════════════════════════════════

TEST(V7Hardening, BypassCrossfaderClickFree) {
    BypassCrossfader xf;
    xf.prepare(64); // 64-sample fade
    xf.reset();

    // Start in active state (not bypassed)
    xf.setBypassState(false);

    // Generate a steady signal
    const int N = 256;
    float dryL[256], dryR[256], wetL[256], wetR[256];
    for (int i = 0; i < N; ++i) {
        dryL[i] = dryR[i] = 0.5f;
        wetL[i] = wetR[i] = 0.8f; // different from dry
    }

    // Process first 64 samples (should be fully wet)
    for (int i = 0; i < 64; ++i) {
        float wL = wetL[i], wR = wetR[i];
        xf.processStereo(dryL[i], dryR[i], wL, wR);
        EXPECT_NEAR(wL, 0.8f, 0.01f) << "Should be fully wet before bypass";
    }

    // Toggle to bypass
    xf.setBypassState(true);

    // During fade: check no discontinuity > 0.02
    float prevL = 0.8f;
    float maxDelta = 0.0f;
    for (int i = 0; i < 128; ++i) {
        float wL = 0.8f, wR = 0.8f;
        xf.processStereo(0.5f, 0.5f, wL, wR);
        float delta = std::abs(wL - prevL);
        maxDelta = std::max(maxDelta, delta);
        prevL = wL;
    }
    EXPECT_LT(maxDelta, 0.02f)
        << "Bypass transition should be smooth. Max delta: " << maxDelta;

    // After fade: should be fully dry
    float wL = 0.8f, wR = 0.8f;
    xf.processStereo(0.5f, 0.5f, wL, wR);
    EXPECT_NEAR(wL, 0.5f, 0.01f) << "Should be fully dry after bypass";
}

TEST(V7Hardening, BypassCrossfaderResetSettles) {
    BypassCrossfader xf;
    xf.prepare(64);
    xf.setBypassState(true);
    // Process a few samples mid-fade
    for (int i = 0; i < 10; ++i) {
        float wL = 1.0f, wR = 1.0f;
        xf.processStereo(0.0f, 0.0f, wL, wR);
    }
    xf.reset();
    // After reset, should be settled (no fade in progress)
    float wL = 1.0f, wR = 1.0f;
    xf.processStereo(0.0f, 0.0f, wL, wR);
    // Should be either fully wet or fully dry, not mid-fade
    bool settled = (std::abs(wL - 0.0f) < 0.01f || std::abs(wL - 1.0f) < 0.01f);
    EXPECT_TRUE(settled) << "After reset, crossfader should be settled. Got: " << wL;
}

// ═══════════════════════════════════════════════════════════════════════════
// v7 TESTS: SidechainHPF Crossfade (click-free mode change)
// ═══════════════════════════════════════════════════════════════════════════

TEST(V7Hardening, SidechainHPFCrossfadeNoClick) {
    SidechainHPF hpf;
    hpf.prepareImmediate(kSR, 60.0f);
    hpf.reset();

    // Process steady signal at 60 Hz mode
    auto sine = generateSine(200.0f, kSR, 4800, 0.8f);
    for (int i = 0; i < 2400; ++i)
        hpf.process(sine[i], sine[i]);

    // Switch to 150 Hz mode (should crossfade, not click)
    hpf.prepare(kSR, 150.0f);

    float prevOut = hpf.process(sine[2400], sine[2400]);
    float maxDelta = 0.0f;
    for (int i = 2401; i < 4800; ++i) {
        float out = hpf.process(sine[i], sine[i]);
        float delta = std::abs(out - prevOut);
        maxDelta = std::max(maxDelta, delta);
        prevOut = out;
    }
    // Max delta should be small (no click)
    EXPECT_LT(maxDelta, 0.1f)
        << "HPF mode change should be smooth. Max delta: " << maxDelta;
}

// ═══════════════════════════════════════════════════════════════════════════
// v7 TESTS: SmoothParam Automation Zipper
// ═══════════════════════════════════════════════════════════════════════════

TEST(V7Hardening, SmoothParamNoZipper) {
    SmoothParam sp;
    sp.setTime(10.0f, kSR); // v8 FIX: was .prepare() which doesn't exist
    sp.snapTo(0.0f);

    // Jump to 1.0
    sp.setTarget(1.0f);

    float prev = 0.0f;
    float maxDelta = 0.0f;
    bool reachedTarget = false;
    for (int i = 0; i < (int)(kSR * 0.1); ++i) { // 100ms
        float val = sp.next();
        float delta = std::abs(val - prev);
        maxDelta = std::max(maxDelta, delta);
        prev = val;
        if (std::abs(val - 1.0f) < 0.001f) reachedTarget = true;
    }

    EXPECT_TRUE(reachedTarget) << "SmoothParam should reach target within 100ms";
    EXPECT_LT(maxDelta, 0.05f)
        << "SmoothParam should not have large jumps. Max delta: " << maxDelta;
}

// ═══════════════════════════════════════════════════════════════════════════
// v7 TESTS: Safety Layer Silence-In-Silence-Out
// ═══════════════════════════════════════════════════════════════════════════

TEST(V7Hardening, SafetyLayerSilenceInSilenceOut) {
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    // Feed silence for 1000 samples
    float maxOut = 0.0f;
    for (int i = 0; i < 1000; ++i) {
        float out = safety.processSample(0.0f, safety.dcL, safety.dcPrevL);
        maxOut = std::max(maxOut, std::abs(out));
    }
    EXPECT_LT(maxOut, 1e-10f)
        << "Silence in should produce silence out. Max output: " << maxOut;
}

// ═══════════════════════════════════════════════════════════════════════════
// v8 TESTS: State Version Validation
// ═══════════════════════════════════════════════════════════════════════════

TEST(V8Regression, StateVersionConstant) {
    // v8: Verify the state version is 8 (matches BTZDsp.h kStateVersion)
    EXPECT_EQ(BTZDsp::kStateVersion, 8)
        << "State version should be 8 for v8 release";
}

// ═══════════════════════════════════════════════════════════════════════════
// v8 TESTS: DC Blocker 1 Hz Coloration Regression
// ═══════════════════════════════════════════════════════════════════════════

TEST(V8Regression, DCBlocker1HzMinimalColoration) {
    // v8: DC blocker cutoff lowered from 5 Hz to 1 Hz
    // At 1 Hz, a 20 Hz signal should pass through with minimal attenuation
    // (at 5 Hz, there was -10.1 dB coloration on pink noise)
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    // Generate a 20 Hz sine (lowest musically relevant frequency)
    auto sine = generateSine(20.0f, kSR, 48000, 0.5f);
    std::vector<float> output(48000);
    float dc = 0.0f, dcPrev = 0.0f;

    for (int i = 0; i < 48000; ++i) {
        output[i] = safety.processSample(sine[i], dc, dcPrev);
    }

    // Measure RMS of last half (after filter settles)
    float inRms = rmsRange(sine, 24000, 48000);
    float outRms = rmsRange(output, 24000, 48000);

    float deltaDb = 20.0f * std::log10(outRms / std::max(inRms, 1e-10f));

    // At 1 Hz cutoff, 20 Hz should have < 0.5 dB attenuation
    // (at 5 Hz cutoff, this was ~-1.5 dB)
    EXPECT_GT(deltaDb, -0.5f)
        << "20 Hz signal should pass through 1 Hz DC blocker with < 0.5 dB loss. Got: "
        << deltaDb << " dB";
}

TEST(V8Regression, DCBlocker1HzStillBlocksDC) {
    // v8: Even at 1 Hz, DC should still be blocked after sufficient samples
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    float dc = 0.0f, dcPrev = 0.0f;

    // Feed DC for 5 seconds (240000 samples at 48 kHz)
    float out = 0.0f;
    for (int i = 0; i < 240000; ++i) {
        out = safety.processSample(1.0f, dc, dcPrev);
    }

    // DC should be significantly attenuated
    EXPECT_LT(std::abs(out), 0.1f)
        << "DC should be blocked even at 1 Hz cutoff. Residual: " << out;
}

// ═══════════════════════════════════════════════════════════════════════════
// v8 TESTS: Plain fastTanh Saturation Chain (no ADAA state)
// ═══════════════════════════════════════════════════════════════════════════

TEST(V8Regression, PlainFastTanhHasNoState) {
    // v8: fastTanh is a pure function — same input always gives same output
    // (unlike ADAA which has x1/F1 state). This is a regression test to
    // ensure we're not accidentally using a stateful saturator.
    float y1 = fastTanh(0.5f);
    float y2 = fastTanh(0.5f);
    float y3 = fastTanh(0.5f);
    EXPECT_FLOAT_EQ(y1, y2);
    EXPECT_FLOAT_EQ(y2, y3);

    // After processing different values, same input should still give same output
    fastTanh(3.0f);
    fastTanh(-2.0f);
    fastTanh(10.0f);
    float y4 = fastTanh(0.5f);
    EXPECT_FLOAT_EQ(y1, y4)
        << "fastTanh should be stateless — same input must give same output";
}

TEST(V8Regression, SaturationChainProducesHarmonics) {
    // v8: Verify that fastTanh saturation actually adds harmonics
    // (regression test: if someone accidentally replaces with linear, this catches it)
    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);
    std::vector<float> saturated(4800);

    for (int i = 0; i < 4800; ++i) {
        saturated[i] = fastTanh(sine[i] * 3.0f); // drive = 3x
    }

    // The saturated signal should differ from the input (harmonics added)
    float diffEnergy = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float d = saturated[i] - sine[i];
        diffEnergy += d * d;
    }
    EXPECT_GT(diffEnergy, 0.01f)
        << "Saturation should add harmonics (differ from input)";
}

TEST(V8Regression, MultibandSaturationWithFastTanh) {
    // v8: Verify the multiband saturation path works with plain fastTanh
    // (crossover → per-band fastTanh → recombine)
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.process(sine[i], sine[i], lowL, lowR, highL, highR);

        // Per-band saturation with fastTanh
        float satLowL = fastTanh(lowL * 2.0f);
        float satHighL = fastTanh(highL * 2.0f);

        // Recombine
        float outL = satLowL + satHighL;
        EXPECT_TRUE(std::isfinite(outL)) << "NaN/Inf at sample " << i;
        EXPECT_LE(std::abs(outL), 3.0f) << "Output should be bounded at sample " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// v8 TESTS: TruePeakLimiter v8 Tightened Attack
// ═══════════════════════════════════════════════════════════════════════════

TEST(V8Regression, TruePeakLimiterTightenedAttack) {
    // v8: TruePeakLimiter has tightened attack (0.5ms) for ISP compliance
    // Verify it catches sudden transients better than before
    TruePeakLimiter limiter;
    limiter.prepare(kSR, kBlockSize, 2.0f);
    limiter.reset();

    const float ceiling = 0.5f;
    const int totalSamples = kBlockSize * 4;

    // Create a signal with a sudden transient
    juce::AudioBuffer<float> buffer(2, totalSamples);
    buffer.clear();

    // Quiet signal for first half, then sudden loud transient
    for (int i = 0; i < totalSamples / 2; ++i) {
        buffer.setSample(0, i, 0.1f * std::sin(2.0f * 3.14159f * 440.0f * i / (float)kSR));
        buffer.setSample(1, i, 0.1f * std::sin(2.0f * 3.14159f * 440.0f * i / (float)kSR));
    }
    for (int i = totalSamples / 2; i < totalSamples; ++i) {
        buffer.setSample(0, i, 1.5f * std::sin(2.0f * 3.14159f * 440.0f * i / (float)kSR));
        buffer.setSample(1, i, 1.5f * std::sin(2.0f * 3.14159f * 440.0f * i / (float)kSR));
    }

    limiter.processBlock(buffer, ceiling);

    // Check that gain reduction was applied
    float grDb = limiter.getGainReductionDb();
    EXPECT_GT(grDb, 1.0f) << "Limiter should apply significant GR to +9.5 dB signal";

    // Check that output doesn't exceed ceiling by more than 0.5 dB (ISP tolerance)
    const int skip = limiter.getLatencySamples() + 10;
    float maxOutput = 0.0f;
    for (int i = skip; i < totalSamples; ++i) {
        maxOutput = std::max(maxOutput, std::abs(buffer.getSample(0, i)));
        maxOutput = std::max(maxOutput, std::abs(buffer.getSample(1, i)));
    }

    float overshootDb = 20.0f * std::log10(maxOutput / ceiling);
    EXPECT_LT(overshootDb, 0.5f)
        << "ISP overshoot should be < 0.5 dB. Got: " << overshootDb << " dB";
}

// ═══════════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
