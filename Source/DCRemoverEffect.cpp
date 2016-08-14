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
   
   mBiquad.SetFilterParams(10, 1);
   mBiquad.SetFilterType(kFilterType_Highpass);
   mBiquad.UpdateFilterCoeff();
}

DCRemoverEffect::~DCRemoverEffect()
{
}

void DCRemoverEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("DCRemoverEffect");
   
   if (!mEnabled)
      return;
   
   mBiquad.Filter(audio, bufferSize);
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

void DCRemoverEffect::GetModuleDimensions(int& width, int& height)
{
   width = 30;
   height = 0;
}
