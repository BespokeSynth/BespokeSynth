//
//  Pumper.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#include "Pumper.h"
#include "Profiler.h"

Pumper::Pumper()
: mAmount(.75f)
, mAmountSlider(NULL)
, mOffset(0)
, mOffsetSlider(NULL)
, mInterval(kInterval_4n)
, mIntervalSelector(NULL)
, mPump(.5f)
, mPumpSlider(NULL)
, mLastValue(0)
{
   SetEnabled(false);

   mLFO.SetPeriod(mInterval);
   mLFO.SetType(kOsc_Saw);
}

void Pumper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAmountSlider = new FloatSlider(this,"amount",5,4,80,15,&mAmount,0,1);
   mOffsetSlider = new FloatSlider(this,"off",42,21,43,15,&mOffset,0,1,1);
   mIntervalSelector = new DropdownList(this,"interval",5,21,(int*)(&mInterval));
   mPumpSlider = new FloatSlider(this,"pump",5,39,80,15,&mPump,0,1);
   
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

void Pumper::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("Pumper");

   if (!mEnabled)
      return;

   ComputeSliders(0);

   const float smoothingTimeMs = 35;
   float smoothingOffset = smoothingTimeMs / TheTransport->GetDuration(mInterval);
   mLFO.SetOffset(mOffset + smoothingOffset);

   for (int i=0; i<bufferSize; ++i)
   {
      float value = mLastValue * .99f + mLFO.Value(i) * .01f;
      audio[i] = audio[i] * (1 - (mAmount * (1-powf(value,mPump))));
      mLastValue = value;
   }
}

void Pumper::DrawModule()
{
   if (!mEnabled)
      return;

   ofPushStyle();
   ofSetColor(0,200,0);
   ofFill();
   ofRect(5, 6, powf(mLastValue, mPump) * 80 * mAmount, 14);
   ofPopStyle();

   mAmountSlider->Draw();
   mIntervalSelector->Draw();
   mOffsetSlider->Draw();
   mPumpSlider->Draw();
}

void Pumper::GetModuleDimensions(int& width, int& height)
{
   if (mEnabled)
   {
      width = 90;
      height = 56;
   }
   else
   {
      width = 90;
      height = 0;
   }
}

float Pumper::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void Pumper::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      mLFO.SetPeriod(mInterval);
}

void Pumper::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

