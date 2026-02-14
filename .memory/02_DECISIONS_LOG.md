# Decisions Log

## 2026-02-13
Decision:
- Switch BTZ build target from plain shared library to JUCE plugin target using `juce_add_plugin(... FORMATS VST3 ...)`.

Reason:
- A renamed shared library is not a valid JUCE VST3 packaging flow.

Tradeoffs:
- Requires JUCE-specific CMake conventions.
- Build output paths differ from previous assumptions.

Impact:
- Produces proper VST3 bundle structure and host-compatible artifact.

Reversal Strategy:
- Revert `BTZ/CMakeLists.txt` to prior `add_library` approach (not recommended for host compatibility).

---

## 2026-02-13
Decision:
- Keep JUCE source path explicit in CMake: `C:/Users/marcm/OneDrive/Desktop/JUCE-8.0.6`.

Reason:
- User environment did not have package-installed JUCE config and used zip source.

Tradeoffs:
- Path is machine-specific.

Impact:
- Deterministic local setup for current machine.

Reversal Strategy:
- Replace with `find_package(JUCE CONFIG REQUIRED)` + proper `JUCE_DIR` or repo submodule strategy.

---

## 2026-02-13
Decision:
- Standardize Windows build/install flow through scripts in `scripts/`.

Reason:
- Manual command drift caused repeated environment and path errors.

Tradeoffs:
- Additional script maintenance.

Impact:
- One-step build/install reliability improved.

Reversal Strategy:
- Return to manual CLI workflow and remove scripts.

---

## 2026-02-13
Decision:
- Force clean build + x64 toolchain in `build_windows_with_vs_env.bat`.

Reason:
- Encountered x86/x64 linker conflicts in `juce_vst3_helper.exe`.

Tradeoffs:
- Build directory wiped each run by this script.

Impact:
- Prevents stale cache/toolchain mix.

Reversal Strategy:
- Make clean optional and preserve incremental builds.

---

## 2026-02-13
Decision:
- Add automated memory sync tooling (`update_memory.ps1`, watcher, and installer script).

Reason:
- User requested memory to auto-update and reduce manual maintenance/context drift.

Tradeoffs:
- Auto-generated entries may require human cleanup/refinement.
- Live watcher adds background process overhead.

Impact:
- `.memory` can now be updated automatically after edits and optionally after commits.

Reversal Strategy:
- Disable watcher/hook and return to manual updates only.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 239 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 8 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 10 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 16 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 159 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.

## 2026-02-13 [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- 157 file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.
