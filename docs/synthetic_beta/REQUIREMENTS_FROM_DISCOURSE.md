# Discourse-Derived Requirements for BTZ

**Generated:** 2026-01-14
**Source:** DISCOURSE_DATASET.md analysis
**Purpose:** Translate user complaints into testable, ship-blocking requirements

---

## R1: Parameter Automation Must Be Artifact-Free

**Requirement ID:** REQ-AUTO-001
**Priority:** P0 (SHIP-BLOCKER)
**Derived From:** Complaint Pattern #1

### Acceptance Criteria

**AC1.1:** Parameter sweeps produce no audible zipper noise
- **Test:** Automate Drive 0→100% over 1 second
- **PASS:** FFT shows no spectral artifacts > -80dB

**AC1.2:** All smoothable parameters use sample-rate smoothing
- **Test:** Code review of parameter updates in processBlock
- **PASS:** Critical params (Drive, Gain, Mix) call `getNextValue()` per-sample, not per-block

**AC1.3:** FL Studio 64-step automation is compensated
- **Test:** Simulate 64-step automation input, measure output smoothness
- **PASS:** No audible steps, even with 64-step input

### Implementation Status
- ⚠️ **Current:** Block-rate smoothing (20ms) via `SmoothedValue`
- 🔧 **Required:** Per-sample smoothing in critical sections

### Verification Method
```cpp
AutomationArtifactTest:
  - Generate linear ramp automation (0→1 over 100ms)
  - Process through BTZ with Drive parameter
  - FFT analysis of output
  - Check for aliasing artifacts
  - PASS if THD+N < -80dB
```

---

## R2: Bypass Transitions Must Be Click-Free

**Requirement ID:** REQ-BYPASS-001
**Priority:** P0 (SHIP-BLOCKER)
**Derived From:** Complaint Pattern #2

### Acceptance Criteria

**AC2.1:** Bypass toggle produces no transient spikes
- **Test:** Toggle bypass every 100ms during 1kHz sine playback
- **PASS:** Peak transients < -60dBFS

**AC2.2:** Crossfade time is configurable (default 10ms)
- **Test:** Measure bypass transition duration
- **PASS:** Smooth 10ms crossfade observed

**AC2.3:** Bypass is bit-perfect when possible (VST3 BypassParameter)
- **Test:** Process silence with bypass on
- **PASS:** Output = input bit-exact (or documented difference)

### Implementation Status
- ✅ **Available:** ClickFreeSwitch in SafetyLayer (10ms crossfade)
- ⚠️ **Not Integrated:** Not used in PluginProcessor bypass logic

### Verification Method
```cpp
BypassClickTest:
  - Generate 1kHz sine wave
  - Toggle bypass 100 times over 10 seconds
  - Detect transients > -10dBFS
  - PASS if zero transients detected
```

---

## R3: CPU Usage Must Be Predictable and Bounded

**Requirement ID:** REQ-PERF-001
**Priority:** P0 (SHIP-BLOCKER)
**Derived From:** Complaint Pattern #3

### Acceptance Criteria

**AC3.1:** CPU variance is < 10% of average
- **Test:** Process 1000 blocks, measure stddev of CPU%
- **PASS:** stddev < 10% of mean

**AC3.2:** Worst-case CPU is < 2x average
- **Test:** Process 1000 blocks with worst-case input (white noise, full drive)
- **PASS:** max CPU < 2x average CPU

**AC3.3:** No sustained CPU spikes > 5 seconds
- **Test:** Monitor CPU over 5 minutes of playback
- **PASS:** No spikes > 2x average lasting > 5 seconds

**AC3.4:** Denormal protection is active and effective
- **Test:** Process 1000 blocks of silence, measure CPU
- **PASS:** CPU remains constant (no denormal spike)

### Implementation Status
- ✅ **Implemented:** PerformanceMonitor, denormal protection (4 layers)
- ⚠️ **Not Integrated:** PerformanceGuardrails not active in PluginProcessor

### Verification Method
```cpp
CPUStabilityTest:
  - Run 1000 processBlock calls with random input
  - Measure CPU% per block (high-resolution timer)
  - Calculate: min, max, avg, stddev
  - PASS if: max < 2*avg AND stddev < 0.1*avg
```

---

## R4: Plugin Must Pass Strictest Validation

