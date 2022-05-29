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
//  Amplifier.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 7/13/13.
//
//

#include "Amplifier.h"
#include "ModularSynth.h"
#include "Profiler.h"

Amplifier::Amplifier()
: IAudioProcessor(gBufferSize)
{
}

void Amplifier::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGainSlider = new FloatSlider(this, "gain", 5, 2, 110, 15, &mGain, 0, 4);
}

Amplifier::~Amplifier()
{
}

void Amplifier::Process(double time)
{
   PROFILER(Amplifier);

   if (!mEnabled)
      return;

   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         auto getBufferChannelCh = GetBuffer()->GetChannel(ch);
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            gWorkBuffer[i] = getBufferChannelCh[i] * mGain;
         }
         Add(out->GetChannel(ch), gWorkBuffer, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gWorkBuffer, GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void Amplifier::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGainSlider->Draw();
}

void Amplifier::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Amplifier::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
