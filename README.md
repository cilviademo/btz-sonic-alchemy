# BTZ – Box Tone Zone Enhancer

**BTZ** is a precision drum tone sculptor built as a **VST3 / AU (AAX-ready)** audio plugin using **JUCE** and **CMake**.
It is designed to enhance punch, warmth, weight, and texture in drums and rhythmic material while maintaining transient clarity and mix safety.

---

## Overview

**Primary Use Case**

* Drum bus enhancement
* Individual kick / snare sculpting
* Parallel punch & warmth processing
* Console-style glue and loudness-safe drive

**Core Philosophy**

* No transient smear
* No low-end mud
* Loudness-safe by design
* Studio-grade DSP with optional ML augmentation

---

## Signal Flow

Configurable order (Punch ↔ Warmth):

```
Input
 → Punch
   (3-band transient shaping + FET-style comp + gate)
 → Warmth
   (even/odd harmonics, asymmetry, tape bump)
 → Boom
   (sub synth + dynamic low shelf + anti-mud)
 → Texture
   (exciter + micro-IR + subtle modulation)
 → Drive
   (3-band limiter → soft clip → true-peak limiter)
 → Console Glue
   (light crosstalk + summing coloration)
 → Parallel Mix
   (latency compensated)
 → Output (−1.0 dBTP ceiling)
```

---

## Features

### Controls

* **Punch**
* **Warmth**
* **Boom**
* **Drive**
* **Mix**
* **Texture** (toggle)

### Metering

* Input / Output Peak + RMS
* Gain Reduction
* Integrated LUFS (short reset)

### DSP / Engineering

* Polyphase oversampling **x4–x8 (HQ mode)**
* ZDF filters in HQ path
* Denormal protection
* Vectorized hot paths
* Tested at:

  * 44.1 / 48 / 96 kHz
  * 64 / 128 / 256 buffer sizes

---

## Project Structure

```
BTZ/
├── Source/
│   ├── PluginProcessor.*
│   ├── PluginEditor.*
│   ├── Models/          # ML models (optional)
│   ├── IRs/             # Impulse responses
│   ├── DSP modules      # Punch, Warmth, Boom, etc.
├── tests/
├── CMakeLists.txt
└── README.md
```

> ⚠️ The `src/`, `public/`, and frontend-related folders in this repo are **not part of the JUCE plugin build** unless explicitly integrated via a webview system.

---

## Build Requirements

* **JUCE** 7+
* **CMake** 3.15+
* **C++17 compatible compiler**
* (Optional) ML backends such as Torch

---

## Building the Plugin

### Default Build (No ML)

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build With ML Enabled

```sh
cmake -B build -S . -DWITH_ML=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Output

```
build/VST3/BTZ.vst3
```

---

## Installing the Plugin

### Windows

Copy `BTZ.vst3` to:

```
C:\Program Files\Common Files\VST3
```

### macOS

Copy `BTZ.vst3` to:

```
/Library/Audio/Plug-Ins/VST3
```

Then rescan plugins or restart your DAW.

---

## Presets (Planned / Included)

* Streaming Safe (−14 LUFS / −1 dBTP)
* Loud & Clean (−8 LUFS guard)
* Punchy Kick
* Silky Snare
* Room Glue
* Tape Warmth
* Boom Sculpt

---

## Build Options

| Flag          | Description                              |
| ------------- | ---------------------------------------- |
| `WITH_ML=ON`  | Enables DeepFilterNet / Timbral Transfer |
| `BTZ_WITH_ML` | Compile-time macro guarding ML code      |
| Oversampling  | x4 default, x8 HQ                        |

> Model files must be placed in `Source/Models/` when ML is enabled.

---

## Quality & Acceptance Targets

* Aliasing: **< −80 dB** in HQ mode at max Warmth/Drive
* No overs above **−1.0 dBTP**
* LUFS targets achievable without pumping
* Subjective A/B: more punch and weight, no smear or mud
* Stable automation, preset recall, and DAW reloads

---

## Notes

* DSP modules are scaffolded; production DSP may replace stubs.
* Ensure proper licensing for all third-party code and models.
* AAX support requires Avid SDK and is not enabled by default.

---

## License

**Proprietary plugin code.**
Third-party components retain their original licenses (see respective sources).

---

