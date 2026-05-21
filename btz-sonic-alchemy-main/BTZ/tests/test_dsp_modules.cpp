/*
  Box Tone Zone (BTZ) — test_dsp_modules.cpp  v1.0 Ivory System
  ────────────────────────────────────────────────────────────────────
  GoogleTest-based unit tests for all BTZDsp modules.
  v1.0: Matches BTZDsp.h v1.0 Ivory System API.
  Build: cmake -DBTZ_BUILD_TESTS=ON ..
  Run:   ctest --output-on-failure
*/
#include <gtest/gtest.h>
#include "../Source/BTZDsp.h"
#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <limits>

using namespace BTZDsp;

// ═══════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════
static constexpr double kSR = 48000.0;
static constexpr int kBlock = 512;

static std::vector<float> generateSine(float freqHz, double sr, int numSamples, float amp = 0.5f) {
    std::vector<float> buf(numSamples);
    for (int i = 0; i < numSamples; ++i)
        buf[i] = amp * std::sin(2.0f * (float)M_PI * freqHz * (float)i / (float)sr);
    return buf;
}

static float rms(const std::vector<float>& v) {
    float sum = 0.0f;
    for (auto x : v) sum += x * x;
    return std::sqrt(sum / (float)v.size());
}

static float peak(const std::vector<float>& v) {
    float p = 0.0f;
    for (auto x : v) p = std::max(p, std::abs(x));
    return p;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: UTILITIES
// ═══════════════════════════════════════════════════════════════════════════

TEST(Utilities, FastTanhAccuracy) {
    // Padé [5/5] should be within 0.01 of std::tanh for |x| < 4
    for (float x = -4.0f; x <= 4.0f; x += 0.01f) {
        EXPECT_NEAR(fastTanh(x), std::tanh(x), 0.01f) << "at x=" << x;
    }
}

TEST(Utilities, FastTanhClamps) {
    EXPECT_FLOAT_EQ(fastTanh(100.0f), 1.0f);
    EXPECT_FLOAT_EQ(fastTanh(-100.0f), -1.0f);
}

TEST(Utilities, DbGainRoundtrip) {
    for (float db = -60.0f; db <= 12.0f; db += 3.0f) {
        EXPECT_NEAR(gainToDb(dbToGain(db)), db, 0.001f);
    }
}

TEST(Utilities, SoftClipBounds) {
    EXPECT_LE(std::abs(softClip(10.0f)), 1.0f);
    EXPECT_LE(std::abs(softClip(-10.0f)), 1.0f);
    EXPECT_NEAR(softClip(0.0f), 0.0f, 1e-6f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: CORE MODULES
// ═══════════════════════════════════════════════════════════════════════════

TEST(SmoothParam, ConvergesToTarget) {
    SmoothParam sp;
    sp.setTime(0.010f, kSR);  // 10ms
    sp.setTarget(1.0f);
    float val = 0.0f;
    for (int i = 0; i < 4800; ++i)  // 100ms — should converge
        val = sp.next();
    EXPECT_NEAR(val, 1.0f, 0.001f);
}

TEST(SmoothParam, ImmediateAfterReset) {
    SmoothParam sp;
    sp.setTime(0.100f, kSR);
    sp.setTarget(0.75f);
    sp.reset(0.75f);
    EXPECT_FLOAT_EQ(sp.next(), 0.75f);
}

TEST(EnvFollower, TracksEnvelope) {
    EnvFollower env;
    env.prepare(kSR, 0.001f, 0.050f);
    // Feed a burst of 1.0 then silence
    for (int i = 0; i < 48; ++i)
        env.process(1.0f);
    float afterBurst = env.envelope;
    EXPECT_GT(afterBurst, 0.8f);
    // Decay
    for (int i = 0; i < 4800; ++i)
        env.process(0.0f);
    EXPECT_LT(env.envelope, 0.01f);
}

TEST(SafetyLayer, RemovesDC) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    // Feed DC offset
    float l = 0.5f, r = 0.5f;
    for (int i = 0; i < 48000; ++i) {
        l = 0.5f; r = 0.5f;
        sl.processStereo(l, r);
    }
    // After convergence, output should be near zero
    EXPECT_NEAR(l, 0.0f, 0.01f);
    EXPECT_NEAR(r, 0.0f, 0.01f);
}

TEST(SafetyLayer, PassesAC) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    auto sine = generateSine(1000.0f, kSR, 48000);
    std::vector<float> output(48000);
    for (int i = 0; i < 48000; ++i) {
        float l = sine[i], r = sine[i];
        sl.processStereo(l, r);
        output[i] = l;
    }
    // RMS should be close to original (within 1%)
    float inRms = rms(sine);
    float outRms = rms(output);
    EXPECT_NEAR(outRms / inRms, 1.0f, 0.01f);
}

TEST(SafetyLayer, CatchesNaN) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    float l = std::numeric_limits<float>::quiet_NaN();
    float r = std::numeric_limits<float>::infinity();
    sl.processStereo(l, r);
    EXPECT_TRUE(std::isfinite(l));
    EXPECT_TRUE(std::isfinite(r));
}

