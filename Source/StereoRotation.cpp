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

    StereoRotation.cpp
    Created: 24 May 2025 9:42:28am
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "StereoRotation.h"
#include "ModularSynth.h"
#include "Profiler.h"

StereoRotation::StereoRotation()
: IAudioProcessor(gBufferSize)
{
}

void StereoRotation::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPhaseSlider = new FloatSlider(this, "phase", 5, 2, 110, 15, &mPhase, -0.5, 0.5);
}

StereoRotation::~StereoRotation()
{
}

void StereoRotation::Process(double time)
{
   PROFILER(StereoRotation);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   int bufferSize = GetBuffer()->BufferSize();

   ChannelBuffer* out = target->GetBuffer();

   SyncBuffers(2);

   int secondChannel = GetBuffer()->NumActiveChannels() - 1;
   for (int i = 0; i < bufferSize; ++i)
   {
      if (mEnabled)
      {
         float phaseSin = sin(mPhase * 2 * PI);
         float phaseCos = cos(mPhase * 2 * PI);

         out->GetChannel(0)[i] += GetBuffer()->GetChannel(0)[i] * phaseCos - GetBuffer()->GetChannel(secondChannel)[i] * phaseSin;
         out->GetChannel(1)[i] += GetBuffer()->GetChannel(0)[i] * phaseSin + GetBuffer()->GetChannel(secondChannel)[i] * phaseCos;
      }
      else
      {
         out->GetChannel(0)[i] += GetBuffer()->GetChannel(0)[i];
         out->GetChannel(1)[i] += GetBuffer()->GetChannel(secondChannel)[i];
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);

   GetBuffer()->Reset();
}

void StereoRotation::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPhaseSlider->Draw();
}

void StereoRotation::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void StereoRotation::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void StereoRotation::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
