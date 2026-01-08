cmake_minimum_required(VERSION 3.15)

project(BTZ_Tests VERSION 1.0.0)

# Add JUCE
if(NOT TARGET juce::juce_audio_processors)
    add_subdirectory(../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
endif()

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

target_compile_definitions(lifecycle_stress_test
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

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

target_compile_definitions(state_roundtrip_test
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

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

target_compile_definitions(bypass_bitperfect_test
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

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

target_compile_definitions(automation_torture_test
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

# Enable testing
enable_testing()
add_test(NAME LifecycleStressTest COMMAND lifecycle_stress_test --iterations 50)
add_test(NAME ParameterConversionTest COMMAND parameter_conversion_test)
add_test(NAME StateRoundTripTest COMMAND state_roundtrip_test)
add_test(NAME BypassBitPerfectTest COMMAND bypass_bitperfect_test)
add_test(NAME AutomationTortureTest COMMAND automation_torture_test)
