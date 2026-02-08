# BTZ P2 Requirements Specification

**Version**: 1.0
**Date**: 2026-01-15
**Status**: Draft
**Author**: BTZ Development Team

---

## Executive Summary

This document defines requirements for BTZ's P2 development sprint, focusing on production readiness, quality assurance, and professional-grade features. P2 addresses critical gaps identified in the P1 milestone analysis: zero DSP unit test coverage, missing undo/redo functionality, incomplete GUI components, and suboptimal DSP performance.

**P2 Goal**: Achieve 95% completion and beta-ready status suitable for professional audio production environments.

---

## 1. Stability Requirements

### 1.1 Testing & Quality Assurance (CRITICAL)

**REQ-STAB-001**: DSP Unit Test Suite
**Priority**: P0 (Must-Have)
**Status**: Not Started
**Complexity**: High

**Description**: Implement comprehensive unit test coverage for all 10+ DSP modules currently shipping without verification.

**Acceptance Criteria**:
- ✅ AdvancedSaturation: 8 tests (6 saturation modes + bypass + extreme values)
- ✅ EnhancedSPARK: 10 tests (hysteresis model, true-peak detection, ITU compliance)
- ✅ EnhancedSHINE: 8 tests (24 Bark bands, psychoacoustic weighting, frequency response)
- ✅ LUFSMeter: 10 tests (K-weighting filters, ITU-R BS.1770-4 compliance, gating)
- ✅ TransientShaper: 6 tests (attack/release, ADAA, envelope follower)
- ✅ ParameterSmoother: 5 tests (smoothing curves, RT-safety, target convergence)
- ✅ Minimum 80% code coverage for critical DSP paths
- ✅ All tests pass in CI/CD pipeline
- ✅ No memory leaks detected (Valgrind on Linux, Instruments on macOS)

**Rationale**: Currently shipping untested DSP modules is a **major quality risk**. Professional users expect verified audio processing.

**Effort Estimate**: 80-120 hours
**Impact**: CRITICAL (blocks professional release)

---

**REQ-STAB-002**: Memory Safety Verification
**Priority**: P0 (Must-Have)
**Status**: Partially Complete

**Description**: Verify zero allocations/locks in audio thread across all code paths.

**Acceptance Criteria**:
- ✅ Valgrind reports zero allocations in processBlock() for 10,000 buffer cycles
- ✅ AddressSanitizer passes on all platforms
- ✅ Thread Sanitizer confirms no race conditions
- ✅ Lock-free data structures validated (AudioBuffer access, parameter updates)

**Effort Estimate**: 20-30 hours
**Impact**: CRITICAL (DAW stability)

---

### 1.2 Error Handling & Graceful Degradation

**REQ-STAB-003**: Robust Model File Loading
**Priority**: P1 (Should-Have)
**Status**: Partially Complete

**Description**: Handle missing/corrupted ML model files without crashing.

**Acceptance Criteria**:
- ✅ Missing DeepFilterNet2.tar.gz → disable ML, log warning, continue with rule-based transients
- ✅ Corrupted .pt file → fallback to default parameters
- ✅ User-visible error message in GUI (non-blocking)
- ✅ Graceful degradation documented in logs

**Effort Estimate**: 10-15 hours
**Impact**: HIGH (user experience)

---

## 2. Performance Requirements

### 2.1 DSP Optimization

**REQ-PERF-001**: Sample-Accurate Parameter Smoothing
**Priority**: P0 (Must-Have)
**Status**: Partially Complete (block-rate only)

**Description**: Replace block-rate parameter smoothing with per-sample interpolation to eliminate zipper noise on fast automation.

**Current Issue**: Parameters updated once per buffer (512 samples @ 48kHz = 10.6ms steps) → audible stepping on fast automation.

**Acceptance Criteria**:
- ✅ Per-sample smoothing for: punch, warmth, boom, drive, mix (5 critical parameters)
- ✅ Smoothing time: 20ms (960 samples @ 48kHz)
- ✅ Zero CPU increase >2% (benchmark on 2015 MacBook Pro)
- ✅ Automation ramps verified smooth in Reaper @ 32-sample buffers
- ✅ No zipper noise detectable at -60dBFS noise floor

