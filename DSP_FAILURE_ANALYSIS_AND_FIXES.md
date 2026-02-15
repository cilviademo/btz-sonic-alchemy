# BTZ DSP FAILURE ANALYSIS & SYSTEMATIC REPAIR

**Date:** 2026-02-15
**Implementation:** `btz-sonic-alchemy-main/BTZ/Source/PluginProcessor.cpp`
**Severity:** CRITICAL - Multiple catastrophic DSP failures rendering plugin unusable

---

## 🚨 EXECUTIVE SUMMARY

**Status:** Plugin is fundamentally broken at the DSP architecture level.

**Root Causes:**
1. **Parameter routing failure** - "Master" parameter scales OTHER parameters instead of output
2. **Broken mixing law** - Linear crossfade causes phase cancellation
3. **Excessive saturation stacking** - 4+ saturation stages with no headroom management
4. **Missing/non-functional features** - 9 of 16 parameters don't work or work incorrectly
5. **No makeup gain compensation** - Glue compressor causes 6dB volume drop
6. **Incorrect M/S matrix** - Width processing uses wrong coefficients
7. **Hard clipping everywhere** - No proper limiting, only clipping

---

## 📊 PARAMETER AUDIT - DEAD/BROKEN CONTROLS

### ✅ Parameters That Work (Partially)
| Parameter | Status | Issues |
|-----------|--------|--------|
| Punch | Partial | Hard clips, smears transients, no proper transient design |
| Warmth | Partial | Hard clips, excessive saturation, distorts low-end |
| Glue | Broken | Acts like limiter, ducks 6dB, no makeup gain |
| Air | Partial | Barely works, removes mid/low frequencies |
| Width | Broken | Phase issues, incorrect M/S matrix coefficients |
| Density | Broken | Yet another saturation stage, crushes dynamics |
| Mix | BROKEN | Phase cancellation at all values except 0%/100% |
| Drive | Partial | Only increases volume, hard clips transients |

### ❌ Parameters That Don't Work
| Parameter | Line | Root Cause |
|-----------|------|------------|
| **Boom** | 347-350 | Only adds 28% of low band, no sub-harmonic generation |
| **Motion** | 378-392 | Just adds white noise, not modulation/panning |
| **Era** | Used in line 251 | Only scales warmth saturation slightly, imperceptible |
| **Master** | 244-246 | **CRITICAL BUG**: Scales ALL parameters instead of output volume |
| **TP Ceil** | 360 | Converted to linear but sparkMix=0 by default, no effect |
| **Spark Mix** | 366-368 | Hard clipper with parallel blend, not a limiter |
| **Shine** | 337 | Barely adds high-end, mixed with Air incorrectly |
| **Shine Mix** | 337 | Combined with Air, not separate control |
| **Autogain** | 525-543 | Works but only matches RMS, user perceives as "barely doing anything" |

---

## 🔴 CRITICAL BUG #1: MASTER PARAMETER DISASTER

**Location:** Lines 244-246

```cpp
const float masterScale = juce::jlimit(0.25f, 1.25f, 0.7f + master * 0.6f);
punch *= masterScale; warmth *= masterScale; boom *= masterScale;
glue *= masterScale; air *= masterScale; density *= masterScale;
```

**Problem:**
- Master parameter SCALES all other parameters instead of controlling output volume
- When master=0, punch/warmth/boom etc. become 0.7x their value
- When master=1, they become 1.3x their value
- This is completely wrong and breaks all parameter behavior

**User Impact:**
> "Master: Doesn't do anything; needs to be tied to master volume overall and not 'master' plugin volume for reference."

**Fix Required:**
- Remove master scaling of parameters entirely
- Apply master as output gain at the END of processing chain
- Rename to "Output" or remove entirely (use DAW fader instead)

---

## 🔴 CRITICAL BUG #2: MIX KNOB PHASE CANCELLATION

**Location:** Lines 401-406

