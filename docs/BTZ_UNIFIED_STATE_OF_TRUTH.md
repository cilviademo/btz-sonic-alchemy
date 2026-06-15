# BTZ — Unified State of Truth

**This document supersedes** the contradictory parts of:
- Master Handoff (June 15, 2026, "two-product" framing)
- Session Handoff (June 15, 2026, "three-implementation" framing)
- The Manus product / scorecard / handoff series
- Earlier per-session reports in this repo's `docs/` tree

It does **not** replace operational docs that are still authoritative
(`BTZ_PERF_SECURITY_AUDIT.md`, `BTZ_PARAMETER_MANIFEST.md`,
`BTZ_RELEASE_BLUEPRINT_AUDIT.md`, `BTZ_PRODUCT_IDENTITY.md`,
`BTZ_FORMAT_ROADMAP.md`, `BTZ_RELEASE_READINESS_SCORECARD.md`,
`BTZ_CHANGELOG.md`). It tells you which of those to trust and where they sit
in the bigger picture.

---

## 1. The single most important reconciliation

Different prior handoffs describe BTZ as **one product**, **two products**, or
**three implementations**. The Session Handoff is the most accurate framing:
**three distinct codebases have circulated under the same name.** That is the
root cause of every contradiction in the docs.

| # | Codebase | Where it lives | What it is | Status |
|---|---|---|---|---|
| 1 | `BTZ_JUCE/` (29-module) | (referenced in Session Handoff, not in this repo) | Largest implementation; production-grade aspirations | Out-of-repo |
| 2 | `btz-sonic-alchemy-main/BTZ/` simplified (20-param macro shaper) | Older state of this repo's tree | The "Box Tone Zone (BTZ).vst3" Windows-workstation binary the Master Handoff describes as broken in Reaper (Mix dead, Master dead) | **Superseded** by codebase #3 in this branch |
| 3 | `overhaul/v1.1-dsp-architecture` (the v12 / "Ivory" rewrite) | This branch, `claude/review-btz-compilation-aN5UD`, HEAD `54e5eb8` | What I have actually been editing all session — production-grade, ~50 parameters, 1700-line `BTZDsp.h`, all integration tests pass | **Current canonical** for this branch |

**Implication:** the Master Handoff's reports of *"Mix does nothing, Master does
nothing, several macros destructive, UI is a placeholder"* are about **codebase
#2 on the Windows workstation**, not about what's in this branch. I verified
this directly:

- The "stuck Mix smoother → fully dry" class of bug **existed** in this branch
  too; I found and fixed it in commit `3dd3eec`.
- This branch's `master` parameter is wired to a real per-block dB gain ramp
  (the Session Handoff's "per-sample inefficient" complaint is also already
  fixed here, in commit `e3c4971`).
- 86/86 unit tests + 10/10 processor integration checks pass on this branch.

