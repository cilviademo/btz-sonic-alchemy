# BTZ P2 Acceptance Criteria & Verification Plan

**Version**: 1.0
**Date**: 2026-01-15
**Status**: Draft
**Author**: BTZ Development Team

---

## Executive Summary

This document defines **verification procedures** for all P2 requirements. Each acceptance criterion maps to a specific requirement in `P2_REQUIREMENTS.md` and provides testable, objective verification steps.

**Purpose**: Ensure P2 delivers production-ready quality (95% completion, beta-ready status).

---

## 1. Stability Requirements Acceptance Criteria

### AC-STAB-001: DSP Unit Test Suite
**Maps to**: REQ-STAB-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**1.1 Test Framework Setup**
- [ ] Google Test framework compiles successfully on all platforms (Win/Mac/Linux)
- [ ] CMake target `BTZ_Tests` builds without errors
- [ ] `ctest` command discovers all test suites
- [ ] Test executable runs standalone (outside CMake)

**1.2 AdvancedSaturation Tests (8 minimum)**
- [ ] **Test 1**: `testBypassMode()` - Input == Output with tolerance 0.0001
- [ ] **Test 2**: `testTubeMode()` - Soft clipping verified at ±1.0
- [ ] **Test 3**: `testTapeMode()` - Frequency-dependent saturation (HF rolloff)
- [ ] **Test 4**: `testConsoleMode()` - Harmonic analysis shows 2nd/3rd harmonics
- [ ] **Test 5**: `testFuzzMode()` - Hard clipping asymmetry verified
- [ ] **Test 6**: `testDigitalMode()` - Bit reduction artifacts present
- [ ] **Test 7**: `testExtremeDrive()` - Drive=1.0 doesn't crash, THD <10%
- [ ] **Test 8**: `testZeroInput()` - Silence in → silence out

**1.3 EnhancedSPARK Tests (10 minimum)**
- [ ] **Test 1**: `testBypass()` - Limiter disabled → unity gain
- [ ] **Test 2**: `testTruePeakDetection()` - 4x oversampled peak detection
- [ ] **Test 3**: `testITUCompliance()` - -1.0 dBTP ceiling enforced
- [ ] **Test 4**: `testHysteresisModel()` - Jiles-Atherton equations verified
- [ ] **Test 5**: `testReleaseCurves()` - 5 release modes (slow/fast/auto/adaptive/vintage)
- [ ] **Test 6**: `testGainReduction()` - GR meter matches actual attenuation
- [ ] **Test 7**: `testLookahead()` - 5ms lookahead prevents overs
- [ ] **Test 8**: `testStereoLinking()` - L/R channels limited identically
- [ ] **Test 9**: `testMakeupGain()` - Auto-makeup compensates reduction
- [ ] **Test 10**: `testExtremeCeiling()` - -20dB ceiling doesn't distort

**1.4 EnhancedSHINE Tests (8 minimum)**
- [ ] **Test 1**: `testBarkBands()` - 24 critical bands verified
- [ ] **Test 2**: `testPsychoacousticWeighting()` - Fletcher-Munson curve applied
- [ ] **Test 3**: `testFrequencyResponse()` - Sine sweep shows correct boost
- [ ] **Test 4**: `testPhaseCoherence()` - Linear-phase verified (group delay constant)
- [ ] **Test 5**: `testBypass()` - Zero processing when disabled
- [ ] **Test 6**: `testLowFrequencyProtection()` - <80Hz unaffected
- [ ] **Test 7**: `testHighFrequencyBoost()` - 8-16kHz enhanced per amount
- [ ] **Test 8**: `testExtremeSHINE()` - Amount=1.0 doesn't cause resonance

**1.5 LUFSMeter Tests (10 minimum)**
- [ ] **Test 1**: `testKWeighting()` - Pre-filter matches ITU-R BS.1770-4 spec
- [ ] **Test 2**: `testMomentaryLUFS()` - 400ms integration window
- [ ] **Test 3**: `testShortTermLUFS()` - 3s integration window
- [ ] **Test 4**: `testIntegratedLUFS()` - Full program loudness
- [ ] **Test 5**: `testGatingThreshold()` - -70 LUFS absolute, -10 relative
- [ ] **Test 6**: `testCalibrationTone()` - -23 LUFS test signal verified
- [ ] **Test 7**: `testStereoSumming()` - L+R correctly weighted
- [ ] **Test 8**: `testDynamicRange()` - PLR (Peak to Loudness Ratio) calculated
- [ ] **Test 9**: `testSilenceHandling()` - Zeros don't cause NaN/inf
- [ ] **Test 10**: `testResetState()` - Clear history resets to -inf

**1.6 TransientShaper Tests (6 minimum)**
- [ ] **Test 1**: `testAttackEnhancement()` - Envelope follower detects transients
- [ ] **Test 2**: `testReleaseShaping()` - Sustain attenuation verified
- [ ] **Test 3**: `testADAA()` - Anti-derivative anti-aliasing reduces artifacts
- [ ] **Test 4**: `testEnvelopeFollower()` - RMS vs Peak detection modes
- [ ] **Test 5**: `testBypass()` - Zero processing when punch=0
- [ ] **Test 6**: `testExtremePunch()` - Punch=1.0 doesn't clip

**1.7 ParameterSmoother Tests (5 minimum)**
- [ ] **Test 1**: `testLinearRamp()` - 20ms ramp time verified
- [ ] **Test 2**: `testTargetConvergence()` - Reaches target within tolerance
- [ ] **Test 3**: `testRTSafety()` - No allocations (Valgrind check)
- [ ] **Test 4**: `testImmediateChange()` - Zero remaining samples → instant
- [ ] **Test 5**: `testSmallChanges()` - Changes <0.0001 ignored (optimization)

