# BTZ - Ship Readiness Gap Analysis & Action Plan

**Project**: BTZ - The Box Tone Zone Enhancer
**Current Status**: 85% Ready (Build Validated, Code Complete)
**Target Status**: 100% Ship-Ready (Commercial Release)
**Date**: 2026-01-08

---

## EXECUTIVE SUMMARY

**Current Achievement**: ✅ **Build Quality Validation Complete**
- All code fixes implemented and verified
- Clean build with 0 errors
- Binary artifacts validated
- RT-safety confirmed
- VST3 exports verified

**Critical Gap**: The difference between **"valid binary"** and **"commercially shippable product"**

Everything validated so far proves the plugin **builds correctly**. What's missing is proof that it **behaves correctly** under:
- Real DAW hosts (lifecycle management, automation, edge cases)
- Extended runtime (memory leaks, stability)
- Production deployment (packaging, signing, multi-platform)
- User scenarios (UX polish, migration testing, performance at scale)

---

## GAP ANALYSIS: 10 CRITICAL AREAS

### 1. Host-Level Validation ⚠️ **CRITICAL GAP**

**What We've Validated**:
- ✅ Binary is valid VST3 format
- ✅ Required exports present (GetPluginFactory, ModuleEntry, ModuleExit)
- ✅ RT-safe code (no allocations in processBlock)

**What's Still Missing**:
- ❌ pluginval --strictness-level 10 (VST3 spec compliance)
- ❌ DAW instantiation behavior
- ❌ Host lifecycle stress testing
- ❌ Format-specific validation (AU, AAX)

**Why This Matters**:
Your script validates the binary is **structurally correct**, but not that it **behaves correctly** under host contracts. Hosts do aggressive things:
- Rapid create/destroy cycles
- Suspend/resume audio engine
- Change sample rate mid-session
- Variable buffer sizes (32 samples in Ableton)
- Offline bounce with different settings
- Editor open/close during processing

**Impact if Shipped Without**:
- Crashes in specific DAWs
- Parameter automation glitches
- State corruption on save/load
- Rejection from plugin marketplaces (PluginBoutique, Splice, etc.)

**Action Plan**:

#### 1.1 pluginval Validation (P0 - Ship Blocker)
**Priority**: CRITICAL
**Time**: 30 minutes
**Blocker**: Network access or manual download

```bash
# Install pluginval
wget https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Linux.zip
unzip pluginval_Linux.zip
sudo mv pluginval /usr/local/bin/

# Run validation
pluginval --strictness-level 10 \
  --validate-in-process \
  --timeout-ms 30000 \
  --verbose \
  "build/BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3"

# Expected: "Results: 0 plugins failed, 1 passed"
```

**Success Criteria**: Zero failures at strictness-10

#### 1.2 DAW Matrix Testing (P0 - Ship Blocker)
**Priority**: CRITICAL
**Time**: 4-6 hours
**Required DAWs**: Minimum 3, Recommended 6+

| DAW | Priority | Test Focus | Platform |
|-----|----------|-----------|----------|
| **Reaper** | P0 | Edge cases, routing, stress test | Win/Mac/Linux |
| **Ableton Live** | P0 | 32-sample buffers, freeze/unfreeze | Win/Mac |
| **FL Studio** | P0 | Aggressive automation, state save/load | Win/Mac |
| Cubase | P1 | VST3 reference host | Win/Mac |
| Studio One | P1 | Common compatibility issues | Win/Mac |
| Pro Tools | P1* | *Only if shipping AAX | Mac |
| Logic Pro | P1* | *Only if shipping AU | Mac |

**Test Protocol Per DAW**:
```
1. Scan plugins → Verify BTZ appears
2. Instantiate on audio track
3. Play audio through plugin
4. Automate 3+ parameters rapidly
5. Save session → Close → Reopen → Verify state
6. Freeze/bounce track → Compare output
7. Create 10 instances → Play session
8. Open/close editor 20x while playing
9. Change buffer size (32/64/128/512/2048)
10. Change sample rate (44.1/48/88.2/96)
11. Suspend/resume audio engine
12. Offline bounce vs realtime → Compare
```

**Success Criteria**:
- No crashes in any scenario
- State persists across save/load
- No audible glitches/pops
- UI responds smoothly
- CPU stable across all tests

#### 1.3 Lifecycle Stress Testing (P0 - Ship Blocker)
**Priority**: CRITICAL
**Time**: 2 hours
**Tool**: Custom script + Reaper ReaScript

**Stress Test Script**:
```python
# stress_test.py - Reaper ReaScript
for i in range(100):
    # Create instance
    track = reaper.InsertTrackAtIndex(0, True)
    reaper.TrackFX_AddByName(track, "BTZ - The Box Tone Zone", False, -1)

    # Randomize parameters
    for param in range(20):
        reaper.TrackFX_SetParam(track, 0, param, random.random())

    # Open/close editor
    reaper.TrackFX_Show(track, 0, 3)  # Open
    time.sleep(0.1)
    reaper.TrackFX_Show(track, 0, 2)  # Close

    # Process audio
    reaper.PlayPreview(audio_source)
    time.sleep(2)

    # Save/load state
    chunk = reaper.TrackFX_GetFXChunk(track, 0)
    reaper.TrackFX_SetFXChunk(track, 0, chunk)

    # Destroy instance
    reaper.DeleteTrack(track)

    # Check for leaks every 10 iterations
    if i % 10 == 0:
        print(f"Iteration {i}, Memory: {get_process_memory()}")
```

