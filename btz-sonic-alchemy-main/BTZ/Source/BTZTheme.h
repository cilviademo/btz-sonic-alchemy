/*
  Box Tone Zone (BTZ) — BTZTheme.h  v3
  ────────────────────────────────────────────────────────────────────────
  Ecosystem-aligned design token system.
  Merges BTZColours.h + BTZTokens.h from the BTZ Ecosystem v1.0 spec
  into a single header for the JUCE plugin build.

  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.

  Sections:
    1. BTZColours — chassis, ink, accents, meters, states, heat
    2. BTZTokens  — spacing, radius, border, typography, timing, dimensions
    3. Legacy compat aliases (BTZTheme::Color, Font, Geometry, etc.)
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — the canonical color token system
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Chassis tier ──
    static const juce::Colour obsidian  { 0xFF0A0A0D };
    static const juce::Colour panel     { 0xFF13141A };
    static const juce::Colour charcoal  { 0xFF1A1C23 };
    static const juce::Colour stone     { 0xFF262932 };
    static const juce::Colour hairline  { 0xFF383B45 };
    static const juce::Colour rule      { 0xFF20232A };

    // ── Ink & typography ──
    static const juce::Colour cream     { 0xFFEAE0CC };
    static const juce::Colour bone      { 0xFFD8CFB8 };
    static const juce::Colour paper     { 0xFFB8AE98 };
    static const juce::Colour mute      { 0xFF7A7465 };
    static const juce::Colour deepMute  { 0xFF52503F };

    // ── Knob body gradient ──
    static const juce::Colour knobHighlight { 0xFFF4ECD8 };
    static const juce::Colour knobMid       { 0xFFEAE0CC };
    static const juce::Colour knobShadow    { 0xFFC2B89C };

    // ── Module accents ──
    static const juce::Colour amber       { 0xFFE8A94A };
    static const juce::Colour amberDim    { 0xFFA07A32 };
    static const juce::Colour amberDark   { 0xFF6E5423 };

    static const juce::Colour coral       { 0xFFE8624A };
    static const juce::Colour coralDim    { 0xFFA04432 };
    static const juce::Colour coralDark   { 0xFF6E2F23 };

    static const juce::Colour cyan        { 0xFF6AD4E8 };
    static const juce::Colour cyanDim     { 0xFF3A92A3 };
    static const juce::Colour cyanDark    { 0xFF1F5A67 };

    // ── Reserved future accents ──
    static const juce::Colour emerald     { 0xFF4AE8A0 };
    static const juce::Colour violet      { 0xFF7B5BE8 };
    static const juce::Colour sulfur      { 0xFFE8D94A };

    // ── Heat spectrum (visualizer only) ──
    static const juce::Colour heatOrange   { 0xFFF28A3A };
    static const juce::Colour heatWarm     { 0xFFE8624A };
    static const juce::Colour heatMagenta  { 0xFFD94A8E };
    static const juce::Colour heatViolet   { 0xFF7B5BE8 };
    static const juce::Colour heatDeep     { 0xFF3A2E8E };

    // ── State colors ──
    static const juce::Colour stateSafe    { 0xFF4AE8A0 };
    static const juce::Colour stateWarn    { 0xFFE8A94A };
    static const juce::Colour stateDanger  { 0xFFE8624A };
    static const juce::Colour stateClip    { 0xFFFF3A1A };

    // ── Meter zones ──
    static const juce::Colour meterSafe    { 0xFF383B45 };
    static const juce::Colour meterOptimal { 0xFFE8A94A };
    static const juce::Colour meterHot     { 0xFFE8624A };
    static const juce::Colour meterClip    { 0xFFFF3A1A };

    // ── Glow alpha values ──
    constexpr float amberGlowAlpha     = 0.25f;   // v6: reduced from 0.4 for restraint
    constexpr float amberSoftGlowAlpha = 0.15f;   // v6: reduced from 0.3
    constexpr float coralGlowAlpha     = 0.20f;   // v6: reduced from 0.35
    constexpr float cyanGlowAlpha      = 0.20f;

    // ── Module accessor ──
    enum class Module { BTZ, SPARK, SHINE, TONE, MOTION, BLOOM, Utility };

    inline juce::Colour accentFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return amber;
            case Module::SPARK:   return coral;
            case Module::SHINE:   return cyan;
            case Module::TONE:    return emerald;
            case Module::MOTION:  return violet;
            case Module::BLOOM:   return sulfur;
            case Module::Utility: return bone;
        }
        return amber;
    }

    inline juce::Colour accentDimFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return amberDim;
            case Module::SPARK:   return coralDim;
            case Module::SHINE:   return cyanDim;
            default:              return mute;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 2. BTZTokens — spacing, radius, border, typography, timing, dimensions
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZTokens {

    namespace Space {
        constexpr int xs    = 4;
        constexpr int sm    = 8;
        constexpr int md    = 12;
        constexpr int lg    = 16;
        constexpr int xl    = 20;
        constexpr int xxl   = 24;
        constexpr int sec   = 32;
        constexpr int secMd = 40;
        constexpr int secLg = 48;
        constexpr int page  = 64;
    }

    namespace Radius {
        constexpr float none = 0.0f;
        constexpr float full = 9999.0f;
    }

    namespace Border {
        constexpr float hairline = 1.0f;
        constexpr float focus    = 1.5f;
    }

    namespace Font {
        static const juce::String display = "Syne";
        static const juce::String ui      = "Inter Tight";
        static const juce::String mono    = "IBM Plex Mono";

        namespace Size {
            constexpr float size3xl  = 82.0f;
            constexpr float size2xl  = 62.0f;
            constexpr float sizeXl   = 32.0f;
            constexpr float sizeLg   = 22.0f;
            constexpr float sizeMd   = 16.0f;
            constexpr float sizeBase = 14.0f;
            constexpr float sizeSm   = 12.0f;
            constexpr float sizeXs   = 11.0f;
            constexpr float size2xs  = 10.0f;
            constexpr float size3xs  = 9.0f;
        }

        namespace Tracking {
            constexpr float tight = -0.03f;
            constexpr float body  = -0.005f;
            constexpr float label = 0.20f;
            constexpr float wide  = 0.22f;
        }
    }

    namespace Duration {
        constexpr int instant = 0;
        constexpr int fast    = 120;
        constexpr int base    = 200;
        constexpr int slow    = 400;
        constexpr int tooltip = 500;
    }

    namespace Dim {
        constexpr int macroKnobDefault = 110;
        constexpr int macroKnobMin     = 72;
        constexpr int macroKnobMax     = 140;
        constexpr int utilityKnobDefault = 48;
        constexpr int brandLineHeight  = 52;
        constexpr int outputGuardWidth = 120;
        constexpr int tickDotCount     = 11;
        constexpr float tickDotSpanDegrees = 240.0f;
        constexpr float haloStartAngleDegrees = 225.0f;
        constexpr float haloSweepDegrees = 270.0f;
        constexpr int indicatorWidth   = 2;
        constexpr int indicatorHeight  = 14;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.3f;
        constexpr float bypassed   = 0.4f;
        constexpr float background = 0.7f;
    }

    namespace Window {
        constexpr int defaultWidth  = 1280;
        constexpr int defaultHeight = 800;
        constexpr int minWidth      = 960;
        constexpr int minHeight     = 600;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 3. Legacy compatibility aliases — BTZTheme:: namespace
//    These map old BTZTheme::Color/Font/Geometry calls to the new tokens.
//    Allows incremental migration without breaking existing code.
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZTheme {

namespace Color {
    inline juce::Colour backgroundRoot()        { return BTZColours::obsidian; }
    inline juce::Colour backgroundPanel()       { return BTZColours::panel; }
    inline juce::Colour backgroundPanelRaised() { return BTZColours::charcoal; }
    inline juce::Colour backgroundInset()       { return juce::Colour(0xFF101218); }
    inline juce::Colour strokeSubtle()          { return BTZColours::hairline; }
    inline juce::Colour strokeStrong()          { return juce::Colour(0xFF3A3D48); }
    inline juce::Colour textPrimary()           { return BTZColours::cream; }
    inline juce::Colour textSecondary()         { return BTZColours::bone; }
    inline juce::Colour textDisabled()          { return BTZColours::mute; }
    inline juce::Colour accentPrimaryAmber()    { return BTZColours::amber; }
    inline juce::Colour accentSecondaryChampagne() { return juce::Colour(0xFFD4B87A); }
    inline juce::Colour accentGlow()            { return BTZColours::amber.withAlpha(BTZColours::amberGlowAlpha); }
    inline juce::Colour accentDim()             { return BTZColours::amberDim; }
    inline juce::Colour meterSafe()             { return BTZColours::meterSafe; }
    inline juce::Colour meterWarn()             { return BTZColours::meterOptimal; }
    inline juce::Colour meterHot()              { return BTZColours::meterHot; }
    inline juce::Colour meterClip()             { return BTZColours::meterClip; }
    inline juce::Colour bypassOn()              { return BTZColours::amber; }
    inline juce::Colour bypassOff()             { return BTZColours::stone; }
    inline juce::Colour hover()                 { return BTZColours::stone; }
    inline juce::Colour focus()                 { return BTZColours::charcoal; }
    inline juce::Colour selected()              { return BTZColours::amber; }
    inline juce::Colour disabled()              { return BTZColours::hairline; }
    inline juce::Colour grMeter()               { return BTZColours::amber; }
}

namespace Font {
    inline juce::String displayFamily() { return BTZTokens::Font::display; }
    inline juce::String bodyFamily()    { return BTZTokens::Font::ui; }
    inline juce::String monoFamily()    { return BTZTokens::Font::mono; }

    inline juce::Font title()         { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font subtitle()      { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font sectionHeader() { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeXs, juce::Font::bold); }
    inline juce::Font label()         { return juce::Font(bodyFamily(), BTZTokens::Font::Size::size3xs, juce::Font::plain); }
    inline juce::Font value()         { return juce::Font(monoFamily(), BTZTokens::Font::Size::size3xs, juce::Font::plain); }
    inline juce::Font tooltip()       { return juce::Font(bodyFamily(), 8.5f, juce::Font::plain); }
    inline juce::Font tab()           { return juce::Font(bodyFamily(), BTZTokens::Font::Size::size2xs, juce::Font::bold); }
    inline juce::Font meter()         { return juce::Font(monoFamily(), 8.0f, juce::Font::plain); }
    inline juce::Font micro()         { return juce::Font(bodyFamily(), 7.0f, juce::Font::plain); }
    inline juce::Font brand()         { return juce::Font(bodyFamily(), 8.5f, juce::Font::plain); }
    inline juce::Font macroLabel()    { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeLg, juce::Font::bold); }
    inline juce::Font macroValue()    { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::plain); }
}

namespace Geometry {
    constexpr float outerRadius   = 0.0f;    // ecosystem: no rounded window corners
    constexpr float panelRadius   = 0.0f;    // ecosystem: Radius::none for panels
    constexpr float controlRadius = 0.0f;    // ecosystem: Radius::none for controls
    constexpr float pillRadius    = 12.0f;

    constexpr int knobLarge       = 74;
    constexpr int knobMedium      = 60;
    constexpr int knobSmall       = 44;
    constexpr int knobMacro       = BTZTokens::Dim::macroKnobDefault;  // 110

    constexpr int sliderHeight    = 28;
    constexpr int sliderTrackH    = 4;

    constexpr int spaceXS         = BTZTokens::Space::xs;
    constexpr int spaceSM         = BTZTokens::Space::sm;
    constexpr int spaceMD         = BTZTokens::Space::md;
    constexpr int spaceLG         = BTZTokens::Space::lg;
    constexpr int spaceXL         = BTZTokens::Space::xxl;
    constexpr int spaceXXL        = BTZTokens::Space::sec;

    constexpr int padPanel        = BTZTokens::Space::lg;
    constexpr int padSection      = BTZTokens::Space::md;
    constexpr int padContent      = BTZTokens::Space::xl;

    constexpr float borderThin    = 0.5f;
    constexpr float borderNormal  = BTZTokens::Border::hairline;
    constexpr float borderThick   = BTZTokens::Border::focus;

    constexpr int windowWidth     = BTZTokens::Window::defaultWidth;
    constexpr int windowHeight    = BTZTokens::Window::defaultHeight;

    constexpr int headerHeight    = BTZTokens::Dim::brandLineHeight;
    constexpr int meterStripHeight = 78;

    constexpr int tabWidth        = 100;
    constexpr int tabGap          = BTZTokens::Space::md;
    constexpr int tabHeight       = 30;

    constexpr int knobLabelHeight = 18;
}

namespace Effects {
    struct ShadowSpec {
        juce::Colour colour;
        int radius;
        juce::Point<int> offset;
    };

    inline ShadowSpec panelShadow()   { return { juce::Colour(0x30000000), 8, { 0, 2 } }; }
    inline ShadowSpec controlShadow() { return { juce::Colour(0x20000000), 4, { 0, 1 } }; }
    inline ShadowSpec innerShadow()   { return { juce::Colour(0x40000000), 3, { 0, 1 } }; }

    inline juce::Colour knobGlow()    { return BTZColours::amber.withAlpha(BTZColours::amberSoftGlowAlpha); }
    inline juce::Colour meterGlow()   { return juce::Colour(0x204A8B6A); }
    inline juce::Colour clipGlow()    { return BTZColours::stateClip.withAlpha(0.2f); }

    constexpr float meterCornerRadius = 2.0f;
    constexpr float meterSegmentGap   = 1.0f;
    constexpr int   meterSegmentCount = 24;
}

namespace KnobStyle {
    constexpr float arcThicknessRatio = 0.065f;
    constexpr float bodyRadiusRatio   = 0.74f;
    constexpr float indicatorStart    = 0.22f;
    constexpr float indicatorEnd      = 0.62f;
    constexpr float tickDotRadius     = 2.0f;
    constexpr int   tickDotCount      = BTZTokens::Dim::tickDotCount;
    constexpr float startAngle        = 1.25f;  // * pi
    constexpr float endAngle          = 2.75f;  // * pi
}

namespace ComponentID {
    constexpr int Knob       = 1;
    constexpr int Fader      = 2;
    constexpr int Toggle     = 3;
    constexpr int Meter      = 4;
    constexpr int PresetNav  = 5;
    constexpr int HeatMap    = 6;
    constexpr int XYPad      = 7;
    constexpr int ModLane    = 8;
    constexpr int RoutingSlot = 9;
    constexpr int WetDry     = 10;
}

inline void drawDropShadow(juce::Graphics& g, juce::Rectangle<int> area, const Effects::ShadowSpec& spec) {
    juce::DropShadow shadow(spec.colour, spec.radius, spec.offset);
    shadow.drawForRectangle(g, area);
}

} // namespace BTZTheme
