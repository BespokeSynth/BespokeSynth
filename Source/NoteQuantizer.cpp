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

void NoteQuantizer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (mQuantizeInterval == kInterval_None)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
   else
   {
      if (pitch < mInputInfos.size())
      {
         if (velocity > 0)
         {
            if ((mScheduledOffs[pitch] || mPreScheduledOffs[pitch] || mInputInfos[pitch].held) && mInputInfos[pitch].voiceIdx != voiceIdx)
            {
               PlayNoteOutput(time, pitch, 0, mInputInfos[pitch].voiceIdx, mInputInfos[pitch].modulation);
               mScheduledOffs[pitch] = false;
               mPreScheduledOffs[pitch] = false;
            }
            mInputInfos[pitch].velocity = velocity;
            mInputInfos[pitch].voiceIdx = voiceIdx;
            mInputInfos[pitch].modulation = modulation;
            mInputInfos[pitch].held = true;
            mInputInfos[pitch].hasPlayedYet = false;
         }
         else
         {
            if (mInputInfos[pitch].hasPlayedYet)
               mScheduledOffs[pitch] = true;
            else
               mPreScheduledOffs[pitch] = true;

            if (mNoteRepeat)
               mInputInfos[pitch].velocity = 0;

            mInputInfos[pitch].held = false;
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
         PlayNoteOutput(time, i, 0, mInputInfos[i].voiceIdx, mInputInfos[i].modulation);
         mScheduledOffs[i] = false;
      }

      if (mInputInfos[i].velocity > 0)
      {
         PlayNoteOutput(time, i, mInputInfos[i].velocity * strength, mInputInfos[i].voiceIdx, mInputInfos[i].modulation);
         mInputInfos[i].hasPlayedYet = true;
         if (!mNoteRepeat)
            mInputInfos[i].velocity = 0;
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