**Success Criteria**:
- No crashes after 100 iterations
- Memory stable (< 5% growth)
- No audio glitches
- No UI freezes

---

### 2. Format-Specific Validation ⚠️ **PLATFORM GAP**

**Current Status**: Linux VST3 only

**Missing Formats**:
- ❌ macOS VST3
- ❌ macOS AU (Audio Units)
- ❌ Windows VST3
- ❌ Windows AAX (Pro Tools) - if targeting

**Action Plan**:

#### 2.1 Define Format Matrix (P0 - Business Decision)
**Priority**: CRITICAL (before marketing)
**Time**: 1 hour (decision + documentation)

**Decision Required**: Which formats to ship?

**Recommendation for v1.0**:
```
Tier 1 (Required):
  ✅ macOS VST3 (64-bit, arm64 + x86_64 universal)
  ✅ Windows VST3 (64-bit)
  ⚠️ Linux VST3 (optional - small market)

Tier 2 (Recommended):
  ✅ macOS AU (Logic/GarageBand users)
  ⚠️ Windows AAX (Pro Tools users - requires iLok/PACE)

Tier 3 (Future):
  ⏸️ iOS AUv3 (mobile market)
  ⏸️ CLAP (emerging format)
```

**Impact of Format Choices**:
- **VST3 only**: ~80% of DAW users
- **+ AU**: ~90% coverage (Mac users)
- **+ AAX**: ~95% coverage (pros)

#### 2.2 macOS Build Setup (P0 if shipping Mac)
**Priority**: CRITICAL
**Time**: 2-3 hours (initial setup)

**Requirements**:
- macOS development machine (or CI/CD runner)
- Xcode 14+ with command-line tools
- CMake 3.15+
- JUCE 7.0.12 (already have)

**Build Commands**:
```bash
# macOS Universal Binary (Apple Silicon + Intel)
cd BTZ_JUCE/build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
         -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13

cmake --build . --config Release

# Output: BTZ_artefacts/Release/VST3/BTZ - The Box Tone Zone.vst3
#         BTZ_artefacts/Release/AU/BTZ - The Box Tone Zone.component
```

**AU Validation**:
```bash
# Official Apple validation tool
auval -v aufx Btzp Btzz

# Expected: "PASSED VALIDATION"
```

#### 2.3 Windows Build Setup (P0 if shipping Windows)
**Priority**: CRITICAL
**Time**: 2-3 hours (initial setup)

**Requirements**:
- Windows 10/11 development machine
- Visual Studio 2019/2022 (Community Edition OK)
- CMake 3.15+
- JUCE 7.0.12

**Build Commands**:
```powershell
# Windows 64-bit
cd BTZ_JUCE\build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# Output: BTZ_artefacts\Release\VST3\BTZ - The Box Tone Zone.vst3
```

---

### 3. Warning Policy & Code Quality ⚠️ **TECHNICAL DEBT**

**Current Status**: 62 warnings (all marked "non-critical")

**Why This Matters**:
Warnings are "non-critical" until they hide a real bug. For commercial release:
- Sign conversion can hide buffer overflow
- Variable shadowing can hide logic errors during refactoring
- Unused parameters can indicate API misunderstanding

**Action Plan**:

#### 3.1 Warning Budget & Policy (P1 - Quality Gate)
**Priority**: HIGH
**Time**: 2 hours

**Create `WARNINGS_POLICY.md`**:
```markdown
# BTZ Warning Policy

## Allowed Warnings (with justification)

### Unused Parameters (15 warnings)
**Category**: `-Wunused-parameter`
**Justification**: Required by JUCE virtual function interfaces
**Example**: `void prepare(const ProcessSpec& spec)` - spec unused in some modules
**Action**: Add `[[maybe_unused]]` or `(void)spec;`
**Budget**: ≤ 20 warnings

### Sign Conversion (12 warnings)
**Category**: `-Wsign-conversion`
**Justification**: Mixing int and size_t in safe contexts
**Action**: Review each case, add static_cast where appropriate
**Budget**: ≤ 5 warnings (reduce from 12)

### Variable Shadowing (8 warnings)
**Category**: `-Wshadow`
**Justification**: Intentional JUCE pattern for setter parameters
**Action**: Rename shadowed variables to avoid confusion
**Budget**: ≤ 3 warnings (reduce from 8)

### Float Equality (6 warnings)
**Category**: `-Wfloat-equal`
**Justification**: Coefficient caching optimization (exact check needed)
**Action**: Document with comment explaining why exact equality is correct
**Budget**: ≤ 6 warnings (acceptable)

## Forbidden Warnings (CI will fail)

- Uninitialized variables
- Array bounds violations
- Use-after-free
- Null pointer dereference (static analysis)
```

#### 3.2 Incremental Warning Reduction (P2 - Continuous Improvement)
**Priority**: MEDIUM
**Time**: 4-6 hours

**Phase 1: Fix High-Risk Warnings** (P1)
```bash
# Fix sign conversions that could overflow
# Example from SparkLimiter.h:99
lookAheadBuffer[static_cast<size_t>(writePos)] = sample;  // Add explicit cast

# Fix shadowing in RBJFilters.h:40
void prepare(double newSampleRate) {  // Rename parameter
    this->sampleRate = newSampleRate;
}
```

