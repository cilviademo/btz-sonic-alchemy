/*
  Box Tone Zone (BTZ) — BTZTheme.h  v8
  ────────────────────────────────────────────────────────────────────────
  v8 (Premium Studio Edition):
    • Dark warm background — espresso/chocolate, never cold gray
    • Desaturated warm accents — muted copper, olive, clay, slate teal
    • Tight precision radius (3px) — professional, not playful
    • Smaller, denser typography — UPPERCASE labels, tight tracking
    • Metallic knob bodies — dark gunmetal with bright thin indicators
    • Separation via luminance, not borders
    • High information density — every pixel earns its place
    • Thin precise meters and arcs
    • Understated brand presence
    • Timeless and immediately identifiable

  Palette DNA: Same warm family (sienna, sage, teal, gold) but DESATURATED
  and placed on DARK backgrounds for professional studio feel.

  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.
  ────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — Premium Warm Dark Palette
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Background tier (warm dark — espresso, not cold gray) ──
    static const juce::Colour canvas        { 0xFF1C1814 };  // primary — warm near-black
    static const juce::Colour canvasWarm    { 0xFF201C18 };  // slightly lighter center
    static const juce::Colour panel         { 0xFF282320 };  // panel — raised surface
    static const juce::Colour panelRaised   { 0xFF302A26 };  // elevated panel
    static const juce::Colour well          { 0xFF141210 };  // inset/well — darker
    static const juce::Colour wellDeep      { 0xFF0E0C0A };  // deepest inset
    static const juce::Colour hairline      { 0xFF3A3430 };  // subtle separation (luminance, not border)
    static const juce::Colour rule          { 0xFF2E2824 };  // barely visible divider

    // ── Ink & typography (warm light on dark — never pure white) ──
    static const juce::Colour text          { 0xFFE8E0D6 };  // primary text — warm off-white
    static const juce::Colour textSecondary { 0xFFA89E92 };  // secondary — muted warm gray
    static const juce::Colour textTertiary  { 0xFF6E6458 };  // dim labels
    static const juce::Colour textDisabled  { 0xFF4A4238 };  // disabled state
    static const juce::Colour textInverse   { 0xFF1C1814 };  // text on light surfaces

    // ── Knob body (dark metallic — gunmetal/brushed aluminum) ──
    static const juce::Colour knobBody      { 0xFF3A3632 };  // main knob fill — dark warm
    static const juce::Colour knobHighlight { 0xFF4E4842 };  // top rim highlight
    static const juce::Colour knobShadow    { 0xFF1E1A16 };  // bottom shadow
    static const juce::Colour knobInnerShadow { 0xFF2A2622 };  // convexity shadow
    static const juce::Colour knobRimLight  { 0xFF5A5248 };  // top rim — subtle metallic
    static const juce::Colour knobPointer   { 0xFFE8E0D6 };  // indicator line — bright on dark

    // ── Primary accent: Muted Copper (desaturated burnt sienna) ──
    static const juce::Colour oak           { 0xFFB87A52 };  // primary — muted copper
    static const juce::Colour oakBright     { 0xFFCC8E64 };  // active/hover
    static const juce::Colour oakDim        { 0xFF8A5C3C };  // dimmed
    static const juce::Colour oakDark       { 0xFF6A4428 };  // dark variant
    static const juce::Colour oakLight      { 0xFFD4A880 };  // light variant

    // ── Secondary accent: Olive (desaturated sage) ──
    static const juce::Colour sage          { 0xFF7A8A6E };  // secondary — muted olive
    static const juce::Colour sageBright    { 0xFF92A284 };  // active/hover
    static const juce::Colour sageDim       { 0xFF5C6852 };  // dimmed
    static const juce::Colour sageDark      { 0xFF404A38 };  // dark variant
    static const juce::Colour sageLight     { 0xFFAAB89E };  // light variant

    // ── Tertiary accent: Clay (desaturated dusty rose) ──
    static const juce::Colour terracotta    { 0xFFA06E62 };  // muted clay/warm mauve
    static const juce::Colour terracottaDim { 0xFF7A5448 };  // dimmed
    static const juce::Colour terracottaLight { 0xFFBE8E82 }; // light variant

    // ── Accent 4: Slate Teal (desaturated) ──
    static const juce::Colour teal          { 0xFF5A8284 };  // muted slate-teal
    static const juce::Colour tealBright    { 0xFF6E9698 };  // active
    static const juce::Colour tealDim       { 0xFF426264 };  // dimmed

    // ── Accent 5: Aged Gold (desaturated mustard) ──
    static const juce::Colour mustard       { 0xFFC4A45C };  // warm aged gold
    static const juce::Colour mustardBright { 0xFFD4B46C };  // active
    static const juce::Colour mustardDim    { 0xFF9A8044 };  // dimmed
    static const juce::Colour mustardDark   { 0xFF6E5C30 };  // dark

    // ── Glassmorphism (adapted for dark — frosted dark) ──
    static const juce::Colour glassLight    { 0x12FFFFFF };  // 7% white overlay
    static const juce::Colour glassMedium   { 0x1EFFFFFF };  // 12% white overlay
    static const juce::Colour glassBorder   { 0x18FFFFFF };  // 9% white border
    static const juce::Colour glassHighlight{ 0x22FFFFFF };  // 13% white top edge
    static const juce::Colour glassShadow   { 0x30000000 };  // 19% black bottom

    // ── Harmonic overtone colors (desaturated warm palette on dark) ──
    static const juce::Colour harmonic1     { 0xFFB87A52 };  // fundamental — copper
    static const juce::Colour harmonic2     { 0xFFC4A45C };  // 2nd — aged gold
    static const juce::Colour harmonic3     { 0xFFA06E62 };  // 3rd — clay
    static const juce::Colour harmonic4     { 0xFFCC8E64 };  // 4th — bright copper
    static const juce::Colour harmonic5     { 0xFF7A8A6E };  // 5th — olive
    static const juce::Colour harmonic6     { 0xFF5A8284 };  // 6th — slate teal
    static const juce::Colour harmonic7     { 0xFF92A284 };  // 7th — bright olive
    static const juce::Colour harmonic8     { 0xFF8A5C3C };  // 8th — dim copper

    inline juce::Colour harmonicColour(int index) noexcept {
        static const juce::Colour table[] = {
            harmonic1, harmonic2, harmonic3, harmonic4,
            harmonic5, harmonic6, harmonic7, harmonic8
        };
        return table[juce::jlimit(0, 7, index)];
    }

    // ── State colors (on dark background) ──
    static const juce::Colour stateSafe     { 0xFF7A8A6E };  // olive = safe
    static const juce::Colour stateWarn     { 0xFFC4A45C };  // gold = warning
    static const juce::Colour stateDanger   { 0xFFA06E62 };  // clay = danger
    static const juce::Colour stateClip     { 0xFFC04030 };  // muted red = clip

    // ── Meter zones (thin, precise, on dark) ──
    static const juce::Colour meterSafe     { 0xFF282320 };  // panel = background
    static const juce::Colour meterOptimal  { 0xFF7A8A6E };  // olive = optimal
    static const juce::Colour meterHot      { 0xFFC4A45C };  // gold = hot
    static const juce::Colour meterClip     { 0xFFC04030 };  // muted red = clip

    // ── Glow alpha values (subtle on dark — more visible) ──
    constexpr float oakGlowAlpha       = 0.18f;
    constexpr float oakSoftGlowAlpha   = 0.08f;
    constexpr float sageGlowAlpha      = 0.14f;
    constexpr float hoverGlowAlpha     = 0.10f;
    constexpr float activeGlowAlpha    = 0.22f;

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
    constexpr float coralGlowAlpha     = 0.12f;
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
        constexpr int xxl   = 22;
        constexpr int sec   = 28;   // section padding
        constexpr int secMd = 36;
        constexpr int secLg = 44;
        constexpr int page  = 56;
    }

    namespace Radius {
        // Premium: tight, precise — professional, not playful
        constexpr float none    = 0.0f;
        constexpr float sm      = 2.0f;   // controls, buttons
        constexpr float md      = 3.0f;   // panels
        constexpr float lg      = 4.0f;   // larger panels
        constexpr float xl      = 6.0f;   // rare, special elements
        constexpr float pill    = 9999.0f;
    }

    namespace Border {
        // Premium: separation via luminance, borders are rare
        constexpr float hairline = 0.5f;   // barely visible
        constexpr float frame    = 0.75f;  // subtle panel edge
        constexpr float focus    = 1.5f;   // focus indicator
        constexpr float accent   = 1.0f;   // accent line
        constexpr float glass    = 0.5f;
    }

    namespace Font {
        // Premium typography: Inter Tight (clean, professional), IBM Plex Mono (values)
        static const juce::String display = "Inter Tight";   // display = same as UI for cohesion
        static const juce::String serif   = "Inter Tight";   // no serif — modern
        static const juce::String ui      = "Inter Tight";   // clean, tight, professional
        static const juce::String mono    = "IBM Plex Mono"; // precise monospace for values

        namespace Size {
            constexpr float size3xl  = 48.0f;
            constexpr float size2xl  = 32.0f;
            constexpr float sizeXl   = 22.0f;
            constexpr float sizeLg   = 16.0f;
            constexpr float sizeMd   = 13.0f;
            constexpr float sizeBase = 11.0f;
            constexpr float sizeSm   = 10.0f;
            constexpr float sizeXs   = 9.0f;
            constexpr float size2xs  = 8.0f;
            constexpr float size3xs  = 7.0f;
        }

        namespace Tracking {
            // Premium: tight for body, slightly open for UPPERCASE labels
            constexpr float tight  = -0.01f;
            constexpr float body   = 0.0f;
            constexpr float label  = 0.06f;   // subtle open for uppercase
            constexpr float wide   = 0.10f;   // section headers
            constexpr float ultra  = 0.14f;   // brand only
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
        constexpr float blurRadius     = 8.0f;
        constexpr float panelOpacity   = 0.12f;
        constexpr float borderOpacity  = 0.09f;
        constexpr float shadowBlur     = 8.0f;
        constexpr float innerHighlight = 0.13f;
        constexpr float noiseOpacity   = 0.02f;
    }

    namespace Accessibility {
        constexpr float minContrastRatio = 4.5f;
        constexpr float largeTextContrast = 3.0f;
        constexpr int   minTouchTarget   = 44;
        constexpr int   focusRingWidth   = 2;
        static const juce::Colour focusRing { 0xFF5A8284 };  // slate teal focus
    }

    namespace Duration {
        constexpr int instant = 0;
        constexpr int fast    = 100;
        constexpr int base    = 180;
        constexpr int slow    = 350;
        constexpr int tooltip = 400;
    }

    namespace Dim {
        constexpr int macroKnobDefault = 52;    // smaller, denser
        constexpr int macroKnobMin     = 40;
        constexpr int macroKnobMax     = 72;
        constexpr int utilityKnobDefault = 38;
        constexpr int brandLineHeight  = 36;    // understated
        constexpr int outputGuardWidth = 100;
        constexpr int tickDotCount     = 11;
        constexpr float tickDotSpanDegrees = 240.0f;
        constexpr float haloStartAngleDegrees = 225.0f;
        constexpr float haloSweepDegrees = 270.0f;
        constexpr int indicatorWidth   = 2;
        constexpr int indicatorHeight  = 12;

        // Simple Mode knobs (still large but not oversized)
        constexpr int simpleKnobSize   = 100;
        constexpr int simpleKnobGap    = 40;

        // Harmonic visualizer (thin, precise bars)
        constexpr int harmonicBarWidth = 4;
        constexpr int harmonicBarGap   = 2;
        constexpr int harmonicDisplayHeight = 140;
        constexpr int harmonicMaxBars  = 16;

        // Preset browser panel
        constexpr int presetPanelWidth = 240;
        constexpr int presetRowHeight  = 28;

        // Dividers
        constexpr int ruleThick        = 1;
        constexpr int ruleThin         = 1;
        constexpr int sectionRuleGap   = 6;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.30f;
        constexpr float bypassed   = 0.40f;
        constexpr float background = 0.80f;
        constexpr float glassPanel = 0.92f;
        constexpr float ghostTrace = 0.20f;
    }

    namespace Window {
        constexpr int defaultWidth  = 1200;
        constexpr int defaultHeight = 720;
        constexpr int minWidth      = 900;
        constexpr int minHeight     = 540;
        constexpr int maxWidth      = 2400;
        constexpr int maxHeight     = 1440;
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

    // Premium typography: small, tight, UPPERCASE labels, precise values
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
    // Premium: tight radius, professional precision
    constexpr float outerRadius   = BTZTokens::Radius::md;   // 3px outer
    constexpr float panelRadius   = BTZTokens::Radius::md;   // 3px panels
    constexpr float controlRadius = BTZTokens::Radius::sm;   // 2px controls
    constexpr float pillRadius    = 12.0f;
    constexpr float glassRadius   = BTZTokens::Radius::md;   // 3px

    constexpr int knobLarge       = 56;    // smaller, denser
    constexpr int knobMedium      = 44;
    constexpr int knobSmall       = 34;
    constexpr int knobMacro       = BTZTokens::Dim::macroKnobDefault;
    constexpr int knobSimple      = BTZTokens::Dim::simpleKnobSize;

    constexpr int sliderHeight    = 24;
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
    constexpr int meterStripHeight = 64;

    constexpr int tabWidth        = 80;
    constexpr int tabGap          = BTZTokens::Space::md;
    constexpr int tabHeight       = 26;

    constexpr int knobLabelHeight = 16;
}

namespace Effects {
    struct ShadowSpec {
        juce::Colour colour;
        int radius;
        juce::Point<int> offset;
    };

    // Premium: subtle shadows on dark (less visible, more about depth)
    inline ShadowSpec panelShadow()   { return { juce::Colour(0x30000000), 8, { 0, 2 } }; }
    inline ShadowSpec controlShadow() { return { juce::Colour(0x20000000), 4, { 0, 1 } }; }
    inline ShadowSpec innerShadow()   { return { juce::Colour(0x40000000), 2, { 0, 1 } }; }
    inline ShadowSpec glassShadow()   { return { juce::Colour(0x28000000), 6, { 0, 2 } }; }

    inline juce::Colour knobGlow()      { return BTZColours::oak.withAlpha(BTZColours::oakSoftGlowAlpha); }
    inline juce::Colour knobHoverGlow() { return BTZColours::oak.withAlpha(BTZColours::hoverGlowAlpha); }
    inline juce::Colour knobActiveGlow(){ return BTZColours::oak.withAlpha(BTZColours::activeGlowAlpha); }
    inline juce::Colour meterGlow()     { return BTZColours::sage.withAlpha(0.08f); }

    constexpr float meterCornerRadius = 1.5f;  // thin, precise
    constexpr float meterSegmentGap   = 1.0f;
    constexpr int   meterSegmentCount = 32;    // more segments = more precise
}

namespace KnobStyle {
    constexpr float arcThicknessRatio = 0.045f;  // thinner arcs — precise
    constexpr float bodyRadiusRatio   = 0.72f;
    constexpr float indicatorStart    = 0.28f;
    constexpr float indicatorEnd      = 0.62f;
    constexpr float tickDotRadius     = 1.2f;    // tiny, precise dots
    constexpr int   tickDotCount      = BTZTokens::Dim::tickDotCount;
    constexpr float startAngle        = 1.25f;   // * pi
    constexpr float endAngle          = 2.75f;   // * pi

    // Premium: metallic feel — subtle 3D
    constexpr float innerShadowOpacity = 0.20f;
    constexpr float rimLightOpacity    = 0.15f;
    constexpr float convexGradientBias = 0.35f;
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

// Draw a premium panel: raised surface via luminance, minimal border
inline void drawFramedPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::panelRadius) {
    // Subtle shadow for depth
    juce::DropShadow shadow(juce::Colour(0x28000000), 6, { 0, 2 });
    shadow.drawForRectangle(g, area.toNearestInt());

    // Panel fill — slightly lighter than canvas
    g.setColour(BTZColours::panel);
    g.fillRoundedRectangle(area, cornerRadius);

    // Top edge highlight — subtle metallic feel
    g.setColour(juce::Colour(0x0AFFFFFF));
    g.drawLine(area.getX() + cornerRadius, area.getY() + 0.5f,
               area.getRight() - cornerRadius, area.getY() + 0.5f, 0.5f);
}

// Draw a frosted glass panel — dark, subtle
inline void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::glassRadius) {
    juce::DropShadow shadow(juce::Colour(0x28000000), 6, { 0, 2 });
    shadow.drawForRectangle(g, area.toNearestInt());

    g.setColour(BTZColours::glassMedium);
    g.fillRoundedRectangle(area, cornerRadius);

    // Top edge highlight
    g.setColour(juce::Colour(0x0CFFFFFF));
    g.drawLine(area.getX() + cornerRadius, area.getY() + 0.5f,
               area.getRight() - cornerRadius, area.getY() + 0.5f, 0.5f);
}

// Draw warm dark background with subtle radial warmth
inline void drawRadialBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    juce::ColourGradient grad(
        BTZColours::canvasWarm, (float)area.getCentreX(), (float)area.getCentreY() * 0.4f,
        BTZColours::canvas, 0.0f, (float)area.getHeight(),
        true
    );
    g.setGradientFill(grad);
    g.fillRect(area);
}

// Draw a minimal section divider — luminance shift, not a visible line
inline void drawSectionRule(juce::Graphics& g, float x, float y, float width, bool /*thick*/ = false) {
    g.setColour(BTZColours::hairline);
    g.fillRect(x, y, width, 1.0f);
}

// Draw very subtle noise texture — adds analog feel to dark surfaces
inline void drawNoiseTexture(juce::Graphics& g, juce::Rectangle<int> area, uint32_t seed = 42) {
    g.setColour(juce::Colour(0x06FFFFFF));
    uint32_t s = seed;
    for (int i = 0; i < 80; ++i) {
        s = s * 1664525u + 1013904223u;
        float x = (float)(s % (uint32_t)area.getWidth());
        s = s * 1664525u + 1013904223u;
        float y = (float)(s % (uint32_t)area.getHeight());
        g.fillEllipse(x, y, 0.6f, 0.6f);
    }
}

} // namespace BTZTheme
