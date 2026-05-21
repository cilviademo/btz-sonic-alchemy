# BTZ — CI Notes

CI workflow: `btz-sonic-alchemy-main/.github/workflows/ci.yml`.

## Current state (after this session's edits)
- Triggers on `main`, `baseline/**`, `overhaul/**`, `feature/**`, `claude/**` push + PRs to `main`.
- Builds on **macos-14**, **windows-latest**, **ubuntu-22.04**; tests via `ctest`; uploads artifacts.
- pluginval (macOS job) now **gates**: strictness 10, `--repeat-count 2 --randomise`,
  and the `|| true` bypass was removed.

## Known issues / recommended fixes (safe to apply locally or via Codex)
1. **It will now fail the build until the macOS/Windows builds are green.** That
   is intended (the build was previously red and hidden). First make the local
   builds pass, then let CI enforce it.
2. **Artifact upload paths.** Globs include a `${BUILD_TYPE}` (`Release/`) segment:
   `BTZ/build/BTZ_artefacts/Release/VST3/`. For single-config Ninja (macOS/Linux)
   with `-DCMAKE_BUILD_TYPE=Release`, JUCE writes to `BTZ_artefacts/Release/...`
   (the `$<CONFIG>` generator expression resolves to `Release`), so the paths are
   likely correct — **verify on a real CI run** and adjust if artifacts upload empty.
3. **Linux `moduleinfo.json`.** The VST3 post-link `juce_vst3_helper` step can fail
   on headless Linux runners. If the ubuntu job fails only there, either build the
   Standalone target on Linux (validation-only) or run the helper under a working
   shell. macOS/Windows are unaffected.
4. **clap-juce-extensions is pinned to `main`** in `CMakeLists.txt` (a moving
   target). CI disables CLAP, so CI is unaffected, but pin to a tag/SHA before
   shipping CLAP for reproducibility.
5. **Dependency caching.** Add `actions/cache` for the CMake `build/_deps` (JUCE +
   GoogleTest clones) to cut CI time.
6. **Upload build logs** on failure (`if: failure()`) to speed triage.
7. Consider a `workflow_dispatch` input to select pluginval strictness (5/8/10) for
   manual escalation runs.

## Linux runner dependency list (already in ci.yml)
`libasound2-dev libjack-jackd2-dev libcurl4-openssl-dev libfreetype6-dev
libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev
libxrandr-dev libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev
mesa-common-dev` — note `webkit2gtk-4.0` may be unavailable on newer Ubuntu
(noble ships 4.1); BTZ sets `JUCE_WEB_BROWSER=0` so webkit/gtk are not strictly
required for the build.
