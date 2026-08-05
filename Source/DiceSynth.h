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
//  DiceSynth.h
//  Bespoke
//
//  Drop a sample in, then click the big die to mold it. Two engines, chosen by the mold mode at the
//  top of the module:
//    effects  - bakes a random handful of deliberate transformations into the sample (the die's face
//               value is how many stack). Each has its own recognisable character, so results are
//               varied and reproducible, and it works on anything including full mixes.
//    resynth  - analyses pitch/harmonic content and mutates a reconstruction, Synplant-style. Only
//               meaningful on monophonic tonal material: a polyphonic source has no single
//               fundamental to model, so the harmonic path gates itself off and it becomes an
//               expensive passthrough.
//  Either way the result feeds an XY pad that plays around with it live: ten effects (distortion,
//  pitch, reverb, freq shift, delay, stutter, bitcrush, filter, repeater, reverse) sit at fixed
//  spots around the pad, and whichever ones are nearest your position blend in.
//

#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Sample.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ChannelBuffer.h"
#include <array>
#include <vector>

class DiceSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener
{
public:
   DiceSynth();
   ~DiceSynth();
   static IDrawableModule* Create() { return new DiceSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override { }

   //IDrawableModule
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void Poll() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 10; }

   static const int kNumLiveEffects = 10;

   enum LiveEffect
   {
      kFxLive_Filter = 0,
      kFxLive_Comb,
      kFxLive_Chorus,
      kFxLive_Pitch,
      kFxLive_FreqShift,
      kFxLive_Delay,
      kFxLive_Reverb,
      kFxLive_Stutter,
      kFxLive_Repeater,
      kFxLive_Reverse
   };

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   //Two ways to mold, because they suit opposite material and neither is a superset of the other.
   //Effects mode BAKES a random handful of deliberate transformations into the sample - each has its
   //own recognisable character, so results are varied and intentional, and it works on anything
   //including full mixes. Resynth mode analyses pitch/harmonics and mutates a reconstruction, which
   //only means anything on monophonic tonal material; on a polyphonic source there is no single
   //fundamental to model, so it degenerates into an expensive passthrough.
   enum MoldMode
   {
      kMold_Effects = 0,
      kMold_Resynth
   };

   //Feed the molded output back in as the new source and mold again. Iterating a transformation on
   //its own output is where emergent behaviour lives - the Lucier "I Am Sitting In A Room" trick.
   //In resynth mode this is DETERMINISTIC: the genome is held fixed and only the re-analysis changes,
   //so the synthesis model's own character progressively takes over from the source material.
   //In effects mode each generation draws fresh effects, which compounds dub-tape-delay style.
   void IterateMold();
   void ResetToOriginal();

   void RollDice(); //dispatches on the current mold mode
   void RollDiceEffects();
   void RollDiceResynth();
   //shared tail: peak-normalize and publish into mMoldedSample without reallocating under the audio thread
   void WriteMolded(const std::vector<float>& buf, const std::string& name);
   //Allocate the molded buffer ONCE per dropped sample, at the largest size the mold could ever
   //need. After that the pointer never moves, so the audio thread can never be left holding a
   //freed buffer - which is what dragging 'mold sec' during playback used to do.
   void EnsureMoldedCapacity();
   int MoldedCapacity() const; //largest mold this source could produce - effects mode uses all of it

   //offline bake transforms - whole buffer, in place
   void BakeFilter(std::vector<float>& buf, float cutoffHz);
   void BakeComb(std::vector<float>& buf, float pitchHz, float feedback);
   void BakeChorus(std::vector<float>& buf, float rateHz, float depthMs);
   void BakePitchShift(std::vector<float>& buf, float semitones);
   void BakeFreqShift(std::vector<float>& buf, float shiftHz);
   void BakeDelay(std::vector<float>& buf, int delaySamples, float feedback);
   void BakeReverb(std::vector<float>& buf, float feedback);
   void BakeStutter(std::vector<float>& buf, int stutterLenSamples);
   void BakeRepeater(std::vector<float>& buf, int segmentLenSamples);
   void BakeReverse(std::vector<float>& buf);
   void SeekWaveformTo(float x); //click/drag position (module-local x) on the waveform view -> seek + play
   void ResetLiveEffectState(); //clear all the live DSP history buffers (called on dice roll / new sample)
   float ComputeEffectWeight(int effectIndex) const; //proximity of mXYPos to that effect's anchor point, 0-1
   void RandomizeEffects(); //reshuffle every effect's pad position and how extreme it gets at full strength