**Phase 2: Add CI Warning Gate** (P2)
```yaml
# .github/workflows/build.yml
- name: Build with warning budget
  run: |
    cmake --build . 2>&1 | tee build.log
    WARNING_COUNT=$(grep -c "warning:" build.log || true)
    if [ "$WARNING_COUNT" -gt 30 ]; then
      echo "ERROR: Warning count ($WARNING_COUNT) exceeds budget (30)"
      exit 1
    fi
```

---

### 4. DSP Correctness Under Automation ⚠️ **AUDIBLE QUALITY GAP**

**Current Status**: Block-rate parameter smoothing

**What's Missing**: Sub-block smoothing (P2-5 from original audit)

**Why This Matters**:
Current implementation calls `skip(numSamples)` once per block. For 512-sample buffers:
- Parameter changes every 512 samples (10ms @ 48kHz)
- Fast automation sounds "steppy"
- Potential zipper noise at low buffer sizes

**Action Plan**:

#### 4.1 Implement Sub-Block Parameter Smoothing (P1 - Quality)
**Priority**: HIGH (audible improvement)
**Time**: 3-4 hours
**Files**: `PluginProcessor.cpp`

**Current Code (Block-Rate)**:
```cpp
// Lines 208-228 (CURRENT - BLOCK-RATE)
smoothedPunch.setTargetValue(punchParam->load());
smoothedPunch.skip(numSamples);  // ❌ Once per block
float punchAmount = smoothedPunch.getCurrentValue();

// Use punchAmount for entire block
transientShaper.setPunch(punchAmount);
```

**Improved Code (Sub-Block)**:
```cpp
// IMPROVED - SUB-BLOCK SMOOTHING (16-sample chunks)
constexpr int subBlockSize = 16;  // From BTZConstants

// Set targets once
smoothedPunch.setTargetValue(punchParam->load());
// ... all parameters ...

// Process in sub-blocks
for (int pos = 0; pos < numSamples; pos += subBlockSize)
{
    const int samplesThisBlock = std::min(subBlockSize, numSamples - pos);

    // Advance smoothers
    smoothedPunch.skip(samplesThisBlock);
    smoothedWarmth.skip(samplesThisBlock);
    // ... all parameters ...

    // Get current values for this sub-block
    float punchAmount = smoothedPunch.getCurrentValue();
    float warmthAmount = smoothedWarmth.getCurrentValue();
    // ... all parameters ...

    // Update DSP modules
    transientShaper.setPunch(punchAmount);
    saturation.setWarmth(warmthAmount);
    // ...

    // Process this sub-block
    auto subBlock = block.getSubBlock(pos, samplesThisBlock);
    juce::dsp::ProcessContextReplacing<float> subContext(subBlock);

    // DSP chain for sub-block
    if (punchAmount > 0.01f)
        transientShaper.process(subContext);
    // ... rest of chain ...
}
```

**Benefits**:
- Smooth parameter automation (no zippers)
- Fast modulation sounds natural
- "Pro feel" parameter response
- **CPU impact**: ~5-10% increase (sub-block overhead)

#### 4.2 Automation Torture Test (P1 - Validation)
**Priority**: HIGH
**Time**: 1 hour
**Tool**: Reaper + ReaScript

**Test Script**:
```python
# automation_stress_test.py
# Test rapid parameter modulation

track = reaper.GetTrack(0, 0)
fx_index = reaper.TrackFX_AddByName(track, "BTZ", False, -1)

# Create LFO automation on multiple parameters
for param_idx in [0, 1, 2, 3]:  # punch, warmth, boom, mix
    envelope = reaper.GetFXEnvelope(track, fx_index, param_idx, True)

    # Add 1000 automation points (rapid modulation)
    for i in range(1000):
        time = i * 0.01  # 10ms intervals
        value = 0.5 + 0.5 * math.sin(2 * math.pi * 5 * time)  # 5Hz sine
        reaper.InsertEnvelopePoint(envelope, time, value, 0, 0, False)

# Play and listen for artifacts
reaper.Play()
```

**Listen For**:
- ❌ Zipper noise
- ❌ Clicks/pops
- ❌ Stair-stepping in modulation
- ✅ Smooth, continuous parameter changes

---

### 5. Performance Profiling ⚠️ **UNKNOWN COST**

**Current Status**: RT-safe, but no performance numbers

**What's Missing**: Actual CPU measurements under real load

**Why This Matters**:
"RT-safe" ≠ "efficient". Plugin could be:
- Burning CPU on denormal handling
- Inefficient filter implementations
- Oversampling overhead too high
- Metering calculations in hot path

**Action Plan**:

#### 5.1 CPU Benchmark Suite (P1 - Performance)
**Priority**: HIGH
**Time**: 3-4 hours
**Tool**: Reaper performance monitor + custom script

**Benchmark Configuration Matrix**:
```
Sample Rates: 44100, 48000, 96000
Buffer Sizes: 64, 128, 256, 512, 2048
Oversampling: 1x, 2x, 4x, 8x, 16x
Instance Count: 1, 5, 10, 20
Platforms: macOS M1, macOS Intel, Windows i7-12700K, Linux
```

