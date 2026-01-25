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

    ChordBounds.cpp
    Created: 4 Jan 2024 5:31:53pm
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "ChordBounds.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ChordBounds::ChordBounds()
{
}

void ChordBounds::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

ChordBounds::~ChordBounds()
{
   TheTransport->RemoveAudioPoller(this);
}

void ChordBounds::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float w, h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w / 2 - 15, h + 3);
   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);

   mPatchCableSource2 = new AdditionalNoteCable();
   mPatchCableSource2->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
   mPatchCableSource2->GetPatchCableSource()->SetManualPosition(w / 2 + 15, h + 3);
   mPatchCableSource2->GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);
   this->AddPatchCableSource(mPatchCableSource2->GetPatchCableSource());
}

void ChordBounds::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void ChordBounds::PlayNote(NoteMessage note)
{
   mActiveNotes[note.pitch] = note;
}

void ChordBounds::OnTransportAdvanced(float amount)
{
   // detect min and max notes
   int minNotePlaying = -1;
   int maxNotePlaying = -1;
   for (int i = 0; i < 128; ++i)
   {
      if (mActiveNotes[i].velocity)
      {
         if (minNotePlaying == -1)
            minNotePlaying = i;
         maxNotePlaying = i;
      }
   }

   if (minNotePlaying > -1 && mNoteMin != minNotePlaying)
   {
      // new low note
      mNoteMin = minNotePlaying;
      mNoteOutput.Flush(gTime);
      mActiveNotes[mNoteMin].time = gTime;
      PlayNoteOutput(mActiveNotes[mNoteMin]);
   }

   if (maxNotePlaying > -1 && mNoteMax != maxNotePlaying)
   {
      // new high note
      mNoteMax = maxNotePlaying;
      mPatchCableSource2->Flush(gTime);
      mActiveNotes[mNoteMax].time = gTime;
      mPatchCableSource2->PlayNoteOutput(mActiveNotes[mNoteMax]);
   }

   if (minNotePlaying == -1 && mNoteMin > -1)
   {
      // no more low notes
      mNoteOutput.Flush(NextBufferTime(false));
      mNoteMin = -1;
   }

   if (maxNotePlaying == -1 && mNoteMax > -1)
   {
      // no more high notes
      mPatchCableSource2->Flush(NextBufferTime(false));
      mNoteMax = -1;
   }
}

void ChordBounds::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void ChordBounds::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ChordBounds::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ChordBounds::SetUpFromSaveData()
{
}
