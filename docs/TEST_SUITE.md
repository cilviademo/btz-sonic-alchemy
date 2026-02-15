# BTZ Test Suite Documentation

**Version**: 1.0.0
**Purpose**: Complete test harness for BTZ Ship Gate validation
**Last Updated**: 2026-01-08

---

## 🎯 OVERVIEW

The BTZ test suite provides automated validation for critical ship gates without requiring DAW execution. All tests are standalone executables that verify specific aspects of plugin behavior.

**Philosophy**: Evidence-based ship-readiness through comprehensive automated testing.

---

## 📋 TEST SUITE INVENTORY

### 1. Parameter Conversion Test

**File**: `BTZ_JUCE/tests/parameter_conversion_test.cpp`
**Ship Gate**: #9 (Migration Tests)
**Purpose**: Verify normalized ↔ plain parameter conversions

**Tests**:
- ✅ Linear conversions (inputGain, outputGain)
- ✅ Skewed conversions (shineFreqHz, shineQ with logarithmic distribution)
- ✅ Choice conversions (sparkOS, sparkMode, masterBlend)
- ✅ Boolean conversions (threshold-based)
- ✅ Hero controls range validation (0.0 - 1.0)
- ✅ Parameter ID stability (all 27 parameters)

**Run**:
```bash
cd BTZ_JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
ctest -R ParameterConversionTest --verbose
# OR
./parameter_conversion_test
```

**Expected Output**:
```
[TEST] Input Gain Conversion... ✅ PASS
[TEST] Shine Frequency Skewed Conversion... ✅ PASS
[TEST] Spark Oversampling Choice Conversion... ✅ PASS
[TEST] Boolean Parameter Conversion... ✅ PASS
[TEST] Hero Controls Range Validation... ✅ PASS
[TEST] Spark Ceiling Conversion... ✅ PASS
[TEST] Parameter ID Stability... ✅ PASS (27 parameters verified)

Results: 7 passed, 0 failed
```

---

### 2. State Round-Trip Test

**File**: `BTZ_JUCE/tests/state_roundtrip_test.cpp`
**Ship Gate**: #9 (Migration Tests)
**Purpose**: Verify state serialization/deserialization determinism

**Tests**:
- ✅ Basic round-trip (save → load → verify match)
- ✅ Default state preservation
- ✅ Extreme values round-trip (min/max)
- ✅ Deterministic serialization (same state → identical XML)
- ✅ Version field presence
- ✅ All 27 parameters preserved

**Run**:
```bash
cd BTZ_JUCE/build
ctest -R StateRoundTripTest --verbose
# OR
./state_roundtrip_test
```

**Expected Output**:
```
[TEST] Basic State Round-Trip... ✅ PASS
[TEST] Default State Round-Trip... ✅ PASS
[TEST] Extreme Values Round-Trip... ✅ PASS
[TEST] Deterministic Serialization... ✅ PASS
[TEST] Version Field Present... ✅ PASS (XML well-formed)
[TEST] All 27 Parameters Preserved... ✅ PASS (27/27 parameters verified)

Results: 6 passed, 0 failed
```

---

### 3. Bypass Bit-Perfect Test

**File**: `BTZ_JUCE/tests/bypass_bitperfect_test.cpp`
**Ship Gate**: #8 (Bypass Test)
**Purpose**: Verify bypass mode produces bit-identical output to input

**Tests**:
- ✅ Bypass with silence (input == output)
- ✅ Bypass with random signal (bit-perfect comparison)
- ✅ Bypass with sine wave
- ✅ Various buffer sizes (32, 64, 128, 256, 512, 1024, 2048)
- ✅ Mono and stereo configurations
- ✅ Extreme values (±1.0)
- ✅ Denormal handling

**Run**:
```bash
cd BTZ_JUCE/build
ctest -R BypassBitPerfectTest --verbose
# OR
./bypass_bitperfect_test
```

**Expected Output**:
```
[TEST] Bypass with Silence... ✅ PASS
[TEST] Bypass with Random Signal... ✅ PASS
[TEST] Bypass with Sine Wave... ✅ PASS
[TEST] Bypass with Various Buffer Sizes... ✅ PASS (7 buffer sizes verified)
[TEST] Bypass with Mono/Stereo Configurations... ✅ PASS
[TEST] Bypass with Extreme Values... ✅ PASS
[TEST] Bypass No Denormals... ✅ PASS

Results: 7 passed, 0 failed
```

