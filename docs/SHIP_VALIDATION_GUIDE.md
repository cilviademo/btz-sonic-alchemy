# BTZ Ship Validation Guide

**Version**: 1.0.0
**Purpose**: Exact step-by-step protocol for release validation
**Target**: Version 1.0.0 release candidate
**Last Updated**: 2026-02-09

---

## 🎯 Overview

This guide provides exact commands and pass/fail criteria for validating BTZ before release. All tests must pass before creating a release tag.

**Validation Philosophy**: Measure everything, guess nothing.

---

## ✅ Pre-Flight Checklist

Before starting validation:

- [ ] All code committed and pushed to `claude/analyze-test-coverage-W9rXL`
- [ ] CI/CD builds passing (Windows, macOS, Linux)
- [ ] No untracked files in working directory
- [ ] Plugin version updated in `CMakeLists.txt` (currently 1.0.0)

---

## 1. Build Verification

### 1.1 Windows Build (FL Studio Primary Target)

```powershell
# Clean build
cd BTZ_JUCE
if (Test-Path build) { Remove-Item -Recurse -Force build }
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release --parallel

# Verify artifacts
if (!(Test-Path "BTZ_artefacts/Release/VST3/BTZ.vst3")) {
    Write-Error "❌ FAIL: VST3 not built"
    exit 1
}
if (!(Test-Path "BTZ_artefacts/Release/Standalone/BTZ.exe")) {
    Write-Error "❌ FAIL: Standalone not built"
    exit 1
}

Write-Host "✅ PASS: Windows build artifacts created"

# Check binary size
$vst3Size = (Get-Item -Recurse "BTZ_artefacts/Release/VST3/BTZ.vst3" | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host "VST3 size: $([math]::Round($vst3Size, 2)) MB (expect 5-15 MB)"

if ($vst3Size -lt 2 -or $vst3Size -gt 50) {
    Write-Warning "⚠️ WARNING: VST3 size unusual (expected 5-15 MB)"
}
```

**Pass Criteria**:
- ✅ VST3 and Standalone binaries created
- ✅ VST3 size: 5-15 MB (reasonable range)
- ✅ No build errors or warnings in BTZ code

### 1.2 macOS Build (Universal Binary)

```bash
# Clean build
cd BTZ_JUCE
rm -rf build
mkdir build
cd build

# Configure (Universal Binary: x86_64 + arm64)
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0

# Build
cmake --build . --config Release --parallel

# Verify artifacts
if [ ! -d "BTZ_artefacts/Release/VST3/BTZ.vst3" ]; then
    echo "❌ FAIL: VST3 not built"
    exit 1
fi

if [ ! -d "BTZ_artefacts/Release/AU/BTZ.component" ]; then
    echo "❌ FAIL: AU not built"
    exit 1
fi

if [ ! -d "BTZ_artefacts/Release/Standalone/BTZ.app" ]; then
    echo "❌ FAIL: Standalone not built"
    exit 1
fi

echo "✅ PASS: macOS build artifacts created"

# Verify Universal Binary
echo "VST3 architectures:"
lipo -info BTZ_artefacts/Release/VST3/BTZ.vst3/Contents/MacOS/BTZ

echo "AU architectures:"
lipo -info BTZ_artefacts/Release/AU/BTZ.component/Contents/MacOS/BTZ

# Expected output: "Architectures in the fat file: ... are: x86_64 arm64"
if ! lipo -info BTZ_artefacts/Release/VST3/BTZ.vst3/Contents/MacOS/BTZ | grep -q "x86_64 arm64"; then
    echo "❌ FAIL: Not a Universal Binary"
    exit 1
fi

echo "✅ PASS: Universal Binary verified"
```

**Pass Criteria**:
- ✅ VST3, AU, and Standalone binaries created
- ✅ All binaries are Universal (x86_64 + arm64)
- ✅ No build errors or warnings in BTZ code

### 1.3 Linux Build

```bash
# Clean build
cd BTZ_JUCE
rm -rf build
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release --parallel

# Verify artifacts
if [ ! -d "BTZ_artefacts/Release/VST3/BTZ.vst3" ]; then
    echo "❌ FAIL: VST3 not built"
    exit 1
fi

if [ ! -f "BTZ_artefacts/Release/Standalone/BTZ" ]; then
    echo "❌ FAIL: Standalone not built"
    exit 1
fi

echo "✅ PASS: Linux build artifacts created"
```

**Pass Criteria**:
- ✅ VST3 and Standalone binaries created
- ✅ No build errors or warnings in BTZ code

---

## 2. FL Studio Scan Test (CRITICAL - PRIMARY TARGET)