   //---- analysis + genetic resynthesis (Synplant-style) ----
   static const int kNumHarmonics = 48; //covers up to ~48x the fundamental - a one-time analysis/render, so the extra cost is trivial
   static const int kNumNoiseBands = 20; //EQ bands the genome uses to reshape the residual (it is NOT resynthesized from these - see residual below)
   static const int kAnalysisFrameSize = 2048;
   static const int kAnalysisHop = 256;

   struct SampleAnalysis
   {
      bool valid{ false };
      float f0{ 110.0f }; //estimated fundamental, Hz (seed for the per-frame track below)
      std::vector<float> f0Envelope; //[frame] fundamental over time - percussion often sweeps (a kick drops in pitch as it decays), which a single fixed f0 can't represent
      std::vector<float> voicing; //[frame] 0-1 periodicity confidence. Gates the harmonic path so unpitched material (hat, clap, noise) never gets 48 fake sines built on a meaningless autocorrelation peak
      int numFrames{ 0 };
      float frameDurationSec{ 0.0f };
      int peakFrame{ 0 }; //loudest frame - splits the render into an "attack" and "decay" half for time-warping
      std::array<std::vector<float>, kNumHarmonics> harmonicEnvelope; //[harmonic][frame], already scaled by that frame's voicing
      //loudest value each partial ever reaches. On polyphonic input voicing is ~0 everywhere so
      //these are all ~0 - checking them lets the render skip 48 sine oscillators that would only
      //ever output silence, which is the difference between a slow and an instant roll on a full song
      std::array<float, kNumHarmonics> harmonicPeak{ };
      //The ACTUAL leftover waveform (original minus the harmonics that were claimed), recovered by
      //inverse-FFT with the original phases intact. Keeping the real signal is what lets a click stay
      //a click: amplitude-per-band synthesis can reproduce a transient's spectrum but never its
      //phase alignment, so an impulse always came back out as a burst of noise.
      std::vector<float> residual;
      float harmonicity{ 0.0f }; //0-1, energy-weighted mean voicing (guitar high, clap low) - shown in the UI
      float globalVoiced{ 0.0f }; //YIN's confidence for the sample as a whole; below a floor the harmonic path is switched off entirely (hat, clap, field recording)
   };

   struct HarmonicGenome
   {
      float harmonicAmp[kNumHarmonics]; //multiplier per partial; 1.0 = matches the analysis as-is
      float noiseBandAmp[kNumNoiseBands]; //EQ gain per band applied to the residual; 1.0 = untouched. Reshaping this is what gives percussive samples somewhere interesting to evolve
      float noiseAmount{ 1.0f }; //residual level; 1.0 = the sample's own natural amount
      float baselineNoiseAmount{ 1.0f }; //mutation pulls back toward this instead of drifting freely
      float attackScale{ 1.0f }; //>1 slower attack, <1 faster
      float decayScale{ 1.0f };
      float brightnessTilt{ 0.0f }; //-1..1, extra darken/brighten on top of the analyzed shape
      float inharmonicity{ 0.0f }; //0..1, detunes upper partials for bell/electric-piano-ish stretch
      float pitchShiftSemitones{ 0.0f }; //cumulative transposition away from the analyzed f0 - the single biggest "different sound" lever
      //partial spacing exponent: freq = f0 * (h+1)^stretch. 1.0 is a normal harmonic series; away
      //from it the series becomes genuinely inharmonic (bell, gong, detuned metal) rather than just
      //slightly detuned, which is the strongest single timbral transform available here
      float harmonicStretch{ 1.0f };
      bool reverseResidual{ false }; //play the residual backwards - turns a cymbal/clap into a reverse swell
   };

