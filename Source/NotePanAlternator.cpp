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

    NotePanAlternator.cpp
    Created: 25 Mar 2018 9:27:26pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NotePanAlternator.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NotePanAlternator::NotePanAlternator()
{
}

void NotePanAlternator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanOneSlider = new FloatSlider(this, "one", 4, 2, 100, 15, &mPanOne, -1, 1);
   mPanTwoSlider = new FloatSlider(this, "two", mPanOneSlider, kAnchor_Below, 100, 15, &mPanTwo, -1, 1);
}

void NotePanAlternator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPanOneSlider->Draw();
   mPanTwoSlider->Draw();

   ofPushStyle();
   ofSetColor(0, 255, 0, 50);
   ofFill();
   ofVec2f activePos = mFlip ? mPanTwoSlider->GetPosition(true) : mPanOneSlider->GetPosition(true);
   ofRect(activePos.x, activePos.y, 100, 15);
   ofPopStyle();
}

void NotePanAlternator::PlayNote(NoteMessage note)
{
   if (mEnabled && note.velocity > 0)
   {
      note.modulation.pan = mFlip ? mPanTwo : mPanOne;
      mFlip = !mFlip;
   }

   PlayNoteOutput(note);
}

void NotePanAlternator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NotePanAlternator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
