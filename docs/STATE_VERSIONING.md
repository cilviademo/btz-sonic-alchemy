# BTZ State Versioning & Migration Strategy

**Version**: 1.0.0
**Purpose**: Ensure backward compatibility across all BTZ versions
**Last Updated**: 2026-01-08

---

## 🎯 GOAL

**Principle**: A DAW project saved with BTZ 1.0.0 must open correctly in BTZ 1.1.0, 2.0.0, and all future versions without manual user intervention.

**Requirements**:
- ✅ Parameter values preserved exactly
- ✅ Sonic behavior unchanged (or gracefully degraded)
- ✅ No crashes or invalid states
- ✅ Clear user feedback if migration required

---

## 📋 VERSION SCHEME

BTZ follows Semantic Versioning (SemVer) with audio-specific interpretations:

### MAJOR.MINOR.PATCH (e.g., 1.0.0)

- **MAJOR**: Breaking changes (DSP algorithm change, parameter removed)
- **MINOR**: New features (new parameters, new DSP modules)
- **PATCH**: Bug fixes (no parameter changes, no DSP changes)

**Migration Rules**:
- **PATCH** bumps: No migration needed (100% compatible)
- **MINOR** bumps: Additive migration (new parameters get defaults)
- **MAJOR** bumps: Complex migration (algorithm changed, manual mapping required)

---

## 📊 STATE FORMAT

### Current Format (v1.0.0)

BTZ uses JUCE `AudioProcessorValueTreeState` (APVTS) for state serialization.

**XML Structure**:
```xml
<?xml version="1.0" encoding="UTF-8"?>

<BTZ version="1.0.0">
  <PARAM id="punch" value="0.3"/>
  <PARAM id="warmth" value="0.5"/>
  <PARAM id="boom" value="0.2"/>
  <PARAM id="mix" value="1.0"/>
  <PARAM id="drive" value="0.0"/>
  <PARAM id="texture" value="0.0"/>

  <PARAM id="inputGain" value="0.5"/>
  <PARAM id="outputGain" value="0.5"/>
  <PARAM id="autoGain" value="0.0"/>

  <PARAM id="sparkEnabled" value="0.0"/>
  <PARAM id="sparkLUFS" value="0.5"/>
  <PARAM id="sparkCeiling" value="0.9"/>
  <PARAM id="sparkMix" value="1.0"/>
  <PARAM id="sparkOS" value="0.75"/>
  <PARAM id="sparkAutoOS" value="0.0"/>
  <PARAM id="sparkMode" value="0.5"/>

  <PARAM id="shineEnabled" value="0.0"/>
  <PARAM id="shineFreqHz" value="0.3"/>
  <PARAM id="shineGainDb" value="0.5"/>
  <PARAM id="shineQ" value="0.5"/>
  <PARAM id="shineMix" value="1.0"/>
  <PARAM id="shineAutoOS" value="0.0"/>

  <PARAM id="masterEnabled" value="0.0"/>
  <PARAM id="masterMacro" value="0.0"/>
  <PARAM id="masterBlend" value="0.33"/>
  <PARAM id="masterMix" value="1.0"/>

  <PARAM id="precisionMode" value="0.0"/>
  <PARAM id="active" value="1.0"/>
  <PARAM id="oversampling" value="0.0"/>
</BTZ>
```

**Key Fields**:
- `version`: BTZ version that saved this state (SemVer string)
- `PARAM id`: Parameter identifier (MUST NEVER CHANGE)
- `PARAM value`: Normalized value (0.0 - 1.0)

---

## 🔄 MIGRATION STRATEGY

### Loading State (setStateInformation)

