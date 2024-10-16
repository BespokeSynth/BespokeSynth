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

    Inverter.cpp
    Created: 13 Nov 2019 10:16:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Inverter.h"
#include "ModularSynth.h"
#include "Profiler.h"

Inverter::Inverter()
: IAudioProcessor(gBufferSize)
{
}

void Inverter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

Inverter::~Inverter()
{
}

void Inverter::Process(double time)
{
   PROFILER(Inverter);

   SyncBuffers();

   IAudioReceiver* target = GetTarget();

   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (mEnabled)
            Mult(GetBuffer()->GetChannel(ch), -1, out->BufferSize());
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void Inverter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void Inverter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Inverter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
