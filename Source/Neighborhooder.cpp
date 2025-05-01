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
//  Neighborhooder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/10/13.
//
//

#include "Neighborhooder.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

Neighborhooder::Neighborhooder()
{
}

void Neighborhooder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinSlider = new IntSlider(this, "min", 4, 3, 84, 15, &mMinPitch, 0, 127);
   mRangeSlider = new IntSlider(this, "range", mMinSlider, kAnchor_Below, 116, 15, &mPitchRange, 12, 36);
}

void Neighborhooder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMinSlider->Draw();
   mRangeSlider->Draw();

   DrawTextNormal(NoteName(mMinPitch) + ofToString(mMinPitch / 12 - 2), 91, 15);
}

void Neighborhooder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void Neighborhooder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mMinSlider || slider == mRangeSlider)
      mNoteOutput.Flush(time);
}

void Neighborhooder::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   while (note.pitch >= mMinPitch + mPitchRange)
      note.pitch -= TheScale->GetPitchesPerOctave();
   while (note.pitch < mMinPitch)
      note.pitch += TheScale->GetPitchesPerOctave();
   PlayNoteOutput(note);
}

void Neighborhooder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Neighborhooder::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
