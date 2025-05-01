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
//  NoteDelayer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#include "NoteDelayer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Profiler.h"

NoteDelayer::NoteDelayer()
{
}

void NoteDelayer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

NoteDelayer::~NoteDelayer()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteDelayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mDelaySlider = new FloatSlider(this, "delay", 4, 4, 100, 15, &mDelay, 0, 1, 4);
}

void NoteDelayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDelaySlider->Draw();

   float t = (gTime - mLastNoteOnTime) / (mDelay * TheTransport->GetDuration(kInterval_1n));
   if (t > 0 && t < 1)
   {
      ofPushStyle();
      ofNoFill();
      ofCircle(54, 11, 10);
      ofFill();
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      ofCircle(54 + sin(t * TWO_PI) * 10, 11 - cos(t * TWO_PI) * 10, 2);
      ofPopStyle();
   }
}

void NoteDelayer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
      mAppendIndex = 0;
      mConsumeIndex = 0;
   }
}

void NoteDelayer::OnTransportAdvanced(float amount)
{
   PROFILER(NoteDelayer);

   ComputeSliders(0);

   int end = mAppendIndex;
   if (mAppendIndex < mConsumeIndex)
      end += kQueueSize;
   for (int i = mConsumeIndex; i < end; ++i)
   {
      const NoteMessage note = mInputNotes[i % kQueueSize];
      if (NextBufferTime(true) >= note.time)
      {
         PlayNoteOutput(note);
         mConsumeIndex = (mConsumeIndex + 1) % kQueueSize;
      }
   }
}

void NoteDelayer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note); // Passthrough notes.
      return;
   }

   if (note.velocity > 0)
      mLastNoteOnTime = note.time;

   if ((mAppendIndex + 1) % kQueueSize != mConsumeIndex)
   {
      mInputNotes[mAppendIndex] = note;
      mInputNotes[mAppendIndex].time += mDelay / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      mAppendIndex = (mAppendIndex + 1) % kQueueSize;
   }
}

void NoteDelayer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void NoteDelayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteDelayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
