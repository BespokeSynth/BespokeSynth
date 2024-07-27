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
//  LooperRecorder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/9/12.
//
//

#include "LooperRecorder.h"
#include "Looper.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Scale.h"
#include "Stutter.h"
#include "ModularSynth.h"
#include "ChaosEngine.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

LooperRecorder::LooperRecorder()
: IAudioProcessor(gBufferSize)
, mRecordBuffer(MAX_BUFFER_SIZE)
, mWriteBuffer(gBufferSize)
{
   mQuietInputRamp.SetValue(1);
}

namespace
{
   const float kBufferSegmentWidth = 30;
   const float kBufferHeight = 50;
}

void LooperRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mCommit1BarButton = new ClickButton(this, "1", 3 + kBufferSegmentWidth * 3, 3);
   mCommit2BarsButton = new ClickButton(this, "2", 3 + kBufferSegmentWidth * 2, 3);
   mCommit4BarsButton = new ClickButton(this, "4", 3 + kBufferSegmentWidth, 3);
   mCommit8BarsButton = new ClickButton(this, "8", 3, 3);

   float width, height;

   UIBLOCK(kBufferSegmentWidth * 4 + 6, 3, 60);
   DROPDOWN(mNumBarsSelector, "length", &mNumBars, 50);
   BUTTON(mDoubleTempoButton, "2xtempo");
   BUTTON(mHalfTempoButton, ".5tempo");
   //BUTTON(mShiftMeasureButton, "shift"); UIBLOCK_SHIFTUP(); UIBLOCK_SHIFTX(30);
   //BUTTON(mHalfShiftButton, "half"); UIBLOCK_NEWLINE();
   //BUTTON(mShiftDownbeatButton, "downbeat");
   UIBLOCK_SHIFTDOWN();
   UIBLOCK_SHIFTDOWN();
   INTSLIDER(mNextCommitTargetSlider, "target", &mNextCommitTargetIndex, 0, 3);
   CHECKBOX(mAutoAdvanceThroughLoopersCheckbox, "auto-advance", &mAutoAdvanceThroughLoopers);
   UIBLOCK_NEWCOLUMN();
   UIBLOCK_PUSHSLIDERWIDTH(80);
   DROPDOWN(mModeSelector, "mode", ((int*)(&mRecorderMode)), 60);
   BUTTON(mClearOverdubButton, "clear");
   CHECKBOX(mFreeRecordingCheckbox, "free rec", &mFreeRecording);
   BUTTON(mCancelFreeRecordButton, "cancel free rec");
   ENDUIBLOCK(width, height);

   mWidth = MAX(mWidth, width);
   mHeight = MAX(mHeight, height);

   UIBLOCK(3, kBufferHeight + 6);
   BUTTON(mOrigSpeedButton, "orig speed");
   BUTTON(mSnapPitchButton, "snap to pitch");
   BUTTON(mResampleButton, "resample");
   BUTTON(mResampAndSetButton, "resample & set key");
   UIBLOCK_PUSHSLIDERWIDTH(120);
   FLOATSLIDER(mLatencyFixMsSlider, "latency fix ms", &mLatencyFixMs, 0, 200);
   ENDUIBLOCK(width, height);

   mWidth = MAX(mWidth, width);
   mHeight = MAX(mHeight, height);

   UIBLOCK(kBufferSegmentWidth * 4 + 110, kBufferHeight + 22);
   for (int i = 0; i < (int)mWriteForLooperCheckbox.size(); ++i)
   {
      CHECKBOX(mWriteForLooperCheckbox[i], ("write" + ofToString(i)).c_str(), &mWriteForLooper[i]);
   }
   ENDUIBLOCK(width, height);
   width += 12;

   mWidth = MAX(mWidth, width);

   mNumBarsSelector->AddLabel(" 1 ", 1);
   mNumBarsSelector->AddLabel(" 2 ", 2);
   mNumBarsSelector->AddLabel(" 3 ", 3);
   mNumBarsSelector->AddLabel(" 4 ", 4);
   mNumBarsSelector->AddLabel(" 6 ", 6);
   mNumBarsSelector->AddLabel(" 8 ", 8);
   mNumBarsSelector->AddLabel("12 ", 12);

   mModeSelector->AddLabel("record", kRecorderMode_Record);
   mModeSelector->AddLabel("overdub", kRecorderMode_Overdub);
   mModeSelector->AddLabel("loop", kRecorderMode_Loop);

   mCommit1BarButton->SetDisplayText(false);
   mCommit1BarButton->SetDimensions(kBufferSegmentWidth, kBufferHeight);
   mCommit2BarsButton->SetDisplayText(false);
   mCommit2BarsButton->SetDimensions(kBufferSegmentWidth * 2, kBufferHeight);
   mCommit4BarsButton->SetDisplayText(false);
   mCommit4BarsButton->SetDimensions(kBufferSegmentWidth * 3, kBufferHeight);
   mCommit8BarsButton->SetDisplayText(false);
   mCommit8BarsButton->SetDimensions(kBufferSegmentWidth * 4, kBufferHeight);

   for (int i = 0; i < kMaxLoopers; ++i)
   {
      mLooperPatchCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mLooperPatchCables[i]->AddTypeFilter("looper");
      ofRectangle rect = mWriteForLooperCheckbox[i]->GetRect(K(local));
      mLooperPatchCables[i]->SetManualPosition(rect.getMaxX() + 5, rect.getCenter().y);
      mLooperPatchCables[i]->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
      ofColor color = mLooperPatchCables[i]->GetColor();
      color.a *= .3f;
      mLooperPatchCables[i]->SetColor(color);
      AddPatchCableSource(mLooperPatchCables[i]);
   }
}

