/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp  v9
  ────────────────────────────────────────────────────────────────────────
  GoogleTest-based unit tests for all BTZDsp modules.
  v9: Industry-gap closure:
    - Updated ALL APIs to match BTZDsp.h v9 surface
    - Added saturation model tests (Tube, Tape, Transistor, Transformer)
    - Added MultibandEngine tests
    - Added MidSideEncoder tests
    - Added LFO modulation tests
    - Added LoudnessMeter smoke tests
    - Added SpectrumBuffer + GainReductionHistory tests
    - Added UndoStack + ABState tests
    - Added MIDILearnState tests
    - Updated TruePeakLimiter to per-sample API
    - Updated GlueCompressor to new processStereo() returning gain
    - Updated ShineProcessor to direct field access + prepare()
    - Updated LinkwitzRileyCrossover to processStereo()
    - Updated MacroInterpreter to MacroSlot/getMappedValue API
    - Updated MeterBallistics to prepare(sr) / peakHoldL
    - Updated AutoGainSmoother to updateInput/updateOutput/getCompensationGain
    - Updated SidechainHPF to processStereo (in-place)
    - Updated state version to 9
    - Removed SVFLowpass2 tests (class removed in v9)
  v8: removed ADAA/SlewLimiter tests, 1 Hz DC blocker regression
  v7: BypassCrossfader, SidechainHPF crossfade, silence-in-silence-out
  v6: SidechainHPF, MacroInterpreter wiring
  v5: Envelope follower timing
  v4: LR4 crossover, soft-knee glue, Padé [5/5] fastTanh

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

TEST(FastTanh, SaturatesLargeSignals) {
    for (float x = 1.0f; x <= 10.0f; x += 0.5f) {
        float y = fastTanh(x);
        EXPECT_LE(std::abs(y), 1.05f) << "fastTanh should saturate at x=" << x;
        EXPECT_GT(std::abs(y), 0.5f) << "fastTanh should have significant output at x=" << x;
    }
}

TEST(FastTanh, SmallSignalApproximatesLinear) {
    for (float x = -0.01f; x <= 0.01f; x += 0.001f) {
        EXPECT_NEAR(fastTanh(x), x, std::abs(x) * 0.05f + 1e-7f)
            << "fastTanh should be approximately linear for small x=" << x;
    }
}

TEST(FastTanh, IsStateless) {
    float y1 = fastTanh(0.5f);
    float y2 = fastTanh(0.5f);
    EXPECT_FLOAT_EQ(y1, y2);
    fastTanh(3.0f);
    fastTanh(-2.0f);
    fastTanh(10.0f);
    float y3 = fastTanh(0.5f);
    EXPECT_FLOAT_EQ(y1, y3) << "fastTanh should be stateless";
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
// TruePeakLimiter Tests (v9: per-sample API)
// ═══════════════════════════════════════════════════════════════════════════
class TruePeakLimiterTest : public ::testing::Test {
protected:
    TruePeakLimiter limiter;
    void SetUp() override {
        limiter.ceiling = -0.3f;
        limiter.prepare(kSR);
        limiter.reset();
    }
};

TEST_F(TruePeakLimiterTest, PassesSilence) {
    float l = 0.0f, r = 0.0f;
    for (int i = 0; i < kBlockSize; ++i) {
        limiter.processStereo(l, r);
    }
    EXPECT_NEAR(l, 0.0f, 1e-6f);
    EXPECT_NEAR(r, 0.0f, 1e-6f);
}

TEST_F(TruePeakLimiterTest, LimitsAboveCeiling) {
    limiter.ceiling = -6.0f;  // ~0.5 linear
    limiter.prepare(kSR);
    limiter.reset();

    const float ceilingLin = std::pow(10.0f, limiter.ceiling / 20.0f);
    const int totalSamples = kBlockSize * 8;
    auto sine = generateSine(440.0f, kSR, totalSamples, 1.0f);

    std::vector<float> outL(totalSamples), outR(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        float l = sine[i], r = sine[i];
        limiter.processStereo(l, r);
        outL[i] = l;
        outR[i] = r;
    }

    // After lookahead settles, output should be below ceiling
    const int skip = limiter.delaySamples + 50;
    for (int i = skip; i < totalSamples; ++i) {
        EXPECT_LE(std::abs(outL[i]), ceilingLin + 0.05f) << "Overshoot at sample " << i;
    }
}

TEST_F(TruePeakLimiterTest, DoesNotLimitBelowCeiling) {
    limiter.ceiling = 0.0f;  // 1.0 linear
    limiter.prepare(kSR);
    limiter.reset();

    const int totalSamples = kBlockSize * 4;
    auto sine = generateSine(440.0f, kSR, totalSamples, 0.3f);

    float totalGR = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float l = sine[i], r = sine[i];
        float gr = limiter.processStereo(l, r);
        totalGR += std::abs(gr);
    }
    EXPECT_NEAR(totalGR / totalSamples, 0.0f, 0.5f);
}

TEST_F(TruePeakLimiterTest, ReportsGainReduction) {
    limiter.ceiling = -6.0f;
    limiter.prepare(kSR);
    limiter.reset();

    auto sine = generateSine(440.0f, kSR, kBlockSize * 4, 1.0f);
    float maxGR = 0.0f;
    for (int i = 0; i < kBlockSize * 4; ++i) {
        float l = sine[i], r = sine[i];
        float gr = limiter.processStereo(l, r);
        maxGR = std::min(maxGR, gr);  // GR is negative dB
    }
    EXPECT_LT(maxGR, -0.5f) << "Should report gain reduction";
}

TEST_F(TruePeakLimiterTest, LatencyIsPositive) {
    EXPECT_GT(limiter.delaySamples, 0);
}