   static float NoiseBandEdgeHz(int edgeIndex); //log-spaced band edges for the residual EQ
   //YIN pitch detector. Returns Hz, and reports how periodic the material actually is via
   //outConfidence - plain autocorrelation is biased toward the shortest lag on bass-heavy signals
   //and cannot be trusted for either value.
   float EstimatePitch(const float* data, int len, float sampleRate, float* outConfidence = nullptr) const;
   //normalized autocorrelation at one lag = "how periodic is this frame", 0-1. Cheap (single lag),
   //and it is the value that decides whether the harmonic path is allowed to contribute at all.
   float ComputeVoicing(const float* data, int totalLen, int start, int frameLen, float f0) const;
   void AnalyzeSample(); //STFT + harmonic/noise tracking of mSourceSample -> mAnalysis
   void InitGenomeFromAnalysis(); //resets mGenome to the un-mutated baseline
   void MutateGenome(int rollStrength); //one generation step - rollStrength (1-6) sets mutation intensity
   void RenderFromGenome(); //additive resynthesis from mAnalysis + mGenome -> mMoldedSample

   SampleAnalysis mAnalysis;
   HarmonicGenome mGenome;
   bool mNeedsReanalysis{ true }; //mAnalysis isn't persisted (it's large and fully derivable from mSourceSample) - set whenever a new sample is dropped in, or right after loading a save file
   bool mGenomeValid{ false }; //false only for a genuinely new sample - keeps a loaded/mutated genome from being reset back to baseline the next time analysis re-runs after a reload

   //---- source ----
   Sample mOriginalSample; //the pristine drop, kept so iteration can always be undone
   Sample mSourceSample; //what the mold currently reads from - becomes the previous output when iterating
   bool mHasDroppedSample{ false };
   int mGeneration{ 0 }; //how many times the output has been fed back in
   ClickButton* mIterateButton{ nullptr };
   ClickButton* mResetButton{ nullptr };

   //---- dice / molded output ----
   Sample mMoldedSample; //the "output sample" the dice roll produces - allocated at capacity, see mMoldedLength
   int mMoldedLength{ 0 }; //how much of mMoldedSample is actually valid audio (the buffer itself is over-allocated)
   bool mHasMolded{ false };
   int mLastRoll{ 0 }; //1-6, shown on the die face; also sets this roll's mutation strength

   //---- playback ----
   bool mPlaying{ false };
   bool mLoop{ true };
   Checkbox* mLoopCheckbox{ nullptr };
   ClickButton* mPlayButton{ nullptr };
   ClickButton* mStopButton{ nullptr };
   double mPlayPos{ 0 }; //position in mMoldedSample, in source samples
   float mPitchRatio{ 1.0f };
   int mRootPitch{ 60 };
   float mGain{ 1.0f };
   FloatSlider* mGainSlider{ nullptr };
   float mVoiceGain{ 0.0f }; //smoothed toward (mPlaying ? mGain : 0) each buffer, to avoid clicks
   float mFxMix{ 1.0f }; //master wet/dry for the whole live 10-effect XY chain, independent of pad position
   FloatSlider* mFxMixSlider{ nullptr };

   //Global chaos macro. One dial for "how far from the original sample should everything push":
   //it scales how many effects stack and how extreme each one is, how hard the resynth genome
   //mutates, and how wide 'randomize fx' throws its ranges. At 0 every engine stays close to the
   //source; at 1 they all go for it.
   float mChaos{ 0.5f };
   FloatSlider* mChaosSlider{ nullptr };

   int mMoldMode{ kMold_Effects };
   DropdownList* mMoldModeSelector{ nullptr };
   std::string mMoldDescription; //which effects this roll used, shown above the waveform

   //---- XY pad ----
   float mXYPos[2]{ 0.5f, 0.5f };
   bool mDraggingXY{ false };
   bool mScrubbingWaveform{ false }; //click/drag on the waveform view to seek playback

   //draggable play region. Playback and looping stay inside these two markers, so a section of a
   //long track can be isolated without re-molding it.
   float mRegionStart{ 0.0f }; //0-1 fraction of the molded sample
   float mRegionEnd{ 1.0f };
   int mDragMarker{ 0 }; //0 none, 1 start, 2 end

   //where each effect sits on the pad - starts on an even circle, but "randomize fx" scatters these
   struct EffectSpot
   {
      float x{ 0.5f };
      float y{ 0.5f };
   };
   EffectSpot mEffectSpots[kNumLiveEffects];
   ClickButton* mRandomizeEffectsButton{ nullptr };

