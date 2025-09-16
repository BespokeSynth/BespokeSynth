/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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

   LatencyCalculator.cpp
   Created: 25 Jun 2025
   Author:  Ryan Challinor

  ==============================================================================
*/

#include "LatencyCalculator.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "FillSaveDropdown.h"

LatencyCalculatorSender::LatencyCalculatorSender()
{
}

void LatencyCalculatorSender::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   BUTTON(mTestButton, "test");
   ENDUIBLOCK0();
}

LatencyCalculatorSender::~LatencyCalculatorSender()
{
}

void LatencyCalculatorSender::Process(double time)
{
   PROFILER(LatencyCalculatorSender);

   IAudioReceiver* target = GetTarget();

   if (target)
   {
      int bufferSize = target->GetBuffer()->BufferSize();
      float* out = target->GetBuffer()->GetChannel(0);
      assert(bufferSize == gBufferSize);

      for (int i = 0; i < bufferSize; ++i)
      {
         if (mTestRequested && time > mTestRequestTime + 500)
         {
            mPhase = FPI / 2.0f;
            mTestStartTime = time;
            if (mReceiver != nullptr)
               mReceiver->OnTestStarted(time, i);
            mTestRequested = false;
         }

         const float kVolume = 1.0f;
         float sample = sin(mPhase) * MAX(1.0 - (time - mTestStartTime) / 10.0f, 0) * kVolume;
         out[i] = sample;
         GetVizBuffer()->Write(sample, 0);

         mPhase += GetPhaseInc(440);
         while (mPhase > FTWO_PI)
         {
            mPhase -= FTWO_PI;
         }

         time += gInvSampleRateMs;
      }
   }
}

void LatencyCalculatorSender::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mTestButton->Draw();

   if (mReceiver)
   {
      if (mTestRequested)
         DrawTextNormal("testing...", 40, 15);
   }
   else
   {
      ofPushStyle();
      ofSetColor(255, 0, 0);
      DrawTextNormal("no receiver!", 40, 15);
      ofPopStyle();
   }
}

void LatencyCalculatorSender::ButtonClicked(ClickButton* button, double time)
{
   if (button == mTestButton)
   {
      if (mReceiver != nullptr)
         mReceiver->OnTestRequested();
      mTestRequestTime = time;
      mTestRequested = true;
   }
}

void LatencyCalculatorSender::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("receiver", moduleInfo, "", FillDropdown<LatencyCalculatorReceiver*>);

   SetUpFromSaveData();
}

void LatencyCalculatorSender::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IDrawableModule* receiver = TheSynth->FindModule(mModuleSaveData.GetString("receiver"), false);
   mReceiver = dynamic_cast<LatencyCalculatorReceiver*>(receiver);
}

LatencyCalculatorReceiver::LatencyCalculatorReceiver()
: IAudioProcessor(gBufferSize)
{
}

void LatencyCalculatorReceiver::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

LatencyCalculatorReceiver::~LatencyCalculatorReceiver()
{
}

void LatencyCalculatorReceiver::Process(double time)
{
   PROFILER(LatencyCalculatorReceiver);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   float* in = GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   for (int i = 0; i < bufferSize; ++i)
   {
      if (mState == State::Testing)
      {
         if (fabsf(in[i]) > .01f)
         {
            mState = State::DisplayResult;
            mTestEndTime = time;
         }
      }

      time += gInvSampleRateMs;

      if (mState == State::Testing)
         ++mTestSamplesElapsed;
   }

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void LatencyCalculatorReceiver::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   const int kMaxTestDurationMs = 2000.0f;
   if (mState == State::Testing && gTime > mTestStartTime + kMaxTestDurationMs)
      mState = State::NoResult;

   switch (mState)
   {
      case State::Init:
         DrawTextNormal("press \"test\" on the sender", 3, 15);
         break;
      case State::TestRequested:
      case State::Testing:
         DrawTextNormal("testing...", 3, 15);
         break;
      case State::DisplayResult:
      {
         double duration = mTestEndTime - mTestStartTime;
         float durationBuffers = (float)mTestSamplesElapsed / gBufferSize;
         DrawTextNormal("latency result: " + ofToString(duration, 2) + " ms (" + ofToString(mTestSamplesElapsed) + " samples, " + ofToString(durationBuffers, 2) + " buffers)", 3, 15);
         DrawTextNormal("buffer size: " + ofToString(gBufferSize) + "   sample rate: " + ofToString(gSampleRate), 3, 30);
         break;
      }
      case State::NoResult:
         DrawTextNormal("test failed", 3, 15);
         break;
   }
}

void LatencyCalculatorReceiver::OnTestRequested()
{
   mState = State::TestRequested;
}

void LatencyCalculatorReceiver::OnTestStarted(double time, int samplesIn)
{
   mTestSamplesElapsed = -samplesIn;
   mTestStartTime = time;
   mState = State::Testing;
}

void LatencyCalculatorReceiver::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void LatencyCalculatorReceiver::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}