**Platform**: Windows 10/11
**FL Studio Version**: 20.9+ or 21.x

### 2.1 Fresh Plugin Scan

```powershell
# Copy VST3 to system plugin folder
Copy-Item "BTZ_artefacts/Release/VST3/BTZ.vst3" `
          "C:\Program Files\Common Files\VST3\" -Recurse -Force

Write-Host "✅ BTZ.vst3 copied to system folder"
```

**Manual Steps**:
1. Open FL Studio
2. Options → Manage plugins → Find plugins
3. Click "Start scan" or "Rescan"

**Pass Criteria**:
- ✅ Plugin appears in plugin list (no red X icon)
- ✅ No "failed to load" error
- ✅ No "crashed during scan" error
- ✅ Plugin info shows: "BTZ - The Box Tone Zone" by "BTZ Audio"
- ✅ No Windows Defender or antivirus warnings

### 2.2 FL Studio Instantiation Test

1. Create new FL Studio project
2. Add BTZ to mixer track: Mixer → Insert Slot → VST3 → BTZ Audio → BTZ

**Pass Criteria**:
- ✅ GUI opens within 2 seconds
- ✅ No crash on instantiation
- ✅ All knobs visible and responsive
- ✅ Meters update during playback

### 2.3 FL Studio Parameter Automation

1. Right-click "Punch" knob → "Create automation clip"
2. Draw automation curve (0% → 100% → 0%)
3. Play project

**Pass Criteria**:
- ✅ Automation clip created successfully
- ✅ Parameter responds to automation in real-time
- ✅ No clicks/pops during automation
- ✅ No CPU spikes

### 2.4 FL Studio Save/Reload

1. Load BTZ, adjust parameters (Punch=50%, Warmth=30%, Boom=20%)
2. Save project as "BTZ_test.flp"
3. Close FL Studio
4. Reopen "BTZ_test.flp"

**Pass Criteria**:
- ✅ Project loads without errors
- ✅ BTZ recalls exact parameter state (Punch=50%, Warmth=30%, Boom=20%)
- ✅ Audio output identical to before save

---

## 3. pluginval Validation (Strictness 10)

**Tool**: [pluginval](https://github.com/Tracktion/pluginval)
**Target Strictness**: 10 (maximum)

### 3.1 Windows VST3

```powershell
# Download pluginval (if not installed)
# https://github.com/Tracktion/pluginval/releases

pluginval.exe --strictness-level 10 `
  --validate "C:\Program Files\Common Files\VST3\BTZ.vst3" `
  --output-dir validation-results `
  --verbose

# Check exit code
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ FAIL: pluginval failed"
    Get-Content validation-results/*.txt
    exit 1
}

Write-Host "✅ PASS: pluginval (Windows VST3)"
```

### 3.2 macOS VST3

```bash
# Download pluginval (if not installed)
# https://github.com/Tracktion/pluginval/releases

./pluginval --strictness-level 10 \
  --validate ~/Library/Audio/Plug-Ins/VST3/BTZ.vst3 \
  --output-dir validation-results \
  --verbose

if [ $? -ne 0 ]; then
    echo "❌ FAIL: pluginval failed"
    cat validation-results/*.txt
    exit 1
fi

echo "✅ PASS: pluginval (macOS VST3)"
```

### 3.3 macOS AU (auval)

```bash
# AU validation (built-in macOS tool)
auval -v aufx Btzp Btzz

# Expected: "PASSED" at end of output
if ! auval -v aufx Btzp Btzz | grep -q "PASSED"; then
    echo "❌ FAIL: AU validation failed"
    exit 1
fi

echo "✅ PASS: AU validation"
```

**Pass Criteria** (all platforms):
- ✅ No FAILED tests
- ✅ No crashes
- ✅ Parameter validation passes
- ✅ State save/load passes
- ✅ No memory leaks detected
- ✅ Thread safety passes (no data races)

---

## 4. Offline Bounce Determinism Test (CRITICAL)

**Purpose**: Ensure identical output for identical input (required for professional workflows)

### 4.1 Generate Test Signal

```bash
# Option A: Use your DAW to export 10 seconds of pink noise at -18 dBFS RMS
# Save as: test_determinism_input.wav (48 kHz, 24-bit, stereo)

# Option B: Use sox (if installed)
sox -n -r 48000 -b 24 test_determinism_input.wav synth 10 pinknoise vol -18dB
```

### 4.2 Run Determinism Test Script

