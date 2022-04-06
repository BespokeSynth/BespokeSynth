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

    NoteRangeFilter.cpp
    Created: 29 Jan 2020 9:18:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteRangeFilter.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteRangeFilter::NoteRangeFilter()
: mMinPitch(24)
, mMinPitchSlider(nullptr)
, mMaxPitch(36)
, mMaxPitchSlider(nullptr)
, mWrap(false)
{
   SetEnabled(true);
}

void NoteRangeFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   INTSLIDER(mMinPitchSlider, "min", &mMinPitch, 0, 127);
   INTSLIDER(mMaxPitchSlider, "max", &mMaxPitch, 0, 127);
   CHECKBOX(mWrapCheckbox, "wrap", &mWrap);
   ENDUIBLOCK(mWidth, mHeight);
}

void NoteRangeFilter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mMinPitchSlider->Draw();
   mMaxPitchSlider->Draw();
   mWrapCheckbox->Draw();
}

void NoteRangeFilter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteRangeFilter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mMinPitchSlider || slider == mMaxPitchSlider)
      mNoteOutput.Flush(gTime);
}

void NoteRangeFilter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   ComputeSliders(0);

   if (mWrap && mMaxPitch > mMinPitch)
   {
      int length = mMaxPitch - mMinPitch + 1;
      while (pitch < mMinPitch)
         pitch += length;
      while (pitch > mMaxPitch)
         pitch -= length;
   }

   if (!mEnabled || (pitch >= mMinPitch && pitch <= mMaxPitch))
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void NoteRangeFilter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteRangeFilter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
