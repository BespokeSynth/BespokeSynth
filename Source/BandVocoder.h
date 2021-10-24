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
//  BandVocoder.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/14.
//
//

#ifndef __modularSynth__BandVocoder__
#define __modularSynth__BandVocoder__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "BiquadFilterEffect.h"
#include "VocoderCarrierInput.h"
#include "PeakTracker.h"

#define VOCODER_MAX_BANDS 64

class BandVocoder : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public VocoderBase, public IIntSliderListener
{
public:
   BandVocoder();
   virtual ~BandVocoder();
   static IDrawableModule* Create() { return new BandVocoder(); }
   
   void CreateUIControls() override;
   
   void SetCarrierBuffer(float* carrier, int bufferSize) override;
   
   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=215; h=130; }
   bool Enabled() const override { return mEnabled; }
   
   void CalcFilters();
   
   float* mCarrierInputBuffer;
   
   float* mWorkBuffer;
   float* mOutBuffer;
   
   float mInputPreamp;
   float mCarrierPreamp;
   float mVolume;
   FloatSlider* mInputSlider;
   FloatSlider* mCarrierSlider;
   FloatSlider* mVolumeSlider;
   float mDryWet;
   FloatSlider* mDryWetSlider;
   float mQ;
   FloatSlider* mQSlider;
   IntSlider* mNumBandsSlider;
   int mNumBands;
   float mFreqBase;
   float mFreqRange;
   FloatSlider* mFBaseSlider;
   FloatSlider* mFRangeSlider;
   float mRingTime;
   FloatSlider* mRingTimeSlider;
   float mMaxBand;
   FloatSlider* mMaxBandSlider;
   float mSpacingStyle;
   FloatSlider* mSpacingStyleSlider;
   
   BiquadFilter mBiquadCarrier[VOCODER_MAX_BANDS];
   BiquadFilter mBiquadOut[VOCODER_MAX_BANDS];
   PeakTracker mPeaks[VOCODER_MAX_BANDS];
   PeakTracker mOutputPeaks[VOCODER_MAX_BANDS];

   bool mCarrierDataSet;
};


#endif /* defined(__modularSynth__BandVocoder__) */

