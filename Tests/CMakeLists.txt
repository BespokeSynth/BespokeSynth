add_executable(BespokeAudioBridgeTests
    StreamingAudioResamplerTest.cpp
    ../Source/StreamingAudioResampler.cpp)
target_include_directories(BespokeAudioBridgeTests PRIVATE ../Source)
target_link_libraries(BespokeAudioBridgeTests PRIVATE SampleRate::samplerate)
add_test(NAME BespokeAudioBridgeTests COMMAND BespokeAudioBridgeTests)
