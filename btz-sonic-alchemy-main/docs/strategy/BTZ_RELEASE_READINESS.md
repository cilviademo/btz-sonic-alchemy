# BTZ Sonic Alchemy — Release Readiness Scorecard

## 1. Current Readiness Scores (Out of 10)

| Category | Score | Notes |
|----------|-------|-------|
| **DSP Quality** | 9/10 | Mathematically correct, RT-safe, low CPU. Needs neural weights. |
| **Stability** | 8/10 | Sanitizer-clean, but needs widespread beta testing across DAWs. |
| **UX Maturity** | 7/10 | Ivory System is beautiful, but lacks typed entry and tooltips. |
| **Visual Identity** | 8/10 | Strong palette, needs the signature Harmonic Bloom animation. |
| **Onboarding** | 4/10 | Transparent default state breaks the "instant good" expectation. |
| **Trust** | 6/10 | Needs published measurements and loudness-matched A/B. |
| **Accessibility** | 5/10 | Needs keyboard focus and colorblind safety checks. |
| **CI / Testing** | 9/10 | 87+ tests, multi-platform GitHub Actions pipeline is excellent. |
| **Documentation** | 8/10 | Manuals and architecture docs are solid. |
| **Market Differentiation** | 9/10 | Target Lock is a killer feature. |
| **Legal Readiness** | 2/10 | Needs JUCE license, EULA, and trademark checks. |
| **Scalability** | 8/10 | Codebase is modular and ready for expansion. |

## 2. Risk Assessment

**Top Risks:**
1. **The "Kitchen Sink" Perception:** If LFOs and Mid/Side are pushed too hard in marketing, users will think it's a generic multi-effect rather than a precision loudness/saturation tool.
2. **Neural Model Stubs:** If the plugin ships without actual neural weights (or highly convincing mathematical approximations), the "Neural" slots will sound identical to the standard slots, destroying credibility.

**Hidden Risks:**
1. **DAW Compatibility:** The plugin compiles, but has it been tested in Pro Tools (AAX), Logic (AU), Ableton (AU), and FL Studio (VST3)? Each DAW handles APVTS state and latency reporting slightly differently.
2. **Apple Silicon:** SSE intrinsics in `BTZDsp.h` will break the ARM64 build unless properly guarded and translated to NEON.

**Unknown Unknowns:**
1. How does the Target Lock engine behave when fed extreme, heavily clipped EDM audio vs highly dynamic classical music? (Needs stress testing).

## 3. Highest ROI Next Actions

1. **Fix the Default State:** Change the initialization to a "Subtle Mix Glue" preset with Target Lock engaged. (1 hour of work, massive UX improvement).
2. **Implement Loudness-Matched Bypass:** Prove to the user that the plugin sounds better, not just louder. (4 hours of DSP work).
3. **Train/Stub Neural Models:** Fill the 4 neural slots with distinct harmonic profiles. (1-2 days of work).
4. **Build Installers:** Create the macOS `.pkg` and Windows `.iss` scripts. (1 day of work).
