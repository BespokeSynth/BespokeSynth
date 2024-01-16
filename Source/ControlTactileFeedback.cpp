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
//  ControlTactileFeedback.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/9/14.
//
//

#include "ControlTactileFeedback.h"
#include "IAudioReceiver.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

ControlTactileFeedback::ControlTactileFeedback()
{
   mPhaseInc = GetPhaseInc(50);
}

void ControlTactileFeedback::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "vol", 5, 43, 70, 15, &mVolume, 0, 1);
}

ControlTactileFeedback::~ControlTactileFeedback()
{
}

void ControlTactileFeedback::Process(double time)
{
   PROFILER(ControlTactileFeedback);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   for (int i = 0; i < bufferSize; ++i)
   {
      float sample = (mPhase / FTWO_PI * 2 - 1) * gControlTactileFeedback * mVolume;
      out[i] += sample;
      GetVizBuffer()->Write(sample, 0);

      mPhase += mPhaseInc;
      while (mPhase > FTWO_PI)
      {
         mPhase -= FTWO_PI;
      }

      const float decayTime = .005f;
      float decay = powf(0.5f, 1.0f / (decayTime * gSampleRate));
      gControlTactileFeedback *= decay;
      if (gControlTactileFeedback <= FLT_EPSILON)
         gControlTactileFeedback = 0;

      time += gInvSampleRateMs;
   }
}

void ControlTactileFeedback::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
}


void ControlTactileFeedback::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ControlTactileFeedback::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
