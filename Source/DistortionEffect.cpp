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
, mTypeDropdown(nullptr)
, mClipSlider(nullptr)
, mPreamp(1.0f)
, mPreampSlider(nullptr)
, mDCAdjust(0)
{
   SetClip(.5f);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
   {
      mDCRemover[i].SetFilterParams(10, 1);
      mDCRemover[i].SetFilterType(kFilterType_Highpass);
      mDCRemover[i].UpdateFilterCoeff();
   }
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
   mTypeDropdown->AddLabel("fold", kFold);
}

void DistortionEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DistortionEffect);

   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();

   ComputeSliders(0);
   
   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
   {
      mDCRemover[ch].Filter(buffer->GetChannel(ch), bufferSize);
      
      if (mType == kDirty)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            buffer->GetChannel(ch)[i] = (ofClamp((buffer->GetChannel(ch)[i]+mDCAdjust) * mPreamp * mGain, -1, 1)) / mGain;
         }
      }
      else if (mType == kClean)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            buffer->GetChannel(ch)[i] = tanh((buffer->GetChannel(ch)[i]+mDCAdjust) * mPreamp * mGain) / mGain;
         }
      }
      else if (mType == kWarm)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            buffer->GetChannel(ch)[i] = sin((buffer->GetChannel(ch)[i]+mDCAdjust) * mPreamp * mGain) / mGain;
         }
      }
      //soft and asymmetric from http://www.music.mcgill.ca/~gary/courses/projects/618_2009/NickDonaldson/#Distortion
      else if (mType == kSoft)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = (buffer->GetChannel(ch)[i]+mDCAdjust) * mPreamp * mGain;
            if (sample > 1)
               sample = .66666f;
            else if (sample < -1)
               sample = -.66666f;
            else
               sample = sample - (sample*sample*sample)/3.0f;
            buffer->GetChannel(ch)[i] = sample / mGain;
         }
      }
      else if (mType == kAsymmetric)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = (buffer->GetChannel(ch)[i]*.5f+mDCAdjust) * mPreamp * mGain;
            if (sample >= .320018f)
               sample = .630035f;
            else if (sample >= -.08905f)
               sample = -6.153f*sample*sample + 3.9375f*sample;
            else if (sample >= -1)
               sample = -.75f*(1-powf(1-(fabsf(sample)-.032847f),12)+.333f*(fabsf(sample)-.032847f))+.01f;
            else
               sample = -.9818f;
            buffer->GetChannel(ch)[i] = sample / mGain;
         }
      }
      else if (mType == kFold)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = (buffer->GetChannel(ch)[i]*.5f+mDCAdjust) * mPreamp * mGain;
            while (sample > 1 || sample < -1)
            {
               if (sample > 1)
                  sample = 2 - sample;
               if (sample < -1)
                  sample = -2 - sample;
            }
            buffer->GetChannel(ch)[i] = sample / mGain;
         }
      }
   }
}

void DistortionEffect::SetClip(float amount)
{
   mClip = amount;
   mGain = 1/pow(amount,3);
}

void DistortionEffect::GetModuleDimensions(float& width, float& height)
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

