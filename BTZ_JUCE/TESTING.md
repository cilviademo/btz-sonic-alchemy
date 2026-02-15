# BTZ - Testing & Validation Guide

**Version:** 1.0.0
**Status:** Ship-Grade Quality Assurance

---

## Overview

BTZ follows industry-standard plugin validation practices used by Waves, UAD, and Plugin Alliance. All plugins must pass automated validation before release.

**Testing Pyramid:**
1. **Unit Tests** - DSP correctness, parameter mapping, state serialization
2. **Integration Tests** - Full plugin in isolated test harness
3. **pluginval** - Official VST3/AU validator (MANDATORY)
4. **Host Matrix** - Real-world DAW compatibility testing

---

## 1. AUTOMATED VALIDATION (pluginval)

### What is pluginval?

pluginval is the **official** VST3/AU validation tool from Tracktion. It's the industry standard for plugin compliance testing.

**Tests performed:**
- ✅ Real-time safety (no allocations/locks in audio thread)
- ✅ Parameter stability and automation compliance
- ✅ State save/load round-trips
- ✅ Bus layout negotiation
- ✅ Sample rate and buffer size stress testing
- ✅ Suspend/resume cycles
- ✅ Threading violations

**Strictness levels:**
- Level 5: Basic compliance (minimum for hobbyist plugins)
- **Level 10: Professional** (BTZ target - matches commercial standards)

### Installation

**macOS:**
```bash
brew install pluginval
```

**Linux:**
```bash
# Download latest release
wget https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Linux.zip
unzip pluginval_Linux.zip
sudo mv pluginval /usr/local/bin/
chmod +x /usr/local/bin/pluginval
```

**Windows:**
Download from https://github.com/Tracktion/pluginval/releases

### Running Validation

**Automated Script (Recommended):**
```bash
cd BTZ_JUCE
./scripts/run_pluginval.sh
```

**Manual Validation:**
```bash
# VST3 (macOS/Linux)
pluginval --strictness-level 10 \
          --validate-in-process \
          --timeout-ms 30000 \
          --verbose \
          build/BTZ_artefacts/Release/VST3/BTZ.vst3

# AU (macOS only)
pluginval --strictness-level 10 \
          --validate-in-process \
          --timeout-ms 30000 \
          --verbose \
          build/BTZ_artefacts/Release/AU/BTZ.component
```

### Interpreting Results

**PASS:** All tests passed - plugin is compliant ✅
```
...
Results: 0 plugins failed, 1 passed
```

**FAIL:** See error output for specific violations ❌

**Common Failures:**

| Error | Cause | Fix |
|-------|-------|-----|
| RT violation | Allocation in processBlock | Remove malloc/new/locks from audio thread |
| State mismatch | getStateInformation/setStateInformation broken | Verify XML round-trip |
| Parameter ID conflict | Duplicate IDs | Ensure all parameter IDs are unique |
| Bus layout rejection | isBusesLayoutSupported too restrictive | Support mono/stereo |
| Timeout | Infinite loop or deadlock | Check for unbounded algorithms |

---

## 2. UNIT TESTS (Coming Soon)

**Test Categories:**
- DSP invariants (NaN/Inf detection, denormal protection)
- Parameter mapping (normalized ↔ plain conversions)
- State serialization (save → load → save equivalence)

**Running:**
```bash
cd build
ctest --output-on-failure
```

---

## 3. REAL-TIME SAFETY VERIFICATION

### Memory Allocation Detection

**macOS (Instruments):**
```bash
# 1. Launch DAW with Instruments
instruments -t "Allocations" -D trace.trace /Applications/Ableton\ Live.app

# 2. Load BTZ plugin
# 3. Process audio for 30 seconds
# 4. Stop and analyze

# 5. Filter for malloc/new calls
# Look for calls from BTZAudioProcessor::processBlock
```

**Linux (Valgrind):**
```bash
# Build standalone
cd build && cmake --build . --target BTZ_Standalone

# Run with Massif (heap profiler)
valgrind --tool=massif --massif-out-file=massif.out ./BTZ_Standalone

# Analyze
ms_print massif.out | grep processBlock
# Should show ZERO allocations in processBlock
```

**Expected Result:** ZERO allocations during processBlock

