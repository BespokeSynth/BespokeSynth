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
#include "FileStream.h"
#include <set>

ChordDisplayer::ChordDisplayer()
{
}

void ChordDisplayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   std::list<int> notes = mNoteOutput.GetHeldNotesList();

   if (notes.size() == 0)
      return;

   if (mAdvancedDetection)
   {
      std::vector<int> chord{ std::begin(notes), std::end(notes) };
      std::set<std::string> chordNames = TheScale->GetChordDatabase().GetChordNamesAdvanced(chord, mUseScaleDegrees, mShowIntervals);

      if (chordNames.size() <= 5)
      {
         int drawY = 14;
         int drawHeight = 20;
         for (std::string chordName : chordNames)
         {
            DrawTextNormal(chordName, 4, drawY);
            drawY += drawHeight;
         }
      }
      else
      {
         DrawTextNormal("(ambiguous)", 4, 14);
      }
   }
   else
   {
      std::vector<int> chord{ std::begin(notes), std::end(notes) };
      DrawTextNormal(TheScale->GetChordDatabase().GetChordName(chord), 4, 14);
   }
}

void ChordDisplayer::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);
}

void ChordDisplayer::GetModuleDimensions(float& width, float& height)
{
   if (mAdvancedDetection)
   {
      width = 300;
      height = 80;
   }
   else
   {
      width = 300;
      height = 20;
   }
}

void ChordDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("advanced_detection", moduleInfo, false);
   mModuleSaveData.LoadBool("use_scale_degrees", moduleInfo, false);
   mModuleSaveData.LoadBool("show_intervals", moduleInfo, false);

   SetUpFromSaveData();
}

void ChordDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mAdvancedDetection = mModuleSaveData.GetBool("advanced_detection");
   mUseScaleDegrees = mModuleSaveData.GetBool("use_scale_degrees");
   mShowIntervals = mModuleSaveData.GetBool("show_intervals");
}

void ChordDisplayer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void ChordDisplayer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