LooperRecorder::~LooperRecorder()
{
}

void LooperRecorder::Init()
{
   IDrawableModule::Init();
   mBaseTempo = TheTransport->GetTempo();
}

void LooperRecorder::Process(double time)
{
   PROFILER(LooperRecorder);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();

      return;
   }

   ComputeSliders(0);
   mWriteBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mRecordBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = GetBuffer()->BufferSize();

   if (mCommitToLooper)
   {
      SyncLoopLengths();
      mCommitToLooper->SetNumBars(mNumBars);
      mCommitToLooper->Commit(&mRecordBuffer, false, mLatencyFixMs);

      mRecorderMode = kRecorderMode_Record;
      if (mTemporarilySilenceAfterCommit)
      {
         mQuietInputRamp.Start(time, 0, time + 10);
         mUnquietInputTime = time + 1000; //no input for 1 second
      }
      mCommitToLooper = nullptr;
   }

   if (mUnquietInputTime != -1 && time >= mUnquietInputTime)
   {
      mQuietInputRamp.Start(time, 1, time + 10);
      mUnquietInputTime = -1;
   }

   UpdateSpeed();

   bool acceptInput = (mRecorderMode == kRecorderMode_Record || mRecorderMode == kRecorderMode_Overdub);
   bool loop = (mRecorderMode == kRecorderMode_Loop || mRecorderMode == kRecorderMode_Overdub);


   for (int i = 0; i < bufferSize; ++i)
   {
      if (acceptInput)
      {
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            GetBuffer()->GetChannel(ch)[i] *= mQuietInputRamp.Value(time);
            mWriteBuffer.GetChannel(ch)[i] = GetBuffer()->GetChannel(ch)[i];
         }
         time += gInvSampleRateMs;
      }
      else
      {
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
            mWriteBuffer.GetChannel(ch)[i] = 0;
      }
   }

   if (loop)
   {
      int delaySamps = TheTransport->GetDuration(kInterval_1n) * GetNumBars() / gInvSampleRateMs;
      delaySamps = MIN(delaySamps, MAX_BUFFER_SIZE - 1);
      for (int i = 0; i < bufferSize; ++i)
      {
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            float sample = mRecordBuffer.GetSample(delaySamps - i, ch);
            mWriteBuffer.GetChannel(ch)[i] += sample;
            GetBuffer()->GetChannel(ch)[i] += sample;
         }
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      mRecordBuffer.WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);

      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);

      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void LooperRecorder::DrawCircleHash(ofVec2f center, float progress, float width, float innerRadius, float outerRadius)
{
   ofSetLineWidth(width);
   float sinTheta = sin(progress * TWO_PI);
   float cosTheta = cos(progress * TWO_PI);
   ofLine(innerRadius * sinTheta + center.x, innerRadius * -cosTheta + center.y,
          outerRadius * sinTheta + center.x, outerRadius * -cosTheta + center.y);
}

