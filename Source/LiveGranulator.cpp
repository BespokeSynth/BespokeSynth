//
//  LiveGranulator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 10/2/13.
//
//

#include "LiveGranulator.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "UIControlMacros.h"

LiveGranulator::LiveGranulator()
: mBufferLength(gSampleRate*5)
, mBuffer(mBufferLength)
, mGranOverlap(nullptr)
, mGranPosRandomize(nullptr)
, mGranSpeed(nullptr)
, mGranSpeedRandomize(nullptr)
, mGranLengthMs(nullptr)
, mFreeze(false)
, mFreezeCheckbox(nullptr)
, mGranOctaveCheckbox(nullptr)
, mFreezeExtraSamples(0)
, mPos(0)
, mPosSlider(nullptr)
, mAdd(false)
, mAddCheckbox(nullptr)
, mAutoCaptureInterval(kInterval_None)
, mAutoCaptureDropdown(nullptr)
, mGranSpacingRandomize(nullptr)
{
   mGranulator.SetLiveMode(true);
   mGranulator.mSpeed = 1;
   mGranulator.mGrainOverlap = 3;
   mGranulator.mGrainLengthMs = 100;
   
   TheTransport->AddListener(this, kInterval_None, OffsetInfo(0, true), false);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      mDCEstimate[i] = 0;
}

void LiveGranulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(80);
   FLOATSLIDER(mGranOverlap,"overlap",&mGranulator.mGrainOverlap,.5f,MAX_GRAINS);
   FLOATSLIDER(mGranSpeed,"speed",&mGranulator.mSpeed,-3,3);
   FLOATSLIDER(mGranLengthMs,"len ms",&mGranulator.mGrainLengthMs,1,200);
   CHECKBOX(mAddCheckbox,"add",&mAdd);
   DROPDOWN(mAutoCaptureDropdown,"autocapture",(int*)(&mAutoCaptureInterval), 45);
   UIBLOCK_NEWCOLUMN();
   FLOATSLIDER(mGranPosRandomize,"pos r",&mGranulator.mPosRandomizeMs,0,200);
   FLOATSLIDER(mGranSpeedRandomize,"spd r",&mGranulator.mSpeedRandomize,0,.3f);
   FLOATSLIDER(mGranSpacingRandomize,"spa r",&mGranulator.mSpacingRandomize,0,1);
   CHECKBOX(mFreezeCheckbox,"frz",&mFreeze); UIBLOCK_SHIFTX(35);
   CHECKBOX(mGranOctaveCheckbox,"g oct",&mGranulator.mOctaves); UIBLOCK_NEWLINE();
   FLOATSLIDER(mPosSlider,"pos",&mPos,-gSampleRate,gSampleRate);
   ENDUIBLOCK(mWidth, mHeight);
   
   mAutoCaptureDropdown->AddLabel("none", kInterval_None);
   mAutoCaptureDropdown->AddLabel("4n", kInterval_4n);
   mAutoCaptureDropdown->AddLabel("8n", kInterval_8n);
   mAutoCaptureDropdown->AddLabel("16n", kInterval_16n);
   
   mGranPosRandomize->SetMode(FloatSlider::kSquare);
   mGranSpeedRandomize->SetMode(FloatSlider::kSquare);
   mGranLengthMs->SetMode(FloatSlider::kSquare);
}

LiveGranulator::~LiveGranulator()
{
   TheTransport->RemoveListener(this);
}

void LiveGranulator::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(LiveGranulator);
   
   float bufferSize = buffer->BufferSize();
   mBuffer.SetNumChannels(buffer->NumActiveChannels());

   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);
      
      mGranulator.SetLiveMode(!mFreeze);
      if (!mFreeze)
      {
         for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
            mBuffer.Write(buffer->GetChannel(ch)[i], ch);
      }
      else if (mFreezeExtraSamples < FREEZE_EXTRA_SAMPLES_COUNT)
      {
         ++mFreezeExtraSamples;
         for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
            mBuffer.Write(buffer->GetChannel(ch)[i], ch);
      }
      
      if (mEnabled)
      {
         float sample[ChannelBuffer::kMaxNumChannels];
         Clear(sample, ChannelBuffer::kMaxNumChannels);
         mGranulator.Process(time, mBuffer.GetRawBuffer(), mBufferLength, mBuffer.GetRawBufferOffset(0)-mFreezeExtraSamples-1+mPos, sample);
         for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         {
            sample[ch] -= mDCEstimate[ch];
         
            if (mAdd)
               buffer->GetChannel(ch)[i] += sample[ch];
            else
               buffer->GetChannel(ch)[i] = sample[ch];
            
            mDCEstimate[ch] = .999f*mDCEstimate[ch] + .001f*buffer->GetChannel(ch)[i]; //rolling average
         }
      }
      
      time += gInvSampleRateMs;
   }
}



void LiveGranulator::DrawModule()
{
   if (!mEnabled)
      return;
   
   mGranOverlap->Draw();
   mGranSpeed->Draw();
   mGranLengthMs->Draw();
   mGranPosRandomize->Draw();
   mGranSpeedRandomize->Draw();
   mFreezeCheckbox->Draw();
   mGranOctaveCheckbox->Draw();
   mPosSlider->Draw();
   mAddCheckbox->Draw();
   mAutoCaptureDropdown->Draw();
   mGranSpacingRandomize->Draw();
   if (mEnabled)
   {
      mGranulator.Draw(0,0,170,32,0,mBufferLength);
      //mBuffer.Draw(0,0,170,32);
   }
}

float LiveGranulator::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp(.5f+fabsf(mGranulator.mSpeed-1),0,1);
}

void LiveGranulator::Freeze()
{
   mFreeze = true;
   mFreezeExtraSamples = 0;
}

void LiveGranulator::OnTimeEvent(double time)
{
   Freeze();
}

void LiveGranulator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mBuffer.ClearBuffer();
   if (checkbox == mFreezeCheckbox)
   {
      mPos = 0;
      mFreezeExtraSamples = 0;
      if (mFreeze)
      {
         mEnabled = true;
         Freeze();
      }
   }
}

void LiveGranulator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mAutoCaptureDropdown)
   {
      TheTransport->UpdateListener(this, mAutoCaptureInterval);
      if (mAutoCaptureInterval == kInterval_None)
      {
         mFreeze = false;
         mPos = 0;
         mFreezeExtraSamples = 0;
      }
   }
}

void LiveGranulator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mPosSlider)
   {
      if (!mFreeze)
         mPos = 0;
   }
}

