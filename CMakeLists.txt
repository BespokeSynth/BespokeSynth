# Bespoke CMake file
#
# Options are
#   - BESPOKE_JUCE_LOCATION (default libs/JUCE) where you get your juce
#   - BESPOKE_VST2_SDK_LOCATION (default to nothing) where you get your SDK if you want non-FOSS software
#   - BESPOKE_ASIO_SDK_LOCATION (default to nothing) from https://www.steinberg.net/developers/

cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS version")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()


project(BespokeSynth VERSION 0.9.0 LANGUAGES C CXX ASM)

message(STATUS "Bespoke: Build Type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Bespoke: Configuring Python with PyBind")
if (NOT DEFINED PYBIND11_FINDPYTHON)
    if (WIN32)
      message(STATUS "Bespoke: PYBIND11_FINDPYTHON is unset; defaulting to OFF on Win32")
      set(PYBIND11_FINDPYTHON OFF)
    else()
      message(STATUS "Bespoke: PYBIND11_FINDPYTHON is unset; defaulting to ON")
      set(PYBIND11_FINDPYTHON ON)
    endif()  
endif()
add_subdirectory(libs/pybind11 CONFIG)
message(STATUS "Bespoke: Python Executable is ${PYTHON_EXECUTABLE}")

if (NOT BESPOKE_JUCE_LOCATION)
    set(BESPOKE_JUCE_LOCATION "libs/JUCE" CACHE STRING "" FORCE)
endif ()
message( STATUS "Bespoke: Using Juce from ${BESPOKE_JUCE_LOCATION}")
add_subdirectory(${BESPOKE_JUCE_LOCATION} ${CMAKE_BINARY_DIR}/JUCE EXCLUDE_FROM_ALL)

if (BESPOKE_VST2_SDK_LOCATION)
    juce_set_vst2_sdk_path(${BESPOKE_VST2_SDK_LOCATION})
    set( BESPOKE_VST2 1 )
    message( STATUS "Bespoke: Enabling VST2 - caution, non FOSS build" )
else()
    set( BESPOKE_VST2 0 )
    message( STATUS "Bespoke: No VST2 support" )
endif()

add_subdirectory(libs/xwax)

add_subdirectory(Source/freeverb)
add_subdirectory(Source/json)
add_subdirectory(Source/nanovg)


