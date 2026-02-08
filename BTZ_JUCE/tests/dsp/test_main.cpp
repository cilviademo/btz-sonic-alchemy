/*
  BTZ DSP Unit Tests - Main Entry Point

  P2 Sprint - TASK-001: Set up Google Test framework
  Requirements: REQ-STAB-001 (DSP Unit Test Suite)
  Acceptance: AC-STAB-001 (47+ tests, 80% coverage)

  This file provides the Google Test main function for all DSP unit tests.

  Usage:
    ./DSP_unit_tests                              # Run all tests
    ./DSP_unit_tests --gtest_filter=Saturation.*  # Run specific test suite
    ./DSP_unit_tests --gtest_list_tests          # List all tests

  Test Suites:
    - AdvancedSaturationTest (8 tests)
    - EnhancedSPARKTest (10 tests)
    - EnhancedSHINETest (8 tests)
    - LUFSMeterTest (10 tests)
    - TransientShaperTest (6 tests)
    - ParameterSmootherTest (5 tests)

  Total: 47+ tests
*/

#include <gtest/gtest.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Google Test main function
// GTest::gtest_main is linked, so we just need this minimal setup
int main(int argc, char** argv)
{
    // Initialize JUCE for DSP tests
    juce::initialiseJuce_GUI();

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    int result = RUN_ALL_TESTS();

    // Cleanup JUCE
    juce::shutdownJuce_GUI();

    return result;
}
