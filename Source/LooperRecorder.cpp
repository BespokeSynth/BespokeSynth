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
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

LooperRecorder::LooperRecorder()
: IAudioProcessor(gBufferSize)
, mWidth(235)
, mHeight(125)
, mRecordBuffer(MAX_BUFFER_SIZE)
, mNumBars(1)
, mNumBarsSelector(nullptr)
, mSpeed(1.0f)
, mBaseTempo(120)
, mResampAndSetButton(nullptr)
, mResampleButton(nullptr)
, mMergeSource(nullptr)
, mSwapSource(nullptr)
, mCopySource(nullptr)
, mCommitCount(0)
, mDoubleTempoButton(nullptr)
, mHalfTempoButton(nullptr)
, mShiftMeasureButton(nullptr)
, mHalfShiftButton(nullptr)
, mShiftDownbeatButton(nullptr)
, mClearOverdubButton(nullptr)
, mUnquietInputTime(-1)
, mOrigSpeedButton(nullptr)
, mSnapPitchButton(nullptr)
, mHeadphonesTarget(nullptr)
, mOutputTarget(nullptr)
, mCommitDelay(0)
, mCommitDelaySlider(nullptr)
, mNextCommitTargetIndex(0)
, mFreeRecording(false)
, mFreeRecordingCheckbox(nullptr)
, mStartFreeRecordTime(0)
, mCancelFreeRecordButton(nullptr)
, mCommitToLooper(nullptr)
, mRecorderMode(kRecorderMode_Record)
, mModeSelector(nullptr)
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
   
   mCommit1BarButton = new ClickButton(this, "1", 3 + kBufferSegmentWidth*3, 3);
   mCommit2BarsButton = new ClickButton(this, "2", 3 + kBufferSegmentWidth * 2, 3);
   mCommit4BarsButton = new ClickButton(this, "4", 3 + kBufferSegmentWidth, 3);
   mCommit8BarsButton = new ClickButton(this, "8", 3, 3);

   float width, height;

   UIBLOCK(kBufferSegmentWidth*4 + 6, 3, 60);
   DROPDOWN(mNumBarsSelector, "length", &mNumBars, 50);
   BUTTON(mDoubleTempoButton, "2xtempo");
   BUTTON(mHalfTempoButton, ".5tempo");
   //BUTTON(mShiftMeasureButton, "shift"); UIBLOCK_SHIFTUP(); UIBLOCK_SHIFTX(30);
   //BUTTON(mHalfShiftButton, "half"); UIBLOCK_NEWLINE();
   //BUTTON(mShiftDownbeatButton, "downbeat");
   INTSLIDER(mNextCommitTargetSlider, "target", &mNextCommitTargetIndex, 0, 3);
   UIBLOCK_NEWCOLUMN();
   UIBLOCK_PUSHSLIDERWIDTH(80);
   DROPDOWN(mModeSelector, "mode", ((int*)(&mRecorderMode)), 60);
   //FLOATSLIDER(mCommitDelaySlider, "delay", &mCommitDelay, 0, 1);
   BUTTON(mClearOverdubButton, "clear");
   CHECKBOX(mFreeRecordingCheckbox, "free rec", &mFreeRecording);
   BUTTON(mCancelFreeRecordButton, "cancel free rec");
   ENDUIBLOCK(width, height);

   mWidth = MAX(mWidth, width);
   mHeight = MAX(mHeight, height);

   UIBLOCK(3, kBufferHeight+6);
   BUTTON(mOrigSpeedButton, "orig speed");
   BUTTON(mSnapPitchButton, "snap to pitch");
   BUTTON(mResampleButton, "resample");
   BUTTON(mResampAndSetButton, "resample & set key");
   ENDUIBLOCK(width, height);

   mWidth = MAX(mWidth, width);
   mHeight = MAX(mHeight, height);   
   
   mNumBarsSelector->AddLabel(" 1 ",1);
   mNumBarsSelector->AddLabel(" 2 ",2);
   mNumBarsSelector->AddLabel(" 3 ",3);
   mNumBarsSelector->AddLabel(" 4 ",4);
   mNumBarsSelector->AddLabel(" 6 ",6);
   mNumBarsSelector->AddLabel(" 8 ",8);
   mNumBarsSelector->AddLabel("12 ",12);
   
   mModeSelector->AddLabel("record", kRecorderMode_Record);
   mModeSelector->AddLabel("overdub", kRecorderMode_Overdub);
   mModeSelector->AddLabel("loop", kRecorderMode_Loop);

   mCommit1BarButton->SetDisplayText(false);
   mCommit1BarButton->SetDimensions(kBufferSegmentWidth,kBufferHeight);
   mCommit2BarsButton->SetDisplayText(false);
   mCommit2BarsButton->SetDimensions(kBufferSegmentWidth*2, kBufferHeight);
   mCommit4BarsButton->SetDisplayText(false);
   mCommit4BarsButton->SetDimensions(kBufferSegmentWidth*3, kBufferHeight);
   mCommit8BarsButton->SetDisplayText(false);
   mCommit8BarsButton->SetDimensions(kBufferSegmentWidth*4, kBufferHeight);
   
   SyncCablesToLoopers();
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

   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);
   SyncBuffers();
   mWriteBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mRecordBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());
   
   int bufferSize = GetBuffer()->BufferSize();

   if (mCommitToLooper)
   {
      SyncLoopLengths();
      mCommitToLooper->SetNumBars(mNumBars);
      mCommitToLooper->Commit();
      
      mRecorderMode = kRecorderMode_Record;
      mQuietInputRamp.Start(gTime, 0, gTime+10);
      mUnquietInputTime = gTime + 1000; //no input for 1 second
      mCommitToLooper = nullptr;
   }
   
   if (mUnquietInputTime != -1 && time >= mUnquietInputTime)
   {
      mQuietInputRamp.Start(time, 1, time+10);
      mUnquietInputTime = -1;
   }
   
   UpdateSpeed();
   
   bool acceptInput = (mRecorderMode == kRecorderMode_Record || mRecorderMode == kRecorderMode_Overdub);
   bool loop = (mRecorderMode == kRecorderMode_Loop || mRecorderMode == kRecorderMode_Overdub);

   
   for (int i=0; i<bufferSize; ++i)
   {
      if (acceptInput)
      {
         for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
         {
            GetBuffer()->GetChannel(ch)[i] *= mQuietInputRamp.Value(time);
            mWriteBuffer.GetChannel(ch)[i] = GetBuffer()->GetChannel(ch)[i];
         }
         time += gInvSampleRateMs;
      }
      else
      {
         for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
            mWriteBuffer.GetChannel(ch)[i] = 0;
      }
   }

   if (loop)
   {
      int delaySamps = TheTransport->GetDuration(kInterval_1n) * NumBars() / gInvSampleRateMs;
      delaySamps = MIN(delaySamps, MAX_BUFFER_SIZE-1);
      for (int i=0; i<bufferSize; ++i)
      {
         for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
         {
            float sample = mRecordBuffer.GetSample(delaySamps - i, ch);
            mWriteBuffer.GetChannel(ch)[i] += sample;
            GetBuffer()->GetChannel(ch)[i] += sample;
         }
      }
   }
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      mRecordBuffer.WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
   
      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
   
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),bufferSize, ch);
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

