# BTZ – The Box Tone Zone Enhancer (MVP Dev Spec v1.0)

A precision drum tone sculptor VST3/AU/AAX built with JUCE.

UI
- Controls: Punch, Warmth, Boom, Mix, Drive, plus Texture toggle
- Meters: In/Out peak+RMS, GR, LUFS integrated (short reset)

Signal Flow (configurable Punch↔Warmth order)
Input → Punch (3‑band transient + FET comp + gate) → Warmth (even/odd, asym, tape bump) → Boom (sub synth + dyn low‑shelf + anti‑mud) → Texture (exciter + micro‑IR + tiny mod delay) → Drive (3‑band limiter → soft clip → true‑peak limiter, OS) → Console glue (light crosstalk) → Parallel Mix (latency‑comped) → Output with TP ceiling −1.0 dBTP

Engineering
- Oversampling: polyphase x4–x8 (HQ mode)
- ZDF filters in HQ path, denormal guards, vectorize hotspots
- Tested at 44.1/48/96 kHz, 64/128/256 buffers
- CMake flags to build with/without ML

Build
- Requirements: JUCE 7+, CMake 3.15+
- Configure
  - Default (no ML):
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
  - With ML backends (Torch):
    cmake -B build -S . -DWITH_ML=ON -DCMAKE_BUILD_TYPE=Release
- Build:
  cmake --build build --config Release
- Output: build/VST3/BTZ.vst3

Options
- WITH_ML=ON enables DeepFilterNet/TimbralTransfer integrations (behind BTZ_WITH_ML macro). Provide compatible model files in Source/Models/.

Acceptance & Quality Bars
- Aliasing < −80 dB in HQ at max Warmth/Drive
- No overs > −1.0 dBTP; LUFS targets (−8/−14) achievable without pumping
- Subjective A/B: more punch/weight, no transient smear, no mud

Presets
- Streaming Safe (−14 LUFS, −1 dBTP)
- Loud & Clean (−8 LUFS guard)
- Punchy Kick / Silky Snare / Room Glue / Tape Warmth / Boom Sculpt

Notes
- Current repo includes reference stubs for DSP modules and scaffolded processing chain; replace stubs with production DSP.
- Ensure proper licensing for any external models or code.

License
- Proprietary plugin code; third‑party components retain their original licenses (see respective repos).
