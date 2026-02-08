# BTZ Parameter Manifest

**Version**: 1.0.0
**Purpose**: Comprehensive parameter documentation for state management and migration
**Plugin Version**: BTZ 1.0.0
**Last Updated**: 2026-01-08

---

## 🎯 OVERVIEW

This document defines **every parameter** in BTZ with compile-time stability guarantees. **Parameter IDs MUST NOT change** without implementing proper migration logic.

**Parameter Count**: 29 total
- **Hero Controls**: 5
- **Texture**: 1
- **I/O Trim**: 3
- **SPARK Engine**: 7
- **SHINE Engine**: 6
- **Master**: 4
- **System**: 3 (precisionMode, active, oversampling)

---

## 📋 PARAMETER REFERENCE TABLE

### Hero Controls (Primary Mix Interface)

| ID | Name | Type | Range | Default | Unit | Skew | Automation | Version |
|----|------|------|-------|---------|------|------|------------|---------|
| `punch` | Punch | Float | 0.0 - 1.0 | 0.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `warmth` | Warmth | Float | 0.0 - 1.0 | 0.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `boom` | Boom | Float | 0.0 - 1.0 | 0.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `mix` | Mix | Float | 0.0 - 1.0 | 1.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `drive` | Drive | Float | 0.0 - 1.0 | 0.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |

**DSP Mapping**:
- `punch`: TransientShaper amount (transient emphasis)
- `warmth`: Saturation drive (harmonic generation)
- `boom`: SubHarmonic amount (low-frequency enhancement)
- `mix`: Dry/wet blend for all processing
- `drive`: Master drive/intensity control

---

### Texture Toggle

| ID | Name | Type | Values | Default | Automation | Version |
|----|------|------|--------|---------|------------|---------|
| `texture` | Texture | Bool | OFF (0), ON (1) | OFF | ✅ Yes | 1.0.0 |

**DSP Mapping**: Console emulation / analog character enable/disable

---

### I/O Trim

| ID | Name | Type | Range | Default | Unit | Skew | Automation | Version |
|----|------|------|-------|---------|------|------|------------|---------|
| `inputGain` | Input Gain | Float | -12.0 - +12.0 | 0.0 | dB | 1.0 | ✅ Yes | 1.0.0 |
| `outputGain` | Output Gain | Float | -12.0 - +12.0 | 0.0 | dB | 1.0 | ✅ Yes | 1.0.0 |
| `autoGain` | Auto Gain | Bool | OFF (0), ON (1) | OFF | ✅ Yes | 1.0.0 |

**DSP Mapping**:
- `inputGain`: Input trim before processing
- `outputGain`: Output trim after processing
- `autoGain`: Automatic gain compensation (future feature)

---

### SPARK - Advanced Clipping Engine

| ID | Name | Type | Range/Values | Default | Unit | Skew | Automation | Version |
|----|------|------|--------------|---------|------|------|------------|---------|
| `sparkEnabled` | Spark Enabled | Bool | OFF (0), ON (1) | ON | - | - | ✅ Yes | 1.0.0 |
| `sparkLUFS` | Spark Target LUFS | Float | -14.0 - 0.0 | -5.0 | LUFS | 1.0 | ✅ Yes | 1.0.0 |
| `sparkCeiling` | Spark Ceiling | Float | -3.0 - 0.0 | -0.3 | dBTP | 1.0 | ✅ Yes | 1.0.0 |
| `sparkMix` | Spark Mix | Float | 0.0 - 1.0 | 1.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `sparkOS` | Spark Oversampling | Choice | 0=1x, 1=2x, 2=4x, 3=8x, 4=16x | 3 (8x) | - | - | ⚠️ Careful | 1.0.0 |
| `sparkAutoOS` | Spark Auto OS | Bool | OFF (0), ON (1) | ON | - | - | ✅ Yes | 1.0.0 |
| `sparkMode` | Spark Mode | Choice | 0=Soft, 1=Hard | 0 (Soft) | - | - | ✅ Yes | 1.0.0 |

**DSP Mapping**:
- `sparkEnabled`: Enable/disable SPARK limiter
- `sparkLUFS`: Target loudness (informational, not guaranteed)
- `sparkCeiling`: True-peak ceiling (absolute limit)
- `sparkMix`: Dry/wet for SPARK processing
- `sparkOS`: Manual oversampling factor
- `sparkAutoOS`: Automatic oversampling based on content
- `sparkMode`: Clipping transfer curve (Soft = gentle, Hard = aggressive)

**Automation Notes**:
- `sparkOS`: Automation possible but triggers async update (may cause brief glitches)

---

### SHINE - Ultra-High Frequency Air