**Test Script**:
```python
# performance_benchmark.py
import subprocess
import psutil
import time

def measure_cpu_usage(num_instances, sample_rate, buffer_size):
    # Create Reaper project with N instances
    project = create_reaper_project(
        plugin="BTZ",
        instances=num_instances,
        sample_rate=sample_rate,
        buffer_size=buffer_size
    )

    # Play for 60 seconds and measure CPU
    process = psutil.Process(reaper_pid)
    cpu_samples = []

    for i in range(60):
        cpu_samples.append(process.cpu_percent(interval=1.0))

    return {
        'avg_cpu': sum(cpu_samples) / len(cpu_samples),
        'max_cpu': max(cpu_samples),
        'min_cpu': min(cpu_samples)
    }

# Run benchmark matrix
results = []
for sr in [44100, 48000, 96000]:
    for bs in [64, 128, 512]:
        for instances in [1, 5, 10]:
            result = measure_cpu_usage(instances, sr, bs)
            results.append({
                'sample_rate': sr,
                'buffer_size': bs,
                'instances': instances,
                **result
            })

# Generate report
print_performance_table(results)
```

**Performance Targets** (10 instances @ 48kHz/128 buffer):
```
Excellent:  < 20% CPU
Good:       20-40% CPU
Acceptable: 40-60% CPU
Poor:       > 60% CPU (needs optimization)
```

#### 5.2 Hotspot Profiling (P1 - Optimization)
**Priority**: HIGH (if CPU > targets)
**Time**: 2-3 hours
**Tool**: Instruments (macOS), VTune (Windows), perf (Linux)

**macOS Instruments Workflow**:
```bash
# Launch plugin in Reaper
# Attach Instruments Time Profiler
instruments -t "Time Profiler" -p `pgrep REAPER`

# Process audio for 60 seconds
# Analyze call tree

# Expected hotspots:
# - processBlock (should be top)
# - Oversampling operations
# - Filter processing
# - Metering (if in hot path - should be minimal)

# Fix hotspots:
# - Move metering to lower priority thread if > 5%
# - Optimize filter implementations if > 20%
# - Consider SIMD for oversampling if > 30%
```

**Optimization Priorities**:
1. **Oversampling** (likely 30-40% of CPU)
2. **Filters** (RBJ biquads, TPT filters - 20-30%)
3. **Nonlinear processing** (saturation, limiting - 15-20%)
4. **Metering** (should be < 5%, move to GUI thread if higher)

---

### 6. Soak Testing & Memory Leak Detection ⚠️ **STABILITY GAP**

**Current Status**: Plugin runs correctly in short tests

**What's Missing**: Long-term stability proof

**Why This Matters**:
Bugs that appear after hours/days:
- Memory leaks (gradual growth)
- Timer/callback leaks
- UI invalidation loops
- Resource exhaustion

**Action Plan**:

#### 6.1 24-Hour Soak Test (P0 - Ship Blocker)
**Priority**: CRITICAL
**Time**: 24 hours runtime + 2 hours setup
**Platform**: All target platforms

**Soak Test Script**:
```python
# soak_test.py - Run for 24 hours
import reaper_python as rpr
import random
import time

start_time = time.time()
iteration = 0

# Create 10 instances
tracks = []
for i in range(10):
    track = rpr.InsertTrackAtIndex(i, True)
    rpr.TrackFX_AddByName(track, "BTZ - The Box Tone Zone", False, -1)
    tracks.append(track)

# Run for 24 hours
while (time.time() - start_time) < 86400:  # 24 hours
    iteration += 1

    # Random actions every 10 seconds
    action = random.choice([
        'automate_parameters',
        'open_close_editor',
        'save_load_state',
        'change_buffer_size',
        'suspend_resume'
    ])

    if action == 'automate_parameters':
        track = random.choice(tracks)
        param = random.randint(0, 19)
        value = random.random()
        rpr.TrackFX_SetParam(track, 0, param, value)

    elif action == 'open_close_editor':
        track = random.choice(tracks)
        rpr.TrackFX_Show(track, 0, 3)  # Open
        time.sleep(1)
        rpr.TrackFX_Show(track, 0, 2)  # Close

    elif action == 'save_load_state':
        track = random.choice(tracks)
        chunk = rpr.TrackFX_GetFXChunk(track, 0, "", 1000000)
        rpr.TrackFX_SetFXChunk(track, 0, chunk[2])

    # Log memory every hour
    if iteration % 360 == 0:  # Every hour
        mem_mb = get_process_memory() / 1024 / 1024
        print(f"Hour {iteration/360}: Memory = {mem_mb:.1f} MB")

    time.sleep(10)

print("24-hour soak test COMPLETE")
print(f"Total iterations: {iteration}")
print(f"Final memory: {get_process_memory() / 1024 / 1024:.1f} MB")
```

**Success Criteria**:
- ✅ No crashes
- ✅ Memory growth < 10% over 24 hours
- ✅ CPU stable (no gradual increase)
- ✅ Audio output unchanged

#### 6.2 Memory Leak Detection (P0 - Ship Blocker)
**Priority**: CRITICAL
**Time**: 2 hours (per platform)

**macOS - Instruments Leaks**:
```bash
# Launch plugin in standalone mode
./BTZ_artefacts/Release/Standalone/BTZ\ -\ The\ Box\ Tone\ Zone &

# Attach Leaks instrument
instruments -t Leaks -p `pgrep BTZ`

# Run for 1 hour with automation
# Check for leaked blocks

# Expected: 0 leaks
```

**Windows - Visual Studio Memory Profiler**:
```
1. Open Reaper in Visual Studio (Attach to Process)
2. Tools → Performance Profiler → Memory Usage
3. Take snapshot at start
4. Run soak test for 1 hour
5. Take snapshot at end
6. Compare snapshots

Expected: < 100 KB growth
```

