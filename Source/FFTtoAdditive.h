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
//  FFTtoAdditive.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/24/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "BiquadFilterEffect.h"

#define VIZ_WIDTH 1000
#define RAZOR_HISTORY 100

class FFTtoAdditive : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   FFTtoAdditive();
   virtual ~FFTtoAdditive();
   static IDrawableModule* Create() { return new FFTtoAdditive(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   //IButtonListener
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;


   bool IsEnabled() const override { return mEnabled; }

private:
   void DrawViz();
   float SinSample(float phase);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 235;
      h = 170;
   }

   FFTData mFFTData;

   float* mWindower{ nullptr };

   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;

   float mInputPreamp{ 1 };
   float mValue1{ 1 };
   float mVolume{ 1 };
   FloatSlider* mInputSlider{ nullptr };
   FloatSlider* mValue1Slider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };
   float mDryWet{ 1 };
   FloatSlider* mDryWetSlider{ nullptr };
   float mValue2{ 1 };
   FloatSlider* mValue2Slider{ nullptr };
   float mValue3{ 0 };
   FloatSlider* mValue3Slider{ nullptr };
   float mPhaseOffset{ 0 };
   FloatSlider* mPhaseOffsetSlider{ nullptr };

   float mPeakHistory[RAZOR_HISTORY][VIZ_WIDTH + 1]{};
   int mHistoryPtr{ 0 };
   float* mPhaseInc{ nullptr };
};