---

## 4. HOST COMPATIBILITY MATRIX

### Minimum Test Matrix (Required Before Ship)

| DAW | Version | Platform | Critical Tests |
|-----|---------|----------|----------------|
| **Pro Tools** | 2023+ | macOS/Win | RT violations log, AAX compat (future) |
| **Logic Pro** | 10.8+ | macOS | auval, automation, HiDPI |
| **Ableton Live** | 11/12 | macOS/Win | 32-sample buffer + automation |
| **FL Studio** | 21+ | Win | Call order resilience, state save |
| **Reaper** | 7+ | macOS/Win/Linux | Stress test (1000 instances) |
| **Bitwig** | 5+ | macOS/Linux | Multi-rate processing |

### Test Protocol (Per Host)

**1. Basic Smoke Test:**
- [ ] Insert BTZ on audio track
- [ ] Adjust all parameters (verify UI updates)
- [ ] Automate 3+ parameters simultaneously
- [ ] Save project
- [ ] Close and reopen project
- [ ] Verify parameters restored correctly

**2. Automation Test:**
- [ ] Create fast automation (1-second sweep 0→1→0)
- [ ] Render/bounce audio
- [ ] Listen for zipper noise or glitches
- [ ] Test at 32, 64, 128, 256, 512 sample buffer sizes

**3. Stress Test:**
- [ ] Load 10+ instances on separate tracks
- [ ] Process simultaneously
- [ ] Monitor CPU usage (should be stable)
- [ ] Check for dropouts or clicks

**4. State Persistence:**
- [ ] Set unique parameter values (e.g., punch=0.73, warmth=0.42)
- [ ] Save project
- [ ] Quit DAW
- [ ] Reopen project
- [ ] Verify exact parameter values restored

**5. Suspend/Resume:**
- [ ] Process audio
- [ ] Change buffer size mid-session (if DAW supports)
- [ ] Verify no crashes or glitches

### Pro Tools Specific (RT Violations)

```bash
# Enable RT violation logging
# Preferences → Playback Engine → Show RTAS/AAX Performance Window

# Process audio with BTZ
# Check for violations in:
#   Setup → Playback Engine → Performance

# Expected: ZERO RT violations
```

### Logic Pro Specific (auval)

```bash
# Run official AU validation
auval -v aufx Btzp Btzz

# Expected output:
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# AU Validation Tool
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# ...
# --------------------------------------------------
# PASSED: All tests succeeded
```

---

## 5. DSP QUALITY VERIFICATION

### Parameter Smoothing Test

**Reproducer:**
1. Set DAW buffer size to 32 samples
2. Load BTZ on audio track with sine wave (440Hz)
3. Automate "Punch" parameter: fast sweep 0→1→0 over 1 second
4. Record output
5. Analyze in spectral editor

**Expected:** Smooth parameter changes, no audible stepping/zipper

### Anti-Aliasing Test

**Reproducer:**
1. Generate 18kHz sine wave
2. Process through BTZ with Warmth=1.0 (saturation)
3. Analyze spectrum up to 24kHz

**Expected:**
- Harmonics at 36kHz, 54kHz (outside audible range - oversampled)
- NO aliases below Nyquist frequency (22.05kHz @ 44.1kHz)

### Denormal Protection Test

**Reproducer:**
1. Load BTZ with all parameters at default
2. Send silence (0.0 samples) for 60 seconds
3. Monitor CPU usage

**Expected:** CPU usage stays constant (no denormal slowdown)

### Latency Compensation Test

**Reproducer:**
1. Create two tracks with identical audio
2. Track 1: Bypass BTZ
3. Track 2: BTZ active (Spark OFF, all processing OFF)
4. Invert polarity of Track 2
5. Sum tracks

**Expected:** Perfect phase cancellation (silence)
**If not silent:** Latency compensation incorrect

---

## 6. PERFORMANCE BENCHMARKING

### CPU Profiling

**macOS (Instruments):**
```bash
instruments -t "Time Profiler" -D trace.trace /Applications/Ableton\ Live.app
# Load BTZ, process for 60 seconds
# Analyze processBlock CPU %
```

**Linux (perf):**
```bash
perf record -g ./BTZ_Standalone
perf report
```

