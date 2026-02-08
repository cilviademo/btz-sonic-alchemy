# Known DAW Quirks & BTZ Mitigations

**Generated:** 2026-01-14
**Purpose:** Document DAW-specific behaviors and how BTZ handles them
**Scope:** VST3, AU, AAX across major DAWs

---

## Matrix Overview

| DAW | Version Tested | Format | Key Quirks | BTZ Mitigation | Status |
|-----|----------------|--------|------------|----------------|--------|
| **Pro Tools** | 2024.x | AAX | Concurrent param/audio threads | Lock-free atomics, no shared mutable state | ⚠️ AAX not built |
| **Logic Pro** | 11.x | AU | Strict AU validation | ProductionSafety call guards | ✅ Should work |
| **Ableton Live** | 12.x | VST3 | Parameter name truncation | Proper JUCE param IDs | ⚠️ Not tested |
| **FL Studio** | 21.x | VST3 | 64-step automation resolution | 20ms parameter smoothing | ⚠️ Not tested |
| **Reaper** | 7.x | VST3 | Lenient, rarely issues | Standard JUCE impl | ✅ Should work |
| **Studio One** | 6.x | VST3 | Plugin state rescan on reopen | Deterministic state save | ✅ Should work |
| **Cubase/Nuendo** | 13.x/14.x | VST3 | VST3 EditController strict | Standard JUCE VST3 | ✅ Should work |
| **Bitwig Studio** | 5.x | VST3 | Multi-instance coordination | Independent instances OK | ✅ Should work |

---

## Pro Tools (AAX) - CRITICAL QUIRKS

### Quirk PT-1: Concurrent Parameter Access

**Behavior:**
- Pro Tools calls `setParameter()` from **non-audio thread** while audio thread runs `processBlock()`
- **No locks allowed** in audio thread (violates RT-safety)
- Shared mutable state = instant data race

**User Impact:** Crashes, audio glitches, corruption

**Example Scenario:**
```cpp
// THREAD 1 (Audio - RT priority)
void processBlock(AudioBuffer& buffer) {
    float drive = driveParameter;  // READ
    // ... process ...
}

// THREAD 2 (GUI/Automation - normal priority)
void setParameter(int index, float value) {
    driveParameter = value;  // WRITE - DATA RACE!
}
```

**BTZ Mitigation:**
- ✅ **APVTS uses std::atomic internally** (thread-safe reads)
- ✅ **No shared mutable structures** (each thread has own copy)
- ⚠️ **Not tested** with concurrent access yet

**Verification:**
```cpp
AAXThreadSafetyTest:
  - Spawn audio thread (1000x processBlock)
  - Spawn param thread (1000x setParameter)
  - Run with ThreadSanitizer
  - PASS if zero data races
```

**Status:** ⚠️ Needs AAX build + testing

---

### Quirk PT-2: AAX SDK Version Sensitivity

**Behavior:**
- Pro Tools requires specific AAX SDK version
- Mismatch = "Plugin could not be loaded" error
- Different PT versions may need different SDK versions

**User Impact:** Plugin doesn't load

**BTZ Mitigation:**
- ⚠️ **Not applicable yet** (AAX not built)
- 🔧 **Action:** Document required AAX SDK version per PT version

**Verification:**
- Manual test in Pro Tools 2023, 2024, 2025

**Status:** ❌ AAX not built

---

### Quirk PT-3: Offline Bounce vs Realtime

**Behavior:**
- Offline bounce may call processBlock faster than realtime
- Plugins must handle non-realtime processing

**User Impact:** Glitches in bounced audio

**BTZ Mitigation:**
- ✅ **RenderModeDetector** in DeterministicProcessing
- ✅ **Offline mode adjusts** long-term memory decay rates

**Verification:**
- Compare realtime playback vs offline bounce (should be identical)

**Status:** ✅ Implemented, needs validation

---

## Logic Pro (AU) - macOS Specific

### Quirk LP-1: Strict AU Validation

**Behavior:**
- Logic runs `auval` during plugin scan
- Fails on: incorrect property values, missing required properties, non-zero latency mismatch

**User Impact:** Plugin doesn't appear in Logic

**BTZ Mitigation:**
- ✅ **ProductionSafety** call order guards
- ✅ **Correct latency reporting** in prepareToPlay
- ⚠️ **Not tested** with auval yet

**Verification:**
```bash
auval -v aufx BTZ Btzz
# PASS if "PASSED" message
```

