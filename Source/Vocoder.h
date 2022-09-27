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
//  Vocoder.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/17/13.
//
//

#ifndef __modularSynth__Vocoder__
#define __modularSynth__Vocoder__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "GateEffect.h"
#include "BiquadFilterEffect.h"
#include "VocoderCarrierInput.h"

class Vocoder : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public VocoderBase, public IIntSliderListener
{
public:
   Vocoder();
   virtual ~Vocoder();
   static IDrawableModule* Create() { return new Vocoder(); }


   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void SetCarrierBuffer(float* carrier, int bufferSize) override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 235;
      h = 170;
   }
   bool Enabled() const override { return mEnabled; }

   FFTData mFFTData;

   float* mWindower;


   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;

   float* mCarrierInputBuffer;
   RollingBuffer mRollingCarrierBuffer;
   FFTData mCarrierFFTData;

   float mInputPreamp;
   float mCarrierPreamp;
   float mVolume;
   FloatSlider* mInputSlider;
   FloatSlider* mCarrierSlider;
   FloatSlider* mVolumeSlider;
   float mDryWet;
   FloatSlider* mDryWetSlider;
   float mFricativeThresh;
   FloatSlider* mFricativeSlider;
   bool mFricDetected;
   float mWhisper;
   FloatSlider* mWhisperSlider;
   float mPhaseOffset;
   FloatSlider* mPhaseOffsetSlider;

   int mCut;
   IntSlider* mCutSlider;

   GateEffect mGate;

   bool mCarrierDataSet;
};


#endif /* defined(__modularSynth__Vocoder__) */