```bash
#!/bin/bash
# determinism_test.sh

INPUT="test_determinism_input.wav"
OUTPUT_DIR="determinism_outputs"

# Create output directory
rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

# Render 5 consecutive bounces
echo "Rendering 5 consecutive bounces..."
for i in {1..5}; do
    echo "Bounce $i/5..."

    # Load plugin in DAW, offline bounce, save as bounce_$i.wav
    # (This step requires manual DAW operation OR offline_render tool)

    # Using offline_render tool:
    ../build/tools/offline_render \
        $INPUT \
        $OUTPUT_DIR/bounce_$i.wav
done

# Compute MD5 hashes
echo -e "\n=== MD5 Hashes ==="
md5sum $OUTPUT_DIR/bounce_*.wav

# Check if all hashes match
HASH1=$(md5sum $OUTPUT_DIR/bounce_1.wav | awk '{print $1}')
HASH2=$(md5sum $OUTPUT_DIR/bounce_2.wav | awk '{print $1}')
HASH3=$(md5sum $OUTPUT_DIR/bounce_3.wav | awk '{print $1}')
HASH4=$(md5sum $OUTPUT_DIR/bounce_4.wav | awk '{print $1}')
HASH5=$(md5sum $OUTPUT_DIR/bounce_5.wav | awk '{print $1}')

if [ "$HASH1" == "$HASH2" ] && [ "$HASH2" == "$HASH3" ] && \
   [ "$HASH3" == "$HASH4" ] && [ "$HASH4" == "$HASH5" ]; then
    echo -e "\n✅ PASS: All 5 bounces IDENTICAL (determinism verified)"
    exit 0
else
    echo -e "\n❌ FAIL: Bounces differ (non-deterministic processing)"
    echo "Hash 1: $HASH1"
    echo "Hash 2: $HASH2"
    echo "Hash 3: $HASH3"
    echo "Hash 4: $HASH4"
    echo "Hash 5: $HASH5"
    exit 1
fi
```

**Pass Criteria**:
- ✅ ALL 5 MD5 hashes must be identical
- ✅ No sample-level differences
- ✅ No timing drift
- ✅ No phase variations

**If Test Fails**:
- Check for random number generators without fixed seed
- Check for uninitialized memory reads
- Check for non-deterministic algorithms (`std::unordered_map` iteration, etc.)
- Check for system clock dependencies in DSP code

---

## 5. Multi-DAW Compatibility Test

### 5.1 Secondary DAWs (Windows)

Test in **at least 2** of the following:

- **Ableton Live 11+**
- **Reaper 6+**
- **Studio One 5+**
- **Cubase 12+**

**Test Protocol** (each DAW):
1. Scan plugin (should appear in plugin list)
2. Instantiate on audio track
3. Load audio file, play
4. Automate one parameter (e.g., Punch)
5. Save project, close DAW, reopen
6. Verify parameter state recalled correctly

**Pass Criteria**:
- ✅ Plugin scans successfully
- ✅ Instantiates without crash
- ✅ Automation works
- ✅ Save/reload preserves state
- ✅ No dropouts or glitches during playback

### 5.2 Secondary DAWs (macOS)

Test in **at least 2** of the following:

- **Logic Pro 10.7+** (AU and VST3)
- **Ableton Live 11+** (VST3)
- **Reaper 6+** (VST3 and AU)

**Same test protocol as Windows above.**

---

## 6. Performance Validation

### 6.1 CPU Usage Test

**Test Setup**:
- DAW: Any
- Sample Rate: 48 kHz
- Buffer Size: 512 samples
- Test Signal: Pink noise, -20 dBFS, 60 seconds

**Test Cases**:

1. **Single Instance (Neutral)**
   - All parameters at default
   - Expected CPU: <2%

2. **Single Instance (Extreme)**
   - Punch=100%, Warmth=100%, Boom=100%, Drive=100%
   - SPARK enabled (8x oversampling)
   - SHINE enabled (+6 dB)
   - Expected CPU: <10%

3. **10 Instances (Typical Use)**
   - Factory preset "Punchy Drums" on each
   - Expected Total CPU: <30%

**Pass Criteria**:
- ✅ CPU usage within expected ranges
- ✅ No dropouts or glitches
- ✅ No CPU spikes >15%

### 6.2 Memory Leak Test

**Windows** (Visual Studio):
```cpp
// Add to PluginProcessor destructor (Debug builds only)
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif
```

Run 100 instantiate/destroy cycles in DAW, check for memory leaks in Output window.

**macOS** (Instruments):
```bash
# Profile standalone app for 5 minutes with automation
instruments -t Leaks BTZ.app

# Check for leaks in report
```

**Pass Criteria**:
- ✅ 0 bytes definitely lost
- ✅ 0 bytes indirectly lost
- ✅ Heap size stable over time

