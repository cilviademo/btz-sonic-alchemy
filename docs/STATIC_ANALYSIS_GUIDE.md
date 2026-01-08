# BTZ Static Analysis Guide

**Version**: 1.0.0
**Purpose**: Instructions for running static analysis and sanitizers
**Last Updated**: 2026-01-08

---

## 🎯 OVERVIEW

Static analysis tools catch bugs **without running the code**. This guide covers:
- Compiler warnings
- clang-tidy (static analysis)
- Sanitizers (ASAN, UBSAN, TSAN, MSAN)
- Valgrind
- Code coverage

**Principle**: Fix issues found by static tools BEFORE shipping.

---

## ⚙️ PREREQUISITES

### Required Tools
```bash
# macOS (Homebrew)
brew install llvm clang-tidy cmake

# Linux (Ubuntu/Debian)
sudo apt-get install clang-tidy clang cmake valgrind lcov

# Verify installation
clang-tidy --version
cmake --version
```

### Generate Compile Commands
All static analysis tools need `compile_commands.json`:
```bash
cd BTZ_JUCE
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

---

## 🔍 1. COMPILER WARNINGS

### Enable Maximum Warnings
BTZ enforces strict warning policy. See `docs/WARNING_POLICY.md`.

### Build with Warnings
```bash
# Release build with all warnings
cd BTZ_JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build 2>&1 | tee build.log

# Count warnings
grep -c "warning:" build.log
```

### Target
- **BTZ Code**: 0-5 warnings
- **Total**: <10 warnings

### Common Warnings
See `docs/WARNING_POLICY.md` for remediation strategies:
- Unused parameters
- Sign conversions
- Variable shadowing
- Float equality
- Missing enum cases

---

## 🧹 2. CLANG-TIDY (STATIC ANALYSIS)

### Configuration
BTZ uses JUCE-safe clang-tidy config: `.clang-tidy` (root directory)

### Run on Single File
```bash
clang-tidy -p build BTZ_JUCE/Source/PluginProcessor.cpp
```

### Run on All Source Files
```bash
# Find all .cpp files and run clang-tidy
find BTZ_JUCE/Source -name "*.cpp" -type f | \
  xargs -I {} clang-tidy -p build {}
```

### Auto-Fix Issues (Use with Caution)
```bash
# Dry run (show fixes)
clang-tidy -p build --fix-errors BTZ_JUCE/Source/PluginProcessor.cpp

# Apply fixes
clang-tidy -p build --fix BTZ_JUCE/Source/PluginProcessor.cpp

# IMPORTANT: Review changes with git diff before committing!
```

### CI Integration
Add to `.github/workflows/build-and-test.yml`:
```yaml
- name: Run clang-tidy
  run: |
    find BTZ_JUCE/Source -name "*.cpp" | \
      xargs clang-tidy -p build --warnings-as-errors='*'
```

### Common Issues Detected
- **bugprone**: Use-after-free, null derefs, logic errors
- **performance**: Unnecessary copies, inefficient patterns
- **readability**: Complex functions, naming violations
- **modernize**: Use modern C++ features (auto, range-for, etc.)
- **cert**: Security vulnerabilities

---

## 🧪 3. SANITIZERS (RUNTIME VALIDATION)

### Overview
Sanitizers instrument code to detect errors at runtime:
- **ASAN**: Memory errors (leaks, use-after-free, buffer overflow)
- **UBSAN**: Undefined behavior (null deref, signed overflow)
- **TSAN**: Data races and deadlocks
- **MSAN**: Uninitialized memory reads (Linux only)

### Using CMake Presets

#### AddressSanitizer (ASAN)
```bash
# Configure
cmake --preset asan

# Build
cmake --build build-asan

# Run tests
cd build-asan
./BTZ_artefacts/Standalone/BTZ

# Check for errors in output:
# "ERROR: AddressSanitizer" = memory error detected
```

#### UndefinedBehaviorSanitizer (UBSAN)
```bash
# Configure
cmake --preset ubsan

# Build
cmake --build build-ubsan

# Run
cd build-ubsan
./BTZ_artefacts/Standalone/BTZ

# Check for errors:
# "runtime error:" = UB detected
```

#### ThreadSanitizer (TSAN)
```bash
# Configure
cmake --preset tsan

# Build
cmake --build build-tsan

# Run with DAW or stress test
cd build-tsan
./BTZ_artefacts/Standalone/BTZ

# Check for errors:
# "WARNING: ThreadSanitizer:" = data race detected
```

#### MemorySanitizer (MSAN - Linux Only)
```bash
# Requires special libc++ build (advanced)
# See: https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo

cmake --preset msan
cmake --build build-msan
cd build-msan
./BTZ_artefacts/Standalone/BTZ
```

### Environment Variables
```bash
# ASAN options
export ASAN_OPTIONS=detect_leaks=1:strict_string_checks=1

# UBSAN options
export UBSAN_OPTIONS=print_stacktrace=1

# TSAN options
export TSAN_OPTIONS=second_deadlock_stack=1

# Run with options
./BTZ_artefacts/Standalone/BTZ
```

### Interpreting Results

**No errors** = clean output
```
No issues detected by AddressSanitizer
```

**Leak detected**:
```
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 64 byte(s) in 1 object(s) allocated from:
    #0 0x... in operator new
    #1 0x... in BTZAudioProcessor::prepareToPlay
