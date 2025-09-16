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
//  NoteFlusher.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/23/14.
//
//

#include "NoteFlusher.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteFlusher::NoteFlusher()
{
}

void NoteFlusher::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFlushButton = new ClickButton(this, "flush", 5, 2);
}

void NoteFlusher::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mFlushButton->Draw();
}

void NoteFlusher::ButtonClicked(ClickButton* button, double time)
{
   if (button == mFlushButton)
   {
      mNoteOutput.Flush(time);
      for (int i = 0; i < 127; ++i)
         mNoteOutput.PlayNote(NoteMessage(time, i, 0));
   }
}

void NoteFlusher::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteFlusher::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
