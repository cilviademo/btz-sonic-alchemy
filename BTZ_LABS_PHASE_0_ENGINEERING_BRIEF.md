# BTZ LABS - PHASE 0 ENGINEERING BRIEF
**Date:** 2026-01-07
**Status:** Nearly Ready → Ship-Grade Audit
**Mission:** Drive from 95% to 100% production-ready

---

## EXECUTIVE SUMMARY

BTZ is a **professional audio tone enhancer plugin** targeting VST3/AU/Standalone formats. The codebase is **architecturally sound** with good DSP fundamentals, but has **3 critical P0 RT-safety violations** and **5 P1 correctness issues** that must be resolved before shipping.

**Critical Finding:** The plugin has RT-unsafe `DBG()` calls in the audio thread and potential allocation hazards in the oversampling system. Additionally, newly created infrastructure files (AutoDebugger, LicenseSystem, ABComparison) are **not integrated into the build system**.

**Estimated Time to Ship:** 2-3 weeks with focused, incremental fixes.

---

## 1. PLUGIN ARCHITECTURE

### 1.1 Purpose
**BTZ - The Box Tone Zone Enhancer**
A creative tone-shaping plugin with 5 hero controls (Punch, Warmth, Boom, Mix, Drive) plus advanced modules (SPARK limiter, SHINE ultra-high EQ, Console emulation).

**Target Market:** Professional mix engineers, mastering engineers, producers
**Competitors:** UAD Neve/SSL, Waves NLS, Plugin Alliance bx_console
**Unique Selling Point:** Simplified workflow with "instant pro sound" controls

### 1.2 Architecture Overview
- **Framework:** JUCE 7+ (assumed)
- **Formats:** VST3, AU, Standalone
- **Channel Support:** Stereo only (mono/stereo verified in isBusesLayoutSupported)
- **Version:** 1.0.0 (hardcoded in PluginProcessor.h:112-114)
- **Build System:** CMake 3.15+

### 1.3 DSP Chain (processBlock order)
```
Input Gain → DC Blocker In → Transient Shaper (Punch)
→ [Oversampling Up → Saturation (Warmth) → SPARK Limiter → Oversampling Down]
→ DC Blocker Out → SubHarmonic (Boom) → SHINE EQ → Console Emulator
→ Output Gain
```

**Key DSP Modules:**
- **TransientShaper:** Envelope follower + gain staging for attack/sustain control
- **Saturation:** Waveshaping with warmth control (requires oversampling)
- **SubHarmonic:** Octave-down synthesis for low-end enhancement
- **SparkLimiter:** Advanced clipping engine with LUFS targeting
- **ShineEQ:** Ultra-high frequency shelf (10kHz-80kHz) using RBJ biquads
- **ConsoleEmulator:** 3 modes (Transparent/Glue/Vintage) for mix glue
- **TPTDCBlocker:** Topology-Preserving Transform DC removal (pre/post saturation)

### 1.4 Parameter Management
- **System:** JUCE AudioProcessorValueTreeState (APVTS)
- **Total Parameters:** ~30 parameters
- **Parameter IDs:** String-based (e.g., "punch", "warmth", "boom")
- **Smoothing:** juce::SmoothedValue with 20-50ms ramps
- **State Format:** XML with version attributes

**Critical Parameter Categories:**
1. **Hero Controls (5):** punch, warmth, boom, mix, drive
2. **I/O Trim (3):** inputGain, outputGain, autoGain
3. **SPARK (7):** sparkEnabled, sparkLUFS, sparkCeiling, sparkMix, sparkOS, sparkAutoOS, sparkMode
4. **SHINE (6):** shineEnabled, shineFreqHz, shineGainDb, shineQ, shineMix, shineAutoOS
5. **Master (4):** masterEnabled, masterMacro, masterBlend, masterMix
6. **System (3):** active, oversampling, precisionMode

---

## 2. THREADING MODEL

### 2.1 Thread Safety Analysis
**Audio Thread (RT-critical):**
- `processBlock()` - **MUST BE RT-SAFE** (no allocations, locks, logging)
- `prepareToPlay()` - Allocation allowed (setup phase)
- `releaseResources()` - Allocation allowed (teardown)