| ID | Name | Type | Range/Values | Default | Unit | Skew | Automation | Version |
|----|------|------|--------------|---------|------|------|------------|---------|
| `shineEnabled` | Shine Enabled | Bool | OFF (0), ON (1) | OFF | - | - | ✅ Yes | 1.0.0 |
| `shineFreqHz` | Shine Frequency | Float | 10000.0 - 80000.0 | 20000.0 | Hz | 0.3 | ✅ Yes | 1.0.0 |
| `shineGainDb` | Shine Gain | Float | -12.0 - +12.0 | +3.0 | dB | 1.0 | ✅ Yes | 1.0.0 |
| `shineQ` | Shine Q | Float | 0.1 - 2.0 | 0.5 | - | 0.5 | ✅ Yes | 1.0.0 |
| `shineMix` | Shine Mix | Float | 0.0 - 1.0 | 0.5 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `shineAutoOS` | Shine Auto OS | Bool | OFF (0), ON (1) | ON | - | - | ✅ Yes | 1.0.0 |

**DSP Mapping**:
- `shineEnabled`: Enable/disable SHINE EQ
- `shineFreqHz`: Air band center frequency (ultrasonic capable)
- `shineGainDb`: Boost/cut amount
- `shineQ`: Filter width (0.1 = very wide, 2.0 = narrow)
- `shineMix`: Dry/wet for SHINE processing
- `shineAutoOS`: Automatic oversampling for ultrasonic processing

**Skew Notes**:
- `shineFreqHz`: Skew 0.3 (logarithmic) for musical frequency distribution
- `shineQ`: Skew 0.5 (slight compression) for better control at low Q

---

### Master

| ID | Name | Type | Range/Values | Default | Unit | Skew | Automation | Version |
|----|------|------|--------------|---------|------|------|------------|---------|
| `masterEnabled` | Master Enabled | Bool | OFF (0), ON (1) | OFF | - | - | ✅ Yes | 1.0.0 |
| `masterMacro` | Master Macro | Float | 0.0 - 1.0 | 0.5 | normalized | 1.0 | ✅ Yes | 1.0.0 |
| `masterBlend` | Master Blend | Choice | 0=Transparent, 1=Glue, 2=Vintage | 0 (Transparent) | - | - | ✅ Yes | 1.0.0 |
| `masterMix` | Master Mix | Float | 0.0 - 1.0 | 1.0 | normalized | 1.0 | ✅ Yes | 1.0.0 |

**DSP Mapping**:
- `masterEnabled`: Enable/disable master processing stage
- `masterMacro`: Master control (implementation-specific)
- `masterBlend`: Console emulation type
- `masterMix`: Dry/wet for master stage

---

### System

| ID | Name | Type | Values | Default | Automation | Version |
|----|------|------|--------|---------|------------|---------|
| `precisionMode` | Precision Mode | Bool | OFF (0), ON (1) | OFF | ✅ Yes | 1.0.0 |
| `active` | Active | Bool | OFF (0), ON (1) | ON | ✅ Yes | 1.0.0 |
| `oversampling` | Oversampling | Bool | OFF (0), ON (1) | ON | ⚠️ Careful | 1.0.0 |

**DSP Mapping**:
- `precisionMode`: Enable extended precision processing (future feature)
- `active`: Global plugin enable/disable (bypass)
- `oversampling`: Global oversampling enable (deprecated, use sparkOS/shineAutoOS)

**Automation Notes**:
- `oversampling`: Deprecated, kept for backward compatibility

---

## 🔒 PARAMETER STABILITY RULES

### ID Stability (CRITICAL)
- ❌ **NEVER** change parameter ID strings
- ❌ **NEVER** remove parameters without migration
- ❌ **NEVER** reorder parameters (affects automation indexing in some DAWs)
- ✅ **CAN** add new parameters at the end
- ✅ **CAN** change parameter ranges (with migration logic)
- ✅ **CAN** change default values (affects new instances only)

### Migration Requirements
If parameter changes are unavoidable:
1. Increment plugin version (major.minor.patch)
2. Add migration logic in `getStateInformation()` / `setStateInformation()`
3. Add test fixtures for old state files
4. Document in `docs/STATE_VERSIONING.md`

---

## 📊 PARAMETER GROUPS & VISIBILITY

### UI Organization

**Primary View** (Always Visible):
- Hero Controls: `punch`, `warmth`, `boom`, `mix`, `drive`
- Texture Toggle: `texture`
- I/O Trim: `inputGain`, `outputGain`

**Advanced View** (Expandable Panel):
- SPARK: All 7 parameters
- SHINE: All 6 parameters
- Master: All 4 parameters

**System** (Menu or Preferences):
- `precisionMode`, `active`, `oversampling`

---

## 🔢 NORMALIZED ↔ PLAIN CONVERSIONS

### Float Parameters (Linear)

**Formula**:
```cpp
// Normalized [0.0, 1.0] → Plain [min, max]
float plain = min + normalized * (max - min);

// Plain [min, max] → Normalized [0.0, 1.0]
float normalized = (plain - min) / (max - min);
```

**Example** (`inputGain`):
```cpp
// Normalized 0.5 → Plain 0.0 dB
float plainGain = -12.0f + 0.5f * (12.0f - (-12.0f));  // = 0.0 dB
```

### Float Parameters (Skewed)

