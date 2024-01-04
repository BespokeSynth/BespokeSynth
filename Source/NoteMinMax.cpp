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

    NoteMinMax.cpp
    Created: 4 Jan 2024 5:31:53pm
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "NoteMinMax.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

NoteMinMax::NoteMinMax()
{
}

void NoteMinMax::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mDestinationCables[0] = new AdditionalNoteCable();
   mDestinationCables[0]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
   mDestinationCables[0]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0));
   AddPatchCableSource(mDestinationCables[0]->GetPatchCableSource());
   mDestinationCables[0]->GetPatchCableSource()->SetManualPosition(35, 10);

   mDestinationCables[1] = new AdditionalNoteCable();
   mDestinationCables[1]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
   mDestinationCables[1]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0));
   AddPatchCableSource(mDestinationCables[1]->GetPatchCableSource());
   mDestinationCables[1]->GetPatchCableSource()->SetManualPosition(35, 25);

   GetPatchCableSource()->SetEnabled(false);
}

void NoteMinMax::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("min", 3, 15);
   DrawTextNormal("max", 3, 30);
}

void NoteMinMax::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0) // New note playing
   {
      // Detect previous min and max notes
      int minNotePlaying = -1;
      int maxNotePlaying = -1;
      for (int i = 0; i < 128; ++i)
      {
         if (mNotePlaying[i])
         {
            if (minNotePlaying == -1)
               minNotePlaying = i;
            maxNotePlaying = i;
         }
      }

      // Store the note
      mNotePlaying[pitch] = true;
      mVelocityPlaying[pitch] = velocity;
      mVoiceIdxPlaying[pitch] = voiceIdx;
      mModulationParametersPlaying[pitch] = modulation;

      if (minNotePlaying == -1 || minNotePlaying > pitch)
      {
         if (minNotePlaying != -1)
            mDestinationCables[0]->PlayNoteOutput(time, minNotePlaying, 0, -1);
         mDestinationCables[0]->PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      }
      if (maxNotePlaying == -1 || maxNotePlaying < pitch)
      {
         if (maxNotePlaying != -1)
            mDestinationCables[1]->PlayNoteOutput(time, maxNotePlaying, 0, -1);
         mDestinationCables[1]->PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      }
   } else { // played note is stopped
      // Unset the note
      mNotePlaying[pitch] = false;
      mVelocityPlaying[pitch] = velocity;
      mVoiceIdxPlaying[pitch] = voiceIdx;
      mModulationParametersPlaying[pitch] = modulation;

      // Detect new min and max notes
      int minNotePlaying = -1;
      int maxNotePlaying = -1;
      for (int i = 0; i < 128; ++i)
      {
         if (mNotePlaying[i])
         {
            if (minNotePlaying == -1)
               minNotePlaying = i;
            maxNotePlaying = i;
         }
      }

      if (minNotePlaying == -1 || minNotePlaying > pitch)
      {
         mDestinationCables[0]->PlayNoteOutput(time, pitch, 0, -1);
         if (minNotePlaying != -1)
            mDestinationCables[0]->PlayNoteOutput(time, minNotePlaying, mVelocityPlaying[minNotePlaying], mVoiceIdxPlaying[minNotePlaying], mModulationParametersPlaying[pitch]);
      }
      if (maxNotePlaying == -1 || maxNotePlaying < pitch)
      {
         mDestinationCables[1]->PlayNoteOutput(time, pitch, 0, -1);
         if (maxNotePlaying != -1)
            mDestinationCables[1]->PlayNoteOutput(time, maxNotePlaying, mVelocityPlaying[maxNotePlaying], mVoiceIdxPlaying[maxNotePlaying], mModulationParametersPlaying[pitch]);
      }
   }
}

void NoteMinMax::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteMinMax::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteMinMax::SetUpFromSaveData()
{
}

void NoteMinMax::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}