```
→ Fix allocation in prepareToPlay

**Data race**:
```
WARNING: ThreadSanitizer: data race

Write of size 4 at 0x7b0400000000 by thread T2:
    #0 BTZAudioProcessor::setParameter
```
→ Add mutex or atomic variable

---

## 🔬 4. VALGRIND (MEMORY ANALYSIS)

### Linux Only
```bash
# Build with debug symbols
cmake --preset valgrind
cmake --build build-valgrind

# Run Valgrind
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind.log \
         build-valgrind/BTZ_artefacts/Standalone/BTZ

# Check results
cat valgrind.log | grep "ERROR SUMMARY"
# "0 errors from 0 contexts" = clean
```

### Common Issues
- **Memory leaks**: "definitely lost", "possibly lost"
- **Invalid reads/writes**: Buffer overflows
- **Uninitialized values**: Using uninitialized memory

---

## 📊 5. CODE COVERAGE

### Generate Coverage Report
```bash
# Build with coverage instrumentation
cmake --preset coverage
cmake --build build-coverage

# Run tests (generates .gcda files)
cd build-coverage
ctest --verbose

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/JUCE/*' --output-file coverage_filtered.info

# Generate HTML report
genhtml coverage_filtered.info --output-directory coverage_html

# View report
open coverage_html/index.html  # macOS
xdg-open coverage_html/index.html  # Linux
```

### Target Coverage
- **BTZ Core DSP**: >80%
- **State Management**: >90%
- **UI Code**: >50% (harder to test)

---

## 🛡️ 6. REAL-TIME SAFETY CHECKS

### Custom RT Safety Build
```bash
# Enable RT safety assertions
cmake --preset rt-checks
cmake --build build-rt-checks

# Run with RT checks enabled
cd build-rt-checks
./BTZ_artefacts/Standalone/BTZ
```

### RT Safety Macros
In code (when `BTZ_RT_CHECKS=1`):
```cpp
#if BTZ_RT_CHECKS
    // Detect allocations in audio thread
    #define BTZ_RT_ONLY() jassert(!juce::Thread::isThreadRunning())
#else
    #define BTZ_RT_ONLY()
#endif

void processBlock(...) {
    BTZ_RT_ONLY();  // Assert not in audio thread during dev
    // ... process audio
}
```

---

## 🔄 7. CONTINUOUS INTEGRATION

### GitHub Actions Integration
Add to `.github/workflows/static-analysis.yml`:
```yaml
name: Static Analysis

on: [push, pull_request]

jobs:
  clang-tidy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install tools
        run: sudo apt-get install clang-tidy cmake
      - name: Configure
        run: cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      - name: Run clang-tidy
        run: |
          find BTZ_JUCE/Source -name "*.cpp" | \
            xargs clang-tidy -p build --warnings-as-errors='*'

  sanitizers:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        sanitizer: [asan, ubsan]
    steps:
      - uses: actions/checkout@v4
      - name: Configure
        run: cmake --preset ${{ matrix.sanitizer }}
      - name: Build
        run: cmake --build build-${{ matrix.sanitizer }}
      - name: Run tests
        run: |
          cd build-${{ matrix.sanitizer }}
          ctest --verbose
```

---

## 📋 CHECKLIST (Pre-Release)

### Static Analysis Pass Criteria
- [ ] **Warnings**: <10 total (see WARNING_POLICY.md)
- [ ] **clang-tidy**: 0 errors in BTZ code
- [ ] **ASAN**: No memory leaks or errors
- [ ] **UBSAN**: No undefined behavior
- [ ] **TSAN**: No data races (if multithreaded)
- [ ] **Valgrind**: Clean run (0 errors)
- [ ] **Coverage**: >70% line coverage on core modules

### Ship Gate Integration
These checks contribute to:
- **Gate 1** (pluginval): Stability validation
- **Gate 7** (automation): No race conditions
- **Gate 8** (bypass): No memory errors
- **Gate 9** (migration): State integrity

---

## 🚨 TROUBLESHOOTING

### "compile_commands.json not found"
```bash
# Regenerate
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### "Sanitizer not available"
```bash
# Check compiler support
clang++ --version  # Must support -fsanitize
gcc --version      # GCC 4.8+ required
```

### "JUCE code triggers warnings"
- This is normal; we only fix warnings in BTZ code
- Use `.clang-tidy` HeaderFilterRegex to exclude JUCE

### "False positives"
- Add suppressions to `.clang-tidy`
- Document why suppression is justified

---

## 📚 REFERENCES

- [Clang-Tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
- [LLVM Code Coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)

---

## 📊 METRICS TRACKING

### Baseline (2026-01-08)
| Tool | Status | Issues |
|------|--------|--------|
| Warnings | ⚠️ | 62 |
| clang-tidy | ❌ | Not run |
| ASAN | ❌ | Not run |
| UBSAN | ❌ | Not run |
| TSAN | ❌ | Not run |
| Coverage | ❌ | 0% |

### Target (Ship Gate)
| Tool | Status | Issues |
|------|--------|--------|
| Warnings | ✅ | <10 |
| clang-tidy | ✅ | 0 errors |
| ASAN | ✅ | 0 errors |
| UBSAN | ✅ | 0 errors |
| TSAN | ✅ | 0 races |
| Coverage | ✅ | >70% |

---

**Bottom Line**: Static analysis catches bugs before they reach users. Run these tools regularly during development and enforce clean results before release.

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ QA Team
