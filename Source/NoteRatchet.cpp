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

    NoteRatchet.cpp
    Created: 2 Aug 2021 10:32:23pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteRatchet.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteRatchet::NoteRatchet()
{
}

void NoteRatchet::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mRatchetDurationSelector, "duration", (int*)&mRatchetDuration, 50);
   DROPDOWN(mRatchetSubdivisionSelector, "subdivision", (int*)&mRatchetSubdivision, 50);
   CHECKBOX(mSkipFirstCheckbox, "skip first", &mSkipFirst);
   ENDUIBLOCK(mWidth, mHeight);

   mRatchetDurationSelector->AddLabel("1n", kInterval_1n);
   mRatchetDurationSelector->AddLabel("2n", kInterval_2n);
   mRatchetDurationSelector->AddLabel("4n", kInterval_4n);
   mRatchetDurationSelector->AddLabel("4nt", kInterval_4nt);
   mRatchetDurationSelector->AddLabel("8n", kInterval_8n);
   mRatchetDurationSelector->AddLabel("8nt", kInterval_8nt);
   mRatchetDurationSelector->AddLabel("16n", kInterval_16n);
   mRatchetDurationSelector->AddLabel("16nt", kInterval_16nt);
   mRatchetDurationSelector->AddLabel("32n", kInterval_32n);
   mRatchetDurationSelector->AddLabel("32nt", kInterval_32nt);
   mRatchetDurationSelector->AddLabel("64n", kInterval_64n);

   mRatchetSubdivisionSelector->AddLabel("1n", kInterval_1n);
   mRatchetSubdivisionSelector->AddLabel("2n", kInterval_2n);
   mRatchetSubdivisionSelector->AddLabel("4n", kInterval_4n);
   mRatchetSubdivisionSelector->AddLabel("4nt", kInterval_4nt);
   mRatchetSubdivisionSelector->AddLabel("8n", kInterval_8n);
   mRatchetSubdivisionSelector->AddLabel("8nt", kInterval_8nt);
   mRatchetSubdivisionSelector->AddLabel("16n", kInterval_16n);
   mRatchetSubdivisionSelector->AddLabel("16nt", kInterval_16nt);
   mRatchetSubdivisionSelector->AddLabel("32n", kInterval_32n);
   mRatchetSubdivisionSelector->AddLabel("32nt", kInterval_32nt);
   mRatchetSubdivisionSelector->AddLabel("64n", kInterval_64n);

   mRatchetDurationSelector->DrawLabel(true);
   mRatchetSubdivisionSelector->DrawLabel(true);

   mWidth = mRatchetSubdivisionSelector->GetRect().width + 6;
}

void NoteRatchet::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRatchetDurationSelector->Draw();
   mRatchetSubdivisionSelector->Draw();
   mSkipFirstCheckbox->Draw();
}

void NoteRatchet::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void NoteRatchet::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity > 0)
   {
      double subdivisionMs = TheTransport->GetDuration(mRatchetSubdivision);
      int repetitions = TheTransport->CountInStandardMeasure(mRatchetSubdivision) / TheTransport->CountInStandardMeasure(mRatchetDuration);
      int startIndex = 0;
      if (mSkipFirst)
         startIndex = 1;
      for (int i = startIndex; i < repetitions; ++i)
      {
         NoteMessage newNote = note.MakeClone();
         newNote.time = note.time + subdivisionMs * i;
         NoteMessage newNoteOff = newNote.MakeNoteOff();
         newNoteOff.time = note.time + subdivisionMs * (i + 1);
         PlayNoteOutput(newNote);
         PlayNoteOutput(newNoteOff);
      }
   }
}

void NoteRatchet::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void NoteRatchet::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteRatchet::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
