/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  ModuleFactory.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//


#if BESPOKE_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "ModuleFactory.h"

#include "LaunchpadKeyboard.h"
#include "DrumPlayer.h"
#include "EffectChain.h"
#include "LooperRecorder.h"
#include "Chorder.h"
#include "Arpeggiator.h"
#include "Razor.h"
#include "Monophonify.h"
#include "StepSequencer.h"
#include "InputChannel.h"
#include "OutputChannel.h"
#include "WaveformViewer.h"
#include "FMSynth.h"
#include "MidiController.h"
#include "PSMoveController.h"
#include "Autotalent.h"
#include "ScaleDetect.h"
#include "KarplusStrong.h"
#include "WhiteKeys.h"
#include "RingModulator.h"
#include "Neighborhooder.h"
#include "Polyrhythms.h"
#include "Looper.h"
#include "Rewriter.h"
#include "Metronome.h"
#include "NoteRouter.h"
#include "NoteLooper.h"
#include "AudioRouter.h"
#include "LaunchpadNoteDisplayer.h"
#include "Vocoder.h"
#include "VocoderCarrierInput.h"
#include "FreqDomainBoilerplate.h"
#include "FFTtoAdditive.h"
#include "FreqDelay.h"
#include "VelocitySetter.h"
#include "NoteSinger.h"
#include "NoteOctaver.h"
#include "SampleFinder.h"
#include "FourOnTheFloor.h"
#include "Amplifier.h"
#include "Snapshots.h"
#include "LFOController.h"
#include "NoteStepSequencer.h"
#include "BeatBloks.h"
#include "Producer.h"
#include "ChaosEngine.h"
#include "SingleOscillator.h"
#include "BandVocoder.h"
#include "Capo.h"
#include "ControlTactileFeedback.h"
//#include "Eigenharp.h"
#include "Beats.h"
#include "Sampler.h"
#include "SliderSequencer.h"
#include "MultibandCompressor.h"
#include "ControllingSong.h"
#include "PanicButton.h"
#include "VelocityStepSequencer.h"
#include "SustainPedal.h"
#include "MultitrackRecorder.h"
//#include "MidiPlayer.h"
#include "SamplerGrid.h"
#include "SignalGenerator.h"
#include "Lissajous.h"
#include "DebugAudioSource.h"
#include "TimerDisplay.h"
#include "DrumSynth.h"
//#include "EigenChorder.h"
#include "PitchBender.h"
//#include "EigenharpNoteSource.h"
#include "FollowingSong.h"
#include "SeaOfGrain.h"
#include "VinylTempoControl.h"
#include "NoteFlusher.h"
#include "FilterViz.h"
#include "NoteCanvas.h"
#include "SlowLayers.h"
#include "ClipLauncher.h"
#include "LoopStorer.h"
#include "CommentDisplay.h"
#include "ComboGridController.h"
#include "StutterControl.h"
#include "CircleSequencer.h"
#ifdef BESPOKE_MAC
#include "KompleteKontrol.h"
#endif
#include "MidiOutput.h"
#include "FloatSliderLFOControl.h"
#include "NoteDisplayer.h"
#include "AudioMeter.h"
#include "NoteSustain.h"
#include "PitchChorus.h"
#include "SampleCanvas.h"
#include "ControlSequencer.h"
#include "PitchSetter.h"
#include "NoteFilter.h"
#include "RandomNoteGenerator.h"
#include "NoteToFreq.h"
#include "MacroSlider.h"
#include "NoteVibrato.h"
#include "ModulationVisualizer.h"
#include "PitchDive.h"
#include "EventCanvas.h"
#include "NoteCreator.h"
#include "ValueSetter.h"
#include "PreviousNote.h"
#include "PressureToVibrato.h"
#include "ModwheelToVibrato.h"
#include "Pressure.h"
#include "ModWheel.h"
#include "PressureToModwheel.h"
#include "ModwheelToPressure.h"
#include "VSTPlugin.h"
#include "FeedbackModule.h"
#include "NoteToMs.h"
#include "Selector.h"
#include "GroupControl.h"
#include "CurveLooper.h"
#include "ScaleDegree.h"
#include "NoteChainNode.h"
#include "NoteDelayer.h"
#include "TimelineControl.h"
#include "VelocityScaler.h"
#include "KeyboardDisplay.h"
#include "Ramper.h"
#include "NoteGate.h"
#include "Prefab.h"
#include "NoteHumanizer.h"
#include "VolcaBeatsControl.h"
#include "RadioSequencer.h"
#include "TakeRecorder.h"
#include "AudioSplitter.h"
#include "Splitter.h"
#include "Panner.h"
#include "SamplePlayer.h"
#include "AudioSend.h"
#include "EnvelopeEditor.h"
#include "EnvelopeModulator.h"
#include "AudioToCV.h"
#include "ModulatorAdd.h"
#include "ModulatorAddCentered.h"
#include "PitchToCV.h"
#include "VelocityToCV.h"
#include "PressureToCV.h"
#include "ModWheelToCV.h"
#include "ModulatorMult.h"
#include "ModulatorCurve.h"
#include "ModulatorSmoother.h"
#include "NotePanner.h"
#include "PitchPanner.h"
#include "NotePanAlternator.h"
#include "ChordDisplayer.h"
#include "NoteStrummer.h"
#include "PitchToSpeed.h"
#include "NoteToPulse.h"
#include "OSCOutput.h"
#include "AudioLevelToCV.h"
#include "Pulser.h"
#include "PulseSequence.h"
#include "LinnstrumentControl.h"
#include "MultitapDelay.h"
#include "MidiCapturer.h"
#include "Inverter.h"
#include "SpectralDisplay.h"
#include "DCOffset.h"
#include "SignalClamp.h"
#include "Waveshaper.h"
#include "ModulatorSubtract.h"
#include "NoteHocket.h"
#include "NoteRangeFilter.h"
#include "NoteChance.h"
#include "PulseChance.h"
#include "PulseDelayer.h"
#include "NotePanRandom.h"
#include "PulseGate.h"
#include "PulseHocket.h"
#include "Push2Control.h"
#include "PulseTrain.h"
#include "NoteLatch.h"
#include "ScriptModule.h"
#include "ScriptStatus.h"
#include "ModulatorGravity.h"
#include "NoteStreamDisplay.h"
#include "PulseButton.h"
#include "GridModule.h"
#include "FubbleModule.h"
#include "GlobalControls.h"
#include "ValueStream.h"
#include "EQModule.h"
#include "SampleCapturer.h"
#include "NoteQuantizer.h"
#include "PlaySequencer.h"
#include "UnstablePitch.h"
#include "UnstableModWheel.h"
#include "UnstablePressure.h"
#include "ChordHolder.h"
#include "LooperGranulator.h"
#include "AudioToPulse.h"
#include "NoteCounter.h"
#include "PitchRemap.h"
#include "ModulatorExpression.h"
#include "SampleBrowser.h"
#include "TransposeFrom.h"
#include "NoteStepper.h"
#include "M185Sequencer.h"
#include "ModulatorAccum.h"
#include "GridSliders.h"
#include "NoteRatchet.h"
#include "NoteSorter.h"
#include "MPESmoother.h"
#include "MidiControlChange.h"
#include "MPETweaker.h"
#include "NoteExpressionRouter.h"
#include "NoteToggle.h"
#include "NoteTable.h"
#include "AbletonLink.h"
#include "MidiClockIn.h"
#include "MidiClockOut.h"
#include "VelocityToChance.h"
#include "NoteEcho.h"
#include "VelocityCurve.h"
#include "BoundsToPulse.h"
#include "SongBuilder.h"
#include "PulseFlag.h"
#include "PulseDisplayer.h"
#include "BufferShuffler.h"
#include "PitchToValue.h"
#include "RhythmSequencer.h"
#include "DotSequencer.h"
#include "VoiceSetter.h"
#include "LabelDisplay.h"
#include "ControlRecorder.h"
#include "EuclideanSequencer.h"
#include "SaveStateLoader.h"
#include "DataProvider.h"
#include "PulseLimit.h"
#include "BassLineSequencer.h"
#include "Acciaccatura.h"
#include "ModulatorWander.h"

