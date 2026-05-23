# BTZ — Release Readiness Scorecard

Honest, measured scores (1–10). "Current" is verified at HEAD; "v1.0 target"
is what must be true before tagging RC1.

| Category | Current | v1.0 Target | Blocker(s) | Owner |
|---|---|---|---|---|
| DSP Quality | **9** | 9 | Neural weights (or stop calling them Neural) | Engineering |
| Sound Identity | **8** | 9 | Default patch ✓; preset library audition pass needed | Product |
| CPU Performance | **9** | 9 | Worst-case 3.73 % single core; no action | — |
| Stability | **8** | 9 | DAW beta testing across hosts | QA |
| UI Maturity | **8** | 9 | Real units ✓; visualizers wired ✓; accessibility next | UI/UX |
| Onboarding | **7** | 9 | First-run tooltip tour; default patch ✓ | UX |
| Accessibility | **5** | 8 | `AccessibilityHandler` + roles missing | UI/UX |
| Metering Honesty | **9** | 9 | True-peak now real ISP; LUFS K-weighted; no claim drift | — |
| Preset Readiness | **7** | 9 | 16 factory presets shipped; need gain-match + audition | Product |
| Documentation | **8** | 9 | Manual chapters split + Quick Start needed | Tech Writer |
| Licensing Readiness | **2** | 9 | JUCE 8 GPL-vs-commercial; trademark search | Legal |
| CI Reliability | **8** | 9 | Caching + log upload + real-runner verification | DevOps |
| DAW Compatibility | **5** | 9 | No DAW smoke tests yet (needs real machine) | QA |
| Market Differentiation | **9** | 9 | Target Lock readout is now visible | — |

## How current scores were derived
- "9/10" = measured/verified at HEAD (CPU, sanitizers, tests, metering).
- "8/10" = present and correct but one polish item away.
- "5–7" = scaffolding present, real-world validation or feature work outstanding.
- "2" = legally blocking; nothing engineering can change alone.

## Top 5 release blockers (must close before RC1)
1. **JUCE license** decision (commercial purchase or GPL distribution). Until
   then we cannot legally ship a closed-source binary.
2. **pluginval @10** run on the patched binary (Linux pluginval install OR
   macOS/Windows CI green run).
3. **DAW smoke tests** in Reaper + ≥ 1 of Logic / Live / Cubase.
4. **Accessibility pass** — `AccessibilityHandler` + roles + labels.
5. **Trademark search** on "Box Tone Zone" / "Sonic Alchemy" / "BTZ".

## Top 5 next-highest-ROI actions (after blockers)
1. **First-run tooltip tour** (Drive → Target Lock → A/B → Bloom → Output).
2. **Gain-match the factory presets** to a target LUFS so preset auditioning is honest.
3. **Real per-band multiband** — wire `split/recombine` in `processNonlinear`, OR keep the
   honesty-pass greying.
4. **External sidechain audio bus** (the Glue compressor's missing capability).
5. **Either** train distinct neural saturation weights, **or** remove the four enum
   values entirely in a v2 schema with state migration.

## What is fully verified (citations)
- Compile + link: BTZ_Standalone + BTZTests + BTZProcessorCheck artefacts.
- Unit tests: 86/86 — `./build/BTZTests_artefacts/Release/BTZTests`.
- Integration: 10/10 — `./build/BTZProcessorCheck_artefacts/Release/BTZProcessorCheck`.
- Sanitizers: 0 findings — `build-san/` (ASan+UBSan+LSan).
- CPU: see `docs/dev/BTZ_PERF_SECURITY_AUDIT.md`.

## What still requires a real machine / DAW
- macOS Universal binary; AU validation (`auval`); Apple Silicon CPU numbers.
- Windows VST3 in Cubase / Studio One / FL.
- pluginval on the patched binary at strictness 10.
- Accessibility verified against VoiceOver / Narrator.
- HiDPI / multi-display visual scaling.
- Latency reporting verified in a real host's PDC (`setLatencySamples`).
