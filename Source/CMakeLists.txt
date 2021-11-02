include(cmake/lib.cmake)
include(cmake/versiontools.cmake)

juce_add_gui_app(BespokeSynth
    PRODUCT_NAME BespokeSynth
    ICON_BIG ${CMAKE_SOURCE_DIR}/bespoke_icon.png
    MICROPHONE_PERMISSION_ENABLED TRUE
    CAMERA_PERMISSION_ENABLED     TRUE
    BLUETOOTH_PERMISSION_ENABLED  TRUE
    FILE_SHARING_ENABLED          TRUE
    DOCUMENT_BROWSER_ENABLED      TRUE
    DOCUMENT_EXTENSIONS           .bsk
    )

bespoke_buildtime_version_info(BespokeSynth)

target_sources(BespokeSynth PRIVATE
    ADSR.cpp
    ADSRDisplay.cpp
    Amplifier.cpp
    Arpeggiator.cpp
    ArrangementController.cpp
    AudioLevelToCV.cpp
    AudioMeter.cpp
    AudioRouter.cpp
    AudioSend.cpp
    AudioToCV.cpp
    AudioToPulse.cpp
    Autotalent.cpp
    BandVocoder.cpp
    BeatBloks.cpp
    Beats.cpp
    BiquadFilter.cpp
    BiquadFilterEffect.cpp
    BitcrushEffect.cpp
    ButterworthFilterEffect.cpp
    Canvas.cpp
    CanvasControls.cpp
    CanvasElement.cpp
    CanvasScrollbar.cpp
    CanvasTimeline.cpp
    Capo.cpp
    ChannelBuffer.cpp
    ChaosEngine.cpp
    Checkbox.cpp
    Chord.cpp
    ChordDatabase.cpp
    ChordDisplayer.cpp
    ChordHolder.cpp
    Chorder.cpp
    CircleSequencer.cpp
    ClickButton.cpp
    ClipArranger.cpp
    ClipLauncher.cpp
    CodeEntry.cpp
    ComboGridController.cpp
    CommentDisplay.cpp
    Compressor.cpp
    ControlSequencer.cpp
    ControlTactileFeedback.cpp
    ControllingSong.cpp
    Curve.cpp
    CurveLooper.cpp
    DCOffset.cpp
    DCRemoverEffect.cpp
    DebugAudioSource.cpp
    DelayEffect.cpp
    DistortionEffect.cpp
    DropdownList.cpp
    DrumPlayer.cpp
    DrumSynth.cpp
    EQEffect.cpp
    EQModule.cpp
    EffectChain.cpp
    EffectFactory.cpp
    EnvOscillator.cpp
    EnvelopeEditor.cpp
    EnvelopeModulator.cpp
    EventCanvas.cpp
    FFT.cpp
    FFTtoAdditive.cpp
    FMSynth.cpp
    FMVoice.cpp
    FeedbackModule.cpp
    FileStream.cpp
    FilterButterworth24db.cpp
    FilterViz.cpp
    FloatSliderLFOControl.cpp
    FollowingSong.cpp
    FormantFilterEffect.cpp
    FourOnTheFloor.cpp
    FreeverbEffect.cpp
    FreqDelay.cpp
    FreqDomainBoilerplate.cpp
    FubbleModule.cpp
    GainStageEffect.cpp
    GateEffect.cpp
    GlobalControls.cpp
    Granulator.cpp
    GridController.cpp
    GridModule.cpp
    GridSliders.cpp
    GroupControl.cpp
    HelpDisplay.cpp
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
    InputChannel.cpp
    Inverter.cpp
    JumpBlender.cpp
    KarplusStrong.cpp
    KarplusStrongVoice.cpp
    KeyboardDisplay.cpp
    Kicker.cpp
    LFO.cpp
    LFOController.cpp
    LaunchpadInterpreter.cpp
    LaunchpadKeyboard.cpp
    LaunchpadNoteDisplayer.cpp
    LinkwitzRileyFilter.cpp
    LinnstrumentControl.cpp
    Lissajous.cpp
    LiveGranulator.cpp
    LocationZoomer.cpp
    LoopStorer.cpp
    Looper.cpp
    LooperGranulator.cpp
    LooperRecorder.cpp
    M185Sequencer.cpp
    MPESmoother.cpp
    MPETweaker.cpp
    MacroSlider.cpp
    Main.cpp
    MainComponent.cpp
    MathUtils.cpp
    Metronome.cpp
    MidiCapturer.cpp
    MidiControlChange.cpp
    MidiController.cpp
    MidiDevice.cpp
    MidiOutput.cpp
    MidiReader.cpp
    Minimap.cpp
    ModWheel.cpp
    ModWheelToCV.cpp
    ModularSynth.cpp
    ModulationChain.cpp
    ModulationVisualizer.cpp
    ModulatorAccum.cpp
    ModulatorAdd.cpp
    ModulatorAddCentered.cpp
    ModulatorCurve.cpp
    ModulatorExpression.cpp
    ModulatorGravity.cpp
    ModulatorMult.cpp
    ModulatorSmoother.cpp
    ModulatorSubtract.cpp
    ModuleContainer.cpp
    ModuleFactory.cpp
    ModuleSaveData.cpp
    ModuleSaveDataPanel.cpp
    ModwheelToPressure.cpp
    ModwheelToVibrato.cpp
    Monome.cpp
    Monophonify.cpp
    MultiBandTracker.cpp
    MultibandCompressor.cpp
    MultitapDelay.cpp
    MultitrackRecorder.cpp
    Muter.cpp
    NamedMutex.cpp
    Neighborhooder.cpp
    NoiseEffect.cpp
    NoteCanvas.cpp
    NoteChainNode.cpp
    NoteChance.cpp
    NoteCounter.cpp
    NoteCreator.cpp
    NoteDelayer.cpp
    NoteDisplayer.cpp
    NoteExpressionRouter.cpp
    NoteFilter.cpp
    NoteFlusher.cpp
    NoteGate.cpp
    NoteHocket.cpp
    NoteHumanizer.cpp
    NoteLatch.cpp
    NoteLooper.cpp
    NoteOctaver.cpp
    NotePanAlternator.cpp
    NotePanRandom.cpp
    NotePanner.cpp
    NoteQuantizer.cpp
    NoteRangeFilter.cpp
    NoteRatchet.cpp
    NoteRouter.cpp
    NoteSinger.cpp
    NoteSorter.cpp
    NoteStepSequencer.cpp
    NoteStepper.cpp
    NoteStreamDisplay.cpp
    NoteStrummer.cpp
    NoteSustain.cpp
    NoteTable.cpp
    NoteToFreq.cpp
    NoteToggle.cpp
    NoteToMs.cpp
    NoteToPulse.cpp
    NoteTransformer.cpp
    NoteVibrato.cpp
    OSCOutput.cpp
    OpenFrameworksPort.cpp
    OscController.cpp
    Oscillator.cpp
    OutputChannel.cpp
    PSMoveController.cpp
    PSMoveMgr.cpp
    PanicButton.cpp
    Panner.cpp
    PatchCable.cpp
    PatchCableSource.cpp
    PeakTracker.cpp
    PerformanceTimer.cpp
    PerlinNoise.cpp
    PitchBender.cpp
    PitchChorus.cpp
    PitchDetector.cpp
    PitchDive.cpp
    PitchPanner.cpp
    PitchRemap.cpp
    PitchSetter.cpp
    PitchShiftEffect.cpp
    PitchShifter.cpp
    PitchToCV.cpp
    PitchToSpeed.cpp
    PlaySequencer.cpp
    PolyphonyMgr.cpp
    Polyrhythms.cpp
    Prefab.cpp
    Presets.cpp
    Pressure.cpp
    PressureToCV.cpp
    PressureToModwheel.cpp
    PressureToVibrato.cpp
    PreviousNote.cpp
    Producer.cpp
    Profiler.cpp
    PulseButton.cpp
    PulseChance.cpp
    PulseDelayer.cpp
    PulseGate.cpp
    PulseHocket.cpp
    PulseSequence.cpp
    PulseTrain.cpp
    Pulser.cpp
    Pumper.cpp
    Push2Control.cpp
    QuickSpawnMenu.cpp
    RadioButton.cpp
    RadioSequencer.cpp
    Ramp.cpp
    Ramper.cpp
    RandomNoteGenerator.cpp
    Razor.cpp
    Rewriter.cpp
    RingModulator.cpp
    RollingBuffer.cpp
    Sample.cpp
    SampleBrowser.cpp
    SampleCanvas.cpp
    SampleCapturer.cpp
    SampleDrawer.cpp
    SampleFinder.cpp
    SampleLayerer.cpp
    SamplePlayer.cpp
    SampleVoice.cpp
    Sampler.cpp
    SamplerGrid.cpp
    Scale.cpp
    ScaleDegree.cpp
    ScaleDetect.cpp
    ScriptModule.cpp
    ScriptModule_PythonInterface.i
    ScriptStatus.cpp
    SeaOfGrain.cpp
    Selector.cpp
    SignalClamp.cpp
    SignalGenerator.cpp
    SingleOscillator.cpp
    SingleOscillatorVoice.cpp
    Slider.cpp
    SliderSequencer.cpp
    SlowLayers.cpp
    SpaceMouseControl.cpp
    SpectralDisplay.cpp
    Splitter.cpp
    StepSequencer.cpp
    Stutter.cpp
    StutterControl.cpp
    SustainPedal.cpp
    SynthGlobals.cpp
    TakeRecorder.cpp
    TextEntry.cpp
    TimelineControl.cpp
    TimerDisplay.cpp
    TitleBar.cpp
    Transport.cpp
    TransposeFrom.cpp
    TremoloEffect.cpp
    TriggerDetector.cpp
    UIGrid.cpp
    UnstableModWheel.cpp
    UnstablePitch.cpp
    UnstablePressure.cpp
    UserData.cpp
    UserPrefs.cpp
    UserPrefsEditor.cpp
    VSTPlayhead.cpp
    VSTPlugin.cpp
    VSTWindow.cpp
    VSTScanner.cpp
    ValueSetter.cpp
    ValueStream.cpp
    VelocityScaler.cpp
    VelocitySetter.cpp
    VelocityStepSequencer.cpp
    VelocityToCV.cpp
    VinylTempoControl.cpp
    Vocoder.cpp
    VocoderCarrierInput.cpp
    VolcaBeatsControl.cpp
    WaveformViewer.cpp
    Waveshaper.cpp
    WhiteKeys.cpp
    ofxJSONElement.cpp
    )

