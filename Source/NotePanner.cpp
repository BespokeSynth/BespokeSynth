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

    NotePanner.cpp
    Created: 24 Mar 2018 8:18:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NotePanner.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NotePanner::NotePanner()
{
}

void NotePanner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this, "pan", 4, 2, 100, 15, &mPan, -1, 1);
}

void NotePanner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPanSlider->Draw();
}

void NotePanner::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      ComputeSliders(0);
      note.modulation.pan = mPan;
   }

   PlayNoteOutput(note);
}

void NotePanner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NotePanner::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
