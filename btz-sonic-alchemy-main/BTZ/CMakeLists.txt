cmake_minimum_required(VERSION 3.22)
project(BTZ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ═══════════════════════════════════════════════════════════════════════════
# JUCE discovery: env var → local path → FetchContent fallback
# ═══════════════════════════════════════════════════════════════════════════
# Default JUCE path — set JUCE_DIR or JUCE_ROOT env var, or it will auto-fetch
set(BTZ_DEFAULT_JUCE_PATH "${CMAKE_SOURCE_DIR}/../JUCE")
set(BTZ_JUCE_PATH "")

if(DEFINED ENV{JUCE_DIR} AND EXISTS "$ENV{JUCE_DIR}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "$ENV{JUCE_DIR}")
    message(STATUS "BTZ: Using JUCE from JUCE_DIR=$ENV{JUCE_DIR}")
elseif(DEFINED ENV{JUCE_ROOT} AND EXISTS "$ENV{JUCE_ROOT}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "$ENV{JUCE_ROOT}")
    message(STATUS "BTZ: Using JUCE from JUCE_ROOT=$ENV{JUCE_ROOT}")
elseif(EXISTS "${BTZ_DEFAULT_JUCE_PATH}/CMakeLists.txt")
    set(BTZ_JUCE_PATH "${BTZ_DEFAULT_JUCE_PATH}")
    message(STATUS "BTZ: Using JUCE from default path: ${BTZ_DEFAULT_JUCE_PATH}")
else()
    message(STATUS "BTZ: No local JUCE found. Fetching JUCE 8.0.6 via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.6
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(juce)
    set(BTZ_JUCE_FETCHED TRUE)
endif()

if(NOT BTZ_JUCE_FETCHED)
    add_subdirectory("${BTZ_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
endif()

# ═══════════════════════════════════════════════════════════════════════════
# CLAP format support (optional, enable with -DBTZ_BUILD_CLAP=ON)
# Requires clap-juce-extensions: https://github.com/free-audio/clap-juce-extensions
# ═══════════════════════════════════════════════════════════════════════════
option(BTZ_BUILD_CLAP "Build CLAP format via clap-juce-extensions" ON)

if(BTZ_BUILD_CLAP)
    include(FetchContent)
    FetchContent_Declare(
        clap-juce-extensions
        GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG        main
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(clap-juce-extensions)
    message(STATUS "BTZ: CLAP format enabled via clap-juce-extensions")
endif()

# ═══════════════════════════════════════════════════════════════════════════
# Plugin target
# ═══════════════════════════════════════════════════════════════════════════
juce_add_plugin(BTZ
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Box Tone Zone (BTZ)"
    PLUGIN_MANUFACTURER_CODE BTZa
    PLUGIN_CODE Btz1
    COMPANY_NAME "BTZ Audio"
    BUNDLE_ID "com.btzaudio.btz"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# v7: Register CLAP format if enabled
if(BTZ_BUILD_CLAP)
    clap_juce_extensions_plugin(TARGET BTZ
        CLAP_ID "com.btzaudio.btz"
        CLAP_FEATURES audio-effect mixing mastering
    )
endif()

juce_generate_juce_header(BTZ)

target_sources(BTZ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/BTZDsp.h
    Source/BTZTheme.h
    Source/BTZComponents.h
    Source/PluginProcessor.h
    Source/PluginEditor.h
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

# ═══════════════════════════════════════════════════════════════════════════
# Test target (optional, enable with -DBTZ_BUILD_TESTS=ON)
# ═══════════════════════════════════════════════════════════════════════════
option(BTZ_BUILD_TESTS "Build BTZ unit tests" OFF)

if(BTZ_BUILD_TESTS)
    enable_testing()

    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(BTZTests
        tests/test_dsp_modules.cpp
    )

    target_include_directories(BTZTests PRIVATE Source)
    target_link_libraries(BTZTests PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        gtest_main
    )

    # Register with CTest
    include(GoogleTest)
    gtest_discover_tests(BTZTests)
endif()

# ═══════════════════════════════════════════════════════════════════════════
# Validator integration targets (manual, not built by default)
# Usage: cmake --build . --target validate_vst3
#        cmake --build . --target validate_au
# ═══════════════════════════════════════════════════════════════════════════

# pluginval — cross-platform VST3/AU/CLAP validator
# Download from: https://github.com/Tracktion/pluginval/releases
# Set PLUGINVAL_PATH to the pluginval executable
if(DEFINED ENV{PLUGINVAL_PATH})
    set(PLUGINVAL "$ENV{PLUGINVAL_PATH}")

    # VST3 validation (strictness 10 = maximum)
    add_custom_target(validate_vst3
        COMMAND ${PLUGINVAL}
            --validate "$<TARGET_FILE:BTZ_VST3>"
            --strictness-level 10
            --timeout-ms 120000
            --repeat-count 2
            --randomise
            --validate-in-process
        DEPENDS BTZ_VST3
        COMMENT "Running pluginval on BTZ VST3 (strictness 10)..."
    )

    # AU validation (macOS only)
    if(APPLE)
        add_custom_target(validate_au
            COMMAND ${PLUGINVAL}
                --validate "$<TARGET_FILE:BTZ_AU>"
                --strictness-level 10
                --timeout-ms 120000
                --repeat-count 2
                --randomise
            DEPENDS BTZ_AU
            COMMENT "Running pluginval on BTZ AU (strictness 10)..."
        )

        # auvaltool — Apple's native AU validator
        add_custom_target(validate_auvaltool
            COMMAND auvaltool -v aufx Btz1 BTZa
            DEPENDS BTZ_AU
            COMMENT "Running auvaltool on BTZ AU..."
        )
    endif()

    # VST3 SDK validator (if available)
    if(DEFINED ENV{VST3_VALIDATOR_PATH})
        add_custom_target(validate_vst3_sdk
            COMMAND "$ENV{VST3_VALIDATOR_PATH}" "$<TARGET_FILE:BTZ_VST3>"
            DEPENDS BTZ_VST3
            COMMENT "Running VST3 SDK validator on BTZ..."
        )
    endif()
else()
    message(STATUS "BTZ: PLUGINVAL_PATH not set. Validator targets disabled.")
    message(STATUS "BTZ: Set PLUGINVAL_PATH env var to enable: export PLUGINVAL_PATH=/path/to/pluginval")
endif()
