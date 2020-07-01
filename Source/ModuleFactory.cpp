//
//  ModuleFactory.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#include "ModuleFactory.h"

#include "LaunchpadKeyboard.h"
#include "Scale.h"
#include "DrumPlayer.h"
#include "EffectChain.h"
#include "LooperRecorder.h"
#include "Chorder.h"
#include "Transport.h"
#include "Arpeggiator.h"
#include "Razor.h"
#include "Monophonify.h"
#include "StepSequencer.h"
#include "InputChannel.h"
#include "OutputChannel.h"
#include "WaveformViewer.h"
#include "FMSynth.h"
#include "MidiController.h"
#ifdef BESPOKE_MAC
#include "PSMoveController.h"
#endif
#include "SampleBank.h"
#include "SampleEditor.h"
#include "Autotalent.h"
#include "ScaleDetect.h"
#include "KarplusStrong.h"
#include "WhiteKeys.h"
#include "Kicker.h"
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
#include "Presets.h"
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
#include "NoteTransformer.h"
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

#define REGISTER(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, false, false);
#define REGISTER_HIDDEN(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, true, false);
#define REGISTER_EXPERIMENTAL(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, false, true);

ModuleFactory::ModuleFactory()
{
   REGISTER(LooperRecorder, looperrecorder, kModuleType_Audio);
   REGISTER(WaveformViewer, waveform, kModuleType_Audio);
   REGISTER(EffectChain, effectchain, kModuleType_Audio);
   REGISTER(DrumPlayer, drumplayer, kModuleType_Synth);
   REGISTER(Chorder, chorder, kModuleType_Note);
   REGISTER(Arpeggiator, arpeggiator, kModuleType_Note);
   REGISTER(Monophonify, portamento, kModuleType_Note);
   REGISTER(StepSequencer, drumsequencer, kModuleType_Instrument);
   REGISTER(LaunchpadKeyboard, gridkeyboard, kModuleType_Instrument);
   REGISTER(FMSynth, fmsynth, kModuleType_Synth);
   REGISTER(MidiController, midicontroller, kModuleType_Instrument);
#ifdef BESPOKE_MAC
   REGISTER(PSMoveController, psmove, kModuleType_Other);
#endif
   REGISTER(SampleBank, samplebank, kModuleType_Other);
   REGISTER(SampleEditor, sampleeditor, kModuleType_Synth);
   REGISTER(Autotalent, autotalent, kModuleType_Audio);
   REGISTER(ScaleDetect, scaledetect, kModuleType_Note);
   REGISTER(KarplusStrong, karplusstrong, kModuleType_Synth);
   REGISTER(WhiteKeys, whitekeys, kModuleType_Note);
   REGISTER(Kicker, kicker, kModuleType_Note);
   REGISTER(RingModulator, ringmodulator, kModuleType_Audio);
   REGISTER(Neighborhooder, notewrap, kModuleType_Note);
   REGISTER(Polyrhythms, polyrhythms, kModuleType_Instrument);
   REGISTER(Looper, looper, kModuleType_Audio);
   REGISTER(Rewriter, rewriter, kModuleType_Audio);
   REGISTER(Metronome, metronome, kModuleType_Synth);
   REGISTER(NoteRouter, noterouter, kModuleType_Note);
   REGISTER(AudioRouter, audiorouter, kModuleType_Audio);
   REGISTER(LaunchpadNoteDisplayer, gridnotedisplayer, kModuleType_Note);
   REGISTER(Vocoder, vocoder, kModuleType_Audio);
   REGISTER(FreqDelay, freqdelay, kModuleType_Audio);
   REGISTER(VelocitySetter, velocitysetter, kModuleType_Note);
   REGISTER(NoteSinger, notesinger, kModuleType_Synth);
   REGISTER(NoteOctaver, noteoctaver, kModuleType_Note);
   REGISTER(FourOnTheFloor, fouronthefloor, kModuleType_Instrument);
   REGISTER(Amplifier, gain, kModuleType_Audio);
   REGISTER(Presets, presets, kModuleType_Other);
   REGISTER(NoteStepSequencer, notesequencer, kModuleType_Instrument);
   REGISTER(SingleOscillator, oscillator, kModuleType_Synth);
   REGISTER(BandVocoder, bandvocoder, kModuleType_Audio);
   REGISTER(Capo, capo, kModuleType_Note);
   REGISTER(VocoderCarrierInput, vocodercarrier, kModuleType_Audio);
   REGISTER(InputChannel, input, kModuleType_Audio);
   REGISTER(OutputChannel, output, kModuleType_Audio);
   //REGISTER(Eigenharp, eigenharp, kModuleType_Synth);
   REGISTER(Beats, beats, kModuleType_Synth);
   REGISTER(Sampler, sampler, kModuleType_Synth);
   REGISTER(NoteTransformer, notetransformer, kModuleType_Note);
   REGISTER(SliderSequencer, slidersequencer, kModuleType_Instrument);
   REGISTER(VelocityStepSequencer, velocitystepsequencer, kModuleType_Note);
   REGISTER(SustainPedal, sustainpedal, kModuleType_Note);
   REGISTER(SamplerGrid, samplergrid, kModuleType_Audio);
   REGISTER(SignalGenerator, signalgenerator, kModuleType_Synth);
   REGISTER(Lissajous, lissajous, kModuleType_Audio);
   REGISTER(TimerDisplay, timerdisplay, kModuleType_Other);
   REGISTER(DrumSynth, drumsynth, kModuleType_Synth);
   //REGISTER(EigenChorder, eigenchorder, kModuleType_Note);
   REGISTER(PitchBender, pitchbender, kModuleType_Note);
   //REGISTER(EigenharpNoteSource, eigenharpnotesource, kModuleType_Instrument);
   REGISTER(LFOController, lfocontroller, kModuleType_Other);
   REGISTER(VinylTempoControl, vinylcontrol, kModuleType_Other);
   REGISTER(NoteFlusher, noteflusher, kModuleType_Note);
   REGISTER(NoteCanvas, notecanvas, kModuleType_Instrument);
   REGISTER(CommentDisplay, comment, kModuleType_Other);
   REGISTER(StutterControl, stutter, kModuleType_Audio);
   REGISTER(CircleSequencer, circlesequencer, kModuleType_Instrument);
   REGISTER(MidiOutputModule, midioutput, kModuleType_Note);
   REGISTER(NoteDisplayer, notedisplayer, kModuleType_Note);
   REGISTER(AudioMeter, audiometer, kModuleType_Audio);
   REGISTER(NoteSustain, notesustain, kModuleType_Note);
   REGISTER(ControlSequencer, controlsequencer, kModuleType_Modulator);
   REGISTER(PitchSetter, pitchsetter, kModuleType_Note);
   REGISTER(NoteFilter, notefilter, kModuleType_Note);
   REGISTER(RandomNoteGenerator, randomnote, kModuleType_Instrument);
   REGISTER(NoteToFreq, notetofreq, kModuleType_Modulator);
   REGISTER(MacroSlider, macroslider, kModuleType_Modulator);
   REGISTER(NoteVibrato, vibrato, kModuleType_Note);
   REGISTER(ModulationVisualizer, modulationvizualizer, kModuleType_Note);
   REGISTER(PitchDive, pitchdive, kModuleType_Note);
   REGISTER(EventCanvas, eventcanvas, kModuleType_Other);
   REGISTER(NoteCreator, notecreator, kModuleType_Instrument);
   REGISTER(ValueSetter, valuesetter, kModuleType_Modulator);
   REGISTER(PreviousNote, previousnote, kModuleType_Note);
   REGISTER(PressureToVibrato, pressuretovibrato, kModuleType_Note);
   REGISTER(ModwheelToVibrato, modwheeltovibrato, kModuleType_Note);
   REGISTER(Pressure, pressure, kModuleType_Note);
   REGISTER(ModWheel, modwheel, kModuleType_Note);
   REGISTER(PressureToModwheel, pressuretomodwheel, kModuleType_Note);
   REGISTER(ModwheelToPressure, modwheeltopressure, kModuleType_Note);
   REGISTER(FeedbackModule, feedback, kModuleType_Audio);
   REGISTER(NoteToMs, notetoms, kModuleType_Modulator);
   REGISTER(Selector, selector, kModuleType_Other);
   REGISTER(GroupControl, groupcontrol, kModuleType_Other);
   REGISTER(CurveLooper, curvelooper, kModuleType_Modulator);
   REGISTER(ScaleDegree, scaledegree, kModuleType_Note);
   REGISTER(NoteChainNode, notechain, kModuleType_Instrument);
   REGISTER(NoteDelayer, notedelayer, kModuleType_Note);
   REGISTER(TimelineControl, timelinecontrol, kModuleType_Other);
   REGISTER(VelocityScaler, velocityscaler, kModuleType_Note);
   REGISTER(KeyboardDisplay, keyboarddisplay, kModuleType_Instrument);
   REGISTER(Ramper, ramper, kModuleType_Modulator);
   REGISTER(NoteGate, notegate, kModuleType_Note);
   REGISTER(Prefab, prefab, kModuleType_Other);
   REGISTER(NoteHumanizer, notehumanizer, kModuleType_Note);
   REGISTER(VolcaBeatsControl, volcabeatscontrol, kModuleType_Note);
   REGISTER(RadioSequencer, radiosequencer, kModuleType_Other);
   REGISTER(TakeRecorder, takerecorder, kModuleType_Audio);
   REGISTER(Splitter, splitter, kModuleType_Audio);
   REGISTER(Panner, panner, kModuleType_Audio);
   REGISTER(SamplePlayer, sampleplayer, kModuleType_Synth);
   REGISTER(AudioSend, send, kModuleType_Audio);
   REGISTER(EnvelopeModulator, envelope, kModuleType_Modulator);
   REGISTER(AudioToCV, audiotocv, kModuleType_Modulator);
   REGISTER(ModulatorAdd, add, kModuleType_Modulator);
   REGISTER(ModulatorAddCentered, addcentered, kModuleType_Modulator);
   REGISTER(ModulatorSubtract, subtract, kModuleType_Modulator);
   REGISTER(PitchToCV, pitchtocv, kModuleType_Modulator);
   REGISTER(VelocityToCV, velocitytocv, kModuleType_Modulator);
   REGISTER(PressureToCV, pressuretocv, kModuleType_Modulator);
   REGISTER(ModWheelToCV, modwheeltocv, kModuleType_Modulator);
   REGISTER(ModulatorMult, mult, kModuleType_Modulator);
   REGISTER(ModulatorCurve, curve, kModuleType_Modulator);
   REGISTER(ModulatorSmoother, smoother, kModuleType_Modulator);
   REGISTER(NotePanner, notepanner, kModuleType_Note);
   REGISTER(PitchPanner, pitchpanner, kModuleType_Note);
   REGISTER(NotePanAlternator, notepanalternator, kModuleType_Note);
   REGISTER(ChordDisplayer, chorddisplayer, kModuleType_Note);
   REGISTER(NoteStrummer, notestrummer, kModuleType_Note);
   REGISTER(SeaOfGrain, seaofgrain, kModuleType_Synth);
   REGISTER(PitchToSpeed, pitchtospeed, kModuleType_Modulator);
   REGISTER(NoteToPulse, notetopulse, kModuleType_Other);
   REGISTER(OSCOutput, oscoutput, kModuleType_Other);
   REGISTER(AudioLevelToCV, leveltocv, kModuleType_Modulator);
   REGISTER(Pulser, pulser, kModuleType_Other);
   REGISTER(PulseSequence, pulsesequence, kModuleType_Other);
   REGISTER(LinnstrumentControl, linnstrumentcontrol, kModuleType_Note);
   REGISTER(MultitapDelay, multitapdelay, kModuleType_Audio);
   REGISTER(MidiCapturer, midicapturer, kModuleType_Note);
   REGISTER(Inverter, inverter, kModuleType_Audio);
   REGISTER(SpectralDisplay, spectrum, kModuleType_Audio);
   REGISTER(DCOffset, dcoffset, kModuleType_Audio);
   REGISTER(SignalClamp, signalclamp, kModuleType_Audio);
   REGISTER(Waveshaper, waveshaper, kModuleType_Audio);
   REGISTER(NoteHocket, notehocket, kModuleType_Note);
   REGISTER(NoteRangeFilter, noterangefilter, kModuleType_Note);
   REGISTER(NoteChance, notechance, kModuleType_Note);
   REGISTER(PulseChance, pulsechance, kModuleType_Other);
   REGISTER(PulseDelayer, pulsedelayer, kModuleType_Other);
   REGISTER(NotePanRandom, notepanrandom, kModuleType_Note);
   REGISTER(PulseGate, pulsegate, kModuleType_Other);
   REGISTER(PulseHocket, pulsehocket, kModuleType_Other);
   REGISTER(Push2Control, push2control, kModuleType_Instrument);
   REGISTER(PulseTrain, pulsetrain, kModuleType_Other);
   REGISTER(NoteLatch, notelatch, kModuleType_Note);
   REGISTER(ScriptModule, script, kModuleType_Other);
   REGISTER(ScriptStatus, scriptstatus, kModuleType_Other);
   REGISTER(ModulatorGravity, gravity, kModuleType_Modulator);
   REGISTER(NoteStreamDisplay, notestream, kModuleType_Note);
   REGISTER(PulseButton, pulsebutton, kModuleType_Other);

   //REGISTER_EXPERIMENTAL(MidiPlayer, midiplayer, kModuleType_Instrument);
   REGISTER_EXPERIMENTAL(Razor, razor, kModuleType_Synth);
   REGISTER_EXPERIMENTAL(SampleCanvas, samplecanvas, kModuleType_Synth);
   REGISTER_EXPERIMENTAL(LoopStorer, loopstorer, kModuleType_Other);
   REGISTER_EXPERIMENTAL(ComboGridController, combogrid, kModuleType_Other);
   REGISTER_EXPERIMENTAL(PitchChorus, pitchchorus, kModuleType_Audio);

   REGISTER_HIDDEN(VSTPlugin, vstplugin, kModuleType_Synth);
   REGISTER_HIDDEN(SampleFinder, samplefinder, kModuleType_Audio);
   REGISTER_HIDDEN(Producer, producer, kModuleType_Audio);
   REGISTER_HIDDEN(ChaosEngine, chaosengine, kModuleType_Other);
   REGISTER_HIDDEN(MultibandCompressor, multiband, kModuleType_Audio);
   REGISTER_HIDDEN(ControllingSong, controllingsong, kModuleType_Synth);
   REGISTER_HIDDEN(PanicButton, panicbutton, kModuleType_Other);
   REGISTER_HIDDEN(MultitrackRecorder, multitrackrecorder, kModuleType_Audio);
   REGISTER_HIDDEN(DebugAudioSource, debugaudiosource, kModuleType_Synth);
   REGISTER_HIDDEN(FollowingSong, followingsong, kModuleType_Synth);
   REGISTER_HIDDEN(BeatBloks, beatbloks, kModuleType_Synth);
   REGISTER_HIDDEN(FilterViz, filterviz, kModuleType_Other);
   REGISTER_HIDDEN(FreqDomainBoilerplate, freqdomainboilerplate, kModuleType_Audio);
   REGISTER_HIDDEN(FFTtoAdditive, ffttoadditive, kModuleType_Audio);
   REGISTER_HIDDEN(SlowLayers, slowlayers, kModuleType_Audio);
   REGISTER_HIDDEN(ClipLauncher, cliplauncher, kModuleType_Synth);
#ifdef BESPOKE_MAC
   REGISTER_HIDDEN(KompleteKontrol, kompletekontrol, kModuleType_Note);
#endif
   REGISTER_HIDDEN(ControlTactileFeedback, controltactilefeedback, kModuleType_Synth);
   REGISTER_HIDDEN(FloatSliderLFOControl, lfo, kModuleType_Other);
   REGISTER_HIDDEN(NoteLooper, notelooper, kModuleType_Note);
   REGISTER_HIDDEN(EnvelopeEditor, envelopeeditor, kModuleType_Other);
}

