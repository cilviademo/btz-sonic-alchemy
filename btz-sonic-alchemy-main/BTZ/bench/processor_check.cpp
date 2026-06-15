/*
  BTZ — processor-level integration check.
  Exercises the REAL BTZAudioProcessor::processBlock end-to-end (the layer the
  unit tests don't cover). Guards the class of bug found in the deep debug pass
  (smoothers frozen -> dry output). Returns non-zero on failure (CI-usable).
  Build: -DBTZ_BUILD_BENCH=ON ; run: BTZProcessorCheck
*/
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <cstdio>
#include <cmath>

namespace {
int failures = 0;
void check(bool cond, const char* what) {
    std::printf("  [%s] %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) ++failures;
}
void setVal(BTZAudioProcessor& p, const char* id, float v) {
    if (auto* prm = p.getAPVTS().getParameter(id))
        prm->setValueNotifyingHost(prm->getNormalisableRange().convertTo0to1(v));
}
float rms(const juce::AudioBuffer<float>& b) {
    double s = 0; int n = 0;
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i) { const float x = b.getSample(ch, i); s += x * x; ++n; }
    return (float)std::sqrt(s / juce::jmax(1, n));
}
bool allFinite(const juce::AudioBuffer<float>& b) {
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (!std::isfinite(b.getSample(ch, i))) return false;
    return true;
}
void fillSine(juce::AudioBuffer<float>& b, float freq, float sr) {
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
            b.setSample(ch, i, 0.4f * std::sin(2.0f * 3.14159265f * freq * i / sr));
}
}