**Linux - Valgrind**:
```bash
# Run under Valgrind (slow but thorough)
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=valgrind.log \
         ./BTZ_artefacts/Release/Standalone/BTZ\ -\ The\ Box\ Tone\ Zone

# Run for 10 minutes (Valgrind is 10x slower)
# Check valgrind.log

# Expected: "0 bytes in 0 blocks are definitely lost"
```

---

### 7. Release Engineering ⚠️ **DEPLOYMENT GAP**

**Current Status**: Valid ELF on Linux

**What's Missing**: Multi-platform release pipeline

**Why This Matters**:
Commercial plugin release requires:
- Code signing (macOS notarization, Windows Authenticode)
- Installers (drag-and-drop or package managers)
- Version stamping
- Crash symbol storage

**Action Plan**:

#### 7.1 macOS Code Signing & Notarization (P0 for Mac)
**Priority**: CRITICAL
**Time**: 4-6 hours (initial setup)
**Requirements**: Apple Developer Account ($99/year)

**Signing Workflow**:
```bash
# 1. Sign VST3 bundle
codesign --deep --force --verify --verbose \
  --timestamp --options runtime \
  --sign "Developer ID Application: Your Name" \
  "BTZ - The Box Tone Zone.vst3"

# 2. Sign AU component
codesign --deep --force --verify --verbose \
  --timestamp --options runtime \
  --sign "Developer ID Application: Your Name" \
  "BTZ - The Box Tone Zone.component"

# 3. Create installer package
pkgbuild --root ./install_root \
  --identifier com.btzaudio.btz \
  --version 1.0.0 \
  --install-location /Library/Audio/Plug-Ins \
  --sign "Developer ID Installer: Your Name" \
  BTZ_Installer_1.0.0.pkg

# 4. Notarize with Apple
xcrun notarytool submit BTZ_Installer_1.0.0.pkg \
  --apple-id your@email.com \
  --team-id TEAMID \
  --password @keychain:AC_PASSWORD \
  --wait

# 5. Staple notarization ticket
xcrun stapler staple BTZ_Installer_1.0.0.pkg

# 6. Verify
spctl -a -vvv -t install BTZ_Installer_1.0.0.pkg
```

**Hardened Runtime Entitlements**:
```xml
<!-- BTZ.entitlements -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" ...>
<plist version="1.0">
<dict>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
    <key>com.apple.security.device.audio-input</key>
    <true/>
</dict>
</plist>
```

#### 7.2 Windows Code Signing (P0 for Windows)
**Priority**: CRITICAL
**Time**: 2-3 hours
**Requirements**: Code signing certificate ($100-400/year)

**Signing Workflow**:
```powershell
# Sign VST3 DLL
signtool sign /f your_certificate.pfx /p password /tr http://timestamp.digicert.com /td sha256 /fd sha256 "BTZ - The Box Tone Zone.vst3\Contents\x86_64-win\BTZ - The Box Tone Zone.dll"

# Create installer with InnoSetup
iscc BTZ_Installer.iss

# Sign installer
signtool sign /f your_certificate.pfx /p password /tr http://timestamp.digicert.com /td sha256 /fd sha256 BTZ_Installer_1.0.0.exe

# Verify
signtool verify /pa BTZ_Installer_1.0.0.exe
```

#### 7.3 Installer Creation (P0 - User Experience)
**Priority**: CRITICAL
**Time**: 4-6 hours

**macOS Installer Requirements**:
```
Install Locations:
  VST3: /Library/Audio/Plug-Ins/VST3/BTZ - The Box Tone Zone.vst3
  AU:   /Library/Audio/Plug-Ins/Components/BTZ - The Box Tone Zone.component
  Presets: ~/Music/Audio Music Apps/Presets/BTZ Audio/BTZ/

Post-install script:
  - Verify plugin can be loaded
  - Register with system
  - Show completion message
```

**Windows Installer (InnoSetup)**:
```iss
[Setup]
AppName=BTZ - The Box Tone Zone
AppVersion=1.0.0
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes

[Files]
Source: "BTZ - The Box Tone Zone.vst3\*"; DestDir: "{commoncf64}\VST3\BTZ - The Box Tone Zone.vst3"; Flags: recursesubdirs

[Code]
// Detect DAW installations
// Rescan plugins in common DAWs
```

#### 7.4 Version Stamping (P1 - Tracking)
**Priority**: HIGH
**Time**: 1 hour

**Add Version Info to Binary**:
```cpp
// BTZ_JUCE/Source/PluginProcessor.h
#define BTZ_VERSION_MAJOR 1
#define BTZ_VERSION_MINOR 0
#define BTZ_VERSION_PATCH 0
#define BTZ_VERSION_BUILD 123  // CI build number
#define BTZ_VERSION_STRING "1.0.0"

// Show in about screen
// Include in crash reports
// Stamp in state files
```

