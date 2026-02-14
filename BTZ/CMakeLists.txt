cmake_minimum_required(VERSION 3.15)
project(BTZ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JUCE CONFIG REQUIRED)

# Options
option(WITH_ML "Enable ML integrations (Torch, model loaders)" OFF)

if (WITH_ML)
  find_package(Torch REQUIRED)
endif()

add_library(BTZ SHARED
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/WaveShaper.cpp
    Source/EQ.cpp
    Source/TransientShaper.cpp
    Source/MultiFilterDelay.cpp
    Source/DeepFilterNet.cpp
    Source/TimbralTransfer.cpp
    Source/Fuzzed.cpp
    Source/BassEnhancer.cpp
    Source/LimiterNo6.cpp
    Source/TapeEmulator.cpp
    Source/GranularProcessor.cpp
    Source/GateProcessor.cpp
    Source/ConsoleEmulator.cpp
)

target_include_directories(BTZ PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
)

if (WITH_ML)
  target_compile_definitions(BTZ PRIVATE BTZ_WITH_ML=1)
  target_link_libraries(BTZ PRIVATE ${TORCH_LIBRARIES})
endif()

target_link_libraries(BTZ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

set_target_properties(BTZ PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/VST3"
    SUFFIX ".vst3"
)
