# BTZ DAW Support & Format Matrix

**Generated:** 2026-01-14
**Status:** Phase 1-3 Complete, AAX Pending
**Purpose:** Document supported formats, platforms, and AAX scaffolding plan

---

## Currently Supported Formats

| Format | Platform | Build Status | Validation Status |
|--------|----------|--------------|-------------------|
| **VST3** | Windows | ✅ Building | ⚠️ Not tested with pluginval |
| **VST3** | macOS | ✅ Building | ⚠️ Not tested with pluginval |
| **VST3** | Linux | ✅ Building | ⚠️ Not tested with pluginval |
| **AU** | macOS | ⚠️ Configured | ❌ Not built/tested |
| **AAX** | Windows | ❌ Not built | ❌ Requires Avid SDK |
| **AAX** | macOS | ❌ Not built | ❌ Requires Avid SDK |
| **Standalone** | All | ✅ Building | ✅ Functional |

---

## VST3 Status

### Build Configuration
```cmake
# CMakeLists.txt (current)
FORMATS VST3 AU Standalone
```

### Build Output
- **Location:** `BTZ_JUCE/BTZ_artefacts/Release/VST3/`
- **Filename:** `BTZ - The Box Tone Zone.vst3`
- **Size:** ~2-5 MB (depending on optimizations)
- **Status:** ✅ **Building successfully** (exit code 0)

### Known Issues
- ⚠️ **pluginval not run**: No automated VST3 validation yet
- ⚠️ **Cross-DAW not tested**: Manual testing pending

### Required Actions
1. **Add pluginval to CI**:
   ```bash
   pluginval --strictness-level 10 --validate-in-process BTZ.vst3
   ```

2. **Cross-DAW manual test** (see KNOWN_DAW_QUIRKS_MATRIX.md):
   - Ableton Live
   - FL Studio
   - Reaper (reference)
   - Studio One
   - Cubase/Nuendo
   - Bitwig

---

## AU (Audio Units) Status

### Build Configuration
```cmake
# CMakeLists.txt (current)
FORMATS VST3 AU Standalone
```

