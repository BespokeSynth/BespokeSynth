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

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
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
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetCarrierBuffer(float* carrier, int bufferSize) override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
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
      w = 215;
      h = 130;
   }

   void CalcFilters();

   float* mCarrierInputBuffer{ nullptr };

   float* mWorkBuffer{ nullptr };
   float* mOutBuffer{ nullptr };

   float mInputPreamp{ 1 };
   float mCarrierPreamp{ 1 };
   float mVolume{ 1 };
   FloatSlider* mInputSlider{ nullptr };
   FloatSlider* mCarrierSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };
   float mDryWet{ 1 };
   FloatSlider* mDryWetSlider{ nullptr };
   float mQ{ 40 };
   FloatSlider* mQSlider{ nullptr };
   IntSlider* mNumBandsSlider{ nullptr };
   int mNumBands{ 40 };
   float mFreqBase{ 200 };
   float mFreqRange{ 6000 };
   FloatSlider* mFBaseSlider{ nullptr };
   FloatSlider* mFRangeSlider{ nullptr };
   float mRingTime{ .03 };
   FloatSlider* mRingTimeSlider{ nullptr };
   float mMaxBand{ 1 };
   FloatSlider* mMaxBandSlider{ nullptr };
   float mSpacingStyle{ 0 };
   FloatSlider* mSpacingStyleSlider{ nullptr };

   BiquadFilter mBiquadCarrier[VOCODER_MAX_BANDS]{};
   BiquadFilter mBiquadOut[VOCODER_MAX_BANDS]{};
   PeakTracker mPeaks[VOCODER_MAX_BANDS]{};
   PeakTracker mOutputPeaks[VOCODER_MAX_BANDS]{};

   bool mCarrierDataSet{ false };
};
