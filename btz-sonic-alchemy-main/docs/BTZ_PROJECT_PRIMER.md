# BTZ Sonic Alchemy — Project Primer

## What Is This?

**Box Tone Zone (BTZ)** is a premium saturation, dynamics, and tone-shaping audio plugin built with JUCE 8 and C++17. It targets professional music producers, mixing engineers, and sound designers who want characterful harmonic processing with modern workflow conveniences.

The plugin ships as VST3, AU, Standalone, and CLAP formats across macOS, Windows, and Linux.

## Current State

This repository is at **v1.0 Baseline** — the consolidated production foundation after 12 development iterations. The codebase is architecturally complete but has not yet achieved a successful first compile. The immediate next step is building the plugin and resolving any remaining type errors.

For the full history of how we arrived here, see [BTZ_V1_BASELINE_ARCHIVE.md](./BTZ_V1_BASELINE_ARCHIVE.md).

## Repository Layout

The repository root contains a JUCE/C++ plugin project alongside legacy web prototype files (React/TypeScript) that were used for early UI exploration. The production plugin lives entirely within the `BTZ/` directory.

| Path | Purpose |
|------|---------|
| `BTZ/Source/` | All plugin source code (7 files) |
| `BTZ/tests/` | GoogleTest unit tests |
| `BTZ/CMakeLists.txt` | Build system (auto-fetches JUCE) |
| `BTZ/README.md` | Plugin-specific documentation |
| `docs/` | Project documentation suite |
| `.github/workflows/` | CI/CD pipeline |
| `src/`, `public/`, `index.html` | Legacy web prototype (not part of plugin) |

## Key Source Files

The entire plugin is implemented in 7 files within `BTZ/Source/`:

| File | Role | Lines |
|------|------|-------|
| `BTZDsp.h` | Single-header DSP library — all processing modules | ~1300 |
| `BTZTheme.h` | Ivory System design tokens (colors, fonts, spacing, IDs) | ~200 |
| `BTZComponents.h` | Custom JUCE UI components (knobs, meters, visualizer) | ~600 |
| `PluginProcessor.h` | AudioProcessor class declaration | ~120 |
| `PluginProcessor.cpp` | Signal chain, state serialization, parameter layout | ~970 |
| `PluginEditor.h` | Editor class declaration | ~100 |
| `PluginEditor.cpp` | 3-mode GUI layout and interaction | ~650 |

## How to Build

```bash
cd BTZ
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

CMake will automatically fetch JUCE 8.0.6 if it is not found locally. Set `JUCE_DIR` to use a local copy.

## How to Run Tests

```bash
cd BTZ
cmake -B build -DBTZ_BUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## Design Philosophy

The plugin follows three core principles:

**Progressive Disclosure** — beginners see 3 knobs, professionals see the full toolkit. No one is overwhelmed or limited.

**Sonic Character Over Transparency** — BTZ is not a clean utility. Every saturation model has a distinct personality. The goal is musical color, not surgical precision.

**Premium Aesthetic** — the Ivory System visual language communicates quality through restraint. Warm tones, ceramic textures, and generous whitespace signal a tool worth paying for.

## Related Documentation

| Document | Purpose |
|----------|---------|
| [BTZ_V1_BASELINE_ARCHIVE.md](./BTZ_V1_BASELINE_ARCHIVE.md) | Full V1-V12 history and baseline definition |
| [BTZ_MASTER_ARCHITECTURE.md](./BTZ_MASTER_ARCHITECTURE.md) | Technical architecture deep-dive |
| [BTZ_AI_WORKFLOW.md](./BTZ_AI_WORKFLOW.md) | Instructions for AI agents working on this project |
| [BTZ_CHANGELOG.md](./BTZ_CHANGELOG.md) | Version history and change log |
| [BTZ/README.md](../BTZ/README.md) | Plugin-specific build and feature documentation |

## Contact

This is a proprietary project by BTZ Audio. All rights reserved.