```cpp
void BTZAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // 1. Parse XML
    auto xml = parseXML(String::createStringFromData(data, sizeInBytes));
    if (xml == nullptr) return;  // Corrupted state

    // 2. Read version field
    String savedVersion = xml->getStringAttribute("version", "0.0.0");

    // 3. Apply migration based on version
    if (savedVersion == "1.0.0")
    {
        // No migration needed (current version)
        apvts.replaceState(ValueTree::fromXml(*xml));
    }
    else if (savedVersion.startsWith("0."))
    {
        // Migrate from pre-1.0 version (if applicable)
        migrateFrom_v0_to_v1(xml);
    }
    else if (savedVersion == "1.1.0")
    {
        // Forward compatibility (1.1 → 1.0 downgrade)
        // Ignore unknown parameters, keep known ones
        apvts.replaceState(ValueTree::fromXml(*xml));
    }
    else
    {
        // Unknown version - best-effort load
        apvts.replaceState(ValueTree::fromXml(*xml));
        // Log warning to user
        DBG("WARNING: State saved with unknown version " + savedVersion);
    }
}
```

---

## 📖 MIGRATION SCENARIOS

### Scenario 1: Patch Upgrade (1.0.0 → 1.0.1)

**Change**: Bug fix in SPARK limiter (no parameter changes)

**Migration**:
- ✅ No migration needed
- ✅ Load state as-is
- ✅ 100% compatibility

**Code**:
```cpp
// No special handling - direct load
apvts.replaceState(ValueTree::fromXml(*xml));
```

---

### Scenario 2: Minor Upgrade (1.0.0 → 1.1.0)

**Change**: Added new parameter `sparkAttackMs` (attack time for limiter)

**Migration**:
- ✅ Load all existing parameters from 1.0.0 state
- ✅ New parameter `sparkAttackMs` uses default value (5.0 ms)
- ✅ User's existing settings preserved

**Code**:
```cpp
// APVTS automatically handles missing parameters
// New parameter uses default from ParameterLayout
apvts.replaceState(ValueTree::fromXml(*xml));
// No explicit migration code needed!
```

**Evidence**: JUCE APVTS handles missing parameters gracefully

---

### Scenario 3: Major Upgrade (1.0.0 → 2.0.0) - Parameter Renamed

**Change**: Renamed `sparkOS` to `sparkOversamplingFactor` (clarity)

**Migration**:
```cpp
void migrateFrom_v1_to_v2(XmlElement* xml)
{
    // 1. Find old parameter
    auto oldParam = xml->getChildByAttribute("id", "sparkOS");
    if (oldParam != nullptr)
    {
        float oldValue = oldParam->getDoubleAttribute("value");

        // 2. Create new parameter with same value
        auto newParam = xml->createNewChildElement("PARAM");
        newParam->setAttribute("id", "sparkOversamplingFactor");
        newParam->setAttribute("value", oldValue);

        // 3. Remove old parameter
        xml->removeChildElement(oldParam, true);
    }

    // 4. Update version field
    xml->setAttribute("version", "2.0.0");

    // 5. Load migrated state
    apvts.replaceState(ValueTree::fromXml(*xml));
}
```

**User Impact**: ✅ Transparent (no manual action required)

---

### Scenario 4: Major Upgrade (1.0.0 → 2.0.0) - Algorithm Changed

**Change**: SPARK limiter algorithm changed (different gain reduction curve)

**Migration Options**:

#### Option A: **Graceful Degradation** (Recommended)
- Load old state as-is
- Display notification: "SPARK limiter updated in v2.0. Sonic behavior may differ slightly."
- Provide "Revert to Legacy Algorithm" button (optional)

#### Option B: **Parameter Remapping**
- Map old parameters to new algorithm equivalents
- Example: Old `sparkCeiling=-0.3dBTP` → New `sparkThreshold=-0.3dBTP`

#### Option C: **Freeze Version**
- Save algorithm version alongside plugin version
- Load legacy algorithm for old projects
- Pro: 100% sonic compatibility
- Con: Maintain legacy code forever

**Recommendation**: Option A (document behavior change, provide legacy mode if critical)

---

## 🛡️ SAFETY RULES