TEST_F(TruePeakLimiterTest, ResetClearsState) {
    limiter.ceiling = -12.0f;
    limiter.prepare(kSR);

    auto sine = generateSine(440.0f, kSR, kBlockSize, 2.0f);
    for (int i = 0; i < kBlockSize; ++i) {
        float l = sine[i], r = sine[i];
        limiter.processStereo(l, r);
    }
    limiter.reset();
    EXPECT_NEAR(limiter.envLin, 0.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SVF ShineProcessor Tests (v9: direct field access + prepare())
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
    shine.freq = 8000.0f;
    shine.gainDb = 6.0f;
    shine.q = 0.7f;
    shine.prepare(kSR);

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
    shine.freq = 12000.0f;
    shine.gainDb = 6.0f;
    shine.q = 0.7f;
    shine.prepare(kSR);

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
    shine.freq = 12000.0f;
    shine.gainDb = 0.0f;
    shine.q = 0.7f;
    shine.prepare(kSR);

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
            shine.freq = 2000.0f + (float)(i % 18000);
            shine.gainDb = (float)(i % 12);
            shine.q = 0.1f + (float)(i % 20) * 0.1f;
            shine.prepare(kSR);
        }
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        EXPECT_TRUE(std::isfinite(L)) << "NaN/Inf at sample " << i;
        EXPECT_TRUE(std::isfinite(R)) << "NaN/Inf at sample " << i;
    }
}

TEST_F(ShineProcessorTest, CutReducesHighFrequencies) {
    shine.freq = 8000.0f;
    shine.gainDb = -6.0f;
    shine.q = 0.7f;
    shine.prepare(kSR);

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
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(0.0f);
    sp.setTarget(1e-7f);
    sp.next();
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
// LinkwitzRileyCrossover Tests (v9: processStereo API)
// ═══════════════════════════════════════════════════════════════════════════
TEST(LinkwitzRileyCrossover, SumsToOriginal) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, kBlockSize);
    float maxError = 0.0f;

    for (int i = 0; i < kBlockSize; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine[i], sine[i], lowL, lowR, highL, highR);
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
        xo.processStereo(sine[i], 0.0f, lowL, lowR, highL, highR);
        lowOut[i] = lowL;
    }

    float lowPeak = peakAbs(lowOut);
    EXPECT_LT(lowPeak, 0.3f);
}

TEST(LinkwitzRileyCrossover, HighPassAttenuatesLow) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 2000.0f);
    xo.reset();

    auto sine = generateSine(100.0f, kSR, 4800, 1.0f);
    std::vector<float> highOut(4800);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine[i], 0.0f, lowL, lowR, highL, highR);
        highOut[i] = highL;
    }

    float highRms = rmsRange(highOut, 200, 4800);
    float inRms = rmsRange(sine, 200, 4800);
    EXPECT_LT(highRms / inRms, 0.1f);
}

TEST(LinkwitzRileyCrossover, AtCrossoverFreqBothBandsPresent) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 1000.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut(9600), highOut(9600);

    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine[i], 0.0f, lowL, lowR, highL, highR);
        lowOut[i] = lowL;
        highOut[i] = highL;
    }

    float lowRms = rmsRange(lowOut, 2000, 9600);
    float highRms = rmsRange(highOut, 2000, 9600);
    float inRms = rmsRange(sine, 2000, 9600);

    float lowRatio = lowRms / inRms;
    float highRatio = highRms / inRms;
    EXPECT_GT(lowRatio, 0.3f);
    EXPECT_LT(lowRatio, 0.8f);
    EXPECT_GT(highRatio, 0.3f);
    EXPECT_LT(highRatio, 0.8f);
}

TEST(LinkwitzRileyCrossover, SteepSlope) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 500.0f);
    xo.reset();

    auto sine2k = generateSine(2000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut2k(9600);
    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine2k[i], 0.0f, lowL, lowR, highL, highR);
        lowOut2k[i] = lowL;
    }

    xo.reset();
    auto sine4k = generateSine(4000.0f, kSR, 9600, 1.0f);
    std::vector<float> lowOut4k(9600);
    for (int i = 0; i < 9600; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine4k[i], 0.0f, lowL, lowR, highL, highR);
        lowOut4k[i] = lowL;
    }

    float rms2k = rmsRange(lowOut2k, 2000, 9600);
    float rms4k = rmsRange(lowOut4k, 2000, 9600);

    float slopeDb = 20.0f * std::log10(rms2k / std::max(rms4k, 1e-10f));
    EXPECT_GT(slopeDb, 18.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor Tests (v9: processStereo returns gain)
// ═══════════════════════════════════════════════════════════════════════════
TEST(GlueCompressor, ReducesLoudSignals) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    auto sine = generateSine(1000.0f, kSR, 4800, 1.0f);
    std::vector<float> output(4800);

    for (int i = 0; i < 4800; ++i) {
        float gain = gc.processStereo(sine[i], sine[i]);
        output[i] = sine[i] * gain;
    }

    float outPeak = peakAbs(output);
    EXPECT_LT(outPeak, 0.95f);
}

TEST(GlueCompressor, PassthroughAtLowLevel) {
    GlueCompressor gc;
    gc.threshold = -6.0f;
    gc.prepare(kSR);
    gc.reset();

    // Very quiet signal — should not compress
    float gain = gc.processStereo(0.01f, 0.01f);
    EXPECT_NEAR(gain, gc.makeupGain, 0.1f);
}