set(BESPOKE_SOURCE_LIST
        Source/glew/Bespoke_glew.c


        Source/push2/push2/JuceToPush2DisplayBridge.cpp
        Source/push2/libusb/libusb_platform_wrapper.c
        Source/push2/push2/Push2-Usb-Communicator.cpp
        Source/push2/Result.cpp

        Source/PerlinNoise.cpp
        Source/BitcrushEffect.cpp
        Source/ButterworthFilterEffect.cpp
        Source/Compressor.cpp
        Source/DCRemoverEffect.cpp
        Source/DelayEffect.cpp
        Source/DistortionEffect.cpp
        Source/EQEffect.cpp
        Source/FormantFilterEffect.cpp
        Source/FreeverbEffect.cpp
        Source/GainStageEffect.cpp
        Source/GateEffect.cpp
        Source/LiveGranulator.cpp
        Source/Muter.cpp
        Source/NoiseEffect.cpp
        Source/PitchShiftEffect.cpp
        Source/Pumper.cpp
        Source/Stutter.cpp
        Source/TremoloEffect.cpp
        Source/Amplifier.cpp
        Source/Arpeggiator.cpp
        Source/AudioLevelToCV.cpp
        Source/AudioMeter.cpp
        Source/AudioRouter.cpp
        Source/AudioSend.cpp
        Source/AudioToCV.cpp
        Source/AudioToPulse.cpp
        Source/Autotalent.cpp
        Source/BandVocoder.cpp
        Source/BeatBloks.cpp
        Source/Beats.cpp
        Source/BiquadFilterEffect.cpp
        Source/Capo.cpp
        Source/ChaosEngine.cpp
        Source/ChordDisplayer.cpp
        Source/Chorder.cpp
        Source/ChordHolder.cpp
        Source/CircleSequencer.cpp
        Source/ClipArranger.cpp
        Source/ClipLauncher.cpp
        Source/ComboGridController.cpp
        Source/CommentDisplay.cpp
        Source/ControllingSong.cpp
        Source/ControlSequencer.cpp
        Source/ControlTactileFeedback.cpp
        Source/CurveLooper.cpp
        Source/DCOffset.cpp
        Source/DebugAudioSource.cpp
        Source/DrumPlayer.cpp
        Source/DrumSynth.cpp
        Source/EffectChain.cpp
        Source/EnvelopeModulator.cpp
        Source/EQModule.cpp
        Source/EventCanvas.cpp
        Source/FeedbackModule.cpp
        Source/FFTtoAdditive.cpp
        Source/FMSynth.cpp
        Source/FollowingSong.cpp
        Source/FourOnTheFloor.cpp
        Source/FreqDelay.cpp
        Source/FreqDomainBoilerplate.cpp
        Source/FubbleModule.cpp
        Source/GlobalControls.cpp
        Source/GridController.cpp
        Source/GridModule.cpp
        Source/GridSliders.cpp
        Source/GroupControl.cpp
        Source/HelpDisplay.cpp
        Source/InputChannel.cpp
        Source/Inverter.cpp
        Source/KarplusStrong.cpp
        Source/KeyboardDisplay.cpp
        Source/Kicker.cpp
        Source/LaunchpadKeyboard.cpp
        Source/LaunchpadNoteDisplayer.cpp
        Source/LFOController.cpp
        Source/LinnstrumentControl.cpp
        Source/Lissajous.cpp
        Source/Looper.cpp
        Source/LooperGranulator.cpp
        Source/LooperRecorder.cpp
        Source/LoopStorer.cpp
        Source/MacroSlider.cpp
        Source/Metronome.cpp
        Source/MidiCapturer.cpp
        Source/MidiControlChange.cpp
        Source/MidiController.cpp
        Source/MidiOutput.cpp
        Source/ModulationVisualizer.cpp
        Source/ModulatorAccum.cpp
        Source/ModulatorAdd.cpp
        Source/ModulatorAddCentered.cpp
        Source/ModulatorCurve.cpp
        Source/ModulatorExpression.cpp
        Source/ModulatorGravity.cpp
        Source/ModulatorMult.cpp
        Source/ModulatorSmoother.cpp
        Source/ModulatorSubtract.cpp
        Source/ModuleSaveDataPanel.cpp
        Source/ModWheel.cpp
        Source/ModWheelToCV.cpp
        Source/ModwheelToPressure.cpp
        Source/ModwheelToVibrato.cpp
        Source/Monophonify.cpp
        Source/MPESmoother.cpp
        Source/MPETweaker.cpp
        Source/MultibandCompressor.cpp
        Source/MultitapDelay.cpp
        Source/MultitrackRecorder.cpp
        Source/Neighborhooder.cpp
        Source/NoteCanvas.cpp
        Source/NoteChainNode.cpp
        Source/NoteChance.cpp
        Source/NoteCounter.cpp
        Source/NoteCreator.cpp
        Source/NoteDelayer.cpp
        Source/NoteDisplayer.cpp
        Source/NoteFilter.cpp
        Source/NoteFlusher.cpp
        Source/NoteGate.cpp
        Source/NoteHocket.cpp
        Source/NoteHumanizer.cpp
        Source/NoteLatch.cpp
        Source/NoteLooper.cpp
        Source/NoteOctaver.cpp
        Source/NotePanAlternator.cpp
        Source/NotePanner.cpp
        Source/NotePanRandom.cpp
        Source/NoteQuantizer.cpp
        Source/NoteRangeFilter.cpp
        Source/NoteRatchet.cpp
        Source/NoteRouter.cpp
        Source/NoteSinger.cpp
        Source/NoteSorter.cpp
        Source/NoteStepper.cpp
        Source/NoteStepSequencer.cpp
        Source/NoteStreamDisplay.cpp
        Source/NoteStrummer.cpp
        Source/NoteSustain.cpp
        Source/NoteToFreq.cpp
        Source/NoteToMs.cpp
        Source/NoteToPulse.cpp
        Source/NoteTransformer.cpp
        Source/NoteVibrato.cpp
        Source/OSCOutput.cpp
        Source/OutputChannel.cpp
        Source/PanicButton.cpp
        Source/Panner.cpp
        Source/PitchBender.cpp
        Source/PitchChorus.cpp
        Source/PitchDive.cpp
        Source/PitchPanner.cpp
        Source/PitchRemap.cpp
        Source/PitchSetter.cpp
        Source/PitchToCV.cpp
        Source/PitchToSpeed.cpp
        Source/PlaySequencer.cpp
        Source/Polyrhythms.cpp
        Source/Prefab.cpp
        Source/Presets.cpp
        Source/Pressure.cpp
        Source/PressureToCV.cpp
        Source/PressureToModwheel.cpp
        Source/PressureToVibrato.cpp
        Source/PreviousNote.cpp
        Source/Producer.cpp
        Source/PulseButton.cpp
        Source/PulseChance.cpp
        Source/PulseDelayer.cpp
        Source/PulseGate.cpp
        Source/PulseHocket.cpp
        Source/Pulser.cpp
        Source/PulseSequence.cpp
        Source/PulseTrain.cpp
        Source/Push2Control.cpp
        Source/QuickSpawnMenu.cpp
        Source/RadioSequencer.cpp
        Source/Ramper.cpp
        Source/RandomNoteGenerator.cpp
        Source/Razor.cpp
        Source/Rewriter.cpp
        Source/RingModulator.cpp
        Source/SampleBrowser.cpp
        Source/SampleCanvas.cpp
        Source/SampleCapturer.cpp
        Source/SampleFinder.cpp
        Source/SampleLayerer.cpp
        Source/SamplePlayer.cpp
        Source/Sampler.cpp
        Source/SamplerGrid.cpp
        Source/Scale.cpp
        Source/ScaleDegree.cpp
        Source/ScaleDetect.cpp
        Source/ScriptModule.cpp
        Source/ScriptModule_PythonInterface.i
        Source/ScriptStatus.cpp
        Source/SeaOfGrain.cpp
        Source/Selector.cpp
        Source/SignalClamp.cpp
        Source/SignalGenerator.cpp
        Source/SingleOscillator.cpp
        Source/SliderSequencer.cpp
        Source/SlowLayers.cpp
        Source/SpectralDisplay.cpp
        Source/Splitter.cpp
        Source/StepSequencer.cpp
        Source/StutterControl.cpp
        Source/SustainPedal.cpp
        Source/TakeRecorder.cpp
        Source/TimelineControl.cpp
        Source/TimerDisplay.cpp
        Source/TitleBar.cpp
        Source/Transport.cpp
        Source/TransposeFrom.cpp
        Source/UnstableModWheel.cpp
        Source/UnstablePitch.cpp
        Source/UnstablePressure.cpp
        Source/UserPrefsEditor.cpp
        Source/ValueSetter.cpp
        Source/ValueStream.cpp
        Source/VelocityScaler.cpp
        Source/VelocitySetter.cpp
        Source/VelocityStepSequencer.cpp
        Source/VelocityToCV.cpp
        Source/VinylTempoControl.cpp
        Source/Vocoder.cpp
        Source/VocoderCarrierInput.cpp
        Source/VolcaBeatsControl.cpp
        Source/VSTPlugin.cpp
        Source/WaveformViewer.cpp
        Source/Waveshaper.cpp
        Source/WhiteKeys.cpp
        Source/CodeEntry.cpp
        Source/ClickButton.cpp
        Source/RadioButton.cpp
        Source/DropdownList.cpp
        Source/Slider.cpp
        Source/Checkbox.cpp
        Source/TextEntry.cpp
        Source/IUIControl.cpp
        Source/ADSR.cpp
        Source/ADSRDisplay.cpp
        Source/ArrangementController.cpp
        Source/Bespoke_Platform.cpp
        Source/BiquadFilter.cpp
        Source/Canvas.cpp
        Source/CanvasControls.cpp
        Source/CanvasElement.cpp
        Source/CanvasScrollbar.cpp
        Source/CanvasTimeline.cpp
        Source/ChannelBuffer.cpp
        Source/Chord.cpp
        Source/ChordDatabase.cpp
        Source/Curve.cpp
        Source/EffectFactory.cpp
        Source/EnvelopeEditor.cpp
        Source/EnvOscillator.cpp
        Source/FFT.cpp
        Source/FileStream.cpp
        Source/FilterButterworth24db.cpp
        Source/FilterViz.cpp
        Source/FloatSliderLFOControl.cpp
        Source/FMVoice.cpp
        Source/Granulator.cpp
        Source/IAudioProcessor.cpp
        Source/IAudioReceiver.cpp
        Source/IAudioSource.cpp
        Source/IClickable.cpp
        Source/IDrawableModule.cpp
        Source/IModulator.cpp
        Source/INoteReceiver.cpp
        Source/INoteSource.cpp
        Source/IPulseReceiver.cpp
        Source/JumpBlender.cpp
        Source/KarplusStrongVoice.cpp
        Source/LaunchpadInterpreter.cpp
        Source/LFO.cpp
        Source/LinkwitzRileyFilter.cpp
        Source/LocationZoomer.cpp
        Source/M185Sequencer.cpp
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/MathUtils.cpp
        Source/MidiDevice.cpp
        Source/MidiReader.cpp
        Source/ModularSynth.cpp
        Source/ModulationChain.cpp
        Source/ModuleContainer.cpp
        Source/ModuleFactory.cpp
        Source/ModuleSaveData.cpp
        Source/Monome.cpp
        Source/MultiBandTracker.cpp
        Source/NamedMutex.cpp
        Source/ofxJSONElement.cpp
        Source/OpenFrameworksPort.cpp
        Source/OscController.cpp
        Source/Oscillator.cpp
        Source/PatchCable.cpp
        Source/PatchCableSource.cpp
        Source/PeakTracker.cpp
        Source/PerformanceTimer.cpp
        Source/PitchDetector.cpp
        Source/PitchShifter.cpp
        Source/PolyphonyMgr.cpp
        Source/Profiler.cpp
        Source/Ramp.cpp
        Source/RollingBuffer.cpp
        Source/Sample.cpp
        Source/SampleDrawer.cpp
        Source/SampleVoice.cpp
        Source/SingleOscillatorVoice.cpp
        Source/SpaceMouseControl.cpp
        Source/SynthGlobals.cpp
        Source/TriggerDetector.cpp
        Source/UIGrid.cpp
        Source/UserData.cpp
        Source/VSTPlayhead.cpp
        Source/VSTWindow.cpp
        )

