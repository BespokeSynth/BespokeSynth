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

    NoteStrummer.cpp
    Created: 2 Apr 2018 9:27:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteStrummer.h"
#include "SynthGlobals.h"
#include "Scale.h"

NoteStrummer::NoteStrummer()
{
}

void NoteStrummer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

NoteStrummer::~NoteStrummer()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteStrummer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mStrumSlider = new FloatSlider(this, "strum", 4, 4, 192, 15, &mStrum, 0, 1);
}

void NoteStrummer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mStrumSlider->Draw();

   const int numNotes = static_cast<int>(mNotes.size());
   int i = 0;
   for (const auto pitch : mNotes)
   {
      const double pos = static_cast<double>(i + .5f) / numNotes;
      DrawTextNormal(NoteName(pitch), mStrumSlider->GetPosition(true).x + pos * mStrumSlider->IClickable::GetDimensions().x, mStrumSlider->GetPosition(true).y + mStrumSlider->IClickable::GetDimensions().y + 12);
      ++i;
   }
}

void NoteStrummer::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      mNotes.push_back(note.pitch);
   }
   else
   {
      PlayNoteOutput(note.MakeNoteOff());
      mNotes.remove(note.pitch);
   }
}

void NoteStrummer::OnTransportAdvanced(float amount)
{
   int numNotes = (int)mNotes.size();

   for (int i = 0; i < gBufferSize; ++i)
   {
      ComputeSliders(i);

      int index = 0;
      for (auto pitch : mNotes)
      {
         double pos = double(index + .5) / numNotes;
         double change = mStrum - mLastStrumPos;
         double offset = pos - mLastStrumPos;
         bool wraparound = fabs(change) > .99;
         if (change * offset > 0 && //same direction
             fabs(offset) <= fabs(change) &&
             !wraparound)
            PlayNoteOutput(NoteMessage(gTime + i * gInvSampleRateMs, pitch, 127));
         ++index;
      }
      mLastStrumPos = mStrum;
   }
}

void NoteStrummer::FloatSliderUpdated(FloatSlider* slider, double oldVal, double time)
{
}

void NoteStrummer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteStrummer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
