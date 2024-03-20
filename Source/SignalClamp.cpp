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

    SignalClamp.cpp
    Created: 1 Dec 2019 3:24:55pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SignalClamp.h"
#include "ModularSynth.h"
#include "Profiler.h"

SignalClamp::SignalClamp()
: IAudioProcessor(gBufferSize)
{
}

void SignalClamp::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinSlider = new FloatSlider(this, "min", 5, 2, 110, 15, &mMin, -2, 2);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 110, 15, &mMax, -2, 2);
}

SignalClamp::~SignalClamp()
{
}

void SignalClamp::Process(double time)
{
   PROFILER(SignalClamp);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   ComputeSliders(0);
   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();

   ChannelBuffer* out = target->GetBuffer();
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      float* buffer = GetBuffer()->GetChannel(ch);
      if (mEnabled)
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer[i] = ofClamp(buffer[i], mMin, mMax);
         }
      Add(out->GetChannel(ch), buffer, bufferSize);
      GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void SignalClamp::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void SignalClamp::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SignalClamp::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
