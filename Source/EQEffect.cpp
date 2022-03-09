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
//  EQEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/14.
//
//

#include "EQEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

EQEffect::EQEffect()
: mNumFilters(NUM_EQ_FILTERS)
, mMultiSlider(nullptr)
, mEvenButton(nullptr)
{
   SetEnabled(true);
   
   for (int ch=0; ch<ChannelBuffer::kMaxNumChannels; ++ch)
   {
      for (int i=0; i<NUM_EQ_FILTERS; ++i)
      {
         mBanks[ch].mBiquad[i].SetFilterType(kFilterType_Peak);
         mBanks[ch].mBiquad[i].SetFilterParams(40 * powf(2.2f,i), .1f);
      }
   }
}

void EQEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMultiSlider = new UIGrid("uigrid", 5,25,80,50,NUM_EQ_FILTERS,1, this);
   AddUIControl(mMultiSlider);
   mEvenButton = new ClickButton(this,"even",5,5);
   
   mMultiSlider->SetGridMode(UIGrid::kMultislider);
   for (int i=0; i<NUM_EQ_FILTERS; ++i)
      mMultiSlider->SetValRefactor(0, i, .5f);
   mMultiSlider->SetListener(this);
}

EQEffect::~EQEffect()
{
}

void EQEffect::Init()
{
   IDrawableModule::Init();
}

void EQEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(EQEffect);
   
   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();
   
   ComputeSliders(0);
   
   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
   {
      for (int i=0; i<mNumFilters; ++i)
         mBanks[ch].mBiquad[i].Filter(buffer->GetChannel(ch),bufferSize);
   }
}

void EQEffect::DrawModule()
{
   mMultiSlider->Draw();
   mEvenButton->Draw();
}

float EQEffect::GetEffectAmount()
{
   if (mEnabled)
   {
      float amount = 0;
      for (int i=0; i<mNumFilters; ++i)
      {
         amount += fabsf(mMultiSlider->GetVal(i,0) - .5f);
      }
      return amount;
   }
   return 0;
}

void EQEffect::GetModuleDimensions(float& width, float& height)
{
   width = 90;
   height = 80;
}

void EQEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void EQEffect::RadioButtonUpdated(RadioButton* list, int oldVal)
{
}

void EQEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int ch=0; ch<ChannelBuffer::kMaxNumChannels; ++ch)
      {
         for (int i=0; i<NUM_EQ_FILTERS; ++i)
            mBanks[ch].mBiquad[i].Clear();
      }
   }
}

void EQEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void EQEffect::ButtonClicked(ClickButton* button)
{
   if (button == mEvenButton)
   {
      for (int ch=0; ch<ChannelBuffer::kMaxNumChannels; ++ch)
      {
         for (int i=0; i<NUM_EQ_FILTERS; ++i)
         {
            mMultiSlider->SetVal(i, 0, .5f);
            mBanks[ch].mBiquad[i].mDbGain = 0;
            if (ch == 0)
               mBanks[0].mBiquad[i].UpdateFilterCoeff();
            else
               mBanks[ch].mBiquad[i].CopyCoeffFrom(mBanks[0].mBiquad[i]);
         }
      }
   }
}

void EQEffect::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   for (int ch=0; ch<ChannelBuffer::kMaxNumChannels; ++ch)
   {
      for (int i=0; i<mNumFilters; ++i)
      {
         mBanks[ch].mBiquad[i].mDbGain = ofMap(mMultiSlider->GetVal(i,0),0,1,-12,12);
         if (ch == 0)
            mBanks[0].mBiquad[i].UpdateFilterCoeff();
         else
            mBanks[ch].mBiquad[i].CopyCoeffFrom(mBanks[0].mBiquad[i]);
      }
   }
}