**Requirement ID:** REQ-VALID-001
**Priority:** P0 (SHIP-BLOCKER)
**Derived From:** Complaint Pattern #4

### Acceptance Criteria

**AC4.1:** pluginval strictness-level 10 passes with 0 failures
- **Test:** `pluginval --strictness-level 10 BTZ.vst3`
- **PASS:** Exit code 0, no failures

**AC4.2:** auval passes on macOS (AU format)
- **Test:** `auval -v aufx BTZ Btzz`
- **PASS:** "PASSED" message, exit code 0

**AC4.3:** AAX validation passes in Pro Tools (when AAX build is available)
- **Test:** Pro Tools plugin scan, load test
- **PASS:** Plugin appears in list, loads without errors

**AC4.4:** Plugin init time < 500ms (prevents scan timeout)
- **Test:** Measure time from DLL load to constructor completion
- **PASS:** < 500ms on reference hardware

### Implementation Status
- ⚠️ **Not Tested:** No automated pluginval/auval in CI
- ⚠️ **AAX:** Not built yet

### Verification Method
```bash
ValidationSuite:
  pluginval --strictness-level 10 --validate-in-process BTZ.vst3
  auval -v aufx BTZ Btzz  # macOS only
  # AAX: Manual Pro Tools scan test
  PASS if all return success
```

---

## R5: Session Recall Must Be Deterministic

**Requirement ID:** REQ-STATE-001
**Priority:** P1 (CRITICAL)
**Derived From:** Complaint Pattern #5

### Acceptance Criteria

**AC5.1:** State round-trip is bit-exact
- **Test:** Set all parameters, save state, load state, compare
- **PASS:** All parameters match exactly

**AC5.2:** Component variance is deterministic (same seed = same sound)
- **Test:** Load preset 10 times, compare audio output
- **PASS:** Bit-exact audio output (or documented tolerance)

**AC5.3:** State includes version tag for migration
- **Test:** Inspect serialized state data
- **PASS:** Version number present (e.g., "BTZ_v1.0.0")

**AC5.4:** Corrupted state loads safely (no crash)
- **Test:** Feed garbage data to setStateInformation()
- **PASS:** No crash, graceful fallback to defaults

### Implementation Status
- ✅ **Partial:** ComponentVariance uses seeded PRNG
- ⚠️ **Gap:** No state versioning or CRC checksum

### Verification Method
```cpp
StateRoundTripTest:
  - Set all parameters to test values (non-default)
  - Call getStateInformation(destData)
  - Create new processor instance
  - Call setStateInformation(destData)
  - Compare all parameter values
  - PASS if bit-exact match
```

---

## R6: GUI Must Scale Correctly on HiDPI Displays

**Requirement ID:** REQ-UI-001
**Priority:** P2 (IMPORTANT)
**Derived From:** Complaint Pattern #6

### Acceptance Criteria

**AC6.1:** UI scales correctly at 100%, 150%, 200% DPI
- **Test:** Manual visual inspection on 4K/Retina displays
- **PASS:** Sharp text, proportional sizing

**AC6.2:** UI elements use scaled units (not fixed pixels)
- **Test:** Code review of GUI sizing code
- **PASS:** Uses JUCE scaling API, no hardcoded pixel sizes

### Implementation Status
- ✅ **Likely OK:** Modern JUCE UI with theme system
- ⚠️ **Not Tested:** No HiDPI testing performed

### Verification Method
```
Manual:
  1. Open BTZ on 4K display (200% DPI)
  2. Check text sharpness and button sizes
  3. Compare with native DAW UI
  PASS if equally sharp and proportional
```

---

## R7: AAX Threading Model Must Be RT-Safe

**Requirement ID:** REQ-AAX-001
**Priority:** P0 (SHIP-BLOCKER for AAX)
**Derived From:** Complaint Pattern #7

### Acceptance Criteria

**AC7.1:** No shared mutable state between parameter and audio threads
- **Test:** Code review + ThreadSanitizer
- **PASS:** Zero data races detected

**AC7.2:** setParameter can be called concurrently with processBlock
- **Test:** Spawn param thread + audio thread, hammer both
- **PASS:** No crashes, no corruption, no hangs

**AC7.3:** Parameter updates are lock-free (no blocking in audio thread)
- **Test:** Profile processBlock with setParameter spam
- **PASS:** No locks detected in processBlock