#### Code Coverage Requirements:
- [ ] Overall DSP code coverage ≥80% (measured by gcov/lcov)
- [ ] Critical paths (processBlock, parameter updates) ≥95%
- [ ] Generated coverage report in `build/coverage/index.html`

#### Acceptance Criteria Summary:
- ✅ All 47+ tests pass (0 failures)
- ✅ Tests run in CI/CD pipeline automatically
- ✅ Code coverage report generated and meets thresholds
- ✅ No memory leaks (Valgrind: 0 bytes definitely lost)
- ✅ Test execution time <30 seconds total

---

### AC-STAB-002: Memory Safety Verification
**Maps to**: REQ-STAB-002
**Priority**: P0 (Must-Have)

#### Verification Steps:

**2.1 Valgrind (Linux)**
```bash
valgrind --leak-check=full \
         --track-origins=yes \
         --num-callers=50 \
         ./build/BTZ_artefacts/Debug/Standalone/BTZ
```
- [ ] Run for 10,000 buffer cycles (approx 2 minutes @ 512 samples)
- [ ] "definitely lost: 0 bytes in 0 blocks"
- [ ] "indirectly lost: 0 bytes in 0 blocks"
- [ ] All allocations accounted for in startup phase

**2.2 AddressSanitizer (All Platforms)**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build
./build/BTZ_Tests
```
- [ ] Zero heap-buffer-overflow errors
- [ ] Zero use-after-free errors
- [ ] Zero stack-buffer-overflow errors

**2.3 ThreadSanitizer (macOS/Linux)**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
cmake --build build
./build/BTZ_Tests
```
- [ ] Zero data race warnings
- [ ] Zero deadlock detections
- [ ] processBlock() confirmed lock-free

**2.4 Xcode Instruments (macOS)**
- [ ] Allocations template: 0 allocations in processBlock()
- [ ] Leaks template: 0 leaks after 5-minute session
- [ ] Time Profiler: processBlock() <0.5ms @ 512 samples

#### Acceptance Criteria Summary:
- ✅ Valgrind reports zero leaks
- ✅ ASan/TSan pass with zero errors
- ✅ Instruments confirms RT-safety
- ✅ Automated sanitizer checks in CI/CD

---

### AC-STAB-003: Robust Model File Loading
**Maps to**: REQ-STAB-003
**Priority**: P1 (Should-Have)

#### Verification Steps:

**3.1 Missing Model File**
- [ ] Delete `BTZ_JUCE/Source/Models/DeepFilterNet2.tar.gz`
- [ ] Launch plugin → verify no crash
- [ ] Console log shows: "WARNING: ML model not found, using rule-based transients"
- [ ] Plugin continues with fallback algorithm
- [ ] GUI shows warning indicator (optional)

**3.2 Corrupted Model File**
- [ ] Create dummy `.pt` file (random bytes)
- [ ] Launch plugin → verify no crash
- [ ] Log shows: "ERROR: Failed to load model, reverting to defaults"
- [ ] Parameters still functional

**3.3 User-Visible Error**
- [ ] Missing model → error message in GUI footer
- [ ] Message: "ML features unavailable (model not found)"
- [ ] Non-blocking (plugin still usable)

#### Acceptance Criteria Summary:
- ✅ Missing model → fallback + warning (no crash)
- ✅ Corrupted model → default params (no crash)
- ✅ Error message visible to user
- ✅ Graceful degradation documented in logs

---

## 2. Performance Requirements Acceptance Criteria

### AC-PERF-001: Sample-Accurate Parameter Smoothing
**Maps to**: REQ-PERF-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**1.1 Automation Recording Test (Reaper)**
1. Load BTZ on drum bus
2. Automate `punch` parameter: 0.0 → 1.0 over 1 second
3. Set buffer size to 32 samples (worst case)
4. Record output to new track
5. Analyze waveform for stepping artifacts

**Pass Criteria**:
- [ ] No visible stepping in waveform
- [ ] Spectral analysis shows no aliasing >20kHz
- [ ] Zipper noise <-60 dBFS (inaudible)

**1.2 Smoothing Time Verification**
```cpp
// Unit test pseudo-code
ParameterSmoother smoother;
smoother.prepare(48000.0, 0.02); // 20ms
smoother.setTarget(1.0);

int samples = 0;
while (smoother.getNextValue() < 0.99) {
    samples++;
}

EXPECT_EQ(samples, 960); // 20ms @ 48kHz
```
- [ ] Ramp time measured: 20ms ±1ms
- [ ] Linear interpolation verified (constant slope)

**1.3 CPU Overhead Test**
```bash
# RenderPerformanceTest at 48kHz, 512 samples
./build/BTZ_Tests --gtest_filter=*RenderPerformance*
```
- [ ] With smoothing: <5.0% CPU (2015 MacBook Pro)
- [ ] Overhead increase: <2% vs block-rate smoothing
- [ ] Benchmark passes on reference hardware

**1.4 DAW Compatibility Test**
Test in multiple DAWs with fast automation:
- [ ] Reaper (32-sample buffer): No zipper noise
- [ ] Ableton Live (64-sample buffer): Smooth ramps
- [ ] Pro Tools (128-sample buffer): No clicks
- [ ] Logic Pro (256-sample buffer): Professional quality

#### Acceptance Criteria Summary:
- ✅ Per-sample smoothing for 5 critical params (punch, warmth, boom, drive, mix)
- ✅ 20ms ramp time measured accurately
- ✅ CPU increase <2%
- ✅ Automation verified smooth at 32 samples
- ✅ No zipper noise at -60 dBFS noise floor

---

### AC-PERF-002: Unified Oversampling Strategy
**Maps to**: REQ-PERF-002
**Priority**: P1 (Should-Have)

