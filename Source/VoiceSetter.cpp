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
//  VoiceSetter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#include "VoiceSetter.h"
#include "SynthGlobals.h"

void VoiceSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mVoiceSlider = new IntSlider(this, "voice index", 5, 2, 80, 15, &mVoiceIdx, 0, kNumVoices - 1);
}

void VoiceSetter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mVoiceSlider->Draw();
}

void VoiceSetter::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mVoiceSlider)
      mNoteOutput.Flush(time);
}

void VoiceSetter::PlayNote(NoteMessage note)
{
   note.voiceIdx = mVoiceIdx;
   PlayNoteOutput(note);
}

void VoiceSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VoiceSetter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
