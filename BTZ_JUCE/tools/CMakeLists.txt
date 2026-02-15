# BTZ Offline Rendering Tool
# Compile-time guarded: only built when BTZ_BUILD_TOOLS=ON
#
# Usage:
#   cmake .. -DBTZ_BUILD_TOOLS=ON
#   cmake --build . --target offline_render

if(BTZ_BUILD_TOOLS)
    message(STATUS "Building BTZ offline rendering tool")

    # Create console executable
    juce_add_console_app(offline_render
        PRODUCT_NAME "BTZ Offline Render"
    )

    target_sources(offline_render PRIVATE
        offline_render.cpp
        ../Source/PluginProcessor.cpp
        # Add only necessary source files for offline rendering
        # (avoid GUI dependencies)
    )

    target_compile_definitions(offline_render PRIVATE
        JUCE_STANDALONE_APPLICATION=1
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(offline_render PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
    )

    # Copy to tools output directory
    set_target_properties(offline_render PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools"
    )

    message(STATUS "Offline rendering tool will be built to: ${CMAKE_BINARY_DIR}/tools/")
else()
    message(STATUS "Skipping BTZ tools (set -DBTZ_BUILD_TOOLS=ON to enable)")
endif()
