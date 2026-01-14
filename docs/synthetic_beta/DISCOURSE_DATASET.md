# Synthetic Beta: Discourse-Derived User Feedback Dataset

**Generated:** 2026-01-14
**Purpose:** Pre-emptive QA dataset synthesized from real-world plugin complaints across forums, Reddit, GitHub, and YouTube
**Scope:** VST3 (Win/macOS/Linux), AU (macOS), AAX (Win/macOS - planned)

---

## Methodology

Analyzed recurring patterns from:
- **Forums**: KVR, Gearspace, JUCE Forum, Avid DUC (Pro Tools)
- **Reddit**: r/audioengineering, r/ableton, r/protools, r/WeAreTheMusicMakers, r/FL_Studio
- **GitHub**: pluginval issues, JUCE issues, popular plugin repos
- **YouTube/TikTok**: "plugin crashes", "CPU spikes", "automation glitches", "AU validation fails"

---

## Complaint Pattern #1: Automation Zipper Noise / Stepping

### Symptom
- **User Report**: "BTZ makes clicking/stepping sounds when I automate parameters"
- **Manifestation**: Audible artifacts during parameter sweeps (especially gain, filter cutoffs)
- **Severity**: **CRITICAL** - breaks automation usability

### Suspected Root Cause
- Insufficient parameter smoothing (block-rate vs sample-rate)
- Host automation resolution mismatches (FL Studio = 64 steps, DAW-specific)
- Missing ramp/crossfade on discrete parameter changes

### Repro Recipe
```
1. Load BTZ in DAW (any)
2. Automate "Drive" or "Input Gain" with fast sweep (1-2 seconds, 0→100%)
3. Play back and listen for clicks/steps
4. Test in: Ableton Live, FL Studio, Logic Pro, Reaper
```

### Prevention Strategy
- ✅ **Already Implemented**: `SmoothedValue` in PluginProcessor (BTZConstants::parameterSmoothingTime = 20ms)
- ⚠️ **Gap Identified**: Smoothing is block-rate (skip entire buffer), not sample-rate
- 🔧 **Fix Required**: Implement sub-block or per-sample smoothing for critical parameters

### Proposed Test
```cpp
// Test: Automation ramp test (automated in CI)
- Generate automation curve: 0→1 over 100ms
- Process 10 blocks at 512 samples/48kHz
- FFT analysis: check for spectral artifacts > -80dB
- PASS if no aliasing/clicking detected
```

**Priority:** P0 (ship-blocker)

---

## Complaint Pattern #2: Bypass Clicks/Pops

### Symptom
- **User Report**: "Loud click when I enable/disable BTZ"
- **Manifestation**: Transient spike at bypass toggle, especially mid-playback
- **Severity:** **HIGH** - unprofessional, damages trust

### Suspected Root Cause
- Hard switch between dry/wet without crossfade
- DC offset mismatch between bypassed and processed signal
- Buffer state not zeroed on bypass engage

### Repro Recipe
```
1. Load BTZ on drum bus (transient-rich content)
2. Play back loop
3. Toggle bypass on/off rapidly (every 500ms)
4. Listen for clicks/pops
```

### Prevention Strategy
- ✅ **Partially Implemented**: SafetyLayer has ClickFreeSwitch (10ms crossfade)
- ⚠️ **Not Integrated**: ClickFreeSwitch not used in PluginProcessor bypass logic
- 🔧 **Fix Required**: Wire ClickFreeSwitch into bypass path

### Proposed Test
```cpp
// Test: Bypass click detection
- Generate 1kHz sine wave
- Toggle bypass every 100ms for 10 seconds
- Measure peak amplitude of transients > -10dBFS
- PASS if no transients detected
```

**Priority:** P0 (ship-blocker)

---

## Complaint Pattern #3: CPU Spikes (Unpredictable)

### Symptom
- **User Report**: "BTZ causes random CPU spikes and dropouts"
- **Manifestation**: Intermittent jumps from 10% to 80% CPU, buffer underruns
- **Severity:** **CRITICAL** - breaks real-time playback

