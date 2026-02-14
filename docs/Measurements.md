# BTZ Measurement & Validation Procedures

## PluginDoctor Validation

### Prerequisites
- **PluginDoctor** (https://ddmf.eu/plugindoctor/)
- **BTZ plugin** installed and scanned by DAW/PluginDoctor
- **Test session**: 48 kHz, 512-sample buffer

### Test 1: Frequency Response (Neutral Mode)

**Purpose:** Verify neutral preset has flat frequency response (±0.5 dB target)

**Procedure:**
1. Load BTZ in PluginDoctor
2. **Preset**: "Mix Bus - Clean Glue" OR manually set:
   - All parameters at default
   - PUNCH: 0
   - WARMTH: 0
   - BOOM: 0
   - GLUE: 0
   - SHINE: 0
   - DRIVE: 0 dB
   - MIX: 100%
   - Quality Mode: Good (2x)
3. **Analysis → Frequency Response**
4. **Signal**: -12 dBFS pink noise
5. **Measurement duration**: 10 seconds
6. **Verify**: Response within ±0.5 dB from 20 Hz to 20 kHz

**Expected Result:**
```
20 Hz:   -0.1 to +0.1 dB
100 Hz:  -0.2 to +0.2 dB
1 kHz:   -0.1 to +0.1 dB (reference)
10 kHz:  -0.3 to +0.3 dB
20 kHz:  -0.4 to +0.5 dB
```

**Pass Criteria:** All frequencies within ±0.5 dB
**Screenshot:** Save as `BTZ_FreqResponse_Neutral.png`

---

### Test 2: THD+N (Total Harmonic Distortion + Noise)

**Purpose:** Verify clean processing meets <0.01% THD+N target

**Procedure:**
1. Load BTZ in PluginDoctor
2. **Preset**: Neutral (all controls at 0)
3. **Analysis → THD+N**
4. **Signal**: -12 dBFS sine wave, 1 kHz
5. **Measurement bandwidth**: 20 Hz – 20 kHz
6. **A-weighting**: ON
7. **Measure at multiple levels:**
   - -6 dBFS
   - -12 dBFS
   - -18 dBFS
   - -24 dBFS

**Expected Results:**

| Level   | THD+N (Target) | THD+N (Acceptable) |
|---------|----------------|-------------------|
| -6 dBFS | < 0.005%       | < 0.01%           |
| -12 dBFS| < 0.003%       | < 0.008%          |
| -18 dBFS| < 0.002%       | < 0.006%          |
| -24 dBFS| < 0.001%       | < 0.005%          |

**Pass Criteria:** All levels < 0.01% THD+N
**Screenshot:** Save as `BTZ_THD_Neutral.png`

---

### Test 3: Aliasing Test (Oversampling Validation)

**Purpose:** Verify oversampling reduces aliasing artifacts

**Test Signal:** 18 kHz sine wave @ -12 dBFS
**Sample Rate:** 48 kHz (Nyquist = 24 kHz)

**Procedure:**
1. Load BTZ, enable **WARMTH: 80** (heavy saturation)
2. **Test A: Draft Mode (No Oversampling)**
   - Quality Mode: Draft
   - Analyze spectrum 0–24 kHz
   - Look for aliasing products:
     - 6 kHz (24 - 18)
     - 12 kHz (30 - 18, folded)
3. **Test B: Good Mode (2x Oversampling)**
   - Quality Mode: Good
   - Measure same spectrum
4. **Test C: Best Mode (4x Oversampling)**
   - Quality Mode: Best
   - Measure same spectrum

**Expected Results:**

| Quality | Aliasing @ 6 kHz | Aliasing @ 12 kHz |
|---------|------------------|-------------------|
| Draft   | -40 to -50 dB    | -50 to -60 dB     |
| Good    | -60 to -70 dB    | -70 to -80 dB     |
| Best    | < -80 dB         | < -90 dB          |

**Pass Criteria:**
- Best mode: Aliasing products < -80 dB
- Good mode: Aliasing products < -60 dB

**Screenshot:** Save as `BTZ_Aliasing_Comparison.png`

---

### Test 4: Gain Reduction Meter Accuracy

**Purpose:** Verify GR meter reflects actual limiting

**Procedure:**
1. Load BTZ
2. **Settings:**
   - SPARK Ceiling: -3 dB
   - SPARK Mix: 100%
   - All other params: 0
3. **Input Signal:** -1 dBFS sine wave (1 kHz)
4. **Expected Output:** -3 dBFS (limited)
5. **Expected GR Meter:** ~2 dB gain reduction

**Verification:**
1. Use PluginDoctor waveform view
2. Measure peak output level → Should be -3 dBFS
3. Read GR meter in BTZ GUI → Should show ~2 dB

**Pass Criteria:** GR meter ±0.5 dB of actual reduction
**Screenshot:** Save as `BTZ_GR_Meter_Validation.png`

---

### Test 5: LUFS Accuracy (ITU-R BS.1770-4)

**Purpose:** Verify LUFS meter matches reference

**Procedure:**
1. Load BTZ + **Reference LUFS meter** (e.g., Youlean Loudness Meter)
2. **Test Signal:** Pink noise, -18 dBFS RMS, 30 seconds
3. Process through BTZ (neutral settings)
4. **Compare readings:**
   - BTZ LUFS meter
   - Reference LUFS meter

**Expected Result:** Within ±0.5 LU of reference
**Pass Criteria:** Match within ±1 LU
**Screenshot:** Save as `BTZ_LUFS_Comparison.png`

---

## REW (Room EQ Wizard) Validation

### Test 6: Null Test (Phase Coherence)

**Purpose:** Verify plugin doesn't introduce phase shifts in neutral mode

**Prerequisites:**
- **REW** (https://www.roomeqwizard.com/)
- **DAW** with parallel routing (Reaper recommended)

**Procedure:**
1. **Reaper Setup:**
   - Track 1: Test signal (pink noise, -12 dBFS)
   - Track 2: Duplicate of Track 1
   - Track 2 → Insert BTZ (neutral settings)
   - Track 2 → **Invert polarity** (Reaper: "Phase Invert" button)
   - Master: Sum both tracks

2. **Record Master Output** (should be near silence if phase-coherent)

3. **REW Analysis:**
   - Import recorded null test
   - Measure RMS level
   - **Expected:** < -60 dBFS (near-perfect cancellation)

**Pass Criteria:**
- Null test residual: < -50 dBFS
- Indicates minimal phase shift in neutral path

**Screenshot:** Save as `BTZ_NullTest_REW.png`

---

### Test 7: Mono Compatibility (Correlation)

**Purpose:** Verify "Mono Safe" mode prevents phase issues

**Procedure:**
1. **Test Signal:** Stereo drum loop with wide low-end
2. **Settings:**
   - WIDTH: 80 (wide)
   - Mono Safe: OFF
3. **Measure:**
   - Correlation meter in BTZ
   - REW stereo correlation analysis
4. **Repeat with Mono Safe: ON**

**Expected Results:**

| Mono Safe | Correlation (Low-End) | Correlation (High-End) |
|-----------|----------------------|------------------------|
| OFF       | +0.2 to +0.5         | +0.3 to +0.7           |
| ON        | +0.9 to +1.0 (mono)  | +0.3 to +0.7 (unchanged)|

**Pass Criteria:**
- Mono Safe ON → Low-end correlation > +0.9
- High-end remains unaffected

**Screenshot:** Save as `BTZ_MonoSafe_Correlation.png`

---

## DAW-Based Tests

### Test 8: Latency Compensation

**Purpose:** Verify reported latency matches actual delay

**Procedure (Reaper):**
1. **Track 1:** 1 kHz click, no processing
2. **Track 2:** Same click → BTZ → Quality Mode: Best (4x)
3. **Align waveforms** visually
4. **Measure delay** in samples
5. **Compare to BTZ reported latency:**
   - Right-click track → "Show FX chain latency"

**Expected:**
- Draft: ~64 samples
- Good: ~128 samples
- Best: ~256 samples

**Pass Criteria:** Measured delay ±5 samples of reported
**Screenshot:** Save as `BTZ_Latency_Validation.png`

---

### Test 9: Automation Smoothness

**Purpose:** Verify no zipper noise during parameter automation

**Procedure:**
1. **Automate WARMTH:** 0 → 100 over 1 second
2. **Input:** Constant -12 dBFS sine (1 kHz)
3. **Listen:** Should be smooth, no clicks/zippers
4. **Visual:** Waveform should show gradual change

**Pass Criteria:** No audible clicks or discontinuities
**Screenshot:** Save as `BTZ_Automation_Smoothness.png`

---

### Test 10: Offline Bounce Determinism

**Purpose:** Verify plugin produces identical output every time

**Procedure:**
1. **Create test project:**
   - Audio track with drum loop
   - Insert BTZ
   - Settings: "Drum Bus - Punch & Glue" preset
2. **Offline bounce** 5 times (Export → Render)
3. **Compute MD5 hashes:**
   ```bash
   md5sum render_1.wav render_2.wav render_3.wav render_4.wav render_5.wav
   ```

**Expected Result:** All 5 hashes IDENTICAL

**Pass Criteria:** 5/5 renders have matching MD5 hashes
**Output:** Save hash comparison to `BTZ_Determinism_Test.txt`

---

## Performance Tests

### Test 11: CPU Stress Test

**Purpose:** Validate CPU usage under heavy load

**Procedure (Reaper):**
1. **Create 50 instances** of BTZ
2. **All instances:** Quality Mode: Best (4x)
3. **Play project** with moderate audio
4. **Monitor CPU:** Reaper Performance Meter
5. **Record peak CPU usage**

**Expected Results (i7-10700K @ 48 kHz, 512 samples):**
- 50 instances @ Best: ~50–60% CPU
- Should not glitch/dropout

**Pass Criteria:** Stable playback without dropouts
**Screenshot:** Save as `BTZ_CPU_StressTest.png`

---

### Test 12: Memory Leak Test

**Purpose:** Ensure no memory leaks over extended use

**Procedure:**
1. **Launch DAW** with BTZ instances
2. **Play project** continuously for 1 hour
3. **Monitor RAM usage:**
   - Task Manager (Windows)
   - Activity Monitor (macOS)
4. **Record initial and final memory**

**Expected Result:** < 5% memory growth over 1 hour

**Pass Criteria:** No significant memory leaks
**Output:** Save to `BTZ_MemoryTest_Results.txt`

---

## Summary Checklist

After completing all tests, verify:

- [x] **Frequency Response:** ±0.5 dB neutral
- [x] **THD+N:** < 0.01% @ -12 dBFS
- [x] **Aliasing:** < -80 dB (Best mode)
- [x] **GR Meter:** Accurate ±0.5 dB
- [x] **LUFS Meter:** Match reference ±1 LU
- [x] **Null Test:** < -50 dBFS residual
- [x] **Mono Safe:** Correlation > +0.9 (low-end)
- [x] **Latency:** Reported = Measured ±5 samples
- [x] **Automation:** No zipper noise
- [x] **Determinism:** 5/5 identical renders
- [x] **CPU:** Stable under 50+ instances
- [x] **Memory:** No leaks over 1 hour

**All Pass?** → Ship-ready for v1.0.0 ✅

---

**Archive all screenshots in:** `docs/measurements/`
**Version Tested:** BTZ v1.0.0
**Test Date:** [YOUR DATE]
**Tester:** [YOUR NAME]
