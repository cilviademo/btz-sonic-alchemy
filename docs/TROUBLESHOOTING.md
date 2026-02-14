# BTZ Troubleshooting Guide

**Version**: 1.0.0
**Purpose**: Common issues and solutions for BTZ users and developers
**Last Updated**: 2026-01-08

---

## 🎯 QUICK DIAGNOSTICS

### Is BTZ Working?

**Checklist**:
1. ✅ Plugin loads in DAW without crash
2. ✅ Audio passes through (input → output)
3. ✅ Parameters respond to changes
4. ✅ No audio dropouts or glitches
5. ✅ CPU usage reasonable (<60% for 10 instances)

If ALL are ✅: BTZ is working correctly
If ANY are ❌: See specific issues below

---

## 🔴 CRITICAL ISSUES (Audio Stops/Crashes)

### Issue 1: Plugin Crashes on Load

**Symptoms**:
- DAW crashes when scanning BTZ
- Error message: "BTZ.vst3 failed validation"
- Plugin doesn't appear in DAW plugin list

**Possible Causes**:
1. Incompatible JUCE version
2. Missing dependencies (JUCE modules)
3. Platform mismatch (x86 vs ARM, 32-bit vs 64-bit)
4. Corrupted binary

**Solutions**:

#### Solution 1A: Verify Binary Architecture
```bash
# macOS - check architecture
file BTZ.vst3/Contents/MacOS/BTZ
# Should show: "Mach-O 64-bit bundle x86_64" or "arm64"

# Linux - check architecture
file BTZ.vst3/Contents/x86_64-linux/BTZ.so
# Should show: "ELF 64-bit LSB shared object, x86-64"

# Windows - check architecture
dumpbin /HEADERS BTZ.vst3 | findstr machine
# Should show: "x64"
```

**Fix**: Download correct architecture for your system

---

#### Solution 1B: Check JUCE Dependencies
```bash
# macOS - check linked libraries
otool -L BTZ.vst3/Contents/MacOS/BTZ

# Linux - check linked libraries
ldd BTZ.vst3/Contents/x86_64-linux/BTZ.so

# Look for missing libraries (e.g., libjuce_core.so)
```

**Fix**: Install missing dependencies or use static build

---

#### Solution 1C: Validate with pluginval
```bash
pluginval --strictness-level 5 --validate BTZ.vst3

# If crashes:
# - Check stack trace in output
# - Report to BTZ support with full log
```

**Fix**: See error message for specific issue

---

### Issue 2: No Audio Output (Silent)

**Symptoms**:
- Plugin loads but produces no sound
- Meters show zero signal
- Input passes through other plugins but not BTZ

**Diagnostics**:
```
1. Check "active" parameter (should be ON/1.0)
2. Check "mix" parameter (should be >0.0)
3. Check inputGain/outputGain (should not be -∞ dB)
4. Check if bypass mode enabled
5. Verify input signal present (check meters)
```

**Solutions**:

#### Solution 2A: Reset to Default Preset
1. Load BTZ factory default preset
2. If sound returns: parameter issue
3. If still silent: processing issue

---

#### Solution 2B: Check Parameter Values
```cpp
// Expected default values:
active = 1.0 (ON)
mix = 1.0 (100% wet)
inputGain = 0.5 (0.0 dB)
outputGain = 0.5 (0.0 dB)

// Check in DAW:
// - Automation lanes for these parameters
// - Preset may have saved incorrect values
```

**Fix**: Manually set parameters to defaults

---

#### Solution 2C: Verify Buffer Processing
If developer/advanced user:
```bash
# Enable RT-safe logging (debug build)
BTZ_ENABLE_LOGGING=1 ./BTZ_Standalone

# Check console for:
# "processBlock called: 512 samples"
# "Output RMS: X dB"
```

**Fix**: If no processBlock logs, host may not be calling plugin

---

### Issue 3: Audio Dropouts / Glitches

**Symptoms**:
- Periodic clicks, pops, or silence
- DAW shows "CPU overload" or buffer underruns
- Worse at low buffer sizes (64 samples)

