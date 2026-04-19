/*
  Box Tone Zone (BTZ) — BTZTheme.h  v2
  ────────────────────────────────────────────────────────────────────────
  Complete design-token system for the BTZ plugin UI.
  Master direction: dark obsidian / graphite / brushed-metal base,
  warm amber / champagne / soft gold accent.

  ALL visual constants live here. No file should contain hardcoded
  juce::Colour(0x...) or juce::Font(...) for styling purposes.
  Every component pulls from this system.

  Sections:
    1. Color palette (backgrounds, accents, text, meters, states)
    2. Typography (font factories for every text role)
    3. Geometry / Metrics (radii, sizes, spacing, borders)
    4. Effects (shadows, glows, meter illumination)
    5. Knob style (arc geometry constants)
    6. Component IDs (vocabulary reference)
*/
#pragma once

#include <JuceHeader.h>

namespace BTZTheme {

// ═══════════════════════════════════════════════════════════════════════════
// 1. COLOR PALETTE
// ═══════════════════════════════════════════════════════════════════════════
namespace Color {

    // ── Background layers (darkest → lightest) ──
    inline juce::Colour backgroundRoot()        { return juce::Colour(0xFF0A0A0D); }  // deepest obsidian
    inline juce::Colour backgroundPanel()       { return juce::Colour(0xFF13141A); }  // panel background
    inline juce::Colour backgroundPanelRaised() { return juce::Colour(0xFF1A1C23); }  // raised surfaces / wells
    inline juce::Colour backgroundInset()       { return juce::Colour(0xFF101218); }  // inset / recessed areas

    // ── Strokes / borders ──
    inline juce::Colour strokeSubtle()          { return juce::Colour(0xFF252830); }  // subtle dividers
    inline juce::Colour strokeStrong()          { return juce::Colour(0xFF3A3D48); }  // strong borders

    // ── Text ──
    inline juce::Colour textPrimary()           { return juce::Colour(0xFFEAE0CC); }  // cream — primary text
    inline juce::Colour textSecondary()         { return juce::Colour(0xFFD8CFB8); }  // bone — secondary text
    inline juce::Colour textDisabled()          { return juce::Colour(0xFF5A5A5A); }  // disabled / tertiary

    // ── Accent ──
    inline juce::Colour accentPrimaryAmber()    { return juce::Colour(0xFFE8A94A); }  // primary amber
    inline juce::Colour accentSecondaryChampagne() { return juce::Colour(0xFFD4B87A); } // champagne / soft gold
    inline juce::Colour accentGlow()            { return juce::Colour(0x40E8A94A); }  // amber glow (25% opacity)
    inline juce::Colour accentDim()             { return juce::Colour(0xFF9C7230); }  // dimmed amber (inactive)

    // ── Meter states ──
    inline juce::Colour meterSafe()             { return juce::Colour(0xFF4A8B6A); }  // green — nominal
    inline juce::Colour meterWarn()             { return juce::Colour(0xFFD4A843); }  // yellow — caution
    inline juce::Colour meterHot()              { return juce::Colour(0xFFD97B3E); }  // orange — hot
    inline juce::Colour meterClip()             { return juce::Colour(0xFFC0543E); }  // red — clip

    // ── Bypass / toggle ──
    inline juce::Colour bypassOn()              { return juce::Colour(0xFFE8A94A); }  // amber when active
    inline juce::Colour bypassOff()             { return juce::Colour(0xFF4A4A4A); }  // dim when bypassed

    // ── Interactive states ──
    inline juce::Colour hover()                 { return juce::Colour(0xFF252830); }  // hover highlight
    inline juce::Colour focus()                 { return juce::Colour(0xFF2A2D38); }  // focus ring fill
    inline juce::Colour selected()              { return juce::Colour(0xFFE8A94A); }  // selected / active tab
    inline juce::Colour disabled()              { return juce::Colour(0xFF3A3A3A); }  // disabled control bg

    // ── Data visualization (heat spectrum) ──
    inline juce::Colour heatCool()              { return juce::Colour(0xFF2A4858); }
    inline juce::Colour heatWarm()              { return juce::Colour(0xFFE8A94A); }
    inline juce::Colour heatHot()               { return juce::Colour(0xFFD94F6B); }
    inline juce::Colour heatPeak()              { return juce::Colour(0xFF9B59B6); }

