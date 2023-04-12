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

    MPETweaker.cpp
    Created: 6 Aug 2021 9:11:13pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MPETweaker.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

MPETweaker::MPETweaker()
{
   for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
   {
      mModulationMult.GetPitchBend(voiceIdx)->SetValue(1);
      mModulationOffset.GetPitchBend(voiceIdx)->SetValue(0);

      mModulationMult.GetPressure(voiceIdx)->SetValue(1);
      mModulationOffset.GetPressure(voiceIdx)->SetValue(0);

      mModulationMult.GetModWheel(voiceIdx)->SetValue(1);
      mModulationOffset.GetModWheel(voiceIdx)->SetValue(0);
   }
}

MPETweaker::~MPETweaker()
{
}

void MPETweaker::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(130);
   FLOATSLIDER_DIGITS(mPitchBendMultiplierSlider, "pitchbend mult", &mPitchBendMultiplier, -3, 3, 2);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mPitchBendOffsetSlider, "pitchbend offset", &mPitchBendOffset, -1, 1, 1);
   UIBLOCK_NEWLINE();
   FLOATSLIDER_DIGITS(mPressureMultiplierSlider, "pressure mult", &mPressureMultiplier, -3, 3, 2);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mPressureOffsetSlider, "pressure offset", &mPressureOffset, -1, 1, 1);
   UIBLOCK_NEWLINE();
   FLOATSLIDER_DIGITS(mModWheelMultiplierSlider, "modwheel mult", &mModWheelMultiplier, -3, 3, 2);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mModWheelOffsetSlider, "modwheel offset", &mModWheelOffset, -1, 1, 1);
   UIBLOCK_NEWLINE();
   ENDUIBLOCK(mWidth, mHeight);
}

void MPETweaker::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPitchBendMultiplierSlider->Draw();
   mPitchBendOffsetSlider->Draw();
   mPressureMultiplierSlider->Draw();
   mPressureOffsetSlider->Draw();
   mModWheelMultiplierSlider->Draw();
   mModWheelOffsetSlider->Draw();
}

void MPETweaker::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      mModulationMult.GetPitchBend(voiceIdx)->MultiplyIn(modulation.pitchBend);
      mModulationOffset.GetPitchBend(voiceIdx)->AppendTo(mModulationMult.GetPitchBend(voiceIdx));
      modulation.pitchBend = mModulationOffset.GetPitchBend(voiceIdx);

      mModulationMult.GetPressure(voiceIdx)->MultiplyIn(modulation.pressure);
      mModulationOffset.GetPressure(voiceIdx)->AppendTo(mModulationMult.GetPressure(voiceIdx));
      modulation.pressure = mModulationOffset.GetPressure(voiceIdx);

      mModulationMult.GetModWheel(voiceIdx)->MultiplyIn(modulation.modWheel);
      mModulationOffset.GetModWheel(voiceIdx)->AppendTo(mModulationMult.GetModWheel(voiceIdx));
      modulation.modWheel = mModulationOffset.GetModWheel(voiceIdx);
   }

   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void MPETweaker::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mPitchBendMultiplierSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationMult.GetPitchBend(voiceIdx)->SetValue(mPitchBendMultiplier);
   }
   if (slider == mPitchBendOffsetSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationOffset.GetPitchBend(voiceIdx)->SetValue(mPitchBendOffset);
   }

   if (slider == mPressureMultiplierSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationMult.GetPressure(voiceIdx)->SetValue(mPressureMultiplier);
   }
   if (slider == mPressureOffsetSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationOffset.GetPressure(voiceIdx)->SetValue(mPressureOffset);
   }

   if (slider == mModWheelMultiplierSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationMult.GetModWheel(voiceIdx)->SetValue(mModWheelMultiplier);
   }
   if (slider == mModWheelOffsetSlider)
   {
      for (int voiceIdx = -1; voiceIdx < kNumVoices; ++voiceIdx)
         mModulationOffset.GetModWheel(voiceIdx)->SetValue(mModWheelOffset);
   }
}

void MPETweaker::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void MPETweaker::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void MPETweaker::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
