# BTZ Repository Map

## Repository Structure

```
btz-sonic-alchemy/
├── BTZ_JUCE/                     # Main JUCE plugin project
│   ├── CMakeLists.txt            # Build configuration (VST3 + Standalone)
│   ├── Source/
│   │   ├── PluginProcessor.cpp/h  # Audio processing engine
│   │   ├── PluginEditor.cpp/h     # GUI implementation
│   │   ├── DSP/                   # Digital Signal Processing modules
│   │   │   ├── TransientShaper.cpp    # Transient enhancement
│   │   │   ├── Saturation.cpp         # Harmonic saturation
│   │   │   ├── SubHarmonic.cpp        # Low-frequency synthesis
│   │   │   ├── SparkLimiter.cpp       # True-peak limiting
│   │   │   ├── ShineEQ.cpp            # High-frequency enhancement
│   │   │   ├── ConsoleEmulator.cpp    # Mix bus glue compression
│   │   │   ├── EnhancedSPARK.cpp      # Advanced limiter with hysteresis
│   │   │   ├── EnhancedSHINE.cpp      # Psychoacoustic air processing
│   │   │   ├── OversamplingManager.cpp # Oversampling infrastructure
│   │   │   └── LUFSMeter.cpp          # ITU-R BS.1770 loudness metering
│   │   ├── GUI/                   # User interface components
│   │   │   ├── MainView.cpp           # Primary plugin interface
│   │   │   ├── MeterStrip.cpp         # Professional metering display
│   │   │   ├── BTZKnob.cpp            # Custom rotary controls
│   │   │   └── ThermalKnob.cpp        # 3D beveled knob style
│   │   ├── Utility/
│   │   │   └── PresetManager.cpp      # Preset loading/saving system
│   │   └── Parameters/
│   │       └── PluginParameters.h     # Parameter definitions (29 total)
│   ├── JUCE/                     # JUCE framework (submodule or FetchContent)
│   └── build/                    # CMake build output directory
├── docs/                         # Documentation
│   ├── Build.md                  # Build instructions (this file)
│   ├── RepoMap.md                # Repository structure guide
│   ├── QuickStart.md             # Quick start guide
│   ├── UserManual.md             # Comprehensive user manual
│   ├── Specs.md                  # Technical specifications
│   ├── Measurements.md           # Measurement procedures (PluginDoctor/REW)
│   ├── Metering.md               # Metering implementation details
│   ├── Presets.md                # Preset catalog and usage
│   └── CompetitiveAnalysis.md    # Competitive positioning
├── btz-sonic-alchemy-main/       # Simplified reference implementation
│   └── BTZ/Source/               # Clean 4-file version (PoC)
└── scripts/                      # Build automation scripts

```

## Key Components

### Audio Processing Chain (PluginProcessor.cpp)

**Signal Flow:**
1. **Input** → DC blocking → Denormal protection
2. **Drive** → Input gain stage (0-12 dB)
3. **Transient Shaper** → Punch parameter (attack/sustain enhancement)
4. **Saturation** → Warmth parameter (harmonic generation)
5. **SPARK Limiter** → True-peak ceiling control
6. **DC Blocking** → Output stage protection
7. **Sub-Harmonic** → Boom parameter (low-frequency synthesis)
8. **SHINE EQ** → Air parameter (high-frequency enhancement)
9. **Console Emulator** → Glue parameter (mix bus compression)
10. **Width** → Stereo width control (mono-safe low-end)
11. **Output** → Master gain + Auto-gain compensation

### Parameter System (APVTS)

**29 Parameters:**
- **Hero Controls (5):** Punch, Warmth, Boom, Shine, Drive
- **Tone (4):** Air, Width, Density, Motion
- **Dynamics (3):** Glue, SPARK Ceiling, SPARK Mix
- **Mix (3):** Input Gain, Mix (wet/dry), Output Gain, Master Intensity
- **Advanced (4):** Quality Mode (oversampling), Character, Auto-Gain, Era (vintage/modern)
- **System (3):** Precision Mode, Active, Bypass
- **A/B/C Presets (3):** Preset A, B, C states

### GUI System (PluginEditor.cpp)

**Three-Tab Interface:**
- **Main Tab:** 12 rotary knobs (Punch, Warmth, Boom, Glue, Air, Width, Density, Motion, Era, Drive, Mix, Master)
- **SPARK Tab:** Limiter controls (Ceiling, SPARK Mix, SHINE Amount, SHINE Mix, Intensity)
- **Advanced Tab:** Quality Mode, Character Mode, Auto-Gain

**Metering:**
- Input Peak L/R, Input RMS L/R
- Output Peak L/R, Output RMS L/R
- SPARK Gain Reduction meter
- LUFS loudness, Stereo correlation
- Clip indicators

### Build System (CMakeLists.txt)

**Targets:**
- VST3 plugin (all platforms)
- Standalone application (all platforms)
- AU plugin (macOS only)

**Dependencies:**
- JUCE 7.0.12 (submodule or FetchContent)
- C++17 standard
- Platform-specific audio/GUI libraries

**Build Configuration:**
- Company: "BTZ Audio"
- Product: "BTZ - The Box Tone Zone"
- Bundle ID: com.btzaudio.btz
- VST3 Categories: Fx, Dynamics, EQ

## Real-Time Safety

**RT-Safe Guarantees:**
- ✅ No heap allocations in processBlock()
- ✅ No mutexes/locks in audio thread
- ✅ Lock-free parameter updates (atomics)
- ✅ Pre-allocated buffers in prepareToPlay()
- ✅ FTZ (Flush-To-Zero) denormal protection
- ✅ NaN/Inf sanitization
- ✅ Host call order validation

**Production Safety Features:**
- FL Studio constructor scan-safe (no DSP in constructor)
- Parameter smoothing (20ms default, per-parameter configurable)
- Silence optimization (skips DSP after 10 silent buffers)
- Bypass bit-perfect processing
- State migration framework (forward-compatible presets)

## File Locations

| Component | File Path |
|-----------|-----------|
| Main processor | `BTZ_JUCE/Source/PluginProcessor.cpp` |
| GUI | `BTZ_JUCE/Source/PluginEditor.cpp` |
| Build config | `BTZ_JUCE/CMakeLists.txt` |
| DSP modules | `BTZ_JUCE/Source/DSP/*.cpp` |
| Presets | `BTZ_JUCE/Source/Utility/PresetManager.cpp` |
| Parameters | `BTZ_JUCE/Source/Parameters/PluginParameters.h` |
| Metering | `BTZ_JUCE/Source/GUI/MeterStrip.cpp` |

## Development Workflow

1. **Make code changes** in `BTZ_JUCE/Source/`
2. **Build:** `cmake --build build --config Release`
3. **Test:** Load in DAW or run standalone
4. **Validate:** Run pluginval (strictness 10)
5. **Commit:** Git commit with clear message
6. **Document:** Update relevant .md files

## Next Steps

- See `Build.md` for compilation instructions
- See `QuickStart.md` for end-user guide
- See `UserManual.md` for comprehensive documentation
- See `Specs.md` for technical specifications
