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

void ChordBounds::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0) // New note playing
   {
      // Detect previous min and max notes
      int minNotePlaying = -1;
      int maxNotePlaying = -1;
      for (int i = 0; i < 128; ++i)
      {
         if (mVelocityPlaying[i])
         {
            if (minNotePlaying == -1)
               minNotePlaying = i;
            maxNotePlaying = i;
         }
      }

      // Store the note
      mVelocityPlaying[pitch] = velocity;
      mVoiceIdxPlaying[pitch] = voiceIdx;
      mModulationParametersPlaying[pitch] = modulation;

      if (minNotePlaying == -1 || minNotePlaying > pitch)
      {
         mNoteOutput.Flush(time);
         PlayNoteOutput(NextBufferTime(false), pitch, velocity, voiceIdx, modulation);
      }
      if (maxNotePlaying == -1 || maxNotePlaying < pitch)
      {
         mPatchCableSource2->Flush(time);
         mPatchCableSource2->PlayNoteOutput(NextBufferTime(false), pitch, velocity, voiceIdx, modulation);
      }
   }
   else
   { // played note is stopped
      // Unset the note
      mVelocityPlaying[pitch] = velocity;
      mVoiceIdxPlaying[pitch] = voiceIdx;
      mModulationParametersPlaying[pitch] = modulation;

      // Detect new min and max notes
      int minNotePlaying = -1;
      int maxNotePlaying = -1;
      for (int i = 0; i < 128; ++i)
      {
         if (mVelocityPlaying[i])
         {
            if (minNotePlaying == -1)
               minNotePlaying = i;
            maxNotePlaying = i;
         }
      }

      if (minNotePlaying == -1 || minNotePlaying > pitch)
      {
         mNoteOutput.Flush(time);
         if (minNotePlaying != -1)
            PlayNoteOutput(NextBufferTime(false), minNotePlaying, mVelocityPlaying[minNotePlaying], mVoiceIdxPlaying[minNotePlaying], mModulationParametersPlaying[pitch]);
      }
      if (maxNotePlaying == -1 || maxNotePlaying < pitch)
      {
         mPatchCableSource2->Flush(time);
         if (maxNotePlaying != -1)
            mPatchCableSource2->PlayNoteOutput(NextBufferTime(false), maxNotePlaying, mVelocityPlaying[maxNotePlaying], mVoiceIdxPlaying[maxNotePlaying], mModulationParametersPlaying[pitch]);
      }
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