**Diagnostics**:
```
1. Check CPU meter in DAW (should be <60%)
2. Increase buffer size (512 or 1024 samples)
3. Disable oversampling (set oversampling = 0.0)
4. Check if other plugins also glitching (system issue)
```

**Solutions**:

#### Solution 3A: Reduce CPU Load
```
1. Disable SPARK limiter oversampling (sparkOS = 1x)
2. Disable SHINE EQ oversampling (shineAutoOS = OFF)
3. Reduce global oversampling (oversampling = 1x)
4. Disable unused modules (sparkEnabled = OFF, shineEnabled = OFF)
```

**Expected Improvement**: 30-50% CPU reduction

---

#### Solution 3B: Increase Buffer Size
```
DAW Settings:
- Buffer Size: 256 → 512 or 1024 samples
- Sample Rate: Keep at 44.1 kHz or 48 kHz (don't use 96 kHz unless needed)

Trade-off: Higher latency (acceptable for mixing/mastering)
```

---

#### Solution 3C: Check for Memory Leaks
```bash
# macOS
leaks --atExit -- ./BTZ_Standalone

# Linux
valgrind --leak-check=full ./BTZ_Standalone

# Look for: "definitely lost" or "still reachable"
```

**Fix**: If leaks found, report bug to BTZ support

---

## 🟡 COMMON ISSUES (Parameters/GUI)

### Issue 4: Parameters Not Responding

**Symptoms**:
- Knobs move but sound doesn't change
- Automation doesn't affect audio
- Parameter changes delayed by several seconds

**Solutions**:

#### Solution 4A: Check Automation Mode
```
DAW Settings:
- Parameter Automation: Set to "Touch" or "Latch" (not "Read")
- Ensure no automation lane is overriding manual changes
```

---

#### Solution 4B: Verify Parameter Range
```
Some parameters are normalized (0.0 - 1.0):
- punch: 0.0 = no effect, 0.5 = moderate, 1.0 = max
- warmth: 0.0 = clean, 0.5 = moderate, 1.0 = saturated

If parameter at 0.0, no audible change expected!
```

**Fix**: Move parameter to 0.5 or higher for noticeable effect

---

#### Solution 4C: Restart DAW/Plugin
```
1. Save project
2. Close and reopen DAW
3. Reload BTZ
4. Test parameter changes

Possible cause: APVTS state corruption (rare)
```

---

### Issue 5: GUI Not Updating

**Symptoms**:
- Meters frozen
- Knobs don't redraw when automated
- Visual glitches or corrupted rendering

**Solutions**:

#### Solution 5A: Check OpenGL/Graphics Acceleration
```
DAW Settings (if available):
- Disable GPU acceleration for plugins
- Use software rendering instead

macOS: Disable "Metal" rendering
Windows: Disable "Direct2D" rendering
```

---

#### Solution 5B: HiDPI/Scaling Issues
```
Windows:
- Right-click BTZ.vst3 → Properties → Compatibility
- Check "Override high DPI scaling behavior"
- Select "System (Enhanced)"

macOS:
- No fix needed (JUCE handles Retina correctly)

Linux:
- Set environment variable: QT_AUTO_SCREEN_SCALE_FACTOR=0
```

---

## 🟢 WORKFLOW ISSUES (Presets/Projects)

### Issue 6: Preset Won't Load

**Symptoms**:
- Error: "Failed to load preset"
- Preset loads but parameters wrong
- Preset appears blank/empty

**Solutions**:

#### Solution 6A: Verify Preset Format
```xml
<!-- BTZ preset format (XML) -->
<BTZ version="1.0.0">
  <PARAM id="punch" value="0.5"/>
  <!-- ... 26 more parameters ... -->
</BTZ>

Check:
1. File extension: .btz or .xml
2. XML well-formed (no syntax errors)
3. Version field present
```

**Fix**: Re-save preset from working instance

---