### Rule 1: **NEVER CHANGE PARAMETER IDs**

❌ **BAD**:
```cpp
// Renaming parameter ID
const juce::String sparkOS = "sparkOversamplingFactor";  // BREAKS OLD PROJECTS
```

✅ **GOOD**:
```cpp
// Keep ID, update UI label only
const juce::String sparkOS = "sparkOS";  // ID unchanged
// UI displays "Oversampling Factor" instead
```

**Exception**: Major version bump with explicit migration code

---

### Rule 2: **NEVER REMOVE PARAMETERS WITHOUT MIGRATION**

❌ **BAD**:
```cpp
// Remove parameter entirely
// const juce::String boom = "boom";  // DELETED - old projects will lose boom value!
```

✅ **GOOD**:
```cpp
// Deprecate but keep for compatibility
const juce::String boom = "boom";  // Deprecated in 2.0, but still loaded for old projects
// Migration: Map boom → newSubBassGain
```

---

### Rule 3: **ALWAYS INCLUDE VERSION FIELD**

✅ **REQUIRED**:
```cpp
void getStateInformation(MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    xml->setAttribute("version", JucePlugin_VersionString);  // "1.0.0"
    copyXmlToBinary(*xml, destData);
}
```

**Current Status**: ⚠️ **NEEDS IMPLEMENTATION** (version field not yet added)

---

### Rule 4: **TEST MIGRATION WITH GOLDEN STATE FILES**

**Golden State Files**: Pre-saved BTZ states for each version

Example:
```
tests/golden_states/
  ├── v1.0.0_default.xml        # Default state from 1.0.0
  ├── v1.0.0_extreme.xml        # All parameters at extremes
  ├── v1.1.0_default.xml        # Default state from 1.1.0
  └── v2.0.0_default.xml        # Default state from 2.0.0
```

**Test**:
```cpp
void test_migration_v1_to_v2()
{
    // Load v1.0.0 state
    auto v1State = loadGoldenState("v1.0.0_default.xml");
    processor.setStateInformation(v1State.data(), v1State.size());

    // Verify all parameters loaded correctly
    assert(processor.getAPVTS().getParameter("punch")->getValue() == 0.0f);
    // ... test all 27 parameters

    // Save as v2.0.0
    MemoryBlock v2State;
    processor.getStateInformation(v2State);

    // Verify version field updated
    auto xml = parseXML(v2State.toString());
    assert(xml->getStringAttribute("version") == "2.0.0");
}
```

**Status**: ⚠️ **PENDING** (golden state tests not yet created)

---

## 📊 MIGRATION TABLE

| From Version | To Version | Migration Required | Complexity | Status |
|--------------|------------|-------------------|------------|--------|
| 1.0.0 | 1.0.1 | No | None | ✅ N/A (no 1.0.1 yet) |
| 1.0.0 | 1.1.0 | Additive | Low | 📝 Planned |
| 1.0.0 | 2.0.0 | Full | High | 📝 Planned |
| 0.x.x | 1.0.0 | Full | High | ❌ N/A (no pre-1.0 versions) |

---

## 🧪 TESTING STRATEGY

### Unit Tests (migration_test.cpp)

```cpp
#include "PluginProcessor.h"
#include <cassert>

void test_default_state_loads()
{
    BTZAudioProcessor processor;
    // Default state should have version "1.0.0"
    MemoryBlock state;
    processor.getStateInformation(state);

    auto xml = parseXML(state.toString());
    assert(xml->getStringAttribute("version") == "1.0.0");
}

void test_load_v1_0_0_state()
{
    // Load pre-saved v1.0.0 state
    auto state = loadGoldenState("v1.0.0_default.xml");

    BTZAudioProcessor processor;
    processor.setStateInformation(state.data(), state.size());

    // Verify all 27 parameters loaded
    auto& apvts = processor.getAPVTS();
    assert(apvts.getParameter("punch") != nullptr);
    // ... test all parameters
}

void test_round_trip_determinism()
{
    BTZAudioProcessor processor1, processor2;

    // Set known state
    processor1.getAPVTS().getParameter("punch")->setValueNotifyingHost(0.5f);

    // Save state
    MemoryBlock state;
    processor1.getStateInformation(state);

    // Load into second processor
    processor2.setStateInformation(state.data(), state.size());

    // Verify match
    assert(processor1.getAPVTS().getParameter("punch")->getValue() ==
           processor2.getAPVTS().getParameter("punch")->getValue());
}
```

