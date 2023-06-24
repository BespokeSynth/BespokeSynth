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
//  BufferShuffler.cpp
//
//  Created by Ryan Challinor on 6/23/23.
//
//

#include "BufferShuffler.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

BufferShuffler::BufferShuffler()
: IAudioProcessor(gBufferSize)
, mInputBuffer(20 * 48000)
{
}

void BufferShuffler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   INTSLIDER(mNumBarsSlider, "num bars", &mNumBars, 1, 8);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mIntervalSelector, "interval", (int*)&mInterval, 40);
   ENDUIBLOCK0();

   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

BufferShuffler::~BufferShuffler()
{
}

void BufferShuffler::Process(double time)
{
   PROFILER(BufferShuffler);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   mInputBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = target->GetBuffer()->BufferSize();

   if (mEnabled)
   {
      int writePosition = GetWritePositionInSamples(time);

      for (int i = 0; i < bufferSize; ++i)
      {
         ComputeSliders(0);

         if (mPlaybackSampleStartTime != -1 && time >= mPlaybackSampleStartTime)
         {
            int numSlices = TheTransport->CountInStandardMeasure(mInterval) * mNumBars;
            float slicePos = (mPlayingSlice % numSlices) / (float)numSlices;
            mPlaybackSample = int(GetLengthInSamples() * slicePos);
            mPlaybackSampleStartTime = -1;
            mSwitchAndRamp.StartSwitch();
         }

         if (mPlaybackSampleStopTime != -1 && time >= mPlaybackSampleStopTime)
         {
            mPlaybackSample = -1;
            mPlaybackSampleStopTime = -1;
            mSwitchAndRamp.StartSwitch();
         }

         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            mInputBuffer.GetChannel(ch)[writePosition] = GetBuffer()->GetChannel(ch)[i];

            float outputSample = GetBuffer()->GetChannel(ch)[i];
            if (mPlaybackSample != -1)
               outputSample = mInputBuffer.GetChannel(ch)[mPlaybackSample];
            GetBuffer()->GetChannel(ch)[i] = mSwitchAndRamp.Process(ch, outputSample);
         }

         if (mPlaybackSample != -1)
            mPlaybackSample = (mPlaybackSample + 1) % GetLengthInSamples();

         writePosition = (writePosition + 1) % GetLengthInSamples();

         time += gInvSampleRateMs;
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void BufferShuffler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mNumBarsSlider->Draw();
   mIntervalSelector->Draw();

   ofPushMatrix();
   ofTranslate(5, 20);
   DrawAudioBuffer(190, 40, &mInputBuffer, 0, GetLengthInSamples(), mPlaybackSample == -1 ? GetWritePositionInSamples(gTime) : mPlaybackSample);
   ofPopMatrix();
}

void BufferShuffler::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mPlayingSlice = pitch;
      mPlaybackSampleStartTime = time;
      mPlaybackSampleStopTime = -1;
   }
   else
   {
      if (mPlayingSlice == pitch)
      {
         mPlayingSlice = -1;
         mPlaybackSampleStopTime = time;
      }
   }
}

int BufferShuffler::GetWritePositionInSamples(double time)
{
   return GetLengthInSamples() / mNumBars * ((TheTransport->GetMeasure(time) % mNumBars) + TheTransport->GetMeasurePos(time));
}

int BufferShuffler::GetLengthInSamples()
{
   return mNumBars * TheTransport->MsPerBar() * gSampleRateMs;
}

void BufferShuffler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BufferShuffler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
