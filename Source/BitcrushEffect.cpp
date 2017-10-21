//
//  BitcrushEffect.cpp
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#include "BitcrushEffect.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"

BitcrushEffect::BitcrushEffect()
: mCrush(1)
, mDownsample(1)
, mCrushSlider(nullptr)
, mDownsampleSlider(nullptr)
{
   SetEnabled(true);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
   {
      mSampleCounter[i] = 0;
      mHeldDownsample[i] = 0;
   }
}

void BitcrushEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mCrushSlider = new FloatSlider(this,"crush",5,4,85,15,&mCrush,1,24);
   mDownsampleSlider = new FloatSlider(this,"downsamp",5,21,85,15,&mDownsample,1,40,0);
}

void BitcrushEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   Profiler profiler("BitcrushEffect");

   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);

	float bitDepth = powf(2, 25-mCrush);
	float invBitDepth = 1.f / bitDepth;

   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         if (mSampleCounter[ch] < (int)mDownsample - 1)
         {
            ++mSampleCounter[ch];
         }
         else
         {
            mHeldDownsample[ch] = buffer->GetChannel(ch)[i];
            mSampleCounter[ch] = 0;
         }
         buffer->GetChannel(ch)[i] = ((int)(mHeldDownsample[ch]*bitDepth)) * invBitDepth;
      }
   }
}

void BitcrushEffect::DrawModule()
{
   if (!mEnabled)
      return;
   
   mDownsampleSlider->Draw();
   mCrushSlider->Draw();
}

void BitcrushEffect::GetModuleDimensions(int& width, int& height)
{
   if (mEnabled)
   {
      width = 95;
      height = 39;
   }
   else
   {
      width = 95;
      height = 0;
   }
}

float BitcrushEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp((mCrush-1)/24.0f+((int)mDownsample-1)/40.0f,0,1);
}

void BitcrushEffect::CheckboxUpdated(Checkbox *checkbox)
{
}

void BitcrushEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void BitcrushEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