if(BESPOKE_PORTABLE)
    set_source_files_properties(ScriptModule.cpp PROPERTIES
        COMPILE_DEFINITIONS BESPOKE_PORTABLE_PYTHON="$<IF:$<BOOL:${WIN32}>,python.exe,bin/python>"
        )
endif()

target_include_directories(BespokeSynth PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python_INCLUDE_DIRS}
    )

target_compile_definitions(BespokeSynth PRIVATE
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
    JUCE_JACK_CLIENT_NAME="BespokeSynth"

    JUCE_WASAPI=1
    JUCE_DIRECTSOUND=1

    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=$<BOOL:${BESPOKE_VST2_SDK_LOCATION}>

    JUCE_CATCH_UNHANDLED_EXCEPTIONS=0

    BESPOKE_LINUX_HIGH_FPS_WITH_THREAD_PROBLEMS=$<BOOL:${BESPOKE_LINUX_HIGH_FPS_WITH_THREAD_PROBLEMS}>
    )

if (APPLE)
    target_compile_definitions(BespokeSynth PRIVATE
        BESPOKE_MAC=1
        JUCE_PLUGINHOST_AU=0 # this needs work but if you turn it to 1 with the link below it works
        )
    target_sources(BespokeSynth PRIVATE
        CFMessaging/KontrolKommunicator.cpp
        CFMessaging/ListenPort.cpp
        CFMessaging/NIMessage.cpp
        CFMessaging/SendPort.cpp
        KompleteKontrol.cpp
        )
    target_link_libraries(BespokeSynth PRIVATE "-framework CoreAudioKit")
