# BTZ Compiler Warning Policy

**Version**: 1.0.0
**Purpose**: Define warning standards, budget, and remediation strategy
**Last Updated**: 2026-01-08

---

## 🎯 WARNING PHILOSOPHY

**Goal**: Maintain **zero** avoidable warnings in BTZ-owned code while respecting JUCE framework patterns.

**Principle**: Warnings are potential bugs. Each warning represents:
- Unclear intent
- Possible runtime error
- Maintenance burden
- Code smell

**Exception**: Warnings in third-party code (JUCE, dependencies) are acceptable if unavoidable and documented.

---

## 📊 CURRENT WARNING INVENTORY

**Last Build**: 2026-01-08 (from BTZ_COMPREHENSIVE_TEST_RESULTS.md)

| Category | Count | Severity | Action Required |
|----------|-------|----------|----------------|
| Unused parameters | 15 | Low | Fix (JUCE interface compliance) |
| Sign conversions | 12 | Medium | Fix (potential logic bugs) |
| Variable shadowing | 8 | Medium | Fix (clarity issue) |
| Float equality | 6 | Low-Medium | Review (some intentional) |
| Switch enum | 1 | Low | Fix |
| Overloaded virtual | 1 | Low | Fix |
| **TOTAL** | **62** | - | Target: <10 |

---

## ⚖️ WARNING BUDGET & GATES

### Warning Budget
| Code Ownership | Max Warnings | Current | Status |
|----------------|--------------|---------|--------|
| **BTZ Core** (Source/) | 0 | ~50 | ❌ OVER |
| **BTZ Tests** (tests/) | 5 | TBD | ✅ |
| **BTZ Tools** (tools/) | 5 | TBD | ✅ |
| **JUCE Framework** | Unlimited | ~12 | ✅ (not our code) |
| **Third-Party** | Document | 0 | ✅ |

### CI Gate
- **Ship Gate**: <10 total warnings from BTZ code
- **CI Enforcement**: Fail build if warning count **increases** from baseline
- **Review Required**: Any new warning must be justified in PR

---

## 🔧 REMEDIATION STRATEGY

### 1. Unused Parameters (15 warnings)

**Root Cause**: JUCE interface methods require specific signatures even if parameters unused.

**Fix Strategy**:
```cpp
// BAD (generates warning)
void prepareToPlay(double sampleRate, int samplesPerBlock) override {
    // sampleRate not used
}

// GOOD (explicit intent)
void prepareToPlay(double sampleRate, int samplesPerBlock) override {
    juce::ignoreUnused(sampleRate);  // Document why
    // OR
    (void)sampleRate;  // C-style suppression
}

// BEST (use it if possible)
void prepareToPlay(double sampleRate, int samplesPerBlock) override {
    currentSampleRate_ = sampleRate;  // Actually use it
}
```

**Action Items**:
- [ ] Audit all unused parameter warnings
- [ ] Add `juce::ignoreUnused()` for JUCE interface compliance
- [ ] Use parameters if possible (improve code quality)
- [ ] Document WHY parameter is unused (interface requirement, future use, etc.)

**Risk**: LOW - Safe to fix

---

### 2. Sign Conversions (12 warnings)

**Root Cause**: Mixing `int` and `unsigned` types, especially with JUCE types like `uint32`, `size_t`.

**Fix Strategy**:
```cpp
// BAD
uint32 value = -1;  // Implicit conversion, sign mismatch
for (int i = 0; i < array.size(); ++i)  // size() returns size_t (unsigned)

// GOOD
uint32 value = static_cast<uint32>(-1);  // Explicit, intentional
for (size_t i = 0; i < array.size(); ++i)  // Correct type
// OR
for (auto i = 0u; i < array.size(); ++i)  // Auto-deduce unsigned
```

**Action Items**:
- [ ] Audit all sign conversion warnings
- [ ] Use `static_cast<>` for intentional conversions
- [ ] Use correct types (`size_t` for indices, `int` for ranges)
- [ ] Prefer range-based `for` loops where possible

**Risk**: MEDIUM - Can hide logic bugs (off-by-one, overflow)

---

### 3. Variable Shadowing (8 warnings)

**Root Cause**: Local variable or parameter has same name as member variable.