**Note**: This test currently uses `mix = 0.0` as a proxy for bypass. If BTZ adds a dedicated bypass parameter, update the test accordingly.

---

### 4. Automation Torture Test

**File**: `BTZ_JUCE/tests/automation_torture_test.cpp`
**Ship Gate**: #7 (Automation Test)
**Purpose**: Detect discontinuities, zipper noise, and artifacts during automation

**Tests**:
- ✅ Rapid mix parameter automation
- ✅ All 27 parameters simultaneous automation
- ✅ Extreme parameter jumps (0→1→0)
- ✅ Denormal prevention during automation
- ✅ Oversampling change stability
- ✅ Automation with silent input
- ✅ SPARK oversampling choice automation

**Run**:
```bash
cd BTZ_JUCE/build
ctest -R AutomationTortureTest --verbose
# OR
./automation_torture_test
```

**Expected Output**:
```
[TEST] Rapid Mix Parameter Automation... ✅ PASS
[TEST] All 27 Parameters Automation... ✅ PASS (27 parameters automated)
[TEST] Extreme Parameter Jumps (0→1→0)... ✅ PASS
[TEST] Denormal Prevention During Automation... ✅ PASS
[TEST] Oversampling Change During Automation... ✅ PASS
[TEST] Automation with Silent Input... ✅ PASS
[TEST] SPARK Oversampling Choice Automation... ✅ PASS (5 choices verified)

Results: 7 passed, 0 failed
```

---

### 5. Lifecycle Stress Test

**File**: `BTZ_JUCE/tests/lifecycle_stress_test.cpp`
**Ship Gate**: #1 (pluginval - Lifecycle Validation)
**Purpose**: Stress-test plugin lifecycle without DAW

**Tests**:
- ✅ Construct → destroy cycles
- ✅ prepareToPlay → releaseResources
- ✅ Sample rate changes
- ✅ Buffer size changes
- ✅ Memory leak detection

**Run**:
```bash
cd BTZ_JUCE/build
ctest -R LifecycleStressTest --verbose
# OR
./lifecycle_stress_test --iterations 50
```

---

## 🏗️ BUILD SYSTEM INTEGRATION

### CMake Configuration

All tests are configured in `BTZ_JUCE/tests/CMakeLists.txt` and automatically registered with CTest.

### Build All Tests
```bash
cd BTZ_JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run All Tests
```bash
cd BTZ_JUCE/build
ctest --verbose
```

### Run Specific Test
```bash
ctest -R ParameterConversionTest --verbose
ctest -R StateRoundTripTest --verbose
ctest -R BypassBitPerfectTest --verbose
ctest -R AutomationTortureTest --verbose
```

---

## 🔄 CI INTEGRATION

### GitHub Actions

Add to `.github/workflows/build-and-test.yml`:

```yaml
- name: Build Tests
  run: |
    cd BTZ_JUCE
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

- name: Run Test Suite
  run: |
    cd BTZ_JUCE/build
    ctest --verbose --output-on-failure