**Effort Estimate**: 20-30 hours
**Impact**: HIGH (audio quality)

---

**REQ-PERF-002**: Unified Oversampling Strategy
**Priority**: P1 (Should-Have)
**Status**: Suboptimal (cascaded stages)

**Description**: Consolidate currently cascaded oversampling stages to reduce CPU overhead.

**Current Issue**:
1. OversamplingProcessor (TransientShaper + Saturation) → 2x/4x upsampling
2. EnhancedSPARK (internal) → potential second upsampling
→ Double conversion overhead, 15-20% CPU waste

**Acceptance Criteria**:
- ✅ Single unified 4x oversampling stage at PluginProcessor level
- ✅ All nonlinear modules (Saturation, SPARK) process upsampled signal
- ✅ 15-20% CPU reduction measured (RenderPerformanceTest)
- ✅ Aliasing performance maintained or improved (THD+N <0.01% @ 10kHz sine)

**Effort Estimate**: 30-50 hours
**Impact**: MEDIUM-HIGH (CPU efficiency)

---

### 2.2 Realtime Safety

**REQ-PERF-003**: Lock-Free Parameter Updates
**Priority**: P0 (Must-Have)
**Status**: Complete (verify in tests)

**Description**: Confirm all parameter updates use lock-free atomic operations.

**Acceptance Criteria**:
- ✅ Thread Sanitizer confirms zero lock contention in processBlock()
- ✅ std::atomic<float> verified for all smoothed parameters
- ✅ APVTS parameter changes never block audio thread

**Effort Estimate**: 5-10 hours (verification only)
**Impact**: CRITICAL (DAW compatibility)

---

## 3. Maintainability Requirements

### 3.1 Code Documentation

**REQ-MAINT-001**: DSP Module API Reference
**Priority**: P1 (Should-Have)
**Status**: Scattered comments

**Description**: Doxygen-style documentation for all DSP module headers.

**Acceptance Criteria**:
- ✅ Every public method: @brief, @param, @return, @note
- ✅ Complex modules include mathematical models (e.g., Jiles-Atherton equations for SPARK)
- ✅ References to open-source inspirations (DPF, JUCE DSP examples)
- ✅ Generated HTML docs via Doxygen

**Effort Estimate**: 15-20 hours
**Impact**: MEDIUM (developer onboarding)

---

**REQ-MAINT-002**: Architecture Documentation
**Priority**: P1 (Should-Have)
**Status**: Not Started

**Description**: High-level system design documentation.

**Deliverables**:
- `ARCHITECTURE.md`: Signal flow diagram, module dependency graph, threading model
- `PARAMETERS.md`: Each parameter's range, effect, interactions, recommended values
- Diagrams: Mermaid/PlantUML flowcharts

**Acceptance Criteria**:
- ✅ New developer can understand signal chain in <30 minutes
- ✅ Parameter interaction matrix documented (e.g., "warmth + drive → saturation stacking")

**Effort Estimate**: 25-35 hours
**Impact**: MEDIUM (long-term maintainability)

---

### 3.2 Test Infrastructure

**REQ-MAINT-003**: Continuous Integration Pipeline
**Priority**: P1 (Should-Have)
**Status**: Not Started

**Description**: Automated build/test pipeline for all platforms.

**Acceptance Criteria**:
- ✅ GitHub Actions workflow: Linux (Ubuntu 22.04), macOS (latest), Windows (2022)
- ✅ Build VST3 on all platforms
- ✅ Run unit tests + integration tests
- ✅ Fail on compiler warnings (-Werror)
- ✅ Generate test coverage reports

**Effort Estimate**: 15-25 hours
**Impact**: MEDIUM (quality gates)

---

## 4. UX Requirements

### 4.1 Parameter Management

**REQ-UX-001**: Undo/Redo System
**Priority**: P0 (Must-Have)
**Status**: Not Started

**Description**: Professional-grade undo/redo functionality expected by DAW users.

**Acceptance Criteria**:
- ✅ Undo/Redo buttons in GUI (keyboard shortcuts: Cmd+Z, Cmd+Shift+Z)
- ✅ State snapshot stack (max 100 changes, circular buffer)
- ✅ Undo reverts ALL parameter changes since last snapshot
- ✅ RT-safe implementation (snapshots taken on message thread, not audio thread)
- ✅ Undo/Redo state persists across preset loads
- ✅ Visual feedback: grayed-out buttons when stack empty