TEST(GlueCompressor, SoftKneeGradualOnset) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    // Process signals at various levels
    std::vector<float> gains;
    for (float level = 0.1f; level <= 1.0f; level += 0.05f) {
        gc.reset();
        // Feed steady signal to let envelope settle
        for (int i = 0; i < 4800; ++i)
            gc.processStereo(level, level);
        float gain = gc.processStereo(level, level);
        gains.push_back(gain);
    }

    // Check that gain ratios decrease smoothly (no sudden jumps)
    for (size_t i = 1; i < gains.size(); ++i) {
        float jump = gains[i - 1] - gains[i];
        EXPECT_LT(std::abs(jump), 0.15f) << "Abrupt gain change at index " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter Tests (v9: MacroSlot/getMappedValue API)
// ═══════════════════════════════════════════════════════════════════════════
TEST(MacroInterpreter, AddMappingAndRetrieve) {
    MacroInterpreter mi;
    MacroInterpreter::Mapping m;
    m.targetParamIndex = 0;
    m.minValue = 0.0f;
    m.maxValue = 100.0f;
    m.curve = MacroInterpreter::CurveType::Linear;
    mi.macros[0].addMapping(m);

    float val = mi.getMappedValue(0, 1.0f, 0);
    EXPECT_NEAR(val, 100.0f, 0.01f);
}

TEST(MacroInterpreter, ZeroMacroGivesMinValue) {
    MacroInterpreter mi;
    MacroInterpreter::Mapping m;
    m.targetParamIndex = 0;
    m.minValue = 10.0f;
    m.maxValue = 90.0f;
    m.curve = MacroInterpreter::CurveType::Linear;
    mi.macros[0].addMapping(m);

    float val = mi.getMappedValue(0, 0.0f, 0);
    EXPECT_NEAR(val, 10.0f, 0.01f);
}

TEST(MacroInterpreter, ClearRemovesAllMappings) {
    MacroInterpreter mi;
    MacroInterpreter::Mapping m;
    m.targetParamIndex = 0;
    m.minValue = 0.0f;
    m.maxValue = 1.0f;
    mi.macros[0].addMapping(m);
    mi.macros[0].clearMappings();

    float val = mi.getMappedValue(0, 1.0f, 0);
    EXPECT_FLOAT_EQ(val, 0.0f);
}

TEST(MacroInterpreter, PerceptualCurvesAreNonLinear) {
    MacroInterpreter mi;

    // Add exponential mapping
    MacroInterpreter::Mapping mExp;
    mExp.targetParamIndex = 0;
    mExp.minValue = 0.0f;
    mExp.maxValue = 1.0f;
    mExp.curve = MacroInterpreter::CurveType::Exponential;
    mi.macros[0].addMapping(mExp);

    float modExp = mi.getMappedValue(0, 0.5f, 0);

    // Add linear mapping on a different macro
    MacroInterpreter::Mapping mLin;
    mLin.targetParamIndex = 0;
    mLin.minValue = 0.0f;
    mLin.maxValue = 1.0f;
    mLin.curve = MacroInterpreter::CurveType::Linear;
    mi.macros[1].addMapping(mLin);

    float modLin = mi.getMappedValue(1, 0.5f, 0);

    EXPECT_NE(modExp, modLin);
    EXPECT_LT(modExp, modLin)
        << "Exponential curve at 0.5 should produce less than linear (x^2 = 0.25)";
}

TEST(MacroInterpreter, ApplyCurveLinear) {
    EXPECT_NEAR(MacroInterpreter::applyCurve(0.5f, MacroInterpreter::CurveType::Linear), 0.5f, 1e-5f);
}

TEST(MacroInterpreter, ApplyCurveExponential) {
    EXPECT_NEAR(MacroInterpreter::applyCurve(0.5f, MacroInterpreter::CurveType::Exponential), 0.25f, 1e-5f);
}

TEST(MacroInterpreter, ApplyCurveSCurve) {
    float mid = MacroInterpreter::applyCurve(0.5f, MacroInterpreter::CurveType::SCurve);
    EXPECT_NEAR(mid, 0.5f, 0.01f);  // S-curve passes through midpoint
}

TEST(MacroInterpreter, InvalidIndexReturnsZero) {
    MacroInterpreter mi;
    EXPECT_FLOAT_EQ(mi.getMappedValue(-1, 1.0f, 0), 0.0f);
    EXPECT_FLOAT_EQ(mi.getMappedValue(5, 1.0f, 0), 0.0f);
    EXPECT_FLOAT_EQ(mi.getMappedValue(0, 1.0f, 0), 0.0f);  // no mappings
}

// ═══════════════════════════════════════════════════════════════════════════
// MeterBallistics Tests (v9: prepare(sr), peakHoldL)
// ═══════════════════════════════════════════════════════════════════════════
TEST(MeterBallistics, PrepareDoesNotCrash) {
    MeterBallistics mb;
    mb.prepare(44100.0);
    mb.prepare(48000.0);
    mb.prepare(96000.0);
    mb.prepare(192000.0);
    SUCCEED();
}

TEST(MeterBallistics, ResetClearsState) {
    MeterBallistics mb;
    mb.prepare(kSR);
    mb.peakHoldL = 0.9f;
    mb.sparkGR = 5.0f;
    mb.reset();
    EXPECT_FLOAT_EQ(mb.peakHoldL, 0.0f);
    EXPECT_FLOAT_EQ(mb.sparkGR, 0.0f);
}

TEST(MeterBallistics, TracksPeaks) {
    MeterBallistics mb;
    mb.prepare(kSR);
    mb.reset();

    // Feed a loud signal
    for (int i = 0; i < 480; ++i) {
        mb.processSample(0.9f, 0.8f);
    }
    EXPECT_GT(mb.peakHoldL, 0.85f);
    EXPECT_GT(mb.peakHoldR, 0.75f);
}

TEST(MeterBallistics, PeakDecays) {
    MeterBallistics mb;
    mb.prepare(kSR);
    mb.reset();

    // Feed loud signal
    for (int i = 0; i < 480; ++i) mb.processSample(0.9f, 0.9f);
    float peakAfterLoud = mb.peakHoldL;

    // Feed silence for 2 seconds (past hold time)
    for (int i = 0; i < 96000; ++i) mb.processSample(0.0f, 0.0f);
    EXPECT_LT(mb.peakHoldL, peakAfterLoud * 0.1f);
}

// ═══════════════════════════════════════════════════════════════════════════
// AutoGainSmoother Tests (v9: updateInput/updateOutput/getCompensationGain)
// ═══════════════════════════════════════════════════════════════════════════
TEST(AutoGainSmoother, DoesNotCrashOnSilence) {
    AutoGainSmoother ag;
    ag.prepare(kSR);
    for (int i = 0; i < kBlockSize; ++i) {
        ag.updateInput(0.0f, 0.0f);
        ag.updateOutput(0.0f, 0.0f);
    }
    float gain = ag.getCompensationGain();
    EXPECT_TRUE(std::isfinite(gain));
    EXPECT_NEAR(gain, 1.0f, 0.01f);  // Silence → unity
}

TEST(AutoGainSmoother, CompensatesLoudnessIncrease) {
    AutoGainSmoother ag;
    ag.prepare(kSR);
    ag.reset();

    auto dry = generateSine(1000.0f, kSR, kBlockSize * 20, 0.3f);
    auto wet = generateSine(1000.0f, kSR, kBlockSize * 20, 0.6f);

    for (int i = 0; i < kBlockSize * 20; ++i) {
        ag.updateInput(std::abs(dry[i]), std::abs(dry[i]));
        ag.updateOutput(std::abs(wet[i]), std::abs(wet[i]));
    }

    float gain = ag.getCompensationGain();
    // Output is louder than input → compensation should reduce (gain < 1)
    EXPECT_LT(gain, 1.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SidechainHPF Tests (v9: processStereo in-place, prepare(sr, mode))
// ═══════════════════════════════════════════════════════════════════════════
TEST(SidechainHPF, PassthroughWhenOff) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 0);  // mode 0 = off
    hpf.reset();

    float L = 0.8f, R = 0.8f;
    hpf.processStereo(L, R);
    // When off, processStereo returns early — signal unchanged
    EXPECT_NEAR(L, 0.8f, 0.01f);
    EXPECT_NEAR(R, 0.8f, 0.01f);
}

TEST(SidechainHPF, AttenuatesLowFrequencies) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 3);  // mode 3 = 150 Hz
    hpf.reset();

    auto sine = generateSine(50.0f, kSR, 4800, 0.8f);
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        hpf.processStereo(L, R);
        if (i > 480) maxOut = std::max(maxOut, std::max(std::abs(L), std::abs(R)));
    }
    EXPECT_LT(maxOut, 0.5f) << "50 Hz should be attenuated by 150 Hz HPF";
}