void LooperRecorder::SyncCablesToLoopers()
{
   int numLoopers = MAX(4, (int)mLoopers.size());
   
   if (!mLoopers.empty() && mLoopers[mLoopers.size()-1] != nullptr)
      ++numLoopers; //add an extra cable for an additional looper
   
   if (numLoopers == mLooperPatchCables.size())
      return;  //nothing to do
   
   if (numLoopers > mLooperPatchCables.size())
   {
      int oldSize = (int)mLooperPatchCables.size();
      mLooperPatchCables.resize(numLoopers);
      for (int i=0; i<oldSize; ++i)
      {
         mLooperPatchCables[i]->SetTarget(mLoopers[i]);
      }
      for (int i=oldSize; i<mLooperPatchCables.size(); ++i)
      {
         mLooperPatchCables[i] = new PatchCableSource(this, kConnectionType_Special);
         mLooperPatchCables[i]->AddTypeFilter("looper");
         Looper* looper = nullptr;
         if (i < mLoopers.size())
            looper = mLoopers[i];
         mLooperPatchCables[i]->SetTarget(looper);
         mLooperPatchCables[i]->SetManualPosition(160+i*12, 117);
         mLooperPatchCables[i]->SetOverrideCableDir(ofVec2f(0, 1));
         ofColor color = mLooperPatchCables[i]->GetColor();
         color.a *= .3f;
         mLooperPatchCables[i]->SetColor(color);
         AddPatchCableSource(mLooperPatchCables[i]);
      }
   }
   else
   {
      for (int i=numLoopers; i<mLooperPatchCables.size(); ++i)
         RemovePatchCableSource(mLooperPatchCables[i]);
      mLooperPatchCables.resize(numLoopers);
   }
}

