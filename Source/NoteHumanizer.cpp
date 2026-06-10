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

    NoteHumanizer.cpp
    Created: 2 Nov 2016 7:56:21pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteHumanizer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NoteHumanizer::NoteHumanizer()
{
}

NoteHumanizer::~NoteHumanizer()
{
}

void NoteHumanizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mTimeMsSlider, "time ms", &mTimeMs, 0, 500);
   FLOATSLIDER(mStrumMsSlider, "strum ms", &mStrumMs, 0, 500);
   FLOATSLIDER(mVelocitySlider, "velocity", &mVelocity, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mTimeMsSlider->SetMode(FloatSlider::kSquare);
   mStrumMsSlider->SetMode(FloatSlider::kSquare);
}

void NoteHumanizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTimeMsSlider->Draw();
   mStrumMsSlider->Draw();
   mVelocitySlider->Draw();
}

void NoteHumanizer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteHumanizer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   float delayMs = 0;
   int outputVelocity;
   if (note.velocity > 0)
   {
      float msSinceLastNote = note.time - mLastNotePlayTimeMs;
      if (msSinceLastNote < mStrumMs)
         delayMs += mStrumMs - msSinceLastNote;
      delayMs += ofRandom(0, mTimeMs);
      outputVelocity = ofClamp((note.velocity / 127.0f * ofRandom(1 - mVelocity, 1 + mVelocity)) * 127, 1, 127);
      mLastDelayMs[note.pitch] = delayMs;
   }
   else
   {
      delayMs = mLastDelayMs[note.pitch];
      outputVelocity = 0;
   }

   note.time += delayMs;
   note.velocity = outputVelocity;
   PlayNoteOutput(note);

   if (note.velocity > 0)
      mLastNotePlayTimeMs = note.time;
}

void NoteHumanizer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void NoteHumanizer::UpdateOldControlName(std::string& oldName)
{
   IDrawableModule::UpdateOldControlName(oldName);

   if (oldName == "time")
      oldName = "time ms";
}

void NoteHumanizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteHumanizer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
