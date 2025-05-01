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
//  ModulationVisualizer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#include "ModulationVisualizer.h"
#include "SynthGlobals.h"
#include "ModulationChain.h"
#include "PolyphonyMgr.h"

ModulationVisualizer::ModulationVisualizer()
{
   mVoices.resize(kNumVoices);
}

void ModulationVisualizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   int y = 15;
   DrawTextNormal("global: " + mGlobalModulation.GetInfoString(), 3, y);
   y += 15;

   for (int i = 0; i < kNumVoices; ++i)
   {
      if (mVoices[i].mActive)
      {
         DrawTextNormal(ofToString(i) + ":" + mVoices[i].GetInfoString(), 3, y);
         y += 15;
      }
   }
}

void ModulationVisualizer::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (note.voiceIdx == -1)
   {
      mGlobalModulation.mActive = note.velocity > 0;
      mGlobalModulation.mModulators = note.modulation;
   }
   else
   {
      mVoices[note.voiceIdx].mActive = note.velocity > 0;
      mVoices[note.voiceIdx].mModulators = note.modulation;
   }
}

void ModulationVisualizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ModulationVisualizer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

std::string ModulationVisualizer::VizVoice::GetInfoString()
{
   std::string info;
   if (mModulators.pitchBend)
      info += "bend:" + ofToString(mModulators.pitchBend->GetValue(0), 2) + "  ";
   if (mModulators.modWheel)
      info += "mod:" + ofToString(mModulators.modWheel->GetValue(0), 2) + "  ";
   if (mModulators.pressure)
      info += "pressure:" + ofToString(mModulators.pressure->GetValue(0), 2) + "  ";
   info += "pan:" + ofToString(mModulators.pan, 2) + "  ";
   return info;
}

void ModulationVisualizer::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}
