//
//  DistortionEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "DistortionEffect.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"

DistortionEffect::DistortionEffect()
: mType(kClean)
, mTypeDropdown(NULL)
, mClipSlider(NULL)
, mPreamp(1.0f)
, mPreampSlider(NULL)
, mDCAdjust(0)
{
   SetClip(.5f);
   
   mDCRemover.SetFilterParams(10, 1);
   mDCRemover.SetFilterType(kFilterType_Highpass);
   mDCRemover.UpdateFilterCoeff();
}

void DistortionEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTypeDropdown = new DropdownList(this,"type", 5, 40, (int*)(&mType));
   mClipSlider = new FloatSlider(this,"clip",5,20,80,15,&mClip,0.001f,1);
   mPreampSlider = new FloatSlider(this,"preamp",5,4,80,15,&mPreamp,1,10);
   
   mTypeDropdown->AddLabel("clean", kClean);
   mTypeDropdown->AddLabel("warm", kWarm);
   mTypeDropdown->AddLabel("dirty", kDirty);
   mTypeDropdown->AddLabel("soft", kSoft);
   mTypeDropdown->AddLabel("asym", kAsymmetric);
}

void DistortionEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("DistortionEffect");

   if (!mEnabled)
      return;

   ComputeSliders(0);
   
   mDCRemover.Filter(audio, bufferSize);
   
   if (mType == kDirty)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         audio[i] = (ofClamp((audio[i]+mDCAdjust) * mPreamp * mGain, -1, 1)) / mGain;
      }
   }
   else if (mType == kClean)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         audio[i] = tanh((audio[i]+mDCAdjust) * mPreamp * mGain) / mGain;
      }
   }
   else if (mType == kWarm)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         audio[i] = sin((audio[i]+mDCAdjust) * mPreamp * mGain) / mGain;
      }
   }
   //soft and asymmetric from http://www.music.mcgill.ca/~gary/courses/projects/618_2009/NickDonaldson/#Distortion
   else if (mType == kSoft)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         float sample = (audio[i]+mDCAdjust) * mPreamp * mGain;
         if (sample > 1)
            sample = .66666f;
         else if (sample < -1)
            sample = -.66666f;
         else
            sample = sample - (sample*sample*sample)/3.0f;
         audio[i] = sample / mGain;
      }
   }
   else if (mType == kAsymmetric)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         float sample = (audio[i]*.5f+mDCAdjust) * mPreamp * mGain;
         if (sample >= .320018f)
            sample = .630035f;
         else if (sample >= -.08905f)
            sample = -6.153f*sample*sample + 3.9375f*sample;
         else if (sample >= -1)
            sample = -.75f*(1-powf(1-(fabsf(sample)-.032847f),12)+.333f*(fabsf(sample)-.032847f))+.01f;
         else
            sample = -.9818f;
         audio[i] = sample / mGain;
      }
   }
}

void DistortionEffect::SetClip(float amount)
{
   mClip = amount;
   mGain = 1/pow(amount,3);
}

void DistortionEffect::GetModuleDimensions(int& width, int& height)
{
   if (mEnabled)
   {
      width = 90;
      height = 60;
   }
   else
   {
      width = 90;
      height = 0;
   }
}

void DistortionEffect::DrawModule()
{
   
   
   if (!Enabled())
      return;
   
   mClipSlider->Draw();
   mTypeDropdown->Draw();
   mPreampSlider->Draw();
}

float DistortionEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp((mPreamp-1)/10+(1-mClip),0,1);
}

void DistortionEffect::CheckboxUpdated(Checkbox* checkbox)
{
}

void DistortionEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mClipSlider)
      SetClip(mClip);
}