    // ── Gain reduction ──
    inline juce::Colour grMeter()               { return juce::Colour(0xFFD4A843); }  // warm gold for GR
}

// ═══════════════════════════════════════════════════════════════════════════
// 2. TYPOGRAPHY — Font factories for every text role
// ═══════════════════════════════════════════════════════════════════════════
namespace Font {

    // Font family names (with system fallbacks)
    inline juce::String displayFamily() { return "Syne"; }
    inline juce::String bodyFamily()    { return "Inter Tight"; }
    inline juce::String monoFamily()    { return "IBM Plex Mono"; }

    // ── Factory methods: return ready-to-use juce::Font ──

    // Title: plugin name, page titles (Syne Bold 16pt)
    inline juce::Font title() {
        return juce::Font(displayFamily(), 16.0f, juce::Font::bold);
    }

    // Subtitle: section headers (Inter Tight SemiBold 12pt)
    inline juce::Font subtitle() {
        return juce::Font(bodyFamily(), 12.0f, juce::Font::bold);
    }

    // Section header: panel titles (Inter Tight Medium 11pt)
    inline juce::Font sectionHeader() {
        return juce::Font(bodyFamily(), 11.0f, juce::Font::bold);
    }

    // Label: knob/slider labels (Inter Tight Regular 9pt)
    inline juce::Font label() {
        return juce::Font(bodyFamily(), 9.0f, juce::Font::plain);
    }

    // Value: readout displays, text boxes (IBM Plex Mono 9pt)
    inline juce::Font value() {
        return juce::Font(monoFamily(), 9.0f, juce::Font::plain);
    }

    // Tooltip: hover info (Inter Tight Regular 8.5pt)
    inline juce::Font tooltip() {
        return juce::Font(bodyFamily(), 8.5f, juce::Font::plain);
    }

    // Tab: navigation tabs (Inter Tight SemiBold 10pt)
    inline juce::Font tab() {
        return juce::Font(bodyFamily(), 10.0f, juce::Font::bold);
    }

    // Meter: meter labels (IBM Plex Mono Light 8pt)
    inline juce::Font meter() {
        return juce::Font(monoFamily(), 8.0f, juce::Font::plain);
    }

    // Micro: tiny annotations (Inter Tight 7pt)
    inline juce::Font micro() {
        return juce::Font(bodyFamily(), 7.0f, juce::Font::plain);
    }

