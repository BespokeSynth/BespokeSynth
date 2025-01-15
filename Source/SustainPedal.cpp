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
//  SustainPedal.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/7/14.
//
//

#include "SustainPedal.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

SustainPedal::SustainPedal()
{
}

void SustainPedal::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSustainCheckbox = new Checkbox(this, "sustain", 3, 3, &mSustain);
}

void SustainPedal::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mSustainCheckbox->Draw();
}

void SustainPedal::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mSustainCheckbox)
   {
      if (!mSustain)
      {
         for (int i = 0; i < 128; ++i)
         {
            if (mIsNoteBeingSustained[i])
            {
               PlayNoteOutput(NoteMessage(time, i, 0, -1));
               mIsNoteBeingSustained[i] = false;
            }
         }
      }
   }
}

void SustainPedal::PlayNote(NoteMessage note)
{
   if (mSustain)
   {
      if (note.velocity > 0)
      {
         PlayNoteOutput(note.MakeNoteOff());
         PlayNoteOutput(note);
         mIsNoteBeingSustained[note.pitch] = false; //not being sustained by this module it if it's held down
      }
      else
      {
         mIsNoteBeingSustained[note.pitch] = true;
      }
   }
   else
   {
      PlayNoteOutput(note);
   }
}

void SustainPedal::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SustainPedal::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
