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
//  Muter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/26/13.
//
//

#include "Muter.h"
#include "SynthGlobals.h"
#include "Profiler.h"

Muter::Muter()
{
   mRamp.SetValue(0);
}

void Muter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPassCheckbox = new Checkbox(this, "pass", 5, 2, &mPass);
   mRampTimeSlider = new FloatSlider(this, "ms", 5, 20, 70, 15, &mRampTimeMs, 3, 1000);
   mRampTimeSlider->SetMode(FloatSlider::kSquare);
}

Muter::~Muter()
{
}

void Muter::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(Muter);

   float bufferSize = buffer->BufferSize();

   for (int i = 0; i < bufferSize; ++i)
   {
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mRamp.Value(time);
      time += gInvSampleRateMs;
   }
}

void Muter::DrawModule()
{
   mPassCheckbox->Draw();
   mRampTimeSlider->Draw();
}

void Muter::CheckboxUpdated(Checkbox* checkbox, double time)
{
   mRamp.Start(time, mPass ? 1 : 0, time + mRampTimeMs);
}
