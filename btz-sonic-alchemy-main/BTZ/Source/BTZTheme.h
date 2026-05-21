/*
  Box Tone Zone (BTZ) — BTZTheme.h  v12 "Ivory System"
  ──────────────────────────────────────────────────────────────────────────
  SINGLE SOURCE OF TRUTH for every visual constant.

  Aesthetic: BTZ Ivory System
    • Modern analog warmth, minimal luxury, intelligent tone design
    • Warm ivory/cream canvas — never cold white, never dark
    • Orange primary action accent, sage green secondary
    • Muted gold supporting accent, clay/terracotta warnings
    • Soft ceramic knobs with subtle bevel + inner shadow
    • Clean spacing, beautiful proportions, soft shadows
    • Uppercase labels, monospace values, architectural type
    • Calm, tactile, premium — like modern furniture meets analog gear

  If a hex value appears in paint() that isn't from this header, it's a bug.
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>

namespace btz {

// ═══════════════════════════════════════════════════════════════════════════
// Palette — BTZ Ivory System
// ═══════════════════════════════════════════════════════════════════════════

namespace palette {
    // Surfaces (warm ivory foundation — never pure white)
    inline constexpr uint32_t canvas       = 0xFFF4EFE4;  // Ivory Cream
    inline constexpr uint32_t surface      = 0xFFFAF7EF;  // Warm Porcelain
    inline constexpr uint32_t surfaceAlt   = 0xFFEFE7D8;  // Soft Linen
    inline constexpr uint32_t well         = 0xFFE2D5C2;  // Warm Sand
    inline constexpr uint32_t border       = 0xFFD8CDBC;  // Panel Border
    inline constexpr uint32_t borderSubtle = 0xFFCFC5B5;  // Muted Line Gray

    // Text (warm charcoal hierarchy — never pure black)
    inline constexpr uint32_t ink          = 0xFF2B2924;  // Charcoal
    inline constexpr uint32_t inkStrong    = 0xFF171613;  // Soft Black (logo only)
    inline constexpr uint32_t inkMuted     = 0xFF837B6E;  // Warm Gray
    inline constexpr uint32_t inkFaint     = 0xFFA89E92;  // Faint labels
    inline constexpr uint32_t inkDisabled  = 0xFFCFC5B5;  // Disabled

    // Primary accent: Warm Orange (action, drive, saturation, focus)
    inline constexpr uint32_t orange       = 0xFFD8662A;  // BTZ Warm Orange
    inline constexpr uint32_t orangeHover  = 0xFFE5783A;  // Hover state
    inline constexpr uint32_t orangePressed= 0xFFC75A2A;  // Soft Burnt Orange
    inline constexpr uint32_t orangeDeep   = 0xFFB95E3D;  // Clay Orange
    inline constexpr uint32_t orangeFaint  = 0xFFFBF0E4;  // Faint background

    // Secondary accent: Sage Green (tone, output, safety, balance)
    inline constexpr uint32_t sage         = 0xFF7E9475;  // BTZ Sage
    inline constexpr uint32_t sageHover    = 0xFF8A987C;  // Muted Olive Sage
    inline constexpr uint32_t sagePressed  = 0xFF5F705D;  // Deep Sage
    inline constexpr uint32_t sageMuted    = 0xFFA1AA8F;  // Soft Moss
    inline constexpr uint32_t sageFaint    = 0xFFEEF5F0;  // Faint background

    // Supporting accent: Muted Gold (harmonic sheen, premium highlights)
    inline constexpr uint32_t gold         = 0xFFC9A24D;  // Soft Brass
    inline constexpr uint32_t goldBright   = 0xFFD6B45F;  // Muted Ochre
    inline constexpr uint32_t goldFaint    = 0xFFDDC894;  // Champagne Gold
    inline constexpr uint32_t goldBg       = 0xFFFFF8E8;  // Faint background

    // Warning: Clay/Terracotta (clipping, danger, excessive GR)
    inline constexpr uint32_t clay         = 0xFFC05A48;
    inline constexpr uint32_t clayFaint    = 0xFFFDF2EE;

    // Utility: Teal (modulation, focus rings, LFO)
    inline constexpr uint32_t teal         = 0xFF5A8A8C;
    inline constexpr uint32_t tealFaint    = 0xFFECF5F5;

    // Highlight Amber (active energy, harmonic peaks)
    inline constexpr uint32_t amber        = 0xFFE5A94B;

    // Knob (ceramic/matte enamel feel)
    inline constexpr uint32_t knobFace     = 0xFFFFFEFA;  // Warm white ceramic
    inline constexpr uint32_t knobBevel    = 0xFFEDE8E0;  // Subtle bevel edge
    inline constexpr uint32_t knobTrack    = 0xFFE2D5C2;  // Inactive arc
    inline constexpr uint32_t knobShadow   = 0x18000000;  // Soft drop shadow
    inline constexpr uint32_t knobInner    = 0x08000000;  // Inner shadow

    // Meters (sage=safe, gold=approaching, orange=energetic, clay=warning)
    inline constexpr uint32_t meterSafe    = 0xFF7E9475;
    inline constexpr uint32_t meterActive  = 0xFFD6B45F;
    inline constexpr uint32_t meterHot     = 0xFFD8662A;
    inline constexpr uint32_t meterClip    = 0xFFC05A48;
    inline constexpr uint32_t meterTrack   = 0xFFE8E3DC;

    // Harmonics (8-colour cycle: orange/gold/clay/amber/sage/teal variants)
    inline constexpr uint32_t harm[8] = {
        0xFFD8662A, 0xFFE5A94B, 0xFFC9A24D, 0xFFB95E3D,
        0xFF7E9475, 0xFF5A8A8C, 0xFF8A987C, 0xFFC75A2A
    };

    // Before/After analyzer colours
    inline constexpr uint32_t analyzerBefore = 0xFF837B6E;  // Gray (before)
    inline constexpr uint32_t analyzerAfter  = 0xFF7E9475;  // Sage fill (after)
    inline constexpr uint32_t analyzerHarm   = 0xFFD8662A;  // Orange (added harmonics)
    inline constexpr uint32_t analyzerReduce = 0xFFC05A48;  // Clay (reduction)
}

// ═══════════════════════════════════════════════════════════════════════════
// Typography — Clean, modern, slightly architectural
// ═══════════════════════════════════════════════════════════════════════════

namespace type {
    // Families
    inline const char* sans()    { return "Inter"; }
    inline const char* mono()    { return "JetBrains Mono"; }
    inline const char* display() { return "Inter"; }  // Brand/header display face

    // Scale (px) — architectural hierarchy
    inline constexpr float brand  = 18.0f;   // Logo/brand wordmark
    inline constexpr float title  = 15.0f;   // Section headers
    inline constexpr float h1     = 13.0f;   // Panel headers
    inline constexpr float h2     = 11.5f;   // Sub-headers
    inline constexpr float body   = 10.5f;   // Body text
    inline constexpr float label  = 9.0f;    // Control labels (uppercase)
    inline constexpr float small  = 8.5f;    // Footer/status text
    inline constexpr float micro  = 7.5f;    // Tiny annotations
    inline constexpr float value  = 10.0f;   // Monospace numeric values

    // Tracking (letter-spacing multiplier)
    inline constexpr float trackBrand = 0.18f;  // Wide brand spacing
    inline constexpr float trackLabel = 0.10f;  // Uppercase labels
    inline constexpr float trackBody  = 0.0f;   // Normal body
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
    // Window dimensions
    inline constexpr int windowW     = 1120;
    inline constexpr int windowH     = 700;
    inline constexpr int defaultW    = 1120;
    inline constexpr int defaultH    = 700;
    inline constexpr int maxW        = 2240;
    inline constexpr int maxH        = 1400;
    inline constexpr int minW        = 840;
    inline constexpr int minH        = 520;

    // Structural zones
    inline constexpr int headerH     = 38;
    inline constexpr int footerH     = 30;
    inline constexpr int tabH        = 28;

    // Knob sizes (ceramic style)
    inline constexpr int knobXL      = 100;  // Simple Mode — large tactile
    inline constexpr int knobLG      = 60;   // Character knobs
    inline constexpr int knobMD      = 46;   // Macro knobs
    inline constexpr int knobSM      = 36;   // Utility knobs

    // Meters
    inline constexpr int meterW      = 5;
    inline constexpr int meterGap    = 2;

    // Buttons
    inline constexpr int buttonH     = 26;
    inline constexpr int pillH       = 22;   // Pill-shaped tab buttons
}

namespace radius {
    inline constexpr float sm   = 3.0f;
    inline constexpr float md   = 5.0f;
    inline constexpr float lg   = 8.0f;
    inline constexpr float pill = 999.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Knob Rendering — Ceramic/matte enamel style
// ═══════════════════════════════════════════════════════════════════════════

namespace knob {
    inline constexpr float arcWidth      = 3.0f;
    inline constexpr float arcGapRad     = 0.45f;   // Gap at bottom (radians)
    inline constexpr float pointerStart  = 0.30f;   // Fraction of radius
    inline constexpr float pointerEnd    = 0.55f;
    inline constexpr float pointerWidth  = 1.6f;
    inline constexpr float shadowOffset  = 1.5f;
    inline constexpr float shadowRadius  = 3.0f;
    inline constexpr float bevelWidth    = 1.0f;    // Subtle edge bevel
    inline constexpr float innerShadow   = 0.5f;    // Soft inner shadow depth
    inline constexpr float hoverLift     = 0.5f;    // Shadow lift on hover
}

// ═══════════════════════════════════════════════════════════════════════════
// Animation — Responsive but restrained
// ═══════════════════════════════════════════════════════════════════════════

namespace anim {
    inline constexpr int fps          = 30;
    inline constexpr float meterDecay = 0.12f;   // dB per frame (gentle easing)
    inline constexpr int hoverMs      = 80;      // Hover transition
    inline constexpr int fadeMs       = 150;     // Fade transitions
    inline constexpr int arcMs        = 60;      // Arc fill transition
}

// ═══════════════════════════════════════════════════════════════════════════
// Component IDs (for accent colour routing)
// ═══════════════════════════════════════════════════════════════════════════

namespace id {
    // Core controls
    inline constexpr int drive     = 1001;
    inline constexpr int mix       = 1002;
    inline constexpr int output    = 1003;
    inline constexpr int master    = 1004;

    // Character macros (Standard Mode)
    inline constexpr int punch     = 1010;
    inline constexpr int warmth    = 1011;
    inline constexpr int boom      = 1012;
    inline constexpr int glue      = 1013;
    inline constexpr int air       = 1014;
    inline constexpr int width     = 1015;

    // Extended macros (Standard Mode bottom row)
    inline constexpr int density   = 1020;
    inline constexpr int motion    = 1021;
    inline constexpr int era       = 1022;
    inline constexpr int intensity = 1023;

    // Simple Mode
    inline constexpr int tone      = 1030;

    // Advanced controls
    inline constexpr int shine     = 1040;
    inline constexpr int ceiling   = 1041;
    inline constexpr int resTame   = 1042;
    inline constexpr int transient = 1043;
    inline constexpr int macro     = 1044;
}

// ═══════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════

// Accent colour for a given component ID
inline juce::Colour accentFor(int componentID) noexcept {
    switch (componentID) {
        // Orange family (drive, saturation, energy, transients)
        case id::drive:     return juce::Colour(palette::orange);
        case id::punch:     return juce::Colour(palette::orange);
        case id::warmth:    return juce::Colour(palette::orange);
        case id::boom:      return juce::Colour(palette::orange);
        case id::intensity: return juce::Colour(palette::amber);
        case id::transient: return juce::Colour(palette::orange);

        // Sage family (tone, output, safety, balance)
        case id::mix:       return juce::Colour(palette::sage);
        case id::output:    return juce::Colour(palette::sage);
        case id::master:    return juce::Colour(palette::sage);
        case id::glue:      return juce::Colour(palette::sage);
        case id::air:       return juce::Colour(palette::sage);
        case id::tone:      return juce::Colour(palette::sage);
        case id::shine:     return juce::Colour(palette::sage);

        // Gold family (harmonic sheen, premium, density)
        case id::density:   return juce::Colour(palette::gold);
        case id::era:       return juce::Colour(palette::gold);
        case id::ceiling:   return juce::Colour(palette::gold);

        // Teal family (modulation, width, spatial)
        case id::width:     return juce::Colour(palette::teal);
        case id::motion:    return juce::Colour(palette::teal);
        case id::macro:     return juce::Colour(palette::teal);
        case id::resTame:   return juce::Colour(palette::teal);

        default:            return juce::Colour(palette::sage);
    }
}

// Harmonic colour by index (wraps at 8)
inline juce::Colour harmonicColour(int idx) noexcept {
    return juce::Colour(palette::harm[idx & 7]);
}

// ─── Drawing Helpers ─────────────────────────────────────────────────────

// Paint a BTZ ceramic-style knob
inline void paintKnob(juce::Graphics& g, juce::Rectangle<float> bounds,
                      float normValue, int componentID, bool hovered = false) {
    const auto accent = accentFor(componentID);
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float r  = bounds.getWidth() * 0.5f - knob::arcWidth;

    const float startAngle = juce::MathConstants<float>::pi + knob::arcGapRad;
    const float endAngle   = 3.0f * juce::MathConstants<float>::pi - knob::arcGapRad;

    // Drop shadow (lifts on hover)
    const float lift = hovered ? knob::hoverLift : 0.0f;
    g.setColour(juce::Colour(palette::knobShadow));
    g.fillEllipse(bounds.translated(0.0f, knob::shadowOffset + lift)
                        .reduced(knob::arcWidth));

    // Bevel ring (subtle edge)
    g.setColour(juce::Colour(palette::knobBevel));
    g.fillEllipse(bounds.reduced(knob::arcWidth));

    // Face (ceramic white)
    g.setColour(juce::Colour(palette::knobFace));
    g.fillEllipse(bounds.reduced(knob::arcWidth + knob::bevelWidth));

    // Inner shadow (soft depth)
    g.setColour(juce::Colour(palette::knobInner));
    g.drawEllipse(bounds.reduced(knob::arcWidth + knob::bevelWidth + 1.0f), 1.0f);

    // Track arc (inactive)
    juce::Path track;
    track.addCentredArc(cx, cy, r, r, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(palette::knobTrack));
    g.strokePath(track, juce::PathStrokeType(knob::arcWidth,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc (active — orange/sage/gold depending on component)
    const float valueAngle = startAngle + normValue * (endAngle - startAngle);
    if (normValue > 0.002f) {
        juce::Path arc;
        arc.addCentredArc(cx, cy, r, r, 0.0f, startAngle, valueAngle, true);
        g.setColour(hovered ? accent.brighter(0.08f) : accent);
        g.strokePath(arc, juce::PathStrokeType(knob::arcWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Pointer (thin charcoal line)
    const float innerR = r * knob::pointerStart;
    const float outerR = r * knob::pointerEnd;
    const float angle  = valueAngle - juce::MathConstants<float>::halfPi;
    g.setColour(juce::Colour(palette::ink));
    g.drawLine(cx + innerR * std::cos(angle), cy + innerR * std::sin(angle),
               cx + outerR * std::cos(angle), cy + outerR * std::sin(angle),
               knob::pointerWidth);
}

// Paint a vertical meter bar
inline void paintMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                       float normLevel, bool isGR = false) {
    g.setColour(juce::Colour(palette::meterTrack));
    g.fillRoundedRectangle(bounds.toFloat(), radius::sm);

    if (normLevel < 0.002f) return;

    const int fillH = static_cast<int>(bounds.getHeight() * normLevel);
    auto fill = bounds.removeFromBottom(fillH);

    juce::Colour c;
    if (isGR)                    c = juce::Colour(palette::orange);
    else if (normLevel > 0.92f)  c = juce::Colour(palette::meterClip);
    else if (normLevel > 0.75f)  c = juce::Colour(palette::meterHot);
    else if (normLevel > 0.55f)  c = juce::Colour(palette::meterActive);
    else                         c = juce::Colour(palette::meterSafe);

    g.setColour(c);
    g.fillRoundedRectangle(fill.toFloat(), radius::sm);
}

// Paint warm ivory background
inline void paintBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colour(palette::canvas));
    g.fillRect(area);
}

// Paint a panel surface
inline void paintPanel(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(juce::Colour(palette::surface));
    g.fillRoundedRectangle(area, radius::md);
}

// Paint a subtle horizontal rule
inline void paintRule(juce::Graphics& g, float x, float y, float w) {
    g.setColour(juce::Colour(palette::borderSubtle));
    g.fillRect(x, y, w, 1.0f);
}

// Paint a well (recessed area)
inline void paintWell(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(juce::Colour(palette::well));
    g.fillRoundedRectangle(area, radius::md);
}

// ─── Tooltip Strings ─────────────────────────────────────────────────────

namespace tooltip {
    inline const char* drive     = "Controls saturation intensity. Higher values add harmonic richness and grit.";
    inline const char* tone      = "Shapes the tonal character. Left = darker/warmer, Right = brighter/airier.";
    inline const char* mix       = "Blends between dry (original) and wet (processed) signal.";
    inline const char* output    = "Final output level. Watch the meters to avoid clipping.";
    inline const char* punch     = "Adds transient punch and low-mid compression timing. Great on drums and vocals.";
    inline const char* warmth    = "Increases low-mid harmonic density with transformer/tape saturation character.";
    inline const char* boom      = "Adds low-end body and weight. Use sparingly on bass-heavy material.";
    inline const char* glue      = "Adds compression-like cohesion and harmonic binding. Use lightly on mix bus.";
    inline const char* air       = "High-frequency harmonic lift with subtle exciter and phase-safe widening.";
    inline const char* width     = "Stereo width control. Center = mono, full right = maximum stereo spread.";
    inline const char* density   = "Increases harmonic density and saturation complexity.";
    inline const char* motion    = "LFO modulation depth. Adds subtle movement to the saturation character.";
    inline const char* era       = "Shifts between vintage (left) and modern (right) saturation voicing.";
    inline const char* intensity = "Overall processing intensity. Controls how aggressively the plugin shapes tone.";
    inline const char* ceiling   = "True peak limiter ceiling. Set to -1.0 dBTP for streaming, -0.3 for mastering.";
    inline const char* shine     = "High-frequency presence enhancement. Adds air and sparkle above 8kHz.";
}

} // namespace btz
