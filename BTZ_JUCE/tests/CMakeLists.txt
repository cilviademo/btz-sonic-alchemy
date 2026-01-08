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

# Enable testing
enable_testing()
add_test(NAME LifecycleStressTest COMMAND lifecycle_stress_test --iterations 50)