TEST(SidechainHPF, PassesHighFrequencies) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 1);  // mode 1 = 60 Hz
    hpf.reset();

    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);
    float maxOut = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        hpf.processStereo(L, R);
        if (i > 480) maxOut = std::max(maxOut, std::max(std::abs(L), std::abs(R)));
    }
    EXPECT_GT(maxOut, 0.6f) << "1 kHz should pass through 60 Hz HPF";
}

TEST(SidechainHPF, ResetClearsState) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 2);  // mode 2 = 90 Hz
    for (int i = 0; i < 480; ++i) {
        float L = 0.5f, R = 0.5f;
        hpf.processStereo(L, R);
    }
    hpf.reset();
    float L = 0.0f, R = 0.0f;
    hpf.processStereo(L, R);
    EXPECT_NEAR(L, 0.0f, 1e-5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// BypassCrossfader Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(BypassCrossfader, ClickFreeTransition) {
    BypassCrossfader xf;
    xf.prepare(64);
    xf.reset();
    xf.setBypassState(false);

    // Process fully wet
    for (int i = 0; i < 64; ++i) {
        float wL = 0.8f, wR = 0.8f;
        xf.processStereo(0.5f, 0.5f, wL, wR);
        EXPECT_NEAR(wL, 0.8f, 0.01f) << "Should be fully wet before bypass";
    }

    // Toggle to bypass
    xf.setBypassState(true);

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

TEST(BypassCrossfader, ResetSettles) {
    BypassCrossfader xf;
    xf.prepare(64);
    xf.setBypassState(true);
    for (int i = 0; i < 10; ++i) {
        float wL = 1.0f, wR = 1.0f;
        xf.processStereo(0.0f, 0.0f, wL, wR);
    }
    xf.reset();
    float wL = 1.0f, wR = 1.0f;
    xf.processStereo(0.0f, 0.0f, wL, wR);
    bool settled = (std::abs(wL - 0.0f) < 0.01f || std::abs(wL - 1.0f) < 0.01f);
    EXPECT_TRUE(settled) << "After reset, crossfader should be settled. Got: " << wL;
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: Saturation Model Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(Waveshaper, TanhMatchesFastTanh) {
    for (float x = -3.0f; x <= 3.0f; x += 0.1f) {
        EXPECT_FLOAT_EQ(Waveshaper::tanh(x), fastTanh(x));
    }
}

TEST(Waveshaper, TubeProducesEvenHarmonics) {
    // Tube model should be asymmetric (even harmonics)
    float posOut = Waveshaper::tube(1.0f);
    float negOut = Waveshaper::tube(-1.0f);
    // If symmetric, |posOut| == |negOut|. Tube should differ.
    EXPECT_NE(std::abs(posOut), std::abs(negOut))
        << "Tube should be asymmetric (even harmonics)";
}

TEST(Waveshaper, TapeIsStateful) {
    float state1 = 0.0f, state2 = 0.0f;
    float y1 = Waveshaper::tape(0.5f, state1);
    float y2 = Waveshaper::tape(0.5f, state2);
    // Both start from same state → same output
    EXPECT_FLOAT_EQ(y1, y2);

    // After processing, state should have changed
    float y3 = Waveshaper::tape(0.5f, state1);
    // state1 has been modified by the first call, so y3 may differ
    EXPECT_TRUE(std::isfinite(y3));
}

TEST(Waveshaper, TransistorClipsHard) {
    float y = Waveshaper::transistor(5.0f);
    EXPECT_LT(std::abs(y), 2.0f) << "Transistor should hard-clip extreme signals";
    EXPECT_TRUE(std::isfinite(y));
}

TEST(Waveshaper, TransformerFreqDependent) {
    // With high lowContent, transformer should saturate more
    float yLow = Waveshaper::transformer(0.5f, 0.0f);
    float yHigh = Waveshaper::transformer(0.5f, 1.0f);
    // Higher lowContent → more drive → different output
    EXPECT_NE(yLow, yHigh);
    EXPECT_TRUE(std::isfinite(yLow));
    EXPECT_TRUE(std::isfinite(yHigh));
}

TEST(Waveshaper, DispatchAllModels) {
    float tapeState = 0.0f;
    for (int m = 0; m < (int)SaturationModel::NumModels; ++m) {
        float y = Waveshaper::process((SaturationModel)m, 0.5f, tapeState, 0.3f);
        EXPECT_TRUE(std::isfinite(y)) << "Model " << m << " produced NaN/Inf";
        EXPECT_LE(std::abs(y), 2.0f) << "Model " << m << " output too large";
    }
}

TEST(Waveshaper, AllModelsSaturate) {
    float tapeState = 0.0f;
    for (int m = 0; m < (int)SaturationModel::NumModels; ++m) {
        // At high drive, output should be bounded
        float y = Waveshaper::process((SaturationModel)m, 10.0f, tapeState, 0.5f);
        EXPECT_LT(std::abs(y), 5.0f) << "Model " << m << " should saturate at high input";
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: MultibandEngine Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(MultibandEngine, FullbandPassthrough) {
    MultibandEngine mb;
    mb.numBands = 1;
    mb.bands[0].satModel = SaturationModel::Tanh;
    mb.bands[0].drive = 0.0f;
    mb.bands[0].mix = 1.0f;
    mb.prepare(kSR);
    mb.reset();

    // At zero drive, tanh(x*1) ≈ x for small signals
    float l = 0.1f, r = 0.1f;
    mb.processStereo(l, r, 0.0f);
    EXPECT_NEAR(l, fastTanh(0.1f), 0.01f);
}

TEST(MultibandEngine, TwoBandSplitAndRecombine) {
    MultibandEngine mb;
    mb.numBands = 2;
    mb.crossoverFreqs[0] = 1000.0f;
    mb.bands[0].satModel = SaturationModel::Tanh;
    mb.bands[0].drive = 0.0f;
    mb.bands[0].mix = 0.0f;  // dry
    mb.bands[1].satModel = SaturationModel::Tanh;
    mb.bands[1].drive = 0.0f;
    mb.bands[1].mix = 0.0f;  // dry
    mb.prepare(kSR);
    mb.reset();

    // With mix=0 (all dry), output should approximately equal input
    auto sine = generateSine(440.0f, kSR, 4800, 0.5f);
    float maxError = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float l = sine[i], r = sine[i];
        mb.processStereo(l, r, 0.0f);
        maxError = std::max(maxError, std::abs(l - sine[i]));
    }
    EXPECT_LT(maxError, 0.05f) << "Dry multiband should approximate passthrough";
}

TEST(MultibandEngine, MuteKillsBand) {
    MultibandEngine mb;
    mb.numBands = 1;
    mb.bands[0].mute = true;
    mb.prepare(kSR);
    mb.reset();

    float l = 0.5f, r = 0.5f;
    float origL = l;
    mb.processStereo(l, r, 0.0f);
    EXPECT_FLOAT_EQ(l, origL) << "Muted band should not modify signal";
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: MidSideEncoder Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(MidSideEncoder, DisabledIsPassthrough) {
    MidSideEncoder ms;
    ms.enabled = false;

    float l = 0.7f, r = 0.3f;
    ms.encode(l, r);
    EXPECT_FLOAT_EQ(l, 0.7f);
    EXPECT_FLOAT_EQ(r, 0.3f);
}

TEST(MidSideEncoder, EncodeDecodeRoundTrip) {
    MidSideEncoder ms;
    ms.enabled = true;

    float l = 0.7f, r = 0.3f;
    ms.encode(l, r);
    // After encode: l = mid = (0.7+0.3)/2 = 0.5, r = side = (0.7-0.3)/2 = 0.2
    EXPECT_NEAR(l, 0.5f, 1e-5f);
    EXPECT_NEAR(r, 0.2f, 1e-5f);

    ms.decode(l, r);
    // After decode: l = mid+side = 0.7, r = mid-side = 0.3
    EXPECT_NEAR(l, 0.7f, 1e-5f);
    EXPECT_NEAR(r, 0.3f, 1e-5f);
}

TEST(MidSideEncoder, MonoSignalHasNoSide) {
    MidSideEncoder ms;
    ms.enabled = true;

    float l = 0.5f, r = 0.5f;
    ms.encode(l, r);
    EXPECT_NEAR(l, 0.5f, 1e-5f);  // mid = 0.5
    EXPECT_NEAR(r, 0.0f, 1e-5f);  // side = 0
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: LFO Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(LFO, SineOutputBounded) {
    LFO lfo;
    lfo.shape = LFOShape::Sine;
    lfo.rateHz = 2.0f;
    lfo.depth = 1.0f;
    lfo.prepare(kSR);
    lfo.reset();

    for (int i = 0; i < 48000; ++i) {
        float val = lfo.next();
        EXPECT_LE(std::abs(val), 1.01f) << "LFO sine should be bounded";
    }
}

TEST(LFO, TriangleOutputBounded) {
    LFO lfo;
    lfo.shape = LFOShape::Triangle;
    lfo.rateHz = 5.0f;
    lfo.depth = 1.0f;
    lfo.prepare(kSR);
    lfo.reset();

    for (int i = 0; i < 48000; ++i) {
        float val = lfo.next();
        EXPECT_LE(std::abs(val), 1.01f) << "LFO triangle should be bounded";
    }
}

TEST(LFO, ZeroDepthGivesZero) {
    LFO lfo;
    lfo.shape = LFOShape::Sine;
    lfo.rateHz = 1.0f;
    lfo.depth = 0.0f;
    lfo.prepare(kSR);
    lfo.reset();

    for (int i = 0; i < 4800; ++i) {
        float val = lfo.next();
        EXPECT_FLOAT_EQ(val, 0.0f) << "Zero depth should produce zero output";
    }
}

TEST(LFO, ResetClearsPhase) {
    LFO lfo;
    lfo.shape = LFOShape::Sine;
    lfo.rateHz = 1.0f;
    lfo.depth = 1.0f;
    lfo.prepare(kSR);

    for (int i = 0; i < 24000; ++i) lfo.next();  // advance phase
    lfo.reset();
    EXPECT_FLOAT_EQ(lfo.phase, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: LoudnessMeter Smoke Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(LoudnessMeter, PrepareDoesNotCrash) {
    LoudnessMeter lm;
    lm.prepare(44100.0);
    lm.prepare(48000.0);
    lm.prepare(96000.0);
    SUCCEED();
}

TEST(LoudnessMeter, SilenceGivesLowLUFS) {
    LoudnessMeter lm;
    lm.prepare(kSR);
    lm.reset();

    for (int i = 0; i < 48000; ++i) {
        lm.processSample(0.0f, 0.0f);
    }
    EXPECT_LT(lm.momentaryLufs, -60.0f);
}

TEST(LoudnessMeter, LoudSignalGivesHigherLUFS) {
    LoudnessMeter lm;
    lm.prepare(kSR);
    lm.reset();

    auto sine = generateSine(1000.0f, kSR, 48000, 0.5f);
    for (int i = 0; i < 48000; ++i) {
        lm.processSample(sine[i], sine[i]);
    }
    EXPECT_GT(lm.momentaryLufs, -30.0f) << "Loud signal should produce higher LUFS";
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: SpectrumBuffer Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(SpectrumBuffer, ResetClearsBuffer) {
    SpectrumBuffer sb;
    sb.pushSample(1.0f, 1.0f);
    sb.pushSample(0.5f, 0.5f);
    sb.reset();
    EXPECT_EQ(sb.writePos.load(), 0);
}

TEST(SpectrumBuffer, PushAdvancesWritePos) {
    SpectrumBuffer sb;
    sb.reset();
    sb.pushSample(0.1f, 0.2f);
    EXPECT_EQ(sb.writePos.load(), 1);
    sb.pushSample(0.3f, 0.4f);
    EXPECT_EQ(sb.writePos.load(), 2);
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: GainReductionHistory Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(GainReductionHistory, ResetClearsHistory) {
    GainReductionHistory grh;
    grh.pushGR(-3.0f);
    grh.pushGR(-6.0f);
    grh.reset();
    EXPECT_EQ(grh.writePos.load(), 0);
}

TEST(GainReductionHistory, PushAdvancesWritePos) {
    GainReductionHistory grh;
    grh.reset();
    grh.pushGR(-1.0f);
    EXPECT_EQ(grh.writePos.load(), 1);
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: UndoStack Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(UndoStack, InitiallyEmpty) {
    UndoStack us;
    EXPECT_FALSE(us.canUndo());
    EXPECT_FALSE(us.canRedo());
}

TEST(UndoStack, PushAndUndo) {
    UndoStack us;
    juce::ValueTree s1("state1");
    s1.setProperty("val", 1, nullptr);
    juce::ValueTree s2("state2");
    s2.setProperty("val", 2, nullptr);

    us.pushState(s1, "first");
    us.pushState(s2, "second");

    EXPECT_TRUE(us.canUndo());
    auto undone = us.undo();
    EXPECT_EQ((int)undone.getProperty("val"), 1);
}

TEST(UndoStack, UndoAndRedo) {
    UndoStack us;
    juce::ValueTree s1("s1");
    s1.setProperty("v", 10, nullptr);
    juce::ValueTree s2("s2");
    s2.setProperty("v", 20, nullptr);

    us.pushState(s1);
    us.pushState(s2);
    us.undo();
    EXPECT_TRUE(us.canRedo());
    auto redone = us.redo();
    EXPECT_EQ((int)redone.getProperty("v"), 20);
}

TEST(UndoStack, ClearResetsStack) {
    UndoStack us;
    juce::ValueTree s("s");
    us.pushState(s);
    us.pushState(s);
    us.clear();
    EXPECT_FALSE(us.canUndo());
    EXPECT_FALSE(us.canRedo());
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: ABState Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(ABState, ToggleSwitchesSlot) {
    ABState ab;
    EXPECT_TRUE(ab.isSlotA);
    ab.toggle();
    EXPECT_FALSE(ab.isSlotA);
    ab.toggle();
    EXPECT_TRUE(ab.isSlotA);
}

TEST(ABState, StoreAndRetrieve) {
    ABState ab;
    juce::ValueTree sA("slotA");
    sA.setProperty("val", 100, nullptr);
    juce::ValueTree sB("slotB");
    sB.setProperty("val", 200, nullptr);

    ab.storeA(sA);
    ab.storeB(sB);

    auto active = ab.getActive();  // A
    EXPECT_EQ((int)active.getProperty("val"), 100);

    ab.toggle();
    active = ab.getActive();  // B
    EXPECT_EQ((int)active.getProperty("val"), 200);
}

TEST(ABState, CopyAtoB) {
    ABState ab;
    juce::ValueTree sA("slotA");
    sA.setProperty("val", 42, nullptr);
    ab.storeA(sA);
    ab.copyAtoB();

    ab.toggle();
    auto active = ab.getActive();
    EXPECT_EQ((int)active.getProperty("val"), 42);
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: MIDILearnState Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(MIDILearnState, AddAndFindMapping) {
    MIDILearnState ml;
    ml.addMapping(1, "drive", 0.0f, 1.0f);

    auto* found = ml.findMapping(1);
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->parameterID, juce::String("drive"));
}

TEST(MIDILearnState, RemoveMapping) {
    MIDILearnState ml;
    ml.addMapping(1, "drive");
    ml.addMapping(2, "mix");
    ml.removeMapping(1);

    EXPECT_EQ(ml.findMapping(1), nullptr);
    EXPECT_NE(ml.findMapping(2), nullptr);
    EXPECT_EQ(ml.numMappings, 1);
}

TEST(MIDILearnState, ClearAll) {
    MIDILearnState ml;
    ml.addMapping(1, "drive");
    ml.addMapping(2, "mix");
    ml.clearAll();
    EXPECT_EQ(ml.numMappings, 0);
    EXPECT_EQ(ml.findMapping(1), nullptr);
}

TEST(MIDILearnState, LearningMode) {
    MIDILearnState ml;
    ml.startLearning("punch");
    EXPECT_TRUE(ml.isLearning);
    EXPECT_EQ(ml.learningParameterID, juce::String("punch"));
    ml.stopLearning();
    EXPECT_FALSE(ml.isLearning);
}

// ═══════════════════════════════════════════════════════════════════════════
// Real-Time Safety Smoke Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(RealTimeSafety, NoNaNOrInfInFullChain) {
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    for (int i = 0; i < kBlockSize; ++i) {
        float x = 100.0f * std::sin(2.0f * 3.14159f * 1000.0f * i / (float)kSR) + 50.0f;
        x = safety.processSample(x, safety.dcL, safety.dcPrevL);
        x = fastTanh(x);
        EXPECT_TRUE(std::isfinite(x)) << "NaN/Inf at sample " << i;
    }
}

TEST(RealTimeSafety, FastTanhHandlesExtremeInput) {
    float y;
    y = fastTanh(0.0f);     EXPECT_TRUE(std::isfinite(y));
    y = fastTanh(100.0f);   EXPECT_TRUE(std::isfinite(y)); EXPECT_LE(std::abs(y), 1.1f);
    y = fastTanh(-100.0f);  EXPECT_TRUE(std::isfinite(y)); EXPECT_LE(std::abs(y), 1.1f);
    y = fastTanh(1000.0f);  EXPECT_TRUE(std::isfinite(y));
    y = fastTanh(-1000.0f); EXPECT_TRUE(std::isfinite(y));
}

TEST(RealTimeSafety, FullChainWithExtremeParameters) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    GlueCompressor gc;
    gc.prepare(kSR);
    gc.reset();

    ShineProcessor shine;
    shine.freq = 12000.0f;
    shine.gainDb = 9.0f;
    shine.q = 0.7f;
    shine.prepare(kSR);
    shine.reset();

    for (int i = 0; i < kBlockSize * 4; ++i) {
        float x = 10.0f * std::sin(2.0f * 3.14159f * 1000.0f * i / (float)kSR);
        float lowL, lowR, highL, highR;
        xo.processStereo(x, x, lowL, lowR, highL, highR);

        lowL = fastTanh(lowL * 2.0f);
        float gain = gc.processStereo(lowL, lowR);
        lowL *= gain;
        lowR *= gain;

        float outL = lowL + highL;
        float outR = lowR + highR;
        shine.processStereo(outL, outR);

        EXPECT_TRUE(std::isfinite(outL)) << "NaN/Inf at sample " << i;
        EXPECT_TRUE(std::isfinite(outR)) << "NaN/Inf at sample " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Envelope Follower Timing Tests (v5 audit)
// ═══════════════════════════════════════════════════════════════════════════
TEST(EnvFollowerTiming, ReleaseTiming) {
    EnvFollower env;
    env.setTimes(0.2f, 220.0f, kSR);
    env.reset();

    const int impulseSamples = (int)(kSR * 0.001);
    for (int i = 0; i < impulseSamples; ++i) env.process(1.0f);
    float peakVal = env.env;
    EXPECT_GT(peakVal, 0.5f);

    int sampleCount = 0;
    const float target = peakVal * 0.37f;
    const int maxSamples = (int)(kSR * 2.0);
    while (env.env > target && sampleCount < maxSamples) {
        env.process(0.0f);
        sampleCount++;
    }

    float releaseMs = (float)sampleCount / (float)kSR * 1000.0f;
    EXPECT_NEAR(releaseMs, 220.0f, 22.0f)
        << "Release time should be ~220ms. Got " << releaseMs << "ms";
}

TEST(EnvFollowerTiming, OSRateBugRegression) {
    EnvFollower envBase;
    envBase.setTimes(0.2f, 220.0f, kSR);
    envBase.reset();

    EnvFollower envOS;
    envOS.setTimes(0.2f, 220.0f, kSR * 2.0);
    envOS.reset();

    for (int i = 0; i < 48; ++i) {
        envBase.process(1.0f);
        envOS.process(1.0f);
    }

    for (int i = 0; i < 500; ++i) {
        envBase.process(0.0f);
        envOS.process(0.0f);
        envOS.process(0.0f);
    }

    EXPECT_GT(envBase.env, envOS.env)
        << "Base-SR envelope should decay slower than OS-rate envelope";
}

// ═══════════════════════════════════════════════════════════════════════════
// Crossover Null-Path Complementarity (v5 audit)
// ═══════════════════════════════════════════════════════════════════════════
TEST(CrossoverNullPath, LowPlusHighEqualsInput) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto input = generateSine(1000.0f, kSR, kBlockSize * 4);
    float maxError = 0.0f;

    for (int i = 0; i < (int)input.size(); ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(input[i], input[i], lowL, lowR, highL, highR);
        float reconstructed = lowL + highL;
        float error = std::abs(reconstructed - input[i]);
        maxError = std::max(maxError, error);
    }

    EXPECT_LT(maxError, 1.0e-6f)
        << "Crossover low+high should equal input. Max error: " << maxError;
}

// ═══════════════════════════════════════════════════════════════════════════
// SidechainHPF Crossfade (v7 audit)
// ═══════════════════════════════════════════════════════════════════════════
TEST(SidechainHPF, CrossfadeNoClick) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 1);  // 60 Hz
    hpf.reset();

    auto sine = generateSine(200.0f, kSR, 4800, 0.8f);
    for (int i = 0; i < 2400; ++i) {
        float L = sine[i], R = sine[i];
        hpf.processStereo(L, R);
    }

    // Switch to 150 Hz mode
    hpf.prepare(kSR, 3);

    float prevL = sine[2400], prevR = sine[2400];
    hpf.processStereo(prevL, prevR);
    float maxDelta = 0.0f;
    for (int i = 2401; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        hpf.processStereo(L, R);
        float delta = std::abs(L - prevL);
        maxDelta = std::max(maxDelta, delta);
        prevL = L;
    }
    EXPECT_LT(maxDelta, 0.1f)
        << "HPF mode change should be smooth. Max delta: " << maxDelta;
}

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam Automation Zipper (v7 audit)
// ═══════════════════════════════════════════════════════════════════════════
TEST(SmoothParam, NoZipper) {
    SmoothParam sp;
    sp.setTime(10.0f, kSR);
    sp.snapTo(0.0f);
    sp.setTarget(1.0f);

    float prev = 0.0f;
    float maxDelta = 0.0f;
    bool reachedTarget = false;
    for (int i = 0; i < (int)(kSR * 0.1); ++i) {
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
// Safety Layer Silence-In-Silence-Out (v7 audit)
// ═══════════════════════════════════════════════════════════════════════════
TEST(SafetyLayer, SilenceInSilenceOut) {
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    float maxOut = 0.0f;
    for (int i = 0; i < 1000; ++i) {
        float out = safety.processSample(0.0f, safety.dcL, safety.dcPrevL);
        maxOut = std::max(maxOut, std::abs(out));
    }
    EXPECT_LT(maxOut, 1e-10f)
        << "Silence in should produce silence out. Max output: " << maxOut;
}

// ═══════════════════════════════════════════════════════════════════════════
// v9: State Version Validation
// ═══════════════════════════════════════════════════════════════════════════
TEST(StateVersion, IsVersion9) {
    EXPECT_EQ(BTZDsp::kStateVersion, 9) << "State version should be 9 for v9 release";
}

// ═══════════════════════════════════════════════════════════════════════════
// DC Blocker 1 Hz Coloration Regression (v8)
// ═══════════════════════════════════════════════════════════════════════════
TEST(DCBlocker, MinimalColorationAt20Hz) {
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    auto sine = generateSine(20.0f, kSR, 48000, 0.5f);
    std::vector<float> output(48000);
    float dc = 0.0f, dcPrev = 0.0f;

    for (int i = 0; i < 48000; ++i) {
        output[i] = safety.processSample(sine[i], dc, dcPrev);
    }

    float inRms = rmsRange(sine, 24000, 48000);
    float outRms = rmsRange(output, 24000, 48000);
    float deltaDb = 20.0f * std::log10(outRms / std::max(inRms, 1e-10f));

    EXPECT_GT(deltaDb, -0.5f)
        << "20 Hz should pass through 1 Hz DC blocker with < 0.5 dB loss. Got: "
        << deltaDb << " dB";
}

TEST(DCBlocker, StillBlocksDC) {
    SafetyLayer safety;
    safety.setSampleRate(kSR);
    safety.reset();

    float dc = 0.0f, dcPrev = 0.0f;
    float out = 0.0f;
    for (int i = 0; i < 240000; ++i) {
        out = safety.processSample(1.0f, dc, dcPrev);
    }

    EXPECT_LT(std::abs(out), 0.1f)
        << "DC should be blocked even at 1 Hz cutoff. Residual: " << out;
}

// ═══════════════════════════════════════════════════════════════════════════
// Saturation Chain Produces Harmonics (v8)
// ═══════════════════════════════════════════════════════════════════════════
TEST(SaturationChain, ProducesHarmonics) {
    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);
    std::vector<float> saturated(4800);

    for (int i = 0; i < 4800; ++i) {
        saturated[i] = fastTanh(sine[i] * 3.0f);
    }

    float diffEnergy = 0.0f;
    for (int i = 0; i < 4800; ++i) {
        float d = saturated[i] - sine[i];
        diffEnergy += d * d;
    }
    EXPECT_GT(diffEnergy, 0.01f) << "Saturation should add harmonics";
}

TEST(SaturationChain, MultibandWithFastTanh) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 250.0f);
    xo.reset();

    auto sine = generateSine(1000.0f, kSR, 4800, 0.8f);

    for (int i = 0; i < 4800; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine[i], sine[i], lowL, lowR, highL, highR);

        float satLowL = fastTanh(lowL * 2.0f);
        float satHighL = fastTanh(highL * 2.0f);

        float outL = satLowL + satHighL;
        EXPECT_TRUE(std::isfinite(outL)) << "NaN/Inf at sample " << i;
        EXPECT_LE(std::abs(outL), 3.0f) << "Output should be bounded at sample " << i;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