#### Solution 6B: Migration from Older Version
```
If preset saved with BTZ 0.x or older:
1. Load preset in BTZ 1.0.0
2. Check console for "Migration Warning"
3. Verify all parameters loaded correctly
4. Re-save preset to update to v1.0.0 format
```

See STATE_VERSIONING.md for migration details

---

### Issue 7: Project Sounds Different After Reopening

**Symptoms**:
- DAW project saved with BTZ sounds different when reopened
- Parameters appear correct but tone changed
- Sonic behavior inconsistent

**Diagnostics**:
```
1. Check BTZ version (project vs current)
2. Check sample rate (project vs current session)
3. Check oversampling settings (may be reset)
4. Verify all parameters match saved values
```

**Solutions**:

#### Solution 7A: Version Mismatch
```
Scenario: Project saved with BTZ 1.0.0, opened in BTZ 2.0.0

Possible cause: Algorithm change in v2.0.0

Fix:
1. Check CHANGELOG.md for breaking changes
2. Use "Legacy Mode" toggle (if available in v2.0.0)
3. Or downgrade to BTZ 1.0.0 for this project
```

---

#### Solution 7B: Sample Rate Change
```
Scenario: Project recorded at 48 kHz, opened at 44.1 kHz

Cause: Some DSP algorithms are sample-rate dependent
(e.g., filter cutoffs, envelope times)

Fix:
1. Set DAW project sample rate to original (48 kHz)
2. Or re-tweak parameters for new sample rate
```

---

## 🔧 DEVELOPER ISSUES (Build/Debug)

### Issue 8: CMake Build Fails

**Symptoms**:
- `cmake -B build` fails with errors
- Missing JUCE modules
- Linker errors

**Solutions**:

#### Solution 8A: JUCE Submodule Not Initialized
```bash
# Initialize JUCE submodule
git submodule update --init --recursive

# Verify JUCE directory exists
ls BTZ_JUCE/JUCE/modules/juce_core/juce_core.h
# Should exist
```

---

#### Solution 8B: CMake Version Too Old
```bash
# Check CMake version
cmake --version
# Requires: 3.15 or newer

# Upgrade CMake:
# macOS: brew upgrade cmake
# Linux: sudo apt-get install cmake
# Windows: Download from cmake.org
```

---

#### Solution 8C: Compiler Not Found
```bash
# Linux - install GCC/Clang
sudo apt-get install build-essential clang

# macOS - install Xcode Command Line Tools
xcode-select --install

# Windows - install Visual Studio 2019/2022
# Or MinGW: choco install mingw
```

---

### Issue 9: Tests Fail to Compile

**Symptoms**:
- `cmake --build build` fails in tests directory
- Missing headers (e.g., "PluginProcessor.h not found")
- Linker errors in test executables

**Solutions**:

#### Solution 9A: Verify Test CMakeLists.txt
```bash
# Check if tests CMakeLists.txt exists
ls BTZ_JUCE/tests/CMakeLists.txt

# Ensure it's included from main CMakeLists.txt
grep "add_subdirectory(tests)" BTZ_JUCE/CMakeLists.txt
```

---

#### Solution 9B: Clean Build Directory
```bash
# Remove build directory
rm -rf BTZ_JUCE/build

# Reconfigure
cmake -B BTZ_JUCE/build -DCMAKE_BUILD_TYPE=Release

# Rebuild
cmake --build BTZ_JUCE/build
```

---

### Issue 10: Sanitizer Errors (ASAN/TSAN)

**Symptoms**:
- ASAN reports "heap-use-after-free"
- TSAN reports "data race"
- Tests crash with sanitizer enabled

**Solutions**:

#### Solution 10A: ASAN Heap Errors
```
Common causes:
1. Buffer overrun in DSP processing
2. Dangling pointer in prepareToPlay
3. Use-after-free in releaseResources

Debug:
1. Run with ASAN: cmake --preset asan && cmake --build build-asan
2. Check stack trace for exact line
3. Use debugger: lldb ./build-asan/BTZ_Standalone
```

See STATIC_ANALYSIS_GUIDE.md for full ASAN workflow

