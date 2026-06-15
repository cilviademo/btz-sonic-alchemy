# BTZ — Format Roadmap (VST3 / CLAP / AAX)

Ship order, gates, and the honest reasons behind them.

## v1.0 — VST3 + AU + Standalone (primary)
**Status:** built and integration-tested on Linux; macOS/Windows build scripts
ready (`scripts/build_macos.sh`, `scripts/build_windows.ps1`).

**Release gates:**
- pluginval @ strictness 10 with `--repeat-count 2 --randomise` (already gating
  in CI, no `|| true`).
- Apple `auval -strict -v aufx Btz1 BTZa` on macOS.
- DAW smoke pass: Reaper + at least one of Logic / Live / Cubase / Bitwig /
  Studio One — load, edit, save, reopen, automate, scrub.
- macOS notarization + hardened runtime; Windows Authenticode signing.

**Why VST3 first:** widest DAW coverage today, Steinberg SDK is mature,
pluginval is the gold-standard validator we already use.

## v1.1 — CLAP
**Status:** scaffolding present in CMake (`BTZ_BUILD_CLAP=ON` fetches
`clap-juce-extensions`); disabled in CI for now.

**Gates before declaring CLAP supported:**
- Pin `clap-juce-extensions` to a tagged release (currently `main` — a moving
  target, deferred until pinned for reproducibility).
- License review of `clap-juce-extensions` (MIT) alongside the JUCE license
  decision.
- Host validation on Bitwig (the reference CLAP host) and Reaper's CLAP path.
- Parameter ID + state-version compatibility verified across VST3 ↔ CLAP loads
  of the same session (CLAP and VST3 use the same APVTS state, but the wrapper
  layer needs round-trip testing).

**No CLAP marketing claim until those gates are green.**

## v1.2+ — AAX (later, only if the business case is clear)
**Status:** intentionally not started. AAX has real cost.

**What it requires (so it's not promised lightly):**
- Avid Developer agreement and AAX SDK access (NDA).
- PACE / iLok integration (commercial license; recurring cost).
- Pro Tools-specific validation; Avid signing.
- Separate code-signing pipeline.
- Marketing/distribution arrangement through Avid's store or direct.

**Recommendation:** ship and stabilize VST3/AU/CLAP. Only commit to AAX once
there is paying-customer demand from a Pro Tools audience.

## Linux
Build target works (Standalone + VST3 link in CI). LV2 not in scope for v1.0.

## What this roadmap does *not* promise
- No commitment to a release date for any format above.
- No CLAP claim before the four gates above are green.
- No AAX timeline until commercial review.
- No mobile / web / hardware port.
