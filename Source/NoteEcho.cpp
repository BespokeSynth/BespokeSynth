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

    NoteEcho.cpp
    Created: 29 March 2022
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteEcho.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

NoteEcho::NoteEcho()
{
   for (int i = 0; i < kMaxDestinations; ++i)
      mDelay[i] = i * .125f;
}

void NoteEcho::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      FLOATSLIDER(mDelaySlider[i], ("delay " + ofToString(i)).c_str(), &mDelay[i], 0, 1);
      mDestinationCables[i] = new AdditionalNoteCable();
      mDestinationCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
      AddPatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      ofRectangle rect = mDelaySlider[i]->GetRect(true);
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
   }
   ENDUIBLOCK(mWidth, mHeight);
   mWidth += 20;

   GetPatchCableSource()->SetEnabled(false);
}

void NoteEcho::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
      mDelaySlider[i]->Draw();
}

void NoteEcho::PlayNote(NoteMessage note)
{
   ComputeSliders(0);

   for (int i = 0; i < kMaxDestinations; ++i)
   {
      double delayMs = mDelay[i] / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();
      NoteMessage delayedNote = note.MakeClone();
      delayedNote.time += delayMs;
      SendNoteToIndex(i, delayedNote);
   }
}

void NoteEcho::SendNoteToIndex(int index, NoteMessage note)
{
   mDestinationCables[index]->PlayNoteOutput(note);
}

void NoteEcho::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteEcho::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteEcho::SetUpFromSaveData()
{
}

void NoteEcho::SaveLayout(ofxJSONElement& moduleInfo)
{
}
