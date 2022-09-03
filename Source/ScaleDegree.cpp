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
//  ScaleDegree.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/5/16.
//
//

#include "ScaleDegree.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

ScaleDegree::ScaleDegree()
{
}

void ScaleDegree::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mScaleDegreeSelector, "degree", &mScaleDegree, 50);
   CHECKBOX(mRetriggerCheckbox, "retrigger", &mRetrigger);
   CHECKBOX(mDiatonicCheckbox, "diatonic", &mDiatonic);
   ENDUIBLOCK(mWidth, mHeight);

   mScaleDegreeSelector->AddLabel("-I", -7);
   mScaleDegreeSelector->AddLabel("-II", -6);
   mScaleDegreeSelector->AddLabel("-III", -5);
   mScaleDegreeSelector->AddLabel("-IV", -4);
   mScaleDegreeSelector->AddLabel("-V", -3);
   mScaleDegreeSelector->AddLabel("-VI", -2);
   mScaleDegreeSelector->AddLabel("-VII", -1);
   mScaleDegreeSelector->AddLabel("I", 0);
   mScaleDegreeSelector->AddLabel("II", 1);
   mScaleDegreeSelector->AddLabel("III", 2);
   mScaleDegreeSelector->AddLabel("IV", 3);
   mScaleDegreeSelector->AddLabel("V", 4);
   mScaleDegreeSelector->AddLabel("VI", 5);
   mScaleDegreeSelector->AddLabel("VII", 6);
   mScaleDegreeSelector->AddLabel("I+", 7);
}

void ScaleDegree::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mScaleDegreeSelector->Draw();
   mRetriggerCheckbox->Draw();
   mDiatonicCheckbox->Draw();
}

void ScaleDegree::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void ScaleDegree::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
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
         mInputNotes[pitch].mOutputPitch = TransformPitch(pitch);
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }

      PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, velocity, mInputNotes[pitch].mVoiceIdx, modulation);
   }
}

int ScaleDegree::TransformPitch(int pitch)
{
   if (mDiatonic)
   {
      int tone = TheScale->GetToneFromPitch(pitch);
      tone += mScaleDegree;
      return TheScale->GetPitchFromTone(tone);
   }
   else
   {
      int semitones = TheScale->GetPitchFromTone(mScaleDegree) - TheScale->ScaleRoot();
      return pitch + semitones;
   }
}

void ScaleDegree::DropdownUpdated(DropdownList* slider, int oldVal)
{
   if (slider == mScaleDegreeSelector && mEnabled && mRetrigger)
   {
      double time = gTime + gBufferSizeMs;
      for (int pitch = 0; pitch < 128; ++pitch)
      {
         if (mInputNotes[pitch].mOn)
         {
            PlayNoteOutput(time + .01, mInputNotes[pitch].mOutputPitch, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
            mInputNotes[pitch].mOutputPitch = TransformPitch(pitch);
            PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         }
      }
   }
}

void ScaleDegree::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ScaleDegree::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
