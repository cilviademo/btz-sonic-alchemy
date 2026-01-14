cmake_minimum_required(VERSION 3.15)

project(BTZ VERSION 1.0.0)

# Add JUCE (assumes JUCE is in a sibling directory or installed)
# Adjust the path to your JUCE installation
add_subdirectory(JUCE)

# Define plugin formats to build
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
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "BTZ - The Box Tone Zone")

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
        # Temporarily disabled files with compilation issues:
        # Source/DSP/AdvancedSaturation.cpp
        # Source/DSP/AdvancedTransientShaper.cpp
        # Source/DSP/WDFSaturation.cpp
        # Source/DSP/LUFSMeter.cpp
        Source/GUI/ThermalKnob.cpp
        Source/GUI/MeterStrip.cpp
        Source/GUI/BTZKnob.cpp
        Source/GUI/BTZButton.cpp
        Source/GUI/MainView.cpp)

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
