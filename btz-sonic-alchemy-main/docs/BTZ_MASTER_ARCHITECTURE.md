# BTZ Sonic Alchemy — Master Architecture

> Reference version: **v1.0 Baseline**  
> Last updated: 2026-05-21

This document describes the technical architecture of the Box Tone Zone plugin. It serves as the authoritative reference for all implementation decisions and should be consulted before making structural changes.

---

## 1. High-Level Overview

BTZ is a stereo audio effect plugin that processes audio through a multi-stage signal chain combining saturation, dynamics, equalization, and spatial processing. The plugin is built on JUCE 8 and targets VST3, AU, CLAP, and Standalone formats.

The architecture follows three design constraints:

1. **Single-header DSP** — all processing modules live in `BTZDsp.h` with zero external dependencies beyond JUCE. This enables easy testing, portability, and compilation speed.

2. **Lock-free audio thread** — the `processBlock` function never allocates memory, never acquires locks, and never calls functions that might block. All communication between threads uses `std::atomic`.

3. **Strict separation of concerns** — DSP knows nothing about UI, UI knows nothing about DSP internals, and the Processor mediates between them via APVTS parameters and atomic state.

---

## 2. Signal Flow

The complete signal chain executes in `PluginProcessor::processBlock()` and is divided into three stages:

```
┌─────────────────────────────────────────────────────────────────┐
│ processLinearPre                                                  │
│   Input Safety (DC block + NaN/Inf guard + denormal flush)       │
│   Auto-Gain Input Measurement                                    │
│   Mid/Side Encode (if enabled)                                   │
│   Resonance Taming (32-band spectral suppression)                │
│   Glue Compressor (with sidechain HPF)                           │
│   Shine EQ (TDF-II parametric biquad)                            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│ processNonlinear                                                  │
│   Multiband Split (LR4 crossover, 1-6 bands)                    │
│   Per-band:                                                       │
│     Transient Split (envelope-based)                              │
│     Saturation (model-selected waveshaper or neural GRU)         │
│     LFO Modulation (drive depth modulation)                       │
│   Multiband Recombine (sum bands)                                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│ processLinearPost                                                  │
│   Auto-Gain Compensation (output level matching)                 │
│   True Peak Limiter (8-sample lookahead, monotonic deque)        │
│   Mid/Side Decode (if enabled)                                   │
│   Stereo Width                                                    │
│   Wet/Dry Mix                                                     │
│   Master Gain                                                     │
│   Output Safety (DC block + NaN/Inf guard)                       │
│   Bypass Crossfade (click-free 64-sample ramp)                   │
│   Delta Monitoring (wet - dry, if enabled)                       │
│   Correlation Measurement                                         │
│   Metering (LUFS, True Peak, Spectrum, GR History)               │
└─────────────────────────────────────────────────────────────────┘
```

---

## 3. DSP Module Inventory

All modules are defined as structs in `BTZDsp.h`. Each struct follows a consistent pattern: `reset()` to clear state, `prepare(sampleRate)` or `setSampleRate(sr)` for initialization, and a `process` method for per-sample or per-block computation.

