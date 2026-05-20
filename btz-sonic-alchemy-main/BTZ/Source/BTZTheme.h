/*
  Box Tone Zone (BTZ) — BTZTheme.h  v6
  ────────────────────────────────────────────────────────────────────────
  v6 (Wes Anderson Edition):
    • Inspired by the visual language of Wes Anderson films:
      - Perfect symmetry and centered compositions
      - Warm pastel palette: parchment, mustard gold, sage, burnt sienna
      - Retro serif/geometric typography with wide letter-spacing
      - Deliberate thin borders framing every element (diorama aesthetic)
      - Flat matte color blocks, minimal gradients
      - Vintage analog hardware feel (1960s studio equipment)
      - Whimsy in restraint — playful but controlled
      - Ornamental thin rules as section dividers

    • Palette lineage: "Fantastic Mr. Fox" warmth + "Moonrise Kingdom" sage
      + "Grand Budapest Hotel" dusty rose + "Life Aquatic" teal accent
    • Retains all v5 UX infrastructure (animation, accessibility, etc.)

  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.
  ────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — Wes Anderson Palette
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Background tier (warm parchment — aged paper feel) ──
    static const juce::Colour canvas        { 0xFFF4EDE4 };  // primary — warm parchment
    static const juce::Colour canvasWarm    { 0xFFF8F2EA };  // lighter center
    static const juce::Colour panel         { 0xFFEDE5D8 };  // panel — slightly warmer
    static const juce::Colour panelRaised   { 0xFFF2EBE0 };  // elevated panel
    static const juce::Colour well          { 0xFFE2D9CA };  // inset/well
    static const juce::Colour wellDeep      { 0xFFD6CCBC };  // deeper inset
    static const juce::Colour hairline      { 0xFF8B7355 };  // warm brown borders (deliberate, visible)
    static const juce::Colour rule          { 0xFFB8A88E };  // thin decorative rules

    // ── Ink & typography (warm dark brown — never pure black) ──
    static const juce::Colour text          { 0xFF2C2418 };  // primary text — deep warm brown
    static const juce::Colour textSecondary { 0xFF5C4E3C };  // secondary — medium brown
    static const juce::Colour textTertiary  { 0xFF8B7D6B };  // muted labels
    static const juce::Colour textDisabled  { 0xFFB8AE9E };  // disabled state
    static const juce::Colour textInverse   { 0xFFF4EDE4 };  // text on dark surfaces

    // ── Knob body (cream with vintage warmth, like bakelite) ──
    static const juce::Colour knobBody      { 0xFFF0E8DA };  // main knob fill — warm cream
    static const juce::Colour knobHighlight { 0xFFFAF6F0 };  // top rim highlight
    static const juce::Colour knobShadow    { 0xFFD6CCBC };  // bottom shadow
    static const juce::Colour knobInnerShadow { 0xFFC8BCA8 };  // convexity shadow
    static const juce::Colour knobRimLight  { 0xFFFFFDF8 };  // top rim — warm white
    static const juce::Colour knobPointer   { 0xFF2C2418 };  // indicator line — dark brown

    // ── Primary accent: Burnt Sienna (Fantastic Mr. Fox warmth) ──
    static const juce::Colour oak           { 0xFFC17A4A };  // primary — burnt sienna/rust
    static const juce::Colour oakBright     { 0xFFD4915E };  // active/hover
    static const juce::Colour oakDim        { 0xFFA06238 };  // dimmed
    static const juce::Colour oakDark       { 0xFF7A4A2A };  // dark variant
    static const juce::Colour oakLight      { 0xFFE8B08A };  // light variant (dusty peach)

    // ── Secondary accent: Sage (Moonrise Kingdom green) ──
    static const juce::Colour sage          { 0xFFA8B5A0 };  // secondary — muted sage
    static const juce::Colour sageBright    { 0xFFBEC9B6 };  // active/hover
    static const juce::Colour sageDim       { 0xFF7E8B76 };  // dimmed
    static const juce::Colour sageDark      { 0xFF5A6654 };  // dark variant
    static const juce::Colour sageLight     { 0xFFD0D9CA };  // light variant

    // ── Tertiary accent: Dusty Rose (Grand Budapest warmth) ──
    static const juce::Colour terracotta    { 0xFFC4887A };  // dusty rose/warm pink-brown
    static const juce::Colour terracottaDim { 0xFFA06E62 };  // dimmed
    static const juce::Colour terracottaLight { 0xFFDEA89C }; // light variant

    // ── Accent 4: Dusty Teal (Life Aquatic) ──
    static const juce::Colour teal          { 0xFF6B9B9E };  // muted blue-green
    static const juce::Colour tealBright    { 0xFF82B2B5 };  // active
    static const juce::Colour tealDim       { 0xFF4E7A7D };  // dimmed

    // ── Accent 5: Mustard Gold (Grand Budapest / Mr. Fox) ──
    static const juce::Colour mustard       { 0xFFE8C87A };  // warm gold
    static const juce::Colour mustardBright { 0xFFF0D88E };  // active
    static const juce::Colour mustardDim    { 0xFFC4A85E };  // dimmed
    static const juce::Colour mustardDark   { 0xFF9A8444 };  // dark

    // ── Glassmorphism (adapted for parchment — frosted cream) ──
    static const juce::Colour glassLight    { 0x30FFFFFF };  // 19% white overlay
    static const juce::Colour glassMedium   { 0x50FFFFFF };  // 31% white overlay
    static const juce::Colour glassBorder   { 0x40FFFFFF };  // 25% white border
    static const juce::Colour glassHighlight{ 0x66FFFFFF };  // 40% white top edge
    static const juce::Colour glassShadow   { 0x14000000 };  // 8% black bottom

    // ── Harmonic overtone colors (Wes Anderson warm palette) ──
    static const juce::Colour harmonic1     { 0xFFC17A4A };  // fundamental — burnt sienna
    static const juce::Colour harmonic2     { 0xFFE8C87A };  // 2nd — mustard gold
    static const juce::Colour harmonic3     { 0xFFC4887A };  // 3rd — dusty rose
    static const juce::Colour harmonic4     { 0xFFD4915E };  // 4th — bright sienna
    static const juce::Colour harmonic5     { 0xFFA8B5A0 };  // 5th — sage
    static const juce::Colour harmonic6     { 0xFF6B9B9E };  // 6th — teal
    static const juce::Colour harmonic7     { 0xFFBEC9B6 };  // 7th — bright sage
    static const juce::Colour harmonic8     { 0xFFA06238 };  // 8th — dim sienna

    inline juce::Colour harmonicColour(int index) noexcept {
        static const juce::Colour table[] = {
            harmonic1, harmonic2, harmonic3, harmonic4,
            harmonic5, harmonic6, harmonic7, harmonic8
        };
        return table[juce::jlimit(0, 7, index)];
    }

    // ── State colors ──
    static const juce::Colour stateSafe     { 0xFFA8B5A0 };  // sage = safe
    static const juce::Colour stateWarn     { 0xFFE8C87A };  // mustard = warning
    static const juce::Colour stateDanger   { 0xFFC4887A };  // dusty rose = danger
    static const juce::Colour stateClip     { 0xFF8B3A2A };  // deep rust = clip

    // ── Meter zones ──
    static const juce::Colour meterSafe     { 0xFFE2D9CA };  // well = background
    static const juce::Colour meterOptimal  { 0xFFA8B5A0 };  // sage = optimal
    static const juce::Colour meterHot      { 0xFFE8C87A };  // mustard = hot
    static const juce::Colour meterClip     { 0xFFC4887A };  // dusty rose = clip

    // ── Glow alpha values (very subtle for Wes Anderson restraint) ──
    constexpr float oakGlowAlpha       = 0.12f;
    constexpr float oakSoftGlowAlpha   = 0.06f;
    constexpr float sageGlowAlpha      = 0.10f;
    constexpr float hoverGlowAlpha     = 0.06f;
    constexpr float activeGlowAlpha    = 0.15f;

    // Legacy aliases for backward compatibility
    static const juce::Colour& amber     = oak;
    static const juce::Colour& amberDim  = oakDim;
    static const juce::Colour& amberDark = oakDark;
    static const juce::Colour& amberBright = oakBright;
    static const juce::Colour& coral     = terracotta;
    static const juce::Colour& coralDim  = terracottaDim;
    static const juce::Colour& cyan      = sage;
    static const juce::Colour& cyanDim   = sageDim;
    static const juce::Colour& emerald   = sageBright;
    constexpr float amberGlowAlpha     = oakGlowAlpha;
    constexpr float amberSoftGlowAlpha = oakSoftGlowAlpha;
    constexpr float coralGlowAlpha     = 0.10f;
    constexpr float cyanGlowAlpha      = sageGlowAlpha;

    // ── Module accent mapping ──
    enum class Module { BTZ, SPARK, SHINE, TONE, MOTION, BLOOM, Utility };

    inline juce::Colour accentFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return oak;
            case Module::SPARK:   return terracotta;
            case Module::SHINE:   return sage;
            case Module::TONE:    return teal;
            case Module::MOTION:  return mustard;
            case Module::BLOOM:   return oakLight;
            case Module::Utility: return textSecondary;
        }
        return oak;
    }

    inline juce::Colour accentDimFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return oakDim;
            case Module::SPARK:   return terracottaDim;
            case Module::SHINE:   return sageDim;
            case Module::TONE:    return tealDim;
            case Module::MOTION:  return mustardDim;
            default:              return textTertiary;
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
        constexpr int sec   = 32;   // section padding
        constexpr int secMd = 40;
        constexpr int secLg = 48;
        constexpr int page  = 64;
    }

    namespace Radius {
        // Wes Anderson: very subtle rounding — almost square, like vintage hardware
        constexpr float none    = 0.0f;
        constexpr float sm      = 2.0f;   // barely rounded
        constexpr float md      = 4.0f;   // subtle
        constexpr float lg      = 6.0f;   // maximum for panels
        constexpr float xl      = 8.0f;   // rare, only for special elements
        constexpr float pill    = 9999.0f;
    }

    namespace Border {
        // Wes Anderson: deliberate, visible, precise thin borders
        constexpr float hairline = 0.75f;  // decorative rules
        constexpr float frame    = 1.0f;   // panel framing borders
        constexpr float focus    = 1.5f;   // focus indicator
        constexpr float accent   = 2.0f;   // accent borders (section dividers)
        constexpr float glass    = 0.5f;
    }

    namespace Font {
        // Wes Anderson typography: Futura (geometric sans) + Playfair (elegant serif)
        // Mapped to available system fonts:
        static const juce::String display = "Futura";       // geometric sans for headers
        static const juce::String serif   = "Playfair Display"; // elegant serif for brand
        static const juce::String ui      = "Futura";       // clean geometric for labels
        static const juce::String mono    = "IBM Plex Mono"; // vintage monospace for values

        namespace Size {
            constexpr float size3xl  = 72.0f;
            constexpr float size2xl  = 48.0f;
            constexpr float sizeXl   = 28.0f;
            constexpr float sizeLg   = 20.0f;
            constexpr float sizeMd   = 15.0f;
            constexpr float sizeBase = 13.0f;
            constexpr float sizeSm   = 11.0f;
            constexpr float sizeXs   = 10.0f;
            constexpr float size2xs  = 9.0f;
            constexpr float size3xs  = 8.0f;
        }

        namespace Tracking {
            // Wes Anderson: wide letter-spacing is signature
            constexpr float tight  = 0.0f;
            constexpr float body   = 0.02f;
            constexpr float label  = 0.25f;   // wide — signature WA feel
            constexpr float wide   = 0.35f;   // very wide for section headers
            constexpr float ultra  = 0.50f;   // extreme — for brand/title only
        }
    }

    namespace Animation {
        constexpr int instant     = 0;
        constexpr int microFast   = 60;
        constexpr int fast        = 120;
        constexpr int base        = 200;
        constexpr int slow        = 400;
        constexpr int dramatic    = 600;
        constexpr int tooltip     = 500;

        constexpr float easeOutExpo    = 0.16f;
        constexpr float easeInOutCubic = 0.65f;
        constexpr float springDamping  = 0.7f;
        constexpr float springStiffness = 300.0f;

        constexpr int meterFps        = 60;
        constexpr int spectrumFps     = 30;
        constexpr int harmonicFps     = 60;
        constexpr int idleFps         = 15;
    }

    namespace Glass {
        constexpr float blurRadius     = 10.0f;
        constexpr float panelOpacity   = 0.30f;
        constexpr float borderOpacity  = 0.20f;
        constexpr float shadowBlur     = 6.0f;
        constexpr float innerHighlight = 0.35f;
        constexpr float noiseOpacity   = 0.015f;
    }

    namespace Accessibility {
        constexpr float minContrastRatio = 4.5f;
        constexpr float largeTextContrast = 3.0f;
        constexpr int   minTouchTarget   = 44;
        constexpr int   focusRingWidth   = 2;
        static const juce::Colour focusRing { 0xFF6B9B9E };  // teal focus indicator
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
        constexpr int brandLineHeight  = 56;   // taller for Wes Anderson header
        constexpr int outputGuardWidth = 120;
        constexpr int tickDotCount     = 11;
        constexpr float tickDotSpanDegrees = 240.0f;
        constexpr float haloStartAngleDegrees = 225.0f;
        constexpr float haloSweepDegrees = 270.0f;
        constexpr int indicatorWidth   = 2;
        constexpr int indicatorHeight  = 14;

        // Simple Mode large knobs
        constexpr int simpleKnobSize   = 160;
        constexpr int simpleKnobGap    = 56;  // more generous spacing

        // Harmonic visualizer
        constexpr int harmonicBarWidth = 8;    // slightly wider for vintage feel
        constexpr int harmonicBarGap   = 4;
        constexpr int harmonicDisplayHeight = 180;
        constexpr int harmonicMaxBars  = 16;

        // Preset browser panel
        constexpr int presetPanelWidth = 280;
        constexpr int presetRowHeight  = 34;

        // Wes Anderson: ornamental rule heights
        constexpr int ruleThick        = 2;
        constexpr int ruleThin         = 1;
        constexpr int sectionRuleGap   = 8;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.35f;
        constexpr float bypassed   = 0.45f;
        constexpr float background = 0.75f;
        constexpr float glassPanel = 0.88f;
        constexpr float ghostTrace = 0.25f;
    }

    namespace Window {
        constexpr int defaultWidth  = 1280;
        constexpr int defaultHeight = 800;
        constexpr int minWidth      = 960;
        constexpr int minHeight     = 600;
        constexpr int maxWidth      = 2560;
        constexpr int maxHeight     = 1600;
        constexpr float minScale    = 0.5f;
        constexpr float maxScale    = 2.0f;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 3. BTZTheme:: — Legacy accessor namespace
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZTheme {

namespace Color {
    inline juce::Colour backgroundRoot()        { return BTZColours::canvas; }
    inline juce::Colour backgroundCenter()      { return BTZColours::canvasWarm; }
    inline juce::Colour backgroundPanel()       { return BTZColours::panel; }
    inline juce::Colour backgroundPanelRaised() { return BTZColours::panelRaised; }
    inline juce::Colour backgroundInset()       { return BTZColours::well; }
    inline juce::Colour strokeSubtle()          { return BTZColours::rule; }
    inline juce::Colour strokeStrong()          { return BTZColours::hairline; }
    inline juce::Colour textPrimary()           { return BTZColours::text; }
    inline juce::Colour textSecondary()         { return BTZColours::textSecondary; }
    inline juce::Colour textDisabled()          { return BTZColours::textDisabled; }
    inline juce::Colour accentPrimaryAmber()    { return BTZColours::oak; }
    inline juce::Colour accentBright()          { return BTZColours::oakBright; }
    inline juce::Colour accentSecondaryChampagne() { return BTZColours::oakLight; }
    inline juce::Colour accentGlow()            { return BTZColours::oak.withAlpha(BTZColours::oakGlowAlpha); }
    inline juce::Colour accentDim()             { return BTZColours::oakDim; }
    inline juce::Colour meterSafe()             { return BTZColours::meterSafe; }
    inline juce::Colour meterWarn()             { return BTZColours::meterOptimal; }
    inline juce::Colour meterHot()              { return BTZColours::meterHot; }
    inline juce::Colour meterClip()             { return BTZColours::meterClip; }
    inline juce::Colour bypassOn()              { return BTZColours::oak; }
    inline juce::Colour bypassOff()             { return BTZColours::well; }
    inline juce::Colour hover()                 { return BTZColours::panelRaised; }
    inline juce::Colour focus()                 { return BTZColours::panel; }
    inline juce::Colour selected()              { return BTZColours::oak; }
    inline juce::Colour disabled()              { return BTZColours::rule; }
    inline juce::Colour grMeter()               { return BTZColours::terracotta; }
}

namespace Font {
    inline juce::String displayFamily() { return BTZTokens::Font::display; }
    inline juce::String serifFamily()   { return BTZTokens::Font::serif; }
    inline juce::String bodyFamily()    { return BTZTokens::Font::ui; }
    inline juce::String monoFamily()    { return BTZTokens::Font::mono; }

    // Modern sleek typography: tight, clean, minimal
    inline juce::Font title()         { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeLg, juce::Font::bold); }
    inline juce::Font subtitle()      { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::plain); }
    inline juce::Font sectionHeader() { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font label()         { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeXs, juce::Font::plain); }
    inline juce::Font value()         { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeXs, juce::Font::plain); }
    inline juce::Font tooltip()       { return juce::Font(bodyFamily(), BTZTokens::Font::Size::size2xs, juce::Font::plain); }
    inline juce::Font tab()           { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font meter()         { return juce::Font(monoFamily(), BTZTokens::Font::Size::size3xs, juce::Font::plain); }
    inline juce::Font micro()         { return juce::Font(bodyFamily(), 7.0f, juce::Font::plain); }
    inline juce::Font brand()         { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font macroLabel()    { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font macroValue()    { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::plain); }
    inline juce::Font simpleValue()   { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font simpleLabel()   { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeBase, juce::Font::bold); }
}

namespace Geometry {
    // Modern: smooth, generous rounding
    constexpr float outerRadius   = BTZTokens::Radius::lg;  // 12px outer
    constexpr float panelRadius   = BTZTokens::Radius::md;  // 8px panels
    constexpr float controlRadius = BTZTokens::Radius::sm;  // 4px controls
    constexpr float pillRadius    = 20.0f;
    constexpr float glassRadius   = BTZTokens::Radius::md;  // 8px glass

    constexpr int knobLarge       = 74;
    constexpr int knobMedium      = 60;
    constexpr int knobSmall       = 44;
    constexpr int knobMacro       = BTZTokens::Dim::macroKnobDefault;
    constexpr int knobSimple      = BTZTokens::Dim::simpleKnobSize;

    constexpr int sliderHeight    = 28;
    constexpr int sliderTrackH    = 4;

    constexpr int spaceXS         = BTZTokens::Space::xs;
    constexpr int spaceSM         = BTZTokens::Space::sm;
    constexpr int spaceMD         = BTZTokens::Space::md;
    constexpr int spaceLG         = BTZTokens::Space::lg;
    constexpr int spaceXL         = BTZTokens::Space::xxl;
    constexpr int spaceXXL        = BTZTokens::Space::sec;

    constexpr int padPanel        = BTZTokens::Space::xl;   // generous
    constexpr int padSection      = BTZTokens::Space::lg;
    constexpr int padContent      = BTZTokens::Space::xxl;

    constexpr float borderThin    = BTZTokens::Border::hairline;
    constexpr float borderNormal  = BTZTokens::Border::frame;
    constexpr float borderThick   = BTZTokens::Border::accent;
    constexpr float borderGlass   = BTZTokens::Border::glass;

    constexpr int windowWidth     = BTZTokens::Window::defaultWidth;
    constexpr int windowHeight    = BTZTokens::Window::defaultHeight;

    constexpr int headerHeight    = BTZTokens::Dim::brandLineHeight;
    constexpr int meterStripHeight = 78;

    constexpr int tabWidth        = 110;
    constexpr int tabGap          = BTZTokens::Space::lg;
    constexpr int tabHeight       = 32;

    constexpr int knobLabelHeight = 20;
}

namespace Effects {
    struct ShadowSpec {
        juce::Colour colour;
        int radius;
        juce::Point<int> offset;
    };

    // Modern: soft, diffused shadows for contemporary depth
    inline ShadowSpec panelShadow()   { return { juce::Colour(0x0C000000), 12, { 0, 4 } }; }
    inline ShadowSpec controlShadow() { return { juce::Colour(0x0E000000), 6, { 0, 2 } }; }
    inline ShadowSpec innerShadow()   { return { juce::Colour(0x14000000), 3, { 0, 1 } }; }
    inline ShadowSpec glassShadow()   { return { juce::Colour(0x0A000000), 10, { 0, 4 } }; }

    inline juce::Colour knobGlow()      { return BTZColours::oak.withAlpha(BTZColours::oakSoftGlowAlpha); }
    inline juce::Colour knobHoverGlow() { return BTZColours::oak.withAlpha(BTZColours::hoverGlowAlpha); }
    inline juce::Colour knobActiveGlow(){ return BTZColours::oak.withAlpha(BTZColours::activeGlowAlpha); }
    inline juce::Colour meterGlow()     { return BTZColours::sage.withAlpha(0.10f); }

    constexpr float meterCornerRadius = 3.0f;  // modern rounded
    constexpr float meterSegmentGap   = 1.0f;
    constexpr int   meterSegmentCount = 24;
}

namespace KnobStyle {
    constexpr float arcThicknessRatio = 0.06f;
    constexpr float bodyRadiusRatio   = 0.76f;
    constexpr float indicatorStart    = 0.24f;
    constexpr float indicatorEnd      = 0.60f;
    constexpr float tickDotRadius     = 2.0f;   // modern, clean
    constexpr int   tickDotCount      = BTZTokens::Dim::tickDotCount;
    constexpr float startAngle        = 1.25f;  // * pi
    constexpr float endAngle          = 2.75f;  // * pi

    // Modern: smooth 3D with soft gradients
    constexpr float innerShadowOpacity = 0.12f;
    constexpr float rimLightOpacity    = 0.30f;
    constexpr float convexGradientBias = 0.55f;
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
    constexpr int HarmonicViz = 11;
    constexpr int GlassPanel  = 12;
    constexpr int SimpleKnob  = 13;
    constexpr int DirectManip = 14;
    constexpr int Tooltip     = 15;
}

inline void drawDropShadow(juce::Graphics& g, juce::Rectangle<int> area, const Effects::ShadowSpec& spec) {
    juce::DropShadow shadow(spec.colour, spec.radius, spec.offset);
    shadow.drawForRectangle(g, area);
}

// Draw a modern panel: soft shadow, warm fill, subtle border
inline void drawFramedPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::panelRadius) {
    // Soft diffused shadow
    juce::DropShadow shadow(juce::Colour(0x0C000000), 12, { 0, 4 });
    shadow.drawForRectangle(g, area.toNearestInt());

    // Warm panel fill
    g.setColour(BTZColours::panel);
    g.fillRoundedRectangle(area, cornerRadius);

    // Subtle border — barely visible, modern restraint
    g.setColour(BTZColours::hairline);
    g.drawRoundedRectangle(area, cornerRadius, BTZTokens::Border::frame);
}

// Draw a frosted glass panel — modern, smooth, warm
inline void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::glassRadius) {
    juce::DropShadow shadow(juce::Colour(0x0A000000), 10, { 0, 4 });
    shadow.drawForRectangle(g, area.toNearestInt());

    g.setColour(BTZColours::glassMedium);
    g.fillRoundedRectangle(area, cornerRadius);

    // Subtle warm border
    g.setColour(BTZColours::hairline.withAlpha(0.4f));
    g.drawRoundedRectangle(area, cornerRadius, BTZTokens::Border::hairline);
}

// Draw warm parchment background with subtle radial warmth
inline void drawRadialBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    juce::ColourGradient grad(
        BTZColours::canvasWarm, (float)area.getCentreX(), (float)area.getCentreY() * 0.4f,
        BTZColours::canvas, 0.0f, (float)area.getHeight(),
        true
    );
    g.setGradientFill(grad);
    g.fillRect(area);
}

// Draw a minimal section divider — modern, barely visible
inline void drawSectionRule(juce::Graphics& g, float x, float y, float width, bool /*thick*/ = false) {
    g.setColour(BTZColours::rule);
    g.fillRect(x, y, width, 1.0f);
}

// Draw very subtle noise texture — modern grain, barely perceptible
inline void drawNoiseTexture(juce::Graphics& g, juce::Rectangle<int> area, uint32_t seed = 42) {
    g.setColour(juce::Colour(0x03000000));
    uint32_t s = seed;
    for (int i = 0; i < 60; ++i) {
        s = s * 1664525u + 1013904223u;
        float x = (float)(s % (uint32_t)area.getWidth());
        s = s * 1664525u + 1013904223u;
        float y = (float)(s % (uint32_t)area.getHeight());
        g.fillEllipse(x, y, 0.8f, 0.8f);
    }
}

} // namespace BTZTheme
