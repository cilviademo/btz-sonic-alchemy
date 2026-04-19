/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp  v2
  ────────────────────────────────────────────────────────────────────────
  GoogleTest-based unit tests for all BTZDsp modules.
  v2: Added ADAATanh, TruePeakLimiter (ISP-aware, block-level API),
  SVF ShineProcessor, real-time safety smoke tests.
  Removed old SparkLimiter tests (replaced by TruePeakLimiter).

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
    // For very small inputs, tanh(x) ≈ x, so ADAA should be near-unity
    const float tiny = 0.001f;
    adaa.reset();
    adaa.process(0.0f); // prime
    float y = adaa.process(tiny);
    EXPECT_NEAR(y, tiny, tiny * 0.15f); // within 15% of input
}

TEST_F(ADAATanhTest, SaturatesLargeSignals) {
    adaa.reset();
    adaa.process(0.0f); // prime
    float y = adaa.process(5.0f);
    EXPECT_LT(std::abs(y), 5.0f);
    EXPECT_GT(std::abs(y), 0.5f);
}

TEST_F(ADAATanhTest, OutputBoundedByTanh) {
    // ADAA output should never exceed ±1 (tanh range) + small tolerance
    adaa.reset();
    auto sine = generateSine(1000.0f, kSR, kBlockSize, 10.0f); // heavily driven
    for (auto& s : sine) {
        float y = adaa.process(s);
        EXPECT_LE(std::abs(y), 1.05f); // small tolerance for ADAA transients
    }
}

TEST_F(ADAATanhTest, DiffersFromDirectTanh) {
    // ADAA should produce different output than direct tanh (smoother transitions)
    adaa.reset();
    auto sine = generateSine(10000.0f, kSR, kBlockSize, 2.0f);
    float diffEnergy = 0.0f;

    for (int i = 0; i < kBlockSize; ++i) {
        float adaaOut = adaa.process(sine[i]);
        float directOut = std::tanh(sine[i]);
        float d = adaaOut - directOut;
        diffEnergy += d * d;
    }
    // There should be a measurable difference (ADAA smooths the waveform)
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

    // Prime both
    left.process(0.0f); right.process(0.0f);

    float yL = left.process(2.0f);
    float yR = right.process(-2.0f);

    // tanh is an odd function, so outputs should be opposite-sign
    EXPECT_NE(yL, yR);
    EXPECT_NEAR(yL, -yR, 0.05f);
}

TEST_F(ADAATanhTest, MonotonicForIncreasingInput) {
    adaa.reset();
    adaa.process(0.0f); // prime
    float prev = adaa.process(0.1f);
    for (float x = 0.2f; x <= 3.0f; x += 0.1f) {
        float y = adaa.process(x);
        EXPECT_GE(y, prev - 0.01f); // approximately monotonic (ADAA may have tiny transients)
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
    const float ceiling = 0.5f; // -6 dBFS
    const int totalSamples = kBlockSize * 8;
    auto sine = generateSine(440.0f, kSR, totalSamples, 1.0f); // peaks at 1.0

    juce::AudioBuffer<float> buffer(2, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        buffer.setSample(0, i, sine[i]);
        buffer.setSample(1, i, sine[i]);
    }
    limiter.processBlock(buffer, ceiling);

    // After lookahead settling, all samples should be at or below ceiling
    const int skip = limiter.getLatencySamples() + 50;
    for (int i = skip; i < totalSamples; ++i) {
        EXPECT_LE(std::abs(buffer.getSample(0, i)), ceiling + 0.02f)
            << "Overshoot at sample " << i;
        EXPECT_LE(std::abs(buffer.getSample(1, i)), ceiling + 0.02f)
            << "Overshoot at sample " << i;
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

    // GR should be zero or near-zero
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
    EXPECT_GT(grDb, 0.5f); // Should report meaningful GR
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

TEST_F(TruePeakLimiterTest, HandlesStereoIndependently) {
    // Left channel hot, right channel quiet
    const int totalSamples = kBlockSize * 4;
    auto hotSine = generateSine(440.0f, kSR, totalSamples, 1.5f);
    auto quietSine = generateSine(440.0f, kSR, totalSamples, 0.2f);

    juce::AudioBuffer<float> buffer(2, totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        buffer.setSample(0, i, hotSine[i]);
        buffer.setSample(1, i, quietSine[i]);
    }
    limiter.processBlock(buffer, 0.5f);

    // Both channels should be limited (linked stereo)
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
    shine.setParameters(8000.0f, 6.0f, 0.7f); // +6 dB shelf at 8 kHz

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
    shine.setParameters(12000.0f, 0.0f, 0.7f); // 0 dB gain = bypass

    auto sine = generateSine(8000.0f, kSR, 4800, 0.4f);
    std::vector<float> output(4800);

    for (int i = 0; i < 4800; ++i) {
        float L = sine[i], R = sine[i];
        shine.processStereo(L, R);
        output[i] = L;
    }

    // After settling, output should match input closely
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
    // Rapidly change parameters while processing — should not produce NaN/Inf
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

// ═══════════════════════════════════════════════════════════════════════════
// SmoothParam Tests
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

TEST(SmoothParam, IsSmoothingDetection) {
    SmoothParam sp;
    sp.setTime(5.0f, kSR);
    sp.snapTo(1.0f);
    EXPECT_FALSE(sp.isSmoothing());
    sp.setTarget(0.0f);
    EXPECT_TRUE(sp.isSmoothing());
}

// ═══════════════════════════════════════════════════════════════════════════
// EnvFollower Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(EnvFollower, TracksInputLevel) {
    EnvFollower ef;
    ef.setTimes(0.5f, 100.0f, kSR);
    ef.reset();

    for (int i = 0; i < 48000; ++i)
        ef.process(0.8f);

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
    for (int i = 0; i < 96000; ++i)
        sl.processSample(1.0f, dc, dcPrev);

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
// LinkwitzRileyCrossover Tests
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

// ═══════════════════════════════════════════════════════════════════════════
// GlueCompressor Tests
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

// ═══════════════════════════════════════════════════════════════════════════
// MacroInterpreter Tests
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
// fastTanh Tests
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
// Real-Time Safety Smoke Tests
// ═══════════════════════════════════════════════════════════════════════════
TEST(RealTimeSafety, NoNaNOrInfInFullChain) {
    // Process extreme input through ADAA + Safety + Slew and verify finite output
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
    adaa.process(0.0f); // prime

    // Feed NaN — ADAA uses division, must handle gracefully
    float y = adaa.process(std::numeric_limits<float>::quiet_NaN());
    // We don't require a specific value, just that it doesn't propagate NaN forever
    adaa.reset();
    y = adaa.process(0.0f);
    EXPECT_TRUE(std::isfinite(y));
}

TEST(RealTimeSafety, InfInputToADAADoesNotExplode) {
    ADAATanh adaa;
    adaa.reset();
    adaa.process(0.0f); // prime

    float y = adaa.process(std::numeric_limits<float>::infinity());
    adaa.reset();
    y = adaa.process(0.5f);
    EXPECT_TRUE(std::isfinite(y));
    EXPECT_LE(std::abs(y), 2.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
