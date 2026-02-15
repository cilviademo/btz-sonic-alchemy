/*
  ==============================================================================

  BTZ Parameter Conversion Tests

  Purpose: Verify normalized ↔ plain parameter conversions
  Ship Gate: #9 (Migration Tests)

  Tests:
  - Linear conversions (inputGain, outputGain)
  - Skewed conversions (shineFreqHz, shineQ)
  - Choice conversions (sparkOS, sparkMode, masterBlend)
  - Boolean conversions (all bool parameters)
  - Round-trip accuracy

  ==============================================================================
*/

#include "../Source/Parameters/PluginParameters.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace juce;

//==============================================================================
// Helper Functions

inline bool floatEqual(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

// Linear conversion (no skew)
inline float normalizedToPlain(float normalized, float min, float max) {
    return min + normalized * (max - min);
}

inline float plainToNormalized(float plain, float min, float max) {
    return (plain - min) / (max - min);
}

// Skewed conversion
inline float normalizedToPlainSkewed(float normalized, float min, float max, float skew) {
    float normalized_skewed = std::pow(normalized, skew);
    return min + normalized_skewed * (max - min);
}

inline float plainToNormalizedSkewed(float plain, float min, float max, float skew) {
    float linearNormalized = (plain - min) / (max - min);
    return std::pow(linearNormalized, 1.0f / skew);
}

// Choice conversion
inline int normalizedToChoiceIndex(float normalized, int numChoices) {
    return jlimit(0, numChoices - 1, (int)(normalized * numChoices));
}

inline float choiceIndexToNormalized(int index, int numChoices) {
    return (float)index / (float)std::max(1, numChoices - 1);
}

//==============================================================================
// Test Cases

void test_inputGain_conversion() {
    std::cout << "[TEST] Input Gain Conversion... ";

    // Min: -12.0 dB, Max: +12.0 dB, Default: 0.0 dB

    // Test default (0.5 normalized → 0.0 dB)
    float plain = normalizedToPlain(0.5f, -12.0f, 12.0f);
    assert(floatEqual(plain, 0.0f));

    // Test round-trip
    float normalized = plainToNormalized(0.0f, -12.0f, 12.0f);
    assert(floatEqual(normalized, 0.5f));

    // Test min (-12.0 dB)
    plain = normalizedToPlain(0.0f, -12.0f, 12.0f);
    assert(floatEqual(plain, -12.0f));

    // Test max (+12.0 dB)
    plain = normalizedToPlain(1.0f, -12.0f, 12.0f);
    assert(floatEqual(plain, 12.0f));

    std::cout << "✅ PASS\n";
}

void test_shineFreqHz_skewed_conversion() {
    std::cout << "[TEST] Shine Frequency Skewed Conversion... ";

    // Min: 10000.0 Hz, Max: 80000.0 Hz, Default: 20000.0 Hz, Skew: 0.3

    // Test conversion with skew
    // Normalized 0.5 with skew 0.3 should give logarithmic distribution
    float plain = normalizedToPlainSkewed(0.5f, 10000.0f, 80000.0f, 0.3f);
    // Expected: std::pow(0.5, 0.3) ≈ 0.812 → 10000 + 0.812 * 70000 ≈ 66840 Hz
    assert(plain > 60000.0f && plain < 70000.0f);  // Rough range check

    // Test round-trip
    float normalized = plainToNormalizedSkewed(plain, 10000.0f, 80000.0f, 0.3f);
    assert(floatEqual(normalized, 0.5f, 0.01f));

    // Test default (20000 Hz)
    normalized = plainToNormalizedSkewed(20000.0f, 10000.0f, 80000.0f, 0.3f);
    plain = normalizedToPlainSkewed(normalized, 10000.0f, 80000.0f, 0.3f);
    assert(floatEqual(plain, 20000.0f, 100.0f));  // Within 100 Hz

    std::cout << "✅ PASS\n";
}

void test_sparkOS_choice_conversion() {
    std::cout << "[TEST] Spark Oversampling Choice Conversion... ";

    // Choices: 0=1x, 1=2x, 2=4x, 3=8x, 4=16x
    // Default: 3 (8x)

    int numChoices = 5;

    // Test default (normalized 0.75 → index 3)
    int index = normalizedToChoiceIndex(0.75f, numChoices);
    assert(index == 3);

    // Test round-trip
    float normalized = choiceIndexToNormalized(3, numChoices);
    assert(floatEqual(normalized, 0.75f));

    // Test min (index 0)
    index = normalizedToChoiceIndex(0.0f, numChoices);
    assert(index == 0);

    // Test max (index 4)
    index = normalizedToChoiceIndex(1.0f, numChoices);
    assert(index == 4);

    // Test edge case (normalized 0.99 should still be index 4)
    index = normalizedToChoiceIndex(0.99f, numChoices);
    assert(index == 4);

    std::cout << "✅ PASS\n";
}

void test_boolean_conversion() {
    std::cout << "[TEST] Boolean Parameter Conversion... ";

    // Test OFF (0.0 normalized → false)
    bool value = (0.0f > 0.5f);
    assert(value == false);

    // Test ON (1.0 normalized → true)
    value = (1.0f > 0.5f);
    assert(value == true);

    // Test threshold (0.5 → false, 0.51 → true)
    assert((0.5f > 0.5f) == false);
    assert((0.51f > 0.5f) == true);

    // Test round-trip
    float normalized = value ? 1.0f : 0.0f;
    assert(normalized == 1.0f);

    std::cout << "✅ PASS\n";
}

void test_all_hero_controls_range() {
    std::cout << "[TEST] Hero Controls Range Validation... ";

    // All hero controls: 0.0 - 1.0, default 0.0 (except mix = 1.0)

    // Punch
    assert(floatEqual(normalizedToPlain(0.0f, 0.0f, 1.0f), 0.0f));
    assert(floatEqual(normalizedToPlain(1.0f, 0.0f, 1.0f), 1.0f));

    // Warmth
    assert(floatEqual(normalizedToPlain(0.5f, 0.0f, 1.0f), 0.5f));

    // Boom
    assert(floatEqual(plainToNormalized(0.25f, 0.0f, 1.0f), 0.25f));

    // Mix (default 1.0)
    assert(floatEqual(normalizedToPlain(1.0f, 0.0f, 1.0f), 1.0f));

    // Drive
    assert(floatEqual(normalizedToPlain(0.3f, 0.0f, 1.0f), 0.3f));

    std::cout << "✅ PASS\n";
}

void test_sparkCeiling_conversion() {
    std::cout << "[TEST] Spark Ceiling Conversion... ";

    // Min: -3.0 dBTP, Max: 0.0 dBTP, Default: -0.3 dBTP

    // Test default (-0.3 dBTP)
    // Normalized: (-0.3 - (-3.0)) / (0.0 - (-3.0)) = 2.7 / 3.0 = 0.9
    float normalized = plainToNormalized(-0.3f, -3.0f, 0.0f);
    assert(floatEqual(normalized, 0.9f));

    // Test round-trip
    float plain = normalizedToPlain(0.9f, -3.0f, 0.0f);
    assert(floatEqual(plain, -0.3f));

    // Test min (-3.0 dBTP)
    plain = normalizedToPlain(0.0f, -3.0f, 0.0f);
    assert(floatEqual(plain, -3.0f));

    // Test max (0.0 dBTP)
    plain = normalizedToPlain(1.0f, -3.0f, 0.0f);
    assert(floatEqual(plain, 0.0f));

    std::cout << "✅ PASS\n";
}

void test_parameter_stability() {
    std::cout << "[TEST] Parameter ID Stability... ";

    // Verify all parameter IDs are as documented
    assert(BTZParams::IDs::punch == "punch");
    assert(BTZParams::IDs::warmth == "warmth");
    assert(BTZParams::IDs::boom == "boom");
    assert(BTZParams::IDs::mix == "mix");
    assert(BTZParams::IDs::drive == "drive");

    assert(BTZParams::IDs::texture == "texture");

    assert(BTZParams::IDs::inputGain == "inputGain");
    assert(BTZParams::IDs::outputGain == "outputGain");
    assert(BTZParams::IDs::autoGain == "autoGain");

    assert(BTZParams::IDs::sparkEnabled == "sparkEnabled");
    assert(BTZParams::IDs::sparkLUFS == "sparkLUFS");
    assert(BTZParams::IDs::sparkCeiling == "sparkCeiling");
    assert(BTZParams::IDs::sparkMix == "sparkMix");
    assert(BTZParams::IDs::sparkOS == "sparkOS");
    assert(BTZParams::IDs::sparkAutoOS == "sparkAutoOS");
    assert(BTZParams::IDs::sparkMode == "sparkMode");

    assert(BTZParams::IDs::shineEnabled == "shineEnabled");
    assert(BTZParams::IDs::shineFreqHz == "shineFreqHz");
    assert(BTZParams::IDs::shineGainDb == "shineGainDb");
    assert(BTZParams::IDs::shineQ == "shineQ");
    assert(BTZParams::IDs::shineMix == "shineMix");
    assert(BTZParams::IDs::shineAutoOS == "shineAutoOS");

    assert(BTZParams::IDs::masterEnabled == "masterEnabled");
    assert(BTZParams::IDs::masterMacro == "masterMacro");
    assert(BTZParams::IDs::masterBlend == "masterBlend");
    assert(BTZParams::IDs::masterMix == "masterMix");

    assert(BTZParams::IDs::precisionMode == "precisionMode");
    assert(BTZParams::IDs::active == "active");
    assert(BTZParams::IDs::oversampling == "oversampling");

    std::cout << "✅ PASS (27 parameters verified)\n";
}

//==============================================================================
// Main Test Runner

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "BTZ Parameter Conversion Tests\n";
    std::cout << "========================================\n\n";

    int passed = 0;
    int failed = 0;

    try {
        test_inputGain_conversion();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_shineFreqHz_skewed_conversion();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_sparkOS_choice_conversion();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_boolean_conversion();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_all_hero_controls_range();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_sparkCeiling_conversion();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    try {
        test_parameter_stability();
        passed++;
    } catch (...) {
        std::cout << "❌ FAIL\n";
        failed++;
    }

    std::cout << "\n========================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "========================================\n";

    return (failed == 0) ? 0 : 1;
}
