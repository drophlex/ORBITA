cmake_minimum_required(VERSION 3.22)
project(ORBITA VERSION 2.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(juce)

# same codes and name as ÓRBITA 1, so it takes its place in your plug-in list
juce_add_plugin(Orbita
  COMPANY_NAME "Polaris Sound Lab"
  COMPANY_WEBSITE "https://drophlex.github.io/ORBITA/"
  BUNDLE_ID mx.polarissoundlab.orbita
  PLUGIN_MANUFACTURER_CODE Pslb
  PLUGIN_CODE Orb1
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "Polaris Orbita"
  DESCRIPTION "a generative FM instrument"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  NEEDS_WEB_BROWSER TRUE
  VST3_CATEGORIES Instrument Synth Generator
  AU_MAIN_TYPE kAudioUnitType_MusicDevice
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(OrbitaData SOURCES ui/index.html)
target_sources(Orbita PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(Orbita PRIVATE ../engine)
target_compile_definitions(Orbita PUBLIC
  JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(Orbita PRIVATE OrbitaData juce::juce_audio_utils juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
