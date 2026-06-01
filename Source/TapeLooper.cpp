/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  TapeLooper.cpp
//
//  Created by Ryan Challinor on 5/27/26.
//
//

#include "TapeLooper.h"

#include "Checkbox.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Transport.h"
#include "UIControlMacros.h"

TapeLooper::TapeLooper()
: IAudioProcessor(gBufferSize)
, IDrawableModule(600, 200)
, mRecordBuffer(MAX_BUFFER_SIZE)
{
}

void TapeLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   UIBLOCK_PUSHSLIDERWIDTH(150);
   RADIOBUTTON(mStateSelector, "state", (int*)&mState, 60, 3);
   FLOATSLIDER(mDisplayLengthSecondsSlider, "display seconds", &mDisplayLengthSeconds, 4, MAX_BUFFER_SIZE / gSampleRate);
   INTSLIDER(mLoopLengthBeatsSlider, "loop length beats", &mLoopLengthBeats, 1, 4 * 16);
   FLOATSLIDER(mLoopBeatsAgoSlider, "loop beats ago", &mLoopBeatsAgo, 0, 4 * 16);
   FLOATSLIDER(mDownbeatOffsetBeatsSlider, "downbeat offset", &mDownbeatOffsetBeats, 0, 4 * 16);
   UIBLOCK_NEWCOLUMN();
   BUTTON(mLoop1BarButton, "loop 1 bar");
   BUTTON(mLoop2BarsButton, "loop 2 bars");
   BUTTON(mLoop4BarsButton, "loop 4 bars");
   BUTTON(mLoop8BarsButton, "loop 8 bars");
   BUTTON(mLoop16BarsButton, "loop 16 bars");
   FLOATSLIDER(mLatencyFixMsSlider, "latency fix ms", &mLatencyFixMs, 0, 200);
   ENDUIBLOCK0();

   mStateSelector->AddLabel("capture", (int)TapeLooperState::Capture);
   mStateSelector->AddLabel("stop", (int)TapeLooperState::Stop);
   mStateSelector->AddLabel("loop", (int)TapeLooperState::Loop);
}

TapeLooper::~TapeLooper()
{
}

void TapeLooper::Process(double time)
{
   PROFILER(TapeLooper);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   mRecordBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());
   int bufferSize = GetBuffer()->BufferSize();

   ChannelBuffer* out = target->GetBuffer();

   if (mState == TapeLooperState::Capture)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         mRecordBuffer.WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }
   else if (mState == TapeLooperState::Stop)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gZeroBuffer, GetBuffer()->BufferSize(), ch);
      }
   }
   else if (mState == TapeLooperState::Loop)
   {
      double loopTime = time;
      for (int i = 0; i < bufferSize; ++i)
      {
         int samplesAgo = GetPlaybackSamplesAgo(loopTime);
         if (samplesAgo != mLastPlayedSamplesAgo - 1) //playhead jumped
            mLoopWrapSmoother.StartSwitch();
         if (samplesAgo == mLastPlayedSamplesAgo)
            ofLog() << "tapelooper: huh? repeating a sample";
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            float sample = 0;
            if (samplesAgo < mRecordBuffer.Size())
               sample = mLoopWrapSmoother.Process(ch, mRecordBuffer.GetSample(samplesAgo, ch));
            out->GetChannel(ch)[i] += sample;
            GetVizBuffer()->Write(sample, ch);
         }
         mLastPlayedSamplesAgo = samplesAgo;
         loopTime += gInvSampleRateMs;
      }
   }

   if (mState == TapeLooperState::Capture)
      mLastCaptureMeasureTime = TheTransport->GetMeasureTime(time);

   GetBuffer()->Reset();
}

int TapeLooper::GetPlaybackSamplesAgo(double time) const
{
   double loopNumMeasures = double(mLoopLengthBeats) / TheTransport->GetTimeSigTop();
   double loopMeasuresAgo = double(mLoopBeatsAgo) / TheTransport->GetTimeSigTop();
   double downbeatMeasureOffset = mDownbeatOffsetBeats / TheTransport->GetTimeSigTop() + fmod(1 - fmod(loopMeasuresAgo, 1.0f), 1.0f);
   double measuresAgo = DoubleWrap(mLastCaptureMeasureTime - TheTransport->GetMeasureTime(time) + downbeatMeasureOffset - mLatencyFixMs / TheTransport->MsPerBar(), loopNumMeasures) + loopMeasuresAgo;
   int samplesAgo = measuresAgo * TheTransport->MsPerBar() * gSampleRateMs;
   return samplesAgo;
}

