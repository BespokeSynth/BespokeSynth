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

    TakeRecorder.cpp
    Created: 9 Aug 2017 11:31:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "TakeRecorder.h"
#include "ModularSynth.h"
#include "Profiler.h"

TakeRecorder::TakeRecorder()
: IAudioProcessor(gBufferSize)
{
}

void TakeRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mStartSecondsSlider = new FloatSlider(this, "start", 5, 2, 110, 15, &mStartSeconds, 0, 4);
}

TakeRecorder::~TakeRecorder()
{
}

void TakeRecorder::Process(double time)
{
   PROFILER(TakeRecorder);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   IAudioReceiver* target = GetTarget();
   if (target)
   {
      Add(target->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);
   }

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

   GetBuffer()->Reset();
}

void TakeRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mStartSecondsSlider->Draw();
}

void TakeRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void TakeRecorder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
