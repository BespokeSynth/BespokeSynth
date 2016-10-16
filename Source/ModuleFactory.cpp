//
//  ModuleFactory.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#include "ModuleFactory.h"

#include "MidiInstrument.h"
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
#include "Stereofier.h"
#include "WaveformViewer.h"
#include "FMSynth.h"
#include "MidiController.h"
#ifdef BESPOKE_MAC
#include "PSMoveController.h"
#endif
#include "SampleBank.h"
#include "SamplePlayer.h"
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
#include "ADSRTrigger.h"
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
#include "FreeverbOutput.h"
#include "NoteCanvas.h"
#include "SlowLayers.h"
#include "ClipLauncher.h"
#include "LoopStorer.h"
#include "CommentDisplay.h"
#include "GridController.h"
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
#include "GridToDrums.h"
#include "ScaleDegree.h"
#include "NoteSequencerColumn.h"
#include "NoteChainNode.h"
#include "NoteDelayer.h"
#include "TimelineControl.h"
#include "VelocityScaler.h"
#include "KeyboardDisplay.h"
#include "Ramper.h"
#include "NoteGate.h"
#include "Prefab.h"

#define REGISTER(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, false, false);
#define REGISTER_HIDDEN(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, true, false);
#define REGISTER_EXPERIMENTAL(class,name,type) Register(#name, &(class::Create), &(class::CanCreate), type, false, true);

