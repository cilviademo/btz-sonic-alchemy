# BTZ Manual DAW Testing Guide

**Version**: 1.0.0
**Purpose**: Ship-Ready Gate - Verify plugin behavior in production DAW environments
**Target**: Minimum 3 DAWs tested (Reaper + Ableton + FL Studio)

---

## 🎯 Ship Gate Requirements

To pass Ship Gate #2, the plugin must demonstrate **ZERO CRASHES** in all scenarios across all tested DAWs.

**Required Test Coverage**:
- ✅ Reaper (reference host, excellent VST3 support)
- ✅ Ableton Live (32-sample buffer stress test)
- ✅ FL Studio (aggressive automation, state management)

**Optional Extended Coverage**:
- Cubase (VST3 reference implementation)
- Studio One (common compatibility issues)
- Pro Tools (AAX if shipping, otherwise skip)

---

## 📋 Test Matrix

Each DAW must pass ALL of the following tests:

| Test ID | Test Name | Expected Result |
|---------|-----------|----------------|
| T1 | Plugin scan/load | Plugin appears in effects list |
| T2 | Instantiation | Plugin loads without crash |
| T3 | Audio processing | Clean audio output |
| T4 | Parameter automation | Smooth, no zippers |
| T5 | State save/load | Parameters persist correctly |
| T6 | Freeze/bounce | Output deterministic |
| T7 | Multi-instance | 10+ instances stable |
| T8 | Editor open/close | No crashes or freezes |
| T9 | Buffer size changes | No glitches |
| T10 | Sample rate changes | No glitches |
| T11 | Engine suspend/resume | Clean resume |
| T12 | Offline bounce | Matches realtime |

---

## 🔧 Test Execution

### Test 1: Plugin Scan & Load

**Objective**: Verify DAW can discover and load the plugin

**Steps**:
1. Copy plugin to correct location:
   - **Windows VST3**: `C:\Program Files\Common Files\VST3\BTZ - The Box Tone Zone.vst3`
   - **macOS VST3**: `/Library/Audio/Plug-Ins/VST3/BTZ - The Box Tone Zone.vst3`
   - **macOS AU**: `/Library/Audio/Plug-Ins/Components/BTZ - The Box Tone Zone.component`

2. Rescan plugins in DAW (or restart DAW)
3. Search for "BTZ" in effects browser

**Expected Result**: ✅ Plugin appears in effects list under "BTZ Audio" vendor

**Failure Criteria**: ❌ Plugin not found, greyed out, or shows error message

---

### Test 2: Instantiation

**Objective**: Verify plugin can be created without crash

**Steps**:
1. Create new audio track
2. Add "BTZ - The Box Tone Zone" to track
3. Wait for plugin to load
4. Verify UI opens (if using default editor)

**Expected Result**: ✅ Plugin loads within 2 seconds, no error dialogs

**Failure Criteria**: ❌ Crash, freeze, error message, or UI doesn't appear

---

### Test 3: Audio Processing

**Objective**: Verify clean audio output

**Steps**:
1. Load test audio file (drum loop or pink noise)
2. Add BTZ to track
3. Play audio
4. Listen for:
   - Clicks/pops
   - Distortion
   - Silence
   - Artifacts

**Expected Result**: ✅ Clean audio output, no artifacts

**Failure Criteria**: ❌ Clicks, pops, distortion, or complete silence

**Test File**: Use `test_assets/drum_loop_48k.wav` (provided)

---

### Test 4: Parameter Automation

**Objective**: Verify smooth parameter changes

**Steps**:
1. Create automation for "Punch" parameter
2. Draw rapid sawtooth wave (5Hz modulation)
3. Play and listen for zipper noise
4. Repeat for "Warmth" and "Mix" parameters

**Expected Result**: ✅ Smooth parameter changes, no zippers/clicks

**Failure Criteria**: ❌ Audible stepping, clicks, or glitches

**Recording**: Record 10 seconds to `results/[DAW]_automation_test.wav`

---

### Test 5: State Save/Load

**Objective**: Verify parameters persist across save/load

**Steps**:
1. Set all parameters to specific values:
   - Punch: 0.75
   - Warmth: 0.5
   - Boom: 0.3
   - Mix: 0.8
2. Save project
3. Close DAW
4. Reopen project
5. Verify all parameters match

**Expected Result**: ✅ All parameters load correctly

**Failure Criteria**: ❌ Parameters reset to defaults or wrong values

**Screenshot**: Capture before and after comparison

---

### Test 6: Freeze/Bounce

**Objective**: Verify deterministic output

**Steps**:
1. Load drum loop
2. Add BTZ with moderate settings
3. Bounce/export audio (realtime)
4. Import bounced audio
5. Bounce again
6. Compare waveforms (should be identical)

**Expected Result**: ✅ Identical waveforms (bit-perfect or < 0.1dB difference)

**Failure Criteria**: ❌ Different output on repeated bounces

**Files**: Save `results/[DAW]_bounce_1.wav` and `results/[DAW]_bounce_2.wav`

---

### Test 7: Multi-Instance Stress

**Objective**: Verify stability with multiple instances

**Steps**:
1. Create 10 audio tracks
2. Add BTZ to each track
3. Load different drum loops on each
4. Play all simultaneously
5. Monitor CPU usage
6. Run for 5 minutes

**Expected Result**: ✅ No crashes, CPU stable

**Failure Criteria**: ❌ Crash, freeze, or runaway CPU

**Log**: Record CPU % and any errors in `results/[DAW]_multi_instance.txt`

---

### Test 8: Editor Open/Close Cycles

**Objective**: Verify UI stability

**Steps**:
1. Add BTZ to track
2. Open editor
3. Close editor
4. Repeat 20 times rapidly
5. Play audio while opening/closing

