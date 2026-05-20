# Box Tone Zone (BTZ) — v1.2.0

A premium saturation, dynamics, and tone-shaping plugin built with JUCE 8.

## Overview

Box Tone Zone is a multiband saturation and dynamics processor featuring:

- **9 Saturation Models**: Tanh, Tube, Tape, Transistor, Transformer, plus 4 Neural slots (Neve, API, SSL, Custom) powered by real-time GRU inference
- **Wave Digital Filter Models**: Physically-modeled tube and transformer stages
- **Dynamic Resonance Taming**: Spectral peak suppression before saturation
- **Transient-Aware Processing**: Envelope-based transient/sustain split with independent saturation depth
- **Glue Compressor**: Sidechain-HPF-equipped bus compressor with adjustable ratio and makeup
- **True Peak Limiter**: 8-sample lookahead brickwall with sub-sample peak detection
- **Multiband Engine**: 1–6 band Linkwitz-Riley crossover with per-band processing
- **Mid/Side Processing**: Full encode/decode with independent mid and side saturation
- **Reference Tone Matching**: Spectral analysis and auto-EQ correction curve
- **Preset Intelligence**: Input signal classification (Drums, Bass, Vocals, etc.) with contextual suggestions
- **MIDI Learn**: Up to 64 CC mappings with per-mapping range and curve
- **Undo/Redo**: 50-step state history
- **A/B Comparison**: Loudness-matched slot comparison
- **Simple Mode**: 3-knob interface (Drive, Tone, Output) for quick results
- **LFO Modulation**: Per-parameter modulation with sine, triangle, square, and S&H shapes
- **Comprehensive Metering**: EBU R128 LUFS (Momentary/Short-term/Integrated), True Peak, Stereo Correlation, Gain Reduction History, Harmonic Spectrum

## Build Requirements

| Requirement | Version |
|-------------|---------|
| CMake | 3.22+ |
| C++ Standard | C++17 |
| JUCE | 8.x (fetched automatically) |
| Compiler (macOS) | Xcode 14+ / Apple Clang 14+ |
| Compiler (Windows) | Visual Studio 2022 / MSVC 17+ |
| Compiler (Linux) | GCC 11+ or Clang 14+ |

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
| `BTZ_ENABLE_CLAP` | ON | Build CLAP format (requires clap-juce-extensions) |

### Running Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBTZ_BUILD_TESTS=ON
cmake --build build --config Debug
cd build && ctest --output-on-failure
```

## Output Formats

| Format | Platform | Notes |
|--------|----------|-------|
| VST3 | All | Primary format |
| AU | macOS | Audio Unit v2 |
| CLAP | All | Modern open standard |
| Standalone | All | For testing without a DAW |

## Installation

After building, copy the plugin to the appropriate system directory:

| Platform | VST3 Path | AU Path |
|----------|-----------|---------|
| macOS | `~/Library/Audio/Plug-Ins/VST3/` | `~/Library/Audio/Plug-Ins/Components/` |
| Windows | `C:\Program Files\Common Files\VST3\` | N/A |
| Linux | `~/.vst3/` | N/A |

## Architecture

```
BTZ/Source/
├── BTZDsp.h           — All DSP modules (header-only, zero dependencies beyond JUCE)
├── BTZTheme.h         — Design tokens, color palette, typography, spacing
├── BTZComponents.h    — Reusable UI components (HarmonicVisualizer, GlassPanel, etc.)
├── PluginProcessor.h  — AudioProcessor declaration
├── PluginProcessor.cpp — Signal flow, state serialization, parameter layout
├── PluginEditor.h     — Editor declaration
└── PluginEditor.cpp   — UI layout, painting, interaction
```

### Signal Flow

```
Input → Safety (DC block + NaN catch)
      → Sidechain HPF → Glue Compressor
      → Mid/Side Encode (optional)
      → Resonance Taming (optional)
      → Multiband Split (1–6 bands)
      → Per-band: Transient Split → Saturation (model select) → Recombine
      → Multiband Recombine
      → Mid/Side Decode (optional)
      → Shine EQ (air band)
      → True Peak Limiter
      → Auto Gain Compensation
      → Safety (output DC block + NaN catch)
      → Dry/Wet Mix
      → Output
```

## Design System

The UI uses a warm, light premium palette:

| Token | Hex | Usage |
|-------|-----|-------|
| Canvas | `#F6F3EE` | Main background |
| Surface | `#EDE9E2` | Panels, sections |
| Well | `#E3DED6` | Inset areas, meters |
| Oak | `#B08D57` | Primary accent (saturation, drive) |
| Sage | `#7E9B8E` | Secondary accent (dynamics, EQ) |
| Terracotta | `#C0543E` | Tertiary (gain reduction, warnings) |
| Text | `#1A1A18` | Primary text |

Typography: **Syne** (display), **Inter Tight** (UI), **IBM Plex Mono** (values).

## State Version

Current state version: **11**. State migration is handled automatically for versions 8–11.

## License

Proprietary. All rights reserved.