### Suspected Root Cause
- Denormal numbers (10-100x CPU slowdown)
- Oversampling always-on at maximum factor
- Memory allocations on audio thread (rare but catastrophic)
- Unguarded worst-case paths (e.g., FFT during transient burst)

### Repro Recipe
```
1. Load 10 instances of BTZ on different tracks
2. Play back complex mix (drums, bass, vocals)
3. Monitor CPU usage over 5 minutes
4. Check for spikes > 50% above average
```

### Prevention Strategy
- ✅ **Implemented**:
  - Denormal protection (FTZ/DAZ in prepareToPlay + ScopedNoDenormals)
  - Silence detection (skips processing after 5 silent buffers)
  - PerformanceMonitor (tracks CPU%, detects sustained overload)
- ⚠️ **Gap**: PerformanceGuardrails not integrated into PluginProcessor
- 🔧 **Fix Required**: Enable dynamic quality tier switching under load

### Proposed Test
```cpp
// Test: CPU stability test
- Run 100 blocks with worst-case input (white noise, full drive)
- Measure: min, max, avg, stddev of CPU%
- PASS if max < 2x average (no spikes)
- PASS if stddev < 10% (predictable)
```

**Priority:** P0 (ship-blocker)

---

## Complaint Pattern #4: Plugin Scan Failures

### Symptom
- **User Report**: "DAW doesn't detect BTZ / crashes during scan"
- **Manifestation**: Plugin fails AU/VST3 validation, doesn't appear in DAW plugin list
- **Severity:** **CRITICAL** - plugin unusable

### Suspected Root Cause
- Heavy initialization in constructor (loads models, allocates large buffers)
- Expensive UI resource loading during scan (DAWs run headless scan)
- Missing exception handling in critical init paths
- Non-standard plugin entry points

### Repro Recipe
```
1. Copy BTZ to plugin folder
2. Run DAW plugin scan (Logic, Ableton, Reaper)
3. Check if BTZ appears in plugin list
4. Run: pluginval --strictness-level 10 BTZ.vst3
5. Run: auval -v aufx BTZ Btzz (macOS)
```

### Prevention Strategy
- ✅ **Implemented**:
  - Lazy initialization (no heavy work in constructor)
  - ProductionSafety call order guard (prevents crashes if processBlock called before prepare)
- ⚠️ **Not Tested**: No automated pluginval/auval in CI
- 🔧 **Fix Required**: Add pluginval to test suite

### Proposed Test
```bash
# Test: Plugin validation suite
pluginval --strictness-level 10 --validate-in-process --output-dir ./validation_reports BTZ.vst3
auval -v aufx BTZ Btzz  # macOS only
# PASS if no failures, warnings acceptable
```

**Priority:** P0 (ship-blocker)

---

## Complaint Pattern #5: Session Recall Bugs / Preset Corruption

### Symptom
- **User Report**: "BTZ sounds different when I reopen project / preset loads wrong values"
- **Manifestation**:
  - Parameters reset to defaults
  - Sonic character changes across sessions
  - Pro Tools complains "session was modified"
- **Severity:** **HIGH** - breaks workflow, damages trust

### Suspected Root Cause
- Versioned state missing (no migration path for old presets)
- Non-deterministic component variance (seed changes across loads)
- Missing state validation on load
- Endianness issues (cross-platform)

### Repro Recipe
```
1. Load BTZ, set all parameters to non-default values
2. Save project
3. Close DAW
4. Reopen project
5. Compare parameter values (should be identical)
6. Repeat test across: Win, macOS, Linux
```

### Prevention Strategy
- ✅ **Implemented**:
  - ComponentVariance uses seeded PRNG (deterministic)
  - DeterministicProcessing preserves seeds
- ⚠️ **Gap**: No state versioning or corruption detection
- 🔧 **Fix Required**: Add state version tag + CRC checksum

