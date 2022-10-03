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
//  NoteOctaver.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/27/13.
//
//

#include "NoteOctaver.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteOctaver::NoteOctaver()
{
}

void NoteOctaver::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, -4, 4);
   CHECKBOX(mRetriggerCheckbox, "retrigger", &mRetrigger);
   ENDUIBLOCK(mWidth, mHeight);
}

void NoteOctaver::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mOctaveSlider->Draw();
   mRetriggerCheckbox->Draw();
}

void NoteOctaver::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteOctaver::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (pitch >= 0 && pitch < 128)
   {
      if (velocity > 0)
      {
         mInputNotes[pitch].mOn = true;
         mInputNotes[pitch].mVelocity = velocity;
         mInputNotes[pitch].mVoiceIdx = voiceIdx;
         mInputNotes[pitch].mOutputPitch = pitch + mOctave * TheScale->GetPitchesPerOctave();
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }
   }

   PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, velocity, mInputNotes[pitch].mVoiceIdx, modulation);
}

void NoteOctaver::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mOctaveSlider && mEnabled && mRetrigger)
   {
      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(time + .01, pitch + oldVal, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
            PlayNoteOutput(time, pitch + mOctave * TheScale->GetPitchesPerOctave(), mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         }
      }
   }
}

void NoteOctaver::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteOctaver::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
