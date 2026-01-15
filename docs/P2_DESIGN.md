# BTZ P2 Design Documentation

**Version**: 1.0
**Date**: 2026-01-15
**Status**: Draft
**Dependencies**: P2_REQUIREMENTS.md

---

## Table of Contents

1. [System Architecture Overview](#1-system-architecture-overview)
2. [DSP Unit Testing Framework](#2-dsp-unit-testing-framework)
3. [Undo/Redo System](#3-undoredo-system)
4. [Sample-Accurate Parameter Smoothing](#4-sample-accurate-parameter-smoothing)
5. [GUI Enhancements](#5-gui-enhancements)
6. [Performance Optimizations](#6-performance-optimizations)
7. [Documentation Infrastructure](#7-documentation-infrastructure)

---

## 1. System Architecture Overview

### 1.1 Signal Flow Diagram

```
Input Audio (Stereo)
    ↓
[PreGain Stage]
    ↓
[TransientShaper] ← Punch Parameter
    ↓
[AdvancedSaturation] ← Warmth/Drive Parameters
    ↓
[BoomProcessor] ← Boom Parameter
    ↓
[EnhancedSHINE] ← Adaptive EQ
    ↓
[EnhancedSPARK Limiter] ← Ceiling Parameter
    ↓
[Dry/Wet Mix] ← Mix Parameter
    ↓
[LUFSMeter] → GUI Display
    ↓
Output Audio (Stereo)
```

### 1.2 Threading Model

```
┌─────────────────┐
│  Audio Thread   │ (RT-Safe, lock-free)
│  processBlock() │
│  - DSP chain    │
│  - Atomic reads │
└────────┬────────┘
         │
         ↓ (atomic parameters)
┌────────┴────────┐
│  Message Thread │ (Non-RT)
│  - GUI updates  │
│  - File I/O     │
│  - Undo/Redo    │
└─────────────────┘
```

### 1.3 Module Dependency Graph

```
PluginProcessor
├── DSP/TransientShaper
├── DSP/AdvancedSaturation
├── DSP/BoomProcessor
├── DSP/EnhancedSHINE
│   ├── BarkBandAnalyzer
│   └── PsychoacousticWeighting
├── DSP/EnhancedSPARK
│   ├── JilesAthertonHysteresis
│   └── TruePeakDetector
├── DSP/LUFSMeter
│   ├── KWeightingFilter
│   └── ITU1770Gating
├── Parameters/PluginParameters (APVTS)
├── Utilities/ParameterSmoother
└── GUI/BTZPluginEditor
    ├── MainView
    ├── MeterStrip
    ├── PresetBrowser
    └── UndoRedoButtons
```

---

## 2. DSP Unit Testing Framework

### 2.1 Test Infrastructure Design

**Goal**: Verify all DSP modules produce expected outputs given known inputs.

#### 2.1.1 Directory Structure

```
BTZ_JUCE/tests/
├── CMakeLists.txt (add unit tests)
├── dsp/
│   ├── saturation_unit_test.cpp
│   ├── spark_unit_test.cpp
│   ├── shine_unit_test.cpp
│   ├── lufs_unit_test.cpp
│   ├── transient_shaper_unit_test.cpp
│   └── parameter_smoother_unit_test.cpp
├── fixtures/
│   ├── sine_440hz_1s.wav (test signals)
│   ├── drum_loop_4bar.wav
│   └── white_noise_1s.wav
└── utils/
    ├── dsp_test_utils.h (common assertions)
    └── signal_generator.h (test signal creation)
```

#### 2.1.2 Test Framework Choice

**Selected**: Google Test (gtest)
**Rationale**: Industry standard, CMake integration, mocking support

**CMakeLists.txt additions**:
```cmake
# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Add unit tests
add_executable(BTZUnitTests
    tests/dsp/saturation_unit_test.cpp
    tests/dsp/spark_unit_test.cpp
    # ... more tests
)
target_link_libraries(BTZUnitTests PRIVATE gtest_main BTZ_SharedCode)
add_test(NAME BTZUnitTests COMMAND BTZUnitTests)
```

### 2.2 AdvancedSaturation Unit Tests

**File**: `tests/dsp/saturation_unit_test.cpp`

#### Test Cases:

1. **test_bypass_mode**: Input == Output when saturation amount = 0
2. **test_soft_clipping_symmetry**: Positive/negative samples produce symmetric output
3. **test_hard_clipping_limits**: Output clamped to [-1, +1]
4. **test_tube_saturation_harmonics**: Verify even-order harmonics dominant
5. **test_tape_saturation_compression**: Verify dynamic range reduction
6. **test_transformer_saturation_warmth**: Verify frequency response tilt
7. **test_extreme_values**: Handle NaN, Inf gracefully
8. **test_dc_offset_removal**: DC component <0.001 after saturation

**Example Test**:
```cpp
TEST(AdvancedSaturationTest, BypassMode) {
    AdvancedSaturation sat;
    sat.prepare(48000.0);
    sat.setAmount(0.0f); // Bypass

    // Generate sine wave
    std::vector<float> input = generateSine(440.0f, 1.0f, 48000, 1.0f);
    std::vector<float> output(input.size());

    // Process
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = sat.processSample(input[i], 0);
    }

    // Assert: output == input (tolerance: 0.0001)
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_NEAR(output[i], input[i], 0.0001f);
    }
}
```

### 2.3 EnhancedSPARK Unit Tests

**File**: `tests/dsp/spark_unit_test.cpp`

#### Critical Test Cases:

1. **test_true_peak_detection**: Verify ITU-R BS.1770-4 compliance (4x oversampling)
2. **test_hysteresis_model**: Jiles-Atherton equations produce expected saturation
3. **test_lookahead_delay**: Limiter introduces exactly 64-sample delay
4. **test_gain_reduction_ballistics**: Attack <1ms, release user-controlled
5. **test_ceiling_accuracy**: Output never exceeds ceiling threshold
6. **test_zero_latency_bypass**: When disabled, no processing delay
7. **test_stereo_linking**: L/R channels limited together (no stereo widening)
8. **test_inter_sample_peaks**: Catch peaks between samples via oversampling
9. **test_extreme_transients**: Handle drum hits >+10dBFS input
10. **test_continuous_limiting**: Sustain limiting without pumping

**Example Test** (True Peak Detection):
```cpp
TEST(EnhancedSPARKTest, TruePeakDetection) {
    EnhancedSPARK spark;
    spark.prepare(48000.0);
    spark.setEnabled(true);
    spark.setCeiling(-0.1f); // -0.1 dBFS ceiling

    // Generate inter-sample peak signal (classic test case)
    // Full-scale square wave → peaks occur between samples
    std::vector<float> input = generateSquareWave(1000.0f, 48000, 1.0f);
    std::vector<float> output(input.size());

    // Process
    float maxTruePeak = 0.0f;
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = spark.processSample(input[i], 0);
        maxTruePeak = std::max(maxTruePeak, spark.getTruePeakLevel());
    }

    // Assert: true peak detected > sample peak (due to oversampling)
    float maxSamplePeak = *std::max_element(input.begin(), input.end());
    EXPECT_GT(maxTruePeak, maxSamplePeak);

    // Assert: output never exceeds ceiling (-0.1 dBFS = 0.989)
    for (float sample : output) {
        EXPECT_LE(std::abs(sample), 0.989f);
    }
}
```

### 2.4 LUFSMeter Unit Tests

**File**: `tests/dsp/lufs_unit_test.cpp`

#### Test Cases:

1. **test_k_weighting_filter**: Verify frequency response matches ITU curve
2. **test_400ms_gating**: Blocks below -70 LUFS excluded
3. **test_relative_gating**: Blocks <10 LU below average excluded
4. **test_known_signal_lufs**: EBU test signals produce expected LUFS values
5. **test_pink_noise_lufs**: Pink noise @ -20dBFS → ~-20 LUFS
6. **test_momentary_loudness**: 400ms window updates at 100ms intervals
7. **test_short_term_loudness**: 3s window
8. **test_integrated_loudness**: Full program loudness
9. **test_reset_measurements**: Clear history on reset
10. **test_stereo_correlation**: L/R correlation in [-1, +1]

**Reference Signals**:
- EBU Tech 3341: Known LUFS test signals
- Pink noise: Predictable spectral content

### 2.5 Test Automation

**CI/CD Integration** (GitHub Actions):

```yaml
name: Unit Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]

    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive

    - name: Configure CMake
      run: cmake -B build -DBUILD_TESTS=ON

    - name: Build Tests
      run: cmake --build build --config Release

    - name: Run Tests
      run: ctest --test-dir build --output-on-failure

    - name: Generate Coverage (Linux only)
      if: matrix.os == 'ubuntu-latest'
      run: |
        lcov --capture --directory build --output-file coverage.info
        lcov --list coverage.info
```

---

## 3. Undo/Redo System

### 3.1 Architecture

**Pattern**: Command Pattern with State Snapshots

```
┌─────────────────────────────────────────────┐
│          UndoRedoManager                    │
│                                             │
│  - std::vector<StateSnapshot> undoStack    │
│  - std::vector<StateSnapshot> redoStack    │
│  - int maxStackSize = 100                  │
│                                             │
│  + pushState(StateSnapshot)                │
│  + undo() → bool                           │
│  + redo() → bool                           │
│  + canUndo() → bool                        │
│  + canRedo() → bool                        │
└─────────────────────────────────────────────┘
         ↓
┌─────────────────────────────────────────────┐
│        StateSnapshot (struct)               │
│                                             │
│  - juce::MemoryBlock data (APVTS state)   │
│  - juce::String description                │
│  - int64_t timestamp                       │
│                                             │
│  + serialize(APVTS) → MemoryBlock          │
│  + deserialize(MemoryBlock) → APVTS values │
└─────────────────────────────────────────────┘
```

### 3.2 Implementation Details

#### 3.2.1 UndoRedoManager Class

**File**: `BTZ_JUCE/Source/Utilities/UndoRedoManager.h`

```cpp
class UndoRedoManager {
public:
    struct StateSnapshot {
        juce::MemoryBlock data;
        juce::String description;
        int64_t timestamp;
    };

    UndoRedoManager(juce::AudioProcessorValueTreeState& apvts);

    void pushState(const juce::String& description);
    bool undo();
    bool redo();

    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }

    void clearHistory();

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<StateSnapshot> undoStack;
    std::vector<StateSnapshot> redoStack;
    static constexpr int MAX_STACK_SIZE = 100;

    StateSnapshot captureCurrentState();
    void restoreState(const StateSnapshot& snapshot);
};
```

#### 3.2.2 State Serialization

**Strategy**: Use APVTS's built-in state serialization

```cpp
StateSnapshot UndoRedoManager::captureCurrentState() {
    StateSnapshot snapshot;

    // Serialize APVTS state tree
    auto stateTree = apvts.copyState();
    juce::MemoryOutputStream stream(snapshot.data, false);
    stateTree.writeToStream(stream);

    snapshot.timestamp = juce::Time::currentTimeMillis();
    return snapshot;
}

void UndoRedoManager::restoreState(const StateSnapshot& snapshot) {
    juce::MemoryInputStream stream(snapshot.data, false);
    auto stateTree = juce::ValueTree::readFromStream(stream);

    // Restore APVTS from snapshot
    apvts.replaceState(stateTree);
}
```

#### 3.2.3 Integration with GUI

**File**: `BTZ_JUCE/Source/GUI/MainView.cpp`

Add undo/redo buttons:

```cpp
class MainView : public juce::Component {
public:
    void createUndoRedoButtons() {
        undoButton = std::make_unique<juce::TextButton>("Undo");
        redoButton = std::make_unique<juce::TextButton>("Redo");

        undoButton->onClick = [this]() {
            if (undoRedoManager->undo()) {
                updateAllControls();
            }
        };

        redoButton->onClick = [this]() {
            if (undoRedoManager->redo()) {
                updateAllControls();
            }
        };

        // Keyboard shortcuts
        undoButton->addShortcut(juce::KeyPress('z', juce::ModifierKeys::commandModifier, 0));
        redoButton->addShortcut(juce::KeyPress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0));
    }

    void timerCallback() override {
        // Update button enabled state
        undoButton->setEnabled(undoRedoManager->canUndo());
        redoButton->setEnabled(undoRedoManager->canRedo());
    }

private:
    std::unique_ptr<UndoRedoManager> undoRedoManager;
    std::unique_ptr<juce::TextButton> undoButton, redoButton;
};
```

### 3.3 Performance Considerations

- **Snapshot Size**: APVTS state ≈ 1KB (28 parameters × 4 bytes each + tree overhead)
- **Memory Footprint**: 100 snapshots × 1KB = 100KB (negligible)
- **Serialization Time**: <0.1ms (non-RT thread only)
- **Stack Management**: Circular buffer overwrites oldest when >100

---

## 4. Sample-Accurate Parameter Smoothing

### 4.1 Current Implementation (Block-Rate)

**Problem**:
```cpp
// processBlock() - current approach
void BTZAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    // Update smoothers ONCE per block
    punchSmoother.setTarget(apvts.getRawParameterValue("punch")->load());
    warmthSmoother.setTarget(apvts.getRawParameterValue("warmth")->load());

    // Process entire block with single smoothed value
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float smoothedPunch = punchSmoother.getNextValue(); // ← Same value for all 512 samples
        // Process with smoothedPunch...
    }
}
```

**Result**: At 512-sample buffers, parameter updates happen every 10.6ms @ 48kHz → audible stepping on fast automation.

### 4.2 Proposed Implementation (Per-Sample)

**Solution**: Interpolate parameter within buffer

```cpp
class ParameterSmoother {
public:
    void prepare(double sampleRate, float rampTimeSeconds = 0.02f) {
        this->sampleRate = sampleRate;
        smoothingSamples = static_cast<int>(sampleRate * rampTimeSeconds);
        currentValue = targetValue = 0.0f;
        samplesRemaining = 0;
    }

    void setTarget(float newTarget) {
        if (std::abs(newTarget - targetValue) > 0.0001f) {
            targetValue = newTarget;
            samplesRemaining = smoothingSamples;
        }
    }

    float getNextValue() {
        if (samplesRemaining > 0) {
            // Linear interpolation (can upgrade to exponential)
            float increment = (targetValue - currentValue) / samplesRemaining;
            currentValue += increment;
            --samplesRemaining;
        } else {
            currentValue = targetValue;
        }
        return currentValue;
    }

    bool isSmoothing() const { return samplesRemaining > 0; }

private:
    double sampleRate = 48000.0;
    int smoothingSamples = 960; // 20ms @ 48kHz
    float currentValue = 0.0f;
    float targetValue = 0.0f;
    int samplesRemaining = 0;
};
```

**Usage**:
```cpp
void BTZAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    // Set targets ONCE per block
    punchSmoother.setTarget(apvts.getRawParameterValue("punch")->load());

    // Process sample-by-sample
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float smoothedPunch = punchSmoother.getNextValue(); // ← Unique value per sample
        // Process with smoothedPunch...
    }
}
```

### 4.3 CPU Impact Analysis

**Overhead**: 1 multiply + 1 add + 1 decrement per parameter per sample
- 5 smoothed parameters × 512 samples = 2,560 operations per block
- Modern CPU: ~0.01% overhead (negligible)

**Benefit**: Eliminates zipper noise, enables responsive automation

---

## 5. GUI Enhancements

### 5.1 MeterStrip Visualization

**File**: `BTZ_JUCE/Source/GUI/MeterStrip.cpp`

#### 5.1.1 Layout Design

```
┌──────────────────────────┐
│   LUFS Meter             │
│   ▓▓▓▓▓▓▓▓▓░░░░░░        │ (green/yellow/red zones)
│   -23 LUFS               │
│                          │
│   True Peak              │
│   ▓▓▓▓▓▓▓▓▓▓▓░░░         │ (red at 0dBFS)
│   -0.5 dBFS              │
│                          │
│   Gain Reduction         │
│   ░░░░▓▓▓░░░░░░░         │ (needle visualization)
│   3.2 dB                 │
│                          │
│   Stereo Correlation     │
│   ━━━━━●━━━━━            │ (-1 ← 0 → +1)
│   0.85                   │
└──────────────────────────┘
```

#### 5.1.2 Implementation

```cpp
void MeterStrip::paint(Graphics& g) {
    // LUFS meter with color zones
    float lufs = processor->getCurrentLUFS();
    Colour lufsColor = getLUFSColor(lufs);

    Rectangle<float> lufsBounds = getLUFSMeterBounds();
    float normalizedLUFS = jmap(lufs, -50.0f, 0.0f, 0.0f, 1.0f);

    g.setColour(lufsColor);
    g.fillRect(lufsBounds.removeFromLeft(lufsBounds.getWidth() * normalizedLUFS));

    // True peak with clip indicator
    float peak = processor->getCurrentPeak();
    if (peak >= 0.0f) {
        flashClipIndicator = true; // Red flash
    }

    // Gain reduction needle
    float gr = processor->getGainReduction();
    drawGainReductionNeedle(g, gr);

    // Stereo correlation graph
    float stereo = processor->getStereoCorrelation();
    drawStereoCorrelation(g, stereo);
}

Colour MeterStrip::getLUFSColor(float lufs) {
    if (lufs < -23.0f) return Colours::green;
    if (lufs < -14.0f) return Colours::yellow;
    return Colours::red;
}
```

### 5.2 Preset Browser

**File**: `BTZ_JUCE/Source/GUI/PresetBrowser.h`

#### 5.2.1 Modal Dialog Design

```
┌─────────────────────────────────────────────┐
│  BTZ Preset Browser                    [×]  │
├─────────────────────────────────────────────┤
│  Search: [________________]  [Factory] [User]│
├────────┬────────────────────────────────────┤
│ Category│  Preset Name                 Tags │
├────────┼────────────────────────────────────┤
│ Factory│  ▶ Gentle Glue          Drums, Safe│
│        │    Pop Punch         Drums, Modern │
│        │    Modern Master          Master, 3│
│        │    Vintage Vibe      Drums, Warm, 4│
│        │    Dynamic Enhancer       Transient│
│ User   │  ▶ My Custom         Drums, Saved  │
└────────┴────────────────────────────────────┘
│  [Load] [Save As...] [Delete] [Cancel]     │
└─────────────────────────────────────────────┘
```

#### 5.2.2 Implementation

```cpp
class PresetBrowser : public juce::Component,
                      public juce::ListBoxModel {
public:
    PresetBrowser(PresetManager& presetManager);

    // ListBoxModel interface
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, Graphics& g,
                         int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const MouseEvent& e) override;

    void showDialog();

private:
    PresetManager& presetManager;
    juce::ListBox presetList;
    juce::TextEditor searchBox;
    juce::TextButton loadButton, saveButton, deleteButton;

    std::vector<PresetManager::Preset> filteredPresets;

    void filterPresets(const juce::String& searchTerm);
    void loadSelectedPreset();
    void saveCurrentPreset();
};
```

---

## 6. Performance Optimizations

### 6.1 Unified Oversampling Strategy

**Current Issue**: Two cascaded oversampling stages

```
Audio Input (48kHz)
    ↓
OversamplingProcessor (2x → 96kHz)
    ├─ TransientShaper
    └─ AdvancedSaturation
    ↓ (downsample to 48kHz)
EnhancedSPARK (internal 4x → 192kHz)
    └─ Limiter processing
    ↓ (downsample to 48kHz)
Output
```

**Problem**: Double conversion overhead

### 6.2 Proposed Unified Strategy

```
Audio Input (48kHz)
    ↓
Unified Upsampler (4x → 192kHz) ← Single upsampling stage
    ├─ TransientShaper
    ├─ AdvancedSaturation
    └─ EnhancedSPARK (no internal oversampling)
    ↓
Unified Downsampler (192kHz → 48kHz) ← Single downsampling stage
    ↓
Output (48kHz)
```

**Implementation**:

1. Move oversampling to PluginProcessor level
2. Remove internal oversampling from EnhancedSPARK
3. All nonlinear modules process at 4x sample rate
4. Use JUCE's `dsp::Oversampling<float>` class

```cpp
class BTZAudioProcessor {
private:
    juce::dsp::Oversampling<float> oversampler{2, 2, // 2 channels, factor 4 (2^2)
                                                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR};

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) override {
        // Upsample
        auto oversampledBlock = oversampler.processSamplesUp(buffer);

        // Process all DSP at 4x rate
        transientShaper.process(oversampledBlock);
        saturation.process(oversampledBlock);
        sparkLimiter.process(oversampledBlock); // ← No internal oversampling

        // Downsample
        oversampler.processSamplesDown(buffer);
    }
};
```

**Expected Gain**: 15-20% CPU reduction

---

## 7. Documentation Infrastructure

### 7.1 Architecture Documentation

**File**: `BTZ_JUCE/ARCHITECTURE.md`

#### Sections:

1. **System Overview**: High-level signal flow
2. **Module Descriptions**: Each DSP module's purpose
3. **Threading Model**: RT vs non-RT threads
4. **Parameter Flow**: APVTS → Smoothing → DSP
5. **State Management**: Preset system, undo/redo
6. **Testing Strategy**: Unit/integration tests

### 7.2 Parameter Reference

**File**: `BTZ_JUCE/PARAMETERS.md`

#### Format (per parameter):

```markdown
### Punch (ID: "punch")

**Range**: 0.0 → 1.0
**Default**: 0.0
**Unit**: Normalized
**DSP Module**: TransientShaper

**Description**: Controls transient enhancement on drum hits. Higher values increase attack sharpness.

**Interactions**:
- + Drive → Aggressive transient + saturation (use sparingly)
- + Warmth → Balanced punch with analog character

**Recommended Values**:
- Gentle: 0.2-0.4
- Modern Pop: 0.5-0.7
- Extreme: 0.8-1.0

**Technical Details**: Applies ADAA (anti-derivative anti-aliasing) to envelope follower for artifact-free transient shaping.
```

### 7.3 Doxygen Configuration

**File**: `BTZ_JUCE/Doxyfile`

```
PROJECT_NAME           = "BTZ - The Box Tone Zone"
INPUT                  = Source/
RECURSIVE              = YES
GENERATE_HTML          = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
HAVE_DOT               = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
```

**Generate docs**:
```bash
cd BTZ_JUCE
doxygen Doxyfile
open docs/html/index.html
```

---

## 8. Implementation Priorities

### Phase 1 (Must-Have) - 4-5 weeks

1. **DSP Unit Tests** (80-120 hrs)
   - AdvancedSaturation, EnhancedSPARK, LUFSMeter (minimum)
   - CI/CD pipeline setup
2. **Undo/Redo System** (30-50 hrs)
   - UndoRedoManager class
   - GUI integration
3. **Sample-Accurate Smoothing** (20-30 hrs)
   - Per-sample parameter interpolation
   - Verify no CPU increase

### Phase 2 (Should-Have) - 2-3 weeks

4. **MeterStrip Visualization** (20-30 hrs)
5. **Preset Browser UI** (30-40 hrs)
6. **DSP Module Documentation** (15-20 hrs)
7. **Unified Oversampling** (30-50 hrs)

### Phase 3 (Nice-to-Have) - 1 week

8. **Architecture Documentation** (25-35 hrs)
9. **Enhanced Tooltips** (10-15 hrs)
10. **Platform Optimizations** (15-25 hrs)

---

## 9. Risk Mitigation

| Risk | Mitigation |
|------|------------|
| Tests delay release | Start testing sprint immediately, parallelize |
| Undo/Redo complexity | Use JUCE's `UndoManager` as base |
| Oversampling breaks DSP | A/B null test, measure THD+N before/after |
| Sample-accurate smoothing CPU | Profile early, fallback to block-rate if needed |

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-15 | BTZ Team | Initial P2 design specification |

---

**Next Steps**: Task Breakdown (P2_TASK_BREAKDOWN.md)
