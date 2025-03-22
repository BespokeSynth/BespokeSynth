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

    NoteSorter.cpp
    Created: 2 Aug 2021 10:32:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteSorter.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

NoteSorter::NoteSorter()
{
   std::fill(mPitch.begin(), mPitch.end(), -1);
}

void NoteSorter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteSorter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < (int)mDestinationCables.size(); ++i)
   {
      mPitchEntry[i]->Draw();
      ofRectangle rect = mPitchEntry[i]->GetRect(true);
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 15, rect.y + rect.height / 2);
   }
}

void NoteSorter::PlayNote(NoteMessage note)
{
   bool foundPitch = false;

   for (int i = 0; i < mDestinationCables.size(); ++i)
   {
      if (note.pitch == mPitch[i])
      {
         mDestinationCables[i]->PlayNoteOutput(note);
         foundPitch = true;
      }
   }

   if (!foundPitch)
      PlayNoteOutput(note);
}

void NoteSorter::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteSorter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_items", moduleInfo, 5, 1, 99, K(isTextField));
   SetUpFromSaveData();
}

void NoteSorter::TextEntryComplete(TextEntry* entry)
{
   mNoteOutput.Flush(NextBufferTime(false));
   for (int i = 0; i < mDestinationCables.size(); ++i)
      mDestinationCables[i]->Flush(NextBufferTime(false));
}

void NoteSorter::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 12 + (mDestinationCables.size() * 19);
}

void NoteSorter::SetUpFromSaveData()
{
   int numItems = mModuleSaveData.GetInt("num_items");
   int oldNumItems = (int)mDestinationCables.size();
   if (numItems > oldNumItems)
   {
      for (int i = oldNumItems; i < numItems; ++i)
      {
         mPitchEntry.push_back(new TextEntry(this, ("pitch " + ofToString(i)).c_str(), 8, 8 + i * 19, 5, &mPitch[i], -1, 127));
         auto* additionalCable = new AdditionalNoteCable();

         additionalCable->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
         AddPatchCableSource(additionalCable->GetPatchCableSource());
         mDestinationCables.push_back(additionalCable);
      }
   }
   else if (numItems < oldNumItems)
   {
      for (int i = oldNumItems - 1; i >= numItems; --i)
      {
         RemoveUIControl(mPitchEntry[i]);
         RemovePatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      }
      mPitchEntry.resize(numItems);
      mDestinationCables.resize(numItems);
   }
}

void NoteSorter::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["num_items"] = (int)mDestinationCables.size();
}
