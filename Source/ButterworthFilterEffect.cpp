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
//  ButterworthFilterEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#include "ButterworthFilterEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"
#include "UIControlMacros.h"

ButterworthFilterEffect::ButterworthFilterEffect()
: mDryBuffer(gBufferSize)
{
}

void ButterworthFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mFSlider, "F", &mF, 10, 4000);
   FLOATSLIDER_DIGITS(mQSlider, "Q", &mQ, 0, 1, 3);
   ENDUIBLOCK(mWidth, mHeight);

   mFSlider->SetMaxValueDisplay("inf");
   mFSlider->SetMode(FloatSlider::kSquare);
}

ButterworthFilterEffect::~ButterworthFilterEffect()
{
}

void ButterworthFilterEffect::Init()
{
   IDrawableModule::Init();
}

void ButterworthFilterEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(ButterworthFilterEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();
   mDryBuffer.SetNumActiveChannels(buffer->NumActiveChannels());

   const float fadeOutStart = mFSlider->GetMax() * .75f;
   const float fadeOutEnd = mFSlider->GetMax();
   bool fadeOut = mF > fadeOutStart;
   if (fadeOut)
      mDryBuffer.CopyFrom(buffer);

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);
      if (mCoefficientsHaveChanged)
      {
         mButterworth[0].Set(mF, mQ);
         for (int ch = 1; ch < buffer->NumActiveChannels(); ++ch)
            mButterworth[ch].CopyCoeffFrom(mButterworth[0]);
         mCoefficientsHaveChanged = false;
      }
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] = mButterworth[ch].Run(buffer->GetChannel(ch)[i]);
   }

   if (fadeOut)
   {
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float dryness = ofMap(mF, fadeOutStart, fadeOutEnd, 0, 1);
         Mult(buffer->GetChannel(ch), 1 - dryness, bufferSize);
         Mult(mDryBuffer.GetChannel(ch), dryness, bufferSize);
         Add(buffer->GetChannel(ch), mDryBuffer.GetChannel(ch), bufferSize);
      }
   }
}

void ButterworthFilterEffect::DrawModule()
{
   mFSlider->Draw();
   mQSlider->Draw();
}

float ButterworthFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp(1 - (mF / (mFSlider->GetMax() * .75f)), 0, 1);
}

void ButterworthFilterEffect::ResetFilter()
{
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
      mButterworth[i].Clear();
}

void ButterworthFilterEffect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void ButterworthFilterEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      ResetFilter();
   }
}

void ButterworthFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mFSlider || slider == mQSlider)
      mCoefficientsHaveChanged = true;
}

void ButterworthFilterEffect::LoadLayout(const ofxJSONElement& info)
{
   mModuleSaveData.LoadFloat("f_min", info, 10, 1, 40000, K(isTextField));
   mModuleSaveData.LoadFloat("f_max", info, 10000, 1, 40000, K(isTextField));
}

void ButterworthFilterEffect::SetUpFromSaveData()
{
   mFSlider->SetExtents(mModuleSaveData.GetFloat("f_min"), mModuleSaveData.GetFloat("f_max"));
   ResetFilter();
}

void ButterworthFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}
