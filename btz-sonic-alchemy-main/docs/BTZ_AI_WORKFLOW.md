# BTZ Sonic Alchemy — AI Agent Workflow Guide

> Reference version: **v1.0 Baseline**  
> Last updated: 2026-05-21

This document provides mandatory instructions for any AI agent (Claude, Codex, Cursor, Manus, or other) that is tasked with modifying, reviewing, or extending the BTZ Sonic Alchemy codebase.

---

## Before You Start

Read these documents in order before making any changes:

1. **This file** (`BTZ_AI_WORKFLOW.md`) — rules and constraints
2. **[BTZ_V1_BASELINE_ARCHIVE.md](./BTZ_V1_BASELINE_ARCHIVE.md)** — what has been built and why
3. **[BTZ_MASTER_ARCHITECTURE.md](./BTZ_MASTER_ARCHITECTURE.md)** — how the system works
4. **[BTZ_CHANGELOG.md](./BTZ_CHANGELOG.md)** — what has changed recently

If you skip these documents and make changes that violate the baseline, your work will be reverted.

---

## Core Rules

### Rule 1: Preserve the Baseline

The v1.0 baseline represents 12 iterations of accumulated work. Do not delete features, rewrite architecture, or simplify systems unless explicitly instructed by the user. The following are locked:

- DSP signal flow order
- Parameter string IDs (in `btz::id::`)
- State version migration compatibility
- 3-mode UI architecture (Simple / Standard / Advanced)
- Ivory System visual identity
- Single-header DSP library pattern
- Lock-free audio thread guarantee

### Rule 2: Use Patch Mode

Make surgical, targeted edits. Do not rewrite entire files unless the user explicitly requests a rewrite. Prefer `edit` operations over `write` operations. When fixing a bug, change only the lines that are broken.

### Rule 3: Document Everything

After every meaningful change:

1. Update `docs/BTZ_CHANGELOG.md` with the change description
2. Use conventional commit messages (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `chore:`)
3. If you add a new DSP module, add a corresponding test
4. If you change a parameter ID, add state migration code

### Rule 4: Respect Real-Time Safety

The audio thread (`processBlock`) must never:

- Allocate memory (no `new`, no `std::vector::push_back`, no `juce::String` construction)
- Acquire locks (no `std::mutex`, no `juce::CriticalSection`)
- Block (no file I/O, no network, no `sleep`)
- Call virtual functions on objects that might not exist

Use `std::atomic` for all cross-thread communication. Use pre-allocated buffers sized in `prepareToPlay`.

### Rule 5: Respect Preset Compatibility

If you change the state format:

1. Increment `kStateVersion` in `BTZDsp.h`
2. Add migration logic in `PluginProcessor::setStateInformation()` that handles the previous version
3. Never rename a parameter ID without migration — this breaks all saved sessions

### Rule 6: Test Before Pushing

If you modify any DSP module:

1. Verify the corresponding test in `test_dsp_modules.cpp` still matches the API
2. If you added a method, add a test for it
3. If you changed behavior, update the test assertions

---

## Workflow for Common Tasks

### Fixing a Compile Error

1. Read the error message carefully
2. Locate the exact file and line
3. Make the minimal fix (add missing include, fix type mismatch, etc.)
4. Do NOT refactor surrounding code while fixing the error
5. Commit with `fix:` prefix

### Adding a New Feature

1. Discuss scope with the user first
2. Add the DSP module to `BTZDsp.h` (if needed)
3. Add the parameter to `btz::id::` and `createParameterLayout()`
4. Wire it into the appropriate signal chain stage
5. Add UI controls in the appropriate view mode
6. Add unit tests
7. Update `BTZ_CHANGELOG.md`
8. Commit with `feat:` prefix

### Changing the UI

1. All visual changes go through `BTZTheme.h` tokens — never hardcode colors or sizes
2. Component changes go in `BTZComponents.h`
3. Layout changes go in `PluginEditor.cpp`
4. Do NOT change the 3-mode architecture without user approval
5. Do NOT change the Ivory System palette without user approval

### Reviewing Code

1. Focus on: compile correctness, RT-safety, API consistency, test coverage
2. Produce findings as a Markdown document (not inline comments)
3. Categorize issues by severity: Critical (won't compile), High (crashes), Medium (incorrect behavior), Low (style/optimization)
4. Do NOT auto-fix issues during a review — report them and wait for approval

---

## File Ownership

| File | Owner | Change Policy |
|------|-------|---------------|
| `BTZDsp.h` | DSP team | Requires test update |
| `BTZTheme.h` | Design team | Token changes only |
| `BTZComponents.h` | UI team | Must match theme tokens |
| `PluginProcessor.cpp` | DSP team | RT-safety audit required |
| `PluginEditor.cpp` | UI team | Must respect 3-mode layout |
| `CMakeLists.txt` | Build team | Minimal changes preferred |
| `test_dsp_modules.cpp` | QA team | Must match DSP API |
| `docs/*` | Any | Update after changes |

---

## Commit Message Format

```
type(scope): short description

Longer explanation if needed.

Refs: #issue-number (if applicable)
```

Types: `feat`, `fix`, `docs`, `test`, `refactor`, `chore`, `build`, `ci`

Scopes: `dsp`, `ui`, `theme`, `build`, `test`, `docs`, `state`

Examples:
- `fix(dsp): correct ShineProcessor coefficient calculation`
- `feat(ui): add spectrum analyzer to Advanced mode`
- `docs: update changelog for v1.0.1`
- `test(dsp): add GlueCompressor ratio test`

---

## Version Numbering

| Version | Scope |
|---------|-------|
| v1.0.x | Bug fixes, cleanup, validation, small refinements |
| v1.1.x | Meaningful feature additions or UI/DSP enhancements |
| v1.2.x | Larger subsystem upgrades |
| v2.0 | Major architecture-breaking redesigns (requires user approval) |

Increment the patch version for every meaningful commit. Increment minor for feature additions. Major version changes require explicit user authorization.

---

## Emergency Procedures

If you break the build:
1. Do NOT make more changes hoping to fix it
2. Check `git diff` to see what you changed
3. Revert the specific broken change
4. Try again with a more targeted fix

If you are unsure about a change:
1. Ask the user before proceeding
2. Explain what you want to change and why
3. Wait for approval

If you encounter a design decision that contradicts this document:
1. This document takes precedence
2. If you believe this document is wrong, flag it to the user
3. Do NOT silently override these rules

---

## Quick Reference Card

```
READ FIRST:     docs/BTZ_AI_WORKFLOW.md (this file)
ARCHITECTURE:   docs/BTZ_MASTER_ARCHITECTURE.md
HISTORY:        docs/BTZ_V1_BASELINE_ARCHIVE.md
CHANGELOG:      docs/BTZ_CHANGELOG.md
DSP CODE:       BTZ/Source/BTZDsp.h
UI TOKENS:      BTZ/Source/BTZTheme.h
UI COMPONENTS:  BTZ/Source/BTZComponents.h
PROCESSOR:      BTZ/Source/PluginProcessor.cpp
EDITOR:         BTZ/Source/PluginEditor.cpp
TESTS:          BTZ/tests/test_dsp_modules.cpp
BUILD:          BTZ/CMakeLists.txt
CI:             .github/workflows/ci.yml
```
