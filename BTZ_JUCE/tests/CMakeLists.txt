cmake_minimum_required(VERSION 3.15)

project(BTZ_Tests VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
if(NOT TARGET juce::juce_audio_processors)
    add_subdirectory(../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
endif()

# Fetch Google Test (TASK-001: Set up Google Test framework)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Common plugin definitions for tests that instantiate BTZAudioProcessor
set(BTZ_PLUGIN_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JucePlugin_Name="BTZ"
    JucePlugin_Desc="BTZ Audio Enhancer"
    JucePlugin_Manufacturer="BTZ Audio"
    JucePlugin_ManufacturerCode=0x42747a7a
    JucePlugin_PluginCode=0x42747a70
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_Version=1.0.0
    JucePlugin_VersionCode=0x10000
    JucePlugin_VersionString="1.0.0"
)

# Lifecycle Stress Test Executable
add_executable(lifecycle_stress_test
    lifecycle_stress_test.cpp
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/DSP/TransientShaper.cpp
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/SubHarmonic.cpp
    ../Source/DSP/SparkLimiter.cpp
    ../Source/DSP/ShineEQ.cpp
    ../Source/DSP/ConsoleEmulator.cpp
    ../Source/DSP/Oversampling.cpp
    ../Source/GUI/ThermalKnob.cpp
    ../Source/GUI/MeterStrip.cpp
)

target_include_directories(lifecycle_stress_test PRIVATE ../Source)

target_link_libraries(lifecycle_stress_test
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(lifecycle_stress_test PRIVATE ${BTZ_PLUGIN_DEFINITIONS})

# Parameter Conversion Test (Ship Gate #9)
add_executable(parameter_conversion_test
    parameter_conversion_test.cpp
)

target_include_directories(parameter_conversion_test PRIVATE ../Source)

target_link_libraries(parameter_conversion_test
    PRIVATE
        juce::juce_audio_processors
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(parameter_conversion_test
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

# State Round-Trip Test (Ship Gate #9)
add_executable(state_roundtrip_test
    state_roundtrip_test.cpp
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/DSP/TransientShaper.cpp
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/SubHarmonic.cpp
    ../Source/DSP/SparkLimiter.cpp
    ../Source/DSP/ShineEQ.cpp
    ../Source/DSP/ConsoleEmulator.cpp
    ../Source/DSP/Oversampling.cpp
    ../Source/GUI/ThermalKnob.cpp
    ../Source/GUI/MeterStrip.cpp
)

target_include_directories(state_roundtrip_test PRIVATE ../Source)

target_link_libraries(state_roundtrip_test
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(state_roundtrip_test PRIVATE ${BTZ_PLUGIN_DEFINITIONS})

# Bypass Bit-Perfect Test (Ship Gate #8)
add_executable(bypass_bitperfect_test
    bypass_bitperfect_test.cpp
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/DSP/TransientShaper.cpp
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/SubHarmonic.cpp
    ../Source/DSP/SparkLimiter.cpp
    ../Source/DSP/ShineEQ.cpp
    ../Source/DSP/ConsoleEmulator.cpp
    ../Source/DSP/Oversampling.cpp
    ../Source/GUI/ThermalKnob.cpp
    ../Source/GUI/MeterStrip.cpp
)

target_include_directories(bypass_bitperfect_test PRIVATE ../Source)

target_link_libraries(bypass_bitperfect_test
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(bypass_bitperfect_test PRIVATE ${BTZ_PLUGIN_DEFINITIONS})

# Automation Torture Test (Ship Gate #7)
add_executable(automation_torture_test
    automation_torture_test.cpp
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/DSP/TransientShaper.cpp
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/SubHarmonic.cpp
    ../Source/DSP/SparkLimiter.cpp
    ../Source/DSP/ShineEQ.cpp
    ../Source/DSP/ConsoleEmulator.cpp
    ../Source/DSP/Oversampling.cpp
    ../Source/GUI/ThermalKnob.cpp
    ../Source/GUI/MeterStrip.cpp
)

target_include_directories(automation_torture_test PRIVATE ../Source)

target_link_libraries(automation_torture_test
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(automation_torture_test PRIVATE ${BTZ_PLUGIN_DEFINITIONS})

# =============================================================================
# TASK-001: DSP Unit Tests with Google Test
# =============================================================================
# These tests verify individual DSP module behavior with formal test cases
# Requirements: REQ-STAB-001 (DSP Unit Test Suite, 80% coverage)
# Acceptance: AC-STAB-001 (47+ tests across 6 modules)
#
# Test modules:
#   - AdvancedSaturation (8 tests: 6 modes + bypass + extreme values)
#   - EnhancedSPARK (10 tests: hysteresis, true-peak, ITU compliance)
#   - EnhancedSHINE (8 tests: 24 Bark bands, psychoacoustic weighting)
#   - LUFSMeter (10 tests: K-weighting, ITU-R BS.1770-4, gating)
#   - TransientShaper (6 tests: attack/release, ADAA, envelope)
#   - ParameterSmoother (5 tests: smoothing curves, RT-safety)
# =============================================================================

add_executable(DSP_unit_tests
    dsp/test_main.cpp
    dsp/test_AdvancedSaturation.cpp
    dsp/test_EnhancedSPARK.cpp
    dsp/test_EnhancedSHINE.cpp
    dsp/test_LUFSMeter.cpp
    dsp/test_TransientShaper.cpp
    dsp/test_ParameterSmoother.cpp
    # DSP module sources
    ../Source/DSP/AdvancedSaturation.cpp
    ../Source/DSP/EnhancedSPARK.cpp
    ../Source/DSP/EnhancedSHINE.cpp
    ../Source/DSP/LUFSMeter.cpp
    ../Source/DSP/TransientShaper.cpp
    # Supporting sources
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/SparkLimiter.cpp
    ../Source/DSP/ShineEQ.cpp
    ../Source/DSP/Oversampling.cpp
    ../Source/DSP/OversamplingManager.cpp
)

target_include_directories(DSP_unit_tests
    PRIVATE
        ../Source
        ${CMAKE_CURRENT_SOURCE_DIR}/dsp
)

target_link_libraries(DSP_unit_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(DSP_unit_tests
    PRIVATE
        ${BTZ_PLUGIN_DEFINITIONS}
)

# Enable testing
enable_testing()

# Integration tests (existing)
add_test(NAME LifecycleStressTest COMMAND lifecycle_stress_test --iterations 50)
add_test(NAME ParameterConversionTest COMMAND parameter_conversion_test)
add_test(NAME StateRoundTripTest COMMAND state_roundtrip_test)
add_test(NAME BypassBitPerfectTest COMMAND bypass_bitperfect_test)
add_test(NAME AutomationTortureTest COMMAND automation_torture_test)

# DSP unit tests (Google Test) - P2 Sprint
add_test(NAME DSP_UnitTests COMMAND DSP_unit_tests)

# Google Test discovery (allows running individual tests with --gtest_filter)
include(GoogleTest)
gtest_discover_tests(DSP_unit_tests)
