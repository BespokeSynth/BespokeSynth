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
//  DopplerShift.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 3/26/25.
//
//

#include "DopplerShift.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

DopplerShift::DopplerShift()
{
}

DopplerShift::~DopplerShift()
{
}

void DopplerShift::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mSpeed1Slider = new FloatSlider(this, "receiver speed", 3, 2, 140, 15, &mSpeed1, -100, 100);
   mSpeed2Slider = new FloatSlider(this, "source speed", mSpeed1Slider, kAnchor_Below, 140, 15, &mSpeed2, -100, 100);
   mReverseButton = new ClickButton(this, "reverse", mSpeed2Slider, kAnchor_Below);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

void DopplerShift::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSpeed1Slider->Draw();
   mSpeed2Slider->Draw();
   mReverseButton->Draw();
}

void DopplerShift::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float DopplerShift::Value(int samplesIn)
{
   ComputeSliders(samplesIn);

   if (GetSliderTarget())
      return ofClamp((mSpeedOfSound + mSpeed1) / (mSpeedOfSound - mSpeed2), GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
   else
      return (mSpeedOfSound + mSpeed1) / (mSpeedOfSound - mSpeed2);
}

void DopplerShift::ButtonClicked(ClickButton* button, double time)
{
   mSpeed1 = -mSpeed1;
   mSpeed2 = -mSpeed2;
}

void DopplerShift::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void DopplerShift::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadFloat("speed_of_sound", moduleInfo, 343, 1, 500, K(isTextField));

   SetUpFromSaveData();
}

void DopplerShift::SetUpFromSaveData()
{
   mSpeedOfSound = mModuleSaveData.GetFloat("speed_of_sound");
}
