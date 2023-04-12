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

    TransposeFrom.cpp
    Created: 14 Jul 2021 7:47:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "TransposeFrom.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

TransposeFrom::TransposeFrom()
{
   TheScale->AddListener(this);
}

TransposeFrom::~TransposeFrom()
{
   TheScale->RemoveListener(this);
}

void TransposeFrom::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mRootSelector, "root", &mRoot, 50);
   CHECKBOX(mRetriggerCheckbox, "retrigger", &mRetrigger);
   ENDUIBLOCK(mWidth, mHeight);

   mRootSelector->AddLabel("A", 9);
   mRootSelector->AddLabel("A#/Bb", 10);
   mRootSelector->AddLabel("B", 11);
   mRootSelector->AddLabel("C", 0);
   mRootSelector->AddLabel("C#/Db", 1);
   mRootSelector->AddLabel("D", 2);
   mRootSelector->AddLabel("D#/Eb", 3);
   mRootSelector->AddLabel("E", 4);
   mRootSelector->AddLabel("F", 5);
   mRootSelector->AddLabel("F#/Gb", 6);
   mRootSelector->AddLabel("G", 7);
   mRootSelector->AddLabel("G#/Ab", 8);
}

void TransposeFrom::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRootSelector->Draw();
   mRetriggerCheckbox->Draw();
}

void TransposeFrom::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

int TransposeFrom::GetTransposeAmount() const
{
   return TheScale->ScaleRoot() - mRoot;
}

void TransposeFrom::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
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
         mInputNotes[pitch].mOutputPitch = pitch + GetTransposeAmount();
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }

      PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, velocity, mInputNotes[pitch].mVoiceIdx, modulation);
   }
}

void TransposeFrom::OnScaleChanged()
{
   OnRootChanged(gTime);
}

void TransposeFrom::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mRootSelector && mEnabled && mRetrigger)
      OnRootChanged(time);
}

void TransposeFrom::OnRootChanged(double time)
{
   for (int pitch = 0; pitch < 128; ++pitch)
   {
      if (mInputNotes[pitch].mOn)
      {
         PlayNoteOutput(time + .01, mInputNotes[pitch].mOutputPitch, 0, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
         mInputNotes[pitch].mOutputPitch = pitch + GetTransposeAmount();
         PlayNoteOutput(time, mInputNotes[pitch].mOutputPitch, mInputNotes[pitch].mVelocity, mInputNotes[pitch].mVoiceIdx, ModulationParameters());
      }
   }
}

void TransposeFrom::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void TransposeFrom::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
