# BTZ Legal & Design Safety

**Version**: 1.0.0
**Purpose**: Document clean-room DSP practices and legal compliance
**Last Updated**: 2026-01-08

---

## 🔒 CLEAN-ROOM DSP POLICY

### Principle
**All BTZ DSP code is original, clean-room implementation.**

We do NOT:
- Copy code from proprietary plugins (Waves, UAD, Plugin Alliance, Acustica, SSL, Neve, Output, iZotope, FabFilter, Sonnox, etc.)
- Reverse-engineer proprietary algorithms
- Use decompiled or leaked source code
- Integrate third-party closed-source libraries without proper licensing
- Clone proprietary ML models or IR captures without rights

### What We DO
- Implement well-known DSP algorithms from academic literature and textbooks
- Use open-source reference implementations (with proper attribution)
- Design original transfer curves, filter topologies, and processing chains
- Study publicly available specifications and behaviors for inspiration
- Cite sources when implementing published algorithms

---

## 📚 ACCEPTABLE REFERENCE SOURCES

### Academic & Books
- ✅ "The Art of VA Filter Design" - Vadim Zavalishin
- ✅ "Designing Audio Effect Plugins in C++" - Will Pirkle
- ✅ "DAFX: Digital Audio Effects" - Udo Zölzer
- ✅ IEEE/AES papers on DSP techniques
- ✅ JUCE documentation and examples
- ✅ Open-source DSP libraries (with license check)

### Behavioral Inspiration (NOT code copying)
- ✅ Study frequency response curves of classic hardware
- ✅ Analyze harmonic characteristics of analog gear
- ✅ Understand workflow and UX of professional plugins
- ✅ Learn from published specifications (e.g., "SSL Fusion Air band ~22kHz shelf")

### What Requires License
- ❌ Proprietary IRs (impulse responses)
- ❌ ML models trained on proprietary hardware
- ❌ Sample libraries without redistribution rights
- ❌ Closed-source SDKs (check license terms)

---

## 🏷️ TRADEMARK & NAMING POLICY

### Forbidden in UI/Docs/Presets
- ❌ Hardware brand names: SSL, Neve, API, Pultec, Fairchild, LA-2A, 1176, etc.
- ❌ Plugin brand names: Waves, UAD, Plugin Alliance, iZotope, FabFilter, Output, etc.
- ❌ Model numbers: CLA-76, Pultec EQP-1A, SSL G-Comp, etc.
- ❌ Trademarked terms used in a way that implies endorsement

### Acceptable Practices
- ✅ Generic descriptors: "Vintage Compressor", "Classic EQ", "Tube Saturation"
- ✅ Behavioral descriptions: "Punch-Forward", "Air Enhancement", "Analog Warmth"
- ✅ Technical terms: "Opto-style", "FET-style", "VCA-style" (when describing behavior, not claiming authenticity)
- ✅ Inspirational references in **internal documentation only** (not user-facing)

### Preset Naming
Use generic, evocative names:
- ✅ "Dynamic-Design", "Punch-Smack", "Analog-Reel"
- ❌ "CLA-Drums", "Waves-Vocal", "UAD-Master"

---

## 🛡️ THIRD-PARTY CODE COMPLIANCE

### JUCE Framework
- **License**: Dual GPL v3 / Commercial
- **Our Status**: Using Commercial License OR GPL v3 (specify in build)
- **Compliance**: Must not violate JUCE EULA terms
- **Attribution**: Required in "About" dialog and documentation

### Other Dependencies
All third-party code must be:
1. **Inventoried** in `docs/THIRD_PARTY_LICENSES.md`
2. **License-compatible** with our distribution model
3. **Properly attributed** in UI/docs where required
4. **Audited** for security and quality

### Current Dependencies
- JUCE 7.0.12: GPL v3 / Commercial
- (Add others as they are integrated)

---

## 🎨 DESIGN INSPIRATION vs COPYING

### What "Inspired By" Means
When we say BTZ SHINE is "inspired by SSL Fusion Air + Maag EQ4 Air Band":
- ✅ We studied the **publicly documented behavior**: "Air band around 20-22kHz, subtle harmonic sheen"
- ✅ We implemented our **own filter designs** to achieve similar sonic goals
- ✅ We **did NOT** copy code, IR captures, or proprietary algorithms
- ✅ We **do NOT claim** our implementation sounds identical or is an official emulation

### Example: SPARK Clipper
- **Inspiration**: High-end mastering limiters use oversampling + transparent clipping
- **Implementation**: Our own waveshaper curves, our own oversampling strategy, our own anti-aliasing filters
- **No Code Copied**: Clean-room implementation based on DSP textbooks

---

## 📋 COMPLIANCE CHECKLIST (Before Release)

### Code Review
- [ ] No proprietary code snippets in source
- [ ] All algorithms cite academic sources or are original
- [ ] No reverse-engineered IP

### UI/UX Review
- [ ] No trademarked names in UI strings
- [ ] No brand logos without permission
- [ ] Preset names are generic and safe

### Documentation Review
- [ ] No false claims of authenticity
- [ ] "Inspired by" language is accurate
- [ ] Third-party licenses documented

### Legal Review (if distributing commercially)
- [ ] Consult IP attorney for trademark concerns
- [ ] Verify JUCE license compliance
- [ ] Ensure all dependencies allow commercial use

---

## 🔍 AUDIT TRAIL

### How We Prove Clean-Room
1. **Git History**: All code commits show incremental development, not sudden large additions
2. **Source Citations**: Comments cite textbooks, papers, and open-source references
3. **Design Docs**: Internal docs explain **why** we chose each algorithm (engineering rationale)
4. **Test Fixtures**: Our own test audio, not extracted from proprietary plugins

### If Questioned
- We can demonstrate step-by-step development from first principles
- We can cite all reference materials used
- We can explain engineering decisions without referring to proprietary implementations

---

## ⚖️ ENFORCEMENT

### Developer Responsibility
Every BTZ contributor must:
- Sign off that contributed code is original or properly licensed
- Declare if code is derived from external sources
- Never commit proprietary/leaked code

### Code Review Process
- All PRs require review for legal compliance
- Suspicious code triggers audit
- External contributions require license verification

---

## 📞 QUESTIONS?

If unsure whether a technique, reference, or naming choice is safe:
1. **Document the question** in this file or a GitHub issue
2. **Cite the source** you want to reference
3. **Propose alternative** if the original might be risky
4. **Consult legal counsel** for commercial release

---

**Bottom Line**: BTZ is a professional-grade audio plugin built with integrity. We respect intellectual property, implement algorithms from first principles, and deliver world-class sound through clean-room engineering.

**Version**: 1.0.0
**Last Updated**: 2026-01-08
**Maintained By**: BTZ Development Team