**Status**: ⚠️ **PARTIAL** (state_roundtrip_test.cpp exists, migration tests pending)

---

## 📝 CHANGELOG INTEGRATION

### CHANGELOG.md Format

```markdown
# BTZ Changelog

## [1.1.0] - 2026-03-01

### Added
- New parameter `sparkAttackMs` for limiter attack time control

### Migration Notes
- Projects saved with 1.0.0 will load with `sparkAttackMs` set to default (5.0 ms)
- No manual action required

---

## [2.0.0] - 2026-08-01

### Changed
- SPARK limiter algorithm updated (improved transparency)

### Deprecated
- Parameter `sparkOS` renamed to `sparkOversamplingFactor` (automatic migration)

### Migration Notes
- Projects saved with 1.x will automatically migrate `sparkOS` → `sparkOversamplingFactor`
- Sonic behavior may differ slightly due to limiter algorithm change
- Use "Legacy Mode" toggle to revert to 1.x algorithm
```

---

## 🚨 USER NOTIFICATION STRATEGY

### Migration Success
```
┌─────────────────────────────────────────────────────┐
│ BTZ State Migration                                  │
├─────────────────────────────────────────────────────┤
│ This project was saved with BTZ 1.0.0.              │
│ Automatically migrated to BTZ 2.0.0.                │
│                                                       │
│ What changed:                                        │
│ • Parameter "sparkOS" → "sparkOversamplingFactor"   │
│ • SPARK limiter algorithm improved                  │
│                                                       │
│ [✓] Don't show this again   [ Close ]               │
└─────────────────────────────────────────────────────┘
```

### Migration Warning
```
┌─────────────────────────────────────────────────────┐
│ ⚠️  BTZ Compatibility Warning                       │
├─────────────────────────────────────────────────────┤
│ This project was saved with BTZ 2.5.0.              │
│ You are using BTZ 1.0.0 (older version).            │
│                                                       │
│ Some settings may not load correctly.               │
│                                                       │
│ Recommendation: Update to BTZ 2.5.0 or later.       │
│                                                       │
│ [ Continue Anyway ]   [ Visit Download Page ]       │
└─────────────────────────────────────────────────────┘
```

---

## ✅ SHIP GATE CHECKLIST (1.0.0 Release)

Before shipping BTZ 1.0.0:
- [ ] Add version field to `getStateInformation()`
- [ ] Implement version parsing in `setStateInformation()`
- [ ] Create golden state test files (v1.0.0_default.xml, v1.0.0_extreme.xml)
- [ ] Add migration unit tests (test_load_v1_0_0_state)
- [ ] Document all 27 parameters as "STABLE" (no changes without migration)
- [ ] Update CHANGELOG.md with migration notes template

**Status**: 🔄 **PENDING** (version field implementation required)

---

## 📚 REFERENCES

- **Semantic Versioning**: https://semver.org/
- **PARAMETER_MANIFEST.md**: All 27 parameter definitions
- **TEST_SUITE.md**: state_roundtrip_test.cpp documentation
- **JUCE ValueTree**: https://docs.juce.com/master/classValueTree.html

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Release Engineering

**Bottom Line**: BTZ ensures backward compatibility through versioned state serialization, automatic migration, and comprehensive testing. All future versions will load 1.0.0 projects without user intervention.
