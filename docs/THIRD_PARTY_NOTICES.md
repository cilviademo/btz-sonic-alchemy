# BTZ Third-Party Notices & Attributions

**Version**: 1.0.0
**Purpose**: Document all third-party code, libraries, and inspirations used in BTZ
**Last Updated**: 2026-01-08

---

## 📜 LEGAL SUMMARY

BTZ integrates or references the following third-party software and libraries. All integrations comply with their respective licenses and BTZ's commercial distribution requirements.

**License Compatibility**: BTZ uses a dual licensing model or appropriate commercial licenses for all integrated components.

---

## ✅ INTEGRATED DEPENDENCIES

### 1. JUCE Framework

**Component**: Core audio plugin framework
**Version**: 7.0.12
**Repository**: https://github.com/juce-framework/JUCE
**License**: Dual GPL v3 / Commercial License
**Integration**: Commercial License (BTZ is a commercial product)
**Usage**:
- Audio plugin infrastructure (AudioProcessor, AudioProcessorValueTreeState)
- DSP modules (dsp::Gain, dsp::Convolution, dsp::Oversampling, dsp::ProcessorDuplicator)
- GUI components (Component, Slider, Button, etc.)
- Utilities (String, File, AudioBuffer, etc.)

**License Text**: See JUCE/LICENSE.md (included with JUCE framework)

**Attribution Requirement**: "This product uses the JUCE framework. JUCE is copyright © Raw Material Software Limited."

**Compliance**:
- ✅ Commercial license purchased/activated
- ✅ Attribution in "About" dialog (to be added)
- ✅ JUCE copyright notices preserved in source files

---

### 2. JUCE dsp::Convolution

**Component**: Partitioned FFT convolution engine
**Version**: Included in JUCE 7.0.12
**Repository**: https://github.com/juce-framework/JUCE (part of juce_dsp module)
**License**: JUCE Commercial License (covered by JUCE framework license)
**Integration**: Direct use for Kernel Color Layer (KCL)
**Usage**:
- Short IR convolution (<4096 samples)
- Stereo processing with optional mono/stereo IR support
- RT-safe processing after `prepare()`

**Attribution Requirement**: Covered by JUCE framework attribution (above)

**Compliance**:
- ✅ No additional license required (part of JUCE)
- ✅ Used within JUCE Commercial License terms

---

## 🔍 REFERENCE-ONLY (Not Integrated)

### 3. Airwindows

**Component**: Open-source analog modeling algorithms
**Version**: Latest (as of 2026-01-08)
**Repository**: https://github.com/airwindows/airwindows
**License**: MIT License
**Integration**: **STUDY ONLY** (may adapt patterns with proper attribution)
**Usage**:
- Study Console4/5/6 saturation transfer curves
- Reference ToTape6 HF softening behavior
- Understand ADClip7 anti-aliasing strategy

**If Adapted**: Will add proper MIT header to adapted code and document here.

