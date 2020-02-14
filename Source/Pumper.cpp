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

Pumper::Pumper()
: mAmount(.75f)
, mAmountSlider(nullptr)
, mOffset(0)
, mOffsetSlider(nullptr)
, mInterval(kInterval_4n)
, mIntervalSelector(nullptr)
, mPump(.5f)
, mPumpSlider(nullptr)
, mLastValue(0)
{
   mLFO.SetPeriod(mInterval);
   mLFO.SetType(kOsc_Saw);
}

void Pumper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK();
   FLOATSLIDER(mAmountSlider,"amount",&mAmount,0,1);
   DROPDOWN(mIntervalSelector,"interval",(int*)(&mInterval),40); UIBLOCK_SHIFTRIGHT();
   UIBLOCK_PUSHSLIDERWIDTH(55);
   FLOATSLIDER_DIGITS(mOffsetSlider,"off",&mOffset,0,1,1); UIBLOCK_NEWLINE();
   UIBLOCK_POPSLIDERWIDTH();
   FLOATSLIDER(mPumpSlider,"pump",&mPump,0,1);
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

   const float smoothingTimeMs = 35;
   float smoothingOffset = smoothingTimeMs / TheTransport->GetDuration(mInterval);
   mLFO.SetOffset(mOffset + smoothingOffset);

   for (int i=0; i<bufferSize; ++i)
   {
      float value = mLastValue * .99f + mLFO.Value(i) * .01f;
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= (1 - (mAmount * (1-powf(value,mPump))));
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

