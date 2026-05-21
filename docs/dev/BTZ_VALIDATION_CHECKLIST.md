# BTZ — Validation Checklist

Tick these before tagging a release. Items marked ✅ were verified in the Linux
patch environment; the rest require a real machine / DAW.

## Compile validation
- [✅] Clean CMake configure (JUCE 8.0.6 fetched)
- [✅] Clean plugin build (VST3 + Standalone) — zero source errors
- [✅] Test target builds and links
- [ ] macOS build (VST3 + AU + Standalone), incl. universal arm64+x86_64
- [ ] Windows build (VST3 + Standalone)
- [ ] No new compiler warnings introduced (review `juce_recommended_warning_flags`)
- [ ] VST3 `moduleinfo.json` generated (macOS/Windows; fails only on headless Linux)

## Unit tests
- [✅] 81 / 84 GoogleTests pass
- [ ] Resolve/triage the 3 documented failures (LR4 reconstruction ×2, SafetyLayer clamp)
- [ ] Add a processor-level smoke test (prepare/process/state at multiple SR + block sizes)

## pluginval
- [ ] strictness 5 — pass (smoke)
- [ ] strictness 8 — pass (target)
- [ ] strictness 10 — pass (release gate; CI runs this, no `|| true` bypass)
- [ ] `--repeat-count 2 --randomise` — pass (state-restore / automation races)

## DAW validation (Reaper + at least one of FL/Live/Cubase/Logic)
- [ ] Plugin scans and loads
- [ ] Editor opens; Simple/Standard/Advanced render; Ivory theme intact; no overlap
- [ ] Audio passes; bypass is click-free
- [ ] Parameters automate from host; knobs reflect automation
- [ ] Preset switching stable; close/reopen editor repeatedly — no crash/leak
- [ ] Save session → reload → state restored bit-identically
- [ ] No CPU spikes on silence (denormals) or sustained high drive

## DSP validation
- [ ] Silence in → silence out (no self-noise)
- [ ] Sine sweep — no unexpected artifacts; oversampling toggle changes nothing audibly at unity
- [ ] Bypass null test (dry vs. bypassed wet cancels)
- [ ] True-peak: feed a known inter-sample-peak signal; output respects the ceiling
- [ ] LUFS sanity: −23 LUFS pink noise reads ≈ −23 (K-weighted)
- [ ] Correlation sanity: mono signal → +1; out-of-phase → negative (UI warns)
- [ ] THD at moderate drive within expected range (publish the curve)

## UI validation
- [ ] Simple / Standard / Advanced load and lay out at min, default, and max window sizes
- [ ] Meters + GR ribbon update; correlation indicator responds
- [ ] Spectrum / harmonic visualizers fed live data (currently a known gap — wire first)
- [ ] Window scaling / HiDPI looks correct
- [ ] All controls have tooltips

## Release packaging
- [ ] macOS `.pkg` installs VST3/AU/Standalone to correct dirs
- [ ] Windows installer (Inno Setup/NSIS) installs VST3/Standalone
- [ ] macOS code-signed + notarized; Windows Authenticode-signed
- [ ] Factory presets present (and embedded via BinaryData if chosen)
