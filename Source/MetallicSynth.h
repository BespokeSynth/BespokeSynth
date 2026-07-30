#pragma once

#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include <array>

class MetallicSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener
{
public:
   MetallicSynth();
   virtual ~MetallicSynth();

   static IDrawableModule* Create() { return new MetallicSynth(); }

   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override { }

   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

private:
   static const int kNumModes = 12; // Increased to 12
   static const int kNumVoices = 8;

   NoteInputBuffer mNoteInputBuffer;

   struct Voice
   {
      bool active{ false };
      int pitch{ 60 };
      float velocity{ 1.0f };
      double exciterTime{ 0 };
      float prevNoise{ 0.0f }; // State for noise hardness filter

      // Simple additive synthesis — no filters at all
      double modePhase[kNumModes];
      double modeFreq[kNumModes];
      float modeAmp[kNumModes];
      float modeDecayRate[kNumModes];
      float modePanL[kNumModes];
      float modePanR[kNumModes];
   };

   std::array<Voice, kNumVoices> mVoices;

   enum MaterialType
   {
      kMaterial_Bell = 0,
      kMaterial_Beam,
      kMaterial_String,
      kMaterial_Membrane,
      kMaterial_Plate,
      kMaterial_Tube,
      kMaterial_Marimba
   };

   // Resonator
   MaterialType mMaterial{ kMaterial_Bell };
   float mTune{ 0.0f };
   float mFineTune{ 0.0f };
   float mDecay{ 0.5f };
   float mHarmonicsDecay{ 0.5f }; // Replaces mBrightness
   float mStiffness{ 0.0f };
   float mHitPosition{ 0.2f };

   // Exciter
   float mMallet{ 0.5f };
   float mNoise{ 0.5f };
   float mHardness{ 0.5f };

   // Output
   float mSpread{ 0.5f };
   float mVolume{ 1.0f };

   // UI
   DropdownList* mMaterialSelector{ nullptr };
   FloatSlider* mTuneSlider{ nullptr };
   FloatSlider* mFineTuneSlider{ nullptr };
   FloatSlider* mDecaySlider{ nullptr };
   FloatSlider* mHarmonicsDecaySlider{ nullptr };
   FloatSlider* mStiffnessSlider{ nullptr };
   FloatSlider* mHitPositionSlider{ nullptr };

   FloatSlider* mMalletSlider{ nullptr };
   FloatSlider* mNoiseSlider{ nullptr };
   FloatSlider* mHardnessSlider{ nullptr };

   FloatSlider* mSpreadSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };

   ChannelBuffer mWriteBuffer;

   float mWidth{ 400 }; // Wider for 3 columns
   float mHeight{ 150 };
};
