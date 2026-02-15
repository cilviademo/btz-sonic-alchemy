cmake_minimum_required(VERSION 3.15)

project(BTZ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# JUCE Setup - Submodule or FetchContent
# ============================================================================
# Try submodule first, fall back to FetchContent if not available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(STATUS "Using JUCE submodule")
    add_subdirectory(JUCE)
else()
    message(STATUS "JUCE submodule not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ============================================================================
# Plugin Configuration
# ============================================================================
# Determine plugin formats based on platform
set(PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

# FL Studio scan safety: Must not do heavy work in constructor
# All DSP allocation happens in prepareToPlay()
juce_add_plugin(BTZ
    COMPANY_NAME "BTZ Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Btzz
    PLUGIN_CODE Btzp
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "BTZ - The Box Tone Zone"
    BUNDLE_ID com.btzaudio.btz
    VST3_CATEGORIES Fx Dynamics EQ
)

target_sources(BTZ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/TransientShaper.cpp
        Source/DSP/Saturation.cpp
        Source/DSP/SubHarmonic.cpp
        Source/DSP/SparkLimiter.cpp
        Source/DSP/ShineEQ.cpp
        Source/DSP/ConsoleEmulator.cpp
        Source/DSP/Oversampling.cpp
        Source/DSP/OversamplingManager.cpp
        # Phase 1 Enhanced Components
        Source/DSP/EnhancedSPARK.cpp
        Source/DSP/EnhancedSHINE.cpp
        Source/DSP/ComponentVariance.cpp
        Source/DSP/SafetyLayer.cpp
        # Phase 2 Realism Components
        Source/DSP/LongTermMemory.cpp
        Source/DSP/StereoEnhancement.cpp
        # Phase 3 Polish & Production
        Source/DSP/PerformanceGuardrails.cpp
        Source/DSP/DeterministicProcessing.cpp
        # Advanced DSP Modules (P0.3 validated)
        Source/DSP/AdvancedSaturation.cpp
        Source/DSP/LUFSMeter.cpp
        # Note: AdvancedTransientShaper and WDFSaturation excluded due to compilation issues
        # Can be re-enabled after fixing TPTOnePole API and WDF array initialization
        Source/GUI/ThermalKnob.cpp
        Source/GUI/MeterStrip.cpp
        Source/GUI/BTZKnob.cpp
        Source/GUI/BTZButton.cpp
        Source/GUI/MainView.cpp
        # P1.3: Preset Management
        Source/Utility/PresetManager.cpp)

# Header-only infrastructure files (included for IDE support)
# - Source/ProductionSafety.h (HostCallOrderGuard, RTSafeLogger)
# - Source/AutoDebugger.h (Crash-safe diagnostics)
# - Source/ABComparison.h (A/B comparison + Undo/Redo)
# - Source/LicenseSystem.h (License validation)
# - Source/DSP/ParameterSmoother.h (Lock-free parameter smoothing)
# - Source/DSP/OversamplingManager.h (Centralized oversampling)
# - Source/DSP/EnhancedSPARK.h (Phase 1: True-peak limiter with hysteresis)
# - Source/DSP/EnhancedSHINE.h (Phase 1: Psychoacoustic air enhancement)
# - Source/DSP/ComponentVariance.h (Phase 1: Analog component tolerance)
# - Source/DSP/SafetyLayer.h (Phase 1: DC/denormal/NaN protection)

target_compile_definitions(BTZ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0)

target_link_libraries(BTZ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