**Rationale**: Professional plugins MUST support Edit → Undo. Users expect this in Pro Tools, Logic, Ableton.

**Effort Estimate**: 30-50 hours
**Impact**: HIGH (professional usability)

---

**REQ-UX-002**: Preset Browser UI
**Priority**: P1 (Should-Have)
**Status**: Backend exists, no GUI

**Description**: Visual preset browser dialog for loading/saving custom presets.

**Acceptance Criteria**:
- ✅ Modal dialog triggered by "Browse" button
- ✅ List view: Factory (5 presets) + User (custom presets)
- ✅ Load/Save/Delete/Rename operations
- ✅ Preview mode: load preset temporarily without committing
- ✅ Search/filter by tag (e.g., "drums", "master", "gentle")
- ✅ Connects to existing PresetManager backend (PresetManager.h:75-120)

**Effort Estimate**: 30-40 hours
**Impact**: MEDIUM (power user feature)

---

### 4.2 GUI Polish

**REQ-UX-003**: MeterStrip Visualization
**Priority**: P1 (Should-Have)
**Status**: Stubbed (TODO comment exists)

**Description**: Professional-grade metering display with color-coded zones.

**Acceptance Criteria**:
- ✅ LUFS meter: green (<-23), yellow (-23 to -14), red (>-14)
- ✅ True Peak indicator: red bar at 0 dBFS, flashes on clip
- ✅ Gain Reduction needle: 0-20dB range, smooth ballistics
- ✅ Stereo Correlation graph: +1 (mono) to -1 (out-of-phase)
- ✅ Preset target zones overlaid (e.g., "Streaming: -14 LUFS")
- ✅ 30Hz update rate (already optimized in Quick Win 1)

**Effort Estimate**: 20-30 hours
**Impact**: MEDIUM (visual feedback)

---

**REQ-UX-004**: Tooltip Enhancements
**Priority**: P2 (Nice-to-Have)
**Status**: Basic tooltips added (Quick Win 4)

**Description**: Context-sensitive help with parameter value display.

**Acceptance Criteria**:
- ✅ Tooltips show current parameter value (e.g., "Punch: 0.45")
- ✅ Dynamic tips based on state (e.g., "SPARK: Limiting 3.2dB")
- ✅ Delay before showing: 500ms (macOS standard)

**Effort Estimate**: 10-15 hours
**Impact**: LOW (polish)

---

## 5. Backward Compatibility Requirements

**REQ-COMPAT-001**: Parameter ID Preservation
**Priority**: P0 (Must-Have)
**Status**: Complete (locked)

**Description**: NEVER change parameter IDs once released.

**Acceptance Criteria**:
- ✅ All 28 parameter IDs remain unchanged
- ✅ Session files from v0.8.0 load correctly in v1.0.0
- ✅ Deprecated parameters handled via migration layer (if needed)

**Effort Estimate**: 0 hours (design constraint)
**Impact**: CRITICAL (user trust)

---

## 6. Platform-Specific Requirements

### 6.1 Windows

**REQ-PLAT-WIN-001**: ASIO Buffer Size Compatibility
**Priority**: P0 (Must-Have)
**Status**: To Verify

**Acceptance Criteria**:
- ✅ Tested with buffer sizes: 32, 64, 128, 256, 512, 1024, 2048 samples
- ✅ No clicks/pops at 32 samples (worst case for parameter smoothing)

**Effort Estimate**: 5-10 hours (testing)

---

### 6.2 macOS

**REQ-PLAT-MAC-001**: Apple Silicon (M1/M2) Optimization
**Priority**: P1 (Should-Have)
**Status**: Not Started

**Acceptance Criteria**:
- ✅ Universal binary (x86_64 + arm64)
- ✅ NEON SIMD optimizations for ARM (if applicable)
- ✅ Verified on M1 Max @ 44.1kHz, 64-sample buffers

**Effort Estimate**: 15-25 hours

---

### 6.3 Linux

