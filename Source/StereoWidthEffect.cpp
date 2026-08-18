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
//  StereoWidthEffect.cpp
//  Bespoke
//

#include "StereoWidthEffect.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include <cmath>

void StereoWidthEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMidSlider = new FloatSlider(this, "mid", 3, 3, 94, 15, &mMid, 0.0f, 2.0f);
   mWidthSlider = new FloatSlider(this, "width", 3, 20, 94, 15, &mWidthAmt, 0.0f, 2.0f);
   mPanSlider = new FloatSlider(this, "pan", 3, 37, 94, 15, &mPan, -1.0f, 1.0f);
}

void StereoWidthEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(StereoWidthEffect);

   if (!mEnabled)
      return;

   int bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   //constant-power balance so the centre stays at unity gain
   float p = (ofClamp(mPan, -1.0f, 1.0f) + 1.0f) * 0.5f;
   float gl = cosf(p * FPI * 0.5f) * 1.41421356f;
   float gr = sinf(p * FPI * 0.5f) * 1.41421356f;

   if (buffer->NumActiveChannels() >= 2)
   {
      float* left = buffer->GetChannel(0);
      float* right = buffer->GetChannel(1);
      for (int i = 0; i < bufferSize; ++i)
      {
         float mid = (left[i] + right[i]) * 0.5f * mMid;
         float side = (left[i] - right[i]) * 0.5f * mWidthAmt;
         left[i] = (mid + side) * gl;
         right[i] = (mid - side) * gr;
      }
   }
   else
   {
      //mono input: no Side to widen; just apply the mid (level) control
      float* mono = buffer->GetChannel(0);
      for (int i = 0; i < bufferSize; ++i)
         mono[i] *= mMid;
   }
}

float StereoWidthEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   //"how far from neutral" - drives the effect-active highlight
   return ofClamp(fabsf(mWidthAmt - 1.0f) + fabsf(mMid - 1.0f) + fabsf(mPan), 0.0f, 1.0f);
}

void StereoWidthEffect::DrawModule()
{
   if (!mEnabled)
      return;
   mMidSlider->Draw();
   mWidthSlider->Draw();
   mPanSlider->Draw();
}
