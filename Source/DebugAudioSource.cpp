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
//  DebugAudioSource.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 7/1/14.
//
//

#include "DebugAudioSource.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

DebugAudioSource::DebugAudioSource()
{
}

DebugAudioSource::~DebugAudioSource()
{
}

void DebugAudioSource::Process(double time)
{
   PROFILER(DebugAudioSource);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   for (int i = 0; i < bufferSize; ++i)
   {
      float sample = 1;
      out[i] += sample;
      GetVizBuffer()->Write(sample, 0);
   }
}

void DebugAudioSource::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
}

void DebugAudioSource::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void DebugAudioSource::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
