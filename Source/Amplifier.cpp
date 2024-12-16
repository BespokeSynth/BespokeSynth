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

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();

   mNumChannels = GetBuffer()->NumActiveChannels();

   ChannelBuffer* out = target->GetBuffer();
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      auto getBufferChannelCh = GetBuffer()->GetChannel(ch);
      if (mEnabled)
      {
         for (int i = 0; i < bufferSize; ++i)
         {
            ComputeSliders(i);
            gWorkBuffer[i] = getBufferChannelCh[i] * mGain;
         }

         if (mShowLevelMeter)
            mLevelMeterDisplay.Process(ch, gWorkBuffer, bufferSize);

         Add(out->GetChannel(ch), gWorkBuffer, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gWorkBuffer, GetBuffer()->BufferSize(), ch);
      }
      else
      {
         if (mShowLevelMeter)
            mLevelMeterDisplay.Process(ch, getBufferChannelCh, bufferSize);

         Add(out->GetChannel(ch), getBufferChannelCh, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(getBufferChannelCh, GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void Amplifier::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mShowLevelMeter)
   {
      if (mNumChannels == 1)
      {
         mLevelMeterDisplay.Draw(3, 20, 114, 8, mNumChannels);
         mHeight = 30;
      }
      else
      {
         mLevelMeterDisplay.Draw(3, 20, 114, 18, mNumChannels);
         mHeight = 40;
      }
   }
   else
   {
      mHeight = 22;
   }

   mGainSlider->Draw();
}

void Amplifier::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("show_level_meter", moduleInfo, true);

   SetUpFromSaveData();
}

void Amplifier::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mShowLevelMeter = mModuleSaveData.GetBool("show_level_meter");
}
