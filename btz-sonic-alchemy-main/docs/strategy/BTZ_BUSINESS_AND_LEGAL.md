# BTZ Sonic Alchemy — Business & Legal Readiness

*Disclaimer: This document provides technical and commercial awareness, not legal advice. Consult an attorney for formal legal review.*

## 1. Licensing & Dependencies

**JUCE Framework:**
- BTZ is built on JUCE 8.0.6.
- If you intend to sell BTZ commercially (closed source), you **must** purchase a JUCE Indie or Pro license.
- If you release it under the GPLv3 open-source license, it can be free, but your source code must be public.

**Third-Party Libraries:**
- **clap-juce-extensions:** Check the license (usually MIT or BSD). Ensure attribution is included in the manual or an "About" screen.
- **Neural Network Inference:** If you integrate RTNeural or ONNX Runtime for the neural models, verify their licenses. RTNeural is typically permissive, but ONNX Runtime has specific attribution requirements.

## 2. Trademark Considerations

- **"Box Tone Zone" / "BTZ":** Search the USPTO database to ensure no existing audio hardware or software companies hold trademarks for these terms.
- **"Sonic Alchemy":** This is a common phrase. Ensure it isn't trademarked in the software/audio plugin class (Class 9).

## 3. Commercialization Strategy

**1. Licensing Model:**
- **Perpetual License:** The standard for premium plugins. Charge a flat fee (e.g., $99 - $149).
- **Subscription:** Not recommended for a single plugin. Only viable if you build an ecosystem of 5+ plugins.

**2. Trial / Demo Approach:**
- **Time-Limited:** Fully functional for 14 days. Requires a licensing server (e.g., PACE iLok or a custom Keygen system).
- **Feature-Limited:** Free version has no Target Lock and only 3 saturation models. Paid version unlocks everything. (Easier to implement, no DRM server needed).
- **Audio-Interrupt:** Emits white noise or silence every 45 seconds until registered. (Standard, easy to code in `processBlock`).

**3. Copy Protection (DRM):**
- **PACE iLok:** Industry standard, but expensive for indie developers.
- **Serial Keys:** Generate offline serial keys using RSA cryptography. Users paste the key into the plugin UI. This is the recommended path for indie commercialization.

## 4. Analytics & Privacy

- **Crash Reporting:** Implement a system (like Sentry or Crashlytics) to catch C++ exceptions in the wild.
- **Analytics:** If you track usage (e.g., "how many times is Target Lock used?"), you **must** include a GDPR/CCPA compliant privacy policy and an opt-out toggle in the plugin settings. For a VST, it is highly recommended to have **zero telemetry** to build trust with paranoid audio engineers.