**Target Metrics:**
- **Single instance:** < 5% CPU @ 48kHz, 512-sample buffer (2019 MacBook Pro baseline)
- **Scalability:** Linear scaling up to 100 instances
- **Silence:** < 1% CPU when input is silent (silence optimization active)

### Memory Footprint

```bash
# macOS
instruments -t "Allocations" -D trace.trace /Applications/Ableton\ Live.app

# Linux
valgrind --tool=massif ./BTZ_Standalone
ms_print massif.out
```

**Target Metrics:**
- **Heap:** < 1MB per instance
- **Stack:** < 64KB per instance
- **Growth:** Zero heap growth after initialization

---

## 7. REGRESSION TESTING

### Golden Reference Tests (Future)

```bash
# Render known input through BTZ with specific settings
# Compare output to golden reference (statistical comparison)
./scripts/regression_test.sh

# Expected: < 0.001 dB deviation from reference
```

---

## 8. CONTINUOUS INTEGRATION (Planned)

### GitHub Actions Workflow

```yaml
name: Validate
on: [push, pull_request]
jobs:
  pluginval:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release
          cmake --build .
      - name: Install pluginval
        run: brew install pluginval
      - name: Validate
        run: ./scripts/run_pluginval.sh
```

---

## 9. DEFINITION OF DONE - VALIDATION CHECKLIST

### Required for Beta Release

- [ ] pluginval VST3 passes (strictness 10)
- [ ] pluginval AU passes (strictness 10) *(macOS)*
- [ ] Pro Tools: No RT violations
- [ ] Logic Pro: auval passes *(macOS)*
- [ ] Ableton: 32-sample automation smooth
- [ ] FL Studio: State save/load works
- [ ] Zero allocations in processBlock (Instruments/Valgrind)
- [ ] Parameter smoothing test passes
- [ ] Latency compensation verified

### Required for v1.0 Release

- [ ] All beta requirements ✓
- [ ] Reaper stress test (100+ instances)
- [ ] Bitwig multi-rate test
- [ ] 24-hour soak test (no crashes/leaks)
- [ ] Regression suite passes
- [ ] Performance benchmarks within targets

---

## 10. TROUBLESHOOTING GUIDE

### "pluginval failed: RT violation"

**Diagnosis:**
```bash
# Run with verbose output
pluginval --strictness-level 10 --verbose <plugin-path> 2>&1 | grep -A 5 "RT"
```

**Common Causes:**
- DBG() or juce::String in processBlock
- std::vector growth, std::map access
- Mutex locks (std::lock_guard)
- File I/O, network calls
- std::shared_ptr refcount contention

**Fix:** Use RTSafeLogger, pre-allocate buffers, defer to message thread

### "State doesn't restore correctly"

**Diagnosis:**
```bash
# Enable XML logging in debugger
# Check getStateInformation() output
```

**Common Causes:**
- Parameter IDs changed between saves
- Missing version attributes
- XML corruption handling missing
- Floating point precision issues

**Fix:** Add version migration, validate all XML fields

### "auval fails on macOS"

**Common Failures:**
- Bus layout not supported
- Missing required Info.plist keys
- Sandboxing violations (file access)
- Code signing issues

**Fix:**
```bash
# Check exact failure
auval -v aufx Btzp Btzz -w -de -strict

# Common fixes:
codesign --force --sign - BTZ.component
```

---

## 11. RELEASE CHECKLIST

**Pre-Release Validation:**
1. ✅ Run `./scripts/run_pluginval.sh`
2. ✅ Test in Pro Tools (RT violations = 0)
3. ✅ Test in Logic (auval pass)
4. ✅ Test in Ableton (32-sample buffer)
5. ✅ Test in FL Studio (state save/load)
6. ✅ Verify CPU usage < 5% single instance
7. ✅ Verify no memory leaks (24-hour test)
8. ✅ Version number matches release tag

**Sign-Off:**
- [ ] DSP Lead: Sound quality verified
- [ ] QA Lead: All tests pass
- [ ] Release Engineer: Packages built and signed
- [ ] PM: Release notes finalized

---

**BTZ Audio Systems Group**
*Evidence-Based Engineering. Ship-Grade Quality.*