**Status:** ⚠️ Needs macOS testing

---

### Quirk LP-2: Latency Changes Not Supported Mid-Session

**Behavior:**
- If plugin changes latency after session start, Logic may glitch or crash
- Latency must be stable or require session reload

**User Impact:** Crashes when changing oversampling factor

**BTZ Mitigation:**
- ⚠️ **Current:** SPARK oversampling factor change triggers latency update
- 🔧 **Fix Needed:** Either:
  - (A) Lock oversampling factor on session start, or
  - (B) Document "changing OS requires bounce/reload"

**Verification:**
- Load BTZ in Logic, change SPARK OS factor, observe behavior

**Status:** ⚠️ Potential issue

---

## Ableton Live (VST3) - Cross-Platform

### Quirk AL-1: VST3 Parameter Name Truncation

**Behavior:**
- Live's VST3 implementation sometimes shows "Param1", "Param2" instead of real names
- Caused by VST3 parameter string encoding

**User Impact:** Confusing automation UI

**BTZ Mitigation:**
- ✅ **Proper JUCE parameter IDs** (e.g., "drive", "punch")
- ⚠️ **Not tested** in Live yet

**Verification:**
- Load BTZ VST3 in Ableton Live
- Check automation lane names
- PASS if shows "Drive", "Punch" (not "Param1")

**Status:** ⚠️ Needs testing

---

### Quirk AL-2: VST3 Sample-Accurate Automation

**Behavior:**
- Live supports sample-accurate VST3 automation
- Plugin must check for per-sample parameter changes

**User Impact:** Smoother automation, but plugin must support it

**BTZ Mitigation:**
- ⚠️ **Current:** Block-rate smoothing only
- 🔧 **Enhancement:** Implement per-sample parameter processing

**Verification:**
- Automate Drive in Live, check for zipper noise
- Compare with other DAWs

**Status:** ⚠️ Enhancement opportunity

---

## FL Studio (VST3) - Windows Specific

### Quirk FL-1: 64-Step Automation Resolution

**Behavior:**
- FL Studio sends automation as **64 discrete steps** (not smooth curves)
- Plugin must smooth heavily to prevent audible artifacts

**User Impact:** Stepped, zipper noise on automation

**BTZ Mitigation:**
- ✅ **20ms parameter smoothing** (reduces stepping)
- ⚠️ **May need more** for FL Studio specifically

**Verification:**
- Automate Drive in FL Studio with smooth curve
- Listen for stepping (compare with Ableton)
- PASS if smooth

**Status:** ⚠️ Needs FL Studio testing

---

### Quirk FL-2: Fast Plugin Unload/Reload

**Behavior:**
- FL Studio rapidly unloads/reloads plugins during certain operations
- Plugins must handle fast lifecycle changes

**User Impact:** Crashes during pattern/channel operations

**BTZ Mitigation:**
- ✅ **ProductionSafety** guards handle this
- ✅ **No heavy init in constructor**

**Verification:**
- Lifecycle stress test (1000x load/unload)

**Status:** ✅ Should work

---

## Reaper (VST3) - Cross-Platform

### Quirk RP-1: Generally Lenient, Few Issues

**Behavior:**
- Reaper is the most lenient DAW
- Rarely exposes plugin bugs (good for baseline testing)

**User Impact:** Works well

**BTZ Mitigation:**
- ✅ Standard JUCE implementation

**Verification:**
- Use Reaper as reference DAW for testing

**Status:** ✅ Should work

---

### Quirk RP-2: Supports Multiple Plugin Formats Simultaneously

**Behavior:**
- Can load VST2, VST3, AU side-by-side
- Useful for A/B testing formats

**User Impact:** None (feature)

**BTZ Mitigation:**
- N/A (beneficial for testing)

**Status:** ✅ Beneficial

---

## Studio One (VST3) - Cross-Platform

### Quirk SO-1: Plugin State Rescan on Project Reopen

**Behavior:**
- Studio One re-scans plugin state when reopening projects
- Non-deterministic state = "plugin settings changed" warning

**User Impact:** False positives on "modified plugin"

**BTZ Mitigation:**
- ✅ **Deterministic ComponentVariance** (seeded PRNG)
- ✅ **Stable state serialization**

**Verification:**
- Save project, close S1, reopen
- Should not show "modified" warning

**Status:** ✅ Should work

---

## Cubase/Nuendo (VST3) - Cross-Platform

### Quirk CB-1: Strict VST3 EditController Implementation