juce_add_gui_app(BespokeSynth
        PRODUCT_NAME BespokeSynth
        ICON_BIG ${CMAKE_SOURCE_DIR}/bespoke_icon.png
        MICROPHONE_PERMISSION_ENABLED TRUE
        CAMERA_PERMISSION_ENABLED       TRUE
        BLUETOOTH_PERMISSION_ENABLED    TRUE
        FILE_SHARING_ENABLED            TRUE
        DOCUMENT_BROWSER_ENABLED        TRUE
        )

juce_generate_juce_header(BespokeSynth)
target_sources(BespokeSynth PRIVATE ${BESPOKE_SOURCE_LIST})
target_include_directories(BespokeSynth PRIVATE
        Source/glew/include
        Source/push2
        Source/push2/libusb
        Source/push2/modules/libusb)

target_compile_definitions(BespokeSynth PUBLIC
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1

        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CAMERA=disabled

        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0


        JUCE_COREGRAPHICS_DRAW_ASYNC=1

        JUCE_ALSA=1
        JUCE_JACK=1

        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1

        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_VST=${BESPOKE_VST2}

        JUCE_CATCH_UNHANDLED_EXCEPTIONS=$<CONFIG:Debug>
        )

if (APPLE)
    target_compile_definitions(BespokeSynth PRIVATE
            BESPOKE_MAC=1
            JUCE_PLUGINHOST_AU=0 # this needs work but if you turn it to 1 with the link below it works
            )
    target_sources(BespokeSynth PRIVATE
            Source/CFMessaging/Bespoke_CFMessaging.cpp
            Source/psmove/Bespoke_PSMove.c
            )
    target_link_libraries(BespokeSynth PRIVATE
            "-framework CoreAudioKit")

    # Post install tasks - copy resources to the bundle
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E  copy_directory resource ${OUTDIR}/BespokeSynth.app/Contents/Resources/resource)