#### Verification Steps:

**2.1 CPU Reduction Measurement**
```bash
# Before (cascaded oversampling)
./build/BTZ_Tests --gtest_filter=*CPUBenchmark*
# Record: X% CPU

# After (unified oversampling)
./build/BTZ_Tests --gtest_filter=*CPUBenchmark*
# Record: Y% CPU
# Expected: Y = 0.80 * X (20% reduction)
```
- [ ] CPU reduction ≥15% measured
- [ ] Target: 20% reduction achieved

**2.2 Aliasing Performance (THD+N Test)**
```cpp
// 10kHz sine wave at 48kHz sample rate
// Measure aliasing products at 38kHz (48-10)
float thdBeforeUnification = measureTHD(10000.0f);
float thdAfterUnification = measureTHD(10000.0f);

EXPECT_LT(thdAfterUnification, 0.0001); // <0.01%
EXPECT_LE(thdAfterUnification, thdBeforeUnification); // Not worse
```
- [ ] THD+N <0.01% @ 10kHz sine
- [ ] Aliasing maintained or improved vs cascaded

**2.3 Architecture Verification**
- [ ] Only ONE oversampling stage in PluginProcessor::processBlock()
- [ ] OversamplingProcessor removed or integrated
- [ ] EnhancedSPARK internal oversampling disabled
- [ ] Signal flow diagram updated

#### Acceptance Criteria Summary:
- ✅ Single 4x oversampling stage at PluginProcessor level
- ✅ All nonlinear modules process upsampled signal
- ✅ 15-20% CPU reduction measured
- ✅ THD+N <0.01% maintained
- ✅ Architecture simplified (code review confirms)

---

### AC-PERF-003: Lock-Free Parameter Updates
**Maps to**: REQ-PERF-003
**Priority**: P0 (Must-Have)

#### Verification Steps:

**3.1 ThreadSanitizer Verification**
```bash
cmake -B build -DENABLE_TSAN=ON
./build/BTZ_Standalone &
# Automate parameters rapidly while playing audio
```
- [ ] Zero lock contention warnings
- [ ] Zero data race warnings

**3.2 Code Audit**
```cpp
// Verify all smoothed parameters use std::atomic
std::atomic<float> punchTarget;
std::atomic<float> warmthTarget;
std::atomic<float> boomTarget;
std::atomic<float> driveTarget;
std::atomic<float> mixTarget;
```
- [ ] All 28 parameters use std::atomic or lock-free mechanism
- [ ] APVTS parameter callbacks never lock audio thread
- [ ] Manual code review passes

**3.3 Stress Test**
- [ ] Automate 5 parameters simultaneously @ 1ms intervals
- [ ] Run for 10 minutes
- [ ] Verify zero audio dropouts (DAW buffer underruns)
- [ ] Verify zero glitches in output

#### Acceptance Criteria Summary:
- ✅ ThreadSanitizer confirms zero lock contention
- ✅ std::atomic verified for all parameters
- ✅ APVTS updates confirmed non-blocking
- ✅ Stress test passes without dropouts

---

## 3. Maintainability Requirements Acceptance Criteria

### AC-MAINT-001: DSP Module API Reference
**Maps to**: REQ-MAINT-001
**Priority**: P1 (Should-Have)

#### Verification Steps:

**1.1 Doxygen Documentation**
```bash
# Generate docs
doxygen Doxyfile
# Open build/docs/html/index.html
```
- [ ] Doxygen generates without warnings
- [ ] All public methods have @brief
- [ ] All parameters documented with @param
- [ ] All return values documented with @return
- [ ] Complex algorithms include @note with math

**1.2 Coverage Audit**
Check these critical files:
- [ ] `AdvancedSaturation.h`: 6 saturation modes documented
- [ ] `EnhancedSPARK.h`: Jiles-Atherton equations explained
- [ ] `EnhancedSHINE.h`: 24 Bark bands referenced
- [ ] `LUFSMeter.h`: ITU-R BS.1770-4 compliance noted
- [ ] `TransientShaper.h`: ADAA algorithm explained
- [ ] `ParameterSmoother.h`: Smoothing curves documented

**1.3 References to Inspirations**
- [ ] DPF (DISTRHO Plugin Framework) cited where applicable
- [ ] JUCE DSP examples referenced
- [ ] Academic papers linked (e.g., Jiles-Atherton 1986)

#### Acceptance Criteria Summary:
- ✅ Every public method documented (@brief, @param, @return, @note)
- ✅ Complex modules include mathematical models
- ✅ Open-source inspirations cited
- ✅ Generated HTML docs viewable in browser

---

### AC-MAINT-002: Architecture Documentation
**Maps to**: REQ-MAINT-002
**Priority**: P1 (Should-Have)

#### Verification Steps:

**2.1 ARCHITECTURE.md Completeness**
- [ ] Signal flow diagram (Mermaid/PlantUML)
- [ ] Module dependency graph
- [ ] Threading model explained (message thread vs audio thread)
- [ ] Parameter flow: GUI → APVTS → Smoother → DSP
- [ ] Preset system architecture

**2.2 PARAMETERS.md Completeness**
For each of 28 parameters:
- [ ] Range documented (min/max)
- [ ] Effect described (what it does sonically)
- [ ] Interactions noted (e.g., "warmth + drive → stacking")
- [ ] Recommended values (e.g., "punch: 0.3-0.5 for gentle")

**2.3 Developer Onboarding Test**
- [ ] New developer reads ARCHITECTURE.md
- [ ] Can explain signal chain in <30 minutes
- [ ] Can locate where punch parameter affects DSP
- [ ] Understands threading model