**Message Thread (GUI):**
- `createEditor()` - Safe to allocate
- Parameter callbacks (APVTS listeners) - Safe to allocate
- Metering updates - Read atomics only

**Thread Communication:**
- **Parameters:** APVTS uses lock-free atomics (`getRawParameterValue()->load()`)
- **Metering:** std::atomic<float> for LUFS, peak, GR, stereo correlation (PluginProcessor.h:97-100)
- **Smoothing:** juce::SmoothedValue (pre-allocated, RT-safe)

### 2.2 RT-Safety Status
✅ **Safe:**
- Parameter reading via APVTS (lock-free atomics)
- Metering updates (atomic stores)
- SmoothedValue usage
- DSP module processing (pre-allocated buffers)

❌ **P0 VIOLATIONS FOUND:**
1. **PluginProcessor.cpp:333-334, 340** - `DBG()` with string construction in processBlock()
2. **Oversampling.h:37-41** - `std::make_unique()` in setOversamplingFactor()
3. **Oversampling.h:41** - `oversampler->initProcessing()` allocates buffers

---

## 3. STATE/PRESET MODEL

### 3.1 State Serialization
**Format:** XML via JUCE copyXmlToBinary/getXmlFromBinary
**Location:** PluginProcessor.cpp:417-451

**State Structure:**
```xml
<Parameters pluginVersion="1.0.0" pluginName="BTZ">
  <PARAM id="punch" value="0.5"/>
  <PARAM id="warmth" value="0.3"/>
  <!-- ... all parameters ... -->
</Parameters>
```

### 3.2 Versioning
✅ **Version Attributes Added:** Lines 424-427
⚠️ **Migration Logic Missing:** Lines 443-446 (empty comment placeholder)

**Current Behavior:**
- Saves version string ("1.0.0") to state
- Loads version string on restore
- **NO MIGRATION LOGIC** - relies on APVTS to use defaults for missing parameters

**Risk Assessment:**
- **Low Risk (for now):** APVTS handles missing parameters gracefully
- **Future Risk:** If parameter IDs/ranges change, users lose presets
- **Recommendation:** Implement versioned migration before v1.1

### 3.3 Backward Compatibility
✅ **Parameter ID Stability:** All IDs are const strings in PluginParameters.h
⚠️ **No Validation:** State loading doesn't validate parameter ranges
⚠️ **No Corruption Handling:** Assumes XML is always valid

---

## 4. BUILD TARGETS

### 4.1 CMakeLists.txt Analysis
**Location:** BTZ_JUCE/CMakeLists.txt

**Plugin Formats:**
- VST3 ✅
- AU ✅
- Standalone ✅
- AAX ❌ (not enabled - require for Pro Tools users?)

**Source Files Listed:**
```cmake
PluginProcessor.cpp ✅
PluginEditor.cpp ✅
DSP/*.cpp (9 files) ✅
GUI/*.cpp (2 files) ✅
```

❌ **P0 BUILD ISSUE: Missing Files**
The following newly created infrastructure files are **NOT in CMakeLists.txt:**
- Source/WDFSaturation.cpp
- Source/ProductionSafety.h (header-only, but needs awareness)
- Source/AutoDebugger.h
- Source/ABComparison.h
- Source/LicenseSystem.h

**Impact:** Plugin will not compile with new features. Must update CMakeLists.txt.

### 4.2 Dependencies
- JUCE (version not pinned - risk for reproducible builds)
- No external DSP libraries (good for portability)
- Assumes JUCE in `JUCE/` directory (fragile)

---

## 5. RISK AREAS IDENTIFIED

### 5.1 ⚠️ P0 - AUDIO THREAD HAZARDS (SHIP BLOCKERS)

| Issue | Location | Impact | Fix Effort |
|-------|----------|--------|------------|
| **DBG() in processBlock()** | PluginProcessor.cpp:333, 334, 340 | RT violation → dropouts in Pro Tools, Logic | 30 min |
| **Oversampling allocation** | Oversampling.h:37-41 | Potential RT violation if param changes trigger it | 1 hour |
| **Build system incomplete** | CMakeLists.txt | Plugin won't compile with new infrastructure | 15 min |

