# BTZ Disabled Modules Analysis

**Version**: 1.0.0
**Purpose**: Document disabled code modules and ship-readiness recommendations
**Last Updated**: 2026-01-08

---

## 🎯 EXECUTIVE SUMMARY

BTZ currently has **4 disabled modules** that are commented out in `BTZ_JUCE/CMakeLists.txt`. These modules represent advanced DSP features that are incomplete or have compilation issues.

**Ship-Readiness Impact**: ⚠️ **MEDIUM RISK**
- Files exist in codebase but aren't compiled
- May confuse future maintainers
- Could have licensing or IP issues if not properly vetted
- Increase technical debt

**Recommendation**: **DOCUMENT + DEFER** (not critical for 1.0.0 ship)
- Document why disabled and cleanup plan
- Move to `Source/DSP/Experimental/` directory
- Add explicit "NOT FOR PRODUCTION" headers
- Create GitHub issues for future re-evaluation

---

## 📋 DISABLED MODULES INVENTORY

### 1. AdvancedSaturation.cpp / AdvancedSaturation.h

**Purpose**: Advanced multi-mode saturation algorithms from open-source research

**Features**:
- 6 saturation modes:
  - Spiral (Airwindows 2024 - smoothest distortion)
  - Density (sine-based, infinitely smooth)
  - PurestDrive (natural harmonics)
  - Tape (hysteresis modeling)
  - Transformer (even harmonics)
  - Tube (2nd + 3rd harmonics)
- DC blocker integration
- Drive and warmth controls

**Status**: 🔴 **DISABLED**
- File location: `BTZ_JUCE/Source/DSP/AdvancedSaturation.cpp`
- LOC: ~150 lines (partial implementation)
- Header exists: Yes
- Compilation status: Unknown (not tested)

**Why Disabled**:
- Likely incomplete implementation (methods defined but not fully implemented)
- Potential compilation errors in saturation functions
- Not integrated into PluginProcessor
- Overlaps with existing `Saturation.cpp` module

**License Concerns**: ⚠️ **NEEDS REVIEW**
- References Airwindows algorithms (MIT license - SAFE)
- References ChowDSP AnalogTapeModel (BSD license - SAFE)
- **Action Required**: Verify all algorithms are clean-room implementations
- **Risk Level**: LOW (if properly attributed)

**Ship-Readiness Recommendation**: **DEFER TO 1.1.0**
- ✅ Safe to leave disabled for 1.0.0 ship
- ⚠️ Move to `Source/DSP/Experimental/AdvancedSaturation.cpp`
- ⚠️ Add header comment: "NOT FOR PRODUCTION - Experimental feature"
- ⚠️ Update OPEN_SOURCE_RECON.md with Airwindows algorithm status
- 📝 Create GitHub issue: "Complete AdvancedSaturation implementation (v1.1)"

---

### 2. AdvancedTransientShaper.cpp / AdvancedTransientShaper.h

**Purpose**: Advanced transient shaping with envelope detection

**Features**:
- Peak and RMS envelope followers
- Adaptive threshold detection
- Attack/sustain control
- RMS window for smoothing

**Status**: 🔴 **DISABLED**
- File location: `BTZ_JUCE/Source/DSP/AdvancedTransientShaper.cpp`
- LOC: ~50 lines (partial implementation)
- Header exists: Yes
- Compilation status: Unknown

**Why Disabled**:
- Incomplete implementation (`updateCoefficients()` called but not implemented)
- RMS window logic incomplete
- Not integrated into PluginProcessor
- Overlaps with existing `TransientShaper.cpp`

**License Concerns**: ✅ **CLEAN**
- No external algorithm references
- Appears to be original BTZ implementation

**Ship-Readiness Recommendation**: **DEFER TO 1.1.0**
- ✅ Safe to leave disabled for 1.0.0 ship
- ⚠️ Move to `Source/DSP/Experimental/AdvancedTransientShaper.cpp`
- ⚠️ Add header comment: "NOT FOR PRODUCTION - Incomplete implementation"
- 📝 Create GitHub issue: "Complete AdvancedTransientShaper (v1.1)"

---

### 3. WDFSaturation.cpp / WDFSaturation.h

**Purpose**: Wave Digital Filter-based analog circuit modeling

**Features**:
- WDF framework implementation (Resistor, Capacitor, VoltageSource, DiodeClipper)
- 6 circuit models:
  - Tube 12AX7 (triode vacuum tube)
  - Transformer (iron-core saturation)
  - Transistor Si/Ge (clipper circuits)
  - Op-Amp NE5534/TL072 (soft clipping)
- Frequency-dependent saturation
- Physically accurate modeling

**Status**: 🔴 **DISABLED**
- File location: `BTZ_JUCE/Source/DSP/WDFSaturation.cpp`
- LOC: ~50 lines implementation + ~319 lines header (WDF framework)
- Header exists: Yes (includes full WDF port implementation)
- Compilation status: Unknown