#### Acceptance Criteria Summary:
- ✅ ARCHITECTURE.md: Signal flow, dependencies, threading
- ✅ PARAMETERS.md: All 28 params documented with interactions
- ✅ Diagrams render correctly (Mermaid/PlantUML)
- ✅ New developer onboarding <30 minutes

---

### AC-MAINT-003: Continuous Integration Pipeline
**Maps to**: REQ-MAINT-003
**Priority**: P1 (Should-Have)

#### Verification Steps:

**3.1 GitHub Actions Workflow**
- [ ] `.github/workflows/ci.yml` exists
- [ ] Builds on: Ubuntu 22.04, macOS latest, Windows 2022
- [ ] Builds VST3 on all platforms
- [ ] Runs unit tests automatically
- [ ] Generates coverage reports

**3.2 Build Quality Gates**
- [ ] Compiler warnings treated as errors (`-Werror` / `/WX`)
- [ ] All tests must pass (0 failures)
- [ ] Code coverage ≥80% required to merge
- [ ] Build artifacts uploaded (VST3 binaries)

**3.3 Pull Request Checks**
- [ ] PR creation triggers CI build
- [ ] Status checks block merge if failing
- [ ] Coverage report posted as comment
- [ ] Automated checks: format, lint, security scan

#### Acceptance Criteria Summary:
- ✅ GitHub Actions workflow for Linux/macOS/Windows
- ✅ Builds VST3 on all platforms
- ✅ Runs unit tests + integration tests
- ✅ Fails on compiler warnings
- ✅ Generates test coverage reports
- ✅ PR checks enforce quality gates

---

## 4. UX Requirements Acceptance Criteria

### AC-UX-001: Undo/Redo System
**Maps to**: REQ-UX-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**1.1 Basic Undo/Redo**
1. Load plugin
2. Change punch: 0.0 → 0.5
3. Change warmth: 0.0 → 0.7
4. Click Undo button (or Cmd+Z)
5. Verify warmth reverts to 0.0
6. Click Undo again
7. Verify punch reverts to 0.0
8. Click Redo button (or Cmd+Shift+Z)
9. Verify punch → 0.5

**Pass Criteria**:
- [ ] Undo reverts last parameter change
- [ ] Redo restores undone change
- [ ] Undo stack max 100 changes (circular buffer)
- [ ] Buttons grayed out when stack empty

**1.2 Multi-Parameter Snapshot**
1. Change punch, warmth, boom simultaneously
2. Click Undo
3. Verify ALL THREE revert to previous state

- [ ] Snapshot captures ALL 28 parameters
- [ ] Undo/Redo atomic (all params or none)

**1.3 Preset Load Behavior**
1. Make changes (undo stack populated)
2. Load preset "Pop Punch"
3. Verify undo stack cleared (preset is new baseline)

- [ ] Preset load clears undo stack
- [ ] Preset load creates new snapshot

**1.4 RT-Safety**
- [ ] Snapshots taken on message thread (not audio thread)
- [ ] ThreadSanitizer confirms no blocking in processBlock()

**1.5 DAW Integration**
- [ ] Test in Ableton Live: Edit → Undo works
- [ ] Test in Pro Tools: Cmd+Z works
- [ ] Test in Logic Pro: Undo menu item works

#### Acceptance Criteria Summary:
- ✅ Undo/Redo buttons in GUI (Cmd+Z, Cmd+Shift+Z)
- ✅ State snapshot stack (max 100, circular buffer)
- ✅ Undo reverts ALL parameters since last snapshot
- ✅ RT-safe implementation (message thread only)
- ✅ Undo/Redo state persists across preset loads
- ✅ Visual feedback: grayed-out when stack empty
- ✅ DAW Edit menu integration works

---

### AC-UX-002: Preset Browser UI
**Maps to**: REQ-UX-002
**Priority**: P1 (Should-Have)

#### Verification Steps:

**2.1 Modal Dialog**
- [ ] "Browse" button opens modal preset browser
- [ ] Dialog displays: Factory (5 presets) + User (custom)
- [ ] List view shows preset names and categories

**2.2 Load/Save/Delete/Rename**
- [ ] Click preset → "Load" button enables
- [ ] Load preset → parameters update
- [ ] "Save As" button creates new user preset
- [ ] "Delete" button removes user preset (factory protected)
- [ ] "Rename" button edits user preset name

**2.3 Preview Mode**
- [ ] Click preset → temporary load (preview)
- [ ] Cancel → revert to original state
- [ ] OK → commit changes

**2.4 Search/Filter**
- [ ] Search box filters by name
- [ ] Tag filter: "drums", "master", "gentle"
- [ ] Results update in real-time

**2.5 Backend Integration**
- [ ] Connects to PresetManager (PresetManager.h:75-120)
- [ ] User presets saved to: `~/Documents/BTZ/UserPresets/`
- [ ] Preset files are XML (.btzpreset)

#### Acceptance Criteria Summary:
- ✅ Modal dialog triggered by "Browse" button
- ✅ List view: Factory (5) + User (custom)
- ✅ Load/Save/Delete/Rename operations
- ✅ Preview mode (load without commit)
- ✅ Search/filter by tag
- ✅ Connects to PresetManager backend

---

### AC-UX-003: MeterStrip Visualization
**Maps to**: REQ-UX-003
**Priority**: P1 (Should-Have)

#### Verification Steps:

**3.1 LUFS Meter**
- [ ] Color zones: green (<-23), yellow (-23 to -14), red (>-14)
- [ ] Numeric readout: "-14.2 LUFS"
- [ ] Updates at 30Hz (smooth, not flickering)

**3.2 True Peak Indicator**
- [ ] Red bar at 0 dBFS
- [ ] Flashes red on clip (>0 dBTP)
- [ ] Peak hold for 2 seconds
- [ ] Numeric readout: "-0.1 dBTP"