**Detailed Analysis:**

**Issue 1: DBG() Calls in Audio Thread**
```cpp
// PluginProcessor.cpp:333-334
if (!BTZValidation::validateBuffer(buffer))
{
    DBG("BTZ: Invalid samples detected in output buffer!");  // ❌ ALLOCATES
    BTZValidation::sanitizeBuffer(buffer);
}
```
- `DBG()` constructs juce::String → heap allocation → RT violation
- **Only in DEBUG builds**, but still violates contract
- Pro Tools **WILL DROP AUDIO** if RT violation detected
- **Solution:** Use RTSafeLogger (already exists in ProductionSafety.h!)

**Issue 2: Oversampling Dynamic Allocation**
```cpp
// Oversampling.h:37-38
oversampler = std::make_unique<juce::dsp::Oversampling<SampleType>>(
    2, stages, ...);  // ❌ ALLOCATES
```
- Called from `setOversamplingFactor()` if stages change
- If parameter changes trigger this during processing → RT violation
- **Current code (PluginProcessor.cpp:231):** Sets OS factor in processBlock!
- **Solution:** Pre-allocate all OS stages in prepareToPlay(), OR defer changes to message thread

**Issue 3: Build System**
- New files not in CMakeLists.txt → compile failure
- **Solution:** Add missing files (5 min fix)

---

### 5.2 ⚠️ P1 - CORRECTNESS ISSUES (MUST FIX)

| Issue | Location | Impact | Fix Effort |
|-------|----------|--------|------------|
| **Parameter smoothing per-block not per-sample** | PluginProcessor.cpp:199-205 | Zipper noise at small buffer sizes (32/64 samples) | 2 hours |
| **ProductionSafety not used** | ProductionSafety.h exists but not integrated | Missing HostCallOrderGuard, RTSafeLogger | 3 hours |
| **No custom GUI** | PluginProcessor.cpp:414 | GenericAudioProcessorEditor → unprofessional | N/A (feature) |
| **State migration unimplemented** | PluginProcessor.cpp:443-446 | Future preset breakage risk | 4 hours |
| **Oversampling factor changes in RT** | PluginProcessor.cpp:231 | See P0 issue #2 | 1 hour |

**Detailed Analysis:**

**Issue 1: Smoothing Per-Block**
```cpp
// PluginProcessor.cpp:199-205
smoothedPunch.setTargetValue(apvts.getRawParameterValue(BTZParams::IDs::punch)->load());
// ...
float punchAmount = smoothedPunch.getNextValue();  // ❌ ONCE per block!
```
- `getNextValue()` called **once per block** instead of **once per sample**
- At 32-sample buffers (common in low-latency sessions), ramp is only 32 samples long
- **Should be:** Call `getNextValue()` in sample loop OR use `skip()` method
- **Impact:** Audible zipper noise during automation in low-latency sessions
- **Fix:** Refactor to per-sample smoothing or batch smoothing

**Issue 2: ProductionSafety Not Used**
- `HostCallOrderGuard` exists but not used in PluginProcessor
- `RTSafeLogger` exists but not used (DBG() still used instead!)
- **This is the infrastructure to fix P0 issues**, but it's not integrated
- **Fix:** Integrate HostCallOrderGuard in prepareToPlay/processBlock, replace DBG() with RTSafeLogger

**Issue 3: State Migration**
- Version string saved but not used for migration
- If parameter structure changes in v1.1, v1.0 presets will break
- **Fix:** Implement migration logic with version checks

---

### 5.3 ⚠️ P2 - POLISH ISSUES (NICE TO HAVE)

| Issue | Location | Impact | Fix Effort |
|-------|----------|--------|------------|
| **No NaN/Inf protection in release** | Only DEBUG (line 330-342) | Silent failure if DSP bug in production | 30 min |
| **LUFS calculation simplistic** | PluginProcessor.cpp:365-380 | Inaccurate loudness metering | 2 hours |
| **Gain reduction fake** | PluginProcessor.cpp:395-404 | Not reading actual limiter GR | 1 hour |
| **Magic numbers** | Line 374: `lufsSampleCount > 100` | Hard to maintain | 15 min |
| **Denormal protection once** | prepareToPlay only (line 91) | Some hosts reset FTZ flags | 15 min |

