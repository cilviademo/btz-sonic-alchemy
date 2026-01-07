# Projucer Quick Start Guide

## Setting Up BTZ in Projucer

### 1. Install JUCE

Download JUCE from [https://juce.com/get-juce/](https://juce.com/get-juce/)

Extract to a permanent location (e.g., `~/JUCE` or `C:\JUCE`)

### 2. Open Projucer

Launch Projucer from the JUCE folder:
- macOS: `JUCE/Projucer.app`
- Windows: `JUCE\Projucer.exe`
- Linux: `JUCE/Projucer`

### 3. Create New Project

1. **File → New Project**
2. Select **Audio Plug-In → Basic**
3. Configure settings:

```
Project Name: BTZ
Project Type: Audio Plug-In
Create .jucer file in: /path/to/btz-sonic-alchemy/BTZ_JUCE/
```

### 4. Project Settings

Click the project settings (top-left gear icon):

**Main Settings:**
```
Project Name: BTZ
Project Version: 1.0.0
Company Name: BTZ Audio
Company Website: (leave blank or your website)
Company Email: (leave blank or your email)
```

**Plugin Characteristics:**
```
Plugin Manufacturer: BTZ Audio
Plugin Manufacturer Code: Btzz
Plugin Code: Btzp
Plugin Name: BTZ - The Box Tone Zone
Plugin Description: Precision drum tone sculptor
Plugin is a Synth: NO
Plugin Wants Midi Input: NO
Plugin Produces Midi Output: NO
Plugin is a Midi Effect: NO
Plugin Editor Requires Keyboard Focus: NO
```

**Plugin Formats:**
```
☑ VST3
☑ AU (macOS only)
☑ Standalone
☐ AAX (requires iLok SDK)
```

**Channel Configurations:**
```
Plugin Channel Configurations: {1, 1}, {2, 2}
```

### 5. Add Source Files

In the left sidebar, right-click **Source** folder:

**Add Existing Files** and select ALL files from `BTZ_JUCE/Source/`:

```
Source/
├── PluginProcessor.h
├── PluginProcessor.cpp
├── PluginEditor.h
├── PluginEditor.cpp
├── Parameters/
│   └── PluginParameters.h
├── DSP/
│   ├── TransientShaper.h
│   ├── TransientShaper.cpp
│   ├── Saturation.h
│   ├── Saturation.cpp
│   ├── SubHarmonic.h
│   ├── SubHarmonic.cpp
│   ├── SparkLimiter.h
│   ├── SparkLimiter.cpp
│   ├── ShineEQ.h
│   ├── ShineEQ.cpp
│   ├── ConsoleEmulator.h
│   ├── ConsoleEmulator.cpp
│   ├── Oversampling.h
│   └── Oversampling.cpp
└── GUI/
    ├── ThermalKnob.h
    ├── ThermalKnob.cpp
    ├── MeterStrip.h
    └── MeterStrip.cpp
```

### 6. Add JUCE Modules

The following modules should be auto-included. Verify in **Modules** section:

```
☑ juce_audio_basics
☑ juce_audio_devices
☑ juce_audio_formats
☑ juce_audio_plugin_client
☑ juce_audio_processors
☑ juce_audio_utils
☑ juce_core
☑ juce_data_structures
☑ juce_dsp (IMPORTANT!)
☑ juce_events
☑ juce_graphics
☑ juce_gui_basics
☑ juce_gui_extra
```

**If `juce_dsp` is missing:**
1. Click **+** button in Modules section
2. Select `juce_dsp` from list
3. Click **Add Module**

### 7. Configure Build Settings

Click the **Xcode (macOS)** / **Visual Studio** / **Linux Makefile** exporter:

**C++ Language Standard:**
```
C++17 or C++20
```

**Preprocessor Definitions:**
```
JUCE_WEB_BROWSER=0
JUCE_USE_CURL=0
JUCE_VST3_CAN_REPLACE_VST2=0
JUCE_DISPLAY_SPLASH_SCREEN=0
JUCE_REPORT_APP_USAGE=0
```

**Optimizations (Release build):**
```
Optimisation: -O3 (Maximum speed)
Enable Link-Time Optimization: YES
```

### 8. Save and Open in IDE

1. Click **Save Project** (Ctrl/Cmd + S)
2. Click **Open in IDE** button (or use the exporter dropdown)

**macOS:**
- Opens in **Xcode**
- Select scheme: **BTZ - VST3** or **BTZ - AU**
- Build: **Product → Build** (Cmd + B)

**Windows:**
- Opens in **Visual Studio**
- Set configuration: **Release x64**
- Build: **Build → Build Solution** (Ctrl + Shift + B)

**Linux:**
- Run `make` in the generated `Builds/LinuxMakefile/` directory

### 9. Build Output Locations

After successful build:

**macOS:**
```
~/Library/Audio/Plug-Ins/VST3/BTZ.vst3
~/Library/Audio/Plug-Ins/Components/BTZ.component
```

**Windows:**
```
C:\Program Files\Common Files\VST3\BTZ.vst3
```

**Linux:**
```
~/.vst3/BTZ.vst3
```

### 10. Test in DAW

1. Open your DAW (Logic, Ableton, FL Studio, etc.)
2. Rescan plugins if needed
3. Load **BTZ** on a drum track
4. Adjust parameters using the Generic Editor
5. Monitor LUFS/Peak meters

---

## Troubleshooting

**Error: "juce_dsp module not found"**
- Add `juce_dsp` module manually (see step 6)

**Error: "C++17 required"**
- Update C++ Language Standard in exporter settings (see step 7)

**Plugin doesn't appear in DAW**
- Check build output location
- Rescan plugins in DAW preferences
- Verify plugin format matches DAW (VST3/AU)

**Build errors with "Parameters/PluginParameters.h"**
- Ensure folder structure is correct in Projucer
- Verify all files are added with correct paths

**Audio glitches or high CPU**
- Reduce oversampling to 4x or 2x
- Increase DAW buffer size (512/1024 samples)

---

## Next Steps

Once the plugin is built and working:

1. **Test all parameters** using the Generic Editor
2. **Verify SPARK** limiting behavior
3. **Check SHINE** high-frequency response
4. **Develop custom GUI** (see `README.md` for roadmap)
5. **Create presets** based on the React UI preset list

---

## Custom GUI Development

To replace the Generic Editor with a custom GUI:

1. Edit `Source/PluginEditor.h/cpp`
2. Add custom components from `GUI/` folder
3. Implement ThermalKnob and MeterStrip rendering
4. Match styling from React UI (`theme.css`)
5. Rebuild and test

**Reference the React UI components:**
```
src/btz/EnhancedBTZPlugin.tsx
src/btz/components/ThermalKnob.tsx
src/btz/components/EnhancedMeterStrip.tsx
src/btz/theme.css
```

---

**Happy Building! 🎛️**
