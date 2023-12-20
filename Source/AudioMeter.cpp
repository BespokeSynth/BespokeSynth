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
}

void AudioMeter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLevelSlider = new FloatSlider(this, "level", 5, 2, 110, 15, &mLevel, 0, mMaxLevel);
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
      if (target)
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      Add(mAnalysisBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   mPeakTracker.Process(mAnalysisBuffer, gBufferSize);
   mLevel = sqrtf(mPeakTracker.GetPeak());

   GetBuffer()->Reset();
}

void AudioMeter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mLevelSlider->Draw();
}

void AudioMeter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("maxlevel", moduleInfo, 1);

   SetUpFromSaveData();
}

void AudioMeter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mMaxLevel = mModuleSaveData.GetFloat("maxlevel");
   mLevelSlider->SetExtents(0, mMaxLevel);
}