---

### 5.4 HOST COMPATIBILITY RISKS

**Not Yet Tested:**
- ❌ pluginval (official VST3 validator)
- ❌ FL Studio (known for call order issues)
- ❌ Ableton Live (automation rate issues)
- ❌ Pro Tools (strict RT requirements)
- ❌ Logic Pro (AU validation)
- ❌ Reaper (stress testing)
- ❌ Bitwig (multi-rate hosts)

**Known Risks:**
1. **FL Studio:** May call processBlock before prepareToPlay → need HostCallOrderGuard
2. **Pro Tools:** Zero-tolerance RT violations → must fix all DBG() calls
3. **Ableton:** Automation at audio rate → must fix smoothing
4. **Logic:** AU validation strictness → must pass auval
5. **Bitwig:** Multi-rate processing → latency compensation critical

---

### 5.5 UI PERFORMANCE RISKS

**Current State:**
- No custom GUI (using GenericAudioProcessorEditor)
- Cannot assess 60fps, HiDPI, or rendering performance
- Metering atomics present but no GUI to consume them

**When Custom GUI Built:**
- Must verify 60fps with timer-based repaints
- Must test HiDPI scaling (1x, 1.5x, 2x, 3x)
- Must test GPU acceleration (OpenGL/Metal)
- Must avoid GUI→Audio thread calls

---

## 6. PRIORITIZED ISSUE LIST

### P0 (CRITICAL - MUST FIX BEFORE FIRST TEST)
| # | Issue | File:Line | Fix | Time |
|---|-------|-----------|-----|------|
| P0-1 | DBG() in processBlock | PluginProcessor.cpp:333,334,340 | Replace with RTSafeLogger | 30m |
| P0-2 | Oversampling allocation in RT path | Oversampling.h:37-41, PluginProcessor.cpp:231 | Defer to message thread | 1h |
| P0-3 | Build system incomplete | CMakeLists.txt | Add missing files | 15m |

**Total P0 Effort:** ~2 hours

---

### P1 (CORRECTNESS - MUST FIX BEFORE SHIPPING)
| # | Issue | File:Line | Fix | Time |
|---|-------|-----------|-----|------|
| P1-1 | Parameter smoothing per-block | PluginProcessor.cpp:199-205 | Per-sample smoothing | 2h |
| P1-2 | ProductionSafety not integrated | PluginProcessor.cpp | Add HostCallOrderGuard | 3h |
| P1-3 | State migration unimplemented | PluginProcessor.cpp:443-446 | Add version checks | 4h |
| P1-4 | No pluginval testing | N/A | Run pluginval, fix issues | 4h |
| P1-5 | No DAW compatibility testing | N/A | Test 6 major DAWs | 8h |

**Total P1 Effort:** ~21 hours (~3 days)

---

### P2 (POLISH - IMPROVE BEFORE V1.1)
| # | Issue | File:Line | Fix | Time |
|---|-------|-----------|-----|------|
| P2-1 | NaN/Inf only in DEBUG | PluginProcessor.cpp:330-342 | Add release protection | 30m |
| P2-2 | LUFS calculation simplistic | PluginProcessor.cpp:365-380 | Implement ITU-R BS.1770-4 | 2h |
| P2-3 | Gain reduction fake | PluginProcessor.cpp:395-404 | Read from SparkLimiter | 1h |
| P2-4 | Magic numbers | PluginProcessor.cpp:374 | Extract constants | 15m |
| P2-5 | Denormal protection once | PluginProcessor.cpp:91 | Add to processBlock | 15m |

**Total P2 Effort:** ~4 hours

---

## 7. PROPOSED FIX PLAN (2-3 WEEKS)

### Week 1: P0 + P1 RT Safety & Build
**Commits:**
1. `[P0] Fix: Add missing files to CMakeLists.txt` (15m)
2. `[P0] Fix: Replace DBG() with RTSafeLogger in processBlock` (30m)
3. `[P0] Fix: Defer oversampling factor changes to message thread` (1h)
4. `[P1] Fix: Add HostCallOrderGuard to prevent call order crashes` (1h)
5. `[P1] Fix: Implement per-sample parameter smoothing` (2h)