**Formula**:
```cpp
// Normalized [0.0, 1.0] → Plain [min, max] with skew
float normalized_skewed = std::pow(normalized, skew);
float plain = min + normalized_skewed * (max - min);
```

**Example** (`shineFreqHz`, skew 0.3):
```cpp
// Normalized 0.5 → Plain ~16 kHz (logarithmic feel)
float normalized_skewed = std::pow(0.5f, 0.3f);  // ~0.812
float plainFreq = 10000.0f + 0.812f * (80000.0f - 10000.0f);  // ~66840 Hz
```

### Choice Parameters

**Formula**:
```cpp
// Normalized [0.0, 1.0] → Choice index [0, N-1]
int index = std::clamp((int)(normalized * numChoices), 0, numChoices - 1);

// Choice index → Normalized
float normalized = (float)index / (float)(numChoices - 1);
```

**Example** (`sparkOS`, 5 choices):
```cpp
// Normalized 0.75 → Index 3 (8x oversampling)
int osIndex = std::clamp((int)(0.75f * 5), 0, 4);  // = 3
```

### Boolean Parameters

**Formula**:
```cpp
// Normalized → Bool
bool value = normalized > 0.5f;

// Bool → Normalized
float normalized = value ? 1.0f : 0.0f;
```

---

## 💾 STATE SERIALIZATION FORMAT

### JUCE APVTS Format (XML)

**Example State**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<BTZ version="1.0.0">
  <PARAM id="punch" value="0.25"/>
  <PARAM id="warmth" value="0.5"/>
  <PARAM id="boom" value="0.0"/>
  <PARAM id="mix" value="1.0"/>
  <PARAM id="drive" value="0.3"/>
  <PARAM id="texture" value="1.0"/>
  <!-- ... all 27 parameters ... -->
</BTZ>
```

### Version Field
- **Always include**: `<BTZ version="X.Y.Z">`
- **Read on load**: Determine if migration needed
- **Write on save**: Current plugin version

---

## 🔄 MIGRATION TABLE

### Version 1.0.0 → Future Versions

**Current Version**: 1.0.0 (baseline, no migrations yet)

**Future Migration Example** (if needed):
```cpp
void migrateState(const juce::String& oldVersion, juce::ValueTree& state) {
    if (oldVersion == "0.9.0") {
        // Example: Rename parameter
        auto oldParam = state.getChildWithProperty("id", "oldParamName");
        if (oldParam.isValid()) {
            oldParam.setProperty("id", "newParamName", nullptr);
        }
    }
}
```

**Migration Test Requirements**:
- Golden state file for each old version
- Round-trip test (save → load → verify)
- Corrupt state resilience test

---

## ⚠️ AUTOMATION SAFETY

### Safe for Automation (All Parameters)
All 27 parameters support automation. However, some require care:

**Smooth Automation** (No Issues):
- Hero controls (punch, warmth, boom, mix, drive)
- I/O gain (inputGain, outputGain)
- SPARK/SHINE mix parameters
- SHINE frequency, gain, Q

**Trigger-Based** (May Cause Brief Glitches):
- `sparkOS`: Changes oversampling factor → async update → brief latency change
- `oversampling`: Global toggle → may cause buffer reallocation
- `texture`: Toggles console emulation → parameter smoothing handles this

**Best Practice**: Use parameter smoothing (LinearSmoothedValue) for all automatable floats.

---

## 🧪 VERIFICATION TESTS

### Unit Tests (Required)
```cpp
// Test: Normalized ↔ Plain conversion (inputGain)
ASSERT_FLOAT_EQ(convertNormalizedToPlain(0.5f, -12.0f, 12.0f), 0.0f);
ASSERT_FLOAT_EQ(convertPlainToNormalized(0.0f, -12.0f, 12.0f), 0.5f);

// Test: Choice index conversion (sparkOS)
ASSERT_EQ(convertNormalizedToChoiceIndex(0.75f, 5), 3);

// Test: State round-trip
auto state1 = saveStateToMemoryBlock();
loadStateFromMemoryBlock(state1);
auto state2 = saveStateToMemoryBlock();
ASSERT_EQ(state1, state2);  // Deterministic
```

---

## 📚 REFERENCES

**JUCE Documentation**:
- [AudioProcessorValueTreeState](https://docs.juce.com/master/classAudioProcessorValueTreeState.html)
- [AudioParameterFloat](https://docs.juce.com/master/classAudioParameterFloat.html)
- [AudioParameterChoice](https://docs.juce.com/master/classAudioParameterChoice.html)

**BTZ Documentation**:
- `Source/Parameters/PluginParameters.h` - Parameter definitions
- `docs/STATE_VERSIONING.md` - State format and migration
- `docs/ARCHITECTURE.md` - Parameter flow (UI → APVTS → DSP)

---

## 📝 CHANGELOG

### 1.0.0 (2026-01-08)
- Initial parameter manifest
- Documented all 27 parameters
- Defined conversion formulas
- Established migration table baseline

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Release Engineering

**Bottom Line**: This manifest is the source of truth for all BTZ parameters. Any changes MUST be documented here with migration logic and tests.
