#pragma once

#include "IAudioProcessor.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include <array>

class MutableResonator : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener
{
public:
   MutableResonator();
   virtual ~MutableResonator();

   static IDrawableModule* Create() { return new MutableResonator(); }

   static bool AcceptsAudio() { return true; }
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
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   static const int kNumModes = 24;

   NoteInputBuffer mNoteInputBuffer;

   //------ SVF Bandpass Mode ------
   // Each mode is a 2-pole state-variable filter in bandpass configuration
   struct SvfMode
   {
      float ic1{ 0 }; // integrator state 1
      float ic2{ 0 }; // integrator state 2
      float freq{ 440 };
      float q{ 50 };
      float amp{ 1.0f }; // amplitude weight (from position/brightness)
      float panL{ 0.707f };
      float panR{ 0.707f };
   };

   std::array<SvfMode, kNumModes> mModes;

   void UpdateFilterBank(); // Recompute all mode frequencies/amplitudes from parameters

   //------ Internal Exciter ------
   float mExciterEnv{ 0 };
   float mExciterPhase{ 0 };
   float mExciterDecay{ 0.999f };
   float mPrevNoise{ 0 };
   bool mExciterActive{ false };
   float mExciterVelocity{ 0 };

   //------ Parameters ------
   // Resonator
   float mFrequency{ 220.0f };
   int mPitch{ 57 }; // Current MIDI pitch (A3)
   float mTune{ 0.0f };
   float mStructure{ 0.4f }; // Inharmonicity: 0=harmonic, 1=very metallic
   float mBrightness{ 0.7f }; // High-mode rolloff
   float mDamping{ 0.3f }; // Ring time: 0=long, 1=short
   float mPosition{ 0.25f }; // Excitation point

   // Exciter
   float mExciterMix{ 0.5f }; // 0=pure click, 1=pure noise
   float mMalletHardness{ 0.5f };
   float mInputGain{ 1.0f }; // Gain for external audio exciter

   // Output
   float mSpread{ 0.5f };
   float mVolume{ 0.8f };

   //------ UI ------
   FloatSlider* mTuneSlider{ nullptr };
   FloatSlider* mStructureSlider{ nullptr };
   FloatSlider* mBrightnessSlider{ nullptr };
   FloatSlider* mDampingSlider{ nullptr };
   FloatSlider* mPositionSlider{ nullptr };

   FloatSlider* mExciterMixSlider{ nullptr };
   FloatSlider* mMalletHardnessSlider{ nullptr };
   FloatSlider* mInputGainSlider{ nullptr };

   FloatSlider* mSpreadSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };

   ClickButton* mStrikeButton{ nullptr };

   ChannelBuffer mWriteBuffer;

   float mWidth{ 380 };
   float mHeight{ 165 };
};