```cpp
if (n < maxPreparedBlockSize) {
    const float dryL = dryBuffer.getSample(0, n);
    const float dryR = dryBuffer.getSample(1, n);
    L = dryL + (L - dryL) * mix;  // ❌ LINEAR CROSSFADE
    R = dryR + (R - dryR) * mix;
}
```

**Problems:**
1. **Linear crossfade** - Should be equal-power (cosine/sine law)
2. **No time alignment** - Dry and wet are processed differently (oversampling delays wet)
3. **No latency compensation** - Causes comb filtering at intermediate mix values

**User Impact:**
> "Mix: phase/ringing sounds and issues if not on 0% or 100% for all knobs and iterations"

**Mathematical Error:**
- Linear: `out = dry + (wet - dry) * mix`
- Correct: `out = dry * cos(mix * π/2) + wet * sin(mix * π/2)`

**Fix Required:**
```cpp
// Equal-power crossfade with latency compensation
const float mixAngle = mix * juce::MathConstants<float>::halfPi;
const float dryGain = std::cos(mixAngle);
const float wetGain = std::sin(mixAngle);

// Compensate for oversampling latency if active
float dryL_compensated, dryR_compensated;
if (activeQualityMode > 0) {
    // Delay dry signal to match wet processing latency
    int latencySamples = getLatencySamples();
    dryL_compensated = dryDelayLine[0].read(latencySamples);
    dryR_compensated = dryDelayLine[1].read(latencySamples);
} else {
    dryL_compensated = dryL;
    dryR_compensated = dryR;
}

L = dryL_compensated * dryGain + L * wetGain;
R = dryR_compensated * dryGain + R * wetGain;
```

---

## 🔴 CRITICAL BUG #3: GLUE COMPRESSOR VOLUME DROP

**Location:** Lines 302-319

```cpp
if (glue > 0.01f) {
    const float threshold = juce::Decibels::decibelsToGain(-8.0f - glue * 10.0f);  // ❌ -8 to -18 dB
    const float ratio = 2.0f + glue * 5.0f;  // ❌ 2:1 to 7:1
    const float sidechain = juce::jmax(std::abs(L), std::abs(R));
    const float envVal = glueEnv.process(sidechain);

    float gainReduction = 1.0f;
    if (envVal > threshold) {
        const float overDb = juce::Decibels::gainToDecibels(envVal / threshold, -100.0f);
        const float reducedDb = overDb * (1.0f - 1.0f / ratio);
        gainReduction = juce::Decibels::decibelsToGain(-reducedDb);
    }

    const float smoothCoeff = gainReduction < glueGain ? 0.02f : 0.002f;
    glueGain += smoothCoeff * (gainReduction - glueGain);
    L *= glueGain;  // ❌ NO MAKEUP GAIN
    R *= glueGain;
}
```

**Problems:**
1. **No makeup gain compensation** - Gain reduction applied with no recovery
2. **Threshold too low** - -8dB to -18dB for mastering is absurdly aggressive
3. **Ratio too high** - 7:1 at 100% is limiting, not glue compression
4. **Envelope too slow** - 0.02/0.002 coefficients cause pumping

**User Impact:**
> "Glue: volume ducks nearly 6db... it's mainly due to perceived togetherness from volume reduction and not actually sonically gluing"

**Fix Required:**
```cpp
// Proper glue compressor with makeup gain
if (glue > 0.01f) {
    const float threshold = juce::Decibels::decibelsToGain(-18.0f + glue * 6.0f); // -18 to -12 dB
    const float ratio = 2.0f + glue * 2.0f;  // 2:1 to 4:1 (gentle)
    const float sidechain = juce::jmax(std::abs(L), std::abs(R));
    const float envVal = glueEnv.process(sidechain);

    float gainReductionDb = 0.0f;
    if (envVal > threshold) {
        const float overDb = juce::Decibels::gainToDecibels(envVal / threshold, -100.0f);
        gainReductionDb = overDb * (1.0f - 1.0f / ratio);
    }

    const float gainReduction = juce::Decibels::decibelsToGain(-gainReductionDb);
    const float smoothCoeff = gainReduction < glueGain ? 0.1f : 0.01f; // Faster attack/release
    glueGain += smoothCoeff * (gainReduction - glueGain);

    // Apply compression
    L *= glueGain;
    R *= glueGain;

    // MAKEUP GAIN - compensate for average GR
    const float makeupGain = juce::Decibels::decibelsToGain(gainReductionDb * 0.7f); // Recover 70% of GR
    L *= makeupGain;
    R *= makeupGain;

    // Update GR meter
    sparkGrInst = juce::jmax(sparkGrInst, gainReductionDb); // Show actual GR
}
```

