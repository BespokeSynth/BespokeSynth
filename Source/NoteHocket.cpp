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

    NoteHocket.cpp
    Created: 19 Dec 2019 10:40:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteHocket.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

NoteHocket::NoteHocket()
{
   for (int i = 0; i < 128; ++i)
      mLastNoteDestinations[i] = -1;
}

void NoteHocket::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      FLOATSLIDER(mWeightSlider[i], ("weight " + ofToString(i)).c_str(), &mWeight[i], 0, 1);
      mDestinationCables[i] = new AdditionalNoteCable();
      mDestinationCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0));
      AddPatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      ofRectangle rect = mWeightSlider[i]->GetRect(true);
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
   }
   ENDUIBLOCK(mWidth, mHeight);
   mWidth += 20;

   GetPatchCableSource()->SetEnabled(false);
}

void NoteHocket::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
      mWeightSlider[i]->Draw();
}

void NoteHocket::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int selectedDestination = 0;
   if (velocity > 0)
   {
      ComputeSliders(0);

      float totalWeight = 0;
      for (int i = 0; i < kMaxDestinations; ++i)
         totalWeight += mWeight[i];
      float random = ofRandom(totalWeight);

      for (selectedDestination = 0; selectedDestination < kMaxDestinations; ++selectedDestination)
      {
         if (random <= mWeight[selectedDestination] || selectedDestination == kMaxDestinations - 1)
            break;
         random -= mWeight[selectedDestination];
      }

      if (mLastNoteDestinations[pitch] != -1 && mLastNoteDestinations[pitch] != selectedDestination)
         SendNoteToIndex(mLastNoteDestinations[pitch], time, pitch, 0, voiceIdx, modulation);
      mLastNoteDestinations[pitch] = selectedDestination;
   }
   else
   {
      selectedDestination = mLastNoteDestinations[pitch];
      if (selectedDestination == -1)
         return;
      mLastNoteDestinations[pitch] = -1;
   }

   SendNoteToIndex(selectedDestination, time, pitch, velocity, voiceIdx, modulation);
}

void NoteHocket::SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   mDestinationCables[index]->PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteHocket::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteHocket::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteHocket::SetUpFromSaveData()
{
}

void NoteHocket::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}
