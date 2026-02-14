cmake_minimum_required(VERSION 3.22)
project(BTZ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# JUCE discovery strategy:
# 1) Use JUCE_DIR/JUCE_ROOT env variables if provided
# 2) Fallback to known local paths (workstation/submodule layouts)
# 3) Auto-fetch JUCE when local source is unavailable
set(BTZ_DEFAULT_JUCE_PATH "C:/Users/marcm/OneDrive/Desktop/JUCE-8.0.6")
set(BTZ_JUCE_GIT_TAG "8.0.6" CACHE STRING "JUCE tag used by FetchContent fallback")
set(BTZ_JUCE_PATH "")
if(DEFINED ENV{JUCE_DIR} AND EXISTS "$ENV{JUCE_DIR}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "$ENV{JUCE_DIR}")
elseif(DEFINED ENV{JUCE_ROOT} AND EXISTS "$ENV{JUCE_ROOT}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "$ENV{JUCE_ROOT}")
elseif(EXISTS "${BTZ_DEFAULT_JUCE_PATH}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "${BTZ_DEFAULT_JUCE_PATH}")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/JUCE/CMakeLists.txt")
    set(BTZ_JUCE_PATH "${CMAKE_CURRENT_LIST_DIR}/JUCE")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../JUCE/CMakeLists.txt")
    set(BTZ_JUCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../JUCE")
endif()

if(BTZ_JUCE_PATH)
    message(STATUS "Using JUCE source at: ${BTZ_JUCE_PATH}")
    add_subdirectory("${BTZ_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    message(STATUS "JUCE source not found locally. Fetching JUCE ${BTZ_JUCE_GIT_TAG} with FetchContent.")
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${BTZ_JUCE_GIT_TAG}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(BTZ
    FORMATS VST3 Standalone
    PRODUCT_NAME "Box Tone Zone (BTZ)"
    PLUGIN_MANUFACTURER_CODE BTZa
    PLUGIN_CODE Btz1
    COMPANY_NAME "BTZ Audio"
    BUNDLE_ID "com.btzaudio.btz"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(BTZ)

target_sources(BTZ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(BTZ PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(BTZ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
