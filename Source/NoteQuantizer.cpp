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

    NoteQuantizer.cpp
    Created: 6 Dec 2020 7:36:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteQuantizer.h"
#include "SynthGlobals.h"

NoteQuantizer::NoteQuantizer()
{
}

void NoteQuantizer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mQuantizeInterval, OffsetInfo(0, true), false);
}

NoteQuantizer::~NoteQuantizer()
{
   TheTransport->RemoveListener(this);
}

void NoteQuantizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mQuantizeIntervalSelector = new DropdownList(this, "quantize", 3, 4, (int*)(&mQuantizeInterval));
   mNoteRepeatCheckbox = new Checkbox(this, "repeat", 3, 22, &mNoteRepeat);

   mQuantizeIntervalSelector->AddLabel("none", kInterval_None);
   mQuantizeIntervalSelector->AddLabel("4n", kInterval_4n);
   mQuantizeIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mQuantizeIntervalSelector->AddLabel("8n", kInterval_8n);
   mQuantizeIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mQuantizeIntervalSelector->AddLabel("16n", kInterval_16n);
   mQuantizeIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mQuantizeIntervalSelector->AddLabel("32n", kInterval_32n);
   mQuantizeIntervalSelector->AddLabel("64n", kInterval_64n);
}

void NoteQuantizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mQuantizeIntervalSelector->SetShowing(!mHasReceivedPulse);
   mQuantizeIntervalSelector->Draw();
   mNoteRepeatCheckbox->Draw();
}

void NoteQuantizer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (mQuantizeInterval == kInterval_None)
   {
      PlayNoteOutput(note);
   }
   else
   {
      if (note.pitch < mInputInfos.size())
      {
         if (note.velocity > 0)
         {
            if ((mScheduledOffs[note.pitch] || mPreScheduledOffs[note.pitch] || mInputInfos[note.pitch].held) && mInputInfos[note.pitch].note.voiceIdx != note.voiceIdx)
            {
               PlayNoteOutput(mInputInfos[note.pitch].note.MakeNoteOff());
               mScheduledOffs[note.pitch] = false;
               mPreScheduledOffs[note.pitch] = false;
            }
            mInputInfos[note.pitch].note = note;
            mInputInfos[note.pitch].held = true;
            mInputInfos[note.pitch].hasPlayedYet = false;
         }
         else
         {
            if (mInputInfos[note.pitch].hasPlayedYet)
               mScheduledOffs[note.pitch] = true;
            else
               mPreScheduledOffs[note.pitch] = true;

            if (mNoteRepeat)
               mInputInfos[note.pitch].note.velocity = 0;

            mInputInfos[note.pitch].held = false;
         }
      }
   }
}

void NoteQuantizer::OnEvent(double time, float strength)
{
   for (int i = 0; i < (int)mInputInfos.size(); ++i)
   {
      if (mScheduledOffs[i])
      {
         NoteMessage noteOff = mInputInfos[i].note.MakeNoteOff();
         noteOff.time = time;
         PlayNoteOutput(noteOff);
         mScheduledOffs[i] = false;
      }

      if (mInputInfos[i].note.velocity > 0)
      {
         NoteMessage note = mInputInfos[i].note;
         note.time = time;
         note.velocity *= strength;
         PlayNoteOutput(note);
         mInputInfos[i].hasPlayedYet = true;
         if (!mNoteRepeat)
            mInputInfos[i].note.velocity = 0;
         else
            mScheduledOffs[i] = true;
         if (mPreScheduledOffs[i])
         {
            mPreScheduledOffs[i] = false;
            mScheduledOffs[i] = true;
         }
      }
   }
}

void NoteQuantizer::OnTimeEvent(double time)
{
   if (!mHasReceivedPulse)
      OnEvent(time, 1);
}

void NoteQuantizer::OnPulse(double time, float velocity, int flags)
{
   mHasReceivedPulse = true;
   OnEvent(time, velocity);
}

void NoteQuantizer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mQuantizeIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mQuantizeInterval;
   }
}

void NoteQuantizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteQuantizer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