### Implementation Status
- ⚠️ **Current:** APVTS uses atomics (should be safe)
- ⚠️ **Unknown:** Not tested with concurrent access
- ❌ **AAX:** Not built yet

### Verification Method
```cpp
AAXThreadingTest:
  std::thread audioThread([&]{
    for(int i=0; i<1000; ++i) processor.processBlock(buffer, midi);
  });
  std::thread paramThread([&]{
    for(int i=0; i<1000; ++i) processor.setParameterNotifyingHost(0, random());
  });
  audioThread.join(); paramThread.join();
  PASS if no crashes, no TSAN warnings
```

---

## R8: Cross-DAW Compatibility

**Requirement ID:** REQ-COMPAT-001
**Priority:** P1 (CRITICAL)
**Derived From:** Complaint Patterns #8, #9

### Acceptance Criteria

**AC8.1:** BTZ works in all major DAWs
- **Test Matrix:**
  - Pro Tools (AAX) - Win, macOS
  - Logic Pro (AU) - macOS
  - Ableton Live (VST3) - Win, macOS
  - FL Studio (VST3) - Win
  - Reaper (VST3) - Win, macOS, Linux
  - Studio One (VST3) - Win, macOS
  - Cubase/Nuendo (VST3) - Win, macOS
- **PASS:** Loads, processes audio, automation works in all

**AC8.2:** FL Studio automation is smooth (no 64-step artifacts)
- **Test:** Automate Drive in FL Studio, listen for stepping
- **PASS:** Smooth automation, no audible artifacts

**AC8.3:** Ableton parameter names display correctly
- **Test:** Load VST3 in Live, check automation lane names
- **PASS:** Shows "Drive", "Punch" (not "Param1")

### Implementation Status
- ⚠️ **Not Tested:** No cross-DAW testing performed yet

### Verification Method
```
Manual cross-DAW test protocol:
  For each DAW:
    1. Load BTZ
    2. Process audio (drums, bass, vocals)
    3. Automate 3 parameters
    4. Save & reload project
    5. Check sonic consistency
  PASS if all DAWs behave identically
```

---

## Summary of Requirements

| Req ID | Title | Priority | Status | Tests |
|--------|-------|----------|--------|-------|
| REQ-AUTO-001 | Artifact-Free Automation | P0 | ⚠️ Partial | AutomationArtifactTest |
| REQ-BYPASS-001 | Click-Free Bypass | P0 | ⚠️ Not Integrated | BypassClickTest |
| REQ-PERF-001 | Predictable CPU | P0 | ⚠️ Not Integrated | CPUStabilityTest |
| REQ-VALID-001 | Strictest Validation | P0 | ⚠️ Not Tested | ValidationSuite |
| REQ-STATE-001 | Deterministic Recall | P1 | ⚠️ Gap | StateRoundTripTest |
| REQ-UI-001 | HiDPI Scaling | P2 | ⚠️ Not Tested | Manual HiDPI Test |
| REQ-AAX-001 | RT-Safe AAX Threading | P0* | ❌ Not Built | AAXThreadingTest |
| REQ-COMPAT-001 | Cross-DAW Compat | P1 | ⚠️ Not Tested | Cross-DAW Protocol |

**P0 (SHIP-BLOCKERS):** 4 items (+ AAX when ready)
**P1 (CRITICAL):** 2 items
**P2 (IMPORTANT):** 1 item

---

## Test Implementation Priority

### Phase 1: Automated Unit Tests (CI-friendly)
1. **CPUStabilityTest** - Run in CI, fail if variance > 10%
2. **StateRoundTripTest** - Run in CI, fail if not bit-exact
3. **AutomationArtifactTest** - Run in CI, fail if THD+N > -80dB
4. **BypassClickTest** - Run in CI, fail if transients detected

### Phase 2: Validation Gates (Pre-release)
5. **ValidationSuite** - pluginval + auval (automated in release pipeline)

### Phase 3: Manual Cross-DAW Testing (Human QA)
6. **Cross-DAW Protocol** - Manual test in 7+ DAWs
7. **HiDPI Test** - Manual visual inspection on 4K/Retina

### Phase 4: AAX-Specific (Post-AAX build)
8. **AAXThreadingTest** - Concurrent access stress test

---

## Next Action: Implement Missing Tests

See `docs/synthetic_beta/TEST_IMPLEMENTATION_PLAN.md` for detailed test code.