void LooperRecorder::PreRepatch(PatchCableSource* cableSource)
{
   for (int i=0; i<mLooperPatchCables.size(); ++i)
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
   mLoopers.resize(mLooperPatchCables.size());
   int maxLooperIndex = 0;
   for (int i=0; i< mLoopers.size(); ++i)
   {
      if (cableSource == mLooperPatchCables[i])
      {
         mLoopers[i] = dynamic_cast<Looper*>(mLooperPatchCables[i]->GetTarget());
         if (mLoopers[i])
         {
            mLoopers[i]->SetRecorder(this);
            maxLooperIndex = i;
         }
      }
   }

   if (maxLooperIndex > 0)
      mNextCommitTargetSlider->SetExtents(0, maxLooperIndex);
   
   SyncCablesToLoopers();
}

void LooperRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

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
   //mCommitDelaySlider->Draw();
   mFreeRecordingCheckbox->Draw();
   mCancelFreeRecordButton->Draw();
   mNextCommitTargetSlider->Draw();

   if (mSpeed != 1)
   {
      float rootPitch = AdjustedRootForSpeed();
      int pitch = int(rootPitch + .5f);
      int cents = (rootPitch - pitch) * 100;
      
      std::string speed = "speed "+ofToString(mSpeed,2)+", ";
      
      std::string detune = NoteName(pitch);
      if (cents > 0)
         detune += " +" + ofToString(cents) + " cents";
      if (cents < 0)
         detune += " -" + ofToString(-cents) + " cents";
      
      DrawTextNormal(speed + detune,100,80);
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
   for (int i = 0; i < 4; ++i)   //segments
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
      mRecordBuffer.Draw(3+(3-i)*kBufferSegmentWidth, 3, kBufferSegmentWidth, kBufferHeight, sampsPerBar*bars, -1, sampsPerBar*barOffset);
   }

   ofSetColor(0, 0, 0, 20);
   for (int i = 1; i < 4; ++i)
   {
      float x = 3 + i * kBufferSegmentWidth;
      ofLine(x, 3, x, 3 + kBufferHeight);
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
      mRecordBuffer.Draw(0,162,800,100);
   
   DrawTextNormal("loopers:",155,109);
}

void LooperRecorder::RemoveLooper(Looper* looper)
{
   for (int i=0; i<mLoopers.size(); ++i)
   {
      if (mLoopers[i] == looper)
         mLoopers[i] = nullptr;
   }
}

float LooperRecorder::AdjustedRootForSpeed()
{
   float rootFreq = TheScale->PitchToFreq(TheScale->ScaleRoot()+24);
   rootFreq *= mSpeed;
   return TheScale->FreqToPitch(rootFreq);
}

void LooperRecorder::SnapToClosestPitch()
{
   float currentPitch = AdjustedRootForSpeed();
   float desiredPitch = int(currentPitch + .5f);

   float currentFreq = TheScale->PitchToFreq(currentPitch);
   float desiredFreq = TheScale->PitchToFreq(desiredPitch);

   TheTransport->SetTempo(TheTransport->GetTempo() * desiredFreq/currentFreq);
}

void LooperRecorder::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   if (GetKeyModifiers() == (kModifier_Command | kModifier_Shift))
   {
      if (key >= '1' && key <= '4')
      {
         int looper = key - '1';
         if (looper < mLoopers.size())
            Commit(mLoopers[key - '1']);
      }
      if (key >= '5' && key <= '8')
      {
         int idx = key - '5';
         SetNumBars(powf(2,idx));
      }
   }

   if (key == 'k')   //resample and set key
      Resample(true);
   if (key == 'l')   //resample (without setting key)
      Resample(false);
}

void LooperRecorder::Resample(bool setKey)
{
   if (setKey)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed()+.5f));
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
   if (mSpeed <= 0)  //avoid crashing
      return;

   for (int i=0; i<mLoopers.size(); ++i)
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

void LooperRecorder::RequestSwap(Looper *looper)
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

void LooperRecorder::RequestCopy(Looper *looper)
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

void LooperRecorder::StartFreeRecord()
{
   if (mFreeRecording)
      return;
   
   mFreeRecording = true;
   mStartFreeRecordTime = gTime;
}

void LooperRecorder::EndFreeRecord()
{
   if (!mFreeRecording)
      return;
   
   float recordedTime = gTime - mStartFreeRecordTime;
   int beats = mNumBars * TheTransport->GetTimeSigTop();
   float minutes = recordedTime / 1000.0f / 60.0f;
   TheTransport->SetTempo(beats/minutes);
   TheTransport->SetDownbeat();
   mRecorderMode = kRecorderMode_Loop;
   mFreeRecording = false;
}