**Expected Result**: ✅ No crashes, UI responds smoothly

**Failure Criteria**: ❌ Crash, freeze, or UI artifacts

---

### Test 9: Buffer Size Changes

**Objective**: Verify robustness across buffer sizes

**Steps**:
1. Add BTZ to track with audio
2. Start playback
3. While playing, change buffer size:
   - 32 samples (if supported)
   - 64 samples
   - 128 samples
   - 512 samples
   - 2048 samples
4. Listen for glitches

**Expected Result**: ✅ Clean transitions, no glitches

**Failure Criteria**: ❌ Clicks, pops, or crashes on buffer change

**Notes**: Some DAWs require stop/start for buffer change

---

### Test 10: Sample Rate Changes

**Objective**: Verify handling of different sample rates

**Steps**:
1. Create projects at each sample rate:
   - 44.1 kHz
   - 48 kHz
   - 88.2 kHz
   - 96 kHz
2. Add BTZ to each
3. Process same audio file
4. Listen for artifacts

**Expected Result**: ✅ Clean audio at all sample rates

**Failure Criteria**: ❌ Artifacts, crashes, or wrong pitch

---

### Test 11: Engine Suspend/Resume

**Objective**: Verify clean audio engine resume

**Steps**:
1. Add BTZ to track
2. Play audio
3. Stop playback
4. Wait 30 seconds
5. Resume playback
6. Listen for initialization clicks

**Expected Result**: ✅ Clean resume, no clicks

**Failure Criteria**: ❌ Click on resume or distorted audio

---

### Test 12: Offline Bounce vs Realtime

**Objective**: Verify consistent output

**Steps**:
1. Setup project with BTZ on drum loop
2. Bounce realtime (1x speed)
3. Bounce offline (fastest possible)
4. Compare waveforms

**Expected Result**: ✅ Identical or < 0.1dB difference

**Failure Criteria**: ❌ Audible difference or waveform mismatch

---

## 📊 Results Reporting

### Result Template

Copy this template for each DAW tested:

```markdown
## [DAW NAME] v[VERSION] Test Results

**Date**: YYYY-MM-DD
**Tester**: [Name]
**OS**: [macOS/Windows] [Version]
**CPU**: [Model]
**Plugin Version**: BTZ v1.0.0
**Plugin Format**: [VST3/AU/AAX]

### Test Results

| Test ID | Test Name | Result | Notes |
|---------|-----------|--------|-------|
| T1 | Plugin scan/load | ✅/❌ | |
| T2 | Instantiation | ✅/❌ | |
| T3 | Audio processing | ✅/❌ | |
| T4 | Parameter automation | ✅/❌ | |
| T5 | State save/load | ✅/❌ | |
| T6 | Freeze/bounce | ✅/❌ | |
| T7 | Multi-instance | ✅/❌ | CPU: __% |
| T8 | Editor open/close | ✅/❌ | |
| T9 | Buffer size changes | ✅/❌ | |
| T10 | Sample rate changes | ✅/❌ | |
| T11 | Engine suspend/resume | ✅/❌ | |
| T12 | Offline bounce | ✅/❌ | |

### Summary

**Total Tests**: 12
**Passed**: __
**Failed**: __
**Overall Result**: PASS/FAIL

### Critical Issues

[List any critical bugs found]

### Non-Critical Issues

[List any minor issues or notes]

### Artifacts

- [ ] Automation test recording: `results/[DAW]_automation_test.wav`
- [ ] Bounce comparison: `results/[DAW]_bounce_1.wav`, `results/[DAW]_bounce_2.wav`
- [ ] Multi-instance log: `results/[DAW]_multi_instance.txt`
- [ ] Screenshots: `results/[DAW]_screenshots/`
```

---

## 🎬 Automated Collection Script

Run this script to bundle all results:

```bash
#!/bin/bash
# collect_daw_test_results.sh

RESULTS_DIR="results"
ARCHIVE_NAME="BTZ_DAW_Test_Results_$(date +%Y%m%d_%H%M%S).zip"

echo "Collecting BTZ DAW test results..."

# Create archive
zip -r "$ARCHIVE_NAME" "$RESULTS_DIR"/*.wav "$RESULTS_DIR"/*.txt "$RESULTS_DIR"/*.md "$RESULTS_DIR"/screenshots/

echo "✓ Results packaged: $ARCHIVE_NAME"
echo "Upload this file for review."
```

---

## 📝 Notes

### Critical Test Priorities

1. **T2 (Instantiation)** - Ship blocker
2. **T3 (Audio Processing)** - Ship blocker
3. **T4 (Parameter Automation)** - Ship blocker
4. **T5 (State Save/Load)** - Ship blocker
5. **T7 (Multi-Instance)** - High priority

### Platform-Specific Notes

**Windows**:
- Test with ASIO and WASAPI drivers
- Check Windows Defender doesn't flag plugin

**macOS**:
- Test both Intel and Apple Silicon if possible
- Verify Gatekeeper doesn't block plugin
- Run `auval` for AU validation

### Known DAW Quirks

**Ableton Live**:
- Freezes aggressively (good stress test)
- 32-sample buffer is common (test this!)

**FL Studio**:
- Aggressive automation engine
- State management can be tricky

**Reaper**:
- Excellent error reporting
- Good reference for correct behavior

---

## ✅ Ship Gate Pass Criteria

**PASS** if:
- All 12 tests pass in all 3 DAWs
- Zero crashes
- Zero critical bugs

**FAIL** if:
- Any crashes
- Any critical bugs (data loss, corruption, etc.)
- More than 2 non-critical bugs across all DAWs

---

**Last Updated**: 2026-01-08
**Maintained By**: BTZ Ship Team
