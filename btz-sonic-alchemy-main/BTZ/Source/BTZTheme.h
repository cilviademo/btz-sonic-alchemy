/*
  Box Tone Zone (BTZ) — BTZTheme.h  v11
  ──────────────────────────────────────────────────────────────────────────
  SINGLE SOURCE OF TRUTH for every visual constant.

  Aesthetic: Premium light, warm identity, modern precision.
    • Warm cream canvas — never cold white, never dark
    • Sage green primary accent, warm orange secondary
    • Tight 3-4px radii, dense professional typography
    • Clean flat knobs with colored arcs — no skeuomorphism
    • Separation via luminance shifts, not heavy borders
    • Monospace values, uppercase labels, controlled density
    • Immediately identifiable warm palette

  If a hex value appears in paint() that isn't from this header, it's a bug.
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>

namespace btz {

// ═══════════════════════════════════════════════════════════════════════════
// Palette
// ═══════════════════════════════════════════════════════════════════════════

namespace palette {
    // Surfaces (warm light — cream/beige)
    inline constexpr uint32_t canvas       = 0xFFF5F2ED;
    inline constexpr uint32_t surface      = 0xFFEDE9E2;
    inline constexpr uint32_t surfaceAlt   = 0xFFE5E0D8;
    inline constexpr uint32_t well         = 0xFFDAD5CC;
    inline constexpr uint32_t border       = 0xFFCCC6BC;
    inline constexpr uint32_t borderSubtle = 0xFFE0DAD2;

    // Text (warm charcoal hierarchy)
    inline constexpr uint32_t ink          = 0xFF2C2A27;
    inline constexpr uint32_t inkMuted     = 0xFF5C5750;
    inline constexpr uint32_t inkFaint     = 0xFF9A9088;
    inline constexpr uint32_t inkDisabled  = 0xFFC0B8AE;

    // Sage green (primary accent)
    inline constexpr uint32_t sage         = 0xFF7E9B8E;
    inline constexpr uint32_t sageHover    = 0xFF8AAE92;
    inline constexpr uint32_t sagePressed  = 0xFF5E7B6E;
    inline constexpr uint32_t sageFaint    = 0xFFEEF5F0;

    // Warm orange (secondary accent)
    inline constexpr uint32_t orange       = 0xFFD4854A;
    inline constexpr uint32_t orangeHover  = 0xFFE09558;
    inline constexpr uint32_t orangePressed= 0xFFB87040;
    inline constexpr uint32_t orangeFaint  = 0xFFFBF0E4;

    // Tertiary: clay/terracotta (warnings, GR)
    inline constexpr uint32_t clay         = 0xFFC05A48;
    inline constexpr uint32_t clayFaint    = 0xFFFDF2EE;

    // Utility: gold (highlights, meters)
    inline constexpr uint32_t gold         = 0xFFD4A84C;
    inline constexpr uint32_t goldFaint    = 0xFFFFF8E8;

    // Utility: teal (modulation, focus)
    inline constexpr uint32_t teal         = 0xFF5A8A8C;
    inline constexpr uint32_t tealFaint    = 0xFFECF5F5;

    // Knob
    inline constexpr uint32_t knobFace     = 0xFFFFFEFC;
    inline constexpr uint32_t knobTrack    = 0xFFE0DAD2;
    inline constexpr uint32_t knobShadow   = 0x14000000;

    // Meters
    inline constexpr uint32_t meterSafe    = 0xFF7E9B8E;
    inline constexpr uint32_t meterCaution = 0xFFD4A84C;
    inline constexpr uint32_t meterClip    = 0xFFC05A48;
    inline constexpr uint32_t meterTrack   = 0xFFE8E3DC;

    // Harmonics (8-colour cycle)
    inline constexpr uint32_t harm[8] = {
        0xFFD4854A, 0xFFD4A84C, 0xFFC05A48, 0xFFE09558,
        0xFF7E9B8E, 0xFF5A8A8C, 0xFF8AAE92, 0xFFB87040
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// Typography
// ═══════════════════════════════════════════════════════════════════════════

namespace type {
    // Families
    inline const char* sans()  { return "Inter"; }
    inline const char* mono()  { return "JetBrains Mono"; }

    // Scale (px)
    inline constexpr float brand  = 20.0f;
    inline constexpr float h1     = 14.0f;
    inline constexpr float h2     = 12.0f;
    inline constexpr float body   = 11.0f;
    inline constexpr float label  = 9.5f;
    inline constexpr float micro  = 8.5f;
    inline constexpr float value  = 10.0f;  // monospace values

    // Tracking
    inline constexpr float trackLabel = 0.08f;  // uppercase labels
    inline constexpr float trackBody  = 0.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Spacing & Layout
// ═══════════════════════════════════════════════════════════════════════════

namespace space {
    inline constexpr int xxs = 2;
    inline constexpr int xs  = 4;
    inline constexpr int sm  = 6;
    inline constexpr int md  = 10;
    inline constexpr int lg  = 14;
    inline constexpr int xl  = 20;
    inline constexpr int xxl = 28;
}

namespace layout {
    inline constexpr int windowW     = 1120;
    inline constexpr int windowH     = 700;
    inline constexpr int minW        = 840;
    inline constexpr int minH        = 520;

    inline constexpr int headerH     = 36;
    inline constexpr int footerH     = 28;
    inline constexpr int tabH        = 26;

    inline constexpr int knobXL      = 96;   // Simple Mode
    inline constexpr int knobLG      = 56;   // Character knobs
    inline constexpr int knobMD      = 44;   // Macro knobs
    inline constexpr int knobSM      = 34;   // Utility knobs

    inline constexpr int meterW      = 5;
    inline constexpr int meterGap    = 2;
    inline constexpr int buttonH     = 24;
}

namespace radius {
    inline constexpr float sm  = 3.0f;
    inline constexpr float md  = 4.0f;
    inline constexpr float lg  = 6.0f;
    inline constexpr float pill = 999.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Knob Rendering
// ═══════════════════════════════════════════════════════════════════════════

namespace knob {
    inline constexpr float arcWidth      = 3.0f;
    inline constexpr float arcGapRad     = 0.45f;  // gap at bottom (radians)
    inline constexpr float pointerStart  = 0.32f;  // fraction of radius
    inline constexpr float pointerEnd    = 0.58f;
    inline constexpr float pointerWidth  = 1.8f;
    inline constexpr float shadowOffset  = 1.0f;
    inline constexpr float shadowRadius  = 2.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Animation
// ═══════════════════════════════════════════════════════════════════════════

namespace anim {
    inline constexpr int fps          = 30;
    inline constexpr float meterDecay = 0.15f;  // dB per frame
    inline constexpr int hoverMs      = 100;
    inline constexpr int fadeMs       = 180;
}

// ═══════════════════════════════════════════════════════════════════════════
// Component IDs (for accent colour routing)
// ═══════════════════════════════════════════════════════════════════════════

namespace id {
    inline constexpr int drive     = 1001;
    inline constexpr int mix       = 1002;
    inline constexpr int output    = 1003;
    inline constexpr int punch     = 1004;
    inline constexpr int warmth    = 1005;
    inline constexpr int boom      = 1006;
    inline constexpr int glue      = 1007;
    inline constexpr int air       = 1008;
    inline constexpr int width     = 1009;
    inline constexpr int density   = 1010;
    inline constexpr int motion    = 1011;
    inline constexpr int era       = 1012;
    inline constexpr int intensity = 1013;
    inline constexpr int tone      = 1014;
    inline constexpr int resTame   = 1015;
    inline constexpr int transient = 1016;
}

// ═══════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════

// Accent colour for a given component ID
inline juce::Colour accentFor(int componentID) noexcept {
    switch (componentID) {
        case id::drive:     return juce::Colour(palette::orange);
        case id::mix:       return juce::Colour(palette::sage);
        case id::output:    return juce::Colour(palette::sage);
        case id::punch:     return juce::Colour(palette::orange);
        case id::warmth:    return juce::Colour(palette::orange);
        case id::boom:      return juce::Colour(palette::orange);
        case id::glue:      return juce::Colour(palette::sage);
        case id::air:       return juce::Colour(palette::sage);
        case id::width:     return juce::Colour(palette::teal);
        case id::density:   return juce::Colour(palette::gold);
        case id::motion:    return juce::Colour(palette::teal);
        case id::era:       return juce::Colour(palette::gold);
        case id::intensity: return juce::Colour(palette::gold);
        case id::tone:      return juce::Colour(palette::sage);
        case id::resTame:   return juce::Colour(palette::teal);
        case id::transient: return juce::Colour(palette::orange);
        default:            return juce::Colour(palette::sage);
    }
}

// Harmonic colour by index (wraps at 8)
inline juce::Colour harmonicColour(int idx) noexcept {
    return juce::Colour(palette::harm[idx & 7]);
}

// ─── Drawing Helpers ─────────────────────────────────────────────────────

inline void paintKnob(juce::Graphics& g, juce::Rectangle<float> bounds,
                      float normValue, int componentID, bool hovered = false) {
    const auto accent = accentFor(componentID);
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float r  = bounds.getWidth() * 0.5f - knob::arcWidth;

    const float startAngle = juce::MathConstants<float>::pi + knob::arcGapRad;
    const float endAngle   = 3.0f * juce::MathConstants<float>::pi - knob::arcGapRad;

    // Shadow
    g.setColour(juce::Colour(palette::knobShadow));
    g.fillEllipse(bounds.translated(0.0f, knob::shadowOffset).reduced(knob::arcWidth));

    // Face
    g.setColour(juce::Colour(palette::knobFace));
    g.fillEllipse(bounds.reduced(knob::arcWidth + 1.0f));

    // Track arc
    juce::Path track;
    track.addCentredArc(cx, cy, r, r, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(palette::knobTrack));
    g.strokePath(track, juce::PathStrokeType(knob::arcWidth,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc
    const float valueAngle = startAngle + normValue * (endAngle - startAngle);
    if (normValue > 0.002f) {
        juce::Path arc;
        arc.addCentredArc(cx, cy, r, r, 0.0f, startAngle, valueAngle, true);
        g.setColour(hovered ? accent.brighter(0.12f) : accent);
        g.strokePath(arc, juce::PathStrokeType(knob::arcWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Pointer
    const float innerR = r * knob::pointerStart;
    const float outerR = r * knob::pointerEnd;
    const float angle  = valueAngle - juce::MathConstants<float>::halfPi;
    g.setColour(juce::Colour(palette::ink));
    g.drawLine(cx + innerR * std::cos(angle), cy + innerR * std::sin(angle),
               cx + outerR * std::cos(angle), cy + outerR * std::sin(angle),
               knob::pointerWidth);
}

inline void paintMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                       float normLevel, bool isGR = false) {
    g.setColour(juce::Colour(palette::meterTrack));
    g.fillRoundedRectangle(bounds.toFloat(), radius::sm);

    if (normLevel < 0.002f) return;

    const int fillH = static_cast<int>(bounds.getHeight() * normLevel);
    auto fill = bounds.removeFromBottom(fillH);

    juce::Colour c;
    if (isGR)             c = juce::Colour(palette::orange);
    else if (normLevel > 0.92f) c = juce::Colour(palette::meterClip);
    else if (normLevel > 0.72f) c = juce::Colour(palette::meterCaution);
    else                  c = juce::Colour(palette::meterSafe);

    g.setColour(c);
    g.fillRoundedRectangle(fill.toFloat(), radius::sm);
}

inline void paintBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colour(palette::canvas));
    g.fillRect(area);
}

inline void paintPanel(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(juce::Colour(palette::surface));
    g.fillRoundedRectangle(area, radius::md);
}

inline void paintRule(juce::Graphics& g, float x, float y, float w) {
    g.setColour(juce::Colour(palette::borderSubtle));
    g.fillRect(x, y, w, 1.0f);
}

} // namespace btz
