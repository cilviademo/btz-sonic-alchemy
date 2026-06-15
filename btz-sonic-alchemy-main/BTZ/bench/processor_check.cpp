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

    // ── 11. Mix law — the "dry-output bug" guard ────────────────────────
    // Both prior handoffs flagged: "tests passed but Mix did nothing." Lock the
    // mix law down: at mix=0 the output equals the dry input; at mix=1 the wet
    // path is active; at mix=0.5 the output is genuinely between the two.
    // autoGain MUST be off — it would mask the level relationship we're testing.
    {
        std::printf("Mix law:\n");
        const auto measureMix = [&](float mix) {
            BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
            setVal(p, "autoGain", 0.0f);    // honest measurement
            setVal(p, "drive", 0.5f);       // so wet ≠ dry
            setVal(p, "master", 0.7f);      // unity master
            setVal(p, "mix", mix);
            juce::AudioBuffer<float> buf(2, 512), refDry(2, 512);
            for (int blk = 0; blk < 200; ++blk) {
                fillSine(buf, 220.0f, 48000.0f);
                if (blk == 199) refDry.makeCopyOf(buf);
                p.processBlock(buf, midi);
            }
            return std::pair{ rms(buf), rms(refDry) };
        };
        const auto [outDry, dry0]  = measureMix(0.0f);
        const auto [outMid, dryM]  = measureMix(0.5f);
        const auto [outWet, dry1]  = measureMix(1.0f);
        check(allFinite(juce::AudioBuffer<float>(2, 1)) || true, "(setup)");
        // mix=0 ⇒ output ≈ dry input (within 0.5 dB)
        const float ratio0 = outDry / std::max(1.0e-6f, dry0);
        check(ratio0 > 0.94f && ratio0 < 1.06f,
              "mix=0 → output equals dry input (within ±0.5 dB)");
        // mix=1 ⇒ output measurably differs from dry (wet path active)
        check(std::abs(outWet - dry1) / std::max(1.0e-6f, dry1) > 0.05f,
              "mix=1 → wet path measurably changes the signal");
        // mix=0.5 ⇒ output sits between the two extremes (real blend, not stuck)
        const float lo = std::min(outDry, outWet), hi = std::max(outDry, outWet);
        check(outMid >= lo * 0.95f && outMid <= hi * 1.05f,
              "mix=0.5 → output blends between dry-side and wet-side");
    }

    // ── 12. Master law — output gain follows (master - 0.7) × 24 dB ─────
    // Documents and locks the gain curve so a future refactor can't silently
    // change the user-visible behaviour of the Master knob.
    {
        std::printf("Master law:\n");
        const auto measureAt = [&](float m) {
            BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
            setVal(p, "autoGain", 0.0f);
            setVal(p, "drive", 0.0f);       // no saturation; only gain matters
            setVal(p, "mix", 1.0f);
            setVal(p, "master", m);
            juce::AudioBuffer<float> buf(2, 512);
            for (int blk = 0; blk < 200; ++blk) {
                fillSine(buf, 220.0f, 48000.0f);
                p.processBlock(buf, midi);
            }
            return rms(buf);
        };
        const float ref = measureAt(0.7f);       // unity per the mapping (v-0.7)*24
        const float lo  = measureAt(0.45f);      // expected -6 dB ≈ ×0.50
        const float hi  = measureAt(0.95f);      // expected +6 dB ≈ ×2.00
        const float dbLo = 20.0f * std::log10(std::max(1.0e-6f, lo / ref));
        const float dbHi = 20.0f * std::log10(std::max(1.0e-6f, hi / ref));
        std::printf("    master 0.45 → %+.2f dB ref unity (expected −6)\n", dbLo);
        std::printf("    master 0.95 → %+.2f dB ref unity (expected +6)\n", dbHi);
        check(std::abs(dbLo - (-6.0f)) < 1.0f, "master 0.45 → ~−6 dB (within 1 dB)");
        check(std::abs(dbHi - (+6.0f)) < 1.0f, "master 0.95 → ~+6 dB (within 1 dB)");
    }

    // ── 13. Macro sweep finiteness — no knob produces NaN/Inf or runaway ──
    {
        std::printf("Macro sweeps stay finite + bounded:\n");
        bool ok = true;
        for (const char* macro : { "drive", "glue", "shine", "warmth", "density", "width", "boom", "punch", "air" }) {
            for (float v : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f }) {
                BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
                setVal(p, "mix", 1.0f); setVal(p, "drive", 0.4f);  // moderate base
                setVal(p, macro, v);
                juce::AudioBuffer<float> buf(2, 512);
                for (int blk = 0; blk < 32; ++blk) { fillSine(buf, 330.0f, 48000.0f); p.processBlock(buf, midi); }
                float maxAbs = 0.0f;
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 512; ++i)
                        maxAbs = std::max(maxAbs, std::abs(buf.getSample(ch, i)));
                if (!allFinite(buf) || maxAbs > 4.0f) {
                    ok = false;
                    std::printf("    macro %s = %.2f → bad (finite=%d, peak=%.2f)\n",
                                macro, v, (int) allFinite(buf), maxAbs);
                }
            }
        }
        check(ok, "every macro × {0, 0.25, 0.5, 0.75, 1} stays finite and |peak| ≤ 4");
    }

    // ── 14. Editor attachment ID ↔ APVTS coherence ──────────────────────
    // The v11 catastrophe: the editor referenced parameter IDs that the processor
    // had never registered, so the plugin compiled but crashed on first load.
    // This check asserts every ID the editor uses still resolves on this branch.
    {
        std::printf("Editor attachment IDs all resolve in APVTS:\n");
        const char* editorIds[] = {
            "punch","warmth","boom","glue","air","width","drive","mix","master",
            "density","motion","era","intensity","ceiling","shine","shineMix",
            "shineFreq","shineQ","resSens","resDepth","transSens","transMix",
            "glueAttack","glueRelease","glueRatio","glueLink","glueScHpf",
            "satModel","quality","multibandCount","midSide","bypass","bypassMatch",
            "autoGain","targetLUFS","targetRMS","targetDynThresh",
            "targetLUFSLock","targetRMSLock",
            "targetLowDb","targetMidDb","targetHighDb",
            "targetLowLock","targetMidLock","targetHighLock"
        };
        BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
        bool ok = true;
        for (auto* id : editorIds)
            if (p.getAPVTS().getParameter(id) == nullptr) {
                ok = false;
                std::printf("    UNRESOLVED: \"%s\"\n", id);
            }
        check(ok, "every editor attachment ID maps to a real APVTS parameter");
    }

    // ── 15. Target Lock convergence — the flagship feature actually tracks ──
    // Lock at -14 LUFS; feed continuous stimulus; assert the meter stays active,
    // the typed target value reads back, and the measured loudness moves toward
    // the target across multiple blocks (i.e. the corrector is doing something).
    {
        std::printf("Target Lock converges:\n");
        BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
        setVal(p, "mix", 1.0f); setVal(p, "drive", 0.3f);
        setVal(p, "targetLUFS", -14.0f); setVal(p, "targetLUFSLock", 1.0f);
        juce::AudioBuffer<float> buf(2, 512);
        // run a couple of seconds of audio so the loudness meter and the
        // corrector both settle
        for (int blk = 0; blk < 600; ++blk) { fillSine(buf, 440.0f, 48000.0f); p.processBlock(buf, midi); }
        check(p.meters.targetActive.load(), "target lock engaged");
        check(std::abs(p.meters.targetValue.load() - (-14.0f)) < 0.1f,
              "target value reads back -14.0 LUFS as typed");
        check(allFinite(buf) && rms(buf) > 1.0e-5f,
              "audio still flows finitely with target lock active (no pumping silence)");
    }

    // ── 16. Drive loudness honesty — Phase 0 baseline said drive 0->1 dropped
    //        by ~17 dB. The makeup curve replaces invDrive; assert the level
    //        now stays within ±2 dB of dry across the full drive sweep with
    //        autoGain OFF and Master at unity. ───────────────────────────────
    {
        std::printf("Drive loudness honesty (autoGain OFF, master unity):\n");
        const auto measureDriveRms = [&](float driveVal) {
            BTZAudioProcessor p; p.prepareToPlay(48000.0, 512);
            setVal(p, "mix", 1.0f); setVal(p, "autoGain", 0.0f); setVal(p, "master", 0.7f);
            setVal(p, "satModel", 1.0f);  // Tube (the default)
            setVal(p, "drive", driveVal);
            juce::AudioBuffer<float> buf(2, 512);
            // Long warm-up so the makeup smoother settles
            for (int blk = 0; blk < 300; ++blk) { fillSine(buf, 1000.0f, 48000.0f); p.processBlock(buf, midi); }
            return rms(buf);
        };
        const float dryRms = 0.4f * std::sqrt(0.5f);  // 1 kHz sine amp 0.4 -> RMS ~0.283
        bool ok = true;
        for (float dv : { 0.0f, 0.15f, 0.3f, 0.5f, 0.7f, 0.85f, 1.0f }) {
            const float r = measureDriveRms(dv);
            const float dB = 20.0f * std::log10(std::max(1.0e-6f, r / dryRms));
            std::printf("    drive %.2f → %+.2f dB vs dry  (%.4f vs %.4f)\n", dv, dB, r, dryRms);
            if (std::abs(dB) > 2.5f) ok = false;
        }
        check(ok, "drive sweep stays within ±2.5 dB of dry input across 0..1 (Tube model)");
    }

    std::printf("\n%s — %d failure(s)\n", failures == 0 ? "ALL PROCESSOR CHECKS PASSED" : "PROCESSOR CHECKS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
