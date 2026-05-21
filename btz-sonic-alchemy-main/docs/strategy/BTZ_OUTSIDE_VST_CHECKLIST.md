# BTZ Sonic Alchemy — Outside-of-VST Commercialization Checklist

This document outlines the actionable steps required to launch BTZ Sonic Alchemy as a commercial product, completely separate from writing C++ code.

## 1. Legal & Business Setup

**1. Form a Business Entity**
- **How-To:** Register an LLC (Limited Liability Company) in your jurisdiction. This protects your personal assets if someone claims your plugin blew their speakers.
- **Action:** File articles of organization, get an EIN (Employer Identification Number), and open a business bank account.

**2. Secure the JUCE License**
- **How-To:** Go to juce.com. If your revenue is under $50k/year, you can use the Indie tier ($40/month). If over, you need Pro.
- **Action:** Purchase the license and remove the JUCE splash screen from the CMake configuration (`JUCE_DISPLAY_SPLASH_SCREEN=0`).

**3. Trademark Search**
- **How-To:** Use the USPTO TESS database to search for "Box Tone Zone" and "Sonic Alchemy" in Class 9 (Computer Software).
- **Action:** If clear, file a trademark application to protect your brand name.

## 2. Web Presence & E-Commerce

**1. Build the Landing Page**
- **How-To:** Use Webflow, Framer, or WordPress. The page must have:
  - A hero section with the tagline ("Dial in the tone. Lock in the target.")
  - A high-resolution, animated GIF/video of the Harmonic Bloom visualizer.
  - Audio examples (Before/After sliders using the exact same LUFS).
  - A clear "Download 14-Day Trial" button.
- **Action:** Design and deploy the website.

**2. Set Up E-Commerce (FastSpring or Paddle)**
- **How-To:** Do not build your own payment gateway. Use FastSpring or Paddle, as they act as the Merchant of Record (MoR) and handle global VAT/Sales Tax automatically.
- **Action:** Create an account, set up the BTZ product, and integrate their checkout overlay into your website.

## 3. Marketing & Trust Building

**1. Create the "Whitepaper"**
- **How-To:** Write a 3-page PDF explaining the math behind the Target Lock engine and the TruePeak limiter. Include PluginDoctor screenshots showing the flat frequency response of the LR4 crossover and the lack of aliasing.
- **Action:** Publish this on the website under a "Technology" tab.

**2. Prepare the Reviewer Kit**
- **How-To:** Create a Google Drive folder containing:
  - The macOS/Windows installers.
  - A NFR (Not For Resale) serial key.
  - The Reviewer Guide PDF (explaining the Target Lock feature).
  - High-res transparent PNGs of the plugin UI.
- **Action:** Send this kit to audio YouTubers (e.g., White Sea Studio, Dan Worrall, Produce Like A Pro) 2 weeks before launch.

**3. Audio Demos**
- **How-To:** Create 5 distinct audio examples (Drum Bus, Vocal, Synth Bass, Master Bus, Acoustic Guitar).
- **Action:** Bounce the "Before" and "After" versions. **Crucial:** Ensure both versions are exactly the same LUFS. If the "After" is louder, audio engineers will dismiss it as a trick.

## 4. Support & Distribution

**1. Set Up a Support Desk**
- **How-To:** Use a tool like HelpScout or Zendesk. Create an email address (e.g., `support@btzaudio.com`).
- **Action:** Populate the knowledge base with the FAQ document created earlier.

**2. Code Signing & Notarization (macOS)**
- **How-To:** You must sign the `.pkg` installer and the `.vst3`/`.component` files with an Apple Developer ID Application certificate, then send them to Apple's notarization service. If you don't, macOS will block the installation.
- **Action:** Enroll in the Apple Developer Program ($99/year), generate certificates, and use the `xcrun altool` command-line tools to notarize the installer.

**3. Windows Authenticode Signing**
- **How-To:** Purchase an EV (Extended Validation) Code Signing Certificate (e.g., from Sectigo or DigiCert). Use the Microsoft `signtool` to sign the `.iss` installer and the `.vst3` file.
- **Action:** Sign the Windows binaries so Windows SmartScreen doesn't flag the installer as malware.
