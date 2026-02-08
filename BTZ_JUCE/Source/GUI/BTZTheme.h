/*
  ==============================================================================

  BTZTheme.h

  Modern color palette and design system for BTZ UI

  Design System (Phase 4: Output Thermal/Portal Inspired):
  - Base: Warm beige/tan neutrals
  - Accents: Sage green (primary), Natural oak brown (secondary)
  - Typography: Charcoal black for contrast, clean professional hierarchy
  - Knobs: 3D beveled with top-left warm lighting and gentle shadows
  - Spacing: 8px grid system

  Color Palette: Beige + Sage Green + Natural Oak + Black (no neon gradients)

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace BTZTheme
{
    // =========================================================================
    // COLOR PALETTE - Beige + Sage Green + Natural Oak + Black
    // =========================================================================

    namespace Colors
    {
        // Background Neutrals (Beige/Tan base)
        const juce::Colour background       = juce::Colour(0xFFF5F1E8);  // Warm cream
        const juce::Colour backgroundDark   = juce::Colour(0xFFE8E1D3);  // Tan
        const juce::Colour backgroundLight  = juce::Colour(0xFFFAF8F3);  // Off-white

        // Panel Backgrounds
        const juce::Colour panelBackground  = juce::Colour(0xFFEDE6D8);  // Warm beige
        const juce::Colour panelBorder      = juce::Colour(0xFFD4C9B5);  // Border tan

        // Primary Accent (Sage Green)
        const juce::Colour primary          = juce::Colour(0xFF9CAF88);  // Sage green
        const juce::Colour primaryDark      = juce::Colour(0xFF6D7A67);  // Dark sage
        const juce::Colour primaryLight     = juce::Colour(0xFFB5C9A3);  // Light sage

        // Secondary Accent (Natural Oak Brown)
        const juce::Colour secondary        = juce::Colour(0xFF8B7355);  // Natural oak
        const juce::Colour secondaryDark    = juce::Colour(0xFF6B5642);  // Dark oak
        const juce::Colour secondaryLight   = juce::Colour(0xFFA89279);  // Light oak

        // Text Colors (with true black option)
        const juce::Colour textPrimary      = juce::Colour(0xFF1A1A1A);  // Charcoal black
        const juce::Colour textSecondary    = juce::Colour(0xFF6B5D4F);  // Medium brown
        const juce::Colour textDisabled     = juce::Colour(0xFF9B8B7A);  // Light brown
        const juce::Colour textBlack        = juce::Colour(0xFF000000);  // Pure black

        // Knob/Control Colors (Output Thermal/Portal inspired)
        const juce::Colour knobFill         = juce::Colour(0xFF9CAF88);  // Sage green
        const juce::Colour knobOutline      = juce::Colour(0xFF6D7A67);  // Dark sage
        const juce::Colour knobBackground   = juce::Colour(0xFFE8E1D3);  // Tan
        const juce::Colour knobPointer      = juce::Colour(0xFF8B7355);  // Natural oak
        const juce::Colour knobHighlight    = juce::Colour(0xFFFAF8F3);  // Top-left lighting
        const juce::Colour knobShadow       = juce::Colour(0xFF6B5642);  // Bottom-right shadow

        // Meter Colors
        const juce::Colour meterLow         = juce::Colour(0xFF9CAF88);  // Sage (safe)
        const juce::Colour meterMid         = juce::Colour(0xFF8B7355);  // Natural oak (caution)
        const juce::Colour meterHigh        = juce::Colour(0xFF6B5642);  // Dark oak (warn)
        const juce::Colour meterBackground  = juce::Colour(0xFFD4C9B5);  // Border tan

        // Button States
        const juce::Colour buttonNormal     = juce::Colour(0xFF9CAF88);  // Sage
        const juce::Colour buttonHover      = juce::Colour(0xFFB5C9A3);  // Light sage
        const juce::Colour buttonActive     = juce::Colour(0xFF6D7A67);  // Dark sage
        const juce::Colour buttonDisabled   = juce::Colour(0xFFD4C9B5);  // Border tan
    }

    // =========================================================================
    // TYPOGRAPHY
    // =========================================================================

    namespace Fonts
    {
        inline juce::Font getTitle()    { return juce::Font(24.0f, juce::Font::bold); }
        inline juce::Font getHeading()  { return juce::Font(18.0f, juce::Font::bold); }
        inline juce::Font getBody()     { return juce::Font(14.0f, juce::Font::plain); }
        inline juce::Font getLabel()    { return juce::Font(12.0f, juce::Font::plain); }
        inline juce::Font getCaption()  { return juce::Font(10.0f, juce::Font::plain); }
        inline juce::Font getValue()    { return juce::Font(13.0f, juce::Font::bold); }
    }

    // =========================================================================
    // SPACING & LAYOUT
    // =========================================================================

    namespace Layout
    {
        const int gridSize = 8;

        // Margins
        const int marginXS  = gridSize;      // 8px
        const int marginS   = gridSize * 2;  // 16px
        const int marginM   = gridSize * 3;  // 24px
        const int marginL   = gridSize * 4;  // 32px
        const int marginXL  = gridSize * 6;  // 48px

        // Component Sizes
        const int knobSize      = 80;
        const int knobSizeLarge = 100;
        const int knobSizeSmall = 60;

        const int buttonHeight  = 32;
        const int buttonWidth   = 80;

        const int meterWidth    = 12;
        const int meterHeight   = 200;

        // Border Radius
        const float cornerRadius   = 8.0f;
        const float cornerRadiusS  = 4.0f;
        const float cornerRadiusL  = 12.0f;
    }

    // =========================================================================
    // UI CONSTANTS
    // =========================================================================

    namespace UI
    {
        // Window Dimensions
        const int windowWidth = 900;
        const int windowHeight = 600;

        // Meter Update Rate (Hz)
        const int meterRefreshRate = 30;  // 30 Hz for smooth but efficient metering

        // Animation Durations (ms)
        const int animationFast   = 150;
        const int animationNormal = 250;
        const int animationSlow   = 400;
    }

    // =========================================================================
    // HELPER FUNCTIONS
    // =========================================================================

    inline juce::Colour withAlpha(const juce::Colour& color, float alpha)
    {
        return color.withAlpha(alpha);
    }

    inline juce::Colour lighten(const juce::Colour& color, float amount)
    {
        return color.brighter(amount);
    }

    inline juce::Colour darken(const juce::Colour& color, float amount)
    {
        return color.darker(amount);
    }

} // namespace BTZTheme
