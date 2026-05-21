# BTZ — Cursor Handoff Guide

You (Cursor, on the real local machine) are taking over a JUCE/C++ audio plugin
that **now compiles** and passes 81/84 unit tests on Linux. Your job: build it on
macOS/Windows, validate it, load it in a DAW, and fix any real runtime issues —
**in patch mode**.

## Recommended opening prompt
> "Read docs/dev/BTZ_CURSOR_HANDOFF.md, SENIOR_DEV_AUDIT_v1.0.1.md, and
> docs/BTZ_AI_WORKFLOW.md. Work in patch mode only. Configure, build, debug
> compile errors, run pluginval, load in Reaper, and document all results. Do
> not rewrite architecture."

## Operating rules (hard constraints)
- **Patch mode only.** No architecture rewrites, no DSP topology changes, no UI redesign.
- Preserve the **Ivory System** visual language (`BTZTheme.h` palette/type are the source of truth).
- Preserve **parameter IDs** (they are a public ABI — renaming breaks saved sessions/presets).
- Preserve **preset compatibility** and the state-version migration path.
- Show diffs before any non-trivial edit. After each fix: **rebuild + retest**.
- Update `docs/BTZ_CHANGELOG.md` and `docs/dev/BTZ_BUILD_AND_TEST_STATUS.md` as you go.

## Step-by-step
1. **Open the repo**; confirm branch (`claude/review-btz-compilation-aN5UD`) and HEAD commit (`git log -1`).
2. **Install deps** — see `BTZ_LOCAL_DEV_SETUP.md` for your OS.
3. **Configure CMake**: `scripts/build_macos.sh Release --tests` (or `build_windows.ps1`).
4. **Build** plugin + tests. Fix any platform-specific compile errors surgically
   (most likely candidates: macOS/Windows-only JUCE API differences). The Linux
   build is already clean, so editor/theme/DSP should compile everywhere.
5. **Run unit tests**: `ctest --test-dir btz-sonic-alchemy-main/BTZ/build --output-on-failure`.
   Expect 81/84. The 3 known failures are documented design questions (see
   BUILD_AND_TEST_STATUS §2) — decide with the user, don't blindly "fix".
6. **Run pluginval**: `scripts/run_pluginval.sh 10` (or `.ps1`). Triage any failures.
7. **Open in Reaper** (and ideally a second host). Verify:
   - plugin loads, editor opens, no crash
   - Simple / Standard / Advanced views render; Ivory theme intact; no overlapping controls
   - audio passes; bypass is click-free; A/B toggle is glitch-free
   - parameters automate from the host; knobs update
   - meters + GR ribbon update; correlation indicator responds to a mono vs. wide signal
   - preset switching is stable; close/reopen the editor repeatedly (no leak/crash)
   - no CPU spikes on silence (denormals) or at high drive
8. **Decide the 3 documented test failures** with the user (LR4 reconstruction
   semantics ×2, SafetyLayer clamp policy).
9. **Push fixes to a new branch** (e.g., `cursor/local-build-fixes`) and open a PR;
   keep `claude/review-btz-compilation-aN5UD` as the reference baseline.

## High-value follow-ups (still open, see the shipping brief)
- Visualizer data flow: spectrum/harmonic displays are not yet fed live FFT data
  (they render empty). Wire a timer-thread FFT read of `SpectrumBuffer` to
  `SpectrumDisplay::setSpectrum` / `HarmonicVisualizer::setMagnitudes`. Keep it
  off the audio thread (the buffer is single-writer audio / single-reader UI).
- Factory presets (none ship yet) + optional `BinaryData` embedding.
- Neural model weights (4 slots fall back to tanh) — train or hide the slots.
- Installers + code signing/notarization (infrastructure; needs real certs).