#include <juce_core/juce_core.h>

#include "PulseRouter.h"

#define REGISTER(class, name, type) Register(#name, &(class ::Create), &(class ::CanCreate), type, false, false, class ::AcceptsAudio(), class ::AcceptsNotes(), class ::AcceptsPulses());
#define REGISTER_HIDDEN(class, name, type) Register(#name, &(class ::Create), &(class ::CanCreate), type, true, false, class ::AcceptsAudio(), class ::AcceptsNotes(), class ::AcceptsPulses());
#define REGISTER_EXPERIMENTAL(class, name, type) Register(#name, &(class ::Create), &(class ::CanCreate), type, false, true, class ::AcceptsAudio(), class ::AcceptsNotes(), class ::AcceptsPulses());

ModuleFactory::ModuleFactory()
{
   REGISTER(LooperRecorder, looperrecorder, kModuleCategory_Audio);
   REGISTER(WaveformViewer, waveformviewer, kModuleCategory_Audio);
   REGISTER(EffectChain, effectchain, kModuleCategory_Audio);
   REGISTER(DrumPlayer, drumplayer, kModuleCategory_Synth);
   REGISTER(Chorder, chorder, kModuleCategory_Note);
   REGISTER(Arpeggiator, arpeggiator, kModuleCategory_Note);
   REGISTER(Monophonify, portamento, kModuleCategory_Note);
   REGISTER(StepSequencer, drumsequencer, kModuleCategory_Instrument);
   REGISTER(LaunchpadKeyboard, gridkeyboard, kModuleCategory_Instrument);
   REGISTER(FMSynth, fmsynth, kModuleCategory_Synth);
   REGISTER(MidiController, midicontroller, kModuleCategory_Instrument);
   REGISTER(ScaleDetect, scaledetect, kModuleCategory_Note);
   REGISTER(KarplusStrong, karplusstrong, kModuleCategory_Synth);
   REGISTER(WhiteKeys, whitekeys, kModuleCategory_Note);
   //REGISTER(Kicker, kicker, kModuleCategory_Note);
   REGISTER(RingModulator, ringmodulator, kModuleCategory_Audio);
   REGISTER(Neighborhooder, notewrap, kModuleCategory_Note);
   REGISTER(Polyrhythms, polyrhythms, kModuleCategory_Instrument);
   REGISTER(Looper, looper, kModuleCategory_Audio);
   REGISTER(Rewriter, looperrewriter, kModuleCategory_Audio);
   REGISTER(Metronome, metronome, kModuleCategory_Synth);
   REGISTER(NoteRouter, noterouter, kModuleCategory_Note);
   REGISTER(AudioRouter, audiorouter, kModuleCategory_Audio);
   REGISTER(LaunchpadNoteDisplayer, gridnotedisplayer, kModuleCategory_Note);
   REGISTER(Vocoder, fftvocoder, kModuleCategory_Audio);
   REGISTER(FreqDelay, freqdelay, kModuleCategory_Audio);
   REGISTER(VelocitySetter, velocitysetter, kModuleCategory_Note);
   REGISTER(NoteSinger, notesinger, kModuleCategory_Instrument);
   REGISTER(NoteOctaver, noteoctaver, kModuleCategory_Note);
   REGISTER(FourOnTheFloor, fouronthefloor, kModuleCategory_Instrument);
   REGISTER(Amplifier, gain, kModuleCategory_Audio);
   REGISTER(Snapshots, snapshots, kModuleCategory_Other);
   REGISTER(NoteStepSequencer, notesequencer, kModuleCategory_Instrument);
   REGISTER(SingleOscillator, oscillator, kModuleCategory_Synth);
   REGISTER(BandVocoder, vocoder, kModuleCategory_Audio);
   REGISTER(Capo, capo, kModuleCategory_Note);
   REGISTER(VocoderCarrierInput, vocodercarrier, kModuleCategory_Audio);
   REGISTER(InputChannel, input, kModuleCategory_Audio);
   REGISTER(OutputChannel, output, kModuleCategory_Audio);
   //REGISTER(Eigenharp, eigenharp, kModuleCategory_Synth);
   REGISTER(Beats, beats, kModuleCategory_Synth);
   REGISTER(Sampler, sampler, kModuleCategory_Synth);
   //REGISTER(NoteTransformer, notetransformer, kModuleCategory_Note);
   REGISTER(SliderSequencer, slidersequencer, kModuleCategory_Instrument);
   REGISTER(VelocityStepSequencer, velocitystepsequencer, kModuleCategory_Note);
   REGISTER(SustainPedal, sustainpedal, kModuleCategory_Note);
   REGISTER(SamplerGrid, samplergrid, kModuleCategory_Audio);
   REGISTER(SignalGenerator, signalgenerator, kModuleCategory_Synth);
   REGISTER(Lissajous, lissajous, kModuleCategory_Audio);
   REGISTER(TimerDisplay, timerdisplay, kModuleCategory_Other);
   REGISTER(DrumSynth, drumsynth, kModuleCategory_Synth);
   //REGISTER(EigenChorder, eigenchorder, kModuleCategory_Note);
   REGISTER(PitchBender, pitchbender, kModuleCategory_Note);
   //REGISTER(EigenharpNoteSource, eigenharpnotesource, kModuleCategory_Instrument);
   REGISTER(VinylTempoControl, vinylcontrol, kModuleCategory_Modulator);
   REGISTER(NoteFlusher, noteflusher, kModuleCategory_Note);
   REGISTER(NoteCanvas, notecanvas, kModuleCategory_Instrument);
   REGISTER(CommentDisplay, comment, kModuleCategory_Other);
   REGISTER(LabelDisplay, label, kModuleCategory_Other);
   REGISTER(StutterControl, stutter, kModuleCategory_Audio);
   REGISTER(CircleSequencer, circlesequencer, kModuleCategory_Instrument);
   REGISTER(MidiOutputModule, midioutput, kModuleCategory_Note);
   REGISTER(NoteDisplayer, notedisplayer, kModuleCategory_Note);
   REGISTER(AudioMeter, audiometer, kModuleCategory_Audio);
   REGISTER(NoteSustain, noteduration, kModuleCategory_Note);
   REGISTER(ControlSequencer, controlsequencer, kModuleCategory_Modulator);
   REGISTER(PitchSetter, pitchsetter, kModuleCategory_Note);
   REGISTER(NoteFilter, notefilter, kModuleCategory_Note);
   REGISTER(PulseRouter, pulserouter, kModuleCategory_Pulse);
   REGISTER(RandomNoteGenerator, randomnote, kModuleCategory_Instrument);
   REGISTER(NoteToFreq, notetofreq, kModuleCategory_Modulator);
   REGISTER(MacroSlider, macroslider, kModuleCategory_Modulator);
   REGISTER(NoteVibrato, vibrato, kModuleCategory_Note);
   REGISTER(ModulationVisualizer, modulationvizualizer, kModuleCategory_Note);
   REGISTER(PitchDive, pitchdive, kModuleCategory_Note);
   REGISTER(EventCanvas, eventcanvas, kModuleCategory_Other);
   REGISTER(NoteCreator, notecreator, kModuleCategory_Instrument);
   REGISTER(ValueSetter, valuesetter, kModuleCategory_Modulator);
   REGISTER(PreviousNote, previousnote, kModuleCategory_Note);
   REGISTER(PressureToVibrato, pressuretovibrato, kModuleCategory_Note);
   REGISTER(ModwheelToVibrato, modwheeltovibrato, kModuleCategory_Note);
   REGISTER(Pressure, pressure, kModuleCategory_Note);
   REGISTER(ModWheel, modwheel, kModuleCategory_Note);
   REGISTER(PressureToModwheel, pressuretomodwheel, kModuleCategory_Note);
   REGISTER(ModwheelToPressure, modwheeltopressure, kModuleCategory_Note);
   REGISTER(FeedbackModule, feedback, kModuleCategory_Audio);
   REGISTER(NoteToMs, notetoms, kModuleCategory_Modulator);
   REGISTER(Selector, selector, kModuleCategory_Other);
   REGISTER(GroupControl, groupcontrol, kModuleCategory_Other);
   REGISTER(CurveLooper, curvelooper, kModuleCategory_Modulator);
   REGISTER(ScaleDegree, scaledegree, kModuleCategory_Note);
   REGISTER(NoteChainNode, notechain, kModuleCategory_Instrument);
   REGISTER(NoteDelayer, notedelayer, kModuleCategory_Note);
   REGISTER(VelocityScaler, velocityscaler, kModuleCategory_Note);
   REGISTER(KeyboardDisplay, keyboarddisplay, kModuleCategory_Instrument);
   REGISTER(Ramper, ramper, kModuleCategory_Modulator);
   REGISTER(NoteGate, notegate, kModuleCategory_Note);
   REGISTER(Prefab, prefab, kModuleCategory_Other);
   REGISTER(NoteHumanizer, notehumanizer, kModuleCategory_Note);
   REGISTER(VolcaBeatsControl, volcabeatscontrol, kModuleCategory_Note);
   REGISTER(RadioSequencer, radiosequencer, kModuleCategory_Other);
   REGISTER(AudioSplitter, audiosplitter, kModuleCategory_Audio);
   REGISTER(Splitter, splitter, kModuleCategory_Audio);
   REGISTER(Panner, panner, kModuleCategory_Audio);
   REGISTER(SamplePlayer, sampleplayer, kModuleCategory_Synth);
   REGISTER(AudioSend, send, kModuleCategory_Audio);
   REGISTER(EnvelopeModulator, envelope, kModuleCategory_Modulator);
   REGISTER(AudioToCV, audiotocv, kModuleCategory_Modulator);
   REGISTER(ModulatorAdd, add, kModuleCategory_Modulator);
   REGISTER(ModulatorAddCentered, addcentered, kModuleCategory_Modulator);
   REGISTER(ModulatorSubtract, subtract, kModuleCategory_Modulator);
   REGISTER(PitchToCV, pitchtocv, kModuleCategory_Modulator);
   REGISTER(VelocityToCV, velocitytocv, kModuleCategory_Modulator);
   REGISTER(PressureToCV, pressuretocv, kModuleCategory_Modulator);
   REGISTER(ModWheelToCV, modwheeltocv, kModuleCategory_Modulator);
   REGISTER(ModulatorMult, mult, kModuleCategory_Modulator);
   REGISTER(ModulatorCurve, curve, kModuleCategory_Modulator);
   REGISTER(ModulatorSmoother, smoother, kModuleCategory_Modulator);
   REGISTER(NotePanner, notepanner, kModuleCategory_Note);
   REGISTER(PitchPanner, pitchpanner, kModuleCategory_Note);
   REGISTER(NotePanAlternator, notepanalternator, kModuleCategory_Note);
   REGISTER(ChordDisplayer, chorddisplayer, kModuleCategory_Note);
   REGISTER(NoteStrummer, notestrummer, kModuleCategory_Note);
   REGISTER(SeaOfGrain, seaofgrain, kModuleCategory_Synth);
   REGISTER(PitchToSpeed, pitchtospeed, kModuleCategory_Modulator);
   REGISTER(NoteToPulse, notetopulse, kModuleCategory_Pulse);
   REGISTER(OSCOutput, oscoutput, kModuleCategory_Other);
   REGISTER(AudioLevelToCV, leveltocv, kModuleCategory_Modulator);
   REGISTER(Pulser, pulser, kModuleCategory_Pulse);
   REGISTER(PulseSequence, pulsesequence, kModuleCategory_Pulse);
   REGISTER(LinnstrumentControl, linnstrumentcontrol, kModuleCategory_Note);
   REGISTER(MultitapDelay, multitapdelay, kModuleCategory_Audio);
   REGISTER(Inverter, inverter, kModuleCategory_Audio);
   REGISTER(SpectralDisplay, spectrum, kModuleCategory_Audio);
   REGISTER(DCOffset, dcoffset, kModuleCategory_Audio);
   REGISTER(SignalClamp, signalclamp, kModuleCategory_Audio);
   REGISTER(Waveshaper, waveshaper, kModuleCategory_Audio);
   REGISTER(NoteHocket, notehocket, kModuleCategory_Note);
   REGISTER(NoteRangeFilter, noterangefilter, kModuleCategory_Note);
   REGISTER(NoteChance, notechance, kModuleCategory_Note);
   REGISTER(PulseChance, pulsechance, kModuleCategory_Pulse);
   REGISTER(PulseDelayer, pulsedelayer, kModuleCategory_Pulse);
   REGISTER(NotePanRandom, notepanrandom, kModuleCategory_Note);
   REGISTER(PulseGate, pulsegate, kModuleCategory_Pulse);
   REGISTER(PulseHocket, pulsehocket, kModuleCategory_Pulse);
   REGISTER(Push2Control, push2control, kModuleCategory_Other);
   REGISTER(PulseTrain, pulsetrain, kModuleCategory_Pulse);
   REGISTER(NoteLatch, notelatch, kModuleCategory_Note);
   REGISTER(ScriptModule, script, kModuleCategory_Other);
   REGISTER(ScriptStatus, scriptstatus, kModuleCategory_Other);
   REGISTER(ModulatorGravity, gravity, kModuleCategory_Modulator);
   REGISTER(NoteStreamDisplay, notestream, kModuleCategory_Note);
   REGISTER(PulseButton, pulsebutton, kModuleCategory_Pulse);
   REGISTER(GridModule, grid, kModuleCategory_Other);
   REGISTER(FubbleModule, fubble, kModuleCategory_Modulator);
   REGISTER(GlobalControls, globalcontrols, kModuleCategory_Other);
   REGISTER(ValueStream, valuestream, kModuleCategory_Other);
   REGISTER(EQModule, eq, kModuleCategory_Audio);
   REGISTER(SampleCapturer, samplecapturer, kModuleCategory_Audio);
   REGISTER(NoteQuantizer, quantizer, kModuleCategory_Note);
   REGISTER(NoteLooper, notelooper, kModuleCategory_Instrument);
   REGISTER(PlaySequencer, playsequencer, kModuleCategory_Instrument);
   REGISTER(UnstablePitch, unstablepitch, kModuleCategory_Note);
   REGISTER(UnstableModWheel, unstablemodwheel, kModuleCategory_Note);
   REGISTER(UnstablePressure, unstablepressure, kModuleCategory_Note);
   REGISTER(ChordHolder, chordholder, kModuleCategory_Note);
   REGISTER(LooperGranulator, loopergranulator, kModuleCategory_Other);
   REGISTER(AudioToPulse, audiotopulse, kModuleCategory_Pulse);
   REGISTER(NoteCounter, notecounter, kModuleCategory_Instrument);
   REGISTER(PitchRemap, pitchremap, kModuleCategory_Note);
   REGISTER(ModulatorExpression, expression, kModuleCategory_Modulator);
   REGISTER(SampleCanvas, samplecanvas, kModuleCategory_Synth);
   REGISTER(SampleBrowser, samplebrowser, kModuleCategory_Other);
   REGISTER(TransposeFrom, transposefrom, kModuleCategory_Note);
   REGISTER(NoteStepper, notestepper, kModuleCategory_Note);
   REGISTER(M185Sequencer, m185sequencer, kModuleCategory_Instrument);
   REGISTER(ModulatorAccum, accum, kModuleCategory_Modulator);
   REGISTER(NoteSorter, notesorter, kModuleCategory_Note);
   REGISTER(NoteRatchet, noteratchet, kModuleCategory_Note);
   REGISTER(MPESmoother, mpesmoother, kModuleCategory_Note);
   REGISTER(MidiControlChange, midicc, kModuleCategory_Note);
   REGISTER(MPETweaker, mpetweaker, kModuleCategory_Note);
   REGISTER(GridSliders, gridsliders, kModuleCategory_Modulator);
   REGISTER(MultitrackRecorder, multitrackrecorder, kModuleCategory_Other);
   REGISTER(NoteExpressionRouter, noteexpression, kModuleCategory_Note);
   REGISTER(FloatSliderLFOControl, lfo, kModuleCategory_Modulator);
   REGISTER(NoteToggle, notetoggle, kModuleCategory_Other);
   REGISTER(NoteTable, notetable, kModuleCategory_Note);
   REGISTER(AbletonLink, abletonlink, kModuleCategory_Other);
   REGISTER(MidiClockIn, clockin, kModuleCategory_Other);
   REGISTER(MidiClockOut, clockout, kModuleCategory_Other);
   REGISTER(VelocityToChance, velocitytochance, kModuleCategory_Note);
   REGISTER(NoteEcho, noteecho, kModuleCategory_Note);
   REGISTER(VelocityCurve, velocitycurve, kModuleCategory_Note);
   REGISTER(BoundsToPulse, boundstopulse, kModuleCategory_Pulse);
   REGISTER(SongBuilder, songbuilder, kModuleCategory_Other);
   REGISTER(TimelineControl, timelinecontrol, kModuleCategory_Other);
   REGISTER(PulseFlag, pulseflag, kModuleCategory_Pulse);
   REGISTER(PulseDisplayer, pulsedisplayer, kModuleCategory_Pulse);
   REGISTER(BufferShuffler, buffershuffler, kModuleCategory_Audio);
   REGISTER(PitchToValue, pitchtovalue, kModuleCategory_Modulator);
   REGISTER(RhythmSequencer, rhythmsequencer, kModuleCategory_Note);
   REGISTER(DotSequencer, dotsequencer, kModuleCategory_Instrument);
   REGISTER(VoiceSetter, voicesetter, kModuleCategory_Note);
   REGISTER(ControlRecorder, controlrecorder, kModuleCategory_Modulator);
   REGISTER(EuclideanSequencer, euclideansequencer, kModuleCategory_Instrument);
   REGISTER(SaveStateLoader, savestateloader, kModuleCategory_Other);
   REGISTER(DataProvider, dataprovider, kModuleCategory_Modulator);
   REGISTER(PulseLimit, pulselimit, kModuleCategory_Pulse);
   REGISTER(BassLineSequencer, basslinesequencer, kModuleCategory_Instrument);
   REGISTER(Acciaccatura, acciaccatura, kModuleCategory_Note);
   REGISTER(ModulatorWander, wander, kModuleCategory_Modulator);

   //REGISTER_EXPERIMENTAL(MidiPlayer, midiplayer, kModuleCategory_Instrument);
   REGISTER_HIDDEN(Autotalent, autotalent, kModuleCategory_Audio);
   REGISTER_HIDDEN(TakeRecorder, takerecorder, kModuleCategory_Audio);
   REGISTER_HIDDEN(LoopStorer, loopstorer, kModuleCategory_Other);
   REGISTER_HIDDEN(PitchChorus, pitchchorus, kModuleCategory_Audio);
   REGISTER_HIDDEN(ComboGridController, combogrid, kModuleCategory_Other);
   REGISTER_HIDDEN(VSTPlugin, plugin, kModuleCategory_Synth);
   REGISTER_HIDDEN(SampleFinder, samplefinder, kModuleCategory_Audio);
   REGISTER_HIDDEN(Producer, producer, kModuleCategory_Audio);
   REGISTER_HIDDEN(ChaosEngine, chaosengine, kModuleCategory_Other);
   REGISTER_HIDDEN(MultibandCompressor, multiband, kModuleCategory_Audio);
   REGISTER_HIDDEN(ControllingSong, controllingsong, kModuleCategory_Synth);
   REGISTER_HIDDEN(PanicButton, panicbutton, kModuleCategory_Other);
   REGISTER_HIDDEN(DebugAudioSource, debugaudiosource, kModuleCategory_Synth);
   REGISTER_HIDDEN(FollowingSong, followingsong, kModuleCategory_Synth);
   REGISTER_HIDDEN(BeatBloks, beatbloks, kModuleCategory_Synth);
   REGISTER_HIDDEN(FilterViz, filterviz, kModuleCategory_Other);
   REGISTER_HIDDEN(FreqDomainBoilerplate, freqdomainboilerplate, kModuleCategory_Audio);
   REGISTER_HIDDEN(FFTtoAdditive, ffttoadditive, kModuleCategory_Audio);
   REGISTER_HIDDEN(SlowLayers, slowlayers, kModuleCategory_Audio);
   REGISTER_HIDDEN(ClipLauncher, cliplauncher, kModuleCategory_Synth);
#ifdef BESPOKE_MAC
   REGISTER_HIDDEN(KompleteKontrol, kompletekontrol, kModuleCategory_Note);
#endif
   REGISTER_HIDDEN(PSMoveController, psmove, kModuleCategory_Other);
   REGISTER_HIDDEN(ControlTactileFeedback, controltactilefeedback, kModuleCategory_Synth);
   REGISTER_HIDDEN(EnvelopeEditor, envelopeeditor, kModuleCategory_Other);
   REGISTER_HIDDEN(LFOController, lfocontroller, kModuleCategory_Other); //old, probably irrelevant
   REGISTER_HIDDEN(Razor, razor, kModuleCategory_Synth);
   REGISTER_HIDDEN(MidiCapturer, midicapturer, kModuleCategory_Note);
   REGISTER_HIDDEN(ScriptReferenceDisplay, scriptingreference, kModuleCategory_Other);
   REGISTER_HIDDEN(ScriptWarningPopup, scriptwarning, kModuleCategory_Other);
   REGISTER_HIDDEN(MultitrackRecorderTrack, multitrackrecordertrack, kModuleCategory_Audio);
}

