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

    PitchPanner.cpp
    Created: 25 Mar 2018 9:57:24am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PitchPanner.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

PitchPanner::PitchPanner()
: mPitchLeft(36)
, mPitchLeftSlider(nullptr)
, mPitchRight(96)
, mPitchRightSlider(nullptr)
{
}

void PitchPanner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchLeftSlider = new IntSlider(this, "left", 4, 2, 100, 15, &mPitchLeft, 0, 127);
   mPitchRightSlider = new IntSlider(this, "right", mPitchLeftSlider, kAnchor_Below, 100, 15, &mPitchRight, 0, 127);
}

void PitchPanner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchLeftSlider->Draw();
   mPitchRightSlider->Draw();
}

void PitchPanner::PlayNote(NoteMessage note)
{
   if (mEnabled)
      note.modulation.pan = ofMap(note.pitch, mPitchLeft, mPitchRight, -1, 1);

   PlayNoteOutput(note);
}

void PitchPanner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PitchPanner::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
