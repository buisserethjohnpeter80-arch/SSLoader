cmake_minimum_required(VERSION 3.22)
project(SSLoader VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(SSLoader
    COMPANY_NAME "SSLoader"
    BUNDLE_ID "com.ssloader.audio"
    PLUGIN_MANUFACTURER_CODE SsLd
    PLUGIN_CODE SsL1
    FORMATS VST3 Standalone
    PRODUCT_NAME "SSLoader"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(SSLoader PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(SSLoader PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SSLoader PRIVATE juce::juce_audio_utils juce::juce_gui_extra juce::juce_dsp juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
