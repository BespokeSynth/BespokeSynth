include(cmake/lib.cmake)
include(cmake/versiontools.cmake)
include(../libs/link/AbletonLinkConfig.cmake)

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

target_sources(BespokeSynth PRIVATE
    AbletonLink.cpp
    AbletonLink.h
    ADSR.cpp
    ADSRDisplay.cpp
    ADSRDisplay.h
    ADSR.h
    ArrangementController.cpp
    ArrangementController.h
    AudioEffects/Amplifier.cpp
    AudioEffects/Amplifier.h
    AudioEffects/AudioMeter.cpp
    AudioEffects/AudioMeter.h
    AudioEffects/AudioRouter.cpp
    AudioEffects/AudioRouter.h
    AudioEffects/AudioSend.cpp
    AudioEffects/AudioSend.h
    AudioEffects/Autotalent.cpp
    AudioEffects/Autotalent.h
    AudioEffects/BandVocoder.cpp
    AudioEffects/BandVocoder.h
    AudioEffects/DCOffset.cpp
    AudioEffects/DCOffset.h
    AudioEffects/EffectChain.cpp
    AudioEffects/EffectChain.h
    AudioEffects/EQModule.cpp
    AudioEffects/EQModule.h
    AudioEffects/FeedbackModule.cpp
    AudioEffects/FeedbackModule.h
    AudioEffects/FFTtoAdditive.cpp
    AudioEffects/FFTtoAdditive.h
    AudioEffects/FreqDelay.cpp
    AudioEffects/FreqDelay.h
    AudioEffects/FreqDomainBoilerplate.cpp
    AudioEffects/FreqDomainBoilerplate.h
    AudioEffects/InputChannel.cpp
    AudioEffects/InputChannel.h
    AudioEffects/Inverter.cpp
    AudioEffects/Inverter.h
    AudioEffects/Lissajous.cpp
    AudioEffects/Lissajous.h
    AudioEffects/Looper.cpp
    AudioEffects/Looper.h
    AudioEffects/LooperRecorder.cpp
    AudioEffects/LooperRecorder.h
    AudioEffects/MultibandCompressor.cpp
    AudioEffects/MultibandCompressor.h
    AudioEffects/MultitapDelay.cpp
    AudioEffects/MultitapDelay.h
    AudioEffects/OutputChannel.cpp
    AudioEffects/OutputChannel.h
    AudioEffects/Panner.cpp
    AudioEffects/Panner.h
    AudioEffects/PitchChorus.cpp
    AudioEffects/PitchChorus.h
    AudioEffects/Producer.cpp
    AudioEffects/Producer.h
    AudioEffects/Rewriter.cpp
    AudioEffects/Rewriter.h
    AudioEffects/RingModulator.cpp
    AudioEffects/RingModulator.h
    AudioEffects/SampleCapturer.cpp
    AudioEffects/SampleCapturer.h
    AudioEffects/SampleFinder.cpp
    AudioEffects/SampleFinder.h
    AudioEffects/SamplerGrid.cpp
    AudioEffects/SamplerGrid.h
    AudioEffects/SignalClamp.cpp
    AudioEffects/SignalClamp.h
    AudioEffects/SlowLayers.cpp
    AudioEffects/SlowLayers.h
    AudioEffects/SpectralDisplay.cpp
    AudioEffects/SpectralDisplay.h
    AudioEffects/Splitter.cpp
    AudioEffects/Splitter.h
    AudioEffects/StutterControl.cpp
    AudioEffects/StutterControl.h
    AudioEffects/TakeRecorder.cpp
    AudioEffects/TakeRecorder.h
    AudioEffects/VocoderCarrierInput.cpp
    AudioEffects/VocoderCarrierInput.h
    AudioEffects/Vocoder.cpp
    AudioEffects/Vocoder.h
    AudioEffects/WaveformViewer.cpp
    AudioEffects/WaveformViewer.h
    AudioEffects/Waveshaper.cpp
    AudioEffects/Waveshaper.h
    BiquadFilter.cpp
    BiquadFilterEffect.cpp
    BiquadFilterEffect.h
    BiquadFilter.h
    BitcrushEffect.cpp
    BitcrushEffect.h
    ButterworthFilterEffect.cpp
    ButterworthFilterEffect.h
    CanvasControls.cpp
    CanvasControls.h
    Canvas.cpp
    CanvasElement.cpp
    CanvasElement.h
    Canvas.h
    CanvasScrollbar.cpp
    CanvasScrollbar.h
    CanvasTimeline.cpp
    CanvasTimeline.h
    ChannelBuffer.cpp
    ChannelBuffer.h
    ChaosEngine.cpp
    ChaosEngine.h
    Checkbox.cpp
    Checkbox.h
    Chord.cpp
    ChordDatabase.cpp
    ChordDatabase.h
    Chord.h
    ClickButton.cpp
    ClickButton.h
    ClipArranger.cpp
    ClipArranger.h
    CodeEntry.cpp
    CodeEntry.h
    ComboGridController.cpp
    ComboGridController.h
    CommentDisplay.cpp
    CommentDisplay.h
    Compressor.cpp
    Compressor.h
    Curve.cpp
    Curve.h
    DCRemoverEffect.cpp
    DCRemoverEffect.h
    DelayEffect.cpp
    DelayEffect.h
    DistortionEffect.cpp
    DistortionEffect.h
    DropdownList.cpp
    DropdownList.h
    EffectFactory.cpp
    EffectFactory.h
    EnvelopeEditor.cpp
    EnvelopeEditor.h
    EnvOscillator.cpp
    EnvOscillator.h
    EQEffect.cpp
    EQEffect.h
    EventCanvas.cpp
    EventCanvas.h
    FFT.cpp
    FFT.h
    FileStream.cpp
    FileStream.h
    FillSaveDropdown.h
    FilterButterworth24db.cpp
    FilterButterworth24db.h
    FilterViz.cpp
    FilterViz.h
    FMVoice.cpp
    FMVoice.h
    FormantFilterEffect.cpp
    FormantFilterEffect.h
    FreeverbEffect.cpp
    FreeverbEffect.h
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
    Instruments/CircleSequencer.cpp
    Instruments/CircleSequencer.h
    Instruments/FourOnTheFloor.cpp
    Instruments/FourOnTheFloor.h
    Instruments/KeyboardDisplay.cpp
    Instruments/KeyboardDisplay.h
    Instruments/LaunchpadKeyboard.cpp
    Instruments/LaunchpadKeyboard.h
    Instruments/M185Sequencer.cpp
    Instruments/M185Sequencer.h
    Instruments/MidiController.cpp
    Instruments/MidiController.h
    Instruments/NoteCanvas.cpp
    Instruments/NoteCanvas.h
    Instruments/NoteChainNode.cpp
    Instruments/NoteChainNode.h
    Instruments/NoteCounter.cpp
    Instruments/NoteCounter.h
    Instruments/NoteCreator.cpp
    Instruments/NoteCreator.h
    Instruments/NoteLooper.cpp
    Instruments/NoteLooper.h
    Instruments/NoteSinger.cpp
    Instruments/NoteSinger.h
    Instruments/NoteStepSequencer.cpp
    Instruments/NoteStepSequencer.h
    Instruments/PlaySequencer.cpp
    Instruments/PlaySequencer.h
    Instruments/Polyrhythms.cpp
    Instruments/Polyrhythms.h
    Instruments/RandomNoteGenerator.cpp
    Instruments/RandomNoteGenerator.h
    Instruments/StepSequencer.cpp
    Instruments/StepSequencer.h
    Instruments/SliderSequencer.cpp
    Instruments/SliderSequencer.h
    IPatchable.h
    IPollable.h
    IPulseReceiver.cpp
    IPulseReceiver.h
    IUIControl.cpp
    IUIControl.h
    IVoiceParams.h
    JumpBlender.cpp
    JumpBlender.h
    KarplusStrongVoice.cpp
    KarplusStrongVoice.h
    LaunchpadInterpreter.cpp
    LaunchpadInterpreter.h
    LFOController.cpp
    LFOController.h
    LFO.cpp
    LFO.h
    LinkwitzRileyFilter.cpp
    LinkwitzRileyFilter.h
    LiveGranulator.cpp
    LiveGranulator.h
    LocationZoomer.cpp
    LocationZoomer.h
    LockFreeQueue.h
    LooperGranulator.cpp
    LooperGranulator.h
    LoopStorer.cpp
    LoopStorer.h
    MainComponent.cpp
    Main.cpp
    MathUtils.cpp
    MathUtils.h
    MidiClockIn.cpp
    MidiClockIn.h
    MidiClockOut.cpp
    MidiClockOut.h
    MidiDevice.cpp
    MidiDevice.h
    MidiOutput.cpp
    MidiOutput.h
    MidiReader.cpp
    MidiReader.h
    Minimap.cpp
    Minimap.h
    ModularSynth.cpp
    ModularSynth.h
    ModulationChain.cpp
    ModulationChain.h
    Modulators/AudioLevelToCV.cpp
    Modulators/AudioLevelToCV.h
    Modulators/AudioToCV.cpp
    Modulators/AudioToCV.h
    Modulators/ControlSequencer.cpp
    Modulators/ControlSequencer.h
    Modulators/CurveLooper.cpp
    Modulators/CurveLooper.h
    Modulators/EnvelopeModulator.cpp
    Modulators/EnvelopeModulator.h
    Modulators/FloatSliderLFOControl.cpp
    Modulators/FloatSliderLFOControl.h
    Modulators/FubbleModule.cpp
    Modulators/FubbleModule.h
    Modulators/GridSliders.cpp
    Modulators/GridSliders.h
    Modulators/MacroSlider.cpp
    Modulators/MacroSlider.h
    Modulators/ModulatorAccum.cpp
    Modulators/ModulatorAccum.h
    Modulators/ModulatorAddCentered.cpp
    Modulators/ModulatorAddCentered.h
    Modulators/ModulatorAdd.cpp
    Modulators/ModulatorAdd.h
    Modulators/ModulatorCurve.cpp
    Modulators/ModulatorCurve.h
    Modulators/ModulatorExpression.cpp
    Modulators/ModulatorExpression.h
    Modulators/ModulatorGravity.cpp
    Modulators/ModulatorGravity.h
    Modulators/ModulatorMult.cpp
    Modulators/ModulatorMult.h
    Modulators/ModulatorSmoother.cpp
    Modulators/ModulatorSmoother.h
    Modulators/ModulatorSubtract.cpp
    Modulators/ModulatorSubtract.h
    Modulators/ModWheelToCV.cpp
    Modulators/ModWheelToCV.h
    Modulators/NoteToFreq.cpp
    Modulators/NoteToFreq.h
    Modulators/NoteToMs.cpp
    Modulators/NoteToMs.h
    Modulators/PitchToCV.cpp
    Modulators/PitchToCV.h
    Modulators/PitchToSpeed.cpp
    Modulators/PitchToSpeed.h
    Modulators/PressureToCV.cpp
    Modulators/PressureToCV.h
    Modulators/Ramper.cpp
    Modulators/Ramper.h
    Modulators/ValueSetter.cpp
    Modulators/ValueSetter.h
    Modulators/VelocityToCV.cpp
    Modulators/VelocityToCV.h
    Modulators/VinylTempoControl.cpp
    Modulators/VinylTempoControl.h
    ModuleContainer.cpp
    ModuleContainer.h
    ModuleFactory.cpp
    ModuleFactory.h
    ModuleSaveData.cpp
    ModuleSaveData.h
    ModuleSaveDataPanel.cpp
    ModuleSaveDataPanel.h
    Monome.cpp
    Monome.h
    MultiBandTracker.cpp
    MultiBandTracker.h
    MultitrackRecorder.cpp
    MultitrackRecorder.h
    Muter.cpp
    Muter.h
    NamedMutex.cpp
    NamedMutex.h
    NoiseEffect.cpp
    NoiseEffect.h
    NoteEffectBase.h
    NoteEffects/Arpeggiator.cpp
    NoteEffects/Arpeggiator.h
    NoteEffects/Capo.cpp
    NoteEffects/Capo.h
    NoteEffects/ChordDisplayer.cpp
    NoteEffects/ChordDisplayer.h
    NoteEffects/Chorder.cpp
    NoteEffects/Chorder.h
    NoteEffects/ChordHolder.cpp
    NoteEffects/ChordHolder.h
    NoteEffects/Kicker.cpp
    NoteEffects/Kicker.h
    NoteEffects/LaunchpadNoteDisplayer.cpp
    NoteEffects/LaunchpadNoteDisplayer.h
    NoteEffects/LinnstrumentControl.cpp
    NoteEffects/LinnstrumentControl.h
    NoteEffects/MidiCapturer.cpp
    NoteEffects/MidiCapturer.h
    NoteEffects/MidiControlChange.cpp
    NoteEffects/MidiControlChange.h
    NoteEffects/ModulationVisualizer.cpp
    NoteEffects/ModulationVisualizer.h
    NoteEffects/ModWheel.cpp
    NoteEffects/ModWheel.h
    NoteEffects/ModwheelToPressure.cpp
    NoteEffects/ModwheelToPressure.h
    NoteEffects/ModwheelToVibrato.cpp
    NoteEffects/ModwheelToVibrato.h
    NoteEffects/Monophonify.cpp
    NoteEffects/Monophonify.h
    NoteEffects/MPESmoother.cpp
    NoteEffects/MPESmoother.h
    NoteEffects/MPETweaker.cpp
    NoteEffects/MPETweaker.h
    NoteEffects/Neighborhooder.cpp
    NoteEffects/Neighborhooder.h
    NoteEffects/NoteChance.cpp
    NoteEffects/NoteChance.h
    NoteEffects/NoteDelayer.cpp
    NoteEffects/NoteDelayer.h
    NoteEffects/NoteDisplayer.cpp
    NoteEffects/NoteDisplayer.h
    NoteEffects/NoteEcho.cpp
    NoteEffects/NoteEcho.h
    NoteEffects/NoteExpressionRouter.cpp
    NoteEffects/NoteExpressionRouter.h
    NoteEffects/NoteFilter.cpp
    NoteEffects/NoteFilter.h
    NoteEffects/NoteFlusher.cpp
    NoteEffects/NoteFlusher.h
    NoteEffects/NoteGate.cpp
    NoteEffects/NoteGate.h
    NoteEffects/NoteHocket.cpp
    NoteEffects/NoteHocket.h
    NoteEffects/NoteHumanizer.cpp
    NoteEffects/NoteHumanizer.h
    NoteEffects/NoteLatch.cpp
    NoteEffects/NoteLatch.h
    NoteEffects/NoteOctaver.cpp
    NoteEffects/NoteOctaver.h
    NoteEffects/NotePanAlternator.cpp
    NoteEffects/NotePanAlternator.h
    NoteEffects/NotePanner.cpp
    NoteEffects/NotePanner.h
    NoteEffects/NotePanRandom.cpp
    NoteEffects/NotePanRandom.h
    NoteEffects/NoteQuantizer.cpp
    NoteEffects/NoteQuantizer.h
    NoteEffects/NoteRangeFilter.cpp
    NoteEffects/NoteRangeFilter.h
    NoteEffects/NoteRatchet.cpp
    NoteEffects/NoteRatchet.h
    NoteEffects/NoteRouter.cpp
    NoteEffects/NoteRouter.h
    NoteEffects/NoteSorter.cpp
    NoteEffects/NoteSorter.h
    NoteEffects/NoteStepper.cpp
    NoteEffects/NoteStepper.h
    NoteEffects/NoteStreamDisplay.cpp
    NoteEffects/NoteStreamDisplay.h
    NoteEffects/NoteStrummer.cpp
    NoteEffects/NoteStrummer.h
    NoteEffects/NoteSustain.cpp
    NoteEffects/NoteSustain.h
    NoteEffects/NoteTable.cpp
    NoteEffects/NoteTable.h
    NoteEffects/NoteTransformer.cpp
    NoteEffects/NoteTransformer.h
    NoteEffects/NoteVibrato.cpp
    NoteEffects/NoteVibrato.h
    NoteEffects/PitchBender.cpp
    NoteEffects/PitchBender.h
    NoteEffects/PitchDive.cpp
    NoteEffects/PitchDive.h
    NoteEffects/PitchPanner.cpp
    NoteEffects/PitchPanner.h
    NoteEffects/PitchRemap.cpp
    NoteEffects/PitchRemap.h
    NoteEffects/PitchSetter.cpp
    NoteEffects/PitchSetter.h
    NoteEffects/Pressure.cpp
    NoteEffects/Pressure.h
    NoteEffects/PressureToModwheel.cpp
    NoteEffects/PressureToModwheel.h
    NoteEffects/PressureToVibrato.cpp
    NoteEffects/PressureToVibrato.h
    NoteEffects/PreviousNote.cpp
    NoteEffects/PreviousNote.h
    NoteEffects/ScaleDegree.cpp
    NoteEffects/ScaleDegree.h
    NoteEffects/ScaleDetect.cpp
    NoteEffects/ScaleDetect.h
    NoteEffects/SustainPedal.cpp
    NoteEffects/SustainPedal.h
    NoteEffects/TransposeFrom.cpp
    NoteEffects/TransposeFrom.h
    NoteEffects/UnstableModWheel.cpp
    NoteEffects/UnstableModWheel.h
    NoteEffects/UnstablePitch.cpp
    NoteEffects/UnstablePitch.h
    NoteEffects/UnstablePressure.cpp
    NoteEffects/UnstablePressure.h
    NoteEffects/VelocityCurve.cpp
    NoteEffects/VelocityCurve.h
    NoteEffects/VelocityScaler.cpp
    NoteEffects/VelocityScaler.h
    NoteEffects/VelocitySetter.cpp
    NoteEffects/VelocitySetter.h
    NoteEffects/VelocityStepSequencer.cpp
    NoteEffects/VelocityStepSequencer.h
    NoteEffects/VelocityToChance.cpp
    NoteEffects/VelocityToChance.h
    NoteEffects/VolcaBeatsControl.cpp
    NoteEffects/VolcaBeatsControl.h
    NoteEffects/WhiteKeys.cpp
    NoteEffects/WhiteKeys.h
    NoteOutputQueue.cpp
    NoteOutputQueue.h
    NoteToggle.cpp
    NoteToggle.h
    ofxJSONElement.cpp
    ofxJSONElement.h
    OpenFrameworksPort.cpp
    OpenFrameworksPort.h
    OscController.cpp
    OscController.h
    Oscillator.cpp
    Oscillator.h
    OSCOutput.cpp
    OSCOutput.h
    PanicButton.cpp
    PanicButton.h
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
    PitchDetector.cpp
    PitchDetector.h
    PitchShiftEffect.cpp
    PitchShiftEffect.h
    PitchShifter.cpp
    PitchShifter.h
    PolyphonyMgr.cpp
    PolyphonyMgr.h
    Prefab.cpp
    Prefab.h
    Presets.cpp
    Presets.h
    Profiler.cpp
    Profiler.h
    PSMoveController.cpp
    PSMoveController.h
    PSMoveMgr.cpp
    PSMoveMgr.h
    Pulse/AudioToPulse.cpp
    Pulse/AudioToPulse.h
    Pulse/NoteToPulse.cpp
    Pulse/NoteToPulse.h
    Pulse/BoundsToPulse.cpp
    Pulse/BoundsToPulse.h
    Pulse/PulseButton.cpp
    Pulse/PulseButton.h
    Pulse/PulseChance.cpp
    Pulse/PulseChance.h
    Pulse/PulseDelayer.cpp
    Pulse/PulseDelayer.h
    Pulse/PulseGate.cpp
    Pulse/PulseGate.h
    Pulse/PulseHocket.cpp
    Pulse/PulseHocket.h
    Pulse/Pulser.cpp
    Pulse/Pulser.h
    Pulse/PulseSequence.cpp
    Pulse/PulseSequence.h
    Pulse/PulseTrain.cpp
    Pulse/PulseTrain.h
    Pumper.cpp
    Pumper.h
    Push2Control.cpp
    Push2Control.h
    QuickSpawnMenu.cpp
    QuickSpawnMenu.h
    RadioButton.cpp
    RadioButton.h
    RadioSequencer.cpp
    RadioSequencer.h
    Ramp.cpp
    Ramp.h
    RollingBuffer.cpp
    RollingBuffer.h
    SampleBrowser.cpp
    SampleBrowser.h
    Sample.cpp
    SampleDrawer.cpp
    SampleDrawer.h
    Sample.h
    SampleLayerer.cpp
    SampleLayerer.h
    SampleVoice.cpp
    SampleVoice.h
    Scale.cpp
    Scale.h
    ScriptModule.cpp
    ScriptModule.h
    ScriptModule_PythonInterface.i
    ScriptStatus.cpp
    ScriptStatus.h
    Selector.cpp
    Selector.h
    SingleOscillatorVoice.cpp
    SingleOscillatorVoice.h
    Slider.cpp
    Slider.h
    SongBuilder.cpp
    SongBuilder.h
    SpaceMouseControl.cpp
    SpaceMouseControl.h
    Stutter.cpp
    Stutter.h
    SwitchAndRamp.cpp
    SwitchAndRamp.h
    SynthGlobals.cpp
    SynthGlobals.h
    Synths/BeatBloks.cpp
    Synths/BeatBloks.h
    Synths/Beats.cpp
    Synths/Beats.h
    Synths/ClipLauncher.cpp
    Synths/ClipLauncher.h
    Synths/ControllingSong.cpp
    Synths/ControllingSong.h
    Synths/ControlTactileFeedback.cpp
    Synths/ControlTactileFeedback.h
    Synths/DebugAudioSource.cpp
    Synths/DebugAudioSource.h
    Synths/DrumPlayer.cpp
    Synths/DrumPlayer.h
    Synths/DrumSynth.cpp
    Synths/DrumSynth.h
    Synths/FMSynth.cpp
    Synths/FMSynth.h
    Synths/FollowingSong.cpp
    Synths/FollowingSong.h
    Synths/KarplusStrong.cpp
    Synths/KarplusStrong.h
    Synths/Metronome.cpp
    Synths/Metronome.h
    Synths/Razor.cpp
    Synths/Razor.h
    Synths/SampleCanvas.cpp
    Synths/SampleCanvas.h
    Synths/SamplePlayer.cpp
    Synths/SamplePlayer.h
    Synths/Sampler.cpp
    Synths/Sampler.h
    Synths/SeaOfGrain.cpp
    Synths/SeaOfGrain.h
    Synths/SignalGenerator.cpp
    Synths/SignalGenerator.h
    Synths/SingleOscillator.cpp
    Synths/SingleOscillator.h
    Synths/VSTPlugin.cpp
    Synths/VSTPlugin.h
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
    TremoloEffect.cpp
    TremoloEffect.h
    TriggerDetector.cpp
    TriggerDetector.h
    UIControlMacros.h
    UIGrid.cpp
    UIGrid.h
    UserData.cpp
    UserData.h
    UserPrefs.cpp
    UserPrefsEditor.cpp
    UserPrefsEditor.h
    UserPrefs.h
    ValueStream.cpp
    ValueStream.h
    VSTPlayhead.cpp
    VSTPlayhead.h
    VSTScanner.cpp
    VSTScanner.h
    VSTWindow.cpp
    VSTWindow.h
    WindowCloseListener.h
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
        NoteEffects/KompleteKontrol.cpp
        NoteEffects/KompleteKontrol.h
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
    install(FILES ${CMAKE_SOURCE_DIR}/bespoke_icon.png DESTINATION share/icons/hicolor/512x512/apps)
endif ()

target_link_libraries(BespokeSynth PRIVATE
    bespoke::exprtk
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
