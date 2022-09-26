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
//  FreqDomainBoilerplate.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/24/13.
//
//

#ifndef __modularSynth__FreqDomainBoilerplate__
#define __modularSynth__FreqDomainBoilerplate__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "GateEffect.h"
#include "BiquadFilterEffect.h"

class FreqDomainBoilerplate : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   FreqDomainBoilerplate();
   virtual ~FreqDomainBoilerplate();
   static IDrawableModule* Create() { return new FreqDomainBoilerplate(); }


   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   //IButtonListener
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

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

   float mInputPreamp;
   float mValue1;
   float mVolume;
   FloatSlider* mInputSlider;
   FloatSlider* mValue1Slider;
   FloatSlider* mVolumeSlider;
   float mDryWet;
   FloatSlider* mDryWetSlider;
   float mValue2;
   FloatSlider* mValue2Slider;
   float mValue3;
   FloatSlider* mValue3Slider;
   float mPhaseOffset;
   FloatSlider* mPhaseOffsetSlider;
};


#endif /* defined(__modularSynth__FreqDomainBoilerplate__) */
