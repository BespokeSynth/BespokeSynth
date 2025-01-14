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
//  Kicker.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "Kicker.h"
#include "OpenFrameworksPort.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"

Kicker::Kicker()
{
}

void Kicker::DrawModule()
{

   DrawConnection(mDrumPlayer);
   if (Minimized() || IsVisible() == false)
      return;
}

void Kicker::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void Kicker::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (mEnabled && mDrumPlayer)
   {
      mDrumPlayer->PlayNote(NoteMessage(note.time, 3, note.velocity));
   }
}

void Kicker::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("drumplayer", moduleInfo, "", FillDropdown<DrumPlayer*>);

   SetUpFromSaveData();
}

void Kicker::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetDrumPlayer(dynamic_cast<DrumPlayer*>(TheSynth->FindModule(mModuleSaveData.GetString("drumplayer"), false)));
}