**Fix Strategy**:
```cpp
class Processor {
    double sampleRate_;  // Member variable

    // BAD (shadows member)
    void prepare(double sampleRate) {
        sampleRate = sampleRate;  // WRONG: assigns to parameter, not member
    }

    // GOOD (distinct names)
    void prepare(double newSampleRate) {
        sampleRate_ = newSampleRate;  // Clear intent
    }

    // OR (use this-> for clarity)
    void prepare(double sampleRate) {
        this->sampleRate_ = sampleRate;  // Explicit
    }
};
```

**Action Items**:
- [ ] Audit all shadowing warnings
- [ ] Rename parameters to avoid shadowing (prefer `newValue`, `inputValue`, etc.)
- [ ] Use trailing underscore for member variables (`sampleRate_`)
- [ ] Enable `-Wshadow` to catch future cases

**Risk**: MEDIUM - Can cause subtle bugs (wrong variable modified)

---

### 4. Float Equality (6 warnings)

**Root Cause**: Using `==` or `!=` on floating-point values.

**Fix Strategy**:
```cpp
// BAD (exact equality)
if (value == 0.0f)  // May fail due to precision
if (value == targetValue)  // Unreliable

// GOOD (tolerance-based)
constexpr float epsilon = 1e-6f;
if (std::abs(value) < epsilon)  // Near zero
if (std::abs(value - targetValue) < epsilon)  // Approximately equal

// BEST (use juce utility if available)
if (juce::approximatelyEqual(value, targetValue))
```

**Intentional Exceptions**:
- Checking for **exact** special values: `if (value == 0.0f)` in bypass paths
- Boolean-like flags stored as float: `if (enabled == 1.0f)`

**Action Items**:
- [ ] Audit all float equality warnings
- [ ] Replace with epsilon-based comparison where appropriate
- [ ] Document intentional exact comparisons (e.g., bypass check)
- [ ] Add `BTZ_FLOAT_EQUAL()` macro for clarity

**Risk**: LOW-MEDIUM - Can cause incorrect behavior in edge cases

---

### 5. Switch Enum (1 warning)

**Root Cause**: Switch statement doesn't handle all enum values.

**Fix Strategy**:
```cpp
enum class Mode { Clean, Warm, Aggressive, Unknown };

// BAD (missing case)
switch (mode) {
    case Mode::Clean: return 0.0f;
    case Mode::Warm: return 0.5f;
    case Mode::Aggressive: return 1.0f;
    // Missing: Mode::Unknown
}

// GOOD (handle all cases)
switch (mode) {
    case Mode::Clean: return 0.0f;
    case Mode::Warm: return 0.5f;
    case Mode::Aggressive: return 1.0f;
    case Mode::Unknown:
    default:
        jassertfalse;  // Should never happen
        return 0.0f;
}
```

**Action Items**:
- [ ] Add missing enum cases
- [ ] Add `default:` clause with assertion for unexpected values
- [ ] Enable `-Wswitch-enum` to catch future cases

**Risk**: MEDIUM - Can cause undefined behavior

---

### 6. Overloaded Virtual (1 warning)

**Root Cause**: Derived class method hides base class overloads.

**Fix Strategy**:
```cpp
class Base {
    virtual void process(float* buffer, int size);
    virtual void process(double* buffer, int size);
};

// BAD (hides Base::process(double*))
class Derived : public Base {
    void process(float* buffer, int size) override;
    // Missing: process(double*)
};

// GOOD (bring base overloads into scope)
class Derived : public Base {
    using Base::process;  // Expose base overloads
    void process(float* buffer, int size) override;
};

// OR (override all)
class Derived : public Base {
    void process(float* buffer, int size) override;
    void process(double* buffer, int size) override;
};
```

**Action Items**:
- [ ] Add `using Base::methodName;` to expose base overloads
- [ ] OR override all overloads explicitly
- [ ] Document why some overloads are hidden if intentional

**Risk**: MEDIUM - Can cause unexpected dispatch behavior

---

## 🚨 WARNING FLAGS CONFIGURATION

### Baseline Flags (Already Enabled)
```cmake
# From JUCE recommended flags
-Wall
-Wextra
-Wpedantic
```