   //---- XY path record/replay - like SampleUniverse's recorded path, but sampled once per audio
   //---- buffer instead of transport-quantized, so record and replay stay lock-in-step with no
   //---- timestamps needed ----
   struct PathPoint
   {
      float x{ 0.5f };
      float y{ 0.5f };
   };
   static const int kMaxPathPoints = 20000; //generous cap so a forgotten recording can't grow forever
   bool mRecordingPath{ false };
   bool mPlayingPath{ false };
   bool mLoopPath{ true }; //if false, playback stops after one pass instead of looping
   std::vector<PathPoint> mRecordedPath;
   double mPathPlaybackPhase{ 0.0 }; //in "buffer steps" through the (possibly quantized) loop length
   Checkbox* mRecordPathCheckbox{ nullptr };
   Checkbox* mPlayPathCheckbox{ nullptr };
   Checkbox* mLoopPathCheckbox{ nullptr };

   //quantize: stretch/compress the recorded path's playback duration to the nearest musical
   //division, same idea as FubbleModule's quantized curve length
   bool mQuantizePath{ false };
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_4n };
   Checkbox* mQuantizePathCheckbox{ nullptr };
   DropdownList* mQuantizeIntervalSelector{ nullptr };

   //---- per-effect "how extreme does it get at full strength" - randomized together with the pad
   //---- layout by "randomize fx"; these are the knobs behind the numbers described to the user ----
   float mFilterMinCutoffHz{ 200.0f }; //cutoff at full weight; at zero weight it's always ~7200Hz (off)
   float mCombMaxFeedback{ 0.85f }; //tuned comb resonance at full weight - metallic ringing pitched by position
   float mChorusMaxDepthMs{ 8.0f }; //chorus modulation depth at full weight - movement and width
   float mPitchMaxRateBoost{ 1.0f }; //extra playback-rate at full weight (1.0 = up to +1 octave)
   float mFreqShiftMaxHz{ 300.0f }; //ring-mod shift amount at full weight
   float mDelayMaxTimeMs{ 50.0f };
   float mDelayMaxFeedback{ 0.8f };
   float mReverbMaxFeedback{ 0.7f };
   float mStutterMinLenMs{ 6.0f }; //chunk length at full weight (shorter = more rapid-fire)
   float mRepeaterMinLenMs{ 23.0f }; //segment length at full weight (shorter = more chaotic)
   float mReverseMaxWindowMs{ 90.0f }; //how far back the reverse smear reaches at full weight

   //---- live effect DSP state (stereo) ----
   float mFilterLow[2]{ };
   float mFilterBand[2]{ };

   //comb resonator: a short feedback delay tuned to a pitch, which rings metallically
   static const int kCombBufSize = 2048;
   std::vector<float> mCombBuf[2];
   int mCombWritePos{ 0 };

   //chorus: a short delay whose length is swept by an LFO, giving movement and width
   static const int kChorusBufSize = 4096;
   std::vector<float> mChorusBuf[2];
   int mChorusWritePos{ 0 };
   float mChorusPhase{ 0.0f };

   static const int kPitchBufSize = 8192;
   std::vector<float> mPitchBuf[2];
   int mPitchWritePos{ 0 };
   float mPitchReadPos[2]{ 0, 0 };

   float mFreqShiftPhase{ 0.0f };

   static const int kDelayBufSize = 88200;
   std::vector<float> mDelayBuf[2];
   int mDelayWritePos{ 0 };

   static const int kRevDel1 = 1411;
   static const int kRevDel2 = 1787;
   static const int kRevDel3 = 2111;
   std::vector<float> mRevBuf1[2], mRevBuf2[2], mRevBuf3[2];
   int mRevIdx1{ 0 }, mRevIdx2{ 0 }, mRevIdx3{ 0 };

   static const int kStutterBufSize = 44100;
   std::vector<float> mStutterBuf[2];
   int mStutterWritePos{ 0 };
   int mStutterReadPos{ 0 };

   static const int kRepeaterBufSize = 88200;
   std::vector<float> mRepeaterBuf[2];
   int mRepeaterWritePos{ 0 };
   int mRepeaterReadPos{ 0 };

   static const int kReverseBufSize = 22050;
   std::vector<float> mReverseBuf[2];
   int mReverseWritePos{ 0 };
   int mReverseReadOffset{ 0 };

   NoteInputBuffer mNoteInputBuffer;
   ChannelBuffer mWriteBuffer;

   float mWidth{ 320 };
   float mHeight{ 566 };
};
