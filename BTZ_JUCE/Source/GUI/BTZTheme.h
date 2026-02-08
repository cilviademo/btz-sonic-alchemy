/*
  ==============================================================================

  BTZTheme.h

  Modern color palette and design system for BTZ UI

  Design System:
  - Base: Warm beige/tan neutrals
  - Accents: Sage green (primary), Gold (highlights)
  - Typography: Clean, professional hierarchy
  - Spacing: 8px grid system

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace BTZTheme
{
    // =========================================================================
    // COLOR PALETTE
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
        const juce::Colour primary          = juce::Colour(0xFF8B9D83);  // Sage green
        const juce::Colour primaryDark      = juce::Colour(0xFF6D7A67);  // Dark sage
        const juce::Colour primaryLight     = juce::Colour(0xFFA8B9A0);  // Light sage

        // Secondary Accent (Gold)
        const juce::Colour secondary        = juce::Colour(0xFFD4AF37);  // Gold
        const juce::Colour secondaryDark    = juce::Colour(0xFFB8941C);  // Dark gold
        const juce::Colour secondaryLight   = juce::Colour(0xFFE8C96B);  // Light gold

        // Text Colors
        const juce::Colour textPrimary      = juce::Colour(0xFF2C2418);  // Dark brown
        const juce::Colour textSecondary    = juce::Colour(0xFF6B5D4F);  // Medium brown
        const juce::Colour textDisabled     = juce::Colour(0xFF9B8B7A);  // Light brown

        // Knob/Control Colors
        const juce::Colour knobFill         = juce::Colour(0xFF8B9D83);  // Sage
        const juce::Colour knobOutline      = juce::Colour(0xFF6D7A67);  // Dark sage
        const juce::Colour knobBackground   = juce::Colour(0xFFE8E1D3);  // Tan
        const juce::Colour knobPointer      = juce::Colour(0xFFD4AF37);  // Gold

        // Meter Colors
        const juce::Colour meterLow         = juce::Colour(0xFF8B9D83);  // Sage (safe)
        const juce::Colour meterMid         = juce::Colour(0xFFD4AF37);  // Gold (caution)
        const juce::Colour meterHigh        = juce::Colour(0xFFB8941C);  // Dark gold (warn)
        const juce::Colour meterBackground  = juce::Colour(0xFFD4C9B5);  // Border tan

        // Button States
        const juce::Colour buttonNormal     = juce::Colour(0xFF8B9D83);  // Sage
        const juce::Colour buttonHover      = juce::Colour(0xFFA8B9A0);  // Light sage
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
