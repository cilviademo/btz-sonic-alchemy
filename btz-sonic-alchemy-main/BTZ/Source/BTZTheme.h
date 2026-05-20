/*
  Box Tone Zone (BTZ) — BTZTheme.h  v4
  ────────────────────────────────────────────────────────────────────────
  v4 (UI/UX Overhaul):
    • Glassmorphism: frosted glass panel tokens, backdrop blur constants
    • Animation: micro-interaction timing, easing curves, glow keyframes
    • Accessibility: contrast ratios, shape-coded indicators, high-contrast mode
    • Refined aesthetics: radial gradient background, noise texture, 3D knob depth
    • Resolution independence: scale-factor aware dimensions
    • Harmonic visualizer: color mapping for overtone display
    • Simple Mode: large-knob layout tokens

  v3: Ecosystem-aligned design token system (BTZColours + BTZTokens merged)
  ────────────────────────────────────────────────────────────────────────
  SINGLE SOURCE OF TRUTH for every visual constant.
  If a hex value appears in paint() that isn't from this header, it's a bug.
*/
#pragma once

#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════
// 1. BTZColours — the canonical color token system
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZColours {

    // ── Chassis tier (refined with subtle warmth) ──
    static const juce::Colour obsidian      { 0xFF0A0A0D };
    static const juce::Colour obsidianWarm  { 0xFF0C0B10 };  // v4: center of radial gradient
    static const juce::Colour panel         { 0xFF13141A };
    static const juce::Colour panelRaised   { 0xFF181A22 };  // v4: elevated panel
    static const juce::Colour charcoal      { 0xFF1A1C23 };
    static const juce::Colour stone         { 0xFF262932 };
    static const juce::Colour hairline      { 0xFF383B45 };
    static const juce::Colour rule          { 0xFF20232A };

    // ── Ink & typography ──
    static const juce::Colour cream         { 0xFFEAE0CC };
    static const juce::Colour bone          { 0xFFD8CFB8 };
    static const juce::Colour paper         { 0xFFB8AE98 };
    static const juce::Colour mute          { 0xFF7A7465 };
    static const juce::Colour deepMute      { 0xFF52503F };

    // ── Knob body gradient (v4: enhanced 3D depth) ──
    static const juce::Colour knobHighlight { 0xFFF4ECD8 };
    static const juce::Colour knobMid       { 0xFFEAE0CC };
    static const juce::Colour knobShadow    { 0xFFC2B89C };
    static const juce::Colour knobInnerShadow { 0xFF8A8070 };  // v4: convexity shadow
    static const juce::Colour knobRimLight  { 0xFFFFF8E8 };    // v4: top rim highlight

    // ── Module accents ──
    static const juce::Colour amber         { 0xFFE8A94A };
    static const juce::Colour amberDim      { 0xFFA07A32 };
    static const juce::Colour amberDark     { 0xFF6E5423 };
    static const juce::Colour amberBright   { 0xFFFFC65C };  // v4: active state glow

    static const juce::Colour coral         { 0xFFE8624A };
    static const juce::Colour coralDim      { 0xFFA04432 };
    static const juce::Colour coralDark     { 0xFF6E2F23 };

    static const juce::Colour cyan          { 0xFF6AD4E8 };
    static const juce::Colour cyanDim       { 0xFF3A92A3 };
    static const juce::Colour cyanDark      { 0xFF1F5A67 };

    // ── Reserved accents ──
    static const juce::Colour emerald       { 0xFF4AE8A0 };
    static const juce::Colour violet        { 0xFF7B5BE8 };
    static const juce::Colour sulfur        { 0xFFE8D94A };

    // ── Glassmorphism (v4) ──
    static const juce::Colour glassLight    { 0x1AFFFFFF };  // 10% white
    static const juce::Colour glassMedium   { 0x26FFFFFF };  // 15% white
    static const juce::Colour glassBorder   { 0x33FFFFFF };  // 20% white border
    static const juce::Colour glassHighlight{ 0x0DFFFFFF };  // 5% white top edge
    static const juce::Colour glassShadow   { 0x40000000 };  // 25% black bottom

    // ── Harmonic overtone colors (v4: for visualizer) ──
    static const juce::Colour harmonic1     { 0xFFE8A94A };  // fundamental — amber
    static const juce::Colour harmonic2     { 0xFFFFC65C };  // 2nd — bright gold
    static const juce::Colour harmonic3     { 0xFFE8624A };  // 3rd — coral
    static const juce::Colour harmonic4     { 0xFFFF8844 };  // 4th — orange
    static const juce::Colour harmonic5     { 0xFFD94A8E };  // 5th — magenta
    static const juce::Colour harmonic6     { 0xFF7B5BE8 };  // 6th — violet
    static const juce::Colour harmonic7     { 0xFF6AD4E8 };  // 7th — cyan
    static const juce::Colour harmonic8     { 0xFF4AE8A0 };  // 8th — emerald

    inline juce::Colour harmonicColour(int index) noexcept {
        static const juce::Colour table[] = {
            harmonic1, harmonic2, harmonic3, harmonic4,
            harmonic5, harmonic6, harmonic7, harmonic8
        };
        return table[juce::jlimit(0, 7, index)];
    }

    // ── Heat spectrum (visualizer) ──
    static const juce::Colour heatOrange    { 0xFFF28A3A };
    static const juce::Colour heatWarm      { 0xFFE8624A };
    static const juce::Colour heatMagenta   { 0xFFD94A8E };
    static const juce::Colour heatViolet    { 0xFF7B5BE8 };
    static const juce::Colour heatDeep      { 0xFF3A2E8E };

    // ── State colors ──
    static const juce::Colour stateSafe     { 0xFF4AE8A0 };
    static const juce::Colour stateWarn     { 0xFFE8A94A };
    static const juce::Colour stateDanger   { 0xFFE8624A };
    static const juce::Colour stateClip     { 0xFFFF3A1A };

    // ── Meter zones ──
    static const juce::Colour meterSafe     { 0xFF383B45 };
    static const juce::Colour meterOptimal  { 0xFFE8A94A };
    static const juce::Colour meterHot      { 0xFFE8624A };
    static const juce::Colour meterClip     { 0xFFFF3A1A };

    // ── Glow alpha values ──
    constexpr float amberGlowAlpha     = 0.25f;
    constexpr float amberSoftGlowAlpha = 0.15f;
    constexpr float coralGlowAlpha     = 0.20f;
    constexpr float cyanGlowAlpha      = 0.20f;
    constexpr float hoverGlowAlpha     = 0.12f;   // v4: subtle hover feedback
    constexpr float activeGlowAlpha    = 0.35f;   // v4: active knob glow

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
        constexpr float none    = 0.0f;
        constexpr float sm      = 4.0f;   // v4: subtle rounding for glass panels
        constexpr float md      = 8.0f;   // v4: modal dialogs
        constexpr float lg      = 12.0f;  // v4: preset browser
        constexpr float xl      = 16.0f;  // v4: floating panels
        constexpr float pill    = 9999.0f;
    }

    namespace Border {
        constexpr float hairline = 1.0f;
        constexpr float focus    = 1.5f;
        constexpr float glass    = 0.5f;  // v4: thin glass panel border
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

    // ── v4: Animation & Micro-interaction Timing ──
    namespace Animation {
        constexpr int instant     = 0;
        constexpr int microFast   = 60;    // hover feedback
        constexpr int fast        = 120;   // value changes
        constexpr int base        = 200;   // panel transitions
        constexpr int slow        = 400;   // mode switches
        constexpr int dramatic    = 600;   // startup reveal
        constexpr int tooltip     = 500;   // tooltip delay

        // Easing constants (for cubic bezier approximation)
        constexpr float easeOutExpo  = 0.16f;  // sharp deceleration
        constexpr float easeInOutCubic = 0.65f;
        constexpr float springDamping = 0.7f;   // knob inertia
        constexpr float springStiffness = 300.0f;

        // Meter/visualizer frame rates
        constexpr int meterFps        = 60;
        constexpr int spectrumFps     = 30;
        constexpr int harmonicFps     = 60;
        constexpr int idleFps         = 15;    // when plugin is in background
    }

    // ── v4: Glassmorphism Parameters ──
    namespace Glass {
        constexpr float blurRadius     = 12.0f;   // backdrop blur
        constexpr float panelOpacity   = 0.08f;   // base panel fill
        constexpr float borderOpacity  = 0.15f;   // edge highlight
        constexpr float shadowBlur     = 16.0f;   // drop shadow
        constexpr float innerHighlight = 0.05f;   // top-edge light
        constexpr float noiseOpacity   = 0.03f;   // subtle texture
    }

    // ── v4: Accessibility ──
    namespace Accessibility {
        constexpr float minContrastRatio = 4.5f;   // WCAG AA
        constexpr float largeTextContrast = 3.0f;  // WCAG AA for large text
        constexpr int   minTouchTarget   = 44;     // minimum clickable area
        constexpr int   focusRingWidth   = 2;
        static const juce::Colour focusRing { 0xFF6AD4E8 };  // cyan focus indicator
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

        // v4: Simple Mode large knobs
        constexpr int simpleKnobSize   = 160;
        constexpr int simpleKnobGap    = 48;

        // v4: Harmonic visualizer
        constexpr int harmonicBarWidth = 6;
        constexpr int harmonicBarGap   = 3;
        constexpr int harmonicDisplayHeight = 200;
        constexpr int harmonicMaxBars  = 16;

        // v4: Preset browser panel
        constexpr int presetPanelWidth = 280;
        constexpr int presetRowHeight  = 32;
    }

    namespace Opacity {
        constexpr float idle       = 1.0f;
        constexpr float hover      = 1.0f;
        constexpr float disabled   = 0.3f;
        constexpr float bypassed   = 0.4f;
        constexpr float background = 0.7f;
        constexpr float glassPanel = 0.85f;   // v4
        constexpr float ghostTrace = 0.3f;    // v4: before/after ghost overlay
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
// 3. Legacy compatibility aliases — BTZTheme:: namespace
// ═══════════════════════════════════════════════════════════════════════════
namespace BTZTheme {

namespace Color {
    inline juce::Colour backgroundRoot()        { return BTZColours::obsidian; }
    inline juce::Colour backgroundCenter()      { return BTZColours::obsidianWarm; }  // v4
    inline juce::Colour backgroundPanel()       { return BTZColours::panel; }
    inline juce::Colour backgroundPanelRaised() { return BTZColours::panelRaised; }
    inline juce::Colour backgroundInset()       { return juce::Colour(0xFF101218); }
    inline juce::Colour strokeSubtle()          { return BTZColours::hairline; }
    inline juce::Colour strokeStrong()          { return juce::Colour(0xFF3A3D48); }
    inline juce::Colour textPrimary()           { return BTZColours::cream; }
    inline juce::Colour textSecondary()         { return BTZColours::bone; }
    inline juce::Colour textDisabled()          { return BTZColours::mute; }
    inline juce::Colour accentPrimaryAmber()    { return BTZColours::amber; }
    inline juce::Colour accentBright()          { return BTZColours::amberBright; }  // v4
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
    // v4: Simple Mode large value display
    inline juce::Font simpleValue()   { return juce::Font(monoFamily(), BTZTokens::Font::Size::sizeMd, juce::Font::bold); }
    inline juce::Font simpleLabel()   { return juce::Font(displayFamily(), BTZTokens::Font::Size::sizeBase, juce::Font::bold); }
}

namespace Geometry {
    constexpr float outerRadius   = 0.0f;
    constexpr float panelRadius   = 0.0f;
    constexpr float controlRadius = 0.0f;
    constexpr float pillRadius    = 12.0f;
    constexpr float glassRadius   = BTZTokens::Radius::md;  // v4

    constexpr int knobLarge       = 74;
    constexpr int knobMedium      = 60;
    constexpr int knobSmall       = 44;
    constexpr int knobMacro       = BTZTokens::Dim::macroKnobDefault;
    constexpr int knobSimple      = BTZTokens::Dim::simpleKnobSize;  // v4

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
    constexpr float borderGlass   = BTZTokens::Border::glass;  // v4

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
    inline ShadowSpec glassShadow()   { return { juce::Colour(0x40000000), 12, { 0, 4 } }; }  // v4

    inline juce::Colour knobGlow()    { return BTZColours::amber.withAlpha(BTZColours::amberSoftGlowAlpha); }
    inline juce::Colour knobHoverGlow() { return BTZColours::amber.withAlpha(BTZColours::hoverGlowAlpha); }  // v4
    inline juce::Colour knobActiveGlow() { return BTZColours::amber.withAlpha(BTZColours::activeGlowAlpha); }  // v4
    inline juce::Colour meterGlow()   { return juce::Colour(0x204A8B6A); }

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

    // v4: 3D depth enhancement
    constexpr float innerShadowOpacity = 0.25f;
    constexpr float rimLightOpacity    = 0.4f;
    constexpr float convexGradientBias = 0.6f;  // how convex the knob appears
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
    // v4: New component types
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

// v4: Draw a glassmorphism panel
inline void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = Geometry::glassRadius) {
    // Outer shadow
    juce::DropShadow shadow(juce::Colour(0x40000000), 12, { 0, 4 });
    shadow.drawForRectangle(g, area.toNearestInt());

    // Glass fill
    g.setColour(BTZColours::glassMedium);
    g.fillRoundedRectangle(area, cornerRadius);

    // Top highlight edge
    g.setColour(BTZColours::glassHighlight);
    g.drawHorizontalLine((int)area.getY(), area.getX() + cornerRadius, area.getRight() - cornerRadius);

    // Border
    g.setColour(BTZColours::glassBorder);
    g.drawRoundedRectangle(area, cornerRadius, BTZTokens::Border::glass);
}

// v4: Draw radial gradient background
inline void drawRadialBackground(juce::Graphics& g, juce::Rectangle<int> area) {
    juce::ColourGradient grad(
        BTZColours::obsidianWarm, area.getCentreX(), area.getCentreY() * 0.4f,
        BTZColours::obsidian, 0.0f, (float)area.getHeight(),
        true  // radial
    );
    g.setGradientFill(grad);
    g.fillRect(area);
}

// v4: Draw noise texture overlay (call after background)
inline void drawNoiseTexture(juce::Graphics& g, juce::Rectangle<int> area, uint32_t seed = 42) {
    g.setColour(juce::Colour(0x08FFFFFF));  // very subtle
    // Pseudo-random dot pattern for texture
    uint32_t s = seed;
    for (int i = 0; i < 200; ++i) {
        s = s * 1664525u + 1013904223u;
        float x = (float)(s % (uint32_t)area.getWidth());
        s = s * 1664525u + 1013904223u;
        float y = (float)(s % (uint32_t)area.getHeight());
        g.fillEllipse(x, y, 1.0f, 1.0f);
    }
}

} // namespace BTZTheme
