/*
  ==============================================================================

    UnstablePitch.h
    Created: 2 Mar 2021 7:48:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"
#include "PerlinNoise.h"

struct UnstablePerlinModulation
{
   UnstablePerlinModulation(float amount, float warble, float noise)
      : mPerlinAmount(amount)
      , mPerlinWarble(warble)
      , mPerlinNoise(noise)
   {
      mPerlinSeed = gRandom() % 10000;
   }

   PerlinNoise mNoise;
   float mPerlinAmount;
   float mPerlinWarble;
   float mPerlinNoise;
   int mPerlinSeed;

   float GetValue(double time, float travel, float offset)
   {
      return mNoise.noise(travel * ofClamp(mPerlinWarble * 10, 0, 1), offset + time * mPerlinNoise / 5, time * mPerlinWarble / 100 + mPerlinSeed);
   }
};

class UnstablePitch : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   UnstablePitch();
   virtual ~UnstablePitch();
   static IDrawableModule* Create() { return new UnstablePitch(); }

   string GetTitleLabel() override { return "unstable pitch"; }
   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }

   void FillModulationBuffer(double time, int voiceIdx);

   UnstablePerlinModulation mPerlin;
   FloatSlider* mAmountSlider;
   FloatSlider* mWarbleSlider;
   FloatSlider* mNoiseSlider;
   float mWidth;
   float mHeight;
   std::array<bool, kNumVoices> mIsVoiceUsed { false };
   std::array<int, 128> mPitchToVoice;
   int mVoiceRoundRobin;

   Modulations mModulation;
};