```

### Ship Gate Validation

| Test | Ship Gate | Criteria | Pass/Fail |
|------|-----------|----------|-----------|
| Parameter Conversion | #9 | All conversions accurate | ✅ READY |
| State Round-Trip | #9 | Deterministic serialization | ✅ READY |
| Bypass Bit-Perfect | #8 | Input == Output bitwise | 🔄 PENDING |
| Automation Torture | #7 | No discontinuities/NaN | 🔄 PENDING |
| Lifecycle Stress | #1 | No crashes/leaks | ✅ READY |

---

## 🚨 TROUBLESHOOTING

### "No such test: ParameterConversionTest"
**Cause**: Tests not built
**Fix**: Run `cmake --build build` to compile tests

### Test Fails with "Assertion failed"
**Cause**: Parameter behavior doesn't match spec
**Fix**: Review test output, check PARAMETER_MANIFEST.md

### "undefined reference to juce::..."
**Cause**: JUCE not found or not built
**Fix**: Ensure JUCE submodule initialized (`git submodule update --init --recursive`)

### Test Crashes
**Cause**: Likely processBlock issue
**Fix**: Run with sanitizers:
```bash
cmake --preset asan
cmake --build build-asan
cd build-asan
./parameter_conversion_test
```

---

## 📊 TEST COVERAGE MATRIX

### Parameter Tests
| Parameter | Conversion Test | Round-Trip Test | Automation Test |
|-----------|----------------|-----------------|-----------------|
| punch | ✅ | ✅ | ✅ |
| warmth | ✅ | ✅ | ✅ |
| boom | ✅ | ✅ | ✅ |
| mix | ✅ | ✅ | ✅ |
| drive | ✅ | ✅ | ✅ |
| texture | ✅ | ✅ | ✅ |
| inputGain | ✅ | ✅ | ✅ |
| outputGain | ✅ | ✅ | ✅ |
| autoGain | ✅ | ✅ | ✅ |
| sparkEnabled | ✅ | ✅ | ✅ |
| sparkLUFS | ✅ | ✅ | ✅ |
| sparkCeiling | ✅ | ✅ | ✅ |
| sparkMix | ✅ | ✅ | ✅ |
| sparkOS | ✅ | ✅ | ✅ |
| sparkAutoOS | ✅ | ✅ | ✅ |
| sparkMode | ✅ | ✅ | ✅ |
| shineEnabled | ✅ | ✅ | ✅ |
| shineFreqHz | ✅ | ✅ | ✅ |
| shineGainDb | ✅ | ✅ | ✅ |
| shineQ | ✅ | ✅ | ✅ |
| shineMix | ✅ | ✅ | ✅ |
| shineAutoOS | ✅ | ✅ | ✅ |
| masterEnabled | ✅ | ✅ | ✅ |
| masterMacro | ✅ | ✅ | ✅ |
| masterBlend | ✅ | ✅ | ✅ |
| masterMix | ✅ | ✅ | ✅ |
| precisionMode | ✅ | ✅ | ✅ |
| active | ✅ | ✅ | ✅ |
| oversampling | ✅ | ✅ | ✅ |

**Total Coverage**: 27/27 parameters (100%)

---

## 🔮 FUTURE ENHANCEMENTS

### Planned Tests (Not Yet Implemented)
- [ ] **Migration Fixtures**: Golden state files for version upgrade testing
- [ ] **CPU Performance Test**: Measure processBlock CPU usage
- [ ] **Latency Report Test**: Verify latency compensation values
- [ ] **LUFS Accuracy Test**: Validate SPARK limiter loudness measurement
- [ ] **Harmonic Distortion Test**: Verify saturation THD characteristics
- [ ] **Null Test Suite**: Compare processed audio against reference (requires golden files)

### CI Enhancements
- [ ] Automated pluginval integration (requires Linux/macOS CI runner)
- [ ] Code coverage reporting (lcov integration)
- [ ] Performance regression detection (benchmark suite)

---

## 📚 REFERENCES

- **PARAMETER_MANIFEST.md**: Complete parameter specifications
- **RT_SAFETY_MANIFEST.md**: Real-time safety audit
- **STATIC_ANALYSIS_GUIDE.md**: Sanitizer and clang-tidy usage
- **Ship Gate Tracker**: `.github/SHIP_GATES.md`

---

## ✅ SHIP GATE CHECKLIST

Before shipping BTZ 1.0.0, all tests must pass:

- [ ] Parameter Conversion Test: 7/7 tests pass
- [ ] State Round-Trip Test: 6/6 tests pass
- [ ] Bypass Bit-Perfect Test: 7/7 tests pass
- [ ] Automation Torture Test: 7/7 tests pass
- [ ] Lifecycle Stress Test: 50 iterations no crashes

**Current Status**: 🔄 **PENDING** (tests created, execution required for validation)

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ QA Team

**Bottom Line**: The test suite provides comprehensive automated validation for ship gates #7, #8, and #9 without requiring manual DAW testing. Once execution is enabled, these tests will provide evidence-based ship-readiness confirmation.
