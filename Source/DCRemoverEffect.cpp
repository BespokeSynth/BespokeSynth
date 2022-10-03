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
//  DCRemoverEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/2/14.
//
//

#include "DCRemoverEffect.h"
#include "Profiler.h"

DCRemoverEffect::DCRemoverEffect()
{
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mBiquad[i].SetFilterParams(10, sqrt(2) / 2);
      mBiquad[i].SetFilterType(kFilterType_Highpass);
      mBiquad[i].UpdateFilterCoeff();
   }
}

DCRemoverEffect::~DCRemoverEffect()
{
}

void DCRemoverEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DCRemoverEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      mBiquad[ch].Filter(buffer->GetChannel(ch), bufferSize);
}

void DCRemoverEffect::DrawModule()
{
}

float DCRemoverEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return 1;
}

void DCRemoverEffect::GetModuleDimensions(float& width, float& height)
{
   width = 30;
   height = 0;
}

void DCRemoverEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mBiquad[i].Clear();
   }
}
