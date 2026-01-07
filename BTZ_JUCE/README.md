# BTZ - The Box Tone Zone Enhancer

**Precision drum tone sculptor that adds punch, weight, and character without cluttering your mix.**

From silky transients to analog-style saturation — BTZ puts your drums in the zone.

---

## 🎵 Features

### **Hero Controls** (5 Main Knobs)
- **PUNCH** - Transient shaping for snappy attacks (Waves Smack Attack + CLA-76 + SPL Transient Designer)
- **WARMTH** - Analog-style saturation with harmonic richness (Soundtoys Decapitator + Plugin Alliance HG-2 + Output Thermal)
- **BOOM** - Subharmonic synthesis for powerful low-end (Plugin Alliance bx_subsynth + Unfiltered Audio Bass-Mint)
- **MIX** - Wet/dry blend with console emulation for cohesion (The Glue + SSL/Neve console modeling)
- **DRIVE** - Loudness and impact control

### **SPARK** ⚡ - Advanced Clipping Engine
*The magic of BTZ*
- Combines FL Studio Clipper + GoldClip + BigClipper + KClip + Acustica clippers
- Target LUFS: -14 to 0 LUFS (default: -5 for commercial competitiveness)
- True Peak Ceiling: -3 to 0 dBTP (default: -0.3 to prevent inter-sample peaks)
- Oversampling: 1x, 2x, 4x, 8x, or 16x (default: 8x for artifact-free processing)
- Modes: Soft (musical/warm) or Hard (aggressive/punchy)
- **Brutal loudness with surgical transparency**

### **SHINE** ✨ - Ultra-High Frequency Air
- SSL Fusion Air + Maag EQ Air Band emulation
- Frequency range: 10kHz to 80kHz (extends beyond Nyquist via oversampling)
- Default: 20kHz @ +3dB, Q=0.5, 50% mix
- Ethereal highs, crystalline crispness, analog-inspired sheen

### **Additional Features**
- **I/O Trim**: Input/Output gain (-12 to +12 dB) with Auto-Gain option
- **Master Controls**: Macro control with Transparent/Glue/Vintage blend modes
- **Precision Mode**: Quantized, fine-grained adjustments
- **Enhanced Metering**: LUFS, True Peak, Gain Reduction, Stereo Correlation

---

## 🔧 Building the Plugin

### Prerequisites