**3.3 Gain Reduction Needle**
- [ ] Range: 0-20dB
- [ ] Smooth ballistics (10ms attack, 100ms release)
- [ ] Matches SPARK limiter's actual GR

**3.4 Stereo Correlation Graph**
- [ ] Range: +1 (mono) to -1 (out-of-phase)
- [ ] Goniometer or correlation meter
- [ ] Updates in real-time

**3.5 Preset Target Zones**
- [ ] "Streaming: -14 LUFS" overlay
- [ ] "Broadcast: -23 LUFS" overlay
- [ ] Preset-specific targets highlighted

#### Acceptance Criteria Summary:
- ✅ LUFS meter: green/yellow/red zones, numeric readout
- ✅ True Peak: red bar at 0 dBFS, flashes on clip
- ✅ Gain Reduction: 0-20dB needle, smooth ballistics
- ✅ Stereo Correlation: +1 to -1 graph
- ✅ Preset target zones overlaid
- ✅ 30Hz update rate (Quick Win 1 already optimized)

---

### AC-UX-004: Tooltip Enhancements
**Maps to**: REQ-UX-004
**Priority**: P2 (Nice-to-Have)

#### Verification Steps:

**4.1 Current Value Display**
- [ ] Hover punch knob → "Punch: 0.45"
- [ ] Hover warmth knob → "Warmth: 0.67"
- [ ] Value updates as knob moves

**4.2 Dynamic Tips**
- [ ] SPARK enabled + limiting → "SPARK: Limiting 3.2dB"
- [ ] SPARK disabled → "SPARK: Off (enable to protect peaks)"

**4.3 Delay Timing**
- [ ] Tooltip appears 500ms after hover (macOS standard)
- [ ] Tooltip disappears immediately on mouse move

#### Acceptance Criteria Summary:
- ✅ Tooltips show current parameter value
- ✅ Dynamic tips based on state
- ✅ 500ms delay before showing
- ✅ Follows macOS/Windows platform conventions

---

## 5. Backward Compatibility Acceptance Criteria

### AC-COMPAT-001: Parameter ID Preservation
**Maps to**: REQ-COMPAT-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**1.1 Session File Compatibility**
1. Create session in v0.8.0 (P1 build)
2. Save session with automation on punch, warmth, boom
3. Open session in v1.0.0 (P2 build)
4. Verify all parameters load correctly
5. Verify automation data intact

**Pass Criteria**:
- [ ] All 28 parameter IDs unchanged from P1
- [ ] Session loads without "unknown parameter" warnings
- [ ] Automation data plays back correctly

**1.2 Preset Compatibility**
- [ ] Load P1-era preset "Pop Punch" in P2 build
- [ ] Verify all parameters map correctly
- [ ] No missing/orphaned parameters

**1.3 Deprecated Parameter Handling**
(If any parameters removed in P2)
- [ ] Migration layer converts old IDs → new IDs
- [ ] User notified via console log
- [ ] Session still loads (graceful degradation)

#### Acceptance Criteria Summary:
- ✅ All 28 parameter IDs remain unchanged
- ✅ v0.8.0 sessions load in v1.0.0
- ✅ Deprecated parameters handled via migration layer
- ✅ Zero breaking changes for existing users

---

## 6. Platform-Specific Acceptance Criteria

### AC-PLAT-WIN-001: ASIO Buffer Size Compatibility
**Maps to**: REQ-PLAT-WIN-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**Test Matrix**:
| Buffer Size | Sample Rate | Pass Criteria |
|-------------|-------------|---------------|
| 32 samples  | 48kHz       | No clicks/pops, CPU <10% |
| 64 samples  | 48kHz       | No clicks/pops, CPU <7% |
| 128 samples | 48kHz       | No clicks/pops, CPU <5% |
| 256 samples | 48kHz       | No clicks/pops, CPU <4% |
| 512 samples | 48kHz       | No clicks/pops, CPU <3% |
| 1024 samples| 48kHz       | No clicks/pops, CPU <2.5% |
| 2048 samples| 48kHz       | No clicks/pops, CPU <2% |

**Test Procedure**:
- [ ] Load BTZ in Reaper (Windows)
- [ ] Set ASIO buffer size via preferences
- [ ] Play drum loop for 5 minutes
- [ ] Monitor CPU meter (Reaper Performance Meter)
- [ ] Listen for clicks/pops (headphones, -24 LUFS playback)

#### Acceptance Criteria Summary:
- ✅ Tested with 7 buffer sizes: 32-2048 samples
- ✅ No clicks/pops at 32 samples (worst case)
- ✅ CPU usage within targets
- ✅ Parameter automation smooth at all buffer sizes

---

### AC-PLAT-MAC-001: Apple Silicon Optimization
**Maps to**: REQ-PLAT-MAC-001
**Priority**: P1 (Should-Have)

#### Verification Steps:

**1.1 Universal Binary**
```bash
lipo -info build/BTZ_artefacts/Release/VST3/BTZ.vst3/Contents/MacOS/BTZ
# Expected output: Architectures in the fat file: x86_64 arm64
```
- [ ] Binary contains both x86_64 and arm64 slices

**1.2 NEON SIMD Optimizations**
- [ ] Code uses vDSP (Accelerate framework) where applicable
- [ ] ARM-specific optimizations in critical loops (if any)

**1.3 M1 Max Testing**
- [ ] Tested on M1 Max @ 44.1kHz, 64-sample buffers
- [ ] CPU usage <3% (native ARM performance)
- [ ] No Rosetta translation required