### Proposed Test
```cpp
// Test: State round-trip test
- Set all parameters to test values
- Call getStateInformation()
- Create new instance
- Call setStateInformation()
- Compare all parameters
- PASS if bit-exact match
```

**Priority:** P1 (critical for trust)

---

## Complaint Pattern #6: GUI Scaling / HiDPI Issues

### Symptom
- **User Report**: "BTZ UI is blurry/tiny on my 4K monitor"
- **Manifestation**: UI doesn't scale with system DPI, text unreadable
- **Severity:** **MEDIUM** - usability issue

### Suspected Root Cause
- Missing HiDPI awareness flag
- Fixed pixel sizes instead of scaled units
- No system DPI detection

### Repro Recipe
```
1. Open BTZ on 4K/Retina display
2. Check UI sharpness and size
3. Compare with native DAW UI
```

### Prevention Strategy
- ✅ **Implemented**: Modern BTZ JUCE UI with theme system
- ⚠️ **Unknown**: HiDPI testing not performed
- 🔧 **Fix Required**: Test on 4K/Retina displays, enable JUCE_ENABLE_REPAINT_DEBUGGING

### Proposed Test
```
Manual: Test on 100%, 150%, 200% DPI scaling
Automated: Screenshot comparison at different DPI settings
```

**Priority:** P2 (important for UX)

---

## Complaint Pattern #7: AAX / Pro Tools Specific Issues

### Symptom
- **User Report**: "BTZ crashes in Pro Tools / automation doesn't work / 'unexpected error'"
- **Manifestation**:
  - AAX version crashes or refuses to load
  - Automation jumps/steps more than other DAWs
  - "Plugin caused an error" dialog
- **Severity:** **CRITICAL** - Pro Tools is industry standard

### Suspected Root Cause
- **Threading Model Mismatch**: Pro Tools calls setParameter from non-audio thread concurrently with processBlock
- Missing AAX-specific thread synchronization
- Shared mutable state accessed from multiple threads without locks/atomics
- AAX SDK version mismatch

### Repro Recipe
```
1. Build AAX version (requires Avid AAX SDK)
2. Load in Pro Tools
3. Automate parameters while playing
4. Check for crashes, glitches, error dialogs
```

### Prevention Strategy
- ⚠️ **Not Implemented**: AAX format not built yet
- 🔧 **Fix Required**:
  - Acquire Avid AAX SDK (requires Avid developer account)
  - Implement AAX-safe parameter handoff (lock-free or double-buffered)
  - Test automation thread concurrency

### Proposed Test
```cpp
// Test: Concurrent parameter access (simulates AAX threading)
std::thread audioThread([&]{ processor.processBlock(...); });
std::thread paramThread([&]{ processor.setParameter(...); });
// PASS if no crashes, data races, or corruption
```

**Priority:** P0 (AAX is must-have for pro market)

---

## Complaint Pattern #8: FL Studio Automation Resolution Artifacts

### Symptom
- **User Report**: "BTZ automation sounds stepped in FL Studio"
- **Manifestation**: 64-step resolution becomes audible as zipper noise
- **Severity:** **MEDIUM** - FL Studio specific

### Suspected Root Cause
- FL Studio sends automation as 64-step discrete values (not smooth curves)
- Plugin must provide additional smoothing to compensate

### Repro Recipe
```
1. Load BTZ in FL Studio
2. Automate Drive parameter with smooth curve
3. Play back - listen for stepping
4. Compare with other DAWs (Ableton, Logic)
```

### Prevention Strategy
- ✅ **Implemented**: Parameter smoothing (20ms)
- ⚠️ **Unknown**: Not tested in FL Studio specifically
- 🔧 **Fix Required**: Extend smoothing time for FL Studio (detect host?)

### Proposed Test
```
Manual: Test in FL Studio 21+ with automation clip
Automated: Simulate 64-step automation input, analyze output for artifacts
```

**Priority:** P2 (FL Studio has large user base)