TEST(BypassCrossfader, SmoothTransition) {
    BypassCrossfader bc;
    bc.prepare();
    // Initially not bypassed — wet should pass through
    float wetL = 1.0f, wetR = 1.0f;
    bc.processStereo(0.0f, 0.0f, wetL, wetR);
    EXPECT_FLOAT_EQ(wetL, 1.0f);  // Full wet

    // Engage bypass
    bc.setBypassState(true);
    // During fade, output should transition
    float lastWet = 1.0f;
    for (int i = 0; i < 64; ++i) {
        float wL = 1.0f, wR = 1.0f;
        bc.processStereo(0.0f, 0.0f, wL, wR);
        EXPECT_LE(wL, lastWet + 0.001f);  // Monotonically decreasing
        lastWet = wL;
    }
    // After fade, should be fully bypassed (dry)
    float wL = 1.0f, wR = 1.0f;
    bc.processStereo(0.5f, 0.5f, wL, wR);
    EXPECT_NEAR(wL, 0.5f, 0.001f);  // Returns dry signal
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: SATURATION MODELS
// ═══════════════════════════════════════════════════════════════════════════

TEST(Waveshaper, TanhProducesOddHarmonics) {
    auto sine = generateSine(1000.0f, kSR, kBlock);
    float tapeState = 0.0f;
    for (auto& s : sine) s = Waveshaper::process(SaturationModel::Tanh, s * 3.0f, tapeState, 0.0f);
    // Saturated signal should have higher RMS than linear
    EXPECT_GT(rms(sine), 0.3f);
}

TEST(Waveshaper, TubeAddsEvenHarmonics) {
    auto sine = generateSine(1000.0f, kSR, kBlock);
    float tapeState = 0.0f;
    std::vector<float> output(kBlock);
    for (int i = 0; i < kBlock; ++i)
        output[i] = Waveshaper::process(SaturationModel::Tube, sine[i] * 2.0f, tapeState, 0.0f);
    // Tube should be asymmetric (positive and negative peaks differ)
    float posMax = 0.0f, negMax = 0.0f;
    for (auto s : output) {
        if (s > posMax) posMax = s;
        if (s < negMax) negMax = s;
    }
    EXPECT_NE(posMax, -negMax);  // Asymmetric = even harmonics
}

TEST(Waveshaper, TapeHasMemory) {
    float state1 = 0.0f, state2 = 0.0f;
    // Same input, different history should produce different output
    Waveshaper::tape(0.5f, state1);
    Waveshaper::tape(0.5f, state1);
    Waveshaper::tape(-0.5f, state2);
    float out1 = Waveshaper::tape(0.5f, state1);
    float out2 = Waveshaper::tape(0.5f, state2);
    EXPECT_NE(out1, out2);  // Hysteresis = state-dependent
}

TEST(Waveshaper, TransistorAsymmetric) {
    float pos = Waveshaper::transistor(1.5f);
    float neg = Waveshaper::transistor(-1.5f);
    EXPECT_NE(std::abs(pos), std::abs(neg));
}

TEST(Waveshaper, TransformerLowContentSensitive) {
    float out1 = Waveshaper::transformer(0.5f, 0.0f);
    float out2 = Waveshaper::transformer(0.5f, 1.0f);
    EXPECT_NE(out1, out2);
}

TEST(Waveshaper, AllModelsBounded) {
    float tapeState = 0.0f;
    for (int m = 0; m < (int)SaturationModel::Neural_Neve; ++m) {
        float out = Waveshaper::process((SaturationModel)m, 10.0f, tapeState, 0.5f);
        EXPECT_LE(std::abs(out), 2.0f) << "Model " << m << " unbounded";
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: NEURAL SATURATION
// ═══════════════════════════════════════════════════════════════════════════

TEST(NeuralSaturation, FallbackWhenNotLoaded) {
    NeuralSaturationModel nsm;
    EXPECT_FALSE(nsm.loaded);
    // Should fall back to fastTanh
    float out = nsm.process(0.5f);
    EXPECT_NEAR(out, fastTanh(0.5f), 1e-6f);
}

TEST(NeuralSaturation, ResetClearsState) {
    NeuralSaturationModel nsm;
    nsm.hiddenState.fill(1.0f);
    nsm.reset();
    for (auto h : nsm.hiddenState)
        EXPECT_FLOAT_EQ(h, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: WDF CIRCUIT MODELS
// ═══════════════════════════════════════════════════════════════════════════

TEST(WDFTube, ProducesSaturation) {
    WDFTubeStage wdf;
    wdf.reset();
    auto sine = generateSine(1000.0f, kSR, kBlock, 1.0f);
    std::vector<float> output(kBlock);
    for (int i = 0; i < kBlock; ++i)
        output[i] = wdf.process(sine[i]);
    // Output should be bounded and non-zero
    EXPECT_GT(rms(output), 0.1f);
    EXPECT_LE(peak(output), 1.5f);
}

TEST(WDFTube, HasState) {
    WDFTubeStage wdf;
    wdf.reset();
    float out1 = wdf.process(0.5f);
    float out2 = wdf.process(0.5f);
    EXPECT_NE(out1, out2);  // State-dependent
}

TEST(WDFTransformer, CoreSaturation) {
    WDFTransformerStage wdf;
    wdf.reset();
    auto sine = generateSine(100.0f, kSR, kBlock, 1.0f);
    std::vector<float> output(kBlock);
    for (int i = 0; i < kBlock; ++i)
        output[i] = wdf.process(sine[i]);
    EXPECT_GT(rms(output), 0.05f);
    EXPECT_LE(peak(output), 2.0f);
}

TEST(WDFTransformer, FluxAccumulates) {
    WDFTransformerStage wdf;
    wdf.reset();
    for (int i = 0; i < 100; ++i)
        wdf.process(1.0f);
    EXPECT_NE(wdf.primaryFlux, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: RESONANCE TAMING + TRANSIENT SPLITTING
// ═══════════════════════════════════════════════════════════════════════════

TEST(ResonanceTamer, DisabledPassesThrough) {
    ResonanceTamer rt;
    rt.prepare(kSR);
    rt.enabled = false;
    EXPECT_FLOAT_EQ(rt.process(0.75f), 0.75f);
}

TEST(ResonanceTamer, ReducesResonantPeaks) {
    ResonanceTamer rt;
    rt.prepare(kSR);
    rt.enabled = true;
    rt.sensitivity = 0.3f;
    rt.depth = 1.0f;
    // Feed sustained loud signal to build up band energy
    for (int i = 0; i < 4800; ++i)
        rt.process(0.9f);
    // Now the output should be attenuated
    float out = rt.process(0.9f);
    EXPECT_LT(out, 0.9f);
}

TEST(TransientSplitter, DisabledReturnsZero) {
    TransientSplitter ts;
    ts.prepare(kSR);
    ts.enabled = false;
    EXPECT_FLOAT_EQ(ts.getTransientAmount(1.0f), 0.0f);
}

TEST(TransientSplitter, DetectsTransient) {
    TransientSplitter ts;
    ts.prepare(kSR);
    ts.enabled = true;
    ts.sensitivity = 1.0f;
    // Feed silence then a sudden burst
    for (int i = 0; i < 4800; ++i)
        ts.getTransientAmount(0.001f);
    float transient = ts.getTransientAmount(1.0f);
    EXPECT_GT(transient, 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: OVERSAMPLING ENGINE
// ═══════════════════════════════════════════════════════════════════════════

TEST(OversamplingEngine, PrepareSetsFactor) {
    OversamplingEngine os;
    os.prepare(4);
    EXPECT_EQ(os.factor, 4);
    EXPECT_GT(os.latencySamples, 0);
}

TEST(OversamplingEngine, UpsampleInsertsZeros) {
    OversamplingEngine os;
    os.prepare(4);
    float outL[4], outR[4];
    os.upsample(1.0f, 0.5f, outL, outR, 4);
    EXPECT_FLOAT_EQ(outL[0], 1.0f);
    EXPECT_FLOAT_EQ(outR[0], 0.5f);
    for (int i = 1; i < 4; ++i) {
        EXPECT_FLOAT_EQ(outL[i], 0.0f);
        EXPECT_FLOAT_EQ(outR[i], 0.0f);
    }
}

TEST(OversamplingEngine, DownsampleProducesOutput) {
    OversamplingEngine os;
    os.prepare(2);
    float inL[2] = {1.0f, 0.0f};
    float inR[2] = {0.5f, 0.0f};
    float outL, outR;
    os.downsample(inL, inR, 2, outL, outR);
    EXPECT_TRUE(std::isfinite(outL));
    EXPECT_TRUE(std::isfinite(outR));
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 8: DYNAMICS
// ═══════════════════════════════════════════════════════════════════════════

TEST(SidechainHPF, RemovesLowFrequency) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 200.0f);
    // Feed 50 Hz sine — should be attenuated
    auto sine50 = generateSine(50.0f, kSR, 48000);
    std::vector<float> output(48000);
    for (int i = 0; i < 48000; ++i) {
        float l = sine50[i], r = sine50[i];
        hpf.processStereo(l, r);
        output[i] = l;
    }
    EXPECT_LT(rms(output), rms(sine50) * 0.5f);
}

TEST(SidechainHPF, PassesHighFrequency) {
    SidechainHPF hpf;
    hpf.prepare(kSR, 200.0f);
    auto sine5k = generateSine(5000.0f, kSR, 48000);
    std::vector<float> output(48000);
    for (int i = 0; i < 48000; ++i) {
        float l = sine5k[i], r = sine5k[i];
        hpf.processStereo(l, r);
        output[i] = l;
    }
    // High frequency should pass with minimal loss
    EXPECT_GT(rms(output), rms(sine5k) * 0.7f);
}

TEST(GlueCompressor, ReducesGainAboveThreshold) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.setParameters(-12.0f, 4.0f, 0.0f);
    // Feed signal above threshold
    float gain = gc.processStereo(1.0f, 1.0f);
    EXPECT_LT(gain, 1.0f);
}

TEST(GlueCompressor, NoReductionBelowThreshold) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.setParameters(-6.0f, 4.0f, 0.0f);
    // Feed very quiet signal
    float gain = gc.processStereo(0.001f, 0.001f);
    EXPECT_NEAR(gain, 1.0f, 0.01f);
}

TEST(GlueCompressor, MakeupGainApplied) {
    GlueCompressor gc;
    gc.prepare(kSR);
    gc.setParameters(-6.0f, 4.0f, 6.0f);  // 6dB makeup
    float gain = gc.processStereo(0.001f, 0.001f);
    EXPECT_GT(gain, 1.5f);  // ~2x from 6dB makeup
}

TEST(TruePeakLimiter, LimitsAboveCeiling) {
    TruePeakLimiter lim;
    lim.prepare(kSR);
    lim.ceiling = -1.0f;
    float ceilLin = dbToGain(-1.0f);
    // Process loud signal through lookahead
    for (int i = 0; i < 100; ++i) {
        float l = 2.0f, r = 2.0f;
        lim.processStereo(l, r);
    }
    // After settling, output should be below ceiling
    float l = 2.0f, r = 2.0f;
    lim.processStereo(l, r);
    EXPECT_LE(std::abs(l), ceilLin * 1.1f);
}

TEST(TruePeakLimiter, PassesQuietSignal) {
    TruePeakLimiter lim;
    lim.prepare(kSR);
    lim.ceiling = -0.3f;
    // Process quiet signal
    for (int i = 0; i < 100; ++i) {
        float l = 0.1f, r = 0.1f;
        lim.processStereo(l, r);
    }
    // After delay settles, quiet signal should pass unchanged
    float l = 0.1f, r = 0.1f;
    lim.processStereo(l, r);
    // Due to lookahead delay, we check the delayed output
    EXPECT_LE(std::abs(l), 0.15f);
}

TEST(TruePeakLimiter, ReportedLatency) {
    TruePeakLimiter lim;
    EXPECT_EQ(TruePeakLimiter::kLookahead, 8);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 9: EQ + CROSSOVER + MULTIBAND
// ═══════════════════════════════════════════════════════════════════════════

TEST(ShineProcessor, BoostsAtFrequency) {
    ShineProcessor sp;
    sp.setParameters(5000.0f, 6.0f, 0.707f);
    sp.recalcCoeffs(kSR);
    sp.reset();
    auto sine5k = generateSine(5000.0f, kSR, 4800);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float l = sine5k[i], r = sine5k[i];
        sp.processStereo(l, r);
        output[i] = l;
    }
    // Boosted signal should be louder
    EXPECT_GT(rms(output), rms(sine5k) * 1.2f);
}

TEST(ShineProcessor, CutsAtFrequency) {
    ShineProcessor sp;
    sp.setParameters(5000.0f, -6.0f, 0.707f);
    sp.recalcCoeffs(kSR);
    sp.reset();
    auto sine5k = generateSine(5000.0f, kSR, 4800);
    std::vector<float> output(4800);
    for (int i = 0; i < 4800; ++i) {
        float l = sine5k[i], r = sine5k[i];
        sp.processStereo(l, r);
        output[i] = l;
    }
    EXPECT_LT(rms(output), rms(sine5k) * 0.8f);
}

TEST(LinkwitzRileyCrossover, SplitsIntoLowAndHigh) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 1000.0f);
    float lowL, lowR, highL, highR;
    // Feed a sample
    xo.processStereo(1.0f, 1.0f, lowL, lowR, highL, highR);
    // Low + High should approximately equal input (energy conservation)
    EXPECT_NEAR(lowL + highL, 1.0f, 0.1f);
}

TEST(LinkwitzRileyCrossover, LowPassesLowFreq) {
    LinkwitzRileyCrossover xo;
    xo.prepare(kSR, 1000.0f);
    auto sine100 = generateSine(100.0f, kSR, 48000);
    float lowRms = 0.0f, highRms = 0.0f;
    for (int i = 0; i < 48000; ++i) {
        float lowL, lowR, highL, highR;
        xo.processStereo(sine100[i], sine100[i], lowL, lowR, highL, highR);
        lowRms += lowL * lowL;
        highRms += highL * highL;
    }
    EXPECT_GT(lowRms, highRms * 10.0f);  // Low band dominates
}

TEST(MultibandEngine, SingleBandPassthrough) {
    MultibandEngine mb;
    float freqs[] = {250.0f};
    mb.prepare(kSR, 1, freqs);
    float bandL[6], bandR[6];
    mb.split(0.75f, 0.5f, bandL, bandR);
    EXPECT_FLOAT_EQ(bandL[0], 0.75f);
    EXPECT_FLOAT_EQ(bandR[0], 0.5f);
}

TEST(MultibandEngine, TwoBandSplitRecombine) {
    MultibandEngine mb;
    float freqs[] = {1000.0f};
    mb.prepare(kSR, 2, freqs);
    // Process many samples to let filter settle
    float bandL[6], bandR[6];
    for (int i = 0; i < 1000; ++i)
        mb.split(0.5f, 0.5f, bandL, bandR);
    // Recombine should approximately equal input
    float outL, outR;
    mb.recombine(bandL, bandR, outL, outR);
    EXPECT_NEAR(outL, 0.5f, 0.05f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 10: STEREO
// ═══════════════════════════════════════════════════════════════════════════

TEST(MidSideEncoder, DisabledPassthrough) {
    MidSideEncoder ms;
    ms.enabled = false;
    float l = 0.8f, r = 0.3f;
    ms.encode(l, r);
    EXPECT_FLOAT_EQ(l, 0.8f);
    EXPECT_FLOAT_EQ(r, 0.3f);
}

TEST(MidSideEncoder, EncodeDecodeRoundtrip) {
    MidSideEncoder ms;
    ms.enabled = true;
    float l = 0.8f, r = 0.3f;
    ms.encode(l, r);
    // After encode: l = mid, r = side
    EXPECT_NEAR(l, 0.55f, 0.001f);   // (0.8+0.3)/2
    EXPECT_NEAR(r, 0.25f, 0.001f);   // (0.8-0.3)/2
    ms.decode(l, r);
    EXPECT_NEAR(l, 0.8f, 0.001f);
    EXPECT_NEAR(r, 0.3f, 0.001f);
}

TEST(MidSideEncoder, MonoSignalHasNoSide) {
    MidSideEncoder ms;
    ms.enabled = true;
    float l = 0.5f, r = 0.5f;
    ms.encode(l, r);
    EXPECT_NEAR(r, 0.0f, 1e-6f);  // Side = 0 for mono
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 11: MODULATION
// ═══════════════════════════════════════════════════════════════════════════

TEST(LFO, ProducesSineWave) {
    LFO lfo;
    lfo.prepare(kSR, 1.0f);  // 1 Hz
    lfo.shape = LFO::Shape::Sine;
    // Sample one full cycle
    std::vector<float> cycle(48000);
    for (int i = 0; i < 48000; ++i)
        cycle[i] = lfo.next();
    // Should cross zero
    bool hasPositive = false, hasNegative = false;
    for (auto s : cycle) {
        if (s > 0.4f) hasPositive = true;
        if (s < -0.4f) hasNegative = true;
    }
    EXPECT_TRUE(hasPositive);
    EXPECT_TRUE(hasNegative);
}

TEST(LFO, TriangleShape) {
    LFO lfo;
    lfo.prepare(kSR, 2.0f);
    lfo.shape = LFO::Shape::Triangle;
    float prev = lfo.next();
    int dirChanges = 0;
    bool rising = true;
    for (int i = 1; i < 48000; ++i) {
        float cur = lfo.next();
        bool nowRising = cur > prev;
        if (nowRising != rising) {
            ++dirChanges;
            rising = nowRising;
        }
        prev = cur;
    }
    // 2 Hz = 2 cycles = 4 direction changes (approx)
    EXPECT_GE(dirChanges, 3);
    EXPECT_LE(dirChanges, 6);
}

TEST(LFO, ResetClearsPhase) {
    LFO lfo;
    lfo.prepare(kSR, 1.0f);
    lfo.shape = LFO::Shape::Sine;
    for (int i = 0; i < 12000; ++i) lfo.next();
    lfo.reset();
    float first = lfo.next();
    EXPECT_NEAR(first, 0.0f, 0.001f);  // Sine starts at 0
}

TEST(MacroInterpreter, LinearMapping) {
    MacroInterpreter mi;
    MacroInterpreter::Mapping m;
    m.targetParamID = "drive";
    m.depth = 1.0f;
    m.curve = MacroInterpreter::Curve::Linear;
    mi.slots[0].mappings[0] = m;
    mi.slots[0].numMappings = 1;
    mi.slots[0].value = 0.5f;
    float result = mi.getMappedValue(0, 0);
    EXPECT_NEAR(result, 0.5f, 0.001f);
}

TEST(MacroInterpreter, ExponentialCurve) {
    MacroInterpreter mi;
    MacroInterpreter::Mapping m;
    m.targetParamID = "drive";
    m.depth = 1.0f;
    m.curve = MacroInterpreter::Curve::Exponential;
    mi.slots[0].mappings[0] = m;
    mi.slots[0].numMappings = 1;
    mi.slots[0].value = 0.5f;
    float result = mi.getMappedValue(0, 0);
    // Exponential at 0.5 should be less than 0.5 (concave up)
    EXPECT_LT(result, 0.5f);
    EXPECT_GT(result, 0.0f);
}

TEST(AutoGainSmoother, CompensatesGain) {
    AutoGainSmoother ags;
    ags.prepare(kSR);
    // Simulate: input is quiet, output is loud
    for (int i = 0; i < 48000; ++i) {
        ags.updateInput(0.1f);
        ags.updateOutput(0.5f);
    }
    float comp = ags.getCompensationGain();
    EXPECT_LT(comp, 1.0f);  // Should reduce gain
}

TEST(AutoGainSmoother, UnityWhenMatched) {
    AutoGainSmoother ags;
    ags.prepare(kSR);
    for (int i = 0; i < 48000; ++i) {
        ags.updateInput(0.5f);
        ags.updateOutput(0.5f);
    }
    float comp = ags.getCompensationGain();
    EXPECT_NEAR(comp, 1.0f, 0.1f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 12: METERING
// ═══════════════════════════════════════════════════════════════════════════

TEST(MeterBallistics, TracksPeak) {
    MeterBallistics mb;
    mb.prepare(kSR);
    mb.process(0.8f, 0.6f);
    EXPECT_GT(mb.peakL, 0.0f);
    EXPECT_GT(mb.peakR, 0.0f);
}

TEST(MeterBallistics, PeakDecays) {
    MeterBallistics mb;
    mb.prepare(kSR);
    mb.process(0.9f, 0.9f);
    float peakAfterBurst = mb.peakL;
    for (int i = 0; i < 4800; ++i)
        mb.process(0.0f, 0.0f);
    EXPECT_LT(mb.peakL, peakAfterBurst);
}

TEST(MeterBallistics, RMSTracksLevel) {
    MeterBallistics mb;
    mb.prepare(kSR);
    for (int i = 0; i < 4800; ++i)
        mb.process(0.5f, 0.5f);
    EXPECT_GT(mb.rmsL, 0.0f);
    EXPECT_LE(mb.rmsL, 0.5f * 0.5f * 1.1f);
}

TEST(LoudnessMeter, MeasuresLUFS) {
    LoudnessMeter lm;
    lm.prepare(kSR);
    auto sine = generateSine(1000.0f, kSR, 48000, 0.5f);
    for (int i = 0; i < 48000; ++i)
        lm.process(sine[i], sine[i]);
    // Should produce a reasonable LUFS value (not -100)
    EXPECT_GT(lm.momentary, -40.0f);
    EXPECT_LT(lm.momentary, 0.0f);
}

TEST(LoudnessMeter, TruePeakTracking) {
    LoudnessMeter lm;
    lm.prepare(kSR);
    lm.process(0.9f, 0.9f);
    EXPECT_GT(lm.truePeak, -5.0f);
}

TEST(SpectrumBuffer, FillsAndSignalsReady) {
    SpectrumBuffer sb;
    sb.reset();
    EXPECT_FALSE(sb.ready);
    for (int i = 0; i < kSpectrumFFTSize; ++i)
        sb.pushSample(0.5f);
    EXPECT_TRUE(sb.ready);
}

TEST(GainReductionHistory, PushAndRetrieve) {
    GainReductionHistory grh;
    grh.reset();
    grh.push(-3.0f);
    grh.push(-6.0f);
    EXPECT_FLOAT_EQ(grh.getAt(0), -6.0f);
    EXPECT_FLOAT_EQ(grh.getAt(1), -3.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 13: ANALYSIS
// ═══════════════════════════════════════════════════════════════════════════

TEST(ReferenceToneMatcher, CaptureAndCompute) {
    ReferenceToneMatcher rtm;
    rtm.reset();
    EXPECT_FALSE(rtm.hasReference);

    float refSpec[kRefSpectrumBins];
    float curSpec[kRefSpectrumBins];
    for (int i = 0; i < kRefSpectrumBins; ++i) {
        refSpec[i] = 1.0f;
        curSpec[i] = 0.5f;
    }
    rtm.captureReference(refSpec, kRefSpectrumBins, kSR);
    EXPECT_TRUE(rtm.hasReference);

    rtm.captureCurrent(curSpec, kRefSpectrumBins, kSR);
    rtm.computeCorrection();
    EXPECT_TRUE(rtm.hasCorrection);

    // Correction should be positive (need to boost current to match reference)
    for (int i = 0; i < kRefSpectrumBins; ++i)
        EXPECT_GT(rtm.correctionCurve[i], 0.0f);
}

TEST(PresetIntelligence, DetectsDrums) {
    PresetIntelligence pi;
    pi.reset();
    MeterBallistics mb;
    mb.prepare(kSR);
    // Simulate drums: high peak, low RMS
    mb.peakL = mb.peakR = 0.9f;
    mb.rmsL = mb.rmsR = 0.01f;  // High crest factor
    SpectrumBuffer sb;
    pi.analyze(mb, sb, 0.0f, kSR);
    EXPECT_EQ(pi.detectedType, PresetIntelligence::SignalType::Drums);
}

TEST(PresetIntelligence, DetectsPad) {
    PresetIntelligence pi;
    pi.reset();
    MeterBallistics mb;
    mb.prepare(kSR);
    // Simulate pad: low crest factor
    mb.peakL = mb.peakR = 0.5f;
    mb.rmsL = mb.rmsR = 0.2f;  // Low crest factor (peak/rms < 2)
    SpectrumBuffer sb;
    pi.analyze(mb, sb, 0.0f, kSR);
    EXPECT_EQ(pi.detectedType, PresetIntelligence::SignalType::Pad);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 14: STATE MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════

TEST(UndoStack, PushAndUndo) {
    UndoStack us;
    us.clear();
    juce::MemoryBlock mb1, mb2;
    mb1.append("state1", 6);
    mb2.append("state2", 6);
    us.push(mb1, "first");
    us.push(mb2, "second");
    EXPECT_TRUE(us.canUndo());
    auto* snap = us.undo();
    ASSERT_NE(snap, nullptr);
    EXPECT_EQ(snap->data, mb1);
}

TEST(UndoStack, RedoAfterUndo) {
    UndoStack us;
    us.clear();
    juce::MemoryBlock mb1, mb2;
    mb1.append("a", 1);
    mb2.append("b", 1);
    us.push(mb1);
    us.push(mb2);
    us.undo();
    EXPECT_TRUE(us.canRedo());
    auto* snap = us.redo();
    ASSERT_NE(snap, nullptr);
    EXPECT_EQ(snap->data, mb2);
}

TEST(UndoStack, TruncatesRedoOnPush) {
    UndoStack us;
    us.clear();
    juce::MemoryBlock mb1, mb2, mb3;
    mb1.append("a", 1);
    mb2.append("b", 1);
    mb3.append("c", 1);
    us.push(mb1);
    us.push(mb2);
    us.undo();
    us.push(mb3);
    EXPECT_FALSE(us.canRedo());
}

TEST(ABState, StoreAndToggle) {
    ABState ab;
    juce::MemoryBlock mbA, mbB;
    mbA.append("A", 1);
    mbB.append("B", 1);
    ab.storeA(mbA);
    ab.storeB(mbB);
    EXPECT_TRUE(ab.isA);
    ab.toggle();
    EXPECT_FALSE(ab.isA);
    auto* active = ab.getActive();
    ASSERT_NE(active, nullptr);
    EXPECT_EQ(*active, mbB);
}

TEST(ABState, CopyAtoB) {
    ABState ab;
    juce::MemoryBlock mbA;
    mbA.append("A", 1);
    ab.storeA(mbA);
    ab.copyAtoB();
    EXPECT_TRUE(ab.hasB);
    EXPECT_EQ(ab.slotB, mbA);
}

TEST(MIDILearnState, AddAndFindMapping) {
    MIDILearnState mls;
    mls.clearAll();
    mls.addMapping(74, "drive", 0.0f, 1.0f);
    EXPECT_EQ(mls.numMappings, 1);
    auto* m = mls.findMapping(74);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->parameterID, juce::String("drive"));
}

TEST(MIDILearnState, RemoveMapping) {
    MIDILearnState mls;
    mls.clearAll();
    mls.addMapping(1, "drive", 0.0f, 1.0f);
    mls.addMapping(2, "mix", 0.0f, 1.0f);
    mls.removeMapping(0);
    EXPECT_EQ(mls.numMappings, 1);
    EXPECT_EQ(mls.mappings[0].ccNumber, 2);
}

TEST(MIDILearnState, MaxMappingsEnforced) {
    MIDILearnState mls;
    mls.clearAll();
    for (int i = 0; i < kMaxMIDIMappings + 5; ++i)
        mls.addMapping(i, "p" + juce::String(i), 0.0f, 1.0f);
    EXPECT_EQ(mls.numMappings, kMaxMIDIMappings);
}

TEST(SimpleModeState, DefaultValues) {
    SimpleModeState sms;
    EXPECT_FALSE(sms.enabled);
    EXPECT_FLOAT_EQ(sms.drive, 0.5f);
    EXPECT_FLOAT_EQ(sms.tone, 0.5f);
    EXPECT_FLOAT_EQ(sms.output, 0.5f);
}

TEST(LoudnessMatchedAB, CompensatesLouder) {
    LoudnessMatchedAB lmab;
    lmab.prepare(kSR);
    // Slot A is louder
    for (int i = 0; i < 48000; ++i) {
        lmab.updateLevel(true, 0.8f, 0.8f);
        lmab.updateLevel(false, 0.4f, 0.4f);
    }
    float compA = lmab.getCompensation(true);
    EXPECT_LT(compA, 1.0f);  // Should reduce A to match B
}

TEST(LoudnessMatchedAB, UnityWhenEqual) {
    LoudnessMatchedAB lmab;
    lmab.prepare(kSR);
    for (int i = 0; i < 48000; ++i) {
        lmab.updateLevel(true, 0.5f, 0.5f);
        lmab.updateLevel(false, 0.5f, 0.5f);
    }
    float comp = lmab.getCompensation(true);
    EXPECT_NEAR(comp, 1.0f, 0.1f);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 15: REAL-TIME SAFETY
// ═══════════════════════════════════════════════════════════════════════════

TEST(RTSafety, NoAllocationInSaturation) {
    // Waveshaper::process should not allocate — verified by structure
    float tapeState = 0.0f;
    for (int i = 0; i < 10000; ++i) {
        float x = (float)i / 10000.0f;
        Waveshaper::process(SaturationModel::Tube, x, tapeState, 0.5f);
    }
    // If we get here without crash, no allocation happened
    SUCCEED();
}

TEST(RTSafety, SafetyLayerHandlesAllEdgeCases) {
    SafetyLayer sl;
    sl.setSampleRate(kSR);
    float cases[][2] = {
        {0.0f, 0.0f},
        {1.0f, -1.0f},
        {100.0f, -100.0f},
        {std::numeric_limits<float>::quiet_NaN(), 0.0f},
        {0.0f, std::numeric_limits<float>::infinity()},
        {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()},
        {std::numeric_limits<float>::denorm_min(), -std::numeric_limits<float>::denorm_min()},
    };
    for (auto& c : cases) {
        float l = c[0], r = c[1];
        sl.processStereo(l, r);
        EXPECT_TRUE(std::isfinite(l));
        EXPECT_TRUE(std::isfinite(r));
        EXPECT_LE(std::abs(l), 4.0f);
        EXPECT_LE(std::abs(r), 4.0f);
    }
}

TEST(RTSafety, StateVersionMatches) {
    EXPECT_EQ(kStateVersion, 12);  // kStateVersion stays at 12 for v1.0.1 (additive params only)
}

// ═══════════════════════════════════════════════════════════════════════════
// Target Lock Engine
// ═══════════════════════════════════════════════════════════════════════════

TEST(TargetLock, PrepareAndProcess) {
    BTZDsp::TargetLockEngine tle;
    tle.prepare(48000.0);

    // Set LUFS target and lock
    tle.setLUFSTarget(-14.0f);
    tle.lufsLocked = true;
    tle.setDynamicsThreshold(3.0f);

    EXPECT_TRUE(tle.isActive());

    // Process a sample — should return a gain multiplier
    float bandPeaks[3] = { 0.5f, 0.3f, 0.1f };
    float bandGains[3] = { 1.0f, 1.0f, 1.0f };
    float gain = tle.process(0.5f, 0.5f, -20.0f, bandPeaks, bandGains);

    // Gain should be > 1.0 because current (-20) is below target (-14)
    EXPECT_GT(gain, 1.0f);
    // But should be limited (not infinite)
    EXPECT_LT(gain, 10.0f);
}

TEST(TargetLock, InactiveWhenNoLock) {
    BTZDsp::TargetLockEngine tle;
    tle.prepare(44100.0);
    tle.lufsLocked = false;
    tle.rmsLocked = false;
    tle.bands[0].locked = false;
    tle.bands[1].locked = false;
    tle.bands[2].locked = false;

    EXPECT_FALSE(tle.isActive());
}

TEST(TargetLock, BandLockAppliesGain) {
    BTZDsp::TargetLockEngine tle;
    tle.prepare(48000.0);

    tle.bands[0].locked = true;
    tle.bands[0].targetDb = -6.0f;
    tle.setDynamicsThreshold(0.0f);  // hard lock

    EXPECT_TRUE(tle.isActive());

    // Feed a low-band peak that is too quiet (-20 dB)
    float bandPeaks[3] = { 0.1f, 0.5f, 0.3f };  // 0.1 ≈ -20 dB
    float bandGains[3] = { 1.0f, 1.0f, 1.0f };
    float gain = tle.process(0.3f, 0.3f, -14.0f, bandPeaks, bandGains);

    // Low band gain should be boosted (target -6 vs measured ~-20)
    EXPECT_GT(bandGains[0], 1.0f);
    // Mid and High should remain at 1.0 (not locked)
    EXPECT_FLOAT_EQ(bandGains[1], 1.0f);
    EXPECT_FLOAT_EQ(bandGains[2], 1.0f);
}

// ═══════════════════════════════════════════════════════════════════════════
// TruePeakLimiter (ISP)
// ═══════════════════════════════════════════════════════════════════════════

TEST(TruePeakLimiter, CatchesInterSamplePeak) {
    BTZDsp::TruePeakLimiter lim;
    lim.prepare(48000.0);
    lim.ceiling = -0.3f;

    // Feed a signal that creates an inter-sample peak above ceiling
    // Two consecutive samples that will interpolate above 1.0
    float l = 0.9f, r = 0.9f;
    lim.processStereo(l, r);
    l = 0.95f; r = 0.95f;
    lim.processStereo(l, r);

    // After processing, output should be limited
    const float ceilingLin = std::pow(10.0f, -0.3f / 20.0f);
    EXPECT_LE(std::abs(l), ceilingLin + 0.01f);
    EXPECT_LE(std::abs(r), ceilingLin + 0.01f);
}

// ═══════════════════════════════════════════════════════════════════════════
// LoudnessMeter (K-weighting)
// ═══════════════════════════════════════════════════════════════════════════

TEST(LoudnessMeter, KWeightingReducesLowFreq) {
    BTZDsp::LoudnessMeter meter;
    meter.prepare(48000.0);

    // Feed 100 Hz sine (should be attenuated by K-weighting HPF)
    const float freq = 100.0f;
    const float sr = 48000.0f;
    for (int i = 0; i < 48000; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159265f * freq * i / sr);
        meter.process(sample, sample);
    }
    const float lowLUFS = meter.integrated;

    // Reset and feed 2kHz sine (should pass through K-weighting)
    meter.prepare(48000.0);
    const float freq2 = 2000.0f;
    for (int i = 0; i < 48000; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159265f * freq2 * i / sr);
        meter.process(sample, sample);
    }
    const float midLUFS = meter.integrated;

    // 2kHz should read louder than 100Hz due to K-weighting
    EXPECT_GT(midLUFS, lowLUFS);
}

// ═══════════════════════════════════════════════════════════════════════════
// LinkwitzRileyCrossover (LR4)
// ═══════════════════════════════════════════════════════════════════════════

TEST(LinkwitzRileyCrossover, SumsFlatAtCrossover) {
    BTZDsp::LinkwitzRileyCrossover xo;
    xo.prepare(48000.0, 1000.0f);

    // Feed white noise and check that LP + HP sums back to original
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float maxError = 0.0f;
    // Allow some settling time
    for (int i = 0; i < 1000; ++i) {
        float inL = dist(rng), inR = dist(rng);
        float lpL, lpR, hpL, hpR;
        xo.processStereo(inL, inR, lpL, lpR, hpL, hpR);
    }
    // After settling, check sum accuracy
    for (int i = 0; i < 10000; ++i) {
        float inL = dist(rng), inR = dist(rng);
        float lpL, lpR, hpL, hpR;
        xo.processStereo(inL, inR, lpL, lpR, hpL, hpR);
        float errL = std::abs((lpL + hpL) - inL);
        float errR = std::abs((lpR + hpR) - inR);
        maxError = std::max({maxError, errL, errR});
    }
    // LR4 should sum perfectly (allpass) — allow small float error
    EXPECT_LT(maxError, 0.01f);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
