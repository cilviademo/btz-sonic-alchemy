/*
  Box Tone Zone (BTZ) — BTZTheme.h  v9
  ────────────────────────────────────────────────────────────────────────
  v9 (Modern Light Premium):
    • Light warm canvas — cream/beige, airy, contemporary
    • Sage green + warm orange as primary accents (original DNA)
    • Modern execution: tight radius (3-4px), dense typography, precise
    • NOT toyish: small uppercase labels, professional density
    • NOT dated: no skeuomorphism, no heavy shadows, no gradients
    • Inspired by: Linear, Stripe, Apple HIG — but warm-toned
    • Knobs: clean flat with subtle depth, not metallic or glossy
    • Separation: hairline rules + subtle background tints, not borders
    • Timeless and immediately identifiable

  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.
  ────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — Modern Light Warm Palette
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Background tier (warm light — cream/beige, never cold white) ──
    static const juce::Colour canvas        { 0xFFF6F3EE };  // primary — warm cream
    static const juce::Colour canvasSubtle  { 0xFFF0ECE6 };  // slightly warmer for sections
    static const juce::Colour panel         { 0xFFEBE7E0 };  // raised panel — light tan
    static const juce::Colour panelRaised   { 0xFFE4DFD8 };  // elevated surface
    static const juce::Colour well          { 0xFFDDD8D0 };  // inset/well — slightly darker
    static const juce::Colour wellDeep      { 0xFFD4CEC5 };  // deepest inset
    static const juce::Colour hairline      { 0xFFD0C9C0 };  // subtle separation
    static const juce::Colour rule          { 0xFFE0DAD2 };  // barely visible divider

    // ── Ink & typography (dark on light — never pure black) ──
    static const juce::Colour text          { 0xFF2C2824 };  // primary text — warm dark
    static const juce::Colour textSecondary { 0xFF6B6358 };  // secondary — muted brown
    static const juce::Colour textTertiary  { 0xFF9A9088 };  // dim labels
    static const juce::Colour textDisabled  { 0xFFC0B8AE };  // disabled state
    static const juce::Colour textInverse   { 0xFFF6F3EE };  // text on dark surfaces

    // ── Knob body (clean, flat, subtle depth — modern, not metallic) ──
    static const juce::Colour knobBody      { 0xFFFFFFFF };  // clean white face
    static const juce::Colour knobRing      { 0xFFE8E3DC };  // outer ring — subtle
    static const juce::Colour knobShadow    { 0x18000000 };  // very subtle drop shadow
    static const juce::Colour knobTrack     { 0xFFE0DAD2 };  // unfilled arc track
    static const juce::Colour knobPointer   { 0xFF2C2824 };  // indicator line — dark

    // ── Primary accent: Warm Orange (the original BTZ identity) ──
    static const juce::Colour oak           { 0xFFD4854A };  // primary — warm orange
    static const juce::Colour oakBright     { 0xFFE09558 };  // active/hover
    static const juce::Colour oakDim        { 0xFFB87040 };  // dimmed
    static const juce::Colour oakDark       { 0xFF9A5C34 };  // dark variant
    static const juce::Colour oakLight      { 0xFFF0B888 };  // light variant
    static const juce::Colour oakSubtle     { 0xFFFBF0E4 };  // tinted background

    // ── Secondary accent: Sage Green (the original BTZ identity) ──
    static const juce::Colour sage          { 0xFF7A9B82 };  // secondary — sage green
    static const juce::Colour sageBright    { 0xFF8AAE92 };  // active/hover
    static const juce::Colour sageDim       { 0xFF628070 };  // dimmed
    static const juce::Colour sageDark      { 0xFF4A6454 };  // dark variant
    static const juce::Colour sageLight     { 0xFFB8D4BE };  // light variant
    static const juce::Colour sageSubtle    { 0xFFEEF5F0 };  // tinted background

    // ── Tertiary accent: Warm Red/Clay (warnings, GR) ──
    static const juce::Colour terracotta    { 0xFFC05A48 };  // warm red — alerts/GR
    static const juce::Colour terracottaDim { 0xFF9A4838 };  // dimmed
    static const juce::Colour terracottaLight { 0xFFE8A090 }; // light variant
    static const juce::Colour terracottaSubtle { 0xFFFDF2EE }; // tinted bg

    // ── Accent 4: Teal (utility, focus states) ──
    static const juce::Colour teal          { 0xFF5A8A8C };  // muted teal
    static const juce::Colour tealBright    { 0xFF6A9EA0 };  // active
    static const juce::Colour tealDim       { 0xFF487072 };  // dimmed

    // ── Accent 5: Warm Gold (highlights, special) ──
    static const juce::Colour mustard       { 0xFFD4A84C };  // warm gold
    static const juce::Colour mustardBright { 0xFFE0B85C };  // active
    static const juce::Colour mustardDim    { 0xFFB89040 };  // dimmed

    // ── Glassmorphism (light mode — frosted white) ──
    static const juce::Colour glassLight    { 0xB0FFFFFF };  // 69% white overlay
    static const juce::Colour glassMedium   { 0x90FFFFFF };  // 56% white overlay
    static const juce::Colour glassBorder   { 0x40FFFFFF };  // 25% white border
    static const juce::Colour glassHighlight{ 0x60FFFFFF };  // top edge
    static const juce::Colour glassShadow   { 0x10000000 };  // subtle bottom shadow

    // ── Harmonic overtone colors (warm palette) ──
    static const juce::Colour harmonic1     { 0xFFD4854A };  // fundamental — orange
    static const juce::Colour harmonic2     { 0xFFD4A84C };  // 2nd — gold
    static const juce::Colour harmonic3     { 0xFFC05A48 };  // 3rd — clay
    static const juce::Colour harmonic4     { 0xFFE09558 };  // 4th — bright orange
    static const juce::Colour harmonic5     { 0xFF7A9B82 };  // 5th — sage
    static const juce::Colour harmonic6     { 0xFF5A8A8C };  // 6th — teal
    static const juce::Colour harmonic7     { 0xFF8AAE92 };  // 7th — bright sage
    static const juce::Colour harmonic8     { 0xFFB87040 };  // 8th — dim orange

    inline juce::Colour harmonicColour(int index) noexcept {
        static const juce::Colour table[] = {
            harmonic1, harmonic2, harmonic3, harmonic4,
            harmonic5, harmonic6, harmonic7, harmonic8
        };
        return table[juce::jlimit(0, 7, index)];
    }

    // ── State colors ──
    static const juce::Colour stateSafe     { 0xFF7A9B82 };  // sage = safe
    static const juce::Colour stateWarn     { 0xFFD4A84C };  // gold = warning
    static const juce::Colour stateDanger   { 0xFFC05A48 };  // clay = danger
    static const juce::Colour stateClip     { 0xFFB83828 };  // red = clip

    // ── Meter zones ──
    static const juce::Colour meterBg       { 0xFFE8E3DC };  // track background
    static const juce::Colour meterOptimal  { 0xFF7A9B82 };  // sage = optimal
    static const juce::Colour meterHot      { 0xFFD4A84C };  // gold = hot
    static const juce::Colour meterClip     { 0xFFC05A48 };  // clay = clip

    // ── Glow/focus alpha values (subtle on light) ──
    constexpr float oakGlowAlpha       = 0.12f;
    constexpr float oakSoftGlowAlpha   = 0.06f;
    constexpr float sageGlowAlpha      = 0.10f;
    constexpr float hoverGlowAlpha     = 0.08f;
    constexpr float activeGlowAlpha    = 0.16f;

    // Legacy aliases
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
        constexpr int xs    = 3;
        constexpr int sm    = 6;
        constexpr int md    = 10;
        constexpr int lg    = 14;
        constexpr int xl    = 18;
        constexpr int xxl   = 24;
        constexpr int sec   = 28;
        constexpr int secMd = 36;
        constexpr int secLg = 48;
        constexpr int page  = 56;
    }

    namespace Radius {
        // Modern: tight but not sharp — 3-4px feels contemporary
        constexpr float none    = 0.0f;
        constexpr float sm      = 3.0f;   // controls, buttons
        constexpr float md      = 4.0f;   // panels
        constexpr float lg      = 6.0f;   // larger panels
        constexpr float xl      = 8.0f;   // modals, dropdowns
        constexpr float pill    = 9999.0f;
    }

    namespace Border {
        constexpr float hairline = 0.5f;
        constexpr float frame    = 1.0f;
        constexpr float focus    = 1.5f;
        constexpr float accent   = 1.5f;
        constexpr float glass    = 0.5f;
    }

    namespace Font {
        // Modern, clean: Inter Tight for everything, IBM Plex Mono for values
        static const juce::String display = "Inter Tight";
        static const juce::String serif   = "Inter Tight";
        static const juce::String ui      = "Inter Tight";
        static const juce::String mono    = "IBM Plex Mono";

        namespace Size {
            constexpr float size3xl  = 36.0f;
            constexpr float size2xl  = 26.0f;
            constexpr float sizeXl   = 20.0f;
            constexpr float sizeLg   = 15.0f;
            constexpr float sizeMd   = 12.0f;
            constexpr float sizeBase = 11.0f;
            constexpr float sizeSm   = 10.0f;
            constexpr float sizeXs   = 9.0f;
            constexpr float size2xs  = 8.0f;
            constexpr float size3xs  = 7.0f;
        }

        namespace Tracking {
            constexpr float tight  = -0.01f;
            constexpr float body   = 0.0f;
            constexpr float label  = 0.08f;   // uppercase labels
            constexpr float wide   = 0.12f;   // section headers
            constexpr float ultra  = 0.16f;   // brand
        }
    }

    namespace Animation {
        constexpr int instant     = 0;
        constexpr int microFast   = 50;
        constexpr int fast        = 100;
        constexpr int base        = 180;
        constexpr int slow        = 350;
        constexpr int dramatic    = 500;
        constexpr int tooltip     = 400;

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
        constexpr float blurRadius     = 12.0f;
        constexpr float panelOpacity   = 0.70f;
        constexpr float borderOpacity  = 0.25f;
        constexpr float shadowBlur     = 6.0f;
        constexpr float innerHighlight = 0.60f;
        constexpr float noiseOpacity   = 0.01f;
    }

    namespace Accessibility {
        constexpr float minContrastRatio = 4.5f;
        constexpr float largeTextContrast = 3.0f;
        constexpr int   minTouchTarget   = 44;
        constexpr int   focusRingWidth   = 2;
        static const juce::Colour focusRing { 0xFF5A8A8C };
    }

    namespace Dim {
        constexpr int macroKnobDefault = 48;
        constexpr int macroKnobMin     = 38;
        constexpr int macroKnobMax     = 64;
        constexpr int utilityKnobDefault = 36;
        constexpr int brandLineHeight  = 32;
        constexpr int outputGuardWidth = 100;
        constexpr int tickDotCount     = 11;
        constexpr float tickDotSpanDegrees = 240.0f;
        constexpr float haloStartAngleDegrees = 225.0f;
        constexpr float haloSweepDegrees = 270.0f;
        constexpr int indicatorWidth   = 2;
        constexpr int indicatorHeight  = 10;

        constexpr int simpleKnobSize   = 88;
        constexpr int simpleKnobGap    = 36;

        constexpr int harmonicBarWidth = 4;
        constexpr int harmonicBarGap   = 2;
        constexpr int harmonicDisplayHeight = 120;
        constexpr int harmonicMaxBars  = 16;

        constexpr int presetPanelWidth = 220;
        constexpr int presetRowHeight  = 26;

        constexpr int ruleThick        = 1;
        constexpr int ruleThin         = 1;
        constexpr int sectionRuleGap   = 6;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.35f;
        constexpr float bypassed   = 0.45f;
        constexpr float background = 0.85f;
        constexpr float glassPanel = 0.92f;
        constexpr float ghostTrace = 0.20f;
    }

    namespace Window {
        constexpr int defaultWidth  = 1200;
        constexpr int defaultHeight = 700;
        constexpr int minWidth      = 900;
        constexpr int minHeight     = 540;
        constexpr int maxWidth      = 2400;
        constexpr int maxHeight     = 1400;
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
    inline juce::Colour backgroundCenter()      { return BTZColours::canvas; }
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
    inline juce::Colour meterSafe()             { return BTZColours::meterBg; }
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

    inline juce::Font title()         { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeLg, juce::Font::bold); }
    inline juce::Font subtitle()      { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::plain); }
    inline juce::Font sectionHeader() { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font label()         { return juce::Font(bodyFamily(), BTZTokens::Font::Size::size2xs, juce::Font::plain); }
    inline juce::Font value()         { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeXs, juce::Font::plain); }
    inline juce::Font tooltip()       { return juce::Font(bodyFamily(), BTZTokens::Font::Size::size2xs, juce::Font::plain); }
    inline juce::Font tab()           { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font meter()         { return juce::Font(monoFamily(), BTZTokens::Font::Size::size3xs, juce::Font::plain); }
    inline juce::Font micro()         { return juce::Font(bodyFamily(), 7.0f, juce::Font::plain); }
    inline juce::Font brand()         { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font macroLabel()    { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
    inline juce::Font macroValue()    { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeXs, juce::Font::plain); }
    inline juce::Font simpleValue()   { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font simpleLabel()   { return juce::Font(bodyFamily(), BTZTokens::Font::Size::sizeSm, juce::Font::bold); }
}

namespace Geometry {
    constexpr float outerRadius   = BTZTokens::Radius::md;
    constexpr float panelRadius   = BTZTokens::Radius::md;
    constexpr float controlRadius = BTZTokens::Radius::sm;
    constexpr float pillRadius    = 12.0f;
    constexpr float glassRadius   = BTZTokens::Radius::md;

    constexpr int knobLarge       = 52;
    constexpr int knobMedium      = 42;
    constexpr int knobSmall       = 32;
    constexpr int knobMacro       = BTZTokens::Dim::macroKnobDefault;
    constexpr int knobSimple      = BTZTokens::Dim::simpleKnobSize;

    constexpr int sliderHeight    = 22;
    constexpr int sliderTrackH    = 3;

    constexpr int spaceXS         = BTZTokens::Space::xs;
    constexpr int spaceSM         = BTZTokens::Space::sm;
    constexpr int spaceMD         = BTZTokens::Space::md;
    constexpr int spaceLG         = BTZTokens::Space::lg;
    constexpr int spaceXL         = BTZTokens::Space::xxl;
    constexpr int spaceXXL        = BTZTokens::Space::sec;

    constexpr int padPanel        = BTZTokens::Space::lg;
    constexpr int padSection      = BTZTokens::Space::md;
    constexpr int padContent      = BTZTokens::Space::xl;

    constexpr float borderThin    = BTZTokens::Border::hairline;
    constexpr float borderNormal  = BTZTokens::Border::frame;
    constexpr float borderThick   = BTZTokens::Border::accent;
    constexpr float borderGlass   = BTZTokens::Border::glass;

    constexpr int windowWidth     = BTZTokens::Window::defaultWidth;
    constexpr int windowHeight    = BTZTokens::Window::defaultHeight;

    constexpr int headerHeight    = BTZTokens::Dim::brandLineHeight;
    constexpr int meterStripHeight = 56;

    constexpr int tabWidth        = 72;
    constexpr int tabGap          = BTZTokens::Space::sm;
    constexpr int tabHeight       = 24;

    constexpr int knobLabelHeight = 14;
}

namespace Effects {
    struct ShadowSpec {
        juce::Colour colour;
        int radius;
        juce::Point<int> offset;
    };

    // Modern light: very subtle shadows — just enough to create layers
    inline ShadowSpec panelShadow()   { return { juce::Colour(0x0C000000), 8, { 0, 2 } }; }
    inline ShadowSpec controlShadow() { return { juce::Colour(0x0A000000), 3, { 0, 1 } }; }
    inline ShadowSpec innerShadow()   { return { juce::Colour(0x08000000), 2, { 0, 1 } }; }
    inline ShadowSpec glassShadow()   { return { juce::Colour(0x0E000000), 6, { 0, 2 } }; }

    inline juce::Colour knobGlow()      { return BTZColours::oak.withAlpha(BTZColours::oakSoftGlowAlpha); }
    inline juce::Colour knobHoverGlow() { return BTZColours::oak.withAlpha(BTZColours::hoverGlowAlpha); }
    inline juce::Colour knobActiveGlow(){ return BTZColours::oak.withAlpha(BTZColours::activeGlowAlpha); }
    inline juce::Colour meterGlow()     { return BTZColours::sage.withAlpha(0.06f); }

    constexpr float meterCornerRadius = 2.0f;
    constexpr float meterSegmentGap   = 1.0f;
    constexpr int   meterSegmentCount = 28;
}

namespace KnobStyle {
    // Modern flat knob: clean white face, colored arc, thin indicator
    constexpr float arcThicknessRatio = 0.05f;   // thin arc
    constexpr float bodyRadiusRatio   = 0.70f;
    constexpr float indicatorStart    = 0.30f;
    constexpr float indicatorEnd      = 0.58f;
    constexpr float tickDotRadius     = 1.0f;    // tiny dots
    constexpr int   tickDotCount      = BTZTokens::Dim::tickDotCount;
    constexpr float startAngle        = 1.25f;
    constexpr float endAngle          = 2.75f;

    constexpr float innerShadowOpacity = 0.05f;  // very subtle
    constexpr float rimLightOpacity    = 0.0f;   // no rim — flat modern
    constexpr float convexGradientBias = 0.0f;   // flat
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

// Draw a modern panel: subtle background tint + very light shadow
inline void drawFramedPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::panelRadius) {
    // Very subtle shadow
    juce::DropShadow shadow(juce::Colour(0x0C000000), 6, { 0, 1 });
    shadow.drawForRectangle(g, area.toNearestInt());

    // Panel fill
    g.setColour(BTZColours::panel);
    g.fillRoundedRectangle(area, cornerRadius);
}

// Draw a frosted glass panel — light mode
inline void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::glassRadius) {
    juce::DropShadow shadow(juce::Colour(0x0A000000), 4, { 0, 1 });
    shadow.drawForRectangle(g, area.toNearestInt());

    g.setColour(BTZColours::glassLight);
    g.fillRoundedRectangle(area, cornerRadius);

    // Hairline border
    g.setColour(BTZColours::hairline);
    g.drawRoundedRectangle(area, cornerRadius, 0.5f);
}

// Draw flat warm background — no gradient, clean
inline void drawRadialBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(BTZColours::canvas);
    g.fillRect(area);
}

// Draw a minimal section divider
inline void drawSectionRule(juce::Graphics& g, float x, float y, float width, bool /*thick*/ = false) {
    g.setColour(BTZColours::hairline);
    g.fillRect(x, y, width, 1.0f);
}

// No noise texture for modern light — clean surfaces
inline void drawNoiseTexture(juce::Graphics& /*g*/, juce::Rectangle<int> /*area*/, uint32_t /*seed*/ = 42) {
    // Intentionally empty — modern light theme uses clean flat surfaces
}

} // namespace BTZTheme