| Module | Purpose | Topology |
|--------|---------|----------|
| `Waveshaper` | Static saturation models | Per-sample nonlinear function |
| `NeuralSaturationModel` | Learned saturation (RTNeural GRU) | Per-sample recurrent inference |
| `ShineProcessor` | Parametric EQ (air band) | TDF-II biquad, stereo |
| `SidechainHPF` | Compressor sidechain filter | 1-pole HPF |
| `GlueCompressor` | Bus compression | Feed-forward with soft knee |
| `TruePeakLimiter` | Brickwall limiting | Lookahead + monotonic deque |
| `LinkwitzRileyCrossover` | Multiband splitting | Cascaded 2nd-order Butterworth (LR4) |
| `MultibandEngine` | Per-band processing orchestration | N-band split/process/recombine |
| `ResonanceTamer` | Spectral peak suppression | 32-band envelope follower + gain |
| `TransientSplitter` | Transient/sustain separation | Dual envelope follower |
| `LFO` | Parameter modulation | Phase accumulator (4 shapes) |
| `EnvFollower` | Envelope detection | 1-pole with separate attack/release |
| `SmoothParam` | Parameter smoothing | Exponential 1-pole |
| `AutoGainSmoother` | Level-matched auto-gain | Dual RMS measurement + ratio |
| `SafetyLayer` | Signal protection | DC block + NaN/Inf/denormal + hard clip |
| `OversamplingEngine` | Anti-aliasing | FIR up/downsample (2x/4x/8x) |
| `MacroInterpreter` | Macro-to-parameter mapping | Curve-shaped value remapping |
| `MeterBallistics` | Level metering | Peak hold + RMS + LUFS integration |
| `LoudnessMeter` | EBU R128 LUFS | K-weighted filter + gating |
| `SpectrumBuffer` | FFT input accumulation | Ring buffer with ready flag |
| `GainReductionHistory` | GR timeline | Circular buffer (60s at 60fps) |
| `ReferenceToneMatcher` | Spectral matching | Capture + compare + correction curve |
| `PresetIntelligence` | Signal classification | Crest factor + spectral centroid |

---

## 4. State Management

### 4.1 Parameter State

All automatable parameters are managed through JUCE's `AudioProcessorValueTreeState` (APVTS). Parameter IDs are defined as `constexpr juce::StringRef` values in the `btz::id` namespace within `BTZTheme.h`.

The Processor reads parameter values via `getRawParameterValue()` pointers cached in `prepareToPlay()`, ensuring lock-free access from the audio thread.

### 4.2 Non-Parameter State

Several systems maintain state that is not directly exposed as DAW-automatable parameters:

| System | Storage | Thread Safety |
|--------|---------|---------------|
| Undo/Redo | `UndoStack` (64 `MemoryBlock` snapshots) | Message thread only |
| A/B Comparison | `ABState` (2 `MemoryBlock` slots) | Message thread only |
| MIDI Learn | `MIDILearnState` (32 mappings array) | Atomic flag for learning |
| Delta Monitoring | `std::atomic<bool>` | Lock-free |
| View Mode | Editor-local `int` | UI thread only |
| Preset Index | `std::atomic<int>` | Lock-free |

### 4.3 Serialization

State is serialized via `getStateInformation()` / `setStateInformation()` using JUCE's `MemoryBlock` and XML. The state includes a version number (`kStateVersion = 12`) for forward-compatible migration.

---

## 5. UI Architecture

### 5.1 Component Hierarchy

```
BTZAudioProcessorEditor (root)
├── HeaderBar
│   ├── Logo + Title
│   ├── ModeTabBar (Simple / Standard / Advanced)
│   ├── PresetNav (< name >)
│   ├── A/B Toggle
│   ├── Delta Button
│   └── Undo/Redo Buttons
├── ContentArea (mode-dependent)
│   ├── [Simple] 3x LabeledKnob + HarmonicVisualizer
│   ├── [Standard] 6x CeramicKnob + VerticalMeter + HSlider + HarmonicVisualizer
│   └── [Advanced] SpectrumPanel + NeuralBrowser + 8x CeramicKnob + LFO + Multiband + Meters
└── FooterBar
    ├── LUFS Readout + Meter
    ├── Correlation Meter
    └── Safety Indicator
```

### 5.2 Design Token System

All visual properties are defined as `constexpr` values in the `btz::` namespace within `BTZTheme.h`:

- `btz::color::` — palette colors (ivory, parchment, sage, terracotta, charcoal, etc.)
- `btz::type::` — font constructors (sans, mono, brand, h1, h2, body, label, micro, value)
- `btz::layout::` — dimensions (windowW, windowH, minW, minH, maxW, maxH, knobSize, etc.)
- `btz::id::` — parameter string IDs
- `btz::anim::` — animation timing constants

