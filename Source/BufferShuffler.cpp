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
            int numSlices = GetNumSlices();
            float slicePos = (mQueuedSlice % numSlices) / (float)numSlices;
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

   DrawBuffer(5, 20, mWidth - 10, mHeight - 28);
}

void BufferShuffler::DrawBuffer(float x, float y, float w, float h)
{
   ofPushMatrix();
   ofTranslate(x, y);
   DrawAudioBuffer(w, h, &mInputBuffer, 0, GetLengthInSamples(), -1);
   ofPopMatrix();

   ofPushStyle();
   ofFill();

   float writePosX = x + GetWritePositionInSamples(gTime) / (float)GetLengthInSamples() * w;
   ofSetColor(200, 200, 200);
   ofCircle(writePosX, y, 3);
   if (mPlaybackSample != -1)
   {
      float playPosX = x + mPlaybackSample / (float)GetLengthInSamples() * w;
      ofSetColor(0, 255, 0);
      ofLine(playPosX, y, playPosX, y + h);
   }

   ofSetColor(255, 255, 255, 35);
   int numSlices = GetNumSlices();
   for (int i = 0; i < numSlices; i += 2)
      ofRect(x + i * w / numSlices, y, w / numSlices, h);

   ofPopStyle();
}

void BufferShuffler::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mQueuedSlice = pitch;
      mPlaybackSampleStartTime = time;
      mPlaybackSampleStopTime = -1;
   }
   else
   {
      if (mQueuedSlice == pitch)
      {
         mQueuedSlice = -1;
         mPlaybackSampleStopTime = time;
      }
   }
}

int BufferShuffler::GetNumSlices()
{
   return TheTransport->CountInStandardMeasure(mInterval) * mNumBars;
}

void BufferShuffler::OnClicked(float x, float y, bool right)
{
   if (!right && x >= 5 && x <= mWidth - 5 && y > 20)
   {
      float bufferWidth = mWidth - 10;
      float pos = (x - 5) / bufferWidth;
      int slice = int(pos * GetNumSlices());
      PlayOneShot(slice);
   }
}

void BufferShuffler::PlayOneShot(int slice)
{
   mQueuedSlice = slice;
   double sliceSizeMs = TheTransport->GetMeasureFraction(mInterval) * TheTransport->MsPerBar();
   double currentTime = NextBufferTime(false);
   double remainderMs;
   TransportListenerInfo timeInfo(nullptr, mInterval, OffsetInfo(0, false), false);
   TheTransport->GetQuantized(currentTime, &timeInfo, &remainderMs);
   double timeUntilNextInterval = sliceSizeMs - remainderMs;
   mPlaybackSampleStartTime = currentTime + timeUntilNextInterval;
   mPlaybackSampleStopTime = mPlaybackSampleStartTime + sliceSizeMs;
}

int BufferShuffler::GetWritePositionInSamples(double time)
{
   return GetLengthInSamples() / mNumBars * ((TheTransport->GetMeasure(time) % mNumBars) + TheTransport->GetMeasurePos(time));
}

int BufferShuffler::GetLengthInSamples()
{
   return mNumBars * TheTransport->MsPerBar() * gSampleRateMs;
}

bool BufferShuffler::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;
         int index = x + y * 8;

         if (index < GetNumSlices())
            PlayOneShot(index);

         return true;
      }
   }

   return false;
}

void BufferShuffler::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor;
         int index = x + y * 8;
         int writeSlice = GetWritePositionInSamples(gTime) * GetNumSlices() / GetLengthInSamples();
         int playSlice = mPlaybackSample * GetNumSlices() / GetLengthInSamples();
         if (index < GetNumSlices())
         {
            if (index == mQueuedSlice && mPlaybackSampleStartTime != -1)
               pushColor = 32;
            else if (mPlaybackSample >= 0 && index == playSlice)
               pushColor = 126;
            else if (mPlaybackSample == -1 && index == writeSlice)
               pushColor = 120;
            else
               pushColor = 16;
         }
         else
         {
            pushColor = 0;
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

bool BufferShuffler::DrawToPush2Screen()
{
   DrawBuffer(250, 10, 400, 60);
   return false;
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
