/*
  Box Tone Zone (BTZ) — BTZTheme.h
  Design tokens and component specifications aligned with moodboard v0.2.
  This file centralizes all visual constants for the plugin UI.
  Use these tokens in LookAndFeel and component layout code.
*/
#pragma once

#include <JuceHeader.h>

namespace BTZTheme {

// ═══════════════════════════════════════════════════════════════════════════
// Color Palette — Moodboard v0.2 "Near-black chassis, amber accent"
// ═══════════════════════════════════════════════════════════════════════════
namespace Color {
    // Base layers (darkest to lightest)
    constexpr juce::uint32 obsidian  = 0xFF0A0A0D;  // deepest background
    constexpr juce::uint32 panelBg   = 0xFF13141A;  // panel background
    constexpr juce::uint32 charcoal  = 0xFF1A1C23;  // raised surfaces / wells
    constexpr juce::uint32 slate     = 0xFF252830;  // hover / active states

    // Accent
    constexpr juce::uint32 amber     = 0xFFE8A94A;  // primary accent (knob arcs, active indicators)
    constexpr juce::uint32 amberDim  = 0xFF9C7230;  // dimmed amber (inactive arcs)
    constexpr juce::uint32 amberGlow = 0x40E8A94A;  // glow effect (25% opacity)

    // Text
    constexpr juce::uint32 cream     = 0xFFEAE0CC;  // primary text
    constexpr juce::uint32 bone      = 0xFFD8CFB8;  // secondary text / labels
    constexpr juce::uint32 muted     = 0xFF6B6B6B;  // disabled / tertiary text

    // Data visualization (IR/thermal heat spectrum)
    constexpr juce::uint32 heatCool  = 0xFF2A4858;  // cool end (low energy)
    constexpr juce::uint32 heatWarm  = 0xFFE8A94A;  // warm (mid energy)
    constexpr juce::uint32 heatHot   = 0xFFD94F6B;  // hot (high energy)
    constexpr juce::uint32 heatPeak  = 0xFF9B59B6;  // peak (violet)

    // Semantic
    constexpr juce::uint32 clipRed   = 0xFFC0543E;  // clip indicator
    constexpr juce::uint32 safeGreen = 0xFF4A8B6A;  // safe / nominal
}

// ═══════════════════════════════════════════════════════════════════════════
// Typography — Moodboard v0.2 font stack
// ═══════════════════════════════════════════════════════════════════════════
namespace Font {
    // Display / headers: Syne 800 (fallback to system bold)
    inline juce::String displayFamily() { return "Syne"; }
    constexpr float displayWeight = 800.0f;

    // Body: Inter Tight 300-600 (fallback to system sans)
    inline juce::String bodyFamily() { return "Inter Tight"; }
    constexpr float bodyWeightLight   = 300.0f;
    constexpr float bodyWeightRegular = 400.0f;
    constexpr float bodyWeightMedium  = 500.0f;
    constexpr float bodyWeightSemiBold = 600.0f;

    // Technical / labels: IBM Plex Mono 300-600 (fallback to monospace)
    inline juce::String monoFamily() { return "IBM Plex Mono"; }
    constexpr float monoWeightLight   = 300.0f;
    constexpr float monoWeightRegular = 400.0f;

    // Sizes
    constexpr float sizeTitle    = 16.0f;
    constexpr float sizeHeader   = 12.0f;
    constexpr float sizeBody     = 10.0f;
    constexpr float sizeLabel    = 8.5f;
    constexpr float sizeReadout  = 9.0f;
    constexpr float sizeMicro    = 7.0f;
}

// ═══════════════════════════════════════════════════════════════════════════
// Layout — Component dimensions and spacing
// ═══════════════════════════════════════════════════════════════════════════
namespace Layout {
    // Plugin window
    constexpr int windowWidth  = 980;
    constexpr int windowHeight = 650;
    constexpr float cornerRadius = 10.0f;

    // 3-column grid (moodboard spec)
    constexpr int sidebarWidth = 260;
    // Center = remaining space (1fr)

    // Knob dimensions
    constexpr int knobDiameter     = 74;
    constexpr int knobLabelHeight  = 16;
    constexpr int macroKnobDiameter = 60;

    // Spacing
    constexpr int panelPadding = 14;
    constexpr int sectionGap   = 12;
    constexpr int knobGap      = 8;

    // Header
    constexpr int headerHeight = 54;
    constexpr int meterStripHeight = 78;

    // Tab bar
    constexpr int tabWidth  = 90;
    constexpr int tabGap    = 12;
}

// ═══════════════════════════════════════════════════════════════════════════
// Component IDs — Moodboard component vocabulary
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
// Knob Style — v2 spec from moodboard
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

} // namespace BTZTheme