**Git Tagging**:
```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

---

### 8. Migration & Compatibility Testing ⚠️ **REGRESSION RISK**

**Current Status**: Migration framework exists (P1-6)

**What's Missing**: Proof it works with real state files

**Why This Matters**:
Migration code is **untested**. Could:
- Corrupt user presets
- Crash on old state files
- Lose parameter values silently

**Action Plan**:

#### 8.1 Golden State File Collection (P1 - Regression)
**Priority**: HIGH
**Time**: 2 hours

**Create Test Assets**:
```
tests/golden_states/
├── v0.9.0_default.vstpreset
├── v0.9.0_punchy.vstpreset
├── v0.9.0_warm.vstpreset
├── v1.0.0_default.vstpreset
├── v1.0.0_extreme.vstpreset
├── corrupted_truncated.vstpreset
├── corrupted_wrong_type.vstpreset
└── legacy_no_version.vstpreset
```

**Collection Process**:
```bash
# For each BTZ version:
1. Load plugin
2. Save default preset
3. Save preset with all parameters at extremes
4. Save preset with common use-case settings
5. Commit to test/golden_states/
```

#### 8.2 Migration Test Suite (P1 - Automation)
**Priority**: HIGH
**Time**: 3-4 hours

**Test Script**:
```cpp
// tests/test_migration.cpp
#include <catch2/catch.hpp>
#include "PluginProcessor.h"

TEST_CASE("Load v0.9.0 presets", "[migration]") {
    BTZAudioProcessor processor;

    // Load golden state
    auto state = loadBinaryFile("tests/golden_states/v0.9.0_default.vstpreset");
    processor.setStateInformation(state.data(), state.size());

    // Verify parameters loaded correctly
    REQUIRE(processor.apvts.getParameter("punch")->getValue() == Approx(0.5f));
    REQUIRE(processor.apvts.getParameter("warmth")->getValue() == Approx(0.5f));

    // Verify version upgraded
    auto currentState = processor.getStateAsXml();
    REQUIRE(currentState->getStringAttribute("pluginVersion") == "1.0.0");
}

TEST_CASE("Handle corrupted state gracefully", "[migration]") {
    BTZAudioProcessor processor;

    // Load truncated state
    auto state = loadBinaryFile("tests/golden_states/corrupted_truncated.vstpreset");

    // Should not crash
    REQUIRE_NOTHROW(processor.setStateInformation(state.data(), state.size()));

    // Should fall back to defaults
    REQUIRE(processor.apvts.getParameter("punch")->getValue() == Approx(0.5f));
}

TEST_CASE("Round-trip stability", "[migration]") {
    BTZAudioProcessor processor;

    // Set parameters
    processor.apvts.getParameter("punch")->setValueNotifyingHost(0.75f);
    processor.apvts.getParameter("warmth")->setValueNotifyingHost(0.3f);

    // Save state
    juce::MemoryBlock state1;
    processor.getStateInformation(state1);

    // Load state
    processor.setStateInformation(state1.getData(), state1.getSize());

    // Save again
    juce::MemoryBlock state2;
    processor.getStateInformation(state2);

    // States should be identical
    REQUIRE(state1 == state2);
}
```

**CI Integration**:
```yaml
# .github/workflows/test.yml
- name: Run migration tests
  run: |
    cd tests
    ./test_migration
    if [ $? -ne 0 ]; then
      echo "Migration tests failed!"
      exit 1
    fi
```

---

### 9. Disabled Modules Decision ⚠️ **TECHNICAL DEBT**

**Current Status**: 4 modules excluded from build

**What's Missing**: Clear decision on these modules

**Why This Matters**:
Dead code in repo creates confusion:
- New developers waste time
- Unclear product roadmap
- Potential licensing issues (WDF library?)

**Action Plan**:

#### 9.1 Module Audit & Decision (P2 - Cleanup)
**Priority**: MEDIUM
**Time**: 2 hours

**Decision Matrix**:
```
For each disabled module, decide:
  Option A: FIX (add to v1.0 roadmap)
  Option B: DEFER (move to feature branch for v1.1+)
  Option C: DELETE (not part of product vision)

Current Disabled Modules:
  1. AdvancedSaturation.cpp
     - Decision: ?
     - Reason: ?
     - Action: ?

  2. AdvancedTransientShaper.cpp
     - Decision: ?
     - Reason: ?
     - Action: ?

  3. WDFSaturation.cpp
     - Decision: ?
     - Reason: WDF library licensing? experimental?
     - Action: ?

  4. LUFSMeter.cpp
     - Decision: DEFER to v1.1 (P2-1 task)
     - Reason: Needs ITU-R BS.1770-4 implementation
     - Action: Move to feature/accurate-lufs branch
```

**Recommendation**:
```markdown
# v1.0 Scope (KEEP ONLY):
- Core DSP chain (TransientShaper, Saturation, SPARK, SHINE, etc.)
- Simplified LUFS metering (current RMS approximation)
- No experimental WDF saturation

# v1.1 Roadmap (DEFER):
- Accurate ITU-R BS.1770-4 LUFS metering
- Advanced modules (if justified by user feedback)

# DELETE:
- Any modules with unclear licensing
- Duplicated functionality
- Experimental code not needed for product
```

#### 9.2 Clean Repository (P2 - Maintenance)
**Priority**: MEDIUM
**Time**: 1 hour

**Actions**:
```bash
# Create feature branch for deferred modules
git checkout -b feature/advanced-dsp
git mv Source/DSP/AdvancedSaturation.* .
git mv Source/DSP/AdvancedTransientShaper.* .
git mv Source/DSP/WDFSaturation.* .
git commit -m "Move experimental modules to feature branch"
git push origin feature/advanced-dsp

# Back to main branch
git checkout main

