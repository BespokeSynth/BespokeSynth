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
//  Acciaccatura.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/25/25.
//
//

#include "Acciaccatura.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "Scale.h"

Acciaccatura::Acciaccatura()
{
}

Acciaccatura::~Acciaccatura()
{
}

void Acciaccatura::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   INTSLIDER(mOffsetSlider, "offset", &mOffset, -5, 5);
   DROPDOWN(mNoteModeDropdown, "notemode", ((int*)&mNoteMode), 80);
   FLOATSLIDER(mHoldTimeSlider, "hold ms", &mHoldTimeMs, 0, 200);
   FLOATSLIDER(mGlideTimeSlider, "glide ms", &mGlideTimeMs, 0, 200);
   ENDUIBLOCK(mWidth, mHeight);

   mNoteModeDropdown->AddLabel("scale", (int)NoteMode::Scale);
   mNoteModeDropdown->AddLabel("chromatic", (int)NoteMode::Chromatic);
   mHoldTimeSlider->SetMode(FloatSlider::kSquare);
   mGlideTimeSlider->SetMode(FloatSlider::kSquare);
}

void Acciaccatura::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mOffsetSlider->Draw();
   mNoteModeDropdown->Draw();
   mHoldTimeSlider->Draw();
   mGlideTimeSlider->Draw();
}

void Acciaccatura::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      ComputeSliders(0);
      auto* pitchBend = mModulation.GetPitchBend(note.voiceIdx);
      float bendStart;
      if (mNoteMode == NoteMode::Scale)
      {
         int tone = TheScale->GetToneFromPitch(note.pitch);
         tone += mOffset;
         int startPitch = TheScale->GetPitchFromTone(tone);
         bendStart = startPitch - note.pitch;
      }
      else
      {
         bendStart = mOffset;
      }
      pitchBend->SetValue(bendStart);
      pitchBend->RampValue(note.time + mHoldTimeMs, bendStart, 0, mGlideTimeMs);
      pitchBend->AppendTo(note.modulation.pitchBend);
      note.modulation.pitchBend = pitchBend;
   }

   PlayNoteOutput(note);
}

void Acciaccatura::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Acciaccatura::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void Acciaccatura::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Acciaccatura::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
