cmake_minimum_required(VERSION 3.22)
project(Orbita VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(Orbita
    COMPANY_NAME "Polaris Sound Lab" BUNDLE_ID "mx.polarissoundlab.orbita"
    PLUGIN_MANUFACTURER_CODE Pslb PLUGIN_CODE Orb1
    FORMATS VST3 AU Standalone IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice VST3_CATEGORIES Instrument Synth COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "Polaris Orbita")
target_sources(Orbita PRIVATE Processor.cpp)
target_compile_features(Orbita PRIVATE cxx_std_17)
target_compile_definitions(Orbita PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(Orbita PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
