include(cmake/lib.cmake)
include(cmake/versiontools.cmake)
if(BESPOKE_SYSTEM_ABLETONLINK)
    find_package(AbletonLink NAMES AbletonLink ableton-link link REQUIRED)
else()
    include(../libs/ableton-link/AbletonLinkConfig.cmake)
endif()

juce_add_gui_app(BespokeSynth
    PRODUCT_NAME BespokeSynth
    ICON_BIG ${CMAKE_SOURCE_DIR}/bespoke_icon.png
    MICROPHONE_PERMISSION_ENABLED TRUE
    CAMERA_PERMISSION_ENABLED     TRUE
    BLUETOOTH_PERMISSION_ENABLED  TRUE
    FILE_SHARING_ENABLED          TRUE
    DOCUMENT_BROWSER_ENABLED      TRUE
    DOCUMENT_EXTENSIONS           .bsk
    BUNDLE_ID                     com.ryanchallinor.bespokesynth
    PLUGINHOST_AU                 TRUE
    )

bespoke_buildtime_version_info(BespokeSynth)

if(MSVC)
    if(BESPOKE_WERROR)
        target_compile_options(BespokeSynth PRIVATE /WX)
    endif()
else()
    if (APPLE)
        target_compile_options(BespokeSynth PRIVATE -Wshorten-64-to-32)
    endif()
    if(BESPOKE_WERROR)
        target_compile_options(BespokeSynth PRIVATE -Werror)
    endif()
endif()