void ModuleFactory::Register(std::string type, CreateModuleFn creator, CanCreateModuleFn canCreate, ModuleCategory moduleCategory, bool hidden, bool experimental, bool canReceiveAudio, bool canReceiveNotes, bool canReceivePulses)
{
   ModuleInfo moduleInfo;
   moduleInfo.mCreatorFn = creator;
   moduleInfo.mCanCreateFn = canCreate;
   moduleInfo.mCategory = moduleCategory;
   moduleInfo.mIsHidden = hidden;
   moduleInfo.mIsExperimental = experimental;
   moduleInfo.mCanReceiveAudio = canReceiveAudio;
   moduleInfo.mCanReceiveNotes = canReceiveNotes;
   moduleInfo.mCanReceivePulses = canReceivePulses;
   mFactoryMap[type] = moduleInfo;
}

IDrawableModule* ModuleFactory::MakeModule(std::string type)
{
   auto moduleInfo = mFactoryMap.find(type);
   if (moduleInfo != mFactoryMap.end())
   {
      if (moduleInfo->second.mCanCreateFn())
         return moduleInfo->second.mCreatorFn();
   }
   return nullptr;
}

std::vector<ModuleFactory::Spawnable> ModuleFactory::GetSpawnableModules(ModuleCategory moduleCategory)
{
   std::vector<ModuleFactory::Spawnable> modules{};
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
   {
      if (iter->second.mCategory == moduleCategory &&
          (!iter->second.mIsHidden || gShowDevModules))
      {
         ModuleFactory::Spawnable spawnable{};
         spawnable.mLabel = iter->first;
         modules.push_back(spawnable);
      }
   }

   if (moduleCategory == kModuleCategory_Audio)
   {
      std::vector<std::string> effects = TheSynth->GetEffectFactory()->GetSpawnableEffects();
      for (auto effect : effects)
      {
         ModuleFactory::Spawnable spawnable{};
         spawnable.mLabel = effect;
         spawnable.mDecorator = kEffectChainSuffix;
         spawnable.mSpawnMethod = SpawnMethod::EffectChain;
         modules.push_back(spawnable);
      }
   }

   std::sort(modules.begin(), modules.end(), Spawnable::CompareAlphabetical);
   return modules;
}

