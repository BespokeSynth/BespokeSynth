#pragma once

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "Slider.h"
#include "DropdownList.h"
#include "BiquadFilter.h"
#include "ADSR.h"
#include "ADSRDisplay.h"
#include "IAudioSource.h"
#include "PatchCableSource.h"
#include <array>
#include <vector>

class NoiseSynthVoice
{
public:
   NoiseSynthVoice();
   ~NoiseSynthVoice() = default;

   friend class NoiseSynth;

   void PlayNote(double time, int pitch, int velocity, const ADSR& masterAdsr);
   void Process(float* outL, float* outR, int numSamples, double time, double sampleIncrementMs, float freqShift, float cutoff, float res, int filterType, int noiseColor, float volume);

   bool IsActive(double time) const { return mNoteOn || !mAdsr.IsDone(time); }

   ADSR mAdsr{ 20, 100, 0.5f, 500 };

private:
   bool mNoteOn{ false };
   int mPitch{ 60 };

   // Noise state
   float mNoiseState1{ 0.0f }; // for Brown/Pink filter

   // Filter
   BiquadFilter mFilter;

   // Frequency Shifter (Hilbert)
   static const int kHilbertLen = 65;
   static const int kHilbertMid = 32;
   std::vector<float> mHil;
   std::vector<float> mDelayLine;
   int mDelayPos{ 0 };
   double mPhase{ 0.0 };
};

class NoiseSynth : public IDrawableModule, public INoteReceiver, public IAudioSource, public IFloatSliderListener, public IDropdownListener
{
public:
   NoiseSynth();
   virtual ~NoiseSynth();

   static IDrawableModule* Create() { return new NoiseSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void DrawModule() override;
   void Process(double time) override;

   // INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override { }
   const NoteHistory& GetHistory() const { return mNoteHistory; }
   void SetHistory(const NoteHistory& history) { mNoteHistory = history; }

   // IFloatSliderListener & IDropdownListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override { }
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override { }

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void GetModuleDimensions(float& width, float& height) override;


   DropdownList* mNoiseColorDropdown{ nullptr };
   int mNoiseColor{ 0 }; // 0 = White, 1 = Pink, 2 = Brown

   DropdownList* mFilterTypeDropdown{ nullptr };
   int mFilterType{ 0 }; // 0 = LP, 1 = HP, 2 = BP

   FloatSlider* mCutoffSlider{ nullptr };
   float mCutoff{ 0.5f };

   FloatSlider* mResonanceSlider{ nullptr };
   float mResonance{ 0.1f };

   FloatSlider* mFreqShiftSlider{ nullptr };
   float mFreqShift{ 0.0f };

   ADSRDisplay* mAdsrDisplay{ nullptr };
   ADSR mMasterAdsr{ 20, 100, 0.5f, 500 };

   static const int kNumVoices = 8;
   std::array<NoiseSynthVoice, kNumVoices> mVoices;
   NoteHistory mNoteHistory;
   NoteInputBuffer mNoteInputBuffer;
   float mVolume{ 0.5f };
};
