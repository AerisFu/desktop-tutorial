cmake_minimum_required(VERSION 3.15)

project(VST3ReverbPlugin VERSION 1.0.0)

# Add JUCE
add_subdirectory(JUCE)

# Add the plugin target
juce_add_plugin(VST3ReverbPlugin
    VERSION "1.0.0"
    COMPANY_NAME "Ryan Fu"
    PLUGIN_MANUFACTURER_CODE RyFu
    PLUGIN_CODE Vst3
    FORMATS VST3 Standalone
    PRODUCT_NAME "Ultimate Reverb"
    DESCRIPTION "State-of-the-art reverb plugin with advanced FDN algorithm"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Generate JUCE header
juce_generate_juce_header(VST3ReverbPlugin)

# Add source files
target_sources(VST3ReverbPlugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Compile definitions
target_compile_definitions(VST3ReverbPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link libraries
target_link_libraries(VST3ReverbPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)