---

## Complaint Pattern #9: Ableton Live VST3 Parameter Names

### Symptom
- **User Report**: "Parameter names truncated in Ableton / show as 'Param1', 'Param2'"
- **Manifestation**: VST3 parameter names don't display correctly in Ableton Live
- **Severity:** **LOW** - usability annoyance

### Suspected Root Cause
- VST3 parameter string encoding issues
- Ableton's VST3 implementation quirks

### Repro Recipe
```
1. Load BTZ (VST3) in Ableton Live
2. Check parameter names in automation lane
3. Should show "Drive", "Punch", etc. (not "Param1")
```

### Prevention Strategy
- ✅ **Likely OK**: BTZParams uses proper JUCE parameter IDs
- ⚠️ **Not Tested**: No Ableton Live testing yet
- 🔧 **Fix Required**: Validate parameter names in Live

### Proposed Test
```
Manual: Load in Ableton Live, verify parameter name display
```

**Priority:** P3 (minor UX issue)

---

## Complaint Pattern #10: Denormal Number CPU Spikes (Classic)

### Symptom
- **User Report**: "CPU usage jumps to 100% after a few minutes of silence"
- **Manifestation**: Sustained silence → denormal numbers → 10-100x CPU slowdown
- **Severity:** **HIGH** - breaks real-time playback

### Suspected Root Cause
- Filters/reverbs decay to denormal range (< 1e-38)
- FTZ/DAZ flags not set or reset by host

### Repro Recipe
```
1. Load BTZ
2. Play signal, then stop (silence)
3. Leave running for 5 minutes
4. Resume playback - check CPU usage
```

### Prevention Strategy
- ✅ **FIXED**: Multiple layers of protection:
  - `FloatVectorOperations::disableDenormalisedNumberSupport()` in prepareToPlay
  - `ScopedNoDenormals` in processBlock
  - DenormalGuard with FTZ/DAZ + noise injection in SafetyLayer
  - Silence detection (skips processing after 5 silent buffers)
- ✅ **Verified**: 4 layers of denormal protection

### Proposed Test
```cpp
// Test: Denormal CPU test
- Process 1000 blocks of silence
- Measure CPU% at start and end
- PASS if CPU% remains constant (no spike)
```

**Priority:** P0 (already fixed, validate in test)

---

## Summary of Findings

| Pattern | Severity | Status | Action Required |
|---------|----------|--------|-----------------|
| Automation Zipper Noise | CRITICAL | ⚠️ Partial | Implement sample-rate smoothing |
| Bypass Clicks/Pops | HIGH | ⚠️ Partial | Wire ClickFreeSwitch into bypass |
| CPU Spikes | CRITICAL | ✅ Implemented | Integrate PerformanceGuardrails |
| Plugin Scan Failures | CRITICAL | ⚠️ Not Tested | Add pluginval/auval to CI |
| Session Recall Bugs | HIGH | ⚠️ Gap | Add state versioning + CRC |
| GUI Scaling/HiDPI | MEDIUM | ⚠️ Unknown | Test on 4K displays |
| AAX / Pro Tools | CRITICAL | ❌ Not Implemented | Build AAX, test concurrency |
| FL Studio Automation | MEDIUM | ⚠️ Not Tested | Test in FL Studio |
| Ableton VST3 Names | LOW | ⚠️ Not Tested | Test in Live |
| Denormal Spikes | HIGH | ✅ FIXED | Validate in tests |

**Ship-Blockers (P0):** 5 items
**Critical (P1):** 1 item
**Important (P2):** 2 items
**Minor (P3):** 1 item

---

## Next Steps

1. **Implement Missing Fixes** (P0 items)
2. **Build Test Suite** (see REQUIREMENTS_FROM_DISCOURSE.md)
3. **Cross-DAW Validation** (see KNOWN_DAW_QUIRKS_MATRIX.md)
4. **AAX Scaffolding** (see DAW_SUPPORT.md)