void TapeLooper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushMatrix();
   ofTranslate(3, 3);
   float bufferWidth = mWidth - 6;
   float bufferHeight = mHeight - 6;
   int displaySamples = MIN(mDisplayLengthSeconds * gSampleRate, mRecordBuffer.Size());
   mRecordBuffer.Draw(0, 0, bufferWidth, bufferHeight, displaySamples);

   float lastRecordedMeasurePos = fmod(mLastCaptureMeasureTime, 1.0f);
   float displayLengthMs = displaySamples / gSampleRateMs;
   int beatsPerBar = TheTransport->GetTimeSigTop();
   float msPerBeat = TheTransport->MsPerBar() / beatsPerBar;
   int numDisplayBeats = int(displayLengthMs / msPerBeat) + 1;
   int beat = int(lastRecordedMeasurePos * beatsPerBar);
   float beatProgress = lastRecordedMeasurePos * beatsPerBar - beat;
   int displayBeat = beat;
   for (int i = 0; i < numDisplayBeats; ++i)
   {
      float msSinceBeat = (i + beatProgress) * msPerBeat;
      float x = bufferWidth - (msSinceBeat / displayLengthMs) * bufferWidth;

      float alpha = 150;
      if (msSinceBeat > displayLengthMs * .75f)
         alpha *= ofMap(msSinceBeat, displayLengthMs * .75f, displayLengthMs * .9f, 1, 0, true);

      if (displayBeat == 0)
         ofSetColor(255, 255, 0, alpha);
      else
         ofSetColor(255, 255, 255, alpha);

      ofLine(x, 0, x, bufferHeight / 3);
      ofLine(x, 0 + bufferHeight * 2 / 3, x, bufferHeight);

      --displayBeat;
      if (displayBeat < 0)
         displayBeat += beatsPerBar;
   }

   ofSetColor(255, 255, 255, 50);
   ofFill();

   float loopNumMeasures = float(mLoopLengthBeats) / TheTransport->GetTimeSigTop();
   float loopMeasuresAgo = float(mLoopBeatsAgo) / TheTransport->GetTimeSigTop();
   float downbeatMeasureOffset = mDownbeatOffsetBeats / TheTransport->GetTimeSigTop() + fmod(1 - fmod(loopMeasuresAgo, 1.0f), 1.0f);

   float msSinceLoopStart = (loopNumMeasures + loopMeasuresAgo) * TheTransport->MsPerBar();
   float loopStartX = bufferWidth - (msSinceLoopStart / displayLengthMs) * bufferWidth;
   float loopWidth = (TheTransport->MsPerBar() * loopNumMeasures / displayLengthMs) * bufferWidth;
   ofRect(loopStartX, 0, loopWidth, bufferHeight);

   float downbeatPositionMs = FloatWrap(downbeatMeasureOffset - (loopNumMeasures - mLastCaptureMeasureTime), loopNumMeasures) * TheTransport->MsPerBar();
   float downbeatPositionX = loopStartX + loopWidth - (downbeatPositionMs / displayLengthMs) * bufferWidth;
   ofLine(downbeatPositionX, 0, downbeatPositionX, bufferHeight);

   if (mState == TapeLooperState::Loop)
   {
      ofSetColor(0, 255, 0);
      float msSincePlaybackPos = GetPlaybackSamplesAgo(gTime) / gSampleRateMs;
      float playbackPosX = bufferWidth - (msSincePlaybackPos / displayLengthMs) * bufferWidth;
      ofLine(playbackPosX, 0, playbackPosX, bufferHeight);
   }

   ofPopMatrix();

   mStateSelector->Draw();
   mDisplayLengthSecondsSlider->Draw();
   mLoopLengthBeatsSlider->Draw();
   mLoopBeatsAgoSlider->Draw();
   mDownbeatOffsetBeatsSlider->Draw();
   mLoop1BarButton->Draw();
   mLoop2BarsButton->Draw();
   mLoop4BarsButton->Draw();
   mLoop8BarsButton->Draw();
   mLoop16BarsButton->Draw();
   mLatencyFixMsSlider->Draw();
}

void TapeLooper::StartLoop(int numBars)
{
   mLoopLengthBeats = numBars * TheTransport->GetTimeSigTop();
   mLoopBeatsAgo = 0;
   mDownbeatOffsetBeats = 0;
   mState = TapeLooperState::Loop;
}

void TapeLooper::SetRecording(bool record)
{
   if (record)
   {
      mState = TapeLooperState::Capture;
      mStartRecordingMeasureTime = TheTransport->GetMeasureTime(gTime);
   }
   else
   {
      if (IsRecording())
      {
         float numBars = round(TheTransport->GetMeasureTime(gTime) - mStartRecordingMeasureTime);
         if (numBars > 0)
            StartLoop(numBars);
         else
            mState = TapeLooperState::Capture;
         mStartRecordingMeasureTime = -1;
      }
   }
}

void TapeLooper::DoRetroactiveRecord(int numBars)
{
   mLoopLengthBeats = numBars * TheTransport->GetTimeSigTop();
   mState = TapeLooperState::Loop;
}

void TapeLooper::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void TapeLooper::ButtonClicked(ClickButton* button, double time)
{
   if (button == mLoop1BarButton)
      StartLoop(1);
   else if (button == mLoop2BarsButton)
      StartLoop(2);
   else if (button == mLoop4BarsButton)
      StartLoop(4);
   else if (button == mLoop8BarsButton)
      StartLoop(8);
   else if (button == mLoop16BarsButton)
      StartLoop(16);
}

void TapeLooper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void TapeLooper::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void TapeLooper::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mLastCaptureMeasureTime;
   mRecordBuffer.SaveState(out);
}

void TapeLooper::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mLastCaptureMeasureTime;
   mRecordBuffer.LoadState(in);
}