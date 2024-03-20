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
/*
  ==============================================================================

    SpectralDisplay.cpp
    Created: 14 Nov 2019 10:39:24am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SpectralDisplay.h"
#include "ModularSynth.h"
#include "Profiler.h"

namespace
{
   const int kNumFFTBins = 1024;
   const int kBinIgnore = 2;
};

SpectralDisplay::SpectralDisplay()
: IAudioProcessor(gBufferSize)
, mFFT(kNumFFTBins)
, mFFTData(kNumFFTBins, kNumFFTBins / 2 + 1)
, mRollingInputBuffer(kNumFFTBins)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[kNumFFTBins];
   for (int i = 0; i < kNumFFTBins; ++i)
      mWindower[i] = -.5f * cos(FTWO_PI * i / kNumFFTBins) + .5f;
   mSmoother = new float[kNumFFTBins / 2 + 1 - kBinIgnore];
   for (int i = 0; i < kNumFFTBins / 2 + 1 - kBinIgnore; ++i)
      mSmoother[i] = 0;
}

void SpectralDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

SpectralDisplay::~SpectralDisplay()
{
   delete[] mWindower;
   delete[] mSmoother;
}

void SpectralDisplay::Process(double time)
{
   PROFILER(SpectralDisplay);

   SyncBuffers();

   if (mEnabled)
   {
      ComputeSliders(0);

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (ch == 0)
            BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         else
            Add(gWorkBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      }

      mRollingInputBuffer.WriteChunk(gWorkBuffer, GetBuffer()->BufferSize(), 0);

      //copy rolling input buffer into working buffer and window it
      mRollingInputBuffer.ReadChunk(mFFTData.mTimeDomain, kNumFFTBins, 0, 0);
      Mult(mFFTData.mTimeDomain, mWindower, kNumFFTBins);

      mFFT.Forward(mFFTData.mTimeDomain,
                   mFFTData.mRealValues,
                   mFFTData.mImaginaryValues);
   }

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void SpectralDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false || !mEnabled)
      return;

   ofPushStyle();
   ofPushMatrix();

   float w, h;
   GetDimensions(w, h);

   ofSetColor(255, 255, 255);
   ofSetLineWidth(1);

   //raw
   int end = kNumFFTBins / 2 + 1;
   ofBeginShape();
   for (int i = kBinIgnore; i < end; i++)
   {
      float x = sqrtf(float(i - kBinIgnore) / (end - kBinIgnore - 1)) * w;
      float samp = sqrtf(fabsf(mFFTData.mRealValues[i]) / end) * 3;
      float y = ofClamp(samp, 0, 1) * h;
      ofVertex(x, h - y);

      mSmoother[i - kBinIgnore] = ofLerp(mSmoother[i - kBinIgnore], samp, .1f);
   }
   ofEndShape(false);

   ofSetColor(245, 58, 135);
   ofSetLineWidth(3);

   //smoothed
   ofBeginShape();
   for (int i = kBinIgnore; i < end; i++)
   {
      float x = sqrtf(float(i - kBinIgnore) / (end - kBinIgnore - 1)) * w;
      float y = ofClamp(mSmoother[i - kBinIgnore], 0, 1) * h;
      ofVertex(x, h - y);
   }
   ofEndShape(false);

   ofPopMatrix();
   ofPopStyle();
}

void SpectralDisplay::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
   mModuleSaveData.SetInt("width", w);
   mModuleSaveData.SetInt("height", h);
}

void SpectralDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 600, 50, 2000, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, 100, 50, 2000, K(isTextField));

   SetUpFromSaveData();
}

void SpectralDisplay::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void SpectralDisplay::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
}
