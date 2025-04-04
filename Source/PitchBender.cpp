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
//  PitchBender.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 9/7/14.
//
//

#include "PitchBender.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

PitchBender::PitchBender()
: mBend(0)
, mBendSlider(nullptr)
, mRange(2)
//, mBendingCheckbox(this,"bending",HIDDEN_UICONTROL,HIDDEN_UICONTROL,mBendSlider->mTouching)
, mModulation(true)
{
   //mBendSlider->SetRelative(true);
}

void PitchBender::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

PitchBender::~PitchBender()
{
   TheTransport->RemoveAudioPoller(this);
}

void PitchBender::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mBendSlider = new FloatSlider(this, "bend", 5, 2, 110, 15, &mBend, -mRange, mRange);
}

void PitchBender::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mBendSlider->Draw();
}

void PitchBender::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      mModulation.GetPitchBend(note.voiceIdx)->AppendTo(note.modulation.pitchBend);
      note.modulation.pitchBend = mModulation.GetPitchBend(note.voiceIdx);
   }

   PlayNoteOutput(note);
}

void PitchBender::OnTransportAdvanced(float amount)
{
   ComputeSliders(0);
}

void PitchBender::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mBendSlider)
      mModulation.GetPitchBend(-1)->SetValue(mBend);
}

void PitchBender::CheckboxUpdated(Checkbox* checkbox, double time)
{
   /*if (checkbox == &mBendingCheckbox)
   {
      mBendSlider->UpdateTouching();
   }*/
}

void PitchBender::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("range", moduleInfo, 2, 0, 48, true);

   SetUpFromSaveData();
}

void PitchBender::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mRange = mModuleSaveData.GetFloat("range");
   mBendSlider->SetExtents(-mRange, mRange);
}