---

## 7. Audio Quality Spot Checks

### 7.1 Bypass Null Test

1. Import test audio to DAW
2. Duplicate track
3. Track 1: Insert BTZ, set all parameters to 0/default
4. Track 2: Leave dry (no plugin)
5. Invert phase on Track 2
6. Render to file

**Pass Criteria**:
- ✅ Output is near-silence (< -90 dBFS RMS)
- ✅ Any residual is <0.01% THD (acceptable rounding error)

### 7.2 DC Offset Check

Process 30 seconds of pink noise, measure DC offset in output.

**Pass Criteria**:
- ✅ DC offset < -90 dBFS (±0.0001 absolute value)

### 7.3 Frequency Response Check (Neutral Mode)

Process pink noise (30s, -18 dBFS RMS), compare input/output spectrum.

**Pass Criteria**:
- ✅ 20 Hz - 20 kHz: ±0.5 dB tolerance
- ✅ No subsonic rumble (<10 Hz should be -12 dB/octave rolloff)

---

## 8. Edge Case Testing

### 8.1 Extreme Sample Rates

Test at: 44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz, 192 kHz

**Pass Criteria**:
- ✅ No crashes
- ✅ No NaN/Inf in output
- ✅ Output bounded (no runaway gain)

### 8.2 Extreme Buffer Sizes

Test at: 64, 128, 512, 2048, 8192 samples

**Pass Criteria**:
- ✅ No crashes
- ✅ No glitches
- ✅ Parameter smoothing works at all buffer sizes

### 8.3 Extreme Parameter Values

- Drive = 1000% (if range allows)
- SPARK Ceiling = -20 dB (extreme limiting)
- SHINE = +12 dB (maximum boost)

**Pass Criteria**:
- ✅ No NaN/Inf in output
- ✅ Output bounded (absolute max ±1.0 for digital audio)
- ✅ No crashes

---

## 9. Final Ship Gate Report

After completing all tests above, fill out `SHIP_GATE_REPORT.md`:

```markdown
# Ship Gate Report - BTZ 1.0.0

Date: YYYY-MM-DD
Tester: [Your Name]
Build: [Git commit hash]

## Build Verification
- [ ] Windows VST3 + Standalone: PASS / FAIL
- [ ] macOS Universal VST3 + AU + Standalone: PASS / FAIL
- [ ] Linux VST3 + Standalone: PASS / FAIL

## FL Studio (Primary Target)
- [ ] Fresh scan: PASS / FAIL
- [ ] Instantiation: PASS / FAIL
- [ ] Automation: PASS / FAIL
- [ ] Save/reload: PASS / FAIL

## pluginval (Strictness 10)
- [ ] Windows VST3: PASS / FAIL
- [ ] macOS VST3: PASS / FAIL
- [ ] macOS AU (auval): PASS / FAIL

## Offline Bounce Determinism
- [ ] 5/5 bounces identical (MD5): PASS / FAIL
- MD5 hash: [hash value]

## Multi-DAW Compatibility
- [ ] Ableton Live: PASS / FAIL
- [ ] Reaper: PASS / FAIL
- [ ] [Other DAW]: PASS / FAIL

## Performance
- [ ] CPU usage within targets: PASS / FAIL
- [ ] No memory leaks: PASS / FAIL

## Audio Quality
- [ ] Bypass null test (<-90 dBFS): PASS / FAIL
- [ ] DC offset (<-90 dBFS): PASS / FAIL
- [ ] Frequency response (±0.5 dB): PASS / FAIL

## Edge Cases
- [ ] Extreme sample rates: PASS / FAIL
- [ ] Extreme buffer sizes: PASS / FAIL
- [ ] Extreme parameters: PASS / FAIL

## SHIP DECISION
- [ ] ✅ APPROVED - All gates passed, ready for release
- [ ] ⚠️ CONDITIONAL - Minor issues, document in Known Issues
- [ ] ❌ BLOCKED - Critical failures, fix before release

Known Issues:
- [List any issues found during testing]

Next Steps:
- [Action items for conditional approval or blockers]
```

---

## 10. Release Checklist

If all Ship Gates pass:

- [ ] Tag release: `git tag -a v1.0.0 -m "BTZ 1.0.0 Release"`
- [ ] Push tag: `git push origin v1.0.0`
- [ ] Create GitHub Release with artifacts (VST3/AU/Standalone for all platforms)
- [ ] Update CHANGELOG.md with release notes
- [ ] Archive build artifacts for support/debugging
- [ ] Update documentation website (if applicable)

---

**Last Updated**: 2026-02-09
**Maintained By**: BTZ QA Team
