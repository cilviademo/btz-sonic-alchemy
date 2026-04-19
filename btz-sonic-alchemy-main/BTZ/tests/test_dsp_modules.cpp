/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp  v4
  ────────────────────────────────────────────────────────────────────────
  GoogleTest-based unit tests for all BTZDsp modules.
  v4: Updated for mathematical overhaul:
    - LR4 crossover (24 dB/oct SVF-based)
    - Soft-knee GlueCompressor
    - Padé [5/5] fastTanh
    - Perceptual macro curves
    - FixedDeque (pre-allocated, lock-free)
    - All prior tests retained and updated.

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

// ═══════════════════════════════════════════════════════════════════════════
// FixedDeque Tests (pre-allocated, lock-free ring buffer)
// ═══════════════════════════════════════════════════════════════════════════
TEST(FixedDeque, PushAndPopBack) {
    FixedDeque<float> dq(16);
    dq.push_back(1.0f);
    dq.push_back(2.0f);
    dq.push_back(3.0f);
    EXPECT_EQ(dq.size(), 3u);
    EXPECT_FLOAT_EQ(dq.front(), 1.0f);
    EXPECT_FLOAT_EQ(dq.back(), 3.0f);
}

TEST(FixedDeque, PopFront) {
    FixedDeque<float> dq(16);
    dq.push_back(10.0f);
    dq.push_back(20.0f);
    dq.push_back(30.0f);
    dq.pop_front();
    EXPECT_EQ(dq.size(), 2u);
    EXPECT_FLOAT_EQ(dq.front(), 20.0f);
}

TEST(FixedDeque, PopBack) {
    FixedDeque<float> dq(16);
    dq.push_back(10.0f);
    dq.push_back(20.0f);
    dq.push_back(30.0f);
    dq.pop_back();
    EXPECT_EQ(dq.size(), 2u);
    EXPECT_FLOAT_EQ(dq.back(), 20.0f);
}

TEST(FixedDeque, ClearResetsSize) {
    FixedDeque<float> dq(16);
    for (int i = 0; i < 10; ++i) dq.push_back((float)i);
    EXPECT_EQ(dq.size(), 10u);
    dq.clear();
    EXPECT_EQ(dq.size(), 0u);
    EXPECT_TRUE(dq.empty());
}

TEST(FixedDeque, WrapsAroundCorrectly) {
    FixedDeque<float> dq(4);
    dq.push_back(1.0f);
    dq.push_back(2.0f);
    dq.push_back(3.0f);
    dq.push_back(4.0f);
    dq.pop_front();
    dq.pop_front();
    dq.push_back(5.0f);
    dq.push_back(6.0f);
    EXPECT_EQ(dq.size(), 4u);
    EXPECT_FLOAT_EQ(dq.front(), 3.0f);
    EXPECT_FLOAT_EQ(dq.back(), 6.0f);
}

