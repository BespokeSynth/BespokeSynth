# Interfaces

add_library(core STATIC
        ChannelBuffer.cpp
        FileStream.cpp
        LFO.cpp
        ModulationChain.cpp
        ModularSynth.cpp
        ModuleSaveData.cpp
        Ramp.cpp
        RollingBuffer.cpp
        SynthGlobals.cpp
        Transport.cpp
        OpenFrameworksPort.cpp
        PatchCable.cpp
        PatchCableSource.cpp

        IAudioProcessor.cpp
        IAudioReceiver.cpp
        IAudioSource.cpp
        IClickable.cpp
        IDrawableModule.cpp
        IModulator.cpp
        INoteReceiver.cpp
        INoteSource.cpp
        IPulseReceiver.cpp
        IUIControl.cpp
        )

target_link_libraries(core PRIVATE
        juce::juce_gui_basics
        juce::juce_opengl)
target_include_directories(core PUBLIC .)
add_library(bespoke::core ALIAS core)