**Validation:**
- Build succeeds on macOS/Windows/Linux
- pluginval passes with zero warnings
- No RT violations detected in Release build

**Deliverable:** Commits 1-5 (5 hours coding + 3 hours testing = **1 day**)

---

### Week 2: P1 Host Compatibility & State Migration
**Commits:**
6. `[P1] Fix: Implement versioned state migration logic` (4h)
7. `[P1] Test: Add pluginval to CI, fix all reported issues` (4h)
8. `[P1] Test: Verify compatibility with FL Studio, Ableton, Logic, Pro Tools` (8h)
9. `[P1] Fix: Address any host-specific crashes found in testing` (4h)

**Validation:**
- All 6 major DAWs load plugin without crashes
- Preset forward/backward compatibility verified
- Automation works smoothly at all buffer sizes

**Deliverable:** Commits 6-9 (20 hours = **2.5 days**)

---

### Week 3: P2 Polish & Final Validation
**Commits:**
10. `[P2] Improve: Add NaN/Inf sanitization in release builds` (30m)
11. `[P2] Improve: Implement accurate ITU-R BS.1770-4 LUFS metering` (2h)
12. `[P2] Improve: Read actual gain reduction from SparkLimiter` (1h)
13. `[P2] Refactor: Extract magic numbers to named constants` (15m)
14. `[P2] Improve: Add denormal protection to processBlock` (15m)

**Validation:**
- Full pluginval suite passes (VST3 + AU)
- Stress testing (1000 instances, 32-sample buffers, automation)
- Metering accuracy verified against reference plugins

**Deliverable:** Commits 10-14 (4 hours coding + 4 hours testing = **1 day**)

---

### Total Timeline Summary
- **Week 1:** P0 fixes + critical P1 (1 day)
- **Week 2:** P1 host compat + state (2.5 days)
- **Week 3:** P2 polish + validation (1 day)
- **Total:** 4.5 days coding + buffer = **2 weeks** (conservative: 3 weeks)

---

## 8. DEFINITION OF DONE (SHIP READINESS)

### 8.1 RT Safety ✅ Checklist
- [ ] No allocations in processBlock (verified with Valgrind/Instruments)
- [ ] No locks in processBlock (verified with TSAN)
- [ ] No DBG()/logging in processBlock
- [ ] No std::vector/String construction in processBlock
- [ ] HostCallOrderGuard prevents crashes if prepareToPlay not called

### 8.2 State Compatibility ✅ Checklist
- [ ] Version string saved to all presets
- [ ] Migration logic implemented for v1.0 → v1.1
- [ ] Missing parameters use defaults (no crashes)
- [ ] Corrupted XML handled gracefully
- [ ] Parameter ID registry is FROZEN (documented as API contract)

### 8.3 Host Compatibility ✅ Checklist
- [ ] pluginval passes (VST3 + AU)
- [ ] FL Studio: Loads, processes, saves state
- [ ] Ableton Live: Automation at 32-sample buffer works
- [ ] Logic Pro: auval passes
- [ ] Pro Tools: No RT violations (verified with RT checker)
- [ ] Reaper: Stress test (1000 instances)
- [ ] Bitwig: Multi-rate processing correct

### 8.4 DSP Correctness ✅ Checklist
- [ ] No NaN/Inf in output (verified with validation suite)
- [ ] DC offset < 0.001 (verified with BTZValidation::hasDCOffset)
- [ ] Denormal protection active
- [ ] Parameter smoothing eliminates zipper noise at 32-sample buffers
- [ ] Latency compensation accurate (verified with phase alignment)

### 8.5 Build System ✅ Checklist
- [ ] CMakeLists.txt includes ALL source files
- [ ] VST3 builds and validates
- [ ] AU builds and passes auval
- [ ] Standalone builds and runs
- [ ] CI pipeline runs pluginval automatically

---

## 9. IMMEDIATE NEXT STEPS (Phase 1)

**Objective:** Produce concrete diffs for first 3 P0 fixes

**Task 1: Update CMakeLists.txt** (15 min)
- Add missing .cpp files to target_sources
- Verify JUCE path configuration
- Test build on macOS

