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

    NoteChance.cpp
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteChance.h"
#include "SynthGlobals.h"

NoteChance::NoteChance()
: mChance(1)
, mLastRejectTime(0)
, mLastAcceptTime(0)
{
}

NoteChance::~NoteChance()
{
}

void NoteChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChanceSlider = new FloatSlider(this, "chance", 3, 2, 100, 15, &mChance, 0, 1);
}

void NoteChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mChanceSlider->Draw();

   if (gTime - mLastAcceptTime > 0 && gTime - mLastAcceptTime < 200)
   {
      ofPushStyle();
      ofSetColor(0, 255, 0, 255 * (1 - (gTime - mLastAcceptTime) / 200));
      ofFill();
      ofRect(106, 2, 10, 7);
      ofPopStyle();
   }

   if (gTime - mLastRejectTime > 0 && gTime - mLastRejectTime < 200)
   {
      ofPushStyle();
      ofSetColor(255, 0, 0, 255 * (1 - (gTime - mLastRejectTime) / 200));
      ofFill();
      ofRect(106, 9, 10, 7);
      ofPopStyle();
   }
}

void NoteChance::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (velocity > 0)
      ComputeSliders(0);

   bool accept = ofRandom(1) <= mChance;
   if (accept || velocity == 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (velocity > 0)
   {
      if (accept)
         mLastAcceptTime = time;
      else
         mLastRejectTime = time;
   }
}

void NoteChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = 20;
}

void NoteChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