namespace
{
   bool CheckHeldKeysMatch(juce::String name, std::string heldKeys, bool continuous)
   {
      name = name.toLowerCase();

      if (name.isEmpty() || heldKeys.empty())
         return false;

      if (continuous)
         return name.contains(heldKeys);

      if (name[0] != heldKeys[0])
         return false;

      int stringPos = 0;
      int end = name.indexOfChar('.');
      if (end == -1)
         end = name.indexOfChar(' ');
      if (end == -1)
         end = name.length() - 1;
      for (size_t j = 1; j < heldKeys.length(); ++j)
      {
         stringPos = name.substring(stringPos + 1, end + 1).indexOfChar(heldKeys[j]);
         if (stringPos == -1) //couldn't find key in remaining string
            return false;
      }

      return true;
   }
}

std::vector<ModuleFactory::Spawnable> ModuleFactory::GetSpawnableModules(std::string keys, bool continuousString)
{
   std::vector<ModuleFactory::Spawnable> modules{};
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
   {
      if ((!iter->second.mIsHidden || gShowDevModules) &&
          CheckHeldKeysMatch(iter->first, keys, continuousString))
      {
         ModuleFactory::Spawnable spawnable{};
         spawnable.mLabel = iter->first;
         modules.push_back(spawnable);
      }
   }

   std::vector<juce::PluginDescription> vsts;
   VSTLookup::GetAvailableVSTs(vsts);
   std::vector<juce::PluginDescription> matchingVsts;
   for (auto& pluginDesc : vsts)
   {
      std::string pluginName = pluginDesc.name.toStdString();
      if (CheckHeldKeysMatch(pluginName, keys, continuousString))
         matchingVsts.push_back(pluginDesc);
   }
   const int kMaxQuickspawnVstCount = 10;
   if ((int)matchingVsts.size() > kMaxQuickspawnVstCount)
      VSTLookup::SortByLastUsed(matchingVsts);

   for (int i = 0; i < (int)matchingVsts.size() && i < kMaxQuickspawnVstCount; ++i)
   {
      ModuleFactory::Spawnable spawnable{};
      auto& pluginDesc = matchingVsts[i];
      spawnable.mLabel = pluginDesc.name.toStdString();
      spawnable.mDecorator = "[" + ModuleFactory::Spawnable::GetPluginLabel(pluginDesc) + "]";
      spawnable.mPluginDesc = pluginDesc;
      spawnable.mSpawnMethod = SpawnMethod::Plugin;
      modules.push_back(spawnable);
   }

   std::vector<Spawnable> prefabs;
   ModuleFactory::GetPrefabs(prefabs);
   for (auto prefab : prefabs)
   {
      if (CheckHeldKeysMatch(prefab.mLabel, keys, continuousString) || keys[0] == ';')
         modules.push_back(prefab);
   }

   std::vector<std::string> midicontrollers = MidiController::GetAvailableInputDevices();
   for (auto midicontroller : midicontrollers)
   {
      if (CheckHeldKeysMatch(midicontroller, keys, continuousString))
      {
         ModuleFactory::Spawnable spawnable{};
         spawnable.mLabel = midicontroller;
         spawnable.mDecorator = kMidiControllerSuffix;
         spawnable.mSpawnMethod = SpawnMethod::MidiController;
         modules.push_back(spawnable);
      }
   }

   std::vector<std::string> effects = TheSynth->GetEffectFactory()->GetSpawnableEffects();
   for (auto effect : effects)
   {
      if (CheckHeldKeysMatch(effect, keys, continuousString))
      {
         ModuleFactory::Spawnable spawnable{};
         spawnable.mLabel = effect;
         spawnable.mDecorator = kEffectChainSuffix;
         spawnable.mSpawnMethod = SpawnMethod::EffectChain;
         modules.push_back(spawnable);
      }
   }

   std::vector<Spawnable> presets;
   ModuleFactory::GetPresets(presets);
   for (auto preset : presets)
   {
      if (CheckHeldKeysMatch(preset.mLabel, keys, continuousString) || keys[0] == ';')
         modules.push_back(preset);
   }

   sort(modules.begin(), modules.end(), Spawnable::CompareAlphabetical);

   std::vector<ModuleFactory::Spawnable> ret;
   for (size_t i = 0; i < modules.size(); ++i)
      ret.push_back(modules[i]);

   return ret;
}

