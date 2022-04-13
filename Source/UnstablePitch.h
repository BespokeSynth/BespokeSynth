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
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   void FillModulationBuffer(double time, int voiceIdx);

   UnstablePerlinModulation mPerlin;
   FloatSlider* mAmountSlider;
   FloatSlider* mWarbleSlider;
   FloatSlider* mNoiseSlider;
   float mWidth;
   float mHeight;
   std::array<bool, kNumVoices> mIsVoiceUsed{ false };
   std::array<int, 128> mPitchToVoice;
   int mVoiceRoundRobin;

   Modulations mModulation;
};
