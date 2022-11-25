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
#include "UIControlMacros.h"

NoteSorter::NoteSorter()
{
   for (int i = 0; i < kMaxDestinations; ++i)
      mPitch[i] = -1;
}

void NoteSorter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      TEXTENTRY_NUM(mPitchEntry[i], ("pitch " + ofToString(i)).c_str(), 4, &mPitch[i], -1, 127);
      mDestinationCables[i] = new AdditionalNoteCable();
      mDestinationCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
      AddPatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      ofRectangle rect = mPitchEntry[i]->GetRect(true);
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
   }
   ENDUIBLOCK(mWidth, mHeight);
   mWidth += 20;
}

void NoteSorter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
      mPitchEntry[i]->Draw();
}

void NoteSorter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   bool foundPitch = false;

   for (int i = 0; i < kMaxDestinations; ++i)
   {
      if (pitch == mPitch[i])
      {
         mDestinationCables[i]->PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
         foundPitch = true;
      }
   }

   if (!foundPitch)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteSorter::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteSorter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteSorter::TextEntryComplete(TextEntry* entry)
{
   mNoteOutput.Flush(NextBufferTime(false));
   for (int i = 0; i < kMaxDestinations; ++i)
      mDestinationCables[i]->Flush(NextBufferTime(false));
}

void NoteSorter::SetUpFromSaveData()
{
}

void NoteSorter::SaveLayout(ofxJSONElement& moduleInfo)
{
}