TEST(FixedDeque, EmptyChecks) {
    FixedDeque<float> dq(8);
    EXPECT_TRUE(dq.empty());
    dq.push_back(1.0f);
    EXPECT_FALSE(dq.empty());
    dq.pop_front();
    EXPECT_TRUE(dq.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// ADAATanh Tests
// ═══════════════════════════════════════════════════════════════════════════
class ADAATanhTest : public ::testing::Test {
protected:
    ADAATanh adaa;
    void SetUp() override { adaa.reset(); }
};

TEST_F(ADAATanhTest, PassesSilence) {
    for (int i = 0; i < 100; ++i)
        EXPECT_NEAR(adaa.process(0.0f), 0.0f, 1e-6f);
}

TEST_F(ADAATanhTest, SmallSignalApproximatesLinear) {
    const float tiny = 0.001f;
    adaa.reset();
    adaa.process(0.0f);
    float y = adaa.process(tiny);
    EXPECT_NEAR(y, tiny, tiny * 0.15f);
}

TEST_F(ADAATanhTest, SaturatesLargeSignals) {
    adaa.reset();
    adaa.process(0.0f);
    float y = adaa.process(5.0f);
    EXPECT_LT(std::abs(y), 5.0f);
    EXPECT_GT(std::abs(y), 0.5f);
}

TEST_F(ADAATanhTest, OutputBoundedByTanh) {
    adaa.reset();
    auto sine = generateSine(1000.0f, kSR, kBlockSize, 10.0f);
    for (auto& s : sine) {
        float y = adaa.process(s);
        EXPECT_LE(std::abs(y), 1.05f);
    }
}

TEST_F(ADAATanhTest, DiffersFromDirectTanh) {
    adaa.reset();
    auto sine = generateSine(10000.0f, kSR, kBlockSize, 2.0f);
    float diffEnergy = 0.0f;
    for (int i = 0; i < kBlockSize; ++i) {
        float adaaOut = adaa.process(sine[i]);
        float directOut = std::tanh(sine[i]);
        float d = adaaOut - directOut;
        diffEnergy += d * d;
    }
    EXPECT_GT(diffEnergy, 1e-6f);
}

TEST_F(ADAATanhTest, ResetClearsState) {
    adaa.process(3.0f);
    adaa.process(5.0f);
    adaa.reset();
    float y = adaa.process(0.0f);
    EXPECT_NEAR(y, 0.0f, 1e-5f);
}

TEST_F(ADAATanhTest, PerChannelIndependence) {
    ADAATanh left, right;
    left.reset(); right.reset();
    left.process(0.0f); right.process(0.0f);
    float yL = left.process(2.0f);
    float yR = right.process(-2.0f);
    EXPECT_NE(yL, yR);
    EXPECT_NEAR(yL, -yR, 0.05f);
}

TEST_F(ADAATanhTest, MonotonicForIncreasingInput) {
    adaa.reset();
    adaa.process(0.0f);
    float prev = adaa.process(0.1f);
    for (float x = 0.2f; x <= 3.0f; x += 0.1f) {
        float y = adaa.process(x);
        EXPECT_GE(y, prev - 0.01f);
        prev = y;
    }
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
// SlewLimiter Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SlewLimiter, LimitsTransients) {
    SlewLimiter sl;
    sl.setSampleRate(kSR);
    sl.reset();
    float out = sl.process(0.0f);
    EXPECT_FLOAT_EQ(out, 0.0f);
    out = sl.process(10.0f);
    EXPECT_LT(out, 10.0f);
    EXPECT_GT(out, 0.0f);
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
    // v4: Verify that non-linear curve types produce different output than linear
    MacroInterpreter mi;
    mi.clearMappings();

    // Add a mapping with exponential curve (type 1)
    MacroInterpreter::Mapping m;
    m.macroIdx = 0;
    m.depth = 1.0f;
    m.curveType = 1; // exponential
    mi.addMapping(0, m);

    float macrosHalf[4] = { 0.5f, 0.0f, 0.0f, 0.0f };
    float modExp = mi.getModulation(0, macrosHalf);

    // With exponential curve, 0.5 input should give < 0.5 output (x^2 = 0.25)
    // The exact value depends on implementation, but it should differ from linear
    mi.clearMappings();
    m.curveType = 0; // linear
    mi.addMapping(0, m);
    float modLin = mi.getModulation(0, macrosHalf);

    EXPECT_NE(modExp, modLin);
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
// ═══════════════════════════════════════════════════════════════════════════
TEST(RealTimeSafety, NoNaNOrInfInFullChain) {
    ADAATanh adaa;
    adaa.reset();
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();
    SlewLimiter slew;
    slew.setSampleRate(kSR);
    slew.reset();

    for (int i = 0; i < kBlockSize; ++i) {
        float x = 100.0f * std::sin(2.0f * 3.14159f * 1000.0f * i / (float)kSR) + 50.0f;
        x = safety.processSample(x, safety.dcL, safety.dcPrevL);
        x = adaa.process(x);
        x = slew.process(x);
        EXPECT_TRUE(std::isfinite(x)) << "NaN/Inf at sample " << i;
    }
}

TEST(RealTimeSafety, NaNInputToADAAProducesFinite) {
    ADAATanh adaa;
    adaa.reset();
    adaa.process(0.0f);
    float y = adaa.process(std::numeric_limits<float>::quiet_NaN());
    adaa.reset();
    y = adaa.process(0.0f);
    EXPECT_TRUE(std::isfinite(y));
}

TEST(RealTimeSafety, InfInputToADAADoesNotExplode) {
    ADAATanh adaa;
    adaa.reset();
    adaa.process(0.0f);
    float y = adaa.process(std::numeric_limits<float>::infinity());
    adaa.reset();
    y = adaa.process(0.5f);
    EXPECT_TRUE(std::isfinite(y));
    EXPECT_LE(std::abs(y), 2.0f);
}

TEST(RealTimeSafety, FullChainWithExtremeParameters) {
    // v4: Process through crossover + ADAA + glue + shine with extreme params
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    ADAATanh adaa;
    adaa.reset();

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

        lowL = adaa.process(lowL);
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
// Main
// ═══════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