**REQ-PLAT-LIN-001**: JACK/ALSA Compatibility
**Priority**: P1 (Should-Have)
**Status**: Basic support exists

**Acceptance Criteria**:
- ✅ Tested in Reaper (Linux) with JACK backend
- ✅ No Xruns at 128-sample buffers on Ubuntu 22.04
- ✅ Correct VST3 installation path (~/.vst3/)

**Effort Estimate**: 10-15 hours (testing)

---

## 7. Non-Functional Requirements

### 7.1 Build System

**REQ-NFR-001**: CMake Cross-Platform Build
**Priority**: P0 (Must-Have)
**Status**: Complete (verify)

**Acceptance Criteria**:
- ✅ Single CMakeLists.txt builds on Win/Mac/Linux
- ✅ Optional `WITH_ML=ON` flag for Torch integration
- ✅ No hardcoded paths (use CMake variables)

**Effort Estimate**: 0 hours (already done, verify only)

---

### 7.2 Licensing

**REQ-NFR-002**: Open Source Compliance
**Priority**: P0 (Must-Have)
**Status**: To Document

**Description**: Verify all third-party code licenses are compatible.

**Deliverables**:
- `LICENSES.md`: JUCE (GPL/Commercial), libsamplerate (BSD), etc.
- Ensure DeepFilterNet2 model license allows redistribution

**Effort Estimate**: 5-10 hours (legal review)

---

## 8. Success Metrics

### 8.1 Quality Gates

**Release Criteria for P2 (95% Complete)**:
- ✅ All P0 requirements implemented
- ✅ 80%+ code coverage for DSP modules
- ✅ Zero critical bugs (crashes, memory leaks)
- ✅ <5% CPU usage on reference hardware (2015 MacBook Pro)
- ✅ Manual testing checklist passed (see SHIP_GATE_MANUAL_TEST.md)

### 8.2 Performance Benchmarks

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| DSP Unit Test Coverage | 80% | 0% | ❌ |
| CPU Usage (Release) | <5% | ~4% | ✅ |
| Memory Allocations (RT) | 0 | 0 | ✅ |
| Parameter Smoothing | Per-sample | Block-rate | ❌ |
| GUI Frame Rate | 30Hz | 30Hz | ✅ |

---

## 9. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Unit tests delay release | HIGH | MEDIUM | Start testing sprint immediately, parallelize with feature work |
| Undo/Redo complexity | MEDIUM | HIGH | Use JUCE's built-in undo manager (juce::UndoManager) |
| Oversampling refactor breaks DSP | MEDIUM | CRITICAL | A/B test with null tests, measure THD+N before/after |
| ML model licensing issues | LOW | HIGH | Clarify DeepFilterNet2 license early, prepare fallback |

---

## 10. Dependencies & Constraints

### External Dependencies:
- JUCE 7.0.12 (submodule)
- CMake 3.15+
- Optional: libtorch (if WITH_ML=ON)

### Timeline Constraints:
- P2 Sprint: 6-9 weeks (245-370 hours total)
- Must-Have features: 4-5 weeks
- Should-Have features: 2-3 weeks
- Nice-to-Have features: 1 week

### Resource Constraints:
- Single developer (adjust estimates accordingly)
- CI/CD infrastructure not yet set up

---

## 11. P2 Roadmap Summary

### Must-Have (Ship-Blocking):
1. ✅ DSP Unit Test Suite (AdvancedSaturation, EnhancedSPARK, LUFSMeter minimum)
2. ✅ Undo/Redo System
3. ✅ Sample-Accurate Parameter Smoothing
4. ✅ Memory Safety Verification

### Should-Have (Quality Critical):
5. ✅ MeterStrip Visualization
6. ✅ Preset Browser UI
7. ✅ DSP Module API Reference
8. ✅ Unified Oversampling Strategy
9. ✅ CI/CD Pipeline

### Nice-to-Have (Polish):
10. ✅ Architecture Documentation
11. ✅ Enhanced Tooltips
12. ✅ Platform-Specific Optimizations

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-15 | BTZ Team | Initial P2 requirements specification |

---

**Approval Status**: Draft → Review → Approved
**Next Steps**: Design Documentation (P2_DESIGN.md)
