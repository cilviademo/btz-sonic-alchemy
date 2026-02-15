# BTZ - The Box Tone Zone

**AI-Enhanced Drum Processing Plugin**

Perfect — this repo has **two "worlds" living in one place**:

* **JUCE/C++ plugin** lives in `BTZ_JUCE/` (with `Source/`, `CMakeLists.txt`, `tests/`, model files, IRs, etc.)
* **Web/UI (React/Vite)** lives in `src/`, `public/`, `package.json`, etc. (this is *not* JUCE, it's a separate frontend)

So the "ELI5" is basically: **only the `BTZ_JUCE/` folder builds the VST3**. The React UI does not automatically become your JUCE UI unless you intentionally embed a webview (advanced).

Below is the step-by-step to take everything you have → build/test the plugin "for real".

---

## 0) What you need installed (once)

### Windows

* **Visual Studio 2022** with "Desktop development with C++"
* **CMake** (3.15+)
* **Git**
* Optional but recommended: **JUCE** (as a submodule or downloaded folder)

### macOS

* **Xcode**
* **CMake**
* **Git**

### Linux

* **Build essentials** (gcc/g++, make)
* **CMake** (3.15+)
* **Git**
* **JUCE dependencies**: `libasound2-dev libcurl4-openssl-dev libfreetype6-dev libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev mesa-common-dev webkit2gtk-4.0`

✅ Quick check in terminal:

```bash
git --version
cmake --version
```

---

## 1) Get the repo onto your computer (local clone)

1. In GitHub, copy the repo URL.
2. In terminal:

```bash
git clone <YOUR_REPO_URL>
cd <YOUR_REPO_FOLDER>
```

From the project structure, the plugin project is inside:

```bash
cd BTZ_JUCE
```

---

## 2) Make sure JUCE is actually connected to the C++ build

Your `BTZ_JUCE/CMakeLists.txt` must be able to find JUCE. There are 3 common ways:

### Option A (best): JUCE as a Git submodule

Inside `BTZ_JUCE/` you'd have something like `BTZ_JUCE/JUCE/`.

If you already have submodules, run:

```bash
git submodule update --init --recursive
```

✅ **This repo already has JUCE as a submodule** at `BTZ_JUCE/JUCE/`.

### Option B: JUCE downloaded locally

You manually download JUCE and point CMake to it (via `JUCE_DIR` or similar).

### Option C: CMake FetchContent

Your CMake pulls JUCE automatically.

✅ **If your build fails saying "JUCE not found"**, you're in this step and need to connect JUCE properly.

---

## 3) (Important) Handle your model files correctly (DeepFilterNet2.tar.gz, .pt)

Those model files can be huge. GitHub sometimes breaks them unless you use **Git LFS**.

If you plan to keep models in the repo:

1. Install Git LFS
2. In your repo root:

```bash
git lfs install
git lfs track "*.pt"
git lfs track "*.tar.gz"
git add .gitattributes
git add BTZ_JUCE/Source/Models/*
git commit -m "Track model files with Git LFS"
git push
```

If you **don't** want models in repo (also valid):

* keep empty placeholders in `BTZ_JUCE/Source/Models/`
* download models during setup (later)

---

## 4) Build the JUCE plugin with CMake (your spec already matches this)

From inside `BTZ_JUCE/` run:

### Default build (no ML)

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### ML build (Torch/etc.)

```bash
cmake -B build -S . -DWITH_ML=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

✅ If it succeeds, you should get:

* `build/BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3/` (or similar)

If you want Debug for easier troubleshooting:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

---

## 5) Where the built plugin goes (and how to "install" it)

A DAW will only see it if the `.vst3` is in the standard VST3 folder.

### Windows

Copy `BTZ - The Box Tone Zone.vst3` to:
`C:\Program Files\Common Files\VST3`

### macOS

Copy `BTZ - The Box Tone Zone.vst3` to:
`/Library/Audio/Plug-Ins/VST3`

### Linux

Copy `BTZ - The Box Tone Zone.vst3` to:
`~/.vst3/` (user plugins) or `/usr/lib/vst3/` (system-wide)

✅ **This repo's CMake automatically installs to `~/.vst3/` on build** (see build output).

Then **restart DAW** or **rescan plugins**.

---

## 6) Open and test it in a DAW (ELI5 checklist)

1. Open DAW (FL Studio / Ableton / Reaper)
2. Create an audio track
3. Insert plugin: **BTZ - The Box Tone Zone**
4. Play a drum loop through it
5. Verify:

   * UI opens
   * knobs move
   * sound changes
   * doesn't crash

Then test the "real stuff":

* Save session → close DAW → reopen → plugin state restores
* Automate Mix/Drive → playback → no glitches/clicks (smoothing works)
* Test preset A/B/C switching (20ms click-free ramping)
* Verify SPARK limiter with true-peak metering
* Check double-click reset on knobs (should default to sensible values)

---

## 7) "How do I take what Claude generated and put it in the right place?"

Think of Claude's output as **files that must match your `BTZ_JUCE/Source/` structure**.

### The rule:

* Any new `.cpp/.h` must live in `BTZ_JUCE/Source/...`
* And **CMake must compile it**

So if Claude created:

* `PunchProcessor.cpp/.h`
* `WarmthProcessor.cpp/.h`

You put them in:
`BTZ_JUCE/Source/DSP/` (or appropriate subfolder)

Then you update `BTZ_JUCE/CMakeLists.txt` (or whatever it uses) so those files are included in the build.

✅ If you forget to add them to CMake, the project compiles like they don't exist.

### Current Project Structure:

```
BTZ_JUCE/
├── Source/
│   ├── DSP/              # All DSP processors
│   ├── GUI/              # JUCE GUI components
│   ├── Parameters/       # APVTS parameter definitions
│   ├── Utilities/        # Helper functions
│   ├── Models/           # ML model files (.pt, .tar.gz)
│   ├── IRs/              # Impulse responses
│   ├── PluginProcessor.cpp/h  # Main audio processor
│   └── PluginEditor.cpp/h     # Main GUI editor
├── CMakeLists.txt        # Build configuration
├── JUCE/                 # JUCE framework (submodule)
└── build/                # Build artifacts (gitignored)
```

---

## 8) The JUCE "Projucer" question (important)

This repo uses **CMake** (`CMakeLists.txt`). That means you **do not need Projucer** to build.

You only use Projucer if your project is `.jucer`-based.

So for *this repo*, the workflow is:
✅ **Edit code → run CMake build → copy VST3 → test in DAW**

Not: "upload to JUCE".

("JUCE" is the framework; you don't upload code to JUCE. You compile your project *using* JUCE.)

---

## 9) AAX note (so you don't waste time)

The spec mentions VST3/AU/AAX. Reality check:

* **VST3** ✅ straightforward
* **AU** ✅ on macOS
* **AAX** ⚠️ requires **Avid AAX SDK** and extra setup/licensing. You usually add AAX later.

So for now: build/test **VST3 first**.

---

## 10) Common "it doesn't work" fixes

### If build fails

* JUCE not found → fix JUCE path/submodule: `git submodule update --init --recursive`
* Missing Torch when `WITH_ML=ON` → turn ML off or install libs correctly
* Models missing → ensure `BTZ_JUCE/Source/Models/` exists and code can find them

### If DAW can't find plugin

* Wrong folder → check standard VST3 paths above
* DAW cache not refreshed → restart DAW or force rescan
* You built Debug but copied Release from an older build (duplicate plugin versions)

---

## Your repo structure: what I infer from the project

* `BTZ_JUCE/Source/*` = C++ plugin code (this is what builds)
* `BTZ_JUCE/Source/Models/*` = ML assets used when `WITH_ML=ON`
* `BTZ_JUCE/Source/IRs/*` = impulse responses
* `src/`, `public/`, `package.json` = separate web UI project (not JUCE)

So: **don't expect the React UI to appear inside the plugin** unless you intentionally implement that bridge.

---

## React/Vite Frontend (Optional)

The `src/` directory contains a separate React/Vite web application. This is NOT the JUCE plugin UI.

### To run the web frontend:

```bash
# From repo root
npm install
npm run dev
```

This will start a development server at `http://localhost:5173` (or similar).

**Note**: This web UI is completely separate from the JUCE plugin. They do not communicate unless you build a custom bridge.

---

## Quick Start Summary

### For the JUCE Plugin:

```bash
# 1. Clone and navigate
git clone <YOUR_REPO_URL>
cd <YOUR_REPO_FOLDER>/BTZ_JUCE

# 2. Initialize JUCE submodule
git submodule update --init --recursive

# 3. Build plugin
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 4. Plugin automatically installed to ~/.vst3/ (Linux) or standard paths
# 5. Restart your DAW and load "BTZ - The Box Tone Zone"
```

### For the Web UI:

```bash
# From repo root
npm install
npm run dev
```

---

## Current Features (P1 Sprint Complete - 85% Overall)

✅ **DSP Core**
- Adaptive punch/warmth/boom/drive processing
- SPARK true-peak limiter with Jiles-Atherton hysteresis
- ITU BS.1770-compliant LUFS metering
- RT-safe parameter smoothing (20ms ramping)

✅ **GUI**
- Custom MainView-based editor (BTZTheme styling)
- A/B/C preset system with click-free switching
- 5 factory presets (Gentle Glue, Pop Punch, Modern Master, Vintage Vibe, Dynamic Enhancer)
- Real-time metering (LUFS, peak, gain reduction, stereo correlation)
- Tooltips on all 13 controls
- Double-click reset to sensible defaults
- Optimized meter rendering (conditional repaints)

✅ **Production Safety**
- Zero allocations in audio thread
- Lock-free parameter updates
- DAW quirks detection and handling
- Comprehensive unit tests

---

## Documentation

- `docs/QUICK_WINS_2026-01-15.md` - Latest 5 GUI improvements
- `docs/P1_SPRINT_COMPLETE.md` - P1 milestone summary
- `docs/SHIP_GATE_*.md` - Release checklists

---

## Next Steps (P2 Roadmap)

- Advanced preset management (user presets, import/export)
- Undo/redo system
- Additional metering views
- ML-powered transient detection (when `WITH_ML=ON`)

---

## Tell me these 2 things and I'll give you *exact* commands + exact file paths for your machine

1. Are you on **Windows**, **macOS**, or **Linux** right now?
2. Does `BTZ_JUCE/CMakeLists.txt` currently use **JUCE as a submodule** (yes, it does), or do you have JUCE installed separately?

---

## License

[Your license here]

## Contact

[Your contact info here]