No hardcoded colors, fonts, or dimensions exist anywhere outside `BTZTheme.h`.

### 5.3 Component Library

Custom components in `BTZComponents.h`:

| Component | Description |
|-----------|-------------|
| `CeramicKnob` | Rotary control with matte fill, shadow, accent arc |
| `LabeledKnob` | CeramicKnob + label + value display + mouse interaction |
| `VerticalMeter` | Level meter with peak hold and gradient fill |
| `HSlider` | Horizontal slider with rounded track and thumb |
| `HarmonicVisualizer` | 3-mode display (Waveform / Spectrum / Harmonic bars) |
| `PresetBrowser` | Scrollable preset list with selection callback |
| `HeaderBar` | Top navigation bar |
| `FooterBar` | Bottom status bar |
| `TooltipOverlay` | Contextual help overlay |
| `SafetyIndicator` | Green/red signal safety status |

---

## 6. Build System

The build is managed by CMake (`BTZ/CMakeLists.txt`) with the following structure:

1. **JUCE Discovery** — looks for JUCE at `JUCE_DIR`, falls back to `FetchContent` from GitHub
2. **Plugin Target** — `juce_add_plugin` with VST3, AU, Standalone formats
3. **CLAP Target** — optional via `BTZ_BUILD_CLAP` and clap-juce-extensions
4. **Test Target** — optional via `BTZ_BUILD_TESTS`, links GoogleTest

The build produces:
- `BTZ.vst3` (all platforms)
- `BTZ.component` (macOS AU)
- `BTZ` standalone app
- `BTZ.clap` (if enabled)
- `BTZTests` executable (if enabled)

---

## 7. Threading Model

| Thread | Responsibilities | Constraints |
|--------|-----------------|-------------|
| Audio | `processBlock`, parameter reads, metering writes | No allocation, no locks, no blocking |
| Message | UI painting, parameter writes, state save/load, preset management | May allocate, may use JUCE MessageManager |
| Timer | Meter refresh (30-60 Hz), UI animation | Runs on message thread via `Timer` |

Communication patterns:
- Audio → UI: `std::atomic` reads (meters, GR, correlation)
- UI → Audio: APVTS parameter changes (thread-safe by design)
- UI → Audio (non-param): `std::atomic` flags (delta, bypass, learning)

---

## 8. Dependencies

| Dependency | Version | Source | Purpose |
|------------|---------|--------|---------|
| JUCE | 8.0.6 | FetchContent / local | Framework |
| GoogleTest | latest | FetchContent | Unit testing |
| clap-juce-extensions | latest | FetchContent | CLAP format (optional) |
| RTNeural | TBD | Not yet integrated | Neural model inference |

The plugin has **zero runtime dependencies** beyond the host DAW. All DSP is implemented from scratch in `BTZDsp.h`.

---

## 9. Modification Guidelines

When modifying this architecture:

1. **Adding a new DSP module** — add the struct to `BTZDsp.h`, add a test in `test_dsp_modules.cpp`, wire it into the appropriate stage in `PluginProcessor.cpp`.

2. **Adding a new parameter** — add the ID to `btz::id::` in `BTZTheme.h`, register it in `createParameterLayout()`, cache the pointer in `prepareToPlay()`, read it in the appropriate process stage.

3. **Adding a new UI component** — add the class to `BTZComponents.h`, instantiate it in the appropriate mode section of `PluginEditor.cpp`.

4. **Changing the signal flow** — update the diagram in this document and in `BTZ/README.md`.

5. **Bumping state version** — increment `kStateVersion` in `BTZDsp.h`, add migration logic in `setStateInformation()`.

For the full set of rules, see [BTZ_AI_WORKFLOW.md](./BTZ_AI_WORKFLOW.md).