void LooperRecorder::CancelFreeRecord()
{
   mFreeRecording = false;
   mStartFreeRecordTime = 0;
}

void LooperRecorder::ButtonClicked(ClickButton* button)
{
   if (button == mResampleButton)
      SyncLoopLengths();

   if (button == mResampAndSetButton)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed()+.5f));
      SyncLoopLengths();
   }

   if (button == mDoubleTempoButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->DoubleNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo()*2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos(gTime) + (TheTransport->GetMeasure(gTime)%8);
      int count = TheTransport->GetMeasure(gTime) - int(pos);
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
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalveNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo()/2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos(gTime) + (TheTransport->GetMeasure(gTime)%8);
      int count = TheTransport->GetMeasure(gTime) - int(pos);
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
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ShiftMeasure();
      }
      int newMeasure = TheTransport->GetMeasure(gTime)-1;
      if (newMeasure < 0)
         newMeasure = 7;
      TheTransport->SetMeasure(newMeasure);
   }

   if (button == mHalfShiftButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalfShift();
      }
      int newMeasure = int(TheTransport->GetMeasure(gTime)+TheTransport->GetMeasurePos(gTime)-.5f);
      if (newMeasure < 0)
         newMeasure = 7;
      float newMeasurePos = TheTransport->GetMeasurePos(gTime) - .5f;
      FloatWrap(newMeasurePos,1);
      TheTransport->SetMeasureTime(newMeasure + newMeasurePos);
   }

   if (button == mShiftDownbeatButton)
   {
      TheTransport->SetMeasure(TheTransport->GetMeasure(gTime)/8 * 8);   //align to 8 bars
      TheTransport->SetDownbeat();
      for (int i=0; i<mLoopers.size(); ++i)
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
      if (button == mCommit1BarButton)  numBars = 1;
      if (button == mCommit2BarsButton) numBars = 2;
      if (button == mCommit4BarsButton) numBars = 4; 
      if (button == mCommit8BarsButton) numBars = 8;

      mNumBars = numBars;
      if (mNextCommitTargetIndex < (int)mLoopers.size())
         Commit(mLoopers[mNextCommitTargetIndex]);
      for (int i=0; i<(int)mLoopers.size(); ++i)
      {
         mNextCommitTargetIndex = (mNextCommitTargetIndex + 1) % mLoopers.size();
         if (mLoopers[mNextCommitTargetIndex] != nullptr)
            break;
      }
   }
}

void LooperRecorder::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mFreeRecordingCheckbox)
   {
      bool freeRec = mFreeRecording;
      mFreeRecording = !mFreeRecording; //flip back so these methods won't be ignored
      if (freeRec)
         StartFreeRecord();
      else
         EndFreeRecord();
   }
}

void LooperRecorder::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void LooperRecorder::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void LooperRecorder::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mNumBarsSelector)
   {
      mNumBars = MAX(1, mNumBars);
   }
}

void LooperRecorder::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void LooperRecorder::Poll()
{
}

void LooperRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("headphonestarget", moduleInfo, "", FillDropdown<IAudioReceiver*>);
   mModuleSaveData.LoadString("outputtarget", moduleInfo, "", FillDropdown<IAudioReceiver*>);
   
   if (!moduleInfo["loopers"].isNull())
   {
      mLoopers.resize(moduleInfo["loopers"].size());
      for (int i=0; i<moduleInfo["loopers"].size(); ++i)
         mLoopers[i] = dynamic_cast<Looper*>(TheSynth->FindModule(moduleInfo["loopers"][i].asString()));
      SyncCablesToLoopers();
   }

   SetUpFromSaveData();
}

void LooperRecorder::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["loopers"].resize((unsigned int)mLoopers.size());
   for (int i=0; i<mLoopers.size(); ++i)
   {
      std::string name;
      if (mLoopers[i])
         name = mLoopers[i]->Name();
      moduleInfo["loopers"][i] = name;
   }
}

void LooperRecorder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetHeadphonesTarget(TheSynth->FindAudioReceiver(mModuleSaveData.GetString("headphonestarget")));
   SetOutputTarget(TheSynth->FindAudioReceiver(mModuleSaveData.GetString("outputtarget")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void LooperRecorder::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mBaseTempo;
   out << mSpeed;
   mRecordBuffer.SaveState(out);
}

void LooperRecorder::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mBaseTempo;
   in >> mSpeed;
   mRecordBuffer.LoadState(in);
}