void LooperRecorder::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = MAX(mHeight, mWriteForLooperCheckbox[mNumLoopers - 1]->GetRect(K(local)).getMaxY() + 3);
}

void LooperRecorder::PreRepatch(PatchCableSource* cableSource)
{
   for (int i = 0; i < mLooperPatchCables.size(); ++i)
   {
      if (cableSource == mLooperPatchCables[i])
      {
         Looper* looper = nullptr;
         if (i < mLoopers.size())
            looper = mLoopers[i];
         if (looper)
            looper->SetRecorder(nullptr);
      }
   }
}

void LooperRecorder::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (int i = 0; i < mLoopers.size(); ++i)
   {
      if (cableSource == mLooperPatchCables[i])
      {
         mLoopers[i] = dynamic_cast<Looper*>(mLooperPatchCables[i]->GetTarget());
         if (mLoopers[i])
            mLoopers[i]->SetRecorder(this);
      }
   }
}

void LooperRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxLoopers; ++i)
   {
      mWriteForLooperCheckbox[i]->SetShowing(i < mNumLoopers);
      mLooperPatchCables[i]->SetShowing(i < mNumLoopers);
   }

   ofPushStyle();
   ofFill();
   ofColor color = GetColor(kModuleCategory_Audio);
   ofSetColor(color.r, color.g, color.b, 50);
   float x = kBufferSegmentWidth * 4 + 3;
   float y = 70;
   float w, h;
   GetModuleDimensions(w, h);
   ofRect(x, y, w - x - 3, h - y - 3);
   ofPopStyle();

   DrawTextNormal("loopers:", kBufferSegmentWidth * 4 + 6, 82);
   if (mNextCommitTargetIndex < (int)mLooperPatchCables.size())
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofVec2f cablePos = mLooperPatchCables[mNextCommitTargetIndex]->GetPosition();
      cablePos -= GetPosition();
      ofCircle(cablePos.x, cablePos.y, 5);
      ofPopStyle();
   }

   mResampleButton->Draw();
   mResampAndSetButton->Draw();
   mDoubleTempoButton->Draw();
   mHalfTempoButton->Draw();
   //mShiftMeasureButton->Draw();
   //mHalfShiftButton->Draw();
   //mShiftDownbeatButton->Draw();
   mModeSelector->Draw();
   mClearOverdubButton->Draw();
   mNumBarsSelector->Draw();
   mOrigSpeedButton->Draw();
   mSnapPitchButton->Draw();
   mFreeRecordingCheckbox->Draw();
   mCancelFreeRecordButton->Draw();
   mLatencyFixMsSlider->Draw();
   mNextCommitTargetSlider->Draw();
   mAutoAdvanceThroughLoopersCheckbox->Draw();

   for (int i = 0; i < (int)mWriteForLooperCheckbox.size(); ++i)
      mWriteForLooperCheckbox[i]->Draw();

   if (mSpeed != 1)
   {
      float rootPitch = AdjustedRootForSpeed();
      int pitch = int(rootPitch + .5f);
      int cents = (rootPitch - pitch) * 100;

      std::string speed = "speed " + ofToString(mSpeed, 2) + ", ";

      std::string detune = NoteName(pitch);
      if (cents > 0)
         detune += " +" + ofToString(cents) + " cents";
      if (cents < 0)
         detune += " -" + ofToString(-cents) + " cents";

      DrawTextNormal(speed + detune, 100, 80);
   }

   if (mCommit1BarButton == gHoveredUIControl)
      mCommit1BarButton->Draw();
   if (mCommit2BarsButton == gHoveredUIControl)
      mCommit2BarsButton->Draw();
   if (mCommit4BarsButton == gHoveredUIControl)
      mCommit4BarsButton->Draw();
   if (mCommit8BarsButton == gHoveredUIControl)
      mCommit8BarsButton->Draw();

   ofPushStyle();
   int sampsPerBar = abs(int(TheTransport->MsPerBar() / 1000 * gSampleRate));
   for (int i = 0; i < 4; ++i) //segments
   {
      int bars = 1;
      int barOffset = 0;
      if (i == 1)
      {
         barOffset = 1;
      }
      if (i == 2)
      {
         bars = 2;
         barOffset = 2;
      }
      if (i == 3)
      {
         bars = 4;
         barOffset = 4;
      }
      mRecordBuffer.Draw(3 + (3 - i) * kBufferSegmentWidth, 3, kBufferSegmentWidth, kBufferHeight, sampsPerBar * bars, -1, sampsPerBar * barOffset);
   }

   ofSetColor(0, 0, 0, 20);
   for (int i = 1; i < 4; ++i)
   {
      const float bx = 3 + i * kBufferSegmentWidth;
      ofLine(bx, 3, bx, 3 + kBufferHeight);
   }
   ofPopStyle();

   /*ofPushStyle();
   ofVec2f center(48,28);
   float radius = 25;
   ofSetColor(255,255,255,100*gModuleDrawAlpha);
   DrawCircleHash(center, (TheTransport->GetMeasurePos(gTime) + TheTransport->GetMeasure(gTime) % 8) / 8, 1, radius * .9f, radius);
   DrawCircleHash(center, (TheTransport->GetMeasurePos(gTime) + TheTransport->GetMeasure(gTime) % mNumBars) / mNumBars, 3, radius * .7f, radius);
   for (int i=0; i<mNumBars; ++i)
      DrawCircleHash(center, float(i)/mNumBars, 1, radius * .8f, radius);
   ofPopStyle();*/

   if (mDrawDebug)
      mRecordBuffer.Draw(0, 162, 800, 100);
}

