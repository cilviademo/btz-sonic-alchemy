# BTZ — Codex Handoff Guide

Use Codex for **deterministic tooling and small, verifiable fixes** — not for
design or architecture.

## Recommended prompt
> "Operate in patch mode. Focus only on deterministic build/test tooling and
> small compile fixes. Do not remove systems, rename parameters, or alter DSP
> topology. Generate scripts and CI improvements that can be verified locally."

## Good Codex tasks
- Build/validation helper scripts (extend `scripts/` — installers, signing wrappers).
- CI YAML cleanup (`.github/workflows/ci.yml`) — caching, matrix, artifact paths.
- Test harness plumbing (e.g., a processor-level smoke test, golden-file
  regression harness) — see CI notes.
- Changelog formatting (`docs/BTZ_CHANGELOG.md`, Keep-a-Changelog style).
- Static-analysis configs (clang-tidy/cppcheck), `.clang-format`.
- Small, mechanical compile fixes (missing includes, obvious type mismatches)
  that a local build flags — always rebuild to verify.

## Off-limits for Codex
- DSP topology changes (limiter design, crossover reconstruction, saturation models).
- UI redesign or theme changes (Ivory System is fixed).
- Parameter renaming/removal (ABI + preset compatibility).
- Product strategy, neural-model design, broad refactors.

## Concrete starter tasks
1. macOS `.pkg` installer (`pkgbuild`/`productbuild`) wrapper script + a Windows
   Inno Setup `.iss` script. Leave cert IDs as documented placeholders.
2. CI: fix macOS/Linux artifact upload globs, add dependency caching, add a
   `workflow_dispatch` strictness input for pluginval.
3. Add a minimal `tests/test_processor.cpp` that instantiates `BTZAudioProcessor`,
   calls `prepareToPlay`/`processBlock`/`getStateInformation`/`setStateInformation`
   at 44.1/48/96 kHz with varying block sizes (the class pluginval exercises).
