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
//  MultibandCompressor.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/27/14.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "RollingBuffer.h"
#include "LinkwitzRileyFilter.h"
#include "PeakTracker.h"

#define COMPRESSOR_MAX_BANDS 10

class MultibandCompressor : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener
{
public:
   MultibandCompressor();
   virtual ~MultibandCompressor();
   static IDrawableModule* Create() { return new MultibandCompressor(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 210;
      h = 150;
   }

   void CalcFilters();

   float* mWorkBuffer{ nullptr };
   float* mOutBuffer{ nullptr };

   float mDryWet{ 1 };
   FloatSlider* mDryWetSlider{ nullptr };
   IntSlider* mNumBandsSlider{ nullptr };
   int mNumBands{ 4 };
   float mFreqMin{ 150 };
   float mFreqMax{ 7500 };
   FloatSlider* mFMinSlider{ nullptr };
   FloatSlider* mFMaxSlider{ nullptr };
   float mRingTime{ .01 };
   FloatSlider* mRingTimeSlider{ nullptr };
   float mMaxBand{ .3 };
   FloatSlider* mMaxBandSlider{ nullptr };

   CLinkwitzRiley_4thOrder mFilters[COMPRESSOR_MAX_BANDS];
   PeakTracker mPeaks[COMPRESSOR_MAX_BANDS];
};
