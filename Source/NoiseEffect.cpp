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
//  NoiseEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#include "NoiseEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

NoiseEffect::NoiseEffect()
{
}

void NoiseEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAmountSlider = new FloatSlider(this, "amount", 5, 20, 110, 15, &mAmount, 0, 1);
   mWidthSlider = new IntSlider(this, "width", 5, 37, 110, 15, &mWidth, 1, 100);
}

void NoiseEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(NoiseEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   for (int i = 0; i < bufferSize; ++i)
   {
      if (mSampleCounter < mWidth - 1)
      {
         ++mSampleCounter;
      }
      else
      {
         mRandom = ofRandom(mAmount) + (1 - mAmount);
         mSampleCounter = 0;
      }

      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mRandom;
   }
}

void NoiseEffect::DrawModule()
{

   mWidthSlider->Draw();
   mAmountSlider->Draw();
}

float NoiseEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void NoiseEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void NoiseEffect::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void NoiseEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}