**If you intend to ship the Windows-workstation v1.0** (codebase #2), my recent
work is *not* on that codebase. If you intend to ship the v12 / Ivory rewrite
(codebase #3), this branch is the head. **Pick one** before more code is written.

---

## 2. Single canonical name

Both handoffs note that the project uses three names interchangeably:

- `Box Tone Zone (BTZ)` — what the binary actually exports
- `Sonic Alchemy` — what the brand docs say
- `BTZ Ivory` — what the regression report says

This is recursively unsustainable. Recommendation: **commit to one name,
publicly, and stop using the others.** Until that decision is made, every
document and every new feature inherits this confusion.

---

## 3. What is verified, on this branch, right now

Independent of which codebase ships:

| Item | Status | Evidence |
|---|---|---|
| Compiles (Linux, GCC 13, JUCE 8.0.6) | ✅ | `BTZ_Standalone` 12 MB + VST3 `.so` link |
| Unit tests | ✅ **86/86 pass** | `./build/BTZTests_artefacts/Release/BTZTests` |
| Integration tests | ✅ **10/10 pass** | `./build/BTZProcessorCheck_artefacts/Release/BTZProcessorCheck` |
| ASan + UBSan + LeakSanitizer | ✅ **0 findings** | `build-san/` |
| CPU | ✅ 0.75 % default · 3.73 % Ultra + everything-on | `BTZBench` measured |
| State save/restore round-trip | ✅ | Processor check §3 |
| Multi-SR / multi-blocksize matrix | ✅ | Processor check §4 |
| Oversized-block defensive guard | ✅ | Processor check §5 |
| Every factory preset loads + processes | ✅ | Processor check §6 |
| Real units on every visible knob | ✅ | Processor check §7 |
| Target Lock readout (the flagship) | ✅ | Processor check §8 |
| Default patch sounds non-transparent | ✅ | Processor check §9 |
| Loudness-matched bypass works | ✅ | Processor check §10 |

What is **not** verified, and cannot be verified in this environment:
- pluginval @ strictness 10 on the patched binary (needs Linux pluginval install or a real macOS/Windows CI run).
- DAW behaviour in Reaper / Logic / Live / Cubase / Pro Tools / FL / Studio One / Bitwig (needs a real host).
- macOS Universal build / `auval` pass / Apple Silicon CPU.
- Windows VST3 in any DAW.

---

## 4. Reconciling the Session Handoff's v12 issues against the current code

The new Session Handoff listed six issues in the v12 codebase. Honest status:

| Session Handoff issue | Status on this branch | Evidence |
|---|---|---|
| **High:** `M_PI` undefined on MSVC (tests:26) | ✅ **Fixed this commit** — replaced with `BTZDsp::kPi` | `tests/test_dsp_modules.cpp:28` |
| **Med:** Master gain per-sample smoothing inefficient (PluginProcessor.cpp:411) | ✅ Fixed previously (commit `e3c4971`) — now per-block linear gain ramp, 2 `dbToGain` calls/block instead of 512 | `Source/PluginProcessor.cpp:515-528` |
| **Med:** Resonance tamer updates per-sample (PluginProcessor.cpp:476) | ✅ Fixed previously (commit `e3c4971`) — 32-band loop collapsed to single computation (bit-identical, ~32× cheaper) | `Source/BTZDsp.h:491-512` |
| **Med:** Unused `os16x` oversampling | 🟡 Confirmed unused — created, reset, never selected (`qualityMode` switch tops out at 8×). Cosmetic; either delete or expose as Quality 4 = 16×. | `Source/PluginProcessor.cpp:187, 376` |
| **Low:** Monolithic `JuceHeader.h` | 🟡 Unchanged; cosmetic compile-time concern | — |
| **Low:** Multiband band count zipper noise | ✅ Mitigated previously — `updateTargetsFromAPVTS` no longer calls `prepare()` on the audio thread; only sets `numBands` | `Source/PluginProcessor.cpp:311-314` |
| **Build:** Missing X11 dev libs on Linux | ✅ Documented and resolved (see `docs/dev/BTZ_LOCAL_DEV_SETUP.md`) | — |

**Five of six are already fixed or mitigated; one is a 30-second portability fix done in this commit; one (`os16x`) is cosmetic dead-code cleanup.**

The new handoff's "code quality 4.5/5" rating was based on a snapshot before
those fixes; current state would score higher.

---

## 5. Reconciling the Master Handoff's product critique against this branch

| Master Handoff finding | Where it applies | Status on this branch |
|---|---|---|
| Mix does nothing | Codebase #2 (workstation) | Fixed (`sMix.advanceBlock`) on codebase #3 — *verified the wet path runs; Mix law range not yet asserted* — see §7 |
| Master does nothing / no output-gain param | Codebase #2 | Codebase #3 already has a real master gain stage (per-block dB ramp) |
| `static float hpL/hpR` in Air section | Codebase #2 | Not present in codebase #3 |
| `dryL[8192]` stack cap | Codebase #2 | Not present in codebase #3 — `dryBuffer` is a `juce::AudioBuffer` sized in `prepareToPlay`, plus an oversized-block guard |
| Spark is hard-clip, not true peak | Codebase #2 | Codebase #3 has 4× polyphase ISP detection; footer reads dBTP from the limiter, not sample peak |
| UI is placeholder, default state musically broken | Codebase #2 | Codebase #3 has the Ivory editor (synced) and a curated default patch verified to apply audible character |
| Greyed neural slots / dead multiband UI | Codebase #3 | Honesty pass (v1.0.3, this branch) greys these out |
| **Test gap: "passing tests ≠ working plugin"** | **All three** | **Genuine gap on this branch too** — my integration tests verify "wet ≠ dry" but do NOT assert the Mix law across its range. See §7. |

---

## 6. What is open on this branch (the real, current backlog)

### P0 — operational / legal release blockers
1. **JUCE 8 commercial license decision** (GPL vs paid). Cannot legally ship closed-source without it.
2. **Trademark search** on the chosen product name.
3. **EULA** + privacy notice.
4. **Build on macOS + Windows** (scripts ready; never executed on those OSes).
5. **AU build + `auval`** on macOS.
6. **pluginval @ strictness 10** on the patched binary.
7. **DAW smoke tests** in Reaper + ≥1 of Logic/Live/Cubase.
8. **Installer** scripts (`.pkg`, Inno Setup).
9. **Code signing + macOS notarization + Windows Authenticode**.
10. **No support channel / no crash reporter / no user manual.**

### P1 — engineering / honesty
11. **Mix-law, Master-law, per-macro tests** (the gap §7 below). Add to `BTZProcessorCheck`.
12. **AccessibilityHandler** + roles + labels on every interactive control.
13. **Sidechain audio bus** for the glue compressor.
14. **Tempo sync** for Motion / LFO.
15. **HQ-on-render** mode.
16. Decide neural slots: train weights or remove enum values in a v2 schema migration.
17. Decide multiband: wire `split/recombine` in `processNonlinear` or remove the param + combo.
18. Remove unused `os16x` (or expose as 16× quality option).

### P2 — product / UX
19. **First-run tooltip tour** (Drive → Target Lock → A/B → Bloom → Output).
20. **Gain-match factory presets** to a target LUFS.
21. **Right-click menu** (reset / copy-paste value / type value / fine-drag).
22. **Preset browser** upgrade (audition, tags, favorites, "surprise me").
23. **Mastering engineer credibility:** lengthen limiter lookahead beyond 8 samples (or stop marketing as mastering).

### P3 — documentation
24. Split `BTZ_MASTER_GUIDE.md` into Quick Start / User Manual chapters / Installation / Troubleshooting / FAQ.
25. `LICENSE_RISK_AUDIT.md` + dependency license inventory.

---

## 7. The honest test gap on this branch

This is what the Master and Session Handoffs both warned about; both my own
`BTZProcessorCheck` and the existing GoogleTest suite fail to catch it on this
branch:

| Should be tested | Currently tested? |
|---|---|
| Mix = 0.0 → output nulls against dry input | ❌ |
| Mix = 0.5 → output is the expected 50 % blend | ❌ |
| Mix = 1.0 → wet path runs (output ≠ dry) | ✅ (Processor check §1) |
| Master = −6 dB → output is measurably −6 dB | ❌ |
| Master = 0 dB → output is unity gain | ❌ |
| Each macro sweep 0 → 1 produces finite, non-runaway output | ❌ |
| Every visible UI control's APVTS attachment resolves to a real param | ❌ (was a v11 disaster class; would catch if attachment IDs drift again) |

**Recommendation:** before any more product work, add a **product-behavior
test pass** to `BTZProcessorCheck` covering the seven rows above. It is the
single highest-leverage test work in the project; both handoffs converge on
this point and they are right.

---

## 8. Which prior docs are still authoritative, and which are superseded

### Still authoritative (use these)
- `docs/BTZ_CURRENT_BASELINE_LOCK.md` — "must not regress" contract.
- `docs/dev/PARAMETER_MANIFEST.md` — every parameter on codebase #3 (ABI contract).
- `docs/dev/BTZ_PERF_SECURITY_AUDIT.md` — measured CPU, ASan/UBSan results, security review.
- `docs/dev/BTZ_RELEASE_BLUEPRINT_AUDIT.md` — 14-phase release-readiness compliance.
- `docs/product/BTZ_PRODUCT_IDENTITY.md` — Target Lock as flagship; feature hierarchy.
- `docs/release/BTZ_FORMAT_ROADMAP.md` — VST3 first; CLAP next; AAX deferred.
- `docs/release/BTZ_RELEASE_READINESS_SCORECARD.md` — measured 1–10 scores.
- `BTZ_CHANGELOG.md` — committed history.

### Superseded by this document
- The Master Handoff's "two-product (A/B)" framing — **replace with the three-implementation framing in §1**.
- The Session Handoff's "v12 code-quality issues" list — **most are already fixed (see §4)**.
- Any prior claim that BTZ is "ready to ship" — it is engineering-ready but operationally and legally unprepared (see §6 P0 list).
- Any prior claim that BTZ is "broken in Reaper" without qualification — that report is about codebase #2, not codebase #3 on this branch.

### Quarantine (keep for history, don't act on)
- The skill-pack's "Known achieved baseline" claim list (the Master Handoff flagged this; both handoffs agree it merges products and overstates status).
- The Rust DSP reference files in the skill-pack zip (use as theory only — they're not C++/JUCE).

---

## 9. Decisions needed from you (gating, ranked)

1. **Which codebase ships v1.0?**
   - **Recommendation:** codebase #3 (this branch). It's verified, sanitized, and has the flagship feature legible. Codebase #2 has known critical bugs in Reaper that aren't yet fixed there; codebase #1 is out of repo.
2. **Single canonical name.** "Box Tone Zone," "Sonic Alchemy," or one new name. Affects binary, CMake target, brand, docs.
3. **JUCE license tier.** Cannot ship closed-source without this.
4. **Multiband + neural:** wire, remove, or keep greyed-and-documented for v1.1?
5. **Mastering positioning:** lengthen limiter lookahead and re-validate, or stop marketing as a mastering plugin?
6. **Distribution path:** direct / Plugin Boutique / Plugin Alliance / Splice?

Until those decisions land, additional code work risks being thrown away or
pointed at the wrong product.

---

## 10. What I'd do next (if you green-light it)

In order, smallest-blast-radius first:

1. **Add the seven Mix/Master/macro behavior tests** to `BTZProcessorCheck` (§7). Verified in this environment; no DAW needed. ~30 minutes.
2. **Delete `os16x`** (or expose as a real 16× quality option). ~5 minutes.
3. **Add `AccessibilityHandler`** to every knob and toggle. Mechanical, verifiable by compilation; full screen-reader verification needs a real OS. ~1 hour.
4. **Author `LICENSE_RISK_AUDIT.md`** with dependency license inventory + a checklist for legal counsel. ~1 hour.
5. **Stop and hand off to a real machine** for items #4–9 in the P0 list (macOS/Windows builds, pluginval, DAW smoke). Those cannot be done here.

---

## 11. What this document does not pretend to know

- Whether codebase #2 on the Windows workstation has had any of its known bugs fixed since the Master Handoff was written. I cannot see that workstation's source.
- Whether anyone has run pluginval against any of the three codebases on a real CI machine since this branch was patched.
- Whether the chosen product name has been trademark-cleared in any jurisdiction.
- Whether the Cursor repair packets referenced in the Session Handoff have been applied to either codebase.
- How many of the 462 files changed across branches are now stale.

Where these are unknown, this document says so rather than asserting.

---

## How to update this document

Treat this as a living rolling-truth file. When **any** of the following
changes, update §3 (the verified-status table) and §6 (the open backlog), then
amend `BTZ_CHANGELOG.md`:

- A new tag is cut.
- A new test class is added to `BTZProcessorCheck`.
- A real DAW or pluginval run produces results.
- A P0 / P1 item in §6 is closed.
- A canonical decision in §9 is made.

Do **not** create a new handoff document instead of updating this one. The
proliferation of overlapping handoffs is most of why this section exists.