int main() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    juce::MidiBuffer midi;

    // ── 1. The wet path actually processes (regression guard for the dry bug) ──
    {
        std::printf("Wet path active:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.7f); setVal(p, "glue", 0.6f);
        setVal(p, "shine", 0.5f); setVal(p, "width", 0.8f);
        juce::AudioBuffer<float> buf(2, 512), dry(2, 512);
        for (int blk = 0; blk < 64; ++blk) { fillSine(buf, 220.0f, 48000.0f); if (blk == 63) dry.makeCopyOf(buf); p.processBlock(buf, midi); }
        check(allFinite(buf), "output is finite");
        check(rms(buf) > 1.0e-4f, "output is non-silent");
        // wet must differ from dry by a meaningful margin
        float diff = 0; for (int i = 0; i < 512; ++i) diff += std::abs(buf.getSample(0, i) - dry.getSample(0, i));
        check(diff / 512.0f > 1.0e-3f, "output differs from input (processing happened)");
    }

    // ── 2. Bypass returns the input ──
    {
        std::printf("Bypass passthrough:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "bypass", 1.0f);
        juce::AudioBuffer<float> buf(2, 512), dry(2, 512);
        for (int blk = 0; blk < 32; ++blk) { fillSine(buf, 440.0f, 48000.0f); dry.makeCopyOf(buf); p.processBlock(buf, midi); }
        float diff = 0; for (int i = 0; i < 512; ++i) diff += std::abs(buf.getSample(0, i) - dry.getSample(0, i));
        check(diff / 512.0f < 1.0e-3f, "bypassed output ≈ input");
    }

    // ── 3. State save/restore round-trip ──
    {
        std::printf("State round-trip:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "drive", 0.83f); setVal(p, "warmth", 0.21f);
        juce::MemoryBlock state; p.getStateInformation(state);
        setVal(p, "drive", 0.1f);  // change away
        p.setStateInformation(state.getData(), (int)state.getSize());
        const float d = *p.getAPVTS().getRawParameterValue("drive");
        const float w = *p.getAPVTS().getRawParameterValue("warmth");
        check(std::abs(d - 0.83f) < 0.01f && std::abs(w - 0.21f) < 0.01f, "params restored from state");
    }

    // ── 4. Sample-rate / block-size matrix (no crash, finite) ──
    {
        std::printf("SR/blocksize matrix:\n");
        bool ok = true;
        for (double sr : { 44100.0, 48000.0, 96000.0 })
            for (int bs : { 32, 128, 512, 1024 }) {
                BTZAudioProcessor p; p.prepareToPlay(sr, bs);
                setVal(p, "mix", 1.0f); setVal(p, "drive", 0.6f); setVal(p, "quality", 2.0f);
                juce::AudioBuffer<float> buf(2, bs);
                for (int blk = 0; blk < 8; ++blk) { fillSine(buf, 1000.0f, (float)sr); p.processBlock(buf, midi); }
                ok = ok && allFinite(buf);
            }
        check(ok, "all SR/blocksize combos produce finite output");
    }

    // ── 5. Oversized-block robustness (host violates declared max) ──
    {
        std::printf("Oversized-block guard:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.5f);
        juce::AudioBuffer<float> big(2, 2048);   // 4x the declared max
        fillSine(big, 500.0f, 48000.0f);
        p.processBlock(big, midi);               // must not crash / OOB
        check(allFinite(big), "oversized block handled without NaN/crash");
    }

    // ── 6. Every factory preset loads and produces finite, non-silent audio ──
    {
        std::printf("Factory presets:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        const int n = p.getNumFactoryPresets();
        check(n >= 10, "at least 10 factory presets exist");
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            p.loadFactoryPreset(i);
            juce::AudioBuffer<float> buf(2, 512);
            for (int blk = 0; blk < 16; ++blk) { fillSine(buf, 330.0f, 48000.0f); p.processBlock(buf, midi); }
            if (!allFinite(buf) || rms(buf) < 1.0e-5f) {
                ok = false;
                std::printf("    preset %d (%s) produced bad output\n",
                            i, p.getFactoryPresets()[(size_t) i].name.toRawUTF8());
            }
        }
        check(ok, "all factory presets produce finite, non-silent output");
    }

    // ── 7. Parameters display real units + parse typed values back ──
    {
        std::printf("Parameter units:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        auto& a = p.getAPVTS();
        auto txt = [&](const char* id) {
            auto* prm = a.getParameter(id);
            return prm ? prm->getText(prm->getValue(), 32) : juce::String("?");
        };
        auto roundtrip = [&](const char* id, const juce::String& typed, float expectedVal, float tol) {
            auto* prm = a.getParameter(id);
            if (!prm) return false;
            const float norm = prm->getValueForText(typed);
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(prm)) {
                const float val = rp->getNormalisableRange().convertFrom0to1(norm);
                return std::abs(val - expectedVal) < tol;
            }
            return false;
        };
        setVal(p, "drive", 0.5f);   check(txt("drive").containsIgnoreCase("%"),  "drive shows %");
        setVal(p, "ceiling", -1.0f);check(txt("ceiling").containsIgnoreCase("dB"), "ceiling shows dB");
        setVal(p, "shineFreq", 8000.0f); check(txt("shineFreq").containsIgnoreCase("Hz"), "shineFreq shows Hz/kHz");
        setVal(p, "glueAttack", 10.0f);  check(txt("glueAttack").containsIgnoreCase("ms"), "glueAttack shows ms");
        setVal(p, "glueRatio", 4.0f);    check(txt("glueRatio").contains(":1"), "glueRatio shows :1");
        setVal(p, "targetLUFS", -14.0f); check(txt("targetLUFS").containsIgnoreCase("LUFS"), "targetLUFS shows LUFS");
        check(roundtrip("drive", "50 %", 0.5f, 0.02f),      "type-in '50 %' -> drive 0.5");
        check(roundtrip("ceiling", "-1.0 dB", -1.0f, 0.05f),"type-in '-1.0 dB' -> ceiling -1.0");
        check(roundtrip("shineFreq", "8.00 kHz", 8000.0f, 50.0f), "type-in '8 kHz' -> 8000 Hz");
    }

    // ── 8. Target Lock readout publishes when engaged (flagship legibility) ──
    {
        std::printf("Target Lock readout:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.4f);
        juce::AudioBuffer<float> buf(2, 512);
        // Not locked yet → readout inactive.
        for (int blk = 0; blk < 8; ++blk) { fillSine(buf, 220.0f, 48000.0f); p.processBlock(buf, midi); }
        check(!p.meters.targetActive.load(), "readout inactive when no target locked");
        // Engage LUFS lock at -14.
        setVal(p, "targetLUFS", -14.0f); setVal(p, "targetLUFSLock", 1.0f);
        for (int blk = 0; blk < 16; ++blk) { fillSine(buf, 220.0f, 48000.0f); p.processBlock(buf, midi); }
        check(p.meters.targetActive.load(), "readout active when target locked");
        check(std::abs(p.meters.targetValue.load() - (-14.0f)) < 0.1f, "readout shows the typed target (-14)");
        check(p.meters.targetIsLufs.load(), "readout flagged as LUFS");
    }

    // ── 9. Default state is "instant good" (applies character out of the box) ──
    {
        std::printf("Default state:\n");
        BTZAudioProcessor p;            // no params set — pure defaults
        p.prepareToPlay(48000.0, 512);
        juce::AudioBuffer<float> buf(2, 512), dry(2, 512);
        for (int blk = 0; blk < 64; ++blk) { fillSine(buf, 220.0f, 48000.0f); if (blk == 63) dry.makeCopyOf(buf); p.processBlock(buf, midi); }
        float diff = 0; for (int i = 0; i < 512; ++i) diff += std::abs(buf.getSample(0, i) - dry.getSample(0, i));
        check(allFinite(buf) && diff / 512.0f > 1.0e-3f, "default patch applies audible character (not transparent)");
    }

    // ── 10. Loudness-matched bypass moves the dry toward the processed loudness ──
    {
        std::printf("Loudness-matched bypass:\n");
        BTZAudioProcessor p;
        p.prepareToPlay(48000.0, 512);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.4f); setVal(p, "autoGain", 0.0f);
        setVal(p, "bypassMatch", 1.0f);
        juce::AudioBuffer<float> buf(2, 512), processed(2, 512), rawDry(2, 512);
        // Process (un-bypassed) to learn the processed/dry ratio + capture processed loudness.
        for (int blk = 0; blk < 250; ++blk) { fillSine(buf, 220.0f, 48000.0f); p.processBlock(buf, midi); if (blk == 249) processed.makeCopyOf(buf); }
        fillSine(rawDry, 220.0f, 48000.0f);                 // the un-processed input level
        // Bypass with match on → capture matched output.
        setVal(p, "bypass", 1.0f);
        juce::AudioBuffer<float> matched(2, 512);
        for (int blk = 0; blk < 80; ++blk) { fillSine(buf, 220.0f, 48000.0f); p.processBlock(buf, midi); if (blk == 79) matched.makeCopyOf(buf); }
        const float rProc = rms(processed), rRaw = rms(rawDry), rMatch = rms(matched);
        check(allFinite(matched), "matched bypass finite");
        check(std::abs(rMatch - rProc) < std::abs(rRaw - rProc),
              "matched bypass is closer to processed loudness than raw dry (level-compensated A/B)");
    }

    std::printf("\n%s — %d failure(s)\n", failures == 0 ? "ALL PROCESSOR CHECKS PASSED" : "PROCESSOR CHECKS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
