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
//  AudioMeter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/18/15.
//
//

#include "AudioMeter.h"
#include "Profiler.h"
#include "ModularSynth.h"

AudioMeter::AudioMeter()
: IAudioProcessor(gBufferSize)
{
   mAnalysisBuffer = new float[gBufferSize];

   for (size_t i = 0; i < mLevelMeters.size(); ++i)
   {
      mLevelMeters[i].mPeakTrackerSlow.SetDecayTime(3);
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(1);
   }
}

void AudioMeter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLevelSlider = new FloatSlider(this, "level", 5, 2, 82, 15, &mLevel, 0, mMaxLevel);
   mVUCheckbox = new Checkbox(this, "vu", 90, 2, &mVUMode);
}

AudioMeter::~AudioMeter()
{
   delete[] mAnalysisBuffer;
}

void AudioMeter::Process(double time)
{
   PROFILER(AudioMeter);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   Clear(mAnalysisBuffer, gBufferSize);

   IAudioReceiver* target = GetTarget();
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      float* chBuf = GetBuffer()->GetChannel(ch);
      if (target)
         Add(target->GetBuffer()->GetChannel(ch), chBuf, gBufferSize);
      Add(mAnalysisBuffer, chBuf, gBufferSize);
      GetVizBuffer()->WriteChunk(chBuf, gBufferSize, ch);

      mLevelMeters[ch].mPeakTracker.Process(chBuf, gBufferSize);
      mLevelMeters[ch].mPeakTrackerSlow.Process(chBuf, gBufferSize);
   }

   mPeakTracker.Process(mAnalysisBuffer, gBufferSize);
   mLevel = sqrtf(mPeakTracker.GetPeak());

   mNumChannels = GetBuffer()->NumActiveChannels();

   GetBuffer()->Reset();
}

void AudioMeter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mLevelSlider->Draw();
   mVUCheckbox->Draw();

   if (!mVUMode)
   {
      mHeight = 22;
      return;
   }

   //int numChannels = GetBuffer()->NumActiveChannels();
   // for some reason this is more stable than calling GetBuffer()->NumActiveChannels(); here? idk
   int numChannels = mNumChannels;

   if (numChannels == 1)
       mHeight = 32;
   else
      mHeight = 42;

   const int kNumSegments = 20;
   const int kPaddingOutside = 3;
   const int kPaddingBetween = 1;
   const int kBarHeight = 8;
   const float kSegmentWidth = (mWidth - kPaddingOutside * 2) / kNumSegments;
   const float kOffsetY = 20;
   
   for (int i = 0; i < numChannels; ++i)
   {
      for (int j = 0; j < kNumSegments; ++j)
      {
         ofPushStyle();
         ofFill();
         float level = mLevelMeters[i].mPeakTracker.GetPeak() / mLimit;
         float slowLevel = mLevelMeters[i].mPeakTrackerSlow.GetPeak() / mLimit;
         ofColor color(0, 255, 0);
         if (j > kNumSegments - 3)
            color.set(255, 0, 0);
         else if (j > kNumSegments - 6)
            color.set(255, 255, 0);

         if (slowLevel > 0 && ofClamp(int(slowLevel * kNumSegments), 0, kNumSegments - 1) == j)
            ofSetColor(color);
         else if (level > 0 && level >= j / (float)kNumSegments)
            ofSetColor(color * .9f);
         else
            ofSetColor(color * .5f);
         ofRect(kPaddingOutside + kSegmentWidth * j, kOffsetY + i * (kBarHeight + 2), kSegmentWidth - kPaddingBetween, kBarHeight, 0);
         ofPopStyle();
      }

      if (mLevelMeters[i].mPeakTrackerSlow.GetLastHitLimitTime() > gTime - 1000)
      {
         ofPushStyle();
         ofSetColor(ofColor::red);
         DrawTextBold("clipped", kPaddingOutside + 10, kOffsetY + i * (kBarHeight + 2) + 8, 12.0f);
         ofPopStyle();
      }
   }
}

void AudioMeter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("maxlevel", moduleInfo, 1);
   mModuleSaveData.LoadFloat("limit", moduleInfo, 1, 0, 1000, K(isTextField));
   mModuleSaveData.LoadBool("vu", moduleInfo, false);

   SetUpFromSaveData();
}

void AudioMeter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mMaxLevel = mModuleSaveData.GetFloat("maxlevel");
   mLevelSlider->SetExtents(0, mMaxLevel);
   mLimit = mModuleSaveData.GetFloat("limit");
   mVUMode = mModuleSaveData.GetBool("vu");

   for (size_t i = 0; i < mLevelMeters.size(); ++i)
      mLevelMeters[i].mPeakTrackerSlow.SetLimit(mLimit);
}