elseif (WIN32)
    target_compile_definitions(BespokeSynth PRIVATE
            BESPOKE_WINDOWS=1
            GLEW_STATIC=0
            GL_GLEXT_PROTOTYPES=0
            POLL_WINDOWS=0
            THREADS_WINDOWS=0
            HAVE_STRUCT_TIMESPEC
            )

    if (BESPOKE_ASIO_SDK_LOCATION)
        message( STATUS "Bespoke: Including ASIO from ${BESPOKE_ASIO_SDK_LOCATION}")
        target_compile_definitions(BespokeSynth PUBLIC JUCE_ASIO=1)
        target_include_directories(BespokeSynth PRIVATE ${BESPOKE_ASIO_SDK_LOCATION}/common)
    endif()

    if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
        message(STATUS "Bespoke: Linking With ${CMAKE_SOURCE_DIR}/Source/3DxWare/Lib/x86/siapp.lib")
        target_link_libraries(BespokeSynth PRIVATE
                ${CMAKE_SOURCE_DIR}/Source/3dxWare/Lib/x86/siapp.lib)
    else ()
        message(STATUS "Bespoke: Linking With ${CMAKE_SOURCE_DIR}/Source/3DxWare/Lib/x64/siapp.lib")
        target_link_libraries(BespokeSynth PRIVATE
                ${CMAKE_SOURCE_DIR}/Source/3DxWare/Lib/x64/siapp.lib)
    endif ()

    # Post install tasks - copy resources to the exe output directory
    set_target_properties(BespokeSynth PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BespokeSynth>"
                                    VS_DEBUGGER_COMMAND           "$<TARGET_FILE:BespokeSynth>"
                                    VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E  copy_directory resource ${OUTDIR}/resource)
else ()
    target_compile_definitions(BespokeSynth PRIVATE
            BESPOKE_LINUX=1
            GLEW_STATIC=0
            GL_GLEXT_PROTOTYPES=0
            )

    find_library(LIBUSB_1_LIBRARY 
       NAMES
         usb-1.0 usb
       PATHS
         /usr/lib
         /usr/local/lib
         /opt/local/lib
         /sw/lib
    )
    if (NOT LIBUSB_1_LIBRARY)
       message(ERROR "Bespoke: Cannot resolve libusb. Did you install libusb-1.0.0-dev package?")
    endif()
    message(STATUS "Bespoke: Using USB from ${LIBUSB_1_LIBRARY}")

    target_link_libraries(BespokeSynth PRIVATE ${LIBUSB_1_LIBRARY})

    # Ant put resources with the exe
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E  copy_directory resource ${OUTDIR}/resource)

endif ()

target_link_libraries(BespokeSynth PRIVATE
        bespoke::freeverb
        bespoke::json
        bespoke::nanovg
        bespoke::xwax
        pybind11::embed
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_opengl
        juce::juce_osc
        juce::juce_gui_basics)