#### Acceptance Criteria Summary:
- ✅ Universal binary (x86_64 + arm64)
- ✅ NEON SIMD optimizations (if applicable)
- ✅ Verified on M1 Max @ 44.1kHz, 64 samples
- ✅ Native ARM performance (no Rosetta)

---

### AC-PLAT-LIN-001: JACK/ALSA Compatibility
**Maps to**: REQ-PLAT-LIN-001
**Priority**: P1 (Should-Have)

#### Verification Steps:

**1.1 JACK Backend Testing**
```bash
# Start JACK
jackd -d alsa -r 48000 -p 128
# Launch Reaper (Linux)
# Load BTZ plugin
```
- [ ] Plugin loads in Reaper (Linux)
- [ ] Audio processes correctly
- [ ] No Xruns at 128-sample buffers (Ubuntu 22.04)

**1.2 ALSA Testing**
- [ ] Direct ALSA backend works (if supported)
- [ ] PulseAudio bridge works

**1.3 VST3 Installation Path**
```bash
ls ~/.vst3/BTZ.vst3
# Or: ls /usr/lib/vst3/BTZ.vst3
```
- [ ] VST3 installs to correct path
- [ ] DAW scans and discovers plugin

#### Acceptance Criteria Summary:
- ✅ Tested in Reaper (Linux) with JACK backend
- ✅ No Xruns at 128-sample buffers (Ubuntu 22.04)
- ✅ Correct VST3 installation path (~/.vst3/)
- ✅ Plugin discoverable by DAW scanner

---

## 7. Non-Functional Acceptance Criteria

### AC-NFR-001: CMake Cross-Platform Build
**Maps to**: REQ-NFR-001
**Priority**: P0 (Must-Have)

#### Verification Steps:

**7.1 Clean Build Test**
```bash
# Windows
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release

# macOS
cmake -B build -GXcode
cmake --build build --config Release

# Linux
cmake -B build
cmake --build build --config Release
```
- [ ] Builds successfully on all 3 platforms
- [ ] No hardcoded paths (uses CMake variables)
- [ ] `WITH_ML=ON` flag works (optional Torch integration)

**7.2 Dependency Management**
- [ ] JUCE submodule auto-detected
- [ ] External libraries (libsamplerate, etc.) found via find_package()
- [ ] No manual path editing required

#### Acceptance Criteria Summary:
- ✅ Single CMakeLists.txt builds on Win/Mac/Linux
- ✅ Optional `WITH_ML=ON` flag for Torch integration
- ✅ No hardcoded paths (CMake variables only)
- ✅ Clean build from scratch succeeds

---

### AC-NFR-002: Open Source Compliance
**Maps to**: REQ-NFR-002
**Priority**: P0 (Must-Have)

#### Verification Steps:

**2.1 LICENSES.md Completeness**
- [ ] JUCE: GPL/Commercial dual-license noted
- [ ] libsamplerate: BSD-2-Clause
- [ ] Google Test: BSD-3-Clause
- [ ] DeepFilterNet2: License verified (Apache 2.0 or compatible)

**2.2 License Headers**
- [ ] All .cpp/.h files include license header
- [ ] Copyright year current (2026)

**2.3 Third-Party Attribution**
- [ ] CREDITS.md lists all dependencies
- [ ] Acknowledgments for DPF, JUCE examples

#### Acceptance Criteria Summary:
- ✅ LICENSES.md documents all third-party licenses
- ✅ DeepFilterNet2 model license allows redistribution
- ✅ All source files have license headers
- ✅ CREDITS.md acknowledges inspirations

---

## 8. Success Metrics & Quality Gates

### Release Criteria for P2 (95% Complete, Beta-Ready)

**8.1 Functional Completeness**
- [ ] All P0 requirements implemented (Must-Have)
- [ ] ≥80% of P1 requirements implemented (Should-Have)
- [ ] P2 requirements optional (Nice-to-Have)

**8.2 Quality Metrics**
- [ ] DSP unit test coverage ≥80%
- [ ] Zero critical bugs (crashes, memory leaks, data corruption)
- [ ] Zero P0 bugs open
- [ ] <5 P1 bugs open

**8.3 Performance Benchmarks**
| Metric | Target | Verified | Status |
|--------|--------|----------|--------|
| DSP Unit Test Coverage | 80% | __% | ⬜ |
| CPU Usage (Release) | <5% | __% | ⬜ |
| Memory Allocations (RT) | 0 | __ | ⬜ |
| Parameter Smoothing | Per-sample | Block/Sample | ⬜ |
| GUI Frame Rate | 30Hz | __Hz | ⬜ |
| Build Time (Release) | <5 min | __min | ⬜ |

**8.4 Manual Testing Checklist**
- [ ] All test cases in `SHIP_GATE_MANUAL_TEST.md` pass
- [ ] Beta tester feedback collected (≥5 users)
- [ ] No regressions from P1 (existing features still work)

---

## 9. Regression Testing Requirements

### 9.1 P1 Feature Regression Tests

**Must verify these P1 features still work after P2 changes**:

**DSP Core**:
- [ ] Adaptive punch/warmth/boom/drive processing intact
- [ ] SPARK limiter still ITU BS.1770-compliant
- [ ] LUFS metering accurate (verify with -23 LUFS tone)
- [ ] RT-safe parameter smoothing (20ms ramping)

**GUI**:
- [ ] Custom MainView-based editor renders correctly
- [ ] A/B/C preset system still click-free
- [ ] 5 factory presets load correctly
- [ ] Real-time metering works (LUFS, peak, GR, stereo)
- [ ] Tooltips on all 13 controls (from Quick Win 4)
- [ ] Double-click reset works (from Quick Win 3)
- [ ] Meter rendering optimization (from Quick Win 1)