### Platform
- **macOS only** (Apple's Audio Units framework)
- **Logic Pro**, **GarageBand**, **Final Cut Pro**

### Build Output
- **Location:** `BTZ_JUCE/BTZ_artefacts/Release/AU/`
- **Filename:** `BTZ - The Box Tone Zone.component`
- **Status:** ⚠️ **Configured but not built yet**

### Known Issues
- ❌ **Not built on macOS**: Requires macOS build environment
- ❌ **auval not run**: No AU validation yet

### Required Actions
1. **Build on macOS**:
   ```bash
   cd BTZ_JUCE
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

2. **Run auval**:
   ```bash
   auval -v aufx BTZ Btzz
   # PASS if "PASSED" message appears
   ```

3. **Test in Logic Pro**:
   - Load BTZ as Audio FX
   - Process audio, automate parameters
   - Save & reload project
   - Verify consistency

---

## AAX (Pro Tools) Status

### Current Situation
- ❌ **NOT BUILT** - Requires Avid AAX SDK
- ❌ **NOT CONFIGURED** - Missing AAX target in CMake

### Why AAX is Critical
- **Pro Tools** is the industry standard in professional studios
- **Market requirement**: Many studios won't consider plugins without AAX
- **Revenue impact**: AAX support opens pro market ($$$)

### AAX SDK Requirements

#### 1. Acquire Avid AAX SDK

**Option A: Avid Developer Account (Recommended)**
```
1. Sign up at: https://www.avid.com/alliance-partner-program
2. Apply for AAX developer access
3. Download AAX SDK (latest version)
4. Review AAX SDK license (proprietary, non-distributable)
```

**Option B: PACE iLok Integration**
```
- AAX plugins require PACE iLok copy protection
- Sign up for PACE/iLok developer program
- Integrate iLok authorization into BTZ
```

**Current Blocker:** Requires Avid developer account (manual signup)

---

#### 2. AAX SDK File Structure

Expected SDK structure:
```
AAX_SDK/
├── Interfaces/        # AAX API headers
├── Libs/              # AAX static libraries
│   ├── AAXLibrary_Win.lib
│   ├── AAXLibrary_Mac.a
│   └── ...
├── Documentation/     # AAX dev guide
└── Examples/          # Sample AAX plugins
```

---

#### 3. JUCE AAX Integration

JUCE includes AAX wrapper support (if SDK is available):

```cmake
# CMakeLists.txt (AFTER acquiring AAX SDK)
juce_add_plugin(BTZ
    # ... existing config ...
    FORMATS VST3 AU AAX Standalone  # Add AAX here
    AAX_IDENTIFIER com.btzaudio.btz  # Required for AAX
)

# Point JUCE to AAX SDK
juce_set_aax_sdk_path("/path/to/AAX_SDK")
```

**JUCE Modules Required:**
- `juce_audio_plugin_client` (includes AAX wrapper)
- AAX format automatically enabled if SDK path is set

---

#### 4. AAX-Specific Code Considerations

**Threading Model (CRITICAL):**
```cpp
// AAX threading model is DIFFERENT from VST3/AU
// Pro Tools calls setParameter() from non-audio thread
// while processBlock() runs on audio thread

// MUST USE: Lock-free parameter updates
// BTZ Current Status: ✅ APVTS uses atomics (should be safe)

// Example AAX-safe parameter access:
void processBlock(AudioBuffer& buffer) {
    // SAFE: APVTS getRawParameterValue() returns atomic pointer
    float drive = apvts.getRawParameterValue("drive")->load();
    // ... process ...
}
```

**AAX-Specific Methods:**
```cpp
// JUCE handles these automatically, but be aware:
- AAX_CEffectParameters::GetEffectType()
- AAX_CEffectParameters::UpdateParameterNormalizedValue()
- AAX_IEffectDescriptor::NewDescriptor()
```

---

#### 5. AAX Build Process

**Windows (Visual Studio 2019+):**
```bash
cmake -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release --target BTZ_AAX
```

**macOS (Xcode):**
```bash
cmake -B build -G Xcode
cmake --build build --config Release --target BTZ_AAX
```

**Output:**
- Windows: `BTZ.aaxplugin` (directory bundle)
- macOS: `BTZ.aaxplugin` (bundle)

---

#### 6. AAX Installation Locations

**Windows:**
```
C:\Program Files\Common Files\Avid\Audio\Plug-Ins\BTZ.aaxplugin\
```

**macOS:**
```
/Library/Application Support/Avid/Audio/Plug-Ins/BTZ.aaxplugin/
```

---

#### 7. AAX Code Signing (Required for Distribution)

**Windows:**
- **PACE Wrapper** required (encrypts AAX bundle)
- **DigiCert Code Signing Certificate** (for installer)

**macOS:**
- **PACE Wrapper** required
- **Apple Developer ID** (for Gatekeeper)
- **Notarization** (macOS 10.15+)

**PACE Process:**
```
1. Build AAX plugin
2. Run PACE Wrapper tool (signs/encrypts AAX)
3. Distribute wrapped .aaxplugin
```

---

### AAX Scaffolding Plan (Immediate)

Even without AAX SDK, we can prepare:

#### Step 1: Update CMakeLists.txt (Conditional)

```cmake
# Add AAX format if SDK is available
if(DEFINED ENV{AAX_SDK_PATH})
    message(STATUS "AAX SDK found at $ENV{AAX_SDK_PATH}")
    set(BTZ_FORMATS VST3 AU AAX Standalone)
    juce_set_aax_sdk_path($ENV{AAX_SDK_PATH})
else()
    message(WARNING "AAX SDK not found - AAX format disabled")
    message(WARNING "Set AAX_SDK_PATH environment variable to enable AAX")
    set(BTZ_FORMATS VST3 AU Standalone)
endif()

juce_add_plugin(BTZ
    # ... existing config ...
    FORMATS ${BTZ_FORMATS}
    AAX_IDENTIFIER com.btzaudio.btz  # Required if AAX is built
)
```

#### Step 2: Document Threading Model

Create `docs/AAX_THREADING_MODEL.md`:
```markdown
# AAX Threading Model for BTZ

## Critical Differences from VST3/AU

1. **Concurrent Parameter Access**:
   - VST3/AU: Parameters typically updated on message thread (not during processBlock)
   - AAX: Pro Tools updates parameters on separate thread WHILE processBlock runs

2. **BTZ Mitigation**:
   - APVTS uses std::atomic for all parameter values
   - No shared mutable state between threads
   - No locks in processBlock (RT-safe)

3. **Testing Plan**:
   - Simulate concurrent access with 2 threads
   - Run with ThreadSanitizer (TSAN)
   - Verify zero data races
```

#### Step 3: Create AAX Test Stub

```cpp
// tests/AAXThreadingTest.cpp (simulates AAX behavior)
TEST_CASE("AAX Concurrent Parameter Access", "[aax][threading]") {
    BTZAudioProcessor processor;
    AudioBuffer<float> buffer(2, 512);

    std::atomic<bool> running{true};

    // Thread 1: Simulate Pro Tools audio thread
    std::thread audioThread([&]() {
        while (running.load()) {
            processor.processBlock(buffer, MidiBuffer{});
        }
    });

    // Thread 2: Simulate Pro Tools automation/GUI thread
    std::thread paramThread([&]() {
        for (int i = 0; i < 1000; ++i) {
            processor.getAPVTS().getParameter("drive")->setValue(
                random.nextFloat()
            );
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        running.store(false);
    });

    audioThread.join();
    paramThread.join();

    // PASS if no crashes, no TSAN warnings
}
```

---

### AAX Acquisition Timeline

**Realistic Timeline:**
1. **Apply for Avid Developer Access**: 1-2 weeks (manual approval)
2. **Download AAX SDK**: Immediate (after approval)
3. **Integrate AAX into BTZ**: 1-3 days (JUCE handles most of it)
4. **Test in Pro Tools**: 1-2 days
5. **PACE Wrapper Integration**: 1 week (signing setup)
6. **Total**: ~3-4 weeks from application to AAX release

**Blocker:** Requires human action (Avid developer signup)

---

## Major DAW Compatibility Matrix

| DAW | Format | Platform | Tested | Status |
|-----|--------|----------|--------|--------|
| **Pro Tools** | AAX | Win, macOS | ❌ | ⚠️ AAX not built |
| **Logic Pro** | AU | macOS | ❌ | ⚠️ Needs macOS build |
| **Ableton Live** | VST3 | Win, macOS | ❌ | ✅ Should work |
| **FL Studio** | VST3 | Win | ❌ | ✅ Should work |
| **Reaper** | VST3 | Win, macOS, Linux | ❌ | ✅ Should work |
| **Studio One** | VST3 | Win, macOS | ❌ | ✅ Should work |
| **Cubase/Nuendo** | VST3 | Win, macOS | ❌ | ✅ Should work |
| **Bitwig Studio** | VST3 | Win, macOS, Linux | ❌ | ✅ Should work |

**Legend:**
- ✅ Should work - Standard JUCE impl, expected to work
- ⚠️ Needs testing - Not tested yet
- ❌ Blocker - Cannot test until format is built

---

## Installation Locations (Standard)

### VST3
- **Windows:** `C:\Program Files\Common Files\VST3\BTZ.vst3\`
- **macOS:** `/Library/Audio/Plug-Ins/VST3/BTZ.vst3/`
- **Linux:** `~/.vst3/BTZ.vst3/`

### AU
- **macOS:** `/Library/Audio/Plug-Ins/Components/BTZ.component/`

### AAX
- **Windows:** `C:\Program Files\Common Files\Avid\Audio\Plug-Ins\BTZ.aaxplugin\`
- **macOS:** `/Library/Application Support/Avid/Audio/Plug-Ins/BTZ.aaxplugin/`

---

## Build System Status

### Current Build
```bash
cd BTZ_JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Output: VST3, Standalone (AU on macOS)
```

### Build Outputs
- ✅ VST3: `BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3`
- ✅ Standalone: `BTZ_artefacts/Release/Standalone/BTZ - The Box Tone Zone`
- ⚠️ AU: `BTZ_artefacts/Release/AU/BTZ - The Box Tone Zone.component` (macOS only)
- ❌ AAX: Not built (requires AAX SDK)

---

## Validation Status

| Validator | Format | Run? | Result |
|-----------|--------|------|--------|
| **pluginval** | VST3 | ❌ | Not run yet |
| **auval** | AU | ❌ | Not run yet (needs macOS) |
| **Pro Tools Scan** | AAX | ❌ | AAX not built |

---

## Ship Readiness

### Blocking Issues (P0)
1. ❌ **pluginval not run** - Must pass strictness-level 10
2. ❌ **No cross-DAW testing** - Must test in 7+ DAWs
3. ❌ **AAX not available** - Pro market blocker

### Critical Issues (P1)
1. ⚠️ **AU not built on macOS** - Logic Pro market
2. ⚠️ **No automated validation in CI** - Regression risk

### Important Issues (P2)
1. ⚠️ **No HiDPI testing** - 4K/Retina users
2. ⚠️ **No installer** - Manual copy required

---

## Immediate Actions

### Week 1: Validation
1. **Run pluginval** on VST3 (all platforms)
2. **Build AU** on macOS
3. **Run auval** on macOS
4. **Fix any validation failures**

### Week 2: Cross-DAW Testing
1. **Test in Reaper** (reference, easiest)
2. **Test in Ableton Live** (popular, VST3)
3. **Test in FL Studio** (64-step automation check)
4. **Test in Studio One** (state determinism check)

### Week 3: AAX Preparation
1. **Apply for Avid developer access**
2. **Update CMakeLists.txt** with AAX scaffolding
3. **Document AAX threading requirements**
4. **Create AAX threading tests**

### Week 4: Production
1. **Integrate pluginval into CI**
2. **Create installer** (Windows NSIS, macOS PKG)
3. **Code signing** (Windows DigiCert, macOS Apple ID)
4. **Final smoke test** in all DAWs

---

## Long-Term Roadmap

### Q1 2026
- ✅ VST3 validated and cross-DAW tested
- ✅ AU validated on macOS
- ⚠️ AAX build (pending SDK approval)
- ✅ Automated validation in CI

### Q2 2026
- ✅ AAX validated in Pro Tools
- ✅ PACE iLok integration
- ✅ Installers for all platforms
- ✅ Code signing and notarization

### Q3 2026
- ✅ Public beta testing
- ✅ Commercial release

---

**Next:** Complete validation suite (pluginval + auval + cross-DAW tests)