ModuleCategory ModuleFactory::GetModuleCategory(std::string typeName)
{
   if (mFactoryMap.find(typeName) != mFactoryMap.end())
      return mFactoryMap[typeName].mCategory;
   if (juce::String(typeName).endsWith(kPluginSuffix))
      return kModuleCategory_Synth;
   if (juce::String(typeName).endsWith(kMidiControllerSuffix))
      return kModuleCategory_Instrument;
   if (juce::String(typeName).endsWith(kEffectChainSuffix))
      return kModuleCategory_Audio;
   return kModuleCategory_Other;
}

ModuleCategory ModuleFactory::GetModuleCategory(Spawnable spawnable)
{
   if (spawnable.mSpawnMethod == SpawnMethod::Module && mFactoryMap.find(spawnable.mLabel) != mFactoryMap.end())
      return mFactoryMap[spawnable.mLabel].mCategory;
   if (spawnable.mSpawnMethod == SpawnMethod::Plugin)
      return kModuleCategory_Synth;
   if (spawnable.mSpawnMethod == SpawnMethod::MidiController)
      return kModuleCategory_Instrument;
   if (spawnable.mSpawnMethod == SpawnMethod::EffectChain)
      return kModuleCategory_Audio;
   if (spawnable.mSpawnMethod == SpawnMethod::Prefab)
      return kModuleCategory_Other;
   if (spawnable.mSpawnMethod == SpawnMethod::Preset)
      return mFactoryMap[spawnable.mPresetModuleType].mCategory;
   return kModuleCategory_Other;
}

