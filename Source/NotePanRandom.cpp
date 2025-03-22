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

    NotePanRandom.cpp
    Created: 22 Feb 2020 10:39:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NotePanRandom.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

NotePanRandom::NotePanRandom()
{
   for (int i = 0; i < kPanHistoryDisplaySize; ++i)
      mPanHistoryDisplay[i].time = -9999;
}

void NotePanRandom::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mSpreadSlider, "spread", &mSpread, 0, 1);
   FLOATSLIDER(mCenterSlider, "center", &mCenter, -1, 1);
   ENDUIBLOCK(mWidth, mHeight);
}

void NotePanRandom::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSpreadSlider->Draw();
   mCenterSlider->Draw();

   for (int i = 0; i < kPanHistoryDisplaySize; ++i)
   {
      if (gTime - mPanHistoryDisplay[i].time > 0 && gTime - mPanHistoryDisplay[i].time < 200)
      {
         ofRectangle sliderRect = mCenterSlider->GetRect(true);
         float t = mPanHistoryDisplay[i].pan / 2 + .5f;
         ofPushStyle();
         ofSetColor(0, 255, 0, 255 * (1 - (gTime - mPanHistoryDisplay[i].time) / 200));
         ofFill();
         ofLine(sliderRect.x + t * sliderRect.width, sliderRect.y, sliderRect.x + t * sliderRect.width, sliderRect.y + sliderRect.height);
         ofPopStyle();
      }
   }
}

void NotePanRandom::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      ComputeSliders(0);
      note.modulation.pan = ofClamp(mCenter + ofRandom(-mSpread, mSpread), -1, 1);

      mPanHistoryDisplay[mPanHistoryDisplayIndex].time = note.time;
      mPanHistoryDisplay[mPanHistoryDisplayIndex].pan = note.modulation.pan;
      mPanHistoryDisplayIndex = (mPanHistoryDisplayIndex + 1) % kPanHistoryDisplaySize;
   }

   PlayNoteOutput(note);
}

void NotePanRandom::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NotePanRandom::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