### Additional Flags (Proposed for BTZ Code Only)
```cmake
# BTZ-specific warning additions
target_compile_options(BTZ PRIVATE
    $<$<CONFIG:Debug>:
        -Wshadow                 # Catch variable shadowing
        -Wconversion             # Catch implicit conversions
        -Wsign-conversion        # Catch sign mismatches
        -Wfloat-equal            # Catch float equality
        -Wswitch-enum            # Catch missing enum cases
        -Woverloaded-virtual     # Catch hidden overloads
        -Wunused                 # Catch unused variables/params
        -Wno-unused-parameter    # Temporarily allow (JUCE interfaces)
    >
)

# IMPORTANT: Do NOT apply to JUCE or third-party
# Use PRIVATE scope, not PUBLIC
```

### Suppression (When Necessary)
```cpp
// Use sparingly and document
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void juceMethod(int unusedParam) override {
    // JUCE interface requirement, param genuinely unused
}
#pragma GCC diagnostic pop
```

---

## 📈 CI INTEGRATION

### CMake Warning Budget Check
Add to `BTZ_JUCE/CMakeLists.txt`:
```cmake
# Warning budget enforcement
set(BTZ_WARNING_BUDGET 10)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Count warnings during build
    # (Implementation requires custom target or script)
endif()
```

### CI Workflow Addition
Add to `.github/workflows/build-and-test.yml`:
```yaml
- name: Check warning count
  run: |
    # Extract warning count from build log
    WARNING_COUNT=$(grep -c "warning:" build.log || echo 0)
    echo "Current warnings: $WARNING_COUNT"
    if [ $WARNING_COUNT -gt 10 ]; then
      echo "❌ Warning budget exceeded: $WARNING_COUNT > 10"
      exit 1
    fi
```

---

## 📋 REMEDIATION CHECKLIST

### Phase 1: Quick Wins (Safe, Low-Risk)
- [ ] Add `juce::ignoreUnused()` for all unused JUCE interface parameters
- [ ] Fix variable shadowing (rename parameters)
- [ ] Add missing switch enum cases
- [ ] Fix overloaded virtual warnings

**Target**: Reduce from 62 → 25 warnings

### Phase 2: Medium-Risk Fixes
- [ ] Audit sign conversion warnings
- [ ] Add explicit `static_cast<>` where intentional
- [ ] Fix incorrect type usage (int vs size_t)

**Target**: Reduce from 25 → 10 warnings

### Phase 3: Float Equality Review
- [ ] Audit each float equality warning
- [ ] Replace with epsilon comparison where appropriate
- [ ] Document intentional exact comparisons
- [ ] Add `BTZ_FLOAT_EQUAL()` macro

**Target**: Reduce from 10 → <5 warnings

### Phase 4: Suppression (Last Resort)
- [ ] For remaining warnings, add pragma suppression with justification
- [ ] Document each suppression in code comment

**Target**: 0 visible warnings (all justified)

---

## 🔍 ENFORCEMENT

### Code Review Checklist
- [ ] No new warnings introduced
- [ ] Warning fixes don't change behavior (test coverage)
- [ ] Suppressions are justified and documented
- [ ] Follow-up issues created for intentional suppressions

### Pre-Release Gate
- **Ship Gate**: <10 warnings from BTZ code
- **Evidence**: Build log with warning count
- **Exception Process**: CTO approval required for >10 warnings

---

## 📚 REFERENCES

- [GCC Warning Options](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)
- [Clang Diagnostic Flags](https://clang.llvm.org/docs/DiagnosticsReference.html)
- [JUCE Coding Standards](https://juce.com/discover/stories/coding-standards)
- "Effective C++" by Scott Meyers (warning-related patterns)

---

## 📊 TRACKING

### Baseline (2026-01-08)
- **Total Warnings**: 62
- **BTZ Code**: ~50
- **JUCE Code**: ~12

### Target (Ship Gate)
- **Total Warnings**: <10
- **BTZ Code**: <5
- **JUCE Code**: <5 (document any)

### Progress Log
| Date | Total | BTZ | JUCE | Notes |
|------|-------|-----|------|-------|
| 2026-01-08 | 62 | 50 | 12 | Baseline inventory |
| TBD | - | - | - | Phase 1 fixes applied |

---

**Bottom Line**: BTZ aims for **zero avoidable warnings**. Each warning fixed improves code quality, reduces bugs, and demonstrates professional engineering.

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Release Engineering