1. **JUCE Framework** (v7.0+)
   - Download from [https://juce.com/](https://juce.com/)
   - Or install via package manager:
     ```bash
     # macOS (Homebrew)
     brew install juce

     # Linux (from source)
     git clone https://github.com/juce-framework/JUCE.git
     ```

2. **CMake** (v3.15+)
   ```bash
   # macOS
   brew install cmake

   # Linux (Debian/Ubuntu)
   sudo apt-get install cmake

   # Windows
   # Download from https://cmake.org/download/
   ```

3. **C++ Compiler**
   - macOS: Xcode Command Line Tools (`xcode-select --install`)
   - Linux: GCC 9+ or Clang 10+
   - Windows: Visual Studio 2019+ or MinGW

### Build Instructions

#### Option 1: Using CMake (Recommended)

```bash
cd BTZ_JUCE

# Create build directory
mkdir build
cd build

# Configure (adjust JUCE path if needed)
cmake .. -DCMAKE_PREFIX_PATH=/path/to/JUCE

# Build
cmake --build . --config Release

# Install (optional)
cmake --install .
```

**Important**: Update the JUCE path in `CMakeLists.txt` line 6:
```cmake
add_subdirectory(/path/to/JUCE build)  # Adjust this path
```

#### Option 2: Using Projucer GUI

1. Download and open **Projucer** (comes with JUCE)
2. Create a new project: **File → New Project → Audio Plug-In → Basic**
3. Configure project settings:
   - **Project Name**: BTZ
   - **Plugin Formats**: VST3, AU, Standalone
   - **Plugin Manufacturer**: BTZ Audio
   - **Plugin Code**: Btzp
4. Add source files from `BTZ_JUCE/Source/` to the Projucer project
5. Add module paths for `juce_audio_utils` and `juce_dsp`
6. Save and open in your IDE (Xcode/Visual Studio)
7. Build

#### Platform-Specific Notes

**macOS**:
- VST3 output: `~/Library/Audio/Plug-Ins/VST3/BTZ.vst3`
- AU output: `~/Library/Audio/Plug-Ins/Components/BTZ.component`

**Windows**:
- VST3 output: `C:\Program Files\Common Files\VST3\BTZ.vst3`

**Linux**:
- VST3 output: `~/.vst3/BTZ.vst3`

---

## 🎛️ DSP Architecture

### Signal Flow

```
INPUT
  ↓
[Input Gain]
  ↓
[Punch - Transient Shaping]
  ↓
[Warmth - Saturation]
  ↓
[Boom - Subharmonic Synthesis]
  ↓
[SPARK - Advanced Clipping/Limiting] ⚡
  ↓
[SHINE - Ultra-High Frequency Air] ✨
  ↓
[Console Emulation - Mix Glue]
  ↓
[Output Gain]
  ↓
OUTPUT
```

### DSP Modules

| Module | Description | Files |
|--------|-------------|-------|
| **TransientShaper** | Envelope-based transient enhancement | `TransientShaper.h/cpp` |
| **Saturation** | Tanh-based soft clipping with harmonic generation | `Saturation.h/cpp` |
| **SubHarmonic** | Octave-down subharmonic synthesis | `SubHarmonic.h/cpp` |
| **SparkLimiter** | Multi-mode clipper/limiter with lookahead | `SparkLimiter.h/cpp` |
| **ShineEQ** | Biquad high-shelf EQ (10kHz-80kHz) | `ShineEQ.h/cpp` |
| **ConsoleEmulator** | SSL/Neve-style console coloration | `ConsoleEmulator.h/cpp` |
| **OversamplingProcessor** | Up to 16x oversampling wrapper | `Oversampling.h/cpp` |

---

## 🎚️ Parameters

### Hero Controls
| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Punch | 0.0 - 1.0 | 0.0 | Transient intensity |
| Warmth | 0.0 - 1.0 | 0.0 | Saturation amount |
| Boom | 0.0 - 1.0 | 0.0 | Subharmonic level |
| Mix | 0.0 - 1.0 | 1.0 | Wet/dry blend |
| Drive | 0.0 - 1.0 | 0.0 | Output level/impact |

### SPARK Parameters
| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Enabled | On/Off | ON | Global enable |
| Target LUFS | -14 to 0 | -5 | Integrated loudness target |
| Ceiling | -3 to 0 dBTP | -0.3 | True peak ceiling |
| Mix | 0.0 - 1.0 | 1.0 | Wet/dry blend |
| Oversampling | 1x/2x/4x/8x/16x | 8x | Oversampling factor |
| Auto OS | On/Off | ON | Automatic oversampling |
| Mode | Soft/Hard | Soft | Clipping character |

### SHINE Parameters
| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Enabled | On/Off | OFF | Global enable |
| Frequency | 10kHz - 80kHz | 20kHz | Center frequency |
| Gain | -12 to +12 dB | +3 dB | Boost/cut amount |
| Q | 0.1 - 2.0 | 0.5 | Resonance/width |
| Mix | 0.0 - 1.0 | 0.5 | Wet/dry blend |
| Auto OS | On/Off | ON | Automatic oversampling |

### I/O & Master
| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Input Gain | -12 to +12 dB | 0 dB | Input trim |
| Output Gain | -12 to +12 dB | 0 dB | Output trim |
| Auto Gain | On/Off | OFF | Loudness matching |
| Master Blend | Transparent/Glue/Vintage | Transparent | Console type |
| Precision Mode | On/Off | OFF | Fine adjustment mode |

---

## 🎨 GUI Development (In Progress)

The current build uses JUCE's **GenericAudioProcessorEditor** for quick testing. A custom GUI matching the React UI design is planned, featuring:

- **ThermalKnob**: Custom rotary knob with thermal gradient visualization
- **MeterStrip**: 4-meter layout (LUFS, True Peak, GR, Stereo Correlation)
- **A/B Toggle**: State comparison with delta visualization
- **ActivePathStrip**: Visual signal flow indicator
- **PresetBrowser**: Categorized preset management
- **QuickModeButtons**: One-click Safe TP, Streaming, Club presets

---

## 📦 Project Structure

```
BTZ_JUCE/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
└── Source/
    ├── PluginProcessor.h/cpp   # Main audio processor
    ├── PluginEditor.h/cpp      # GUI editor
    ├── Parameters/
    │   └── PluginParameters.h  # Parameter definitions
    ├── DSP/
    │   ├── TransientShaper.h/cpp
    │   ├── Saturation.h/cpp
    │   ├── SubHarmonic.h/cpp
    │   ├── SparkLimiter.h/cpp
    │   ├── ShineEQ.h/cpp
    │   ├── ConsoleEmulator.h/cpp
    │   └── Oversampling.h/cpp
    └── GUI/
        ├── ThermalKnob.h/cpp   # Custom knob (placeholder)
        └── MeterStrip.h/cpp    # Meter display (placeholder)
```

---

## 🧪 Testing

1. Build the plugin (see instructions above)
2. Load in your DAW:
   - **Logic Pro**: AU/VST3
   - **Ableton Live**: VST3
   - **FL Studio**: VST3
   - **Reaper**: VST3
   - **Pro Tools**: VST3 (2023+)
3. Test presets:
   - **Default**: SPARK enabled at -5 LUFS
   - **Streaming**: -14 LUFS, -0.3 dBTP ceiling
   - **Club**: -9 LUFS, Hard mode
4. Monitor metering for LUFS/Peak accuracy

---

## 🛠️ Troubleshooting

**Build Errors:**
- Ensure JUCE path is correct in `CMakeLists.txt`
- Check C++17 compiler support
- Verify all source files are included

**Plugin Not Loading:**
- Check plugin format matches DAW (VST3/AU)
- Verify installation path
- Rescan plugins in DAW

**Audio Glitches:**
- Reduce oversampling factor (16x → 8x)
- Increase buffer size in DAW (512/1024 samples)
- Disable Auto OS if CPU is high

---

## 📝 License

This plugin is based on features from numerous commercial plugins. **For educational/personal use only**.

**Modeled plugins include**:
- Waves (Smack Attack, CLA-76, L3, Decapitator concepts)
- Plugin Alliance (bx_subsynth, HG-2, SPL Transient Designer concepts)
- Output (Thermal, Portal concepts)
- Soundtoys (Decapitator concepts)
- SSL / Neve (Console emulation concepts)
- FL Studio (Clipper concept)
- Acustica Audio (Clipping/saturation concepts)

---

## 🚀 Roadmap

- [ ] Custom GUI matching React UI design
- [ ] Preset management system
- [ ] A/B comparison functionality
- [ ] Undo/Redo history
- [ ] Enhanced LUFS metering (ITU-R BS.1770-4 compliant)
- [ ] Oversampling optimization
- [ ] MIDI learn for automation
- [ ] Sidechain input for ducking

---

## 📧 Contact

For questions or feedback about the BTZ plugin architecture, please refer to the original codebase documentation at `btz-sonic-alchemy`.

---

**Built with JUCE Framework**
**Powered by DSP algorithms inspired by industry-standard plugins**