**Production Safety**:
- [ ] Zero allocations in audio thread (Valgrind)
- [ ] Lock-free parameter updates (ThreadSanitizer)
- [ ] DAW quirks detection still works

### 9.2 Automated Regression Suite

**Create regression test suite**:
```bash
./build/BTZ_Tests --gtest_filter=*Regression*
```

**Tests to include**:
- [ ] `testP1FactoryPresetsLoad()` - All 5 presets valid
- [ ] `testP1ParameterRanges()` - All params within bounds
- [ ] `testP1SignalFlow()` - Sine in → expected sine out
- [ ] `testP1MeterAccuracy()` - LUFS meter ±0.1 LUFS accurate

---

## 10. Beta Testing Acceptance Criteria

### 10.1 Beta Program Requirements

**Beta Tester Recruitment**:
- [ ] ≥5 beta testers recruited
- [ ] Mix of professionals (mixing engineers, producers)
- [ ] Platform diversity: 2 macOS, 2 Windows, 1 Linux

**Beta Feedback Collection**:
- [ ] Google Form or survey for structured feedback
- [ ] Questions: Stability, CPU usage, sound quality, UX
- [ ] Bug reports tracked in GitHub Issues

**Beta Exit Criteria**:
- [ ] ≥80% testers rate stability ≥4/5
- [ ] ≥70% testers rate sound quality ≥4/5
- [ ] Zero critical bugs reported
- [ ] All P0 bugs fixed before public release

---

## 11. Documentation Acceptance Criteria

### 11.1 User-Facing Documentation

**User Manual (USER_MANUAL.md)**:
- [ ] Overview of BTZ features
- [ ] Parameter descriptions (all 28 params)
- [ ] Preset guide (when to use each preset)
- [ ] Troubleshooting section
- [ ] System requirements

**Quick Start Guide**:
- [ ] Installation instructions (Win/Mac/Linux)
- [ ] First-time setup (DAW configuration)
- [ ] Basic workflow (load preset, adjust params)

### 11.2 Developer Documentation

