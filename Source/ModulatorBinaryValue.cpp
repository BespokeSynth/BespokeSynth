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

    ModulatorBinaryValue.cpp
    Created: 13 Aug 2025 8:00:00am
    Author:  Andrius Merkys

  ==============================================================================
*/

#include "ModulatorBinaryValue.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

ModulatorBinaryValue::ModulatorBinaryValue()
{
   for (int i = 0; i < NUM_BITS; ++i)
   {
      mBits[i].SetOwner(this);
      mBits[i].SetIndex(NUM_BITS - i - 1);
   }
}

void ModulatorBinaryValue::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 3, 134);
   FLOATSLIDER_DIGITS(mInputSlider, "input", &mInput, 0, 255, 0);
   DROPDOWN(mCodeSelector, "code", (int*)&mCode, 134);
   ENDUIBLOCK(mWidth, mHeight);

   mHeight += 20;

   mCodeSelector->AddLabel("byte", kCodeByte);
   mCodeSelector->AddLabel("gray", kCodeGray);

   for (int i = 0; i < NUM_BITS; ++i)
   {
      mBits[i].SetCableSource(new PatchCableSource(this, kConnectionType_Modulator));
      AddPatchCableSource(mBits[i].GetCableSource());
   }
}

ModulatorBinaryValue::~ModulatorBinaryValue()
{
}

void ModulatorBinaryValue::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mInputSlider->Draw();
   mCodeSelector->Draw();

   int width_per_bit = mWidth / (NUM_BITS + 1);
   for (int i = 0; i < NUM_BITS; ++i)
   {
      mBits[i].GetCableSource()->SetManualPosition(width_per_bit * (i + 1), mHeight);
      DrawTextNormal(ofToString(GetBitValue(NUM_BITS - i - 1)), width_per_bit * (i + 1) - 4, mHeight - 9);
   }
}

void ModulatorBinaryValue::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (int i = 0; i < NUM_BITS; ++i)
   {
      if (cableSource == mBits[i].GetCableSource())
         mBits[i].UpdateControl();
   }
}

int ModulatorBinaryValue::GetBitValue(int index)
{
   int value = (int)mInput;
   if (mCode == kCodeGray)
      value = value ^ (value >> 1);
   return (value & (1 << index)) != 0;
}

void ModulatorBinaryValue::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorBinaryValue::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorBinaryValue::SetUpFromSaveData()
{
}

float ModulatorBinaryValue::BitModulator::Value(int samplesIn)
{
   return mOwner->GetBitValue(mIndex);
}
