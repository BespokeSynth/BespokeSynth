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
//  VelocityToDuration.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 9/15/25.
//
//

#include "VelocityToDuration.h"
#include "SynthGlobals.h"

VelocityToDuration::VelocityToDuration()
{
}

void VelocityToDuration::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void VelocityToDuration::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMaxDurationSlider = new FloatSlider(this, "max duration", 5, 2, 140, 15, &mMaxDuration, 0.01f, 4, 4);
   mMaxDurationSlider->SetMode(FloatSlider::kSquare);
}

VelocityToDuration::~VelocityToDuration()
{
   TheTransport->RemoveAudioPoller(this);
}

void VelocityToDuration::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMaxDurationSlider->Draw();
}

void VelocityToDuration::OnTransportAdvanced(float amount)
{
   for (auto iter = mNoteOffs.begin(); iter != mNoteOffs.end();)
   {
      if (iter->time < NextBufferTime(false))
      {
         PlayNoteOutput(*iter);
         iter = mNoteOffs.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

void VelocityToDuration::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity > 0)
   {
      PlayNoteOutput(note);

      ComputeSliders(0);

      float durationMs = note.velocity / 127 * mMaxDuration / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();

      bool found = false;
      for (auto& queued : mNoteOffs)
      {
         if (queued.pitch == note.pitch)
         {
            queued.time = note.time + durationMs;
            queued.voiceIdx = note.voiceIdx;
            found = true;
            break;
         }
      }

      if (!found)
      {
         NoteMessage noteOff = note.MakeNoteOff();
         noteOff.time = note.time + durationMs;
         mNoteOffs.push_back(noteOff);
      }
   }
}

void VelocityToDuration::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}
