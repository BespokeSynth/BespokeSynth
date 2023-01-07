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
//  Pumper.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#include "Pumper.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "ModularSynth.h"

namespace
{
   const int kAdsrTime = 10000;
}

Pumper::Pumper()
{
   mAdsr.SetNumStages(2);

   mAdsr.GetStageData(0).time = kAdsrTime * .05f;
   mAdsr.GetStageData(0).target = .25f;
   mAdsr.GetStageData(0).curve = 0;
   mAdsr.GetStageData(1).time = kAdsrTime * .3f;
   mAdsr.GetStageData(1).target = 1;
   mAdsr.GetStageData(1).curve = -0.5f;

   SyncToAdsr();
}

void Pumper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mAmountSlider, "amount", &mAmount, 0, 1);
   FLOATSLIDER(mLengthSlider, "length", &mLength, 0, 1);
   FLOATSLIDER(mCurveSlider, "curve", &mAdsr.GetStageData(1).curve, -1, 1);
   FLOATSLIDER(mAttackSlider, "attack", &mAttack, 0, 1);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   ENDUIBLOCK(mWidth, mHeight);

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

Pumper::~Pumper()
{
}

void Pumper::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(Pumper);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

   double intervalPos = GetIntervalPos(time);

   ADSR::EventInfo adsrEvent(0, kAdsrTime);

   /*const float smoothingTimeMs = 35;
   float smoothingOffset = smoothingTimeMs / TheTransport->GetDuration(mInterval);
   mLFO.SetOffset(mOffset + smoothingOffset);*/

   for (int i = 0; i < bufferSize; ++i)
   {
      float adsrValue = mAdsr.Value((intervalPos + i * gInvSampleRateMs / TheTransport->GetDuration(mInterval)) * kAdsrTime, &adsrEvent);
      float value = mLastValue * .99f + adsrValue * .01f;
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= value;
      mLastValue = value;
   }
}

double Pumper::GetIntervalPos(double time)
{
   return fmod(TheTransport->GetMeasurePos(time) * TheTransport->CountInStandardMeasure(mInterval), 1.0);
}

void Pumper::DrawModule()
{
   if (!mEnabled)
      return;

   mAmountSlider->Draw();
   mLengthSlider->Draw();
   mCurveSlider->Draw();
   mAttackSlider->Draw();
   mIntervalSelector->Draw();

   ofPushStyle();
   ofSetColor(0, 200, 0, 50);
   ofFill();
   ofRect(0, mHeight * .8f, mLastValue * mWidth, mHeight * .2f);
   ofPopStyle();

   ofPushStyle();
   ofSetColor(245, 58, 135);
   ofBeginShape();
   ADSR::EventInfo adsrEvent(0, kAdsrTime);
   for (int i = 0; i < mWidth; i++)
   {
      float x = i;
      float y = mAdsr.Value(float(i) / mWidth * kAdsrTime, &adsrEvent) * mHeight;
      ofVertex(x, mHeight - y);
   }
   ofEndShape(false);

   ofSetColor(255, 255, 255, 100);
   {
      float x = GetIntervalPos(gTime) * mWidth;
      ofLine(x, 0, x, mHeight);
   }
   ofPopStyle();
}

float Pumper::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void Pumper::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void Pumper::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mAmountSlider)
   {
      mAdsr.GetStageData(0).target = 1 - mAmount;
   }
   if (slider == mLengthSlider)
   {
      mAdsr.GetStageData(1).time = (mLength + mAttack) * kAdsrTime;
   }
   if (slider == mAttackSlider)
   {
      mAdsr.GetStageData(0).time = mAttack * kAdsrTime;
      mAdsr.GetStageData(1).time = (mLength + mAttack) * kAdsrTime;
   }
}

void Pumper::SyncToAdsr()
{
   mAmount = 1 - mAdsr.GetStageData(0).target;
   mLength = (mAdsr.GetStageData(1).time - mAdsr.GetStageData(0).time) / kAdsrTime;
   mAttack = mAdsr.GetStageData(0).time / kAdsrTime;
}

void Pumper::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mAdsr.SaveState(out);
}

void Pumper::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mAdsr.LoadState(in);

   SyncToAdsr();
}
