cmake_minimum_required(VERSION 3.22)

project(BTZ_Benchmark VERSION 1.0.0)

# Find JUCE
if(NOT TARGET juce::juce_audio_processors)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../BTZ_JUCE/JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
endif()

# CPU Benchmark executable
add_executable(cpu_benchmark
    cpu_benchmark.cpp
    ../../BTZ_JUCE/Source/PluginProcessor.cpp
    ../../BTZ_JUCE/Source/PluginEditor.cpp
    ../../BTZ_JUCE/Source/DSP/TransientShaper.cpp
    ../../BTZ_JUCE/Source/DSP/Saturation.cpp
    ../../BTZ_JUCE/Source/DSP/SubHarmonic.cpp
    ../../BTZ_JUCE/Source/DSP/SparkLimiter.cpp
    ../../BTZ_JUCE/Source/DSP/ShineEQ.cpp
    ../../BTZ_JUCE/Source/DSP/ConsoleEmulator.cpp
    ../../BTZ_JUCE/Source/DSP/Oversampling.cpp
    ../../BTZ_JUCE/Source/GUI/ThermalKnob.cpp
    ../../BTZ_JUCE/Source/GUI/MeterStrip.cpp
)

target_include_directories(cpu_benchmark PRIVATE
    ../../BTZ_JUCE/Source
    ../../BTZ_JUCE/JUCE/modules
)

target_compile_definitions(cpu_benchmark PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JucePlugin_Name="BTZ"
    JucePlugin_VersionString="1.0.0"
    JucePlugin_ManufacturerCode=0x42747a7a
    JucePlugin_PluginCode=0x42747a70
)

target_link_libraries(cpu_benchmark PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
)

# Load Time Benchmark executable
add_executable(load_time_benchmark
    load_time_benchmark.cpp
    ../../BTZ_JUCE/Source/PluginProcessor.cpp
    ../../BTZ_JUCE/Source/PluginEditor.cpp
    ../../BTZ_JUCE/Source/DSP/TransientShaper.cpp
    ../../BTZ_JUCE/Source/DSP/Saturation.cpp
    ../../BTZ_JUCE/Source/DSP/SubHarmonic.cpp
    ../../BTZ_JUCE/Source/DSP/SparkLimiter.cpp
    ../../BTZ_JUCE/Source/DSP/ShineEQ.cpp
    ../../BTZ_JUCE/Source/DSP/ConsoleEmulator.cpp
    ../../BTZ_JUCE/Source/DSP/Oversampling.cpp
    ../../BTZ_JUCE/Source/GUI/ThermalKnob.cpp
    ../../BTZ_JUCE/Source/GUI/MeterStrip.cpp
)

target_include_directories(load_time_benchmark PRIVATE
    ../../BTZ_JUCE/Source
    ../../BTZ_JUCE/JUCE/modules
)

target_compile_definitions(load_time_benchmark PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JucePlugin_Name="BTZ"
    JucePlugin_VersionString="1.0.0"
    JucePlugin_ManufacturerCode=0x42747a7a
    JucePlugin_PluginCode=0x42747a70
)

target_link_libraries(load_time_benchmark PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
)

# Automation Spike Benchmark executable
add_executable(automation_spike_benchmark
    automation_spike_benchmark.cpp
    ../../BTZ_JUCE/Source/PluginProcessor.cpp
    ../../BTZ_JUCE/Source/PluginEditor.cpp
    ../../BTZ_JUCE/Source/DSP/TransientShaper.cpp
    ../../BTZ_JUCE/Source/DSP/Saturation.cpp
    ../../BTZ_JUCE/Source/DSP/SubHarmonic.cpp
    ../../BTZ_JUCE/Source/DSP/SparkLimiter.cpp
    ../../BTZ_JUCE/Source/DSP/ShineEQ.cpp
    ../../BTZ_JUCE/Source/DSP/ConsoleEmulator.cpp
    ../../BTZ_JUCE/Source/DSP/Oversampling.cpp
    ../../BTZ_JUCE/Source/GUI/ThermalKnob.cpp
    ../../BTZ_JUCE/Source/GUI/MeterStrip.cpp
)

target_include_directories(automation_spike_benchmark PRIVATE
    ../../BTZ_JUCE/Source
    ../../BTZ_JUCE/JUCE/modules
)

target_compile_definitions(automation_spike_benchmark PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JucePlugin_Name="BTZ"
    JucePlugin_VersionString="1.0.0"
    JucePlugin_ManufacturerCode=0x42747a7a
    JucePlugin_PluginCode=0x42747a70
)

target_link_libraries(automation_spike_benchmark PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
)

# Install targets
install(TARGETS cpu_benchmark load_time_benchmark automation_spike_benchmark
    RUNTIME DESTINATION bin
)
