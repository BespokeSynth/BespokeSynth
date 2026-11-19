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
      mPreFilter[i].SetFilterParams(300, sqrt(2) / 2);
      mPreFilter[i].SetFilterType(kFilterType_Highpass);
      mPreFilter[i].UpdateFilterCoeff();

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
   DROPDOWN(mPreDistortionFilterSelector, "pre-filter", (int*)(&mPreDistortionFilter), 90);
   FLOATSLIDER(mPassFreqSlider, "passfreq", &mPreFilter[0].mF, 10, 7000);
   ENDUIBLOCK(mWidth, mHeight);

   mTypeDropdown->AddLabel("clean", (int)DistortionType::kClean);
   mTypeDropdown->AddLabel("warm", (int)DistortionType::kWarm);
   mTypeDropdown->AddLabel("dirty", (int)DistortionType::kDirty);
   mTypeDropdown->AddLabel("soft", (int)DistortionType::kSoft);
   mTypeDropdown->AddLabel("asym", (int)DistortionType::kAsymmetric);
   mTypeDropdown->AddLabel("fold", (int)DistortionType::kFold);
   mTypeDropdown->AddLabel("grungy", (int)DistortionType::kGrungy);

   mPreDistortionFilterSelector->AddLabel("no pre-filter", (int)PreDistortionFilter::kNone);
   mPreDistortionFilterSelector->AddLabel("center input", (int)PreDistortionFilter::kCenterInput);
   mPreDistortionFilterSelector->AddLabel("distort lows", (int)PreDistortionFilter::kDistortLows);
   mPreDistortionFilterSelector->AddLabel("distort highs", (int)PreDistortionFilter::kDistortHighs);

   mPassFreqSlider->SetMode(FloatSlider::kSquare);
}

void DistortionEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DistortionEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();

   for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
   {
      float* channelData = buffer->GetChannel(ch);

      if (mPreDistortionFilter == PreDistortionFilter::kCenterInput || mPreDistortionFilter == PreDistortionFilter::kDistortLows)
      {
         mDCRemover[ch].Filter(channelData, bufferSize);
      }

      if (mPreDistortionFilter == PreDistortionFilter::kDistortLows || mPreDistortionFilter == PreDistortionFilter::kDistortHighs)
      {
         BufferCopy(gWorkBuffer, channelData, bufferSize);
         mPreFilter[ch].Filter(channelData, bufferSize);
         Subtract(gWorkBuffer, channelData, bufferSize);

         for (int i = 0; i < bufferSize; ++i)
         {
            mNoDistortionLevel = std::max(mNoDistortionLevel, fabsf(gWorkBuffer[i]));
            mDistortionInputLevel = std::max(mDistortionInputLevel, fabsf(channelData[i]));
         }
      }

      mPeakTracker[ch].Process(channelData, bufferSize);

      if (mType == DistortionType::kDirty)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            channelData[i] = (ofClamp((channelData[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -1, 1)) / mGain;
         }
      }
      else if (mType == DistortionType::kClean)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            channelData[i] = tanh((channelData[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain) / mGain;
         }
      }
      else if (mType == DistortionType::kWarm)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            channelData[i] = sin((channelData[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain) / mGain;
         }
      }
      else if (mType == DistortionType::kGrungy)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            channelData[i] = asin(ofClamp((channelData[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -1, 1)) / mGain;
         }
      }
      //soft and asymmetric from http://www.music.mcgill.ca/~gary/courses/projects/618_2009/NickDonaldson/#Distortion
      else if (mType == DistortionType::kSoft)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = (channelData[i] + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain;
            if (sample > 1)
               sample = .66666f;
            else if (sample < -1)
               sample = -.66666f;
            else
               sample = sample - (sample * sample * sample) / 3.0f;
            channelData[i] = sample / mGain;
         }
      }
      else if (mType == DistortionType::kAsymmetric)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = (channelData[i] * .5f + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain;
            if (sample >= .320018f)
               sample = .630035f;
            else if (sample >= -.08905f)
               sample = -6.153f * sample * sample + 3.9375f * sample;
            else if (sample >= -1)
               sample = -.75f * (1 - powf(1 - (fabsf(sample) - .032847f), 12) + .333f * (fabsf(sample) - .032847f)) + .01f;
            else
               sample = -.9818f;
            channelData[i] = sample / mGain;
         }
      }
      else if (mType == DistortionType::kFold)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            float sample = ofClamp((channelData[i] * .5f + mFuzzAmount * mPeakTracker[ch].GetPeak()) * mPreamp * mGain, -100, 100);
            while (sample > 1 || sample < -1)
            {
               if (sample > 1)
                  sample = 2 - sample;
               if (sample < -1)
                  sample = -2 - sample;
            }
            channelData[i] = sample / mGain;
         }
      }

      if (mPreDistortionFilter == PreDistortionFilter::kDistortLows || mPreDistortionFilter == PreDistortionFilter::kDistortHighs)
         Add(channelData, gWorkBuffer, bufferSize);
   }
}

