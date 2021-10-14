include(cmake/versiontools.cmake)

juce_add_gui_app(BespokeSynth
    PRODUCT_NAME BespokeSynth
    ICON_BIG ${CMAKE_SOURCE_DIR}/bespoke_icon.png
    MICROPHONE_PERMISSION_ENABLED TRUE
    CAMERA_PERMISSION_ENABLED     TRUE
    BLUETOOTH_PERMISSION_ENABLED  TRUE
    FILE_SHARING_ENABLED          TRUE
    DOCUMENT_BROWSER_ENABLED      TRUE
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
    NoteToFreq.cpp
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
    UserPrefsEditor.cpp
    VSTPlayhead.cpp
    VSTPlugin.cpp
    VSTWindow.cpp
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

add_dependencies(BespokeSynth BespokeVSTScanner)

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

    # Post install tasks - copy resources to the bundle
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory resource ${OUTDIR}/BespokeSynth.app/Contents/Resources/resource
        )

    get_filename_component(Python_FRAMEWORK_DIR ${Python_LIBRARY_DIRS} DIRECTORY)
    foreach(PYCAND Python Python3.9 Python3.8 Python3)
        if (EXISTS "${Python_FRAMEWORK_DIR}/${PYCAND}")
            set( Python_BIN "${PYCAND}")
        endif()
    endforeach()
    if (NOT DEFINED Python_BIN)
        message( FATAL_ERROR "Cannot find python binary. Consider setting BESPOKE_PYTHON_ROOT" )
    endif()

    # Alright what is this doing? Well the python build hard-codes a reference to a python
    # framework which means this executable wont even *start* on a machine which isn't
    # configured the same as python. Our desired behavior is that the executable starts
    # but python doesn't work. So copy the python framework build stub into the resources
    # framework and repoint the python to it with install_name_tool.
    #
    # But also: If you have no python installed, or have BESPOKE_PYTHON_ROOT set oddly,
    # you end up with a wierdo case where you use XCode python. We warn about that here
    # but also do the couple of rpath changes just in case below, which deal with that
    message(STATUS "Mac Python Framework is '${Python_BIN}' from '${Python_FRAMEWORK_DIR}'")

    string(FIND ${Python_FRAMEWORK_DIR} "Xcode.app" IS_PYTHON_XCODE)
    if (${IS_PYTHON_XCODE} GREATER 0)
        message( WARNING "Your python built environment came from Xcode.app. "
            "This may not be what you want. Consider setting BESPOKE_PYTHON_ROOT to "
            "a python install of your choosing. Especially, if you are on an M1 "
            "mac and use homebrew, you may want to use something like "
            "'-DBESPOKE_PYTHON_ROOT=/opt/homebrew/Cellar/python\@3.9/3.9.6'")
    endif()

    get_target_property(SCANNEROUTDIR BespokeVSTScanner RUNTIME_OUTPUT_DIRECTORY)

    add_custom_command(TARGET BespokeSynth
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}/BespokeSynth.app/Contents/Frameworks/LocalPython
        COMMAND ${CMAKE_COMMAND} -E copy ${Python_FRAMEWORK_DIR}/${Python_BIN} ${OUTDIR}/BespokeSynth.app/Contents/Frameworks/LocalPython
        # This is pretty questionable
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SCANNEROUTDIR}/BespokeVSTScanner.app ${OUTDIR}/BespokeSynth.app/Contents/Resources/BespokeVSTScanner.app
        COMMAND codesign --remove-signature ${OUTDIR}/BespokeSynth.app/Contents/Frameworks/LocalPython/${Python_BIN}
        COMMAND codesign -s - ${OUTDIR}/BespokeSynth.app/Contents/Frameworks/LocalPython/${Python_BIN}
        # THis is the 'normal' case
        COMMAND install_name_tool -change "${Python_FRAMEWORK_DIR}/${Python_BIN}" "@executable_path/../Frameworks/LocalPython/${Python_BIN}" ${OUTDIR}/BespokeSynth.app/Contents/MacOS/BespokeSynth
        # These are the 'got it from xcode by mistake' cases
        COMMAND install_name_tool -change "@rpath/Python3.framework/Versions/3.8/Python3" "@executable_path/../Frameworks/LocalPython/${Python_BIN}" ${OUTDIR}/BespokeSynth.app/Contents/MacOS/BespokeSynth
        COMMAND install_name_tool -change "@rpath/Python3.framework/Versions/3.9/Python3" "@executable_path/../Frameworks/LocalPython/${Python_BIN}" ${OUTDIR}/BespokeSynth.app/Contents/MacOS/BespokeSynth
        )

elseif (WIN32)
    target_compile_definitions(BespokeSynth PRIVATE BESPOKE_WINDOWS=1)

    if (BESPOKE_ASIO_SDK_LOCATION)
        message( STATUS "Including ASIO from ${BESPOKE_ASIO_SDK_LOCATION}")
        target_compile_definitions(BespokeSynth PRIVATE JUCE_ASIO=1)
        target_include_directories(BespokeSynth PRIVATE ${BESPOKE_ASIO_SDK_LOCATION}/common)
    endif()

    # Post install tasks - copy resources to the exe output directory
    set_target_properties(BespokeSynth PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BespokeSynth>"
        VS_DEBUGGER_COMMAND           "$<TARGET_FILE:BespokeSynth>"
        VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    get_target_property(SCANNEROUTDIR BespokeVSTScanner RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory resource ${OUTDIR}/resource
        COMMAND ${CMAKE_COMMAND} -E copy ${SCANNEROUTDIR}/*.exe ${OUTDIR})
else ()
    target_compile_definitions(BespokeSynth PRIVATE BESPOKE_LINUX=1)

    # Ant put resources with the exe
    get_target_property(OUTDIR BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory resource ${OUTDIR}/resource)

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

# Rules to do some installing and packaging which we will have to refactor  but
# for now gets a nightly going
set(BESPOKE_PRODUCT_DIR "${CMAKE_BINARY_DIR}/products")
set(BESPOKE_NIGHTLY_DIR "${CMAKE_BINARY_DIR}/nightly")
add_custom_command(TARGET BespokeSynth
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BESPOKE_PRODUCT_DIR}
        )
add_custom_target(nightly-package)
add_dependencies(nightly-package BespokeSynth)

if (APPLE)
    get_target_property(output_dir BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET BespokeSynth
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${output_dir}/BespokeSynth.app/ ${BESPOKE_PRODUCT_DIR}/BespokeSynth.app
            )

    add_custom_command(TARGET nightly-package
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BESPOKE_NIGHTLY_DIR}
            COMMAND hdiutil create /tmp/tmp.dmg -ov -volname "BespokeSynth" -fs HFS+ -srcfolder "${BESPOKE_PRODUCT_DIR}"
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
