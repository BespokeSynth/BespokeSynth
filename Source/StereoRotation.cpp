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
/*
  ==============================================================================

    GainStage.cpp
    Created: 24 Apr 2021 3:47:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "StereoRotation.h"
#include "SynthGlobals.h"
#include "Profiler.h"

StereoRotation::StereoRotation()
{
}

void StereoRotation::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPhaseSlider = new FloatSlider(this, "phase", 5, 2, 110, 15, &mPhase, 0, 1);
}

void StereoRotation::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(StereoRotation);

   if (!mEnabled)
      return;

   bool mono = buffer->NumActiveChannels() == 1;

   float bufferSize = buffer->BufferSize();

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);
      if (mono)
      {
         buffer->GetChannel(0)[i] = (buffer->GetChannel(0)[i] * cos(mPhase * 2 * PI) - buffer->GetChannel(0)[i] * sin(mPhase * 2 * PI)) / 2;
      }
      else
      {
         buffer->GetChannel(0)[i] = (buffer->GetChannel(0)[i] * cos(mPhase * 2 * PI) - buffer->GetChannel(1)[i] * sin(mPhase * 2 * PI)) / 2;
         buffer->GetChannel(1)[i] = (buffer->GetChannel(0)[i] * sin(mPhase * 2 * PI) + buffer->GetChannel(1)[i] * cos(mPhase * 2 * PI)) / 2;
      }
   }
}

void StereoRotation::DrawModule()
{
   mPhaseSlider->Draw();
}

void StereoRotation::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void StereoRotation::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}