void LooperRecorder::RemoveLooper(Looper* looper)
{
   for (int i = 0; i < mLoopers.size(); ++i)
   {
      if (mLoopers[i] == looper)
         mLoopers[i] = nullptr;
   }
}

float LooperRecorder::AdjustedRootForSpeed()
{
   float rootFreq = TheScale->PitchToFreq(TheScale->ScaleRoot() + 24);
   rootFreq *= mSpeed;
   return TheScale->FreqToPitch(rootFreq);
}

void LooperRecorder::SnapToClosestPitch()
{
   float currentPitch = AdjustedRootForSpeed();
   float desiredPitch = int(currentPitch + .5f);

   float currentFreq = TheScale->PitchToFreq(currentPitch);
   float desiredFreq = TheScale->PitchToFreq(desiredPitch);

   TheTransport->SetTempo(TheTransport->GetTempo() * desiredFreq / currentFreq);
}

void LooperRecorder::Resample(bool setKey)
{
   if (setKey)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed() + .5f));
   }

   SyncLoopLengths();
}

void LooperRecorder::UpdateSpeed()
{
   float newSpeed = TheTransport->GetTempo() / mBaseTempo;
   if (mSpeed != newSpeed)
   {
      mSpeed = newSpeed;
      if (mSpeed == 0)
         mSpeed = .001f;
   }
}

void LooperRecorder::SyncLoopLengths()
{
   if (mSpeed <= 0) //avoid crashing
      return;

   for (int i = 0; i < mLoopers.size(); ++i)
   {
      if (mLoopers[i])
      {
         if (mSpeed != 1)
            mLoopers[i]->ResampleForSpeed(mLoopers[i]->GetPlaybackSpeed());
         mLoopers[i]->RecalcLoopLength();
      }
   }

   mBaseTempo = TheTransport->GetTempo();
}

void LooperRecorder::Commit(Looper* looper)
{
   mCommitToLooper = looper;
}

void LooperRecorder::RequestMerge(Looper* looper)
{
   if (mMergeSource == nullptr)
   {
      mMergeSource = looper;
   }
   else if (mMergeSource == looper)
   {
      mMergeSource = nullptr;
   }
   else
   {
      looper->MergeIn(mMergeSource);
      mMergeSource = nullptr;
   }
}

void LooperRecorder::RequestSwap(Looper* looper)
{
   if (mSwapSource == nullptr)
   {
      mSwapSource = looper;
   }
   else if (mSwapSource == looper)
   {
      mSwapSource = nullptr;
   }
   else
   {
      looper->SwapBuffers(mSwapSource);
      mSwapSource = nullptr;
   }
}

void LooperRecorder::RequestCopy(Looper* looper)
{
   if (mCopySource == nullptr)
   {
      mCopySource = looper;
   }
   else if (mCopySource == looper)
   {
      mCopySource = nullptr;
   }
   else
   {
      looper->CopyBuffer(mCopySource);
      mCopySource = nullptr;
   }
}

void LooperRecorder::ResetSpeed()
{
   mBaseTempo = TheTransport->GetTempo();
}