target_sources(BespokeSynth PRIVATE
    ADSR.cpp
    ADSR.h
    ADSRDisplay.cpp
    ADSRDisplay.h
    AbletonLink.cpp
    AbletonLink.h
    Acciaccatura.cpp
    Acciaccatura.h
    Amplifier.cpp
    Amplifier.h
    Arpeggiator.cpp
    Arpeggiator.h
    ArrangementController.cpp
    ArrangementController.h
    AudioLevelToCV.cpp
    AudioLevelToCV.h
    AudioMeter.cpp
    AudioMeter.h
    AudioRouter.cpp
    AudioRouter.h
    AudioSend.cpp
    AudioSend.h
    AudioSplitter.cpp
    AudioSplitter.h
    AudioToCV.cpp
    AudioToCV.h
    AudioToPulse.cpp
    AudioToPulse.h
    Autotalent.cpp
    Autotalent.h
    BassLineSequencer.cpp
    BassLineSequencer.h
    BandVocoder.cpp
    BandVocoder.h
    BeatBloks.cpp
    BeatBloks.h
    Beats.cpp
    Beats.h
    BiquadFilter.cpp
    BiquadFilter.h
    BiquadFilterEffect.cpp
    BiquadFilterEffect.h
    BitcrushEffect.cpp
    BitcrushEffect.h
    BufferShuffler.cpp
    BufferShuffler.h
    ButterworthFilterEffect.cpp
    ButterworthFilterEffect.h
    Canvas.cpp
    Canvas.h
    CanvasControls.cpp
    CanvasControls.h
    CanvasElement.cpp
    CanvasElement.h
    CanvasScrollbar.cpp
    CanvasScrollbar.h
    CanvasTimeline.cpp
    CanvasTimeline.h
    Capo.cpp
    Capo.h
    ChannelBuffer.cpp
    ChannelBuffer.h
    ChaosEngine.cpp
    ChaosEngine.h
    Checkbox.cpp
    Checkbox.h
    Chord.cpp
    Chord.h
    ChordDatabase.cpp
    ChordDatabase.h
    ChordDisplayer.cpp
    ChordDisplayer.h
    ChordHolder.cpp
    ChordHolder.h
    Chorder.cpp
    Chorder.h
    CircleSequencer.cpp
    CircleSequencer.h
    ClickButton.cpp
    ClickButton.h
    ClipArranger.cpp
    ClipArranger.h
    ClipLauncher.cpp
    ClipLauncher.h
    CodeEntry.cpp
    CodeEntry.h
    ComboGridController.cpp
    ComboGridController.h
    CommentDisplay.cpp
    CommentDisplay.h
    Compressor.cpp
    Compressor.h
    ControlRecorder.cpp
    ControlRecorder.h
    ControlSequencer.cpp
    ControlSequencer.h
    ControlTactileFeedback.cpp
    ControlTactileFeedback.h
    ControllingSong.cpp
    ControllingSong.h
    Curve.cpp
    Curve.h
    CurveLooper.cpp
    CurveLooper.h
    DataProvider.cpp
    DataProvider.h
    DCOffset.cpp
    DCOffset.h
    DCRemoverEffect.cpp
    DCRemoverEffect.h
    DebugAudioSource.cpp
    DebugAudioSource.h
    DelayEffect.cpp
    DelayEffect.h
    DistortionEffect.cpp
    DistortionEffect.h
    DotGrid.cpp
    DotGrid.h
    DotSequencer.cpp
    DotSequencer.h
    DropdownList.cpp
    DropdownList.h
    DrumPlayer.cpp
    DrumPlayer.h
    DrumSynth.cpp
    DrumSynth.h
    EQEffect.cpp
    EQEffect.h
    EQModule.cpp
    EQModule.h
    EffectChain.cpp
    EffectChain.h
    EffectFactory.cpp
    EffectFactory.h
    EnvOscillator.cpp
    EnvOscillator.h
    EnvelopeEditor.cpp
    EnvelopeEditor.h
    EnvelopeModulator.cpp
    EnvelopeModulator.h
    EuclideanSequencer.cpp
    EuclideanSequencer.h
    EventCanvas.cpp
    EventCanvas.h
    FFT.cpp
    FFT.h
    FFTtoAdditive.cpp
    FFTtoAdditive.h
    FMSynth.cpp
    FMSynth.h
    FMVoice.cpp
    FMVoice.h
    FeedbackModule.cpp
    FeedbackModule.h
    FileStream.cpp
    FileStream.h
    FillSaveDropdown.h
    FilterButterworth24db.cpp
    FilterButterworth24db.h
    FilterViz.cpp
    FilterViz.h
    FloatSliderLFOControl.cpp
    FloatSliderLFOControl.h
    FollowingSong.cpp
    FollowingSong.h
    FormantFilterEffect.cpp
    FormantFilterEffect.h
    FourOnTheFloor.cpp
    FourOnTheFloor.h
    FreeverbEffect.cpp
    FreeverbEffect.h
    FreqDelay.cpp
    FreqDelay.h
    FreqDomainBoilerplate.cpp
    FreqDomainBoilerplate.h
    FubbleModule.cpp
    FubbleModule.h
    GainStageEffect.cpp
    GainStageEffect.h
    GateEffect.cpp
    GateEffect.h
    GlobalControls.cpp
    GlobalControls.h
    Granulator.cpp
    Granulator.h
    GridController.cpp
    GridController.h
    GridModule.cpp
    GridModule.h
    GridSliders.cpp
    GridSliders.h
    GroupControl.cpp
    GroupControl.h
    HelpDisplay.cpp
    HelpDisplay.h
    IAudioEffect.h
    IAudioPoller.h
    IAudioProcessor.cpp
    IAudioProcessor.h
    IAudioReceiver.cpp
    IAudioReceiver.h
    IAudioSource.cpp
    IAudioSource.h
    IClickable.cpp
    IClickable.h
    IDrawableModule.cpp
    IDrawableModule.h
    IDrivableSequencer.h
    IMidiVoice.h
    IModulator.cpp
    IModulator.h
    INonstandardController.h
    INoteReceiver.cpp
    INoteReceiver.h
    INoteSource.cpp
    INoteSource.h
    IPatchable.h
    IPollable.h
    IPulseReceiver.cpp
    IPulseReceiver.h
    IUIControl.cpp
    IUIControl.h
    IVoiceParams.h
    InputChannel.cpp
    InputChannel.h
    Inverter.cpp
    Inverter.h
    JumpBlender.cpp
    JumpBlender.h
    KarplusStrong.cpp
    KarplusStrong.h
    KarplusStrongVoice.cpp
    KarplusStrongVoice.h
    KeyboardDisplay.cpp
    KeyboardDisplay.h
    Kicker.cpp
    Kicker.h
    LabelDisplay.cpp
    LabelDisplay.h
    LFO.cpp
    LFO.h
    LFOController.cpp
    LFOController.h
    LaunchpadInterpreter.cpp
    LaunchpadInterpreter.h
    LaunchpadKeyboard.cpp
    LaunchpadKeyboard.h
    LaunchpadNoteDisplayer.cpp
    LaunchpadNoteDisplayer.h
    LevelMeterDisplay.cpp
    LevelMeterDisplay.h
    LinkwitzRileyFilter.cpp
    LinkwitzRileyFilter.h
    LinnstrumentControl.cpp
    LinnstrumentControl.h
    Lissajous.cpp
    Lissajous.h
    LiveGranulator.cpp
    LiveGranulator.h
    LocationZoomer.cpp
    LocationZoomer.h
    LockFreeQueue.h
    LoopStorer.cpp
    LoopStorer.h
    Looper.cpp
    Looper.h
    LooperGranulator.cpp
    LooperGranulator.h
    LooperRecorder.cpp
    LooperRecorder.h
    M185Sequencer.cpp
    M185Sequencer.h
    MPESmoother.cpp
    MPESmoother.h
    MPETweaker.cpp
    MPETweaker.h
    MacroSlider.cpp
    MacroSlider.h
    Main.cpp
    MainComponent.cpp
    MathUtils.cpp
    MathUtils.h
    Metronome.cpp
    Metronome.h
    MidiCapturer.cpp
    MidiCapturer.h
    MidiClockIn.cpp
    MidiClockIn.h
    MidiClockOut.cpp
    MidiClockOut.h
    MidiControlChange.cpp
    MidiControlChange.h
    MidiController.cpp
    MidiController.h
    MidiDevice.cpp
    MidiDevice.h
    MidiOutput.cpp
    MidiOutput.h
    MidiReader.cpp
    MidiReader.h
    Minimap.cpp
    Minimap.h
    ModWheel.cpp
    ModWheel.h
    ModWheelToCV.cpp
    ModWheelToCV.h
    ModularSynth.cpp
    ModularSynth.h
    ModulationChain.cpp
    ModulationChain.h
    ModulationVisualizer.cpp
    ModulationVisualizer.h
    ModulatorAccum.cpp
    ModulatorAccum.h
    ModulatorAdd.cpp
    ModulatorAdd.h
    ModulatorAddCentered.cpp
    ModulatorAddCentered.h
    BoundsToPulse.cpp
    BoundsToPulse.h
    ModulatorCurve.cpp
    ModulatorCurve.h
    ModulatorExpression.cpp
    ModulatorExpression.h
    ModulatorGravity.cpp
    ModulatorGravity.h
    ModulatorMult.cpp
    ModulatorMult.h
    ModulatorSmoother.cpp
    ModulatorSmoother.h
    ModulatorSubtract.cpp
    ModulatorSubtract.h
    ModulatorWander.cpp
    ModulatorWander.h
    ModuleContainer.cpp
    ModuleContainer.h
    ModuleFactory.cpp
    ModuleFactory.h
    ModuleSaveData.cpp
    ModuleSaveData.h
    ModuleSaveDataPanel.cpp
    ModuleSaveDataPanel.h
    ModwheelToPressure.cpp
    ModwheelToPressure.h
    ModwheelToVibrato.cpp
    ModwheelToVibrato.h
    Monome.cpp
    Monome.h
    Monophonify.cpp
    Monophonify.h
    MultiBandTracker.cpp
    MultiBandTracker.h
    MultibandCompressor.cpp
    MultibandCompressor.h
    MultitapDelay.cpp
    MultitapDelay.h
    MultitrackRecorder.cpp
    MultitrackRecorder.h
    Muter.cpp
    Muter.h
    NamedMutex.cpp
    NamedMutex.h
    Neighborhooder.cpp
    Neighborhooder.h
    NoiseEffect.cpp
    NoiseEffect.h
    NoteCanvas.cpp
    NoteCanvas.h
    NoteChainNode.cpp
    NoteChainNode.h
    NoteChance.cpp
    NoteChance.h
    NoteCounter.cpp
    NoteCounter.h
    NoteCreator.cpp
    NoteCreator.h
    NoteDelayer.cpp
    NoteDelayer.h
    NoteDisplayer.cpp
    NoteDisplayer.h
    NoteEcho.cpp
    NoteEcho.h
    NoteEffectBase.h
    NoteExpressionRouter.cpp
    NoteExpressionRouter.h
    NoteFilter.cpp
    NoteFilter.h
    NoteFlusher.cpp
    NoteFlusher.h
    NoteGate.cpp
    NoteGate.h
    NoteHocket.cpp
    NoteHocket.h
    NoteHumanizer.cpp
    NoteHumanizer.h
    NoteLatch.cpp
    NoteLatch.h
    NoteLooper.cpp
    NoteLooper.h
    NoteOctaver.cpp
    NoteOctaver.h
    NoteOutputQueue.cpp
    NoteOutputQueue.h
    NotePanAlternator.cpp
    NotePanAlternator.h
    NotePanRandom.cpp
    NotePanRandom.h
    NotePanner.cpp
    NotePanner.h
    NoteQuantizer.cpp
    NoteQuantizer.h
    NoteRangeFilter.cpp
    NoteRangeFilter.h
    NoteRatchet.cpp
    NoteRatchet.h
    NoteRouter.cpp
    NoteRouter.h
    NoteSinger.cpp
    NoteSinger.h
    NoteSorter.cpp
    NoteSorter.h
    NoteStepSequencer.cpp
    NoteStepSequencer.h
    NoteStepper.cpp
    NoteStepper.h
    NoteStreamDisplay.cpp
    NoteStreamDisplay.h
    NoteStrummer.cpp
    NoteStrummer.h
    NoteSustain.cpp
    NoteSustain.h
    NoteTable.cpp
    NoteTable.h
    NoteToFreq.cpp
    NoteToFreq.h
    NoteToMs.cpp
    NoteToMs.h
    NoteToPulse.cpp
    NoteToPulse.h
    NoteToggle.cpp
    NoteToggle.h
    NoteTransformer.cpp
    NoteTransformer.h
    NoteVibrato.cpp
    NoteVibrato.h
    OSCOutput.cpp
    OSCOutput.h
    OpenFrameworksPort.cpp
    OpenFrameworksPort.h
    OscController.cpp
    OscController.h
    Oscillator.cpp
    Oscillator.h
    OutputChannel.cpp
    OutputChannel.h
    PSMoveController.cpp
    PSMoveController.h
    PSMoveMgr.cpp
    PSMoveMgr.h
    PanicButton.cpp
    PanicButton.h
    Panner.cpp
    Panner.h
    PatchCable.cpp
    PatchCable.h
    PatchCableSource.cpp
    PatchCableSource.h
    PeakTracker.cpp
    PeakTracker.h
    PerformanceTimer.cpp
    PerformanceTimer.h
    PerlinNoise.cpp
    PerlinNoise.h
    PitchBender.cpp
    PitchBender.h
    PitchChorus.cpp
    PitchChorus.h
    PitchDetector.cpp
    PitchDetector.h
    PitchDive.cpp
    PitchDive.h
    PitchPanner.cpp
    PitchPanner.h
    PitchRemap.cpp
    PitchRemap.h
    PitchSetter.cpp
    PitchSetter.h
    PitchShiftEffect.cpp
    PitchShiftEffect.h
    PitchShifter.cpp
    PitchShifter.h
    PitchToCV.cpp
    PitchToCV.h
    PitchToSpeed.cpp
    PitchToSpeed.h
    PitchToValue.cpp
    PitchToValue.h
    PlaySequencer.cpp
    PlaySequencer.h
    PolyphonyMgr.cpp
    PolyphonyMgr.h
    Polyrhythms.cpp
    Polyrhythms.h
    Prefab.cpp
    Prefab.h
    Pressure.cpp
    Pressure.h
    PressureToCV.cpp
    PressureToCV.h
    PressureToModwheel.cpp
    PressureToModwheel.h
    PressureToVibrato.cpp
    PressureToVibrato.h
    PreviousNote.cpp
    PreviousNote.h
    Producer.cpp
    Producer.h
    Profiler.cpp
    Profiler.h
    PulseButton.cpp
    PulseButton.h
    PulseChance.cpp
    PulseChance.h
    PulseDelayer.cpp
    PulseDelayer.h
    PulseDisplayer.cpp
    PulseDisplayer.h
    PulseFlag.cpp
    PulseFlag.h
    PulseGate.cpp
    PulseGate.h
    PulseHocket.cpp
    PulseHocket.h
    PulseLimit.cpp
    PulseLimit.h
    PulseRouter.cpp
    PulseRouter.h
    PulseSequence.cpp
    PulseSequence.h
    PulseTrain.cpp
    PulseTrain.h
    Pulser.cpp
    Pulser.h
    Pumper.cpp
    Pumper.h
    Push2Control.cpp
    Push2Control.h
    QuickSpawnMenu.cpp
    QuickSpawnMenu.h
    QwertyController.cpp
    QwertyController.h
    QwertyToPitchMapping.cpp
    QwertyToPitchMapping.h
    RadioButton.cpp
    RadioButton.h
    RadioSequencer.cpp
    RadioSequencer.h
    Ramp.cpp
    Ramp.h
    Ramper.cpp
    Ramper.h
    RandomNoteGenerator.cpp
    RandomNoteGenerator.h
    Razor.cpp
    Razor.h
    Rewriter.cpp
    Rewriter.h
    RhythmSequencer.cpp
    RhythmSequencer.h
    RingModulator.cpp
    RingModulator.h
    RollingBuffer.cpp
    RollingBuffer.h
    Sample.cpp
    Sample.h
    SampleBrowser.cpp
    SampleBrowser.h
    SampleCanvas.cpp
    SampleCanvas.h
    SampleCapturer.cpp
    SampleCapturer.h
    SampleDrawer.cpp
    SampleDrawer.h
    SampleFinder.cpp
    SampleFinder.h
    SampleLayerer.cpp
    SampleLayerer.h
    SamplePlayer.cpp
    SamplePlayer.h
    SampleVoice.cpp
    SampleVoice.h
    Sampler.cpp
    Sampler.h
    SamplerGrid.cpp
    SamplerGrid.h
    SaveStateLoader.cpp
    SaveStateLoader.h
    Scale.cpp
    Scale.h
    ScaleDegree.cpp
    ScaleDegree.h
    ScaleDetect.cpp
    ScaleDetect.h
    ScriptModule.cpp
    ScriptModule.h
    ScriptModule_PythonInterface.i
    ScriptStatus.cpp
    ScriptStatus.h
    SeaOfGrain.cpp
    SeaOfGrain.h
    Selector.cpp
    Selector.h
    SignalClamp.cpp
    SignalClamp.h
    SignalGenerator.cpp
    SignalGenerator.h
    SingleOscillator.cpp
    SingleOscillator.h
    SingleOscillatorVoice.cpp
    SingleOscillatorVoice.h
    Slider.cpp
    Slider.h
    SliderSequencer.cpp
    SliderSequencer.h
    SlowLayers.cpp
    SlowLayers.h
    Snapshots.cpp
    Snapshots.h
    SongBuilder.cpp
    SongBuilder.h
    SpaceMouseControl.cpp
    SpaceMouseControl.h
    SpectralDisplay.cpp
    SpectralDisplay.h
    Splitter.cpp
    Splitter.h
    StepSequencer.cpp
    StepSequencer.h
    Stutter.cpp
    Stutter.h
    StutterControl.cpp
    StutterControl.h
    SustainPedal.cpp
    SustainPedal.h
    SwitchAndRamp.cpp
    SwitchAndRamp.h
    SynthGlobals.cpp
    SynthGlobals.h
    TakeRecorder.cpp
    TakeRecorder.h
    TextEntry.cpp
    TextEntry.h
    TimelineControl.cpp
    TimelineControl.h
    TimerDisplay.cpp
    TimerDisplay.h
    TitleBar.cpp
    TitleBar.h
    Transport.cpp
    Transport.h
    TransposeFrom.cpp
    TransposeFrom.h
    TremoloEffect.cpp
    TremoloEffect.h
    TriggerDetector.cpp
    TriggerDetector.h
    UIControlMacros.h
    UIGrid.cpp
    UIGrid.h
    UnstableModWheel.cpp
    UnstableModWheel.h
    UnstablePitch.cpp
    UnstablePitch.h
    UnstablePressure.cpp
    UnstablePressure.h
    UserData.cpp
    UserData.h
    UserPrefs.cpp
    UserPrefs.h
    UserPrefsEditor.cpp
    UserPrefsEditor.h
    VSTPlayhead.cpp
    VSTPlayhead.h
    VSTPlugin.cpp
    VSTPlugin.h
    VSTScanner.cpp
    VSTScanner.h
    VSTWindow.cpp
    VSTWindow.h
    ValueSetter.cpp
    ValueSetter.h
    ValueStream.cpp
    ValueStream.h
    VelocityCurve.cpp
    VelocityCurve.h
    VelocityScaler.cpp
    VelocityScaler.h
    VelocitySetter.cpp
    VelocitySetter.h
    VelocityStepSequencer.cpp
    VelocityStepSequencer.h
    VelocityToCV.cpp
    VelocityToCV.h
    VelocityToChance.cpp
    VelocityToChance.h
    VinylTempoControl.cpp
    VinylTempoControl.h
    Vocoder.cpp
    Vocoder.h
    VocoderCarrierInput.cpp
    VocoderCarrierInput.h
    VoiceSetter.cpp
    VoiceSetter.h
    VolcaBeatsControl.cpp
    VolcaBeatsControl.h
    WaveformViewer.cpp
    WaveformViewer.h
    Waveshaper.cpp
    Waveshaper.h
    WhiteKeys.cpp
    WhiteKeys.h
    WindowCloseListener.h
    ofxJSONElement.cpp
    ofxJSONElement.h
    )