---

## 🔴 CRITICAL BUG #4: WIDTH M/S MATRIX INCORRECT

**Location:** Lines 322-334

```cpp
const float mid = 0.5f * (L + R);      // ❌ Should be 1/√2
const float side = 0.5f * (L - R);     // ❌ Should be 1/√2
const float widthScale = width * 2.0f; // ❌ Can make side 2x louder

sideLowState += sideLowCoeff * (side - sideLowState);
const float sideLow = sideLowState;
const float sideHigh = side - sideLow;
const float lowBandWidth = juce::jmin(widthScale, 1.0f);
const float sideOut = sideLow * lowBandWidth + sideHigh * widthScale;

L = mid + sideOut;  // ❌ Incorrect reconstruction
R = mid - sideOut;
```

**Problems:**
1. **Wrong M/S matrix coefficients** - Using 0.5 instead of 1/√2 (0.7071)
2. **Excessive width scaling** - 2x side gain causes phase cancellation in mono
3. **No energy preservation** - Total power changes with width adjustment

**User Impact:**
> "Width: Sounds too lifeless at 0% (mono interpretation, but not truly mono)... width at 100% gives better perceived stereo imaging, but introduces a lot of phase issues and also makes all mono information stereo"

**Fix Required:**
```cpp
// Proper M/S encoding with correct coefficients
constexpr float kMSScalar = 0.7071067811865476f; // 1/√2
const float mid = (L + R) * kMSScalar;
const float side = (L - R) * kMSScalar;

// Mono-safe width control
const float widthControl = width * 2.0f - 1.0f; // -1 (mono) to +1 (wide)

// Low-end mono collapse (< 120 Hz always mono)
sideLowState += sideLowCoeff * (side - sideLowState);
const float sideLow = sideLowState * 0.0f; // Force mono below crossover
const float sideHigh = side - sideLowState;

// Apply width only to high frequencies
const float sideOut = sideLow + sideHigh * juce::jlimit(0.0f, 2.0f, 1.0f + widthControl);

// Proper M/S decoding
L = (mid + sideOut) * kMSScalar;
R = (mid - sideOut) * kMSScalar;
```

---

## 🔴 CRITICAL BUG #5: EXCESSIVE SATURATION STACKING

**Saturation Stages Found:**
1. **Warmth saturation** (lines 249-261) - `drv = 1.0f + warmth * 2.8f`
2. **Multiband saturation** (lines 266-283) - Separate low/high band tanh
3. **Punch saturation** (lines 285-299) - Odd/even harmonics
4. **Density saturation** (lines 352-356) - `drv = 1.0f + density * 3.0f`

**Problem:**
- 4 CASCADED SATURATION STAGES with no headroom management
- Each stage can add up to 4x gain before clipping
- Total potential gain before clipping: **up to 33x** (30 dB!)
- No inter-stage gain compensation

**User Impact:**
> "Punch at 100% with quality at 0 sounds hard clipped and then volume drastically limited to sound brickwall/squashed"

**Fix Required:**
- Add -6dB pad BEFORE each nonlinear stage
- Add +6dB makeup AFTER each stage
- Limit total drive to prevent cascading distortion
- Apply oversampling AROUND nonlinear blocks, not globally

---

## 🔴 CRITICAL BUG #6: BOOM DOESN'T WORK