ModuleFactory::ModuleInfo ModuleFactory::GetModuleInfo(std::string typeName)
{
   if (mFactoryMap.find(typeName) != mFactoryMap.end())
      return mFactoryMap[typeName];
   return ModuleInfo();
}

bool ModuleFactory::IsExperimental(std::string typeName)
{
   if (mFactoryMap.find(typeName) != mFactoryMap.end())
      return mFactoryMap[typeName].mIsExperimental;
   return false;
}

//static
void ModuleFactory::GetPrefabs(std::vector<ModuleFactory::Spawnable>& prefabs)
{
   using namespace juce;
   File dir(ofToDataPath("prefabs"));
   Array<File> files;
   dir.findChildFiles(files, File::findFiles, false);
   for (auto file : files)
   {
      if (file.getFileExtension() == ".pfb")
      {
         ModuleFactory::Spawnable spawnable;
         spawnable.mLabel = file.getFileName().toStdString();
         spawnable.mDecorator = kPrefabSuffix;
         spawnable.mSpawnMethod = SpawnMethod::Prefab;
         prefabs.push_back(spawnable);
      }
   }
}

//static
void ModuleFactory::GetPresets(std::vector<ModuleFactory::Spawnable>& presets)
{
   using namespace juce;
   File dir(ofToDataPath("presets"));
   Array<File> directories;
   dir.findChildFiles(directories, File::findDirectories, false);
   for (auto moduleDir : directories)
   {
      std::string moduleTypeName = moduleDir.getFileName().toStdString();
      Array<File> files;
      moduleDir.findChildFiles(files, File::findFiles, false);
      for (auto file : files)
      {
         if (file.getFileExtension() == ".preset")
         {
            ModuleFactory::Spawnable spawnable;
            spawnable.mLabel = file.getFileName().toStdString();
            spawnable.mDecorator = "[" + moduleTypeName + "]";
            spawnable.mPresetModuleType = moduleTypeName;
            spawnable.mSpawnMethod = SpawnMethod::Preset;
            presets.push_back(spawnable);
         }
      }
   }
}

//static
std::string ModuleFactory::FixUpTypeName(std::string name)
{
   if (name == "siggen")
      return "signalgenerator";

   if (name == "eqmodule")
      return "eq";

   if (name == "bandvocoder")
      return "vocoder";

   if (name == "vstplugin")
      return "plugin";

   if (name == "presets")
      return "snapshots";

   if (name == "arpsequencer")
      return "rhythmsequencer";

   return name;
}