if(BESPOKE_NIGHTLY)
    target_compile_definitions(BespokeSynth PRIVATE
        BESPOKE_NIGHTLY=1
        )
endif()

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
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_LADSPA=0

    JUCE_CATCH_UNHANDLED_EXCEPTIONS=0
    )

if (APPLE)
    target_compile_definitions(BespokeSynth PRIVATE
        BESPOKE_MAC=1
        )
    target_sources(BespokeSynth PRIVATE
        CFMessaging/KontrolKommunicator.cpp
        CFMessaging/ListenPort.cpp
        CFMessaging/NIMessage.cpp
        CFMessaging/SendPort.cpp
        KompleteKontrol.cpp
        KompleteKontrol.h
        )
    target_link_libraries(BespokeSynth PRIVATE "-framework CoreAudioKit")

    if (BESPOKE_SIGN_AS)
        message(STATUS "Enabling hardened runtime for signed build")
        set_target_properties(BespokeSynth PROPERTIES
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        )

        message(STATUS "Creating target BespokeSynthSigned")
        add_custom_target(BespokeSynthSigned)
        add_dependencies(BespokeSynthSigned BespokeSynth)

        get_target_property(SIGN_HERE BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
        add_custom_command(TARGET BespokeSynthSigned POST_BUILD VERBATIM
                WORKING_DIRECTORY ${SIGN_HERE}
                COMMAND codesign --force -s "${BESPOKE_SIGN_AS}" --deep BespokeSynth.app/Contents/Frameworks/Python.framework/
                COMMAND find BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/ -name *.so -exec codesign --force -s "${BESPOKE_SIGN_AS}" {} \;
                COMMAND find BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/ -name *.o -exec codesign --force -s "${BESPOKE_SIGN_AS}" {} \;
                COMMAND find BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/ -name python3* -perm +111 -exec codesign -o runtime --entitlements "${CMAKE_SOURCE_DIR}/scripts/installer_macOS/entitlements.plist" --force -s "${BESPOKE_SIGN_AS}"  {} \;
                COMMAND find BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/ -name Python -exec codesign -o runtime --entitlements "${CMAKE_SOURCE_DIR}/scripts/installer_macOS/entitlements.plist" --force -s "${BESPOKE_SIGN_AS}" {} \;
                COMMAND codesign --force -s "${BESPOKE_SIGN_AS}" -o runtime --entitlements "${CMAKE_SOURCE_DIR}/scripts/installer_macOS/entitlements.plist" --deep BespokeSynth.app
                )
    endif()
elseif (WIN32)
    target_compile_definitions(BespokeSynth PRIVATE BESPOKE_WINDOWS=1)

    IF (MINGW)
        target_link_libraries(BespokeSynth PRIVATE iphlpapi)
    endif()

    if (BESPOKE_ASIO_SDK_LOCATION)
        message( STATUS "Including ASIO from ${BESPOKE_ASIO_SDK_LOCATION}")
        target_compile_definitions(BespokeSynth PRIVATE JUCE_ASIO=1)
        target_include_directories(BespokeSynth PRIVATE ${BESPOKE_ASIO_SDK_LOCATION}/common)
    endif()

    if (BESPOKE_SPACEMOUSE_SDK_LOCATION)
        message( STATUS "Including SpaceMouse from ${BESPOKE_SPACEMOUSE_SDK_LOCATION}")
        target_compile_definitions(BespokeSynth PRIVATE BESPOKE_SPACEMOUSE_SUPPORT=1)
        target_include_directories(BespokeSynth PRIVATE ${BESPOKE_SPACEMOUSE_SDK_LOCATION}/include)
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            target_link_libraries(BespokeSynth PRIVATE "${BESPOKE_SPACEMOUSE_SDK_LOCATION}/windows/x86/siapp.lib")
        else()
            target_link_libraries(BespokeSynth PRIVATE "${BESPOKE_SPACEMOUSE_SDK_LOCATION}/windows/x64/siapp.lib")
        endif()
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
    install(FILES ${CMAKE_SOURCE_DIR}/scripts/installer_linux/com.bespokesynth.BespokeSynth.metainfo.xml DESTINATION share/metainfo)
    install(FILES ${CMAKE_SOURCE_DIR}/bespoke_icon.png DESTINATION share/icons/hicolor/512x512/apps)
endif ()

target_link_libraries(BespokeSynth PRIVATE
    exprtk::exprtk
    bespoke::freeverb
    $<IF:$<STREQUAL:${BESPOKE_SYSTEM_JSONCPP},OFF>,bespoke::json,jsoncpp>
    bespoke::leathers
    bespoke::nanovg
    bespoke::psmove
    bespoke::push2
    bespoke::pybind11
    bespoke::xwax

    $<$<STREQUAL:${BESPOKE_SYSTEM_TUNING_LIBRARY},OFF>:tuning-library>
    oddsound-mts
    Ableton::Link
    readerwriterqueue

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_cryptography
    juce::juce_gui_basics
    juce::juce_opengl
    juce::juce_osc
    # if JUCE is devendored, these libraries need to be linked against
    $<$<BOOL:${BESPOKE_DEVENDORED_SYSTEM_JUCE}>:FLAC>
    $<$<BOOL:${BESPOKE_DEVENDORED_SYSTEM_JUCE}>:ogg>
    $<$<BOOL:${BESPOKE_DEVENDORED_SYSTEM_JUCE}>:vorbis>
    $<$<BOOL:${BESPOKE_DEVENDORED_SYSTEM_JUCE}>:vorbisenc>
    $<$<BOOL:${BESPOKE_DEVENDORED_SYSTEM_JUCE}>:vorbisfile>

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
    if (BESPOKE_SIGN_AS)
        # If we aren't signing there's no target
        add_dependencies(nightly-package BespokeSynthSigned)
    endif()
    get_target_property(output_dir BespokeSynth RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET nightly-package
             POST_BUILD
             COMMAND ${CMAKE_SOURCE_DIR}/scripts/installer_macOS/make_installer.sh "${output_dir}" "${CMAKE_SOURCE_DIR}/scripts/installer_macOS" "${BESPOKE_NIGHTLY_DIR}" "${CMAKE_PROJECT_VERSION}"
             )
endif()

# CI Pipeline {{{
# this checks if PRs are following clang-format rules
# nabbed from https://github.com/surge-synthesizer/surge/blob/94fa4c7d4d4bbfdfe34820bd365f653b49314747/CMakeLists.txt#L172
# the code will run on linux in the pipeline.
add_custom_target(code-quality-pipeline-checks)

# Clang Format checks
set(CLANG_FORMAT_DIRS Source)
set(CLANG_FORMAT_EXTS cpp h)
foreach(dir ${CLANG_FORMAT_DIRS})
  foreach(ext ${CLANG_FORMAT_EXTS})
    list(APPEND CLANG_FORMAT_GLOBS "':(glob)${dir}/**/*.${ext}'")
  endforeach()
endforeach()
add_custom_command(TARGET code-quality-pipeline-checks
  POST_BUILD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -E echo About to check clang-format
  COMMAND clang-format --version
  COMMAND git ls-files -- ${CLANG_FORMAT_GLOBS} | xargs clang-format --dry-run --Werror
  )
# }}}