**Location:** Lines 347-350

```cpp
if (boom > 0.01f) {
    L += xoverLowL * boom * 0.28f;  // ❌ Just adds 28% of low band
    R += xoverLowR * boom * 0.28f;
}
```

**Problem:**
- Only adds 28% of existing low-frequency content
- No sub-harmonic generation
- No psychoacoustic enhancement
- Completely underwhelming effect

**User Impact:**
> "Boom: Doesn't work at all"

**Fix Required:**
```cpp
// Proper sub-harmonic synthesis
if (boom > 0.01f) {
    // Generate octave-down sub-harmonic
    const float subL = generateSubHarmonic(xoverLowL, 0.5f); // -1 octave
    const float subR = generateSubHarmonic(xoverLowR, 0.5f);

    // Blend with original low-end
    const float boomAmount = boom * 0.5f;
    L += (xoverLowL * 0.3f + subL * 0.7f) * boomAmount;
    R += (xoverLowR * 0.3f + subR * 0.7f) * boomAmount;

    // Optional: Add psychoacoustic bass enhancement
    const float bassPunch = fastTanh(xoverLowL * 2.0f) * 0.1f * boom;
    L += bassPunch;
    R += bassPunch;
}
```

---

## 🔴 CRITICAL BUG #7: SPARK LIMITER IS HARD CLIPPER

**Location:** Lines 358-373

```cpp
const float ceilLin = juce::Decibels::decibelsToGain(ceilDb);
const float absL = std::abs(L);
const float absR = std::abs(R);

if (absL > ceilLin)
    L = ((L > 0.0f ? ceilLin : -ceilLin) * sparkMix) + L * (1.0f - sparkMix);  // ❌ HARD CLIP
if (absR > ceilLin)
    R = ((R > 0.0f ? ceilLin : -ceilLin) * sparkMix) + R * (1.0f - sparkMix);
```

**Problems:**
1. **Hard clipping** - No soft knee, no lookahead, instant brick wall
2. **No true-peak limiting** - Claims to be TP limiter but isn't
3. **Parallel blend AFTER clipping** - Doesn't prevent clipping, just blends it
4. **GR meter calculation broken** - Only shows instantaneous values

**User Impact:**
> "TP Ceil: doesn't do anything"
> "Spark Mix: does not do anything"

**Fix Required:**
- Implement proper lookahead limiter (5ms minimum)
- Use soft knee (0.5-1dB below ceiling)
- Apply oversampling for true-peak detection
- Fix GR meter to show actual limiting action

---

## 🔴 CRITICAL BUG #8: MOTION IS WHITE NOISE

**Location:** Lines 378-392

```cpp
if (motion > 0.01f) {
    // ... LCG random number generator
    noiseSeed = kLcgMultiplier * noiseSeed + kLcgIncrement;
    float white = static_cast<float>((noiseSeed >> 9) & 0x7FFFFF) / kNoiseScale - 0.5f;
    const float noiseLevel = kNoiseBaseLevel * motion * kNoiseGain / juce::jmax(1.0f, osFactor);
    L += white * noiseLevel;  // ❌ Just adding white noise
    // ... same for R
}
```

**Problem:**
- Motion parameter literally just adds white noise
- No actual motion/modulation/panning algorithm
- Not a realistic IR or panning effect

**User Impact:**
> "Motion: Doesn't work, maybe subtly at 100%, but only small like a phasey small room reverb and not a realistic IR/panning algorithm"

**Fix Required:**
- Remove white noise implementation
- Implement proper stereo widening modulation (LFO-based)
- OR implement auto-panning algorithm
- OR remove parameter entirely

---

## 🔴 CRITICAL BUG #9: QUALITY MODES BROKEN

**User Report:**
> "Quality: 0 sounds horrible, like if 'lo-fi' was digitally replicated and it sounds atrocious, remove this all together, 1 meets somewhere in the middle of trying to sound vintage but introduces a lot of phasing/reverb-like issues"