**Already Created**:
- ✅ `P2_REQUIREMENTS.md` (this doc's companion)
- ✅ `P2_DESIGN.md` (architecture decisions)
- ✅ `P2_TASK_BREAKDOWN.md` (78-task roadmap)

**To Create**:
- [ ] `ARCHITECTURE.md` (REQ-MAINT-002)
- [ ] `PARAMETERS.md` (REQ-MAINT-002)
- [ ] Doxygen API reference (REQ-MAINT-001)
- [ ] `CONTRIBUTING.md` (open-source contribution guide)

---

## 12. Final Acceptance Checklist

Before declaring P2 complete, verify:

### Phase 1: Must-Have (Ship-Blocking)
- [ ] AC-STAB-001: DSP Unit Test Suite (47+ tests, 80% coverage)
- [ ] AC-STAB-002: Memory Safety (Valgrind/ASan/TSan pass)
- [ ] AC-UX-001: Undo/Redo System (professional-grade)
- [ ] AC-PERF-001: Sample-Accurate Smoothing (20ms, <2% CPU)
- [ ] AC-COMPAT-001: Parameter ID Preservation (backward compat)
- [ ] AC-PLAT-WIN-001: ASIO Buffer Compatibility (32-2048 samples)
- [ ] AC-NFR-001: CMake Cross-Platform Build
- [ ] AC-NFR-002: Open Source Compliance

### Phase 2: Should-Have (Quality Critical)
- [ ] AC-UX-003: MeterStrip Visualization (LUFS/peak/GR/stereo)
- [ ] AC-UX-002: Preset Browser UI (load/save/search)
- [ ] AC-MAINT-001: DSP API Reference (Doxygen)
- [ ] AC-PERF-002: Unified Oversampling (15-20% CPU reduction)
- [ ] AC-MAINT-003: CI/CD Pipeline (GitHub Actions)
- [ ] AC-PLAT-MAC-001: Apple Silicon Optimization
- [ ] AC-PLAT-LIN-001: JACK/ALSA Compatibility

### Phase 3: Nice-to-Have (Polish)
- [ ] AC-MAINT-002: Architecture Documentation
- [ ] AC-UX-004: Enhanced Tooltips (current values)
- [ ] AC-STAB-003: Robust Model Loading (graceful degradation)

### Final Sign-Off
- [ ] All Phase 1 (P0) criteria met
- [ ] ≥80% Phase 2 (P1) criteria met
- [ ] All automated tests pass (CI/CD green)
- [ ] Manual test checklist complete
- [ ] Beta testing complete (≥5 testers, ≥4/5 rating)
- [ ] Documentation complete (user + developer)
- [ ] Zero known critical bugs
- [ ] Regression tests pass (P1 features intact)

---

## 13. Traceability Matrix

This table maps acceptance criteria back to requirements:

| AC ID | Requirement ID | Priority | Verification Method | Est. Hours |
|-------|----------------|----------|---------------------|------------|
| AC-STAB-001 | REQ-STAB-001 | P0 | Automated tests + coverage report | 80-120 |
| AC-STAB-002 | REQ-STAB-002 | P0 | Valgrind/ASan/TSan | 20-30 |
| AC-STAB-003 | REQ-STAB-003 | P1 | Manual testing | 10-15 |
| AC-PERF-001 | REQ-PERF-001 | P0 | Unit tests + DAW automation | 20-30 |
| AC-PERF-002 | REQ-PERF-002 | P1 | CPU benchmark + THD test | 30-50 |
| AC-PERF-003 | REQ-PERF-003 | P0 | ThreadSanitizer + code audit | 5-10 |
| AC-MAINT-001 | REQ-MAINT-001 | P1 | Doxygen generation | 15-20 |
| AC-MAINT-002 | REQ-MAINT-002 | P1 | Documentation review | 25-35 |
| AC-MAINT-003 | REQ-MAINT-003 | P1 | CI/CD pipeline run | 15-25 |
| AC-UX-001 | REQ-UX-001 | P0 | Manual DAW testing | 30-50 |
| AC-UX-002 | REQ-UX-002 | P1 | Manual GUI testing | 30-40 |
| AC-UX-003 | REQ-UX-003 | P1 | Visual verification | 20-30 |
| AC-UX-004 | REQ-UX-004 | P2 | Manual GUI testing | 10-15 |
| AC-COMPAT-001 | REQ-COMPAT-001 | P0 | Session load test | 0 (design) |
| AC-PLAT-WIN-001 | REQ-PLAT-WIN-001 | P0 | Buffer size matrix | 5-10 |
| AC-PLAT-MAC-001 | REQ-PLAT-MAC-001 | P1 | M1 testing | 15-25 |
| AC-PLAT-LIN-001 | REQ-PLAT-LIN-001 | P1 | JACK testing | 10-15 |
| AC-NFR-001 | REQ-NFR-001 | P0 | Clean build test | 0 (verify) |
| AC-NFR-002 | REQ-NFR-002 | P0 | License audit | 5-10 |

**Total Verification Effort**: 320-480 hours (matches implementation estimate)

---

## 14. Test Execution Schedule

**Week 1-2: Core DSP Testing**
- Execute AC-STAB-001 (unit tests)
- Execute AC-STAB-002 (memory safety)
- Execute AC-PERF-003 (lock-free verification)

**Week 3-4: Performance Testing**
- Execute AC-PERF-001 (sample-accurate smoothing)
- Execute AC-PERF-002 (unified oversampling)
- Run CPU benchmarks

**Week 5-6: UX Testing**
- Execute AC-UX-001 (undo/redo)
- Execute AC-UX-002 (preset browser)
- Execute AC-UX-003 (meter visualization)

**Week 7-8: Platform Testing**
- Execute AC-PLAT-WIN-001 (Windows ASIO)
- Execute AC-PLAT-MAC-001 (Apple Silicon)
- Execute AC-PLAT-LIN-001 (Linux JACK)

**Week 9: Documentation & CI**
- Execute AC-MAINT-001 (Doxygen)
- Execute AC-MAINT-002 (architecture docs)
- Execute AC-MAINT-003 (CI/CD pipeline)

**Week 10: Final Validation**
- Regression testing (P1 features)
- Beta testing (5 users)
- Manual test checklist
- Final sign-off

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-15 | BTZ Team | Initial acceptance criteria specification |

---

**Approval Status**: Draft → Review → Approved
**Next Steps**: Implementation (P2_IMPLEMENTATION.md) + Test Execution

---

## Appendix A: Test Data Files

**Required test assets**:
- `tests/assets/sine_1kHz_-20dBFS.wav` (1kHz reference tone)
- `tests/assets/sine_10kHz_-20dBFS.wav` (aliasing test tone)
- `tests/assets/calibration_-23LUFS.wav` (LUFS calibration tone)
- `tests/assets/drum_loop_stereo.wav` (transient shaper test)
- `tests/assets/white_noise.wav` (noise floor test)

**Generate test tones**:
```python
# generate_test_tones.py
import numpy as np
import scipy.io.wavfile as wav

sr = 48000
duration = 5.0
t = np.linspace(0, duration, int(sr * duration))

# 1kHz sine at -20 dBFS
sine_1k = 0.1 * np.sin(2 * np.pi * 1000 * t)
wav.write('sine_1kHz_-20dBFS.wav', sr, sine_1k.astype(np.float32))

# 10kHz sine for aliasing test
sine_10k = 0.1 * np.sin(2 * np.pi * 10000 * t)
wav.write('sine_10kHz_-20dBFS.wav', sr, sine_10k.astype(np.float32))

# -23 LUFS calibration tone (ITU-R BS.1770-4)
# RMS = 10^((-23 - (-0.691)) / 20) ≈ 0.0689
lufs_tone = 0.0689 * np.sin(2 * np.pi * 1000 * t)
wav.write('calibration_-23LUFS.wav', sr, lufs_tone.astype(np.float32))
```

---

## Appendix B: Manual Test Checklist Template

**BTZ P2 Manual Test Checklist**
**Tester**: __________
**Date**: __________
**Platform**: Windows / macOS / Linux
**DAW**: __________
**Version**: __________

### Test 1: Plugin Loads
- [ ] VST3 discovered by DAW
- [ ] GUI opens without crash
- [ ] All controls visible

### Test 2: Parameter Automation
- [ ] Automate punch: smooth ramp, no zipper noise
- [ ] Automate mix: smooth crossfade
- [ ] Automate SPARK ceiling: limiter responds

### Test 3: Preset System
- [ ] Load "Gentle Glue" → verify parameters
- [ ] A/B/C switching: click-free
- [ ] Save custom preset → reload → verify

### Test 4: Undo/Redo
- [ ] Change parameters → Undo → verify revert
- [ ] Redo → verify restore
- [ ] Undo stack limit: 100 changes

### Test 5: Metering
- [ ] LUFS meter: play -23 LUFS tone → verify -23 ±0.5
- [ ] True peak: clip detector flashes on >0 dBTP
- [ ] Gain reduction: SPARK limiting shown

### Test 6: CPU Usage
- [ ] Reaper Performance Meter: <5% CPU @ 512 samples
- [ ] No audio dropouts during 10-minute session

### Test 7: Session Persistence
- [ ] Save DAW session → close → reopen → verify state

**Overall Rating**: ⭐⭐⭐⭐⭐ (1-5 stars)
**Comments**: ____________________

---

**End of P2 Acceptance Criteria Document**
