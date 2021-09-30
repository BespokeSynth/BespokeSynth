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

#ifndef __Bespoke__MultibandCompressor__
#define __Bespoke__MultibandCompressor__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
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
   
   std::string GetTitleLabel() override { return "multiband compressor"; }
   void CreateUIControls() override;
   
   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=210; h=150; }
   bool Enabled() const override { return mEnabled; }
   
   void CalcFilters();
   
   float* mWorkBuffer;
   float* mOutBuffer;
   
   float mDryWet;
   FloatSlider* mDryWetSlider;
   IntSlider* mNumBandsSlider;
   int mNumBands;
   float mFreqMin;
   float mFreqMax;
   FloatSlider* mFMinSlider;
   FloatSlider* mFMaxSlider;
   float mRingTime;
   FloatSlider* mRingTimeSlider;
   float mMaxBand;
   FloatSlider* mMaxBandSlider;
   
   CLinkwitzRiley_4thOrder mFilters[COMPRESSOR_MAX_BANDS];
   PeakTracker mPeaks[COMPRESSOR_MAX_BANDS];
};

#endif /* defined(__Bespoke__MultibandCompressor__) */