---

#### Solution 10B: TSAN Data Races
```
Common causes:
1. Non-atomic parameter reads in processBlock
2. Shared state modified without mutex
3. GUI thread writing to audio thread buffer

Fix:
1. Use std::atomic for shared variables
2. Use lock-free FIFO for cross-thread communication
3. Never share mutable state between GUI and audio threads
```

See RT_SAFETY_MANIFEST.md for RT-safe patterns

---

## 📊 DIAGNOSTIC COMMANDS

### System Info Collection

```bash
# macOS
system_profiler SPSoftwareDataType SPHardwareDataType > system_info.txt

# Linux
uname -a > system_info.txt
lscpu >> system_info.txt
lsb_release -a >> system_info.txt

# Windows
systeminfo > system_info.txt
```

### BTZ Build Info

```bash
# Check BTZ version
strings BTZ.vst3 | grep "BTZ v"
# Should show: "BTZ v1.0.0"

# Check JUCE version
strings BTZ.vst3 | grep "JUCE v"
# Should show: "JUCE v7.0.12"

# Check build date
stat BTZ.vst3
# Shows last modified date (build date)
```

### DAW Plugin Scan Log

```bash
# Reaper
tail -f ~/Library/Application\ Support/REAPER/reaper-vstplugins.ini

# Logic Pro X
tail -f ~/Library/Preferences/com.apple.logic.pro.plist

# Ableton Live
tail -f ~/Library/Preferences/Ableton/Live\ 11.x.x/Log.txt
```

---

## 🆘 SUPPORT & BUG REPORTING

### Before Reporting a Bug

1. ✅ Check this troubleshooting guide
2. ✅ Search existing GitHub issues
3. ✅ Verify BTZ version is latest
4. ✅ Test with factory default preset
5. ✅ Test in different DAW (if possible)

### Bug Report Template

```markdown
**BTZ Version**: 1.0.0
**Platform**: macOS 13.4 (M1 Pro)
**DAW**: Logic Pro X 10.7.5
**Sample Rate**: 48000 Hz
**Buffer Size**: 256 samples

**Issue**: Plugin crashes when enabling SPARK limiter

**Steps to Reproduce**:
1. Load BTZ on audio track
2. Play audio
3. Toggle sparkEnabled parameter to ON
4. Observe crash

**Expected Behavior**: SPARK limiter engages without crash

**Actual Behavior**: DAW crashes with error: "Signal 11 (SIGSEGV)"

**Logs**:
(Attach crash log or console output)

**Additional Info**:
- Only happens with oversampling enabled (sparkOS > 1x)
- Does not crash with sparkOS = 1x
```

Submit to: https://github.com/btz-audio/btz/issues

---

## 📚 REFERENCES

- **Architecture Documentation**: `docs/ARCHITECTURE.md`
- **Parameter Reference**: `docs/PARAMETER_MANIFEST.md`
- **RT-Safety Guide**: `docs/RT_SAFETY_MANIFEST.md`
- **Static Analysis**: `docs/STATIC_ANALYSIS_GUIDE.md`
- **Test Suite**: `docs/TEST_SUITE.md`
- **State Versioning**: `docs/STATE_VERSIONING.md`

---

## 🔄 KNOWN ISSUES (1.0.0)

### Known Limitations

1. **Oversampling Change Latency**: Changing oversampling setting may cause brief audio glitch (deferred to async thread)
2. **HiDPI Scaling (Windows)**: Some UI elements may appear blurry on 4K displays (JUCE limitation)
3. **M1 Mac Rosetta**: Intel build runs via Rosetta, native ARM build recommended

### Planned Fixes (1.1.0)

- [ ] Seamless oversampling change (crossfade implementation)
- [ ] Improved HiDPI rendering (custom scaling)
- [ ] Native ARM build for macOS

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Support Team

**Bottom Line**: Most issues can be resolved by verifying binary architecture, checking parameter values, and adjusting buffer size/oversampling. For unsolved issues, report to GitHub with full diagnostic info.