void LooperRecorder::StartFreeRecord(double time)
{
   if (mFreeRecording)
      return;

   mFreeRecording = true;
   mStartFreeRecordTime = time;
}

void LooperRecorder::EndFreeRecord(double time)
{
   if (!mFreeRecording)
      return;

   float recordedTime = time - mStartFreeRecordTime;
   int beats = mNumBars * TheTransport->GetTimeSigTop();
   float minutes = recordedTime / 1000.0f / 60.0f;
   TheTransport->SetTempo(beats / minutes);
   TheTransport->SetDownbeat();
   mRecorderMode = kRecorderMode_Loop;
   mFreeRecording = false;
}

void LooperRecorder::CancelFreeRecord()
{
   mFreeRecording = false;
   mStartFreeRecordTime = 0;
}

bool LooperRecorder::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99 && midiValue > 0)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;

         if (y == 0)
         {
            switch (x)
            {
               case 0: mCommit8BarsButton->SetValue(1, gTime); break;
               case 1: mCommit4BarsButton->SetValue(1, gTime); break;
               case 2: mCommit2BarsButton->SetValue(1, gTime); break;
               case 3: mCommit1BarButton->SetValue(1, gTime); break;
               default: break;
            }
         }
         else if (y == 1)
         {
            if (x < mLoopers.size())
               mNextCommitTargetIndex = x;
         }
         else if (y - 2 < mLoopers.size())
         {
            int looperIndex = y - 2;
            if (mLoopers[looperIndex] != nullptr)
            {
               if (x == 0)
                  push2->SetDisplayModule(mLoopers[looperIndex]);
               if (x == 1)
                  mLoopers[looperIndex]->SetMute(gTime, !mLoopers[looperIndex]->GetMute());
            }
         }

         return true;
      }
   }

   return false;
}

void LooperRecorder::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor = 0;

         if (y == 0)
         {
            switch (x)
            {
               case 0: pushColor = mCommit8BarsButton->GetValue() > 0 ? 125 : 33; break;
               case 1: pushColor = mCommit4BarsButton->GetValue() > 0 ? 125 : 33; break;
               case 2: pushColor = mCommit2BarsButton->GetValue() > 0 ? 125 : 33; break;
               case 3: pushColor = mCommit1BarButton->GetValue() > 0 ? 125 : 33; break;
               default: break;
            }
         }
         else if (y == 1)
         {
            if (x < mLoopers.size())
               pushColor = (x == mNextCommitTargetIndex) ? 126 : 86;
         }
         else if (y - 2 < mLoopers.size())
         {
            int looperIndex = y - 2;
            if (mLoopers[looperIndex] != nullptr)
            {
               if (x == 0)
                  pushColor = (push2->GetDisplayModule() == mLoopers[looperIndex]) ? 125 : 33;
               if (x == 1)
                  pushColor = mLoopers[looperIndex]->GetMute() ? 127 : 68;
            }
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

void LooperRecorder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResampleButton)
      SyncLoopLengths();

   if (button == mResampAndSetButton)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed() + .5f));
      SyncLoopLengths();
   }

   if (button == mDoubleTempoButton)
   {
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->DoubleNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo() * 2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos(time) + (TheTransport->GetMeasure(time) % 8);
      int count = TheTransport->GetMeasure(time) - int(pos);
      pos *= 2;
      count += int(pos);
      pos -= int(pos);
      TheTransport->SetMeasureTime(count + pos);
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ResampleForSpeed(1);
      }
   }

   if (button == mHalfTempoButton)
   {
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalveNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo() / 2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos(time) + (TheTransport->GetMeasure(time) % 8);
      int count = TheTransport->GetMeasure(time) - int(pos);
      pos /= 2;
      count += int(pos);
      pos -= int(pos);
      TheTransport->SetMeasureTime(count + pos);
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ResampleForSpeed(1);
      }
   }

   if (button == mShiftMeasureButton)
   {
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ShiftMeasure();
      }
      int newMeasure = TheTransport->GetMeasure(time) - 1;
      if (newMeasure < 0)
         newMeasure = 7;
      TheTransport->SetMeasure(newMeasure);
   }

   if (button == mHalfShiftButton)
   {
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalfShift();
      }
      int newMeasure = int(TheTransport->GetMeasure(time) + TheTransport->GetMeasurePos(time) - .5f);
      if (newMeasure < 0)
         newMeasure = 7;
      float newMeasurePos = FloatWrap(TheTransport->GetMeasurePos(time) - .5f, 1);
      TheTransport->SetMeasureTime(newMeasure + newMeasurePos);
   }

   if (button == mShiftDownbeatButton)
   {
      TheTransport->SetMeasure(TheTransport->GetMeasure(time) / 8 * 8); //align to 8 bars
      TheTransport->SetDownbeat();
      for (int i = 0; i < mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ShiftDownbeat();
      }
      if (TheChaosEngine)
         TheChaosEngine->RestartProgression();
   }

   if (button == mClearOverdubButton)
      mRecordBuffer.ClearBuffer();

   if (button == mOrigSpeedButton)
   {
      TheTransport->SetTempo(mBaseTempo);
   }

   if (button == mSnapPitchButton)
      SnapToClosestPitch();

   if (button == mCancelFreeRecordButton)
      CancelFreeRecord();

   if (button == mCommit1BarButton ||
       button == mCommit2BarsButton ||
       button == mCommit4BarsButton ||
       button == mCommit8BarsButton)
   {
      int numBars = 1;
      if (button == mCommit1BarButton)
         numBars = 1;
      if (button == mCommit2BarsButton)
         numBars = 2;
      if (button == mCommit4BarsButton)
         numBars = 4;
      if (button == mCommit8BarsButton)
         numBars = 8;

      mNumBars = numBars;
      if (mNextCommitTargetIndex < (int)mLoopers.size())
         Commit(mLoopers[mNextCommitTargetIndex]);
      if (mAutoAdvanceThroughLoopers)
      {
         for (int i = 0; i < (int)mLoopers.size(); ++i)
         {
            mNextCommitTargetIndex = (mNextCommitTargetIndex + 1) % mLoopers.size();
            if (mLoopers[mNextCommitTargetIndex] != nullptr)
               break;
         }
      }
   }
}

