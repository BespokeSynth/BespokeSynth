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
//  PitchDive.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#include "PitchDive.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

PitchDive::PitchDive()
: mStart(0)
, mStartSlider(nullptr)
, mTime(0)
, mTimeSlider(nullptr)
, mModulation(false)
{
}

PitchDive::~PitchDive()
{
}

void PitchDive::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mStartSlider = new FloatSlider(this, "start", 5, 2, 110, 15, &mStart, -3, 3);
   mTimeSlider = new FloatSlider(this, "time", 5, 20, 110, 15, &mTime, 0, 1000);

   mTimeSlider->SetMode(FloatSlider::kSquare);
}

void PitchDive::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mStartSlider->Draw();
   mTimeSlider->Draw();
}

void PitchDive::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0 && mStart != 0 && mTime != 0)
   {
      ComputeSliders(0);
      auto* pitchBend = mModulation.GetPitchBend(note.voiceIdx);
      pitchBend->RampValue(note.time, mStart, 0, mTime);
      pitchBend->AppendTo(note.modulation.pitchBend);
      note.modulation.pitchBend = pitchBend;
   }

   PlayNoteOutput(note);
}

void PitchDive::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void PitchDive::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void PitchDive::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PitchDive::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
