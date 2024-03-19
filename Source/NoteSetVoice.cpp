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
//  NoteSetVoice.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#include "NoteSetVoice.h"
#include "SynthGlobals.h"

void NoteSetVoice::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mVoiceSlider = new IntSlider(this, "Voice Index", 5, 2, 80, 15, &mVoiceIdx, 0, kMaxJuceMidiChannels - 1);
}

void NoteSetVoice::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVoiceSlider->Draw();
}

void NoteSetVoice::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mVoiceSlider)
      mNoteOutput.Flush(time);
}

void NoteSetVoice::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   voiceIdx = mVoiceIdx;

   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteSetVoice::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteSetVoice::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void NoteSetVoice::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}