void LooperRecorder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mFreeRecordingCheckbox)
   {
      bool freeRec = mFreeRecording;
      mFreeRecording = !mFreeRecording; //flip back so these methods won't be ignored
      if (freeRec)
         StartFreeRecord(time);
      else
         EndFreeRecord(time);
   }

   for (int i = 0; i < (int)mWriteForLooperCheckbox.size(); ++i)
   {
      if (checkbox == mWriteForLooperCheckbox[i])
      {
         if (mWriteForLooper[i])
         {
            bool isWriteInProgress = false;
            for (int j = 0; j < mNumLoopers; ++j)
            {
               if (j != i && mWriteForLooper[j])
                  isWriteInProgress = true;
            }

            if (isWriteInProgress)
            {
               // cancel all
               for (int j = 0; j < mNumLoopers; ++j)
                  mWriteForLooper[j] = false;
            }
            else
            {
               mStartRecordMeasureTime[i] = TheTransport->GetMeasureTime(gTime);
            }
         }
         else
         {
            double currentMeasureTime = TheTransport->GetMeasureTime(gTime);
            double lengthInMeasures = currentMeasureTime - mStartRecordMeasureTime[i];
            int numBars = 1;
            if (lengthInMeasures < 1.5f)
               numBars = 1;
            else if (lengthInMeasures < 3.0f)
               numBars = 2;
            else if (lengthInMeasures < 6.0f)
               numBars = 4;
            else
               numBars = 8;

            mNumBars = numBars;
            Commit(mLoopers[i]);
         }
      }
   }
}

void LooperRecorder::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void LooperRecorder::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
}

void LooperRecorder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mNumBarsSelector)
   {
      mNumBars = MAX(1, mNumBars);
   }
}

void LooperRecorder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void LooperRecorder::Poll()
{
}

void LooperRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("num_loopers", moduleInfo, 4, 1, kMaxLoopers);
   mModuleSaveData.LoadBool("temp_silence_after_commit", moduleInfo, false);

   SetUpFromSaveData();
}

void LooperRecorder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mNumLoopers = mModuleSaveData.GetInt("num_loopers");
   mTemporarilySilenceAfterCommit = mModuleSaveData.GetBool("temp_silence_after_commit");
}

void LooperRecorder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mBaseTempo;
   out << mSpeed;
   mRecordBuffer.SaveState(out);
}

void LooperRecorder::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mBaseTempo;
   in >> mSpeed;
   mRecordBuffer.LoadState(in);
}
