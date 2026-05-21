# Box Tone Zone (BTZ) — v1.2.0 Ivory System

A premium saturation, dynamics, and tone-shaping plugin built with JUCE 8.

## Overview

Box Tone Zone is a multiband saturation and dynamics processor featuring a refined **Ivory System** design language — warm ceramic knobs, sage/terracotta accents, and a 3-mode progressive disclosure interface.

### Feature Set

- **9 Saturation Models**: Tanh, Tube, Tape, Transistor, Transformer, plus 4 Neural slots (Neve, API, SSL, Custom) powered by real-time GRU inference
- **Wave Digital Filter Models**: Physically-modeled tube and transformer stages
- **Dynamic Resonance Taming**: 32-band spectral peak suppression before saturation
- **Transient-Aware Processing**: Envelope-based transient/sustain split with independent saturation depth
- **Glue Compressor**: Sidechain-HPF-equipped bus compressor with adjustable attack, release, ratio, and stereo link
- **True Peak Limiter**: 8-sample lookahead brickwall with sub-sample peak detection
- **Multiband Engine**: 1-6 band Linkwitz-Riley LR4 crossover with per-band processing
- **Mid/Side Processing**: Full encode/decode with independent mid and side saturation
- **Reference Tone Matching**: Spectral analysis and auto-EQ correction curve
- **Preset Intelligence**: Input signal classification (Drums, Bass, Vocals, Pad, Mix) with contextual suggestions
- **MIDI Learn**: Up to 32 CC mappings with per-mapping range
- **Undo/Redo**: 64-step state history
- **A/B Comparison**: Loudness-matched slot comparison
- **Delta Monitoring**: Listen to only what BTZ is adding to the signal
- **Auto-Gain Compensation**: Maintains perceived loudness across drive settings
- **LFO Modulation**: Up to 4 LFOs with sine, triangle, square, and S&H shapes
- **Comprehensive Metering**: EBU R128 LUFS, True Peak, Stereo Correlation, Gain Reduction History, Harmonic Spectrum

### Three View Modes

| Mode | Target User | Features |
|------|-------------|----------|
| **Simple** | Beginners | 3 macro knobs (Drive, Tone, Output) + harmonic visualizer |
| **Standard** | Working producers | 6 character knobs + meters + macro sliders |
| **Advanced** | Sound designers | Full spectrum analyzer, neural model browser, LFO, multiband, reference tone, all meters |

## Build Requirements

| Requirement | Version |
|-------------|---------|
| CMake | 3.22+ |
| C++ Standard | C++17 |
| JUCE | 8.0.6 (auto-fetched if not found) |
| Compiler (macOS) | Xcode 15+ / Apple Clang 15+ |
| Compiler (Windows) | MSVC 2022+ |
| Compiler (Linux) | GCC 12+ or Clang 15+ |

## Building

```bash
cd BTZ
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BTZ_BUILD_TESTS` | OFF | Build GoogleTest unit tests |
| `BTZ_BUILD_CLAP` | ON | Build CLAP format via clap-juce-extensions |

### Running Tests

```bash
cmake -B build -DBTZ_BUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## Output Formats

| Format | Platform |
|--------|----------|
| VST3 | macOS, Windows, Linux |
| AU | macOS |
| Standalone | All |
| CLAP | All (with `-DBTZ_BUILD_CLAP=ON`) |

## Architecture

```
BTZ/Source/
├── BTZDsp.h            — Single-header DSP library (all processing modules)
├── BTZTheme.h          — Ivory System design tokens (colors, fonts, layout)
├── BTZComponents.h     — Custom JUCE components (knobs, meters, visualizer)
├── PluginProcessor.h   — Audio processor declaration
├── PluginProcessor.cpp — Signal chain implementation
├── PluginEditor.h      — GUI editor declaration
└── PluginEditor.cpp    — 3-mode layout implementation

BTZ/tests/
└── test_dsp_modules.cpp — 80+ GoogleTest unit tests

.github/workflows/
└── ci.yml              — Multi-platform CI/CD (macOS/Windows/Linux + pluginval)
```

### Signal Flow

```
Input → Safety (DC block + NaN guard)
      → Auto-Gain Input Measurement
      → M/S Encode
      → Resonance Taming (32-band)
      → Glue Compressor (with sidechain HPF)
      → Shine EQ (TDF-II biquad)
      → Multiband Split (LR4 crossover)
      → Per-band: Transient Split → Saturation (model-select) → LFO modulation
      → Multiband Recombine
      → Auto-Gain Compensation
      → True Peak Limiter (8-sample lookahead)
      → M/S Decode
      → Stereo Width
      → Wet/Dry Mix
      → Master Gain
      → Safety Post
      → Bypass Crossfade
      → Delta Monitoring (optional)
      → Correlation + Metering
Output
```

## Design System: Ivory

The visual language uses a warm, premium palette:

| Token | Hex | Usage |
|-------|-----|-------|
| Ivory | `#F5F0E8` | Main background |
| Parchment | `#EDE7DB` | Panels, sections |
| Warm Gray | `#D4CFC7` | Inset areas, meters |
| Sage | `#7A9E7E` | Primary accent (dynamics, EQ) |
| Terracotta | `#C17B4A` | Secondary accent (saturation, drive) |
| Charcoal | `#2D2A26` | Primary text |

Typography: **Inter** (UI), **JetBrains Mono** (values).

Knobs are rendered as ceramic-style circles with subtle shadow, matte fill, and accent-colored indicator arcs.

## State Version

Current: **v12** (Ivory System). State migration handles v10 to v12 automatically.

## CI/CD

GitHub Actions runs on every push to `main`, `overhaul/**`, `feature/**`, and `claude/**` branches:

1. **Build** on macOS 14, Windows latest, Ubuntu 22.04
2. **Test** via CTest (GoogleTest)
3. **Validate** via pluginval (macOS, strictness 5)
4. **Artifacts** uploaded for each platform

## License

Proprietary — BTZ Audio. All rights reserved.
