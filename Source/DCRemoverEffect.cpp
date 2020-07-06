//
//  DCRemoverEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/2/14.
//
//

#include "DCRemoverEffect.h"
#include "Profiler.h"

DCRemoverEffect::DCRemoverEffect()
{
   SetEnabled(true);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
   {
      mBiquad[i].SetFilterParams(10, 1);
      mBiquad[i].SetFilterType(kFilterType_Highpass);
      mBiquad[i].UpdateFilterCoeff();
   }
}

DCRemoverEffect::~DCRemoverEffect()
{
}

void DCRemoverEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DCRemoverEffect);
   
   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();
   
   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
      mBiquad[ch].Filter(buffer->GetChannel(ch), bufferSize);
}

void DCRemoverEffect::DrawModule()
{
}

float DCRemoverEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return 1;
}

void DCRemoverEffect::GetModuleDimensions(float& width, float& height)
{
   width = 30;
   height = 0;
}