# Delete from main (now in feature branch)
git rm Source/DSP/Advanced*.{cpp,h}
git rm Source/DSP/WDFSaturation.*
git commit -m "Clean up experimental modules (moved to feature/advanced-dsp)"

# Update CMakeLists.txt - remove comments
# Update README - clarify v1.0 scope
```

---

### 10. Professional Polish & UX Validation ⚠️ **USER EXPERIENCE GAP**

**Current Status**: Technically functional

**What's Missing**: User-facing polish

**Why This Matters**:
First impression matters:
- Default preset should sound amazing
- UI should be intuitive
- Tooltips/labels must be clear
- Bypass behavior obvious

**Action Plan**:

#### 10.1 Default Preset Creation (P1 - First Impression)
**Priority**: HIGH
**Time**: 2-3 hours (with audio engineer)

**Process**:
```
1. Load plugin with factory defaults (0.5 for all params)
2. Process 10 different sources (drums, vocals, bass, etc.)
3. Does it sound good "out of the box"?
4. If not, tune defaults to be:
   - Musical
   - Transparent
   - Subtle enhancement
   - No artifacts

Recommended Starting Point:
  Punch:   0.3 (subtle transient shaping)
  Warmth:  0.2 (gentle saturation)
  Boom:    0.0 (off by default)
  Spark:   DISABLED (transparent by default)
  Shine:   DISABLED (transparent by default)
  Mix:     1.0 (100% wet - but subtle processing)
```

#### 10.2 Bypass Behavior Validation (P0 - Critical)
**Priority**: CRITICAL
**Time**: 1 hour

**Test Scenarios**:
```cpp
// Test 1: Bypass is bit-perfect
void testBypassBitPerfect() {
    BTZAudioProcessor processor;

    // Generate test audio
    AudioBuffer<float> input = generateSineWave(1000.0f, 48000, 1.0);
    AudioBuffer<float> bypassed = input;

    // Set bypass
    processor.apvts.getParameter("active")->setValueNotifyingHost(0.0f);

    // Process
    processor.processBlock(bypassed, midiBuffer);

    // Should be identical
    REQUIRE(buffersEqual(input, bypassed));
}

// Test 2: Bypass ramping (no clicks)
void testBypassRamping() {
    // TODO: Implement smooth bypass ramping
    // Currently bypass is instant (could click)
    // Recommendation: 20ms ramp on bypass enable/disable
}
```

**Fix Bypass Ramping** (if clicks occur):
```cpp
// PluginProcessor.h
LinearSmoothedValue<float> bypassRamp;

// PluginProcessor.cpp prepareToPlay()
bypassRamp.reset(sampleRate, 0.02);  // 20ms ramp

// processBlock()
bool targetBypass = apvts.getRawParameterValue("active")->load() < 0.5f;
bypassRamp.setTargetValue(targetBypass ? 0.0f : 1.0f);

for (int sample = 0; sample < numSamples; ++sample) {
    float wetGain = bypassRamp.getNextValue();
    // Apply crossfade between dry and wet
}
```

#### 10.3 UI/UX Final Checklist (P1 - Polish)
**Priority**: HIGH
**Time**: 2-3 hours

**Checklist**:
```markdown
## Visual Polish
- [ ] Knobs respond smoothly to mouse drag
- [ ] Parameter values display correctly
- [ ] Meters update in real-time
- [ ] No UI freezing during audio processing
- [ ] Resizable UI (if applicable)
- [ ] Retina/HiDPI graphics

## Parameter Behavior
- [ ] All parameters within labeled ranges
- [ ] Parameter units shown (dB, Hz, %, ms)
- [ ] Default values are musical
- [ ] Extreme values don't cause artifacts
- [ ] Automation responds smoothly

## User Feedback
- [ ] Tooltips on all controls
- [ ] Preset browser functional
- [ ] A/B comparison works
- [ ] Undo/redo works (if implemented)
- [ ] No confusing behavior

## Accessibility
- [ ] Keyboard navigation works
- [ ] High-contrast mode (if applicable)
- [ ] Screen reader compatible (if possible)
```

#### 10.4 Beta User Manual QA (P1 - Documentation)
**Priority**: HIGH
**Time**: 3-4 hours

**Create Beta Testing Guide**:
```markdown
# BTZ v1.0 Beta Testing Guide

## Installation
1. Download installer for your platform
2. Run installer (macOS: drag to Applications, Windows: run .exe)
3. Rescan plugins in your DAW
4. Look for "BTZ - The Box Tone Zone" in Effects → BTZ Audio

## Quick Test
1. Load on drum bus
2. Adjust Punch knob → Should enhance transients
3. Adjust Warmth → Should add harmonic richness
4. Enable SPARK → Should add loudness/glue
5. Save preset → Close DAW → Reopen → Preset should load

## Reporting Bugs
Include:
- OS version (macOS 13.2, Windows 11, etc.)
- DAW (Reaper 6.70, Ableton 11.3, etc.)
- Steps to reproduce
- Expected vs actual behavior
- Screenshot or screen recording