elseif (WIN32)
    target_compile_definitions(BespokeSynth PRIVATE BESPOKE_WINDOWS=1)

    if (BESPOKE_ASIO_SDK_LOCATION)
        message( STATUS "Including ASIO from ${BESPOKE_ASIO_SDK_LOCATION}")
        target_compile_definitions(BespokeSynth PRIVATE JUCE_ASIO=1)
        target_include_directories(BespokeSynth PRIVATE ${BESPOKE_ASIO_SDK_LOCATION}/common)
    endif()

    set_target_properties(BespokeSynth PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BespokeSynth>"
        VS_DEBUGGER_COMMAND           "$<TARGET_FILE:BespokeSynth>"
        VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
else ()
    target_compile_definitions(BespokeSynth PRIVATE BESPOKE_LINUX=1)

    # Finally provide install rules if folks want to install into CMAKE_INSTALL_PREFIX
    install(TARGETS BespokeSynth DESTINATION bin)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/resource DESTINATION share/BespokeSynth)
    install(FILES ${CMAKE_SOURCE_DIR}/scripts/installer_linux/BespokeSynth.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/bespoke_icon.png DESTINATION share/icons/hicolor/512x512/apps)
endif ()

target_link_libraries(BespokeSynth PRIVATE
    bespoke::3dxware
    bespoke::exprtk
    bespoke::freeverb
    bespoke::json
    bespoke::leathers
    bespoke::nanovg
    bespoke::psmove
    bespoke::push2
    bespoke::pybind11
    bespoke::xwax

    tuning-library
    oddsound-mts

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_opengl
    juce::juce_osc

    ${Python_LIBRARIES}
    $<$<BOOL:${MINGW}>:dbghelp>
    )

bespoke_copy_resource_dir(BespokeSynth)
bespoke_make_portable(BespokeSynth)

# Rules to do some installing and packaging which we will have to refactor  but
# for now gets a nightly going
set(BESPOKE_NIGHTLY_DIR "${CMAKE_BINARY_DIR}/nightly")
add_custom_target(nightly-package)
add_dependencies(nightly-package BespokeSynth)

if (APPLE)
    add_custom_command(TARGET nightly-package
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BESPOKE_NIGHTLY_DIR}
            COMMAND hdiutil create /tmp/tmp.dmg -ov -volname "BespokeSynth" -fs HFS+ -srcfolder $<TARGET_BUNDLE_DIR:BespokeSynth>
            COMMAND ${CMAKE_COMMAND} -E rm -f "${BESPOKE_NIGHTLY_DIR}/BespokeNightly.dmg"
            COMMAND hdiutil convert /tmp/tmp.dmg -format UDZO -o "${BESPOKE_NIGHTLY_DIR}/BespokeNightly.dmg"
            )
else()
    add_custom_command(TARGET nightly-package
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BESPOKE_NIGHTLY_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "coming soon}" > ${BESPOKE_NIGHTLY_DIR}/installer.txt
            )
endif()
