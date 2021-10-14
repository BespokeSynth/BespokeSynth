juce_add_gui_app(BespokeVSTScanner
        )
target_compile_definitions(BespokeVSTScanner PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0

        JUCE_COREGRAPHICS_DRAW_ASYNC=1

        JUCE_ALSA=1
        JUCE_JACK=1
        JUCE_JACK_CLIENT_NAME="BespokeVSTScanner"

        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1

        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_VST=$<BOOL:${BESPOKE_VST2_SDK_LOCATION}>
        )
target_sources(BespokeVSTScanner PRIVATE Main.cpp MainComponent.cpp)
target_link_libraries(BespokeVSTScanner PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_opengl
        juce::juce_osc
        )
