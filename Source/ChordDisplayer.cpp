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

    ChordDisplayer.cpp
    Created: 27 Mar 2018 9:23:27pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChordDisplayer.h"
#include "SynthGlobals.h"
#include "Scale.h"

ChordDisplayer::ChordDisplayer()
{
}

void ChordDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   std::list<int> notes = mNoteOutput.GetHeldNotesList();

   if (notes.size() > 2)
   {
      std::vector<int> chord{ std::begin(notes), std::end(notes) };
      DrawTextNormal(TheScale->GetChordDatabase().GetChordName(chord), 4, 14);
   }
}

void ChordDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void ChordDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ChordDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