ModuleFactory::ModuleFactory()
{
   REGISTER(Stereofier, stereofier, kModuleType_Audio);
   REGISTER(LooperRecorder, looperrecorder, kModuleType_Audio);
   REGISTER(WaveformViewer, waveform, kModuleType_Audio);
   REGISTER(EffectChain, effectchain, kModuleType_Audio);
   REGISTER(DrumPlayer, drumplayer, kModuleType_Synth);
   REGISTER(Chorder, chorder, kModuleType_Note);
   REGISTER(Arpeggiator, arpeggiator, kModuleType_Note);
   REGISTER(Monophonify, monophonify, kModuleType_Note);
   REGISTER(StepSequencer, drumsequencer, kModuleType_Instrument);
   REGISTER(LaunchpadKeyboard, launchpadkeyboard, kModuleType_Instrument);
   REGISTER(FMSynth, fmsynth, kModuleType_Synth);
   REGISTER(MidiController, midicontroller, kModuleType_Other);
#ifdef BESPOKE_MAC
   REGISTER(PSMoveController, psmove, kModuleType_Other);
#endif
   REGISTER(SampleBank, samplebank, kModuleType_Other);
   REGISTER(SamplePlayer, sampleplayer, kModuleType_Synth);
   REGISTER(Autotalent, autotalent, kModuleType_Audio);
   REGISTER(ScaleDetect, scaledetect, kModuleType_Note);
   REGISTER(KarplusStrong, karplusstrong, kModuleType_Synth);
   REGISTER(MidiInstrument, midiinstrument, kModuleType_Instrument);
   REGISTER(WhiteKeys, whitekeys, kModuleType_Note);
   REGISTER(Kicker, kicker, kModuleType_Note);
   REGISTER(RingModulator, ringmodulator, kModuleType_Audio);
   REGISTER(Neighborhooder, neighborhooder, kModuleType_Note);
   REGISTER(Polyrhythms, polyrhythms, kModuleType_Instrument);
   REGISTER(Looper, looper, kModuleType_Audio);
   REGISTER(Rewriter, rewriter, kModuleType_Audio);
   REGISTER(Metronome, metronome, kModuleType_Synth);
   REGISTER(NoteRouter, noterouter, kModuleType_Note);
   REGISTER(AudioRouter, audiorouter, kModuleType_Audio);
   REGISTER(LaunchpadNoteDisplayer, launchpadnotedisplayer, kModuleType_Note);
   REGISTER(Vocoder, vocoder, kModuleType_Audio);
   REGISTER(FreqDelay, freqdelay, kModuleType_Audio);
   REGISTER(VelocitySetter, velocitysetter, kModuleType_Note);
   REGISTER(NoteSinger, notesinger, kModuleType_Synth);
   REGISTER(NoteOctaver, noteoctaver, kModuleType_Note);
   REGISTER(FourOnTheFloor, fouronthefloor, kModuleType_Instrument);
   REGISTER(Amplifier, amplifier, kModuleType_Audio);
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
   REGISTER(ADSRTrigger, adsrtrigger, kModuleType_Note);
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
   REGISTER(FreeverbOutput, freeverboutput, kModuleType_Other);
   REGISTER(NoteCanvas, notecanvas, kModuleType_Instrument);
   REGISTER(CommentDisplay, comment, kModuleType_Other);
   REGISTER(GridController, gridcontroller, kModuleType_Other);
   REGISTER(StutterControl, stuttercontrol, kModuleType_Other);
   REGISTER(CircleSequencer, circlesequencer, kModuleType_Instrument);
   REGISTER(MidiOutputModule, midioutput, kModuleType_Note);
   REGISTER(NoteDisplayer, notedisplayer, kModuleType_Note);
   REGISTER(AudioMeter, audiometer, kModuleType_Audio);
   REGISTER(NoteSustain, notesustain, kModuleType_Note);
   REGISTER(ControlSequencer, controlsequencer, kModuleType_Other);
   REGISTER(PitchSetter, pitchsetter, kModuleType_Note);
   REGISTER(NoteFilter, notefilter, kModuleType_Note);
   REGISTER(RandomNoteGenerator, randomnote, kModuleType_Instrument);
   REGISTER(NoteToFreq, notetofreq, kModuleType_Note);
   REGISTER(MacroSlider, macroslider, kModuleType_Other);
   REGISTER(NoteVibrato, vibrato, kModuleType_Note);
   REGISTER(ModulationVisualizer, modulationvizualizer, kModuleType_Note);
   REGISTER(PitchDive, pitchdive, kModuleType_Note);
   REGISTER(EventCanvas, eventcanvas, kModuleType_Other);
   REGISTER(NoteCreator, notecreator, kModuleType_Instrument);
   REGISTER(ValueSetter, valuesetter, kModuleType_Note);
   REGISTER(PreviousNote, previousnote, kModuleType_Note);
   REGISTER(PressureToVibrato, pressuretovibrato, kModuleType_Note);
   REGISTER(ModwheelToVibrato, modwheeltovibrato, kModuleType_Note);
   REGISTER(Pressure, pressure, kModuleType_Note);
   REGISTER(ModWheel, modwheel, kModuleType_Note);
   REGISTER(PressureToModwheel, pressuretomodwheel, kModuleType_Note);
   REGISTER(ModwheelToPressure, modwheeltopressure, kModuleType_Note);
   REGISTER(FeedbackModule, feedback, kModuleType_Audio);
   REGISTER(NoteToMs, notetoms, kModuleType_Note);
   REGISTER(Selector, selector, kModuleType_Other);
   REGISTER(GroupControl, groupcontrol, kModuleType_Other);
   REGISTER(CurveLooper, curvelooper, kModuleType_Other);
   REGISTER(GridToDrums, gridtodrums, kModuleType_Instrument);
   REGISTER(ScaleDegree, scaledegree, kModuleType_Note);
   REGISTER(NoteSequencerColumn, notesequencercolumn, kModuleType_Other);
   REGISTER(NoteChainNode, notechain, kModuleType_Instrument);
   REGISTER(NoteDelayer, notedelayer, kModuleType_Note);
   REGISTER(TimelineControl, timelinecontrol, kModuleType_Other);
   REGISTER(VelocityScaler, velocityscaler, kModuleType_Note);
   REGISTER(KeyboardDisplay, keyboarddisplay, kModuleType_Instrument);
   REGISTER(Ramper, ramper, kModuleType_Other);
   REGISTER(NoteGate, notegate, kModuleType_Note);
   REGISTER(Prefab, prefab, kModuleType_Other);

   REGISTER_EXPERIMENTAL(VSTPlugin, vstplugin, kModuleType_Synth);
   //REGISTER_EXPERIMENTAL(MidiPlayer, midiplayer, kModuleType_Instrument);
   REGISTER_EXPERIMENTAL(Razor, razor, kModuleType_Synth);
   REGISTER_EXPERIMENTAL(SampleCanvas, samplecanvas, kModuleType_Synth);
   REGISTER_EXPERIMENTAL(LoopStorer, loopstorer, kModuleType_Other);
   REGISTER_EXPERIMENTAL(ComboGridController, combogrid, kModuleType_Other);
   REGISTER_EXPERIMENTAL(PitchChorus, pitchchorus, kModuleType_Audio);

   REGISTER_HIDDEN(SampleFinder, samplefinder, kModuleType_Audio);
   REGISTER_HIDDEN(Producer, producer, kModuleType_Audio);
   REGISTER_HIDDEN(ChaosEngine, chaosengine, kModuleType_Other);
   REGISTER_HIDDEN(MultibandCompressor, multiband, kModuleType_Audio);
   REGISTER_HIDDEN(ControllingSong, controllingsong, kModuleType_Synth);
   REGISTER_HIDDEN(PanicButton, panicbutton, kModuleType_Other);
   REGISTER_HIDDEN(MultitrackRecorder, multitrackrecorder, kModuleType_Audio);
   REGISTER_HIDDEN(DebugAudioSource, debugaudiosource, kModuleType_Synth);
   REGISTER_HIDDEN(FollowingSong, followingsong, kModuleType_Synth);
   REGISTER_HIDDEN(SeaOfGrain, seaofgrain, kModuleType_Synth);
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
   return NULL;
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