Submit to: beta@btzaudio.com or GitHub Issues
```

---

## PRIORITIZED ACTION PLAN

### Week 1: Critical Blockers (P0)

**Day 1-2**: Host Validation
- [ ] Install pluginval
- [ ] Run pluginval --strictness-level 10
- [ ] Fix any failures
- [ ] Test in Reaper, Ableton, FL Studio (minimum 3 DAWs)

**Day 3-4**: Platform Builds
- [ ] Setup macOS build environment
- [ ] Build macOS VST3 + AU
- [ ] Run auval on AU
- [ ] Setup Windows build environment (if targeting)
- [ ] Build Windows VST3

**Day 5**: Performance & Stability
- [ ] Run CPU benchmark suite
- [ ] Run 24-hour soak test
- [ ] Run memory leak detection (Instruments/Valgrind)

**Day 6-7**: Release Engineering
- [ ] Setup code signing (macOS)
- [ ] Setup code signing (Windows)
- [ ] Create installers
- [ ] Test install/uninstall flow

### Week 2: Quality & Polish (P1)

**Day 8-9**: DSP Improvements
- [ ] Implement sub-block parameter smoothing
- [ ] Run automation torture test
- [ ] Profile and optimize hotspots (if needed)

**Day 10**: Migration Testing
- [ ] Collect golden state files
- [ ] Write migration test suite
- [ ] Run regression tests

**Day 11-12**: UX Polish
- [ ] Tune default preset
- [ ] Validate bypass behavior
- [ ] Complete UI/UX checklist
- [ ] Create beta testing guide

**Day 13-14**: Documentation & Cleanup
- [ ] Decide on disabled modules
- [ ] Clean repository
- [ ] Write release notes
- [ ] Create user manual

### Week 3: Beta Testing

**Day 15-21**: Beta Program
- [ ] Recruit 5-10 beta testers
- [ ] Distribute beta builds
- [ ] Collect feedback
- [ ] Fix critical issues
- [ ] Iterate based on feedback

### Week 4: Release Candidate

**Day 22-25**: RC Preparation
- [ ] Incorporate beta feedback
- [ ] Final round of testing
- [ ] Final performance validation
- [ ] Create marketing materials

**Day 26-28**: Launch
- [ ] Final build with release signing
- [ ] Upload to distribution (website, PluginBoutique, etc.)
- [ ] Announce release
- [ ] Monitor for issues

---

## ESTIMATED TIME TO SHIP

| Phase | Duration | Blockers |
|-------|----------|----------|
| **Week 1: Critical Blockers** | 5-7 days | Network (pluginval), Mac/Win hardware |
| **Week 2: Quality & Polish** | 5-7 days | Audio engineer input |
| **Week 3: Beta Testing** | 7 days | Beta tester availability |
| **Week 4: Release** | 5-7 days | Final approval |

**Total**: **22-28 days** from current state to v1.0 release

**Critical Path Dependencies**:
1. pluginval access (network/manual install)
2. macOS development environment
3. Windows development environment (if targeting)
4. Code signing certificates ($200-500)
5. Beta tester pool (5-10 users)

---

## RISK MITIGATION

### High-Risk Items

1. **pluginval Failures** (Likelihood: MEDIUM, Impact: HIGH)
   - **Mitigation**: Budget 2-3 days for fixes
   - **Contingency**: Known issues list for v1.0, fix in v1.0.1

2. **Performance Issues** (Likelihood: LOW, Impact: HIGH)
   - **Mitigation**: Profile early, optimize critical path
   - **Contingency**: Reduce oversampling options, simplify DSP

3. **Platform-Specific Bugs** (Likelihood: MEDIUM, Impact: MEDIUM)
   - **Mitigation**: Test on multiple machines per platform
   - **Contingency**: Delay platform if critical bugs found

4. **Beta Tester Feedback** (Likelihood: HIGH, Impact: MEDIUM)
   - **Mitigation**: Clear acceptance criteria, prioritize critical bugs only
   - **Contingency**: v1.0.1 update within 30 days

---

## SUCCESS CRITERIA (SHIP-READY DEFINITION)

### Technical Requirements (All Must Pass)
- ✅ pluginval --strictness-level 10 (0 failures)
- ✅ Tested in 3+ major DAWs (no crashes)
- ✅ 24-hour soak test (no leaks, no crashes)
- ✅ CPU < 60% for 10 instances @ 48kHz/128
- ✅ Code signed on all platforms
- ✅ Installers tested (install + uninstall clean)

### Quality Requirements (All Must Pass)
- ✅ No zipper noise under automation
- ✅ Bypass is bit-perfect
- ✅ Migration tests pass (golden states load correctly)
- ✅ Default preset sounds good on 5+ sources
- ✅ Beta testers report 0 critical bugs

### Business Requirements (Recommended)
- ✅ User manual complete
- ✅ Marketing website ready
- ✅ Distribution channel setup (PluginBoutique, etc.)
- ✅ Support email configured
- ✅ Pricing decided

---

## CONCLUSION

**Current State**: 85% technically ready (build validated, code complete)

**Remaining Work**: The "last 15%" that makes the difference between:
- "Valid VST3 binary" → "Commercially shippable product"
- "Works on developer machine" → "Works for all users in all DAWs"
- "Code complete" → "Production tested and proven stable"

**Estimated Effort**: 22-28 days full-time work

**Critical Dependencies**:
1. Access to Mac + Windows environments
2. Code signing certificates (~$300)
3. pluginval tool (network access or manual install)
4. Beta tester pool (5-10 users)

**Recommendation**: Execute Week 1 (Critical Blockers) immediately to validate ship-readiness assumption. If any P0 issues found, may extend timeline.

---

**BTZ Audio Systems Group**
*From 85% to 100%: Bridging the Gap to Commercial Release*

**Document Version**: 1.0
**Date**: 2026-01-08
**Status**: Action Plan Ready for Execution