**Task 2: Replace DBG() with RTSafeLogger** (30 min)
- Integrate ProductionSafety.h
- Add RTSafeLogger instance to BTZAudioProcessor
- Replace 3 DBG() calls with logRT()
- Add timer-based message processing in editor

**Task 3: Fix Oversampling Allocation** (1 hour)
- Move setOversamplingFactor() out of processBlock
- Add parameter listener for sparkOS parameter
- Defer changes to message thread with AsyncUpdater
- OR: Pre-allocate all OS stages (1x-16x) in prepareToPlay

**Expected Output:** 3 commits, ready for PR review

---

## 10. RISK MITIGATION STRATEGY

### 10.1 Backward Compatibility Guarantees
**Rule:** NEVER change existing parameter IDs without migration
- Parameter IDs in PluginParameters.h are **API contracts**
- Adding parameters is OK (use defaults for missing)
- Removing parameters requires migration + deprecation period
- Changing ranges requires migration + user notification

### 10.2 RT Safety Verification
**Tools:**
- **Valgrind** (Linux): Detect allocations in processBlock
- **Instruments** (macOS): Time Profiler → check for malloc
- **TSAN** (GCC/Clang): Thread sanitizer for data races
- **Pro Tools RT Checker**: Built-in DAW validation

**Process:**
1. Build with sanitizers enabled
2. Run pluginval with RT checking
3. Test in Pro Tools with RT violations logging enabled
4. Stress test with 32-sample buffers + automation

### 10.3 Host Compatibility Testing
**Minimum Test Matrix:**
| DAW | Version | Test Cases |
|-----|---------|------------|
| Pro Tools | 2023.x | RT violations, AAX validation |
| Logic Pro | 10.8+ | auval, automation |
| Ableton Live | 11/12 | 32-sample buffers, automation |
| FL Studio | 21+ | Call order, state save/load |
| Reaper | 7.x | Stress test (1000 instances) |
| Bitwig | 5.x | Multi-rate processing |

---

## 11. SUMMARY OF FINDINGS

### What's Good ✅
- Solid JUCE architecture with proper APVTS usage
- Good DSP fundamentals (TPT filters, RBJ biquads, WDF modeling)
- Atomics used correctly for metering
- SmoothedValue for parameter smoothing (needs fix but framework OK)
- Version attributes added to state (foundation for migration)
- ProductionSafety infrastructure exists (just needs integration)

### What's Broken ❌
- **3 P0 RT-safety violations** (DBG + allocation)
- **Build system missing 5 new files**
- **Parameter smoothing incorrect** (per-block not per-sample)
- **ProductionSafety not integrated** (dead code)
- **No pluginval testing** (unknown host compat status)
- **State migration unimplemented** (future preset breakage)

### What's Missing 🔧
- Custom GUI (using GenericAudioProcessorEditor)
- pluginval integration
- Host compatibility testing
- Release-mode NaN/Inf protection
- Accurate LUFS metering
- Unit tests for DSP modules

### Overall Assessment 📊
**Current Status:** 95% → **85% Ship-Ready**
(Downgraded due to P0 RT violations - these are ship blockers)

**After P0 Fixes:** 95% ship-ready
**After P1 Fixes:** 98% ship-ready
**After P2 Polish:** 100% ship-ready

**Conservative Timeline:** 2-3 weeks to 100%

---

## 12. SIGN-OFF

This Phase 0 Engineering Brief represents a comprehensive audit of the BTZ plugin codebase as of 2026-01-07. All findings are based on static code analysis and JUCE best practices. Dynamic testing (pluginval, DAW matrix) will be performed in Phase 2.

**Critical Path:**
1. Fix P0 RT-safety violations (2 hours)
2. Integrate ProductionSafety (3 hours)
3. Run pluginval (4 hours to fix issues)
4. Test in 6 major DAWs (8 hours)
5. Final validation (4 hours)

**Total:** 21 hours = 3 days of focused work

**Next Document:** Phase 1 - Static Audit with Concrete Diffs

---

**BTZ Labs Engineering Team**
*Uncompromising. Evidence-Based. Ship-Grade.*
