# BTZ Sonic Alchemy — Commercial Trust & Accessibility

## 1. Engineering Credibility (Trust Signals)

To compete with brands like FabFilter or Tokyo Dawn, BTZ must project absolute engineering competence.

**1. Published Measurements:**
- Create a "Whitepaper" section on the website.
- Publish PluginDoctor validation reports.
- Publish FFT plots showing the exact harmonic profiles of the 9 saturation models (proving they aren't just the same waveshaper with different gains).
- Publish the aliasing performance (showing the noise floor with and without the Oversampling Engine).

**2. Latency Transparency:**
- The plugin currently reports latency to the host.
- **Improvement:** Display the exact latency in milliseconds and samples in the UI footer (e.g., `Latency: 3.2ms (154 smp)`). This proves the plugin is safe for tracking or parallel processing.

**3. CPU Transparency:**
- Display a small CPU load meter in the footer. If the plugin is truly running at 1-4% CPU, flaunt it.

## 2. Accessibility Audit

**1. Colorblind Safety:**
- The Ivory System uses Sage Green and Terracotta. These can be problematic for red-green colorblindness (Deuteranopia).
- **Action:** Ensure that color is *never* the only indicator of state. If a button is active, it should change color *and* brightness, or include a visual icon (like a checkmark or glowing dot).

**2. Keyboard Navigation:**
- Users must be able to tab through all controls and type values.
- **Action:** Ensure all `juce::Component` objects have `setWantsKeyboardFocus(true)` where appropriate.

**3. Screen Reader Considerations:**
- While full screen reader support in VSTs is rare, providing basic accessible names for APVTS parameters ensures that DAW-native parameter lists (which screen readers *can* read) are properly labeled.

## 3. Reviewer Readiness

When sending BTZ to YouTubers or reviewers (e.g., White Sea Studio, Dan Worrall):
- They *will* run it through PluginDoctor.
- They *will* check if the TruePeak limiter actually catches inter-sample peaks. (The v1.0.1 patch fixed this, but it must be verified).
- They *will* check if the LUFS meter matches standard tools like Youlean Loudness Meter.
- **Action:** Build a "Reviewer Guide" PDF that explicitly states what the plugin does, how the Target Lock works, and provides the mathematical proof of the DSP quality.