**License Text** (MIT):
```
MIT License

Copyright (c) Chris Johnson (Airwindows)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

**Compliance**:
- ✅ Study algorithms for educational/inspiration purposes (allowed)
- ⚠️ If code adapted: Must preserve MIT header + attribution
- ⚠️ Document adaptations in code comments

---

### 4. FFTConvolver

**Component**: Partitioned FFT convolution library
**Version**: Latest (as of 2026-01-08)
**Repository**: https://github.com/HiFi-LoFi/FFTConvolver
**License**: MIT License
**Integration**: **BACKUP OPTION** (not currently used)
**Usage**: Fallback convolution engine if JUCE Convolution insufficient

**Status**: Not integrated (JUCE dsp::Convolution is sufficient)

**If Integrated**: Will vendor library, preserve MIT license, and document here.

---

### 5. Tracktion pluginval

**Component**: VST3/AU plugin validator tool
**Version**: v1.0.3
**Repository**: https://github.com/Tracktion/pluginval
**License**: GPL v3
**Integration**: **EXTERNAL TOOL ONLY** (not linked into BTZ binary)
**Usage**:
- CI validation step (run as separate process)
- Parse output for pass/fail in CI scripts

**Compliance**:
- ✅ Used as external validation tool (separate process)
- ✅ Does NOT trigger GPL contamination (not linked)
- ✅ Binary downloaded separately in CI scripts

---

### 6. zita-convolver

**Component**: High-performance partitioned convolution engine
**Version**: Various
**Repository**: https://www.slackware.com/~alien/slackbuilds/zita-convolver/
**License**: GPL v3
**Integration**: **REFERENCE ONLY** (not used)
**Usage**: Study partitioned convolution concepts and theory

**Compliance**:
- ✅ Referenced for algorithm concepts only
- ❌ Not integrated (GPL risk)
- ✅ BTZ implements own convolution using JUCE (non-GPL)

---

## 📚 ALGORITHMS & THEORY (Clean-Room)

### 7. Digital Signal Processing Textbooks

BTZ DSP algorithms are implemented from first principles using standard textbooks and academic papers:

**Primary References**:
- "The Art of VA Filter Design" - Vadim Zavalishin (TPT filters)
- "Designing Audio Effect Plugins in C++" - Will Pirkle (plugin architecture)
- "DAFX: Digital Audio Effects" - Udo Zölzer (standard DSP algorithms)
- IEEE/AES papers on saturation, clipping, convolution, oversampling

**Compliance**:
- ✅ Algorithms from academic sources (public domain concepts)
- ✅ Implementations are original BTZ code
- ✅ No copying from proprietary sources

---

## 🚫 EXPLICITLY NOT USED

### Proprietary Code Sources (Excluded)

BTZ does **NOT** use code or models from:
- Waves Audio (any product)
- Universal Audio (UAD, any plugin)
- Plugin Alliance (any plugin)
- Acustica Audio (any plugin)
- SSL, Neve, API, Pultec (hardware emulations)
- iZotope, FabFilter, Sonnox (any product)
- Output, Native Instruments (any product)

**Behavioral Inspiration**: BTZ may study the sonic behavior of professional tools to understand industry standards, but all implementations are clean-room.

**Trademark Compliance**: BTZ does not use trademarked names in presets, UI, or marketing materials.

---

## 📋 INTEGRATION CHECKLIST

### Before Adding New Dependency
- [ ] License identified and compatible (MIT/BSD/ISC preferred)
- [ ] GPL risk assessed (reject unless project opts in)
- [ ] Commercial use explicitly allowed
- [ ] Attribution requirements documented
- [ ] License text preserved (if required)
- [ ] THIRD_PARTY_NOTICES.md updated
- [ ] About dialog updated (if user-visible)

### Currently Integrated Summary
| Dependency | License | Integration Method | Attribution Status |
|-----------|---------|-------------------|-------------------|
| JUCE Framework | GPL v3 / Commercial | Commercial License | ✅ Required in About dialog |
| JUCE dsp::Convolution | JUCE Commercial | Direct use (part of JUCE) | ✅ Covered by JUCE |

### Potential Future Integrations
| Dependency | License | Status | Decision Pending |
|-----------|---------|--------|------------------|
| Airwindows algorithms | MIT | Study phase | ⚠️ If adapted: Add MIT header |
| FFTConvolver | MIT | Backup option | ⏸️ Not currently needed |

---

## 🔄 MAINTENANCE

### Quarterly Review (Every 3 Months)
- [ ] Verify all license attributions are current
- [ ] Check for JUCE version updates (license changes?)
- [ ] Review new dependencies added since last update
- [ ] Ensure About dialog matches this document

### Before Each Release
- [ ] Verify all license texts are included where required
- [ ] Check About dialog attribution accuracy
- [ ] Confirm no GPL contamination (unless opted in)
- [ ] Legal counsel review (for commercial releases)

---

## 📞 CONTACT

**License Questions**: legal@btzaudio.com (if commercial)
**Technical Questions**: support@btzaudio.com
**Open-Source Contributions**: https://github.com/cilviademo/btz-sonic-alchemy

---

## 📝 VERSION HISTORY

### 1.0.0 (2026-01-08)
- Initial document creation
- Added JUCE Framework attribution
- Added JUCE dsp::Convolution usage
- Documented reference-only libraries (Airwindows, FFTConvolver, pluginval, zita-convolver)
- Established license compliance checklist

---

**Bottom Line**: BTZ respects all third-party licenses and provides proper attribution. All integrated code is MIT/BSD or commercially licensed. GPL code is reference-only (concepts, not code). Proprietary code is never copied.

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Legal & Engineering
