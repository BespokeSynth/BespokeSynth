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
//  TremoloEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#include "TremoloEffect.h"
#include "OpenFrameworksPort.h"
#include "Profiler.h"
#include "UIControlMacros.h"

TremoloEffect::TremoloEffect()
{
   mLFO.SetPeriod(mInterval);
   mLFO.SetType(mOscType);
   Clear(mWindow, kAntiPopWindowSize);
}

void TremoloEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mAmountSlider, "amount", &mAmount, 0, 1);
   FLOATSLIDER(mOffsetSlider, "offset", &mOffset, 0, 1);
   FLOATSLIDER(mDutySlider, "duty", &mDuty, 0, 1);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 45);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mOscSelector, "osc", (int*)(&mOscType), 45);
   ENDUIBLOCK(mWidth, mHeight);

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);

   mOscSelector->AddLabel("sin", kOsc_Sin);
   mOscSelector->AddLabel("saw", kOsc_Saw);
   mOscSelector->AddLabel("-saw", kOsc_NegSaw);
   mOscSelector->AddLabel("squ", kOsc_Square);
   mOscSelector->AddLabel("tri", kOsc_Tri);
}

void TremoloEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(TremoloEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   if (mAmount > 0)
   {
      for (int i = 0; i < bufferSize; ++i)
      {
         //smooth out LFO a bit to avoid pops with square/saw LFOs
         mWindow[mWindowPos] = mLFO.Value(i + kAntiPopWindowSize / 2);
         mWindowPos = (mWindowPos + 1) % kAntiPopWindowSize;
         float lfoVal = 0;
         for (int j = 0; j < kAntiPopWindowSize; ++j)
            lfoVal += mWindow[j];
         lfoVal /= kAntiPopWindowSize;

         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            buffer->GetChannel(ch)[i] *= (1 - (mAmount * (1 - lfoVal)));
      }
   }
}

void TremoloEffect::DrawModule()
{
   if (!mEnabled)
      return;

   mAmountSlider->Draw();
   mIntervalSelector->Draw();
   mOffsetSlider->Draw();
   mOscSelector->Draw();
   mDutySlider->Draw();

   ofPushStyle();
   ofSetColor(0, 200, 0, gModuleDrawAlpha * .3f);
   ofFill();
   ofRect(5, 4, mLFO.Value() * 85 * mAmount, 14);
   ofPopStyle();
}

float TremoloEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void TremoloEffect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      mLFO.SetPeriod(mInterval);
   if (list == mOscSelector)
      mLFO.SetType(mOscType);
}

void TremoloEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void TremoloEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mOffsetSlider)
      mLFO.SetOffset(mOffset);
   if (slider == mDutySlider)
   {
      mLFO.SetPulseWidth(mDuty);
   }
}
