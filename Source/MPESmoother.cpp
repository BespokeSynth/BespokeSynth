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

    MPESmoother.cpp
    Created: 4 Aug 2021 9:45:26pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MPESmoother.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

MPESmoother::MPESmoother()
{
}

MPESmoother::~MPESmoother()
{
   TheTransport->RemoveAudioPoller(this);
}

void MPESmoother::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void MPESmoother::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(110);
   FLOATSLIDER(mPitchSmoothSlider, "pitch", &mPitchSmooth, 0, 1);
   FLOATSLIDER(mPressureSmoothSlider, "pressure", &mPressureSmooth, 0, 1);
   FLOATSLIDER(mModWheelSmoothSlider, "modwheel", &mModWheelSmooth, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mPitchSmoothSlider->SetMode(FloatSlider::kSquare);
   mPressureSmoothSlider->SetMode(FloatSlider::kSquare);
   mModWheelSmoothSlider->SetMode(FloatSlider::kSquare);
}

void MPESmoother::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchSmoothSlider->Draw();
   mPressureSmoothSlider->Draw();
   mModWheelSmoothSlider->Draw();
}

void MPESmoother::PlayNote(NoteMessage note)
{
   if (mEnabled && note.voiceIdx >= 0 && note.voiceIdx < kNumVoices)
   {
      mModulationInput[note.voiceIdx].pitchBend = note.modulation.pitchBend;
      if (note.velocity > 0 && note.modulation.pitchBend != nullptr)
         mModulationOutput[note.voiceIdx].mPitchBend.SetValue(note.modulation.pitchBend->GetValue(0));
      note.modulation.pitchBend = &mModulationOutput[note.voiceIdx].mPitchBend;

      mModulationInput[note.voiceIdx].pressure = note.modulation.pressure;
      if (note.velocity > 0 && note.modulation.pressure != nullptr)
         mModulationOutput[note.voiceIdx].mPressure.SetValue(note.modulation.pressure->GetValue(0));
      note.modulation.pressure = &mModulationOutput[note.voiceIdx].mPressure;

      mModulationInput[note.voiceIdx].modWheel = note.modulation.modWheel;
      if (note.velocity > 0 && note.modulation.modWheel != nullptr)
         mModulationOutput[note.voiceIdx].mModWheel.SetValue(note.modulation.modWheel->GetValue(0));
      note.modulation.modWheel = &mModulationOutput[note.voiceIdx].mModWheel;
   }

   PlayNoteOutput(note);
}

void MPESmoother::OnTransportAdvanced(float amount)
{
   const float kSmoothTime = 100;
   for (int i = 0; i < kNumVoices; ++i)
   {
      if (mModulationInput[i].pitchBend != nullptr)
         mModulationOutput[i].mPitchBend.RampValue(gTime, mModulationOutput[i].mPitchBend.GetValue(0), mModulationInput[i].pitchBend->GetValue(0), (amount * TheTransport->MsPerBar() * (mPitchSmooth * kSmoothTime)));
      if (mModulationInput[i].pressure != nullptr)
         mModulationOutput[i].mPressure.RampValue(gTime, mModulationOutput[i].mPressure.GetValue(0), mModulationInput[i].pressure->GetValue(0), (amount * TheTransport->MsPerBar() * (mPressureSmooth * kSmoothTime)));
      if (mModulationInput[i].modWheel != nullptr)
         mModulationOutput[i].mModWheel.RampValue(gTime, mModulationOutput[i].mModWheel.GetValue(0), mModulationInput[i].modWheel->GetValue(0), (amount * TheTransport->MsPerBar() * (mModWheelSmooth * kSmoothTime)));
   }
}

void MPESmoother::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void MPESmoother::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void MPESmoother::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void MPESmoother::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