**Why Disabled**:
- Complex WDF framework may have compilation issues
- References ChowDSP WDF library (external dependency)
- Not integrated into PluginProcessor
- High complexity for 1.0.0 release

**License Concerns**: ⚠️ **NEEDS REVIEW**
- References ChowDSP WDF library (https://github.com/Chowdhury-DSP/chowdsp_wdf)
- Header says "simplified implementation" vs "integrate full library"
- **Action Required**: Verify if ChowDSP WDF is MIT/BSD licensed
- **Risk Level**: MEDIUM (external library dependency)

**Ship-Readiness Recommendation**: **DEFER TO 2.0.0 (MAJOR FEATURE)**
- ✅ Safe to leave disabled for 1.0.0 ship
- ⚠️ Move to `Source/DSP/Experimental/WDFSaturation.cpp`
- ⚠️ Add header comment: "NOT FOR PRODUCTION - Requires ChowDSP WDF integration"
- ⚠️ Update OPEN_SOURCE_RECON.md with ChowDSP WDF library status
- 📝 Create GitHub issue: "WDF saturation integration (v2.0 - Pro feature)"

---

### 4. LUFSMeter.cpp / LUFSMeter.h

**Purpose**: ITU-R BS.1770-4 compliant LUFS metering

**Features**:
- Momentary, short-term, and integrated LUFS
- K-weighting filters (high shelf + high pass)
- 400ms block processing
- Thread-safe atomic storage

**Status**: 🔴 **DISABLED**
- File location: `BTZ_JUCE/Source/DSP/LUFSMeter.cpp`
- LOC: ~50 lines (partial implementation)
- Header exists: Yes
- Compilation status: Unknown

**Why Disabled**:
- Incomplete implementation (`calculateFilterCoefficients()` called but not implemented)
- SPARK limiter may already use simpler LUFS measurement
- Not integrated into PluginProcessor
- Overlap with existing metering in `MeterStrip.cpp`

**License Concerns**: ⚠️ **NEEDS REVIEW**
- References libebur128 (https://github.com/jiixyj/libebur128) - MIT license (SAFE)
- ITU-R BS.1770-4 is public specification (SAFE)
- **Action Required**: Verify no libebur128 code copied (clean-room only)
- **Risk Level**: LOW (if clean-room implementation)

**Ship-Readiness Recommendation**: **DEFER TO 1.1.0**
- ✅ Safe to leave disabled for 1.0.0 ship
- ⚠️ Move to `Source/DSP/Experimental/LUFSMeter.cpp`
- ⚠️ Add header comment: "NOT FOR PRODUCTION - Incomplete ITU-R BS.1770-4 implementation"
- ⚠️ Update OPEN_SOURCE_RECON.md with libebur128 reference status
- 📝 Create GitHub issue: "Complete ITU-R BS.1770-4 LUFS meter (v1.1)"

---

## 🚨 IMMEDIATE ACTIONS (PRE-SHIP 1.0.0)

### 1. Quarantine Disabled Modules
```bash
# Create experimental directory
mkdir -p BTZ_JUCE/Source/DSP/Experimental

# Move disabled modules
mv BTZ_JUCE/Source/DSP/AdvancedSaturation.* BTZ_JUCE/Source/DSP/Experimental/
mv BTZ_JUCE/Source/DSP/AdvancedTransientShaper.* BTZ_JUCE/Source/DSP/Experimental/
mv BTZ_JUCE/Source/DSP/WDFSaturation.* BTZ_JUCE/Source/DSP/Experimental/
mv BTZ_JUCE/Source/DSP/LUFSMeter.* BTZ_JUCE/Source/DSP/Experimental/
```

### 2. Update CMakeLists.txt Comments
```cmake
# Experimental modules (not for production)
# Moved to Source/DSP/Experimental/ - see docs/DISABLED_MODULES.md
# - AdvancedSaturation.cpp (multi-mode saturation, incomplete)
# - AdvancedTransientShaper.cpp (advanced envelope detection, incomplete)
# - WDFSaturation.cpp (WDF circuit modeling, requires external library)
# - LUFSMeter.cpp (ITU-R BS.1770-4 metering, incomplete)
```

### 3. Add "NOT FOR PRODUCTION" Headers
Add to top of each disabled file:
```cpp
/*
  ==============================================================================
  ⚠️  NOT FOR PRODUCTION USE ⚠️

  This module is DISABLED and INCOMPLETE.
  Do not enable in CMakeLists.txt without:
  1. Completing implementation
  2. Adding unit tests
  3. Verifying license compliance
  4. Integration testing with full plugin

  See docs/DISABLED_MODULES.md for details.
  ==============================================================================
*/
```

### 4. Update OPEN_SOURCE_RECON.md
Add section:
```markdown
## 🚧 EXPERIMENTAL MODULES (DISABLED)

### Not Integrated (Reference-Only)
| Component | License | Status | Notes |
|-----------|---------|--------|-------|
| Airwindows algorithms | MIT | REFERENCE | AdvancedSaturation references but not yet verified |
| ChowDSP WDF library | Unknown | RESEARCH | WDFSaturation requires external integration |
| libebur128 | MIT | REFERENCE | LUFSMeter references but clean-room only |
```

### 5. Create GitHub Issues
- Issue #1: "Complete AdvancedSaturation implementation (v1.1)"
- Issue #2: "Complete AdvancedTransientShaper (v1.1)"
- Issue #3: "Evaluate ChowDSP WDF integration for WDFSaturation (v2.0)"
- Issue #4: "Complete ITU-R BS.1770-4 LUFS meter (v1.1)"

---

## 📊 RISK ANALYSIS

### License Risk: ⚠️ **MEDIUM**
| Module | External Reference | License | Risk | Action Required |
|--------|-------------------|---------|------|-----------------|
| AdvancedSaturation | Airwindows | MIT | LOW | Verify clean-room implementation |
| AdvancedTransientShaper | None | N/A | NONE | Original implementation |
| WDFSaturation | ChowDSP WDF | Unknown | MEDIUM | Investigate ChowDSP license |
| LUFSMeter | libebur128 | MIT | LOW | Verify no code copied |

### Compilation Risk: 🔴 **HIGH**
- None of the 4 modules have been compiled recently
- Likely contain compilation errors
- May have missing dependencies
- Not tested with JUCE 7.0.12

### Integration Risk: 🟡 **MEDIUM**
- Not integrated into PluginProcessor
- No parameters exposed
- No UI controls
- May overlap with existing features (Saturation.cpp, TransientShaper.cpp)

### Maintenance Risk: 🟡 **MEDIUM**
- 4 files + 4 headers = 8 files of unmaintained code
- No tests
- No documentation (besides headers)
- Increases cognitive load for new developers

---

## ✅ SHIP GATE CHECKLIST (1.0.0 RELEASE)

Before shipping BTZ 1.0.0:
- [ ] Move disabled modules to `Source/DSP/Experimental/`
- [ ] Add "NOT FOR PRODUCTION" headers to all 4 modules
- [ ] Update CMakeLists.txt comments with explicit reason for disabling
- [ ] Update OPEN_SOURCE_RECON.md with experimental module status
- [ ] Create 4 GitHub issues for future completion
- [ ] Verify no disabled code is accidentally linked into plugin binary
- [ ] Update THIRD_PARTY_NOTICES.md if any experimental code uses external algorithms

**Status**: 🔄 **PENDING** (not critical for ship, but good hygiene)

---

## 🔮 FUTURE ROADMAP

### Version 1.1.0 (Minor Feature Release)
- ✅ Complete AdvancedSaturation implementation
- ✅ Complete AdvancedTransientShaper implementation
- ✅ Complete LUFSMeter implementation
- ✅ Add unit tests for all 3 modules
- ✅ Integrate into PluginProcessor (optional parameters)

### Version 2.0.0 (Major Feature Release - "Pro" Edition)
- ✅ Integrate ChowDSP WDF library (proper CMake dependency)
- ✅ Complete WDFSaturation with all 6 circuit models
- ✅ Add "Analog Modeling" preset section
- ✅ Benchmark CPU usage and optimize
- ✅ Market as "WDF-modeled analog saturation"

### Version 2.1.0 (Premium Analog Modeling)
- ✅ Kernel Color Layer (KCL) with legal convolution
- ✅ Dynamic kernel switching based on program material
- ✅ Integration with AdvancedSaturation + WDFSaturation
- ✅ "Premium Analog Modeling Stack" as selling point

---

## 📚 REFERENCES

- **ChowDSP WDF Library**: https://github.com/Chowdhury-DSP/chowdsp_wdf
- **Airwindows Plugins**: https://github.com/airwindows/airwindows (MIT license)
- **libebur128**: https://github.com/jiixyj/libebur128 (MIT license)
- **ITU-R BS.1770-4**: https://www.itu.int/rec/R-REC-BS.1770/ (public spec)
- **WDF Tutorial**: https://ccrma.stanford.edu/~jatin/slides/TAP_WDFs.pdf

---

## 📝 MAINTENANCE LOG

| Date | Action | Reason |
|------|--------|--------|
| 2026-01-08 | Created DISABLED_MODULES.md | Document disabled code for ship-readiness |
| TBD | Move to Experimental/ | Quarantine non-production code |
| TBD | Add NOT FOR PRODUCTION headers | Prevent accidental enablement |
| TBD | Create GitHub issues | Track future completion |

---

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Engineering Team

**Bottom Line**: All 4 disabled modules are safe to leave disabled for 1.0.0 ship. Move to `Experimental/` directory, add clear "NOT FOR PRODUCTION" warnings, and defer completion to 1.1.0/2.0.0 releases. No ship-blocking issues, but good hygiene to document and quarantine.