    // Brand: "BTZ Audio" branding text (Inter Tight Light 8.5pt)
    inline juce::Font brand() {
        return juce::Font(bodyFamily(), 8.5f, juce::Font::plain);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 3. GEOMETRY / METRICS
// ═══════════════════════════════════════════════════════════════════════════
namespace Geometry {

    // ── Radii ──
    constexpr float outerRadius   = 10.0f;   // plugin window corners
    constexpr float panelRadius   = 8.0f;    // panel/card corners
    constexpr float controlRadius = 4.0f;    // buttons, text boxes
    constexpr float pillRadius    = 12.0f;   // pill-shaped toggles / tabs

    // ── Knob diameters ──
    constexpr int knobLarge       = 74;      // core parameter knobs
    constexpr int knobMedium      = 60;      // macro knobs, secondary knobs
    constexpr int knobSmall       = 44;      // compact knobs (SHINE freq/Q)

    // ── Slider heights ──
    constexpr int sliderHeight    = 28;      // standard linear slider
    constexpr int sliderTrackH    = 4;       // track thickness

    // ── Spacing scale (4px base) ──
    constexpr int spaceXS         = 4;
    constexpr int spaceSM         = 8;
    constexpr int spaceMD         = 12;
    constexpr int spaceLG         = 16;
    constexpr int spaceXL         = 24;
    constexpr int spaceXXL        = 32;

    // ── Padding scale ──
    constexpr int padPanel        = 14;      // inside panels
    constexpr int padSection      = 12;      // between sections
    constexpr int padContent      = 20;      // content area margins

    // ── Border thickness ──
    constexpr float borderThin    = 0.5f;    // subtle dividers
    constexpr float borderNormal  = 1.0f;    // standard borders
    constexpr float borderThick   = 1.5f;    // emphasis borders

    // ── Plugin window ──
    constexpr int windowWidth     = 980;
    constexpr int windowHeight    = 650;

    // ── Header ──
    constexpr int headerHeight    = 54;
    constexpr int meterStripHeight = 78;

    // ── Tabs ──
    constexpr int tabWidth        = 90;
    constexpr int tabGap          = 12;
    constexpr int tabHeight       = 28;

    // ── Labels ──
    constexpr int knobLabelHeight = 16;
}

// ═══════════════════════════════════════════════════════════════════════════
// 4. EFFECTS — Shadows, glows, meter illumination
// ═══════════════════════════════════════════════════════════════════════════
namespace Effects {

    // ── Drop shadow presets ──
    struct ShadowSpec {
        juce::Colour colour;
        int radius;
        juce::Point<int> offset;
    };

    inline ShadowSpec panelShadow() {
        return { juce::Colour(0x30000000), 8, { 0, 2 } };
    }

    inline ShadowSpec controlShadow() {
        return { juce::Colour(0x20000000), 4, { 0, 1 } };
    }

    // ── Inner shadow (for inset areas) ──
    inline ShadowSpec innerShadow() {
        return { juce::Colour(0x40000000), 3, { 0, 1 } };
    }

    // ── Glow presets ──
    inline juce::Colour knobGlow()     { return juce::Colour(0x30E8A94A); }  // amber halo around active knob
    inline juce::Colour meterGlow()    { return juce::Colour(0x204A8B6A); }  // green glow behind safe meters
    inline juce::Colour clipGlow()     { return juce::Colour(0x30C0543E); }  // red glow behind clip indicators

    // ── Meter illumination ──
    constexpr float meterCornerRadius  = 2.0f;
    constexpr float meterSegmentGap    = 1.0f;
    constexpr int   meterSegmentCount  = 24;
}

// ═══════════════════════════════════════════════════════════════════════════
// 5. KNOB STYLE — Arc geometry constants
// ═══════════════════════════════════════════════════════════════════════════
namespace KnobStyle {
    constexpr float arcThicknessRatio = 0.065f;   // relative to diameter
    constexpr float bodyRadiusRatio   = 0.74f;    // inner body relative to outer
    constexpr float indicatorStart    = 0.22f;    // indicator line start (ratio)
    constexpr float indicatorEnd      = 0.62f;    // indicator line end (ratio)
    constexpr float tickDotRadius     = 2.0f;     // tick mark dot radius
    constexpr int   tickDotCount      = 11;       // number of tick dots around arc
    constexpr float startAngle        = 1.25f;    // * pi
    constexpr float endAngle          = 2.75f;    // * pi
}

// ═══════════════════════════════════════════════════════════════════════════
// 6. COMPONENT IDs — Vocabulary reference
// ═══════════════════════════════════════════════════════════════════════════
namespace ComponentID {
    constexpr int Knob       = 1;   // C/01 — rotary with tick dots, amber halo arc
    constexpr int Fader      = 2;   // C/02 — linear slider
    constexpr int Toggle     = 3;   // C/03 — on/off toggle
    constexpr int Meter      = 4;   // C/04 — level meter v2
    constexpr int PresetNav  = 5;   // C/05 — arrow + category + name
    constexpr int HeatMap    = 6;   // C/06 — radial gradient heat visualization
    constexpr int XYPad      = 7;   // C/07 — particle-style dot cluster
    constexpr int ModLane    = 8;   // C/08 — SVG waveform display
    constexpr int RoutingSlot = 9;  // C/09 — routing strip slot
    constexpr int WetDry     = 10;  // C/10 — wet/dry with topographic texture
}

// ═══════════════════════════════════════════════════════════════════════════
// 7. HELPER — Draw a drop shadow (utility for paint methods)
// ═══════════════════════════════════════════════════════════════════════════
inline void drawDropShadow(juce::Graphics& g, juce::Rectangle<int> area, const Effects::ShadowSpec& spec) {
    juce::DropShadow shadow(spec.colour, spec.radius, spec.offset);
    shadow.drawForRectangle(g, area);
}

} // namespace BTZTheme
