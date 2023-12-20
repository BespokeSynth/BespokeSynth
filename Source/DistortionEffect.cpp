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
#include "UIControlMacros.h"

DistortionEffect::DistortionEffect()
{
   SetClip(1);

   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mDCRemover[i].SetFilterParams(10, sqrt(2) / 2);
      mDCRemover[i].SetFilterType(kFilterType_Highpass);
      mDCRemover[i].UpdateFilterCoeff();

      mPeakTracker[i].SetDecayTime(.1f);
   }
}

void DistortionEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mTypeDropdown, "type", (int*)(&mType), 50);
   FLOATSLIDER(mClipSlider, "clip", &mClip, 0.001f, 1);
   FLOATSLIDER(mPreampSlider, "preamp", &mPreamp, 1, 10);
   FLOATSLIDER(mFuzzAmountSlider, "fuzz", &mFuzzAmount, -1, 1);
   CHECKBOX(mRemoveInputDCCheckbox, "center input", &mRemoveInputDC);
   ENDUIBLOCK(mWidth, mHeight);

   mTypeDropdown->AddLabel("clean", kClean);
   mTypeDropdown->AddLabel("warm", kWarm);
   mTypeDropdown->AddLabel("dirty", kDirty);
   mTypeDropdown->AddLabel("soft", kSoft);
   mTypeDropdown->AddLabel("asym", kAsymmetric);
   mTypeDropdown->AddLabel("fold", kFold);
   mTypeDropdown->AddLabel("grungy", kGrungy);
}

void DistortionEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DistortionEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
   {
      if (mRemoveInputDC)
         mDCRemover[ch].Filter(buffer->GetChannel(ch), bufferSize);

      mPeakTracker[ch].Process(buffer->GetChannel(ch), bufferSize);

      if (mType == kDirty)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer->GetChannel(ch)[i] = (ofClamp((buffer->GetChannel(ch)[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -1, 1)) / mGain;
         }
      }
      else if (mType == kClean)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer->GetChannel(ch)[i] = tanh((buffer->GetChannel(ch)[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain) / mGain;
         }
      }
      else if (mType == kWarm)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer->GetChannel(ch)[i] = sin((buffer->GetChannel(ch)[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain) / mGain;
         }
      }
      else if (mType == kGrungy)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer->GetChannel(ch)[i] = asin(ofClamp((buffer->GetChannel(ch)[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -1, 1)) / mGain;
         }
      }
      //soft and asymmetric from http://www.music.mcgill.ca/~gary/courses/projects/618_2009/NickDonaldson/#Distortion
      else if (mType == kSoft)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = (buffer->GetChannel(ch)[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain;
            if (sample > 1)
               sample = .66666f;
            else if (sample < -1)
               sample = -.66666f;
            else
               sample = sample - (sample * sample * sample) / 3.0f;
            buffer->GetChannel(ch)[i] = sample / mGain;
         }
      }
      else if (mType == kAsymmetric)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = (buffer->GetChannel(ch)[i] * .5f + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain;
            if (sample >= .320018f)
               sample = .630035f;
            else if (sample >= -.08905f)
               sample = -6.153f * sample * sample + 3.9375f * sample;
            else if (sample >= -1)
               sample = -.75f * (1 - powf(1 - (fabsf(sample) - .032847f), 12) + .333f * (fabsf(sample) - .032847f)) + .01f;
            else
               sample = -.9818f;
            buffer->GetChannel(ch)[i] = sample / mGain;
         }
      }
      else if (mType == kFold)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = ofClamp((buffer->GetChannel(ch)[i] * .5f + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -100, 100);
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
   mGain = 1 / pow(amount, 3);
}

void DistortionEffect::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void DistortionEffect::DrawModule()
{
   if (!IsEnabled())
      return;

   mClipSlider->Draw();
   mTypeDropdown->Draw();
   mPreampSlider->Draw();
   mRemoveInputDCCheckbox->Draw();
   mFuzzAmountSlider->Draw();
}

float DistortionEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp((mPreamp - 1) / 10 + (1 - mClip), 0, 1);
}

void DistortionEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mDCRemover[i].Clear();
   }
}

void DistortionEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mClipSlider)
      SetClip(mClip);
}