**Behavior:**
- Cubase enforces VST3 spec strictly
- Incorrect EditController = plugin fails to load

**User Impact:** Plugin doesn't load

**BTZ Mitigation:**
- ✅ **Standard JUCE VST3** implementation (well-tested)

**Verification:**
- Load in Cubase 13, check for errors

**Status:** ✅ Should work

---

### Quirk CB-2: High DPI Awareness (Windows)

**Behavior:**
- Cubase expects plugins to handle HiDPI correctly on Windows
- Blurry UI if not DPI-aware

**User Impact:** Blurry UI on 4K displays

**BTZ Mitigation:**
- ✅ **Modern JUCE UI** (should handle DPI)
- ⚠️ **Not tested** on Windows 4K

**Verification:**
- Test on Windows with 150%, 200% DPI scaling

**Status:** ⚠️ Needs testing

---

## Bitwig Studio (VST3) - Cross-Platform

### Quirk BW-1: Multi-Instance Coordination

**Behavior:**
- Bitwig allows complex multi-instance routing
- Plugins should not share global state

**User Impact:** Unexpected cross-instance interference

**BTZ Mitigation:**
- ✅ **All state is per-instance** (no globals)

**Verification:**
- Load 10 BTZ instances, verify independence

**Status:** ✅ Should work

---

## Cross-DAW Common Issues

### CI-1: Denormal Number Handling

**Behavior:**
- Some DAWs reset FTZ/DAZ flags between blocks

**BTZ Mitigation:**
- ✅ **4 layers of protection:**
  - prepareToPlay: `disableDenormalisedNumberSupport()`
  - processBlock: `ScopedNoDenormals`
  - SafetyLayer: DenormalGuard (FTZ/DAZ + noise)
  - Silence detection (skips processing)

**Status:** ✅ FIXED

---

### CI-2: Parameter Thread Safety

**Behavior:**
- VST3/AU: Parameter changes usually on message thread (safe)
- AAX: Parameter changes on separate thread (concurrent with audio)

**BTZ Mitigation:**
- ✅ **APVTS uses atomics** (should be safe)
- ⚠️ **Needs concurrent access testing**

**Status:** ⚠️ Needs AAX testing

---

### CI-3: HiDPI / Retina Displays

**Behavior:**
- macOS Retina: 2x scaling expected
- Windows 4K: 150%-200% scaling expected
- Linux: Varies by DE

**BTZ Mitigation:**
- ✅ **Modern JUCE UI** (should handle DPI)
- ⚠️ **Not tested** on all platforms

**Status:** ⚠️ Needs cross-platform HiDPI testing

---

## Testing Priority

### Phase 1: Automated (CI)
- ✅ pluginval strictness-level 10 (VST3)
- ⚠️ auval (AU - macOS)
- ❌ AAX validation (requires AAX build)

### Phase 2: Manual Cross-DAW (Pre-Release)
1. **Pro Tools** (AAX) - after AAX build
2. **Logic Pro** (AU) - macOS
3. **Ableton Live** (VST3) - Win, macOS
4. **FL Studio** (VST3) - Win
5. **Reaper** (VST3) - Win, macOS, Linux (reference DAW)
6. **Studio One** (VST3) - Win, macOS
7. **Cubase** (VST3) - Win, macOS

### Phase 3: Edge Cases
- Bitwig multi-instance
- Nuendo (Cubase equivalent)
- HiDPI on all platforms

---

## Mitigation Summary

| Mitigation | Implementation | Status |
|------------|----------------|--------|
| Lock-free parameter access | APVTS atomics | ✅ Implemented |
| Denormal protection (4 layers) | Multiple guards | ✅ Implemented |
| Deterministic state | Seeded PRNG | ✅ Implemented |
| Call order guards | ProductionSafety | ✅ Implemented |
| Parameter smoothing | SmoothedValue (20ms) | ✅ Implemented |
| Offline render detection | RenderModeDetector | ✅ Implemented |
| HiDPI awareness | Modern JUCE UI | ✅ Implemented |
| Per-instance independence | No global state | ✅ Implemented |
| Latency reporting | setLatencySamples() | ✅ Implemented |

**Remaining Gaps:**
- ⚠️ AAX build and testing
- ⚠️ Cross-DAW manual validation
- ⚠️ HiDPI testing on all platforms
- ⚠️ FL Studio 64-step automation validation
- ⚠️ Ableton parameter name validation

---

**Next:** Complete DAW_SUPPORT.md with AAX scaffolding plan