void DistortionEffect::SetClip(float amount)
{
   mClip = amount;
   mGain = 1 / pow(amount, 3);
}

void DistortionEffect::DrawModule()
{
   if (!IsEnabled())
      return;

   mPreDistortionFilterSelector->Draw();
   mPassFreqSlider->SetShowing(mPreDistortionFilter == PreDistortionFilter::kDistortLows || mPreDistortionFilter == PreDistortionFilter::kDistortHighs);
   mPassFreqSlider->Draw();
   mClipSlider->Draw();
   mTypeDropdown->Draw();
   mPreampSlider->Draw();
   mFuzzAmountSlider->Draw();

   if (mPreDistortionFilter == PreDistortionFilter::kDistortLows || mPreDistortionFilter == PreDistortionFilter::kDistortHighs)
   {
      ofRectangle passFreqRect = mPassFreqSlider->GetRect(K(local));

      float span = passFreqRect.width - 4;
      float xStart = passFreqRect.x + 2;
      float xEnd = passFreqRect.x + passFreqRect.width - 2;

      ofPushStyle();
      ofSetLineWidth(2);
      if (mPreDistortionFilter == PreDistortionFilter::kDistortLows)
      {
         ofSetColor(ofColor::red);
         ofLine(xStart, passFreqRect.y + 2, xStart + span * mDistortionInputLevel, passFreqRect.y + 2);
         ofSetColor(ofColor::green);
         ofLine(xEnd, passFreqRect.y + passFreqRect.height - 2, xEnd - span * mNoDistortionLevel, passFreqRect.y + passFreqRect.height - 2);
      }
      else
      {
         ofSetColor(ofColor::green);
         ofLine(xStart, passFreqRect.y + 2, xStart + span * mNoDistortionLevel, passFreqRect.y + 2);
         ofSetColor(ofColor::red);
         ofLine(xEnd, passFreqRect.y + passFreqRect.height - 2, xEnd - span * mDistortionInputLevel, passFreqRect.y + passFreqRect.height - 2);
      }
      ofPopStyle();

      mNoDistortionLevel = 0;
      mDistortionInputLevel = 0;
   }
}

float DistortionEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp((mPreamp - 1) / 10 + (1 - mClip), 0, 1);
}

void DistortionEffect::UpdateFilter()
{
   mPreFilter[0].UpdateFilterCoeff();
   for (int ch = 1; ch < ChannelBuffer::kMaxNumChannels; ++ch)
   {
      mPreFilter[ch].mF = mPreFilter[0].mF;
      mPreFilter[ch].mType = mPreFilter[0].mType;
      mPreFilter[ch].CopyCoeffFrom(mPreFilter[0]);
   }
}

void DistortionEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mPreFilter[i].Clear();
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mDCRemover[i].Clear();
   }
}

void DistortionEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mClipSlider)
      SetClip(mClip);
   if (slider == mPassFreqSlider)
      UpdateFilter();
}

void DistortionEffect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mPreDistortionFilterSelector)
   {
      if (mPreDistortionFilter == PreDistortionFilter::kDistortLows)
      {
         mPreFilter[0].mType = FilterType::kFilterType_Lowpass;
         UpdateFilter();
      }
      if (mPreDistortionFilter == PreDistortionFilter::kDistortHighs)
      {
         mPreFilter[0].mType = FilterType::kFilterType_Highpass;
         UpdateFilter();
      }
   }
}

bool DistortionEffect::LoadOldControl(FileStreamIn& in, std::string& oldName)
{
   if (oldName == "center input")
   {
      //load checkbox
      int checkboxRev;
      in >> checkboxRev;
      float checkboxVal;
      in >> checkboxVal;
      if (checkboxVal > 0)
         mPreDistortionFilter = PreDistortionFilter::kCenterInput;
      return true;
   }

   return false;
}
