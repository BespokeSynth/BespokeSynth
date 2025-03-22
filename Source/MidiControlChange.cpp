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

    MidiControlChange.cpp
    Created: 5 Aug 2021 9:32:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MidiControlChange.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

MidiControlChange::MidiControlChange()
{
}

void MidiControlChange::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   TEXTENTRY_NUM(mControlEntry, "control", 4, &mControl, 0, 127);
   FLOATSLIDER(mValueSlider, "value", &mValue, 0, 127);
   ENDUIBLOCK(mWidth, mHeight);
}

void MidiControlChange::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mControlEntry->Draw();
   mValueSlider->Draw();
}

void MidiControlChange::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);
}

void MidiControlChange::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mValueSlider && mEnabled)
   {
      if (int(oldVal) != int(mValue) || mResendDuplicateValue)
         SendCCOutput(mControl, int(mValue));
   }
}

void MidiControlChange::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("resend_duplicate_value", moduleInfo, false);

   SetUpFromSaveData();
}

void MidiControlChange::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mResendDuplicateValue = mModuleSaveData.GetBool("resend_duplicate_value");
}
