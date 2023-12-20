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

#include "GainStageEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

GainStageEffect::GainStageEffect()
{
}

void GainStageEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGainSlider = new FloatSlider(this, "gain", 5, 2, 110, 15, &mGain, 0, 4);
}

void GainStageEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(GainStageEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mGain;
   }
}

void GainStageEffect::DrawModule()
{
   mGainSlider->Draw();
}

void GainStageEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void GainStageEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}
