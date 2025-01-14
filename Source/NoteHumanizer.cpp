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

NoteHumanizer::NoteHumanizer()
{
}

NoteHumanizer::~NoteHumanizer()
{
}

void NoteHumanizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mTimeSlider = new FloatSlider(this, "time", 4, 4, 100, 15, &mTime, 0, 500);
   mVelocitySlider = new FloatSlider(this, "velocity", mTimeSlider, kAnchor_Below, 100, 15, &mVelocity, 0, 1);

   mTimeSlider->SetMode(FloatSlider::kSquare);
}

void NoteHumanizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTimeSlider->Draw();
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

   float delayMs;
   int outputVelocity;
   if (note.velocity > 0)
   {
      delayMs = ofRandom(0, mTime);
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
}

void NoteHumanizer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
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
