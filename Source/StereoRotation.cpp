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
   mPhaseSlider = new FloatSlider(this, "phase", 5, 2, 110, 15, &mPhase, 0, 1);
}

StereoRotation::~StereoRotation()
{
}

void StereoRotation::Process(double time)
{
   PROFILER(StereoRotation);

   SyncBuffers();

   IAudioReceiver* target = GetTarget();

   int bufferSize = GetBuffer()->BufferSize();

   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();

   float* lbuf = new float[GetBuffer()->BufferSize()];
   float* rbuf = new float[GetBuffer()->BufferSize()];

      bool mono = GetBuffer()->NumActiveChannels() == 1;
      if (mono)
      {
      }
      else
      {
         Clear(lbuf, bufferSize);
         Clear(rbuf, bufferSize);

         Add(lbuf, GetBuffer()->GetChannel(0), bufferSize);
         if (mEnabled)
         {
            Add(rbuf, GetBuffer()->GetChannel(1), bufferSize);
            Mult(lbuf, cos(mPhase * 2 * PI), bufferSize);
            Mult(rbuf, -sin(mPhase * 2 * PI), bufferSize);
         }
         Add(out->GetChannel(0), lbuf, bufferSize);
         if (mEnabled)
         {
            Add(out->GetChannel(0), rbuf, bufferSize);
            Mult(out->GetChannel(0), 0.5, bufferSize);
         }
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

         Clear(lbuf, bufferSize);
         Clear(rbuf, bufferSize);

         if (mEnabled)
            Add(lbuf, GetBuffer()->GetChannel(0), bufferSize);
         Add(rbuf, GetBuffer()->GetChannel(1), bufferSize);
         if (mEnabled)
         {
            Mult(lbuf, sin(mPhase * 2 * PI), bufferSize);
            Mult(rbuf, cos(mPhase * 2 * PI), bufferSize);
            Add(out->GetChannel(1), lbuf, bufferSize);
         }
         Add(out->GetChannel(1), rbuf, bufferSize);
         if (mEnabled)
            Mult(out->GetChannel(1), 0.5, bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(1), bufferSize, 1);
      }
   }

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
