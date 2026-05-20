/*
  Box Tone Zone (BTZ) — BTZTheme.h  v5
  ────────────────────────────────────────────────────────────────────────
  v5 (Palette Restoration — Original Tan/Oak/Sage Aesthetic):
    • Restored the original warm, minimalistic light-mode palette
    • Canvas/tan backgrounds, oak (warm orange) primary accent, sage green secondary
    • Retains all v4 UX infrastructure: glassmorphism, animation, accessibility,
      harmonic visualizer, Simple Mode, resolution independence
    • "Glassmorphism" adapted for light mode: frosted cream panels with subtle shadows
    • Knob bodies: warm cream/bone with oak accent arcs and sage fills
    • Meters: oak-to-sage gradient (matching V1 arc fill)
    • Dark text on light backgrounds for readability

  Design philosophy: Minimalistic. Warm. Restrained. Let the sound do the talking.
  ────────────────────────────────────────────────────────────────────────
  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — the canonical color token system (Tan/Oak/Sage palette)
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Background tier (warm cream/tan) ──
    static const juce::Colour canvas        { 0xFFF1EFEA };  // primary background
    static const juce::Colour canvasWarm    { 0xFFF5F2EC };  // center of radial gradient (lighter)
    static const juce::Colour panel         { 0xFFE8E3D9 };  // panel background
    static const juce::Colour panelRaised   { 0xFFEDE9E0 };  // elevated panel (slightly lighter)
    static const juce::Colour well          { 0xFFD4CEC2 };  // inset/well background
    static const juce::Colour wellDeep      { 0xFFC8C2B6 };  // deeper inset
    static const juce::Colour hairline      { 0xFFBFB9AD };  // subtle dividers
    static const juce::Colour rule          { 0xFFD4CEC2 };  // horizontal rules

    // ── Ink & typography (dark on light) ──
    static const juce::Colour text          { 0xFF1A1A18 };  // primary text (near-black)
    static const juce::Colour textSecondary { 0xFF4A4640 };  // secondary text (dark brown)
    static const juce::Colour textTertiary  { 0xFF918B82 };  // muted labels
    static const juce::Colour textDisabled  { 0xFFB8B2A8 };  // disabled state
    static const juce::Colour textInverse   { 0xFFF1EFEA };  // text on dark surfaces

    // ── Knob body (warm cream, minimal) ──
    static const juce::Colour knobBody      { 0xFFF0ECE4 };  // main knob fill
    static const juce::Colour knobHighlight { 0xFFFAF8F4 };  // top rim highlight
    static const juce::Colour knobShadow    { 0xFFD4CEC2 };  // bottom shadow
    static const juce::Colour knobInnerShadow { 0xFFC2BDB2 };  // convexity shadow
    static const juce::Colour knobRimLight  { 0xFFFFFFFF };  // top rim pure white

    // ── Primary accent: Oak (warm orange/amber-brown) ──
    static const juce::Colour oak           { 0xFFB08D57 };  // primary accent
    static const juce::Colour oakBright     { 0xFFC9A462 };  // active/hover state
    static const juce::Colour oakDim        { 0xFF8A6E43 };  // dimmed state
    static const juce::Colour oakDark       { 0xFF6B5533 };  // dark variant
    static const juce::Colour oakLight      { 0xFFD4B87A };  // light variant

    // ── Secondary accent: Sage (muted green) ──
    static const juce::Colour sage          { 0xFF7E9B8E };  // secondary accent
    static const juce::Colour sageBright    { 0xFF96B5A6 };  // active/hover
    static const juce::Colour sageDim       { 0xFF5F7A6D };  // dimmed
    static const juce::Colour sageDark      { 0xFF4A5F54 };  // dark variant
    static const juce::Colour sageLight     { 0xFFA8C4B6 };  // light variant

    // ── Tertiary accent: Terracotta (muted red for warnings/GR) ──
    static const juce::Colour terracotta    { 0xFFC0543E };  // warning/GR accent
    static const juce::Colour terracottaDim { 0xFF9A4332 };  // dimmed
    static const juce::Colour terracottaLight { 0xFFD97A66 }; // light variant

    // ── Reserved accents ──
    static const juce::Colour slate         { 0xFF5A6B7A };  // neutral cool accent
    static const juce::Colour wheat         { 0xFFD4B87A };  // warm highlight

    // ── Glassmorphism (adapted for light mode — frosted cream) ──
    static const juce::Colour glassLight    { 0x40FFFFFF };  // 25% white overlay
    static const juce::Colour glassMedium   { 0x66FFFFFF };  // 40% white overlay
    static const juce::Colour glassBorder   { 0x33FFFFFF };  // 20% white border
    static const juce::Colour glassHighlight{ 0x80FFFFFF };  // 50% white top edge
    static const juce::Colour glassShadow   { 0x1A000000 };  // 10% black bottom

    // ── Harmonic overtone colors (warm palette) ──
    static const juce::Colour harmonic1     { 0xFFB08D57 };  // fundamental — oak
    static const juce::Colour harmonic2     { 0xFFC9A462 };  // 2nd — bright oak
    static const juce::Colour harmonic3     { 0xFFC0543E };  // 3rd — terracotta
    static const juce::Colour harmonic4     { 0xFFD4884A };  // 4th — warm orange
    static const juce::Colour harmonic5     { 0xFF7E9B8E };  // 5th — sage
    static const juce::Colour harmonic6     { 0xFF5A6B7A };  // 6th — slate
    static const juce::Colour harmonic7     { 0xFF96B5A6 };  // 7th — bright sage
    static const juce::Colour harmonic8     { 0xFF8A6E43 };  // 8th — dim oak

    inline juce::Colour harmonicColour(int index) noexcept {
        static const juce::Colour table[] = {
            harmonic1, harmonic2, harmonic3, harmonic4,
            harmonic5, harmonic6, harmonic7, harmonic8
        };
        return table[juce::jlimit(0, 7, index)];
    }

    // ── State colors ──
    static const juce::Colour stateSafe     { 0xFF7E9B8E };  // sage = safe
    static const juce::Colour stateWarn     { 0xFFB08D57 };  // oak = warning
    static const juce::Colour stateDanger   { 0xFFC0543E };  // terracotta = danger
    static const juce::Colour stateClip     { 0xFFA03020 };  // deep red = clip

    // ── Meter zones ──
    static const juce::Colour meterSafe     { 0xFFD4CEC2 };  // well = background
    static const juce::Colour meterOptimal  { 0xFF7E9B8E };  // sage = optimal
    static const juce::Colour meterHot      { 0xFFB08D57 };  // oak = hot
    static const juce::Colour meterClip     { 0xFFC0543E };  // terracotta = clip

    // ── Glow alpha values (subtle for light mode) ──
    constexpr float oakGlowAlpha       = 0.15f;
    constexpr float oakSoftGlowAlpha   = 0.08f;
    constexpr float sageGlowAlpha      = 0.12f;
    constexpr float hoverGlowAlpha     = 0.08f;
    constexpr float activeGlowAlpha    = 0.20f;

    // Legacy aliases for code that references old names
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

    // ── Module accessor ──
    enum class Module { BTZ, SPARK, SHINE, TONE, MOTION, BLOOM, Utility };

    inline juce::Colour accentFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return oak;
            case Module::SPARK:   return terracotta;
            case Module::SHINE:   return sage;
            case Module::TONE:    return sageBright;
            case Module::MOTION:  return slate;
            case Module::BLOOM:   return wheat;
            case Module::Utility: return textSecondary;
        }
        return oak;
    }

    inline juce::Colour accentDimFor(Module m) noexcept {
        switch (m) {
            case Module::BTZ:     return oakDim;
            case Module::SPARK:   return terracottaDim;
            case Module::SHINE:   return sageDim;
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
        constexpr int sec   = 32;
        constexpr int secMd = 40;
        constexpr int secLg = 48;
        constexpr int page  = 64;
    }

    namespace Radius {
        constexpr float none    = 0.0f;
        constexpr float sm      = 4.0f;
        constexpr float md      = 8.0f;
        constexpr float lg      = 12.0f;
        constexpr float xl      = 16.0f;
        constexpr float pill    = 9999.0f;
    }

    namespace Border {
        constexpr float hairline = 1.0f;
        constexpr float focus    = 1.5f;
        constexpr float glass    = 0.5f;
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

    // ── Animation & Micro-interaction Timing ──
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

    // ── Glassmorphism Parameters (light-mode adapted) ──
    namespace Glass {
        constexpr float blurRadius     = 12.0f;
        constexpr float panelOpacity   = 0.35f;   // more opaque for light mode
        constexpr float borderOpacity  = 0.20f;
        constexpr float shadowBlur     = 8.0f;    // softer shadows for light mode
        constexpr float innerHighlight = 0.40f;
        constexpr float noiseOpacity   = 0.02f;   // very subtle
    }

    // ── Accessibility ──
    namespace Accessibility {
        constexpr float minContrastRatio = 4.5f;
        constexpr float largeTextContrast = 3.0f;
        constexpr int   minTouchTarget   = 44;
        constexpr int   focusRingWidth   = 2;
        static const juce::Colour focusRing { 0xFF7E9B8E };  // sage focus indicator
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

        // Simple Mode large knobs
        constexpr int simpleKnobSize   = 160;
        constexpr int simpleKnobGap    = 48;

        // Harmonic visualizer
        constexpr int harmonicBarWidth = 6;
        constexpr int harmonicBarGap   = 3;
        constexpr int harmonicDisplayHeight = 200;
        constexpr int harmonicMaxBars  = 16;

        // Preset browser panel
        constexpr int presetPanelWidth = 280;
        constexpr int presetRowHeight  = 32;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.3f;
        constexpr float bypassed   = 0.4f;
        constexpr float background = 0.7f;
        constexpr float glassPanel = 0.90f;
        constexpr float ghostTrace = 0.3f;
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
// 3. Legacy compatibility — BTZTheme:: namespace
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZTheme {

namespace Color {
    inline juce::Colour backgroundRoot()        { return BTZColours::canvas; }
    inline juce::Colour backgroundCenter()      { return BTZColours::canvasWarm; }
    inline juce::Colour backgroundPanel()       { return BTZColours::panel; }
    inline juce::Colour backgroundPanelRaised() { return BTZColours::panelRaised; }
    inline juce::Colour backgroundInset()       { return BTZColours::well; }
    inline juce::Colour strokeSubtle()          { return BTZColours::hairline; }
    inline juce::Colour strokeStrong()          { return juce::Colour(0xFFADA79C); }
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
    inline juce::Colour disabled()              { return BTZColours::hairline; }
    inline juce::Colour grMeter()               { return BTZColours::terracotta; }
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
    inline juce::Font simpleValue()   { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font simpleLabel()   { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeBase, juce::Font::bold); }
}

namespace Geometry {
    constexpr float outerRadius   = 0.0f;
    constexpr float panelRadius   = 0.0f;
    constexpr float controlRadius = 0.0f;
    constexpr float pillRadius    = 12.0f;
    constexpr float glassRadius   = BTZTokens::Radius::md;

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

    constexpr int padPanel        = BTZTokens::Space::lg;
    constexpr int padSection      = BTZTokens::Space::md;
    constexpr int padContent      = BTZTokens::Space::xl;

    constexpr float borderThin    = 0.5f;
    constexpr float borderNormal  = BTZTokens::Border::hairline;
    constexpr float borderThick   = BTZTokens::Border::focus;
    constexpr float borderGlass   = BTZTokens::Border::glass;

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

    // Light-mode shadows: softer, more diffuse
    inline ShadowSpec panelShadow()   { return { juce::Colour(0x18000000), 12, { 0, 3 } }; }
    inline ShadowSpec controlShadow() { return { juce::Colour(0x14000000), 6, { 0, 2 } }; }
    inline ShadowSpec innerShadow()   { return { juce::Colour(0x20000000), 3, { 0, 1 } }; }
    inline ShadowSpec glassShadow()   { return { juce::Colour(0x18000000), 8, { 0, 3 } }; }

    inline juce::Colour knobGlow()      { return BTZColours::oak.withAlpha(BTZColours::oakSoftGlowAlpha); }
    inline juce::Colour knobHoverGlow() { return BTZColours::oak.withAlpha(BTZColours::hoverGlowAlpha); }
    inline juce::Colour knobActiveGlow(){ return BTZColours::oak.withAlpha(BTZColours::activeGlowAlpha); }
    inline juce::Colour meterGlow()     { return BTZColours::sage.withAlpha(0.15f); }

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

    // 3D depth (subtle for minimalistic aesthetic)
    constexpr float innerShadowOpacity = 0.12f;
    constexpr float rimLightOpacity    = 0.3f;
    constexpr float convexGradientBias = 0.5f;
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

// Draw a frosted glass panel (light-mode: cream frost with soft shadow)
inline void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::glassRadius) {
    // Soft drop shadow
    juce::DropShadow shadow(juce::Colour(0x18000000), 8, { 0, 3 });
    shadow.drawForRectangle(g, area.toNearestInt());

    // Frosted cream fill
    g.setColour(BTZColours::glassMedium);
    g.fillRoundedRectangle(area, cornerRadius);

    // Top highlight edge
    g.setColour(BTZColours::glassHighlight);
    g.drawHorizontalLine((int)area.getY(), area.getX() + cornerRadius, area.getRight() - cornerRadius);

    // Subtle border
    g.setColour(BTZColours::glassBorder);
    g.drawRoundedRectangle(area, cornerRadius, BTZTokens::Border::glass);
}

// Draw warm radial gradient background (light mode: canvas center → slightly darker edges)
inline void drawRadialBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    juce::ColourGradient grad(
        BTZColours::canvasWarm, (float)area.getCentreX(), (float)area.getCentreY() * 0.4f,
        BTZColours::canvas, 0.0f, (float)area.getHeight(),
        true  // radial
    );
    g.setGradientFill(grad);
    g.fillRect(area);
}

// Draw subtle noise texture overlay (very light for warm paper feel)
inline void drawNoiseTexture(juce::Graphics& g, juce::Rectangle<int> area, uint32_t seed = 42) {
    g.setColour(juce::Colour(0x06000000));  // very subtle dark dots
    uint32_t s = seed;
    for (int i = 0; i < 150; ++i) {
        s = s * 1664525u + 1013904223u;
        float x = (float)(s % (uint32_t)area.getWidth());
        s = s * 1664525u + 1013904223u;
        float y = (float)(s % (uint32_t)area.getHeight());
        g.fillEllipse(x, y, 1.0f, 1.0f);
    }
}

} // namespace BTZTheme
