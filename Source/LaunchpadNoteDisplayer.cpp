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
//  LaunchpadNoteDisplayer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#include "LaunchpadNoteDisplayer.h"
#include "OpenFrameworksPort.h"
#include "LaunchpadKeyboard.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"

LaunchpadNoteDisplayer::LaunchpadNoteDisplayer()
{
}

void LaunchpadNoteDisplayer::DrawModule()
{
}

void LaunchpadNoteDisplayer::DrawModuleUnclipped()
{
   DrawConnection(mLaunchpad);
}

void LaunchpadNoteDisplayer::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (mLaunchpad)
      mLaunchpad->DisplayNote(note.pitch, note.velocity);
}

void LaunchpadNoteDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("gridkeyboard", moduleInfo, "", FillDropdown<LaunchpadKeyboard*>);

   SetUpFromSaveData();
}

void LaunchpadNoteDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mLaunchpad = dynamic_cast<LaunchpadKeyboard*>(TheSynth->FindModule(mModuleSaveData.GetString("gridkeyboard"), false));
   if (mLaunchpad)
      mLaunchpad->SetDisplayer(this);
}
