# BTZ Sonic Alchemy — First Impression Audit

## 1. The 60-Second Experience

**Current Default State:**
- The plugin loads in Simple Mode.
- All knobs are at 0 or 50%.
- No target is locked.
- The sound is completely transparent (digital bypass).

**The Problem:**
When a user loads a premium saturation/bus plugin, they expect an *immediate* enhancement to the audio. A transparent default state makes the plugin feel broken or ineffective until the user starts turning knobs. Furthermore, the "loudness bias" means users will perceive the plugin as "better" if it makes the audio slightly louder and thicker by default.

## 2. Proposed First Impression Strategy

**1. "Instant Good Sound" Initialization:**
- **Default Preset:** "Subtle Mix Glue"
- **Default Settings:**
  - Saturation Model: `Tape` (adds odd harmonics, smooths transients).
  - Drive: 15% (subtle thickness).
  - Target Lock: **Engaged** at -14 LUFS (streaming standard) with a 6dB dynamics threshold.
- **The Result:** The moment the plugin is instantiated, the mix sounds slightly wider, more glued, and hits streaming loudness targets without pumping.

**2. Honest Loudness-Matched A/B Workflow:**
- The biggest trust-breaker in audio software is the "louder is better" trick.
- **Solution:** The bypass button should automatically engage a loudness-matching algorithm. When bypassed, the dry signal is gain-matched to the processed signal's LUFS. This proves to the user that BTZ is actually improving the *tone*, not just turning up the volume.

**3. Delta Listening UX:**
- The Delta button (listening only to the added harmonics) is a massive trust signal.
- **Improvement:** When Delta is engaged, the UI should tint slightly (e.g., the Ivory background dims to a cool grey) to visually reinforce that the user is in a diagnostic listening mode.

**4. First-Run Tooltip Strategy:**
- On the very first launch, a subtle, elegant tooltip should point to the Target Lock section: *"Type your target LUFS here. BTZ will handle the rest."*
- No intrusive pop-ups or forced tutorials. Just one guiding arrow to the flagship feature.

**5. Simple Mode Optimization:**
- Simple Mode currently has 3 knobs: Drive, Tone, Output.
- **Improvement:** Rename "Output" to "Target LUFS" in Simple Mode. Make the flagship feature the primary interaction point even for beginners.