void ModuleFactory::Register(string type, CreateModuleFn creator, CanCreateModuleFn canCreate, ModuleType moduleType, bool hidden, bool experimental)
{
   mFactoryMap[type] = creator;
   mCanCreateMap[type] = canCreate;
   mModuleTypeMap[type] = moduleType;
   mIsHiddenModuleMap[type] = hidden;
   mIsExperimentalModuleMap[type] = experimental;
}

IDrawableModule* ModuleFactory::MakeModule(string type)
{
   auto canCreate = mCanCreateMap.find(type);
   if (canCreate != mCanCreateMap.end())
   {
      if (canCreate->second())
      {
         auto iter = mFactoryMap.find(type);
         if (iter != mFactoryMap.end())
            return iter->second();
      }
   }
   return nullptr;
}

vector<string> ModuleFactory::GetSpawnableModules(ModuleType moduleType)
{
   vector<string> modules;
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
   {
      if (mModuleTypeMap[iter->first] == moduleType &&
         (mIsHiddenModuleMap[iter->first] == false || gShowDevModules))
         modules.push_back(iter->first);
   }
   sort(modules.begin(), modules.end());
   return modules;
}

vector<string> ModuleFactory::GetSpawnableModules(char c)
{
   vector<string> modules;
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
   {
      if (iter->first[0] == c &&
          (mIsHiddenModuleMap[iter->first] == false || gShowDevModules))
         modules.push_back(iter->first);
   }
   sort(modules.begin(), modules.end());
   return modules;
}

ModuleType ModuleFactory::GetModuleType(string typeName)
{
   if (mModuleTypeMap.find(typeName) != mModuleTypeMap.end())
      return mModuleTypeMap[typeName];
   return kModuleType_Other;
}

bool ModuleFactory::IsExperimental(string typeName)
{
   if (mIsExperimentalModuleMap.find(typeName) != mIsExperimentalModuleMap.end())
      return mIsExperimentalModuleMap[typeName];
   return false;
}