**Analysis:**
- Quality 0 = No oversampling (lines 520-521)
- Quality 1 = 2x oversampling (lines 508-513)
- Quality 2 = 4x oversampling (lines 514-519)

**Problem:**
The quality modes themselves are correctly implemented, BUT:
1. At Quality 0, the **excessive saturation stacking** causes severe aliasing
2. The **lack of headroom** means saturation stages clip and fold back
3. The **broken mix crossfade** adds phase distortion
4. Combined effect sounds "lo-fi digitally replicated"

**Fix Required:**
- Quality 0 should STILL oversample the saturation stages (just at lower rates)
- Reduce saturation drive amounts at Quality 0
- OR remove Quality 0 entirely, make Quality 1 the minimum (rename to Draft/Good/Best)

---

## 📋 COMPLETE FIX CHECKLIST

### Immediate Critical Fixes (Ship Blockers)

- [ ] **FIX #1**: Remove master parameter scaling of other parameters
- [ ] **FIX #2**: Implement equal-power mix crossfade with latency compensation
- [ ] **FIX #3**: Add makeup gain to Glue compressor
- [ ] **FIX #4**: Fix Width M/S matrix coefficients
- [ ] **FIX #5**: Add inter-stage headroom management (-6dB pad between saturators)
- [ ] **FIX #6**: Implement proper Boom sub-harmonic synthesis
- [ ] **FIX #7**: Replace SPARK hard clipper with proper lookahead limiter
- [ ] **FIX #8**: Remove/replace Motion white noise with actual modulation
- [ ] **FIX #9**: Fix or remove Quality mode 0

### Secondary Fixes (Usability)

- [ ] Fix Era parameter (currently imperceptible)
- [ ] Fix Shine/Shine Mix separation (currently combined with Air)
- [ ] Fix GR meter display (currently doesn't move)
- [ ] Add oversampling UI control (code exists but no GUI button)
- [ ] Fix duplicate bypass controls in GUI
- [ ] Improve Autogain perceptual impact

### Architecture Improvements

- [ ] Implement proper gain staging (-18dBFS internal reference)
- [ ] Add per-stage metering for debugging
- [ ] Separate nonlinear and linear processing paths
- [ ] Add optional parallel processing architecture
- [ ] Implement proper transient designer (separate attack/sustain detection)

---

## 🎯 RECOMMENDED IMPLEMENTATION PRIORITY

### Phase 1: Emergency Repair (2-4 hours)
1. Fix mix crossfade (equal-power)
2. Remove master parameter scaling bug
3. Add glue makeup gain
4. Fix M/S matrix
5. Add inter-stage headroom pads

**Result:** Plugin becomes usable

### Phase 2: Core Features (4-8 hours)
6. Implement proper boom sub-harmonic
7. Replace SPARK hard clipper with limiter
8. Fix quality mode 0 or remove it
9. Remove/fix motion white noise
10. Fix GR meter display

**Result:** All parameters work correctly

### Phase 3: Professional Polish (8-16 hours)
11. Implement proper transient designer for Punch
12. Add parallel processing option
13. Implement proper gain staging
14. Add oversampling UI control
15. Comprehensive testing & validation

**Result:** Production-ready plugin

---

## 🔬 TESTING REQUIREMENTS

After fixes, validate:
1. **Mix knob** - No phase artifacts at 25%, 50%, 75%
2. **Glue** - No volume drop, transparent cohesion
3. **Width** - Mono-compatible, no phase issues
4. **Boom** - Perceptible low-end enhancement
5. **Punch** - Transient enhancement without clipping
6. **Warmth** - Saturation without hard clipping
7. **SPARK** - Proper limiting, no clipping, GR meter moves
8. **Quality modes** - No artifacts, clean processing
9. **All parameters** - Audible effect across full range
10. **THD+N** - Validate < 0.01% target after fixes

---

**END OF ANALYSIS**

Next step: Implement Phase 1 emergency repairs.
