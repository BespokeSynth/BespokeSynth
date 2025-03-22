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
//  Looper.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "Looper.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "OpenFrameworksPort.h"
#include "LooperRecorder.h"
#include "Sample.h"
#include "MidiController.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Rewriter.h"
#include "LooperGranulator.h"

float Looper::mBeatwheelPosRight = 0;
float Looper::mBeatwheelDepthRight = 0;
float Looper::mBeatwheelPosLeft = 0;
float Looper::mBeatwheelDepthLeft = 0;
bool Looper::mBeatwheelSingleMeasure = 0;

namespace
{
   const int kMaxNumBars = 16;
}

Looper::Looper()
: IAudioProcessor(gBufferSize)
, mWorkBuffer(gBufferSize)
{
   //TODO(Ryan) buffer sizes
   mBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
   mUndoBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
   Clear();

   mMuteRamp.SetValue(1);

   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mPitchShifter[i] = new PitchShifter(1024);
      mLastInputSample[i] = 0;
   }

   SetLoopLength(4 * 60.0f / TheTransport->GetTempo() * gSampleRate);
}

void Looper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mNumBarsSelector = new DropdownList(this, "num bars", 3, 3, &mNumBars);
   mClearButton = new ClickButton(this, "clear", -1, -1);
   mVolSlider = new FloatSlider(this, "volume", 3, 98, 130, 15, &mVol, 0, 2);
   mWriteInputCheckbox = new Checkbox(this, "write", 80, 3, &mWriteInput);
   mCommitButton = new ClickButton(this, "commit", 126, 3);
   mQueueCaptureButton = new ClickButton(this, "capture", 126, 3);
   mMuteCheckbox = new Checkbox(this, "mute", -1, -1, &mMute);
   mPassthroughCheckbox = new Checkbox(this, "passthrough", -1, -1, &mPassthrough);
   mUndoButton = new ClickButton(this, "undo", -1, -1);
   mAllowScratchCheckbox = new Checkbox(this, "scr", -1, -1, &mAllowScratch);
   mVolumeBakeButton = new ClickButton(this, "b", -1, -1);
   mMergeButton = new ClickButton(this, " m ", -1, -1);
   mDecaySlider = new FloatSlider(this, "decay", -1, -1, 65, 15, &mDecay, 0, 1, 2);
   mSaveButton = new ClickButton(this, "save", -1, -1);
   mSwapButton = new ClickButton(this, "swap", 137, 81);
   mCopyButton = new ClickButton(this, "copy", 140, 65);
   mDoubleSpeedButton = new ClickButton(this, "2x", 127, 19);
   mHalveSpeedButton = new ClickButton(this, ".5x", 149, 19);
   mExtendButton = new ClickButton(this, "extend", 127, 34);
   mLoopPosOffsetSlider = new FloatSlider(this, "offset", -1, -1, 130, 15, &mLoopPosOffset, 0, mLoopLength);
   mWriteOffsetButton = new ClickButton(this, "apply", -1, -1);
   mScratchSpeedSlider = new FloatSlider(this, "scrspd", -1, -1, 130, 15, &mScratchSpeed, -2, 2);
   mFourTetSlider = new FloatSlider(this, "fourtet", 4, 65, 65, 15, &mFourTet, 0, 1, 1);
   mFourTetSlicesDropdown = new DropdownList(this, "fourtetslices", -1, -1, &mFourTetSlices);
   mPitchShiftSlider = new FloatSlider(this, "pitch", -1, -1, 130, 15, &mPitchShift, .5f, 2);
   mBeatwheelCheckbox = new Checkbox(this, "beatwheel on", HIDDEN_UICONTROL, HIDDEN_UICONTROL, &mBeatwheel);
   mBeatwheelPosRightSlider = new FloatSlider(this, "beatwheel pos right", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 1, 1, &mBeatwheelPosRight, 0, 1);
   mBeatwheelDepthRightSlider = new FloatSlider(this, "beatwheel depth right", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 1, 1, &mBeatwheelDepthRight, 0, 1);
   mBeatwheelPosLeftSlider = new FloatSlider(this, "beatwheel pos left", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 1, 1, &mBeatwheelPosLeft, 0, 1);
   mBeatwheelDepthLeftSlider = new FloatSlider(this, "beatwheel depth left", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 1, 1, &mBeatwheelDepthLeft, 0, 1);
   mBeatwheelSingleMeasureCheckbox = new Checkbox(this, "beatwheel single measure", HIDDEN_UICONTROL, HIDDEN_UICONTROL, &mBeatwheelSingleMeasure);
   mKeepPitchCheckbox = new Checkbox(this, "auto", -1, -1, &mKeepPitch);
   mResampleButton = new ClickButton(this, "resample for tempo", 15, 40);

   mNumBarsSelector->AddLabel(" 1 ", 1);
   mNumBarsSelector->AddLabel(" 2 ", 2);
   mNumBarsSelector->AddLabel(" 3 ", 3);
   mNumBarsSelector->AddLabel(" 4 ", 4);
   mNumBarsSelector->AddLabel(" 6 ", 6);
   mNumBarsSelector->AddLabel(" 8 ", 8);
   mNumBarsSelector->AddLabel("12 ", 12);
   mNumBarsSelector->AddLabel("16 ", 16);
   static_assert(kMaxNumBars == 16);

   mFourTetSlicesDropdown->AddLabel(" 1", 1);
   mFourTetSlicesDropdown->AddLabel(" 2", 2);
   mFourTetSlicesDropdown->AddLabel(" 4", 4);
   mFourTetSlicesDropdown->AddLabel(" 8", 8);
   mFourTetSlicesDropdown->AddLabel("16", 16);

   mBeatwheelPosLeftSlider->SetClamped(false);
   mBeatwheelPosRightSlider->SetClamped(false);

   mDecaySlider->SetMode(FloatSlider::kSquare);

   mClearButton->PositionTo(mNumBarsSelector, kAnchor_Right);
   mVolumeBakeButton->PositionTo(mVolSlider, kAnchor_Right);
   mMergeButton->PositionTo(mVolumeBakeButton, kAnchor_Right);
   mDecaySlider->PositionTo(mNumBarsSelector, kAnchor_Below);
   mFourTetSlicesDropdown->PositionTo(mFourTetSlider, kAnchor_Right);
   mMuteCheckbox->PositionTo(mFourTetSlider, kAnchor_Below);
   mSaveButton->PositionTo(mMuteCheckbox, kAnchor_Right);
   mUndoButton->PositionTo(mSaveButton, kAnchor_Right);
   mPitchShiftSlider->PositionTo(mVolSlider, kAnchor_Below);
   mKeepPitchCheckbox->PositionTo(mPitchShiftSlider, kAnchor_Right);
   mLoopPosOffsetSlider->PositionTo(mPitchShiftSlider, kAnchor_Below);
   mWriteOffsetButton->PositionTo(mLoopPosOffsetSlider, kAnchor_Right);
   mScratchSpeedSlider->PositionTo(mLoopPosOffsetSlider, kAnchor_Below);
   mAllowScratchCheckbox->PositionTo(mScratchSpeedSlider, kAnchor_Right);
   mPassthroughCheckbox->PositionTo(mScratchSpeedSlider, kAnchor_Below);
}

Looper::~Looper()
{
   delete mBuffer;
   delete mUndoBuffer;
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
      delete mPitchShifter[i];
}

void Looper::Exit()
{
   IDrawableModule::Exit();
   if (mRecorder)
      mRecorder->RemoveLooper(this);
}

void Looper::SetRecorder(LooperRecorder* recorder)
{
   mRecorder = recorder;
   if (recorder)
      GetBuffer()->SetNumActiveChannels(recorder->GetRecordBuffer()->NumChannels());
}

ChannelBuffer* Looper::GetLoopBuffer(int& loopLength)
{
   loopLength = mLoopLength;
   return mBuffer;
}

void Looper::SetLoopBuffer(ChannelBuffer* buffer)
{
   mQueuedNewBuffer = buffer;
}

void Looper::Poll()
{
   if (mClearCommitBuffer)
   {
      if (mRecorder == nullptr)
         mCommitBuffer->ClearBuffer();
      mCommitBuffer = nullptr;
      mClearCommitBuffer = false;
   }

   mCommitButton->SetShowing(mRecorder != nullptr);
   mSwapButton->SetShowing(mRecorder != nullptr);
   mCopyButton->SetShowing(mRecorder != nullptr);
   mMergeButton->SetShowing(mRecorder != nullptr);
   mWriteInputCheckbox->SetShowing(mRecorder == nullptr);
   mQueueCaptureButton->SetShowing(mRecorder == nullptr);

   if (mGranulator && mGranulator->IsDeleted())
      mGranulator = nullptr;
}

void Looper::Process(double time)
{
   PROFILER(Looper);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   if (!mEnabled)
   {
      SyncBuffers();

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();

      return;
   }

   ComputeSliders(0);
   int numChannels = MAX(GetBuffer()->NumActiveChannels(), mBuffer->NumActiveChannels());
   if (mRecorder)
      numChannels = MAX(numChannels, mRecorder->GetRecordBuffer()->NumChannels());
   GetBuffer()->SetNumActiveChannels(numChannels);
   SyncBuffers();
   mBuffer->SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mWorkBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mUndoBuffer->SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   bool doGranular = mGranulator != nullptr && mGranulator->IsActive();

   if (mWantBakeVolume)
      BakeVolume();
   if (mWantShiftDownbeat)
      DoShiftDownbeat();

   int bufferSize = GetBuffer()->BufferSize();

   float oldLoopPos = mLoopPos;
   int sampsPerBar = mLoopLength / mNumBars;
   if (!doGranular || !mGranulator->ShouldFreeze())
      mLoopPos = sampsPerBar * ((TheTransport->GetMeasure(time) % mNumBars) + TheTransport->GetMeasurePos(time));

   double speed = GetPlaybackSpeed();

   if (oldLoopPos > mLoopLength - bufferSize * speed - 1 && mLoopPos < oldLoopPos)
   {
      ++mLoopCount;
      /*if (mLoopCount > 6 && mMute == false && mDecay)
         mVol *= ofMap(TheTransport->GetTempo(), 80.0f, 160.0f, .95f, .99f, true);*/
      if (mMute == false)
         mVol *= 1 - mDecay;

      if (mCaptureQueued && !mWriteInput)
      {
         mWriteInputRamp.Start(time, 1, time + 10);
         mWriteInput = true;
      }
      else if (mWriteInput && mCaptureQueued)
      {
         mCaptureQueued = false;
         mWriteInputRamp.Start(time, 0, time + 10);
         mWriteInput = false;
      }
   }

   if (speed == 1)
   {
      //TODO(Ryan) reevaluate
      //mLoopPos = int(mLoopPos);  //fix in-between sample error
   }
   else
   {
      mAllowScratch = false;
   }

   if (mQueuedNewBuffer)
   {
      mBufferMutex.lock();
      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
         mJumpBlender[ch].CaptureForJump(mLoopPos, mBuffer->GetChannel(ch), mLoopLength, 0);
      mBuffer = mQueuedNewBuffer;
      mBufferMutex.unlock();
      mQueuedNewBuffer = nullptr;
   }

   if (mKeepPitch)
      mPitchShift = 1 / speed;
   int latencyOffset = 0;
   if (mPitchShift != 1)
      latencyOffset = mPitchShifter[0]->GetLatency();

   double processStartTime = time;
   for (int i = 0; i < bufferSize; ++i)
   {
      float smooth = .001f;
      mSmoothedVol = mSmoothedVol * (1 - smooth) + mVol * smooth;
      float volSq = mSmoothedVol * mSmoothedVol;

      mLoopPosOffsetSlider->Compute(i);

      if (mAllowScratch)
         ProcessScratch();

      if (mFourTet > 0)
         ProcessFourTet(processStartTime, i);

      if (mBeatwheel)
         ProcessBeatwheel(processStartTime, i);

      float offset = mLoopPos + i * speed + mLoopPosOffset + latencyOffset;
      float output[ChannelBuffer::kMaxNumChannels];
      ::Clear(output, ChannelBuffer::kMaxNumChannels);

      if (doGranular)
         mGranulator->ProcessFrame(time, offset, output);

      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
      {
         if (!doGranular)
         {
            output[ch] = GetInterpolatedSample(offset, mBuffer->GetChannel(ch), mLoopLength);
            output[ch] = mJumpBlender[ch].Process(output[ch], i);
         }

         if (mFourTet > 0 && mFourTet < 1) //fourtet wet/dry
         {
            output[ch] *= mFourTet;
            float normalOffset = mLoopPos + i * speed;
            output[ch] += GetInterpolatedSample(normalOffset, mBuffer->GetChannel(ch), mLoopLength) * (1 - mFourTet);
         }

         //write one sample the past so we don't end up feeding into the next output
         float writeAmount = mWriteInputRamp.Value(time);
         if (writeAmount > 0)
            WriteInterpolatedSample(offset - 1, mBuffer->GetChannel(ch), mLoopLength, mLastInputSample[ch] * writeAmount);
         mLastInputSample[ch] = GetBuffer()->GetChannel(ch)[i];

         output[ch] = mSwitchAndRamp.Process(ch, output[ch] * volSq);

         mWorkBuffer.GetChannel(ch)[i] = output[ch] * mMuteRamp.Value(time);

         if (mPassthrough)
            GetVizBuffer()->Write(mWorkBuffer.GetChannel(ch)[i] + GetBuffer()->GetChannel(ch)[i], ch);
         else
            GetVizBuffer()->Write(mWorkBuffer.GetChannel(ch)[i], ch);
      }

      time += gInvSampleRateMs;
   }

   if (mPitchShift != 1)
   {
      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
      {
         mPitchShifter[ch]->SetRatio(mPitchShift);
         mPitchShifter[ch]->Process(mWorkBuffer.GetChannel(ch), bufferSize);
      }
   }

   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
   {
      if (mPassthrough)
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
      Add(target->GetBuffer()->GetChannel(ch), mWorkBuffer.GetChannel(ch), bufferSize);
      GetVizBuffer()->WriteChunk(target->GetBuffer()->GetChannel(ch), bufferSize, ch);
   }

   GetBuffer()->Reset();

   if (mCommitBuffer && !mClearCommitBuffer && !mWantRewrite)
      DoCommit(time);
   if (mWantShiftMeasure)
      DoShiftMeasure();
   if (mWantHalfShift)
      DoHalfShift();
   if (mWantShiftOffset)
      DoShiftOffset();
   if (mWantUndo)
      DoUndo();
   if (mWantRewrite)
   {
      mWantRewrite = false;
      if (mRewriter)
         mRewriter->Go(time);
   }
}

void Looper::DoCommit(double time)
{
   PROFILER(LooperDoCommit);

   assert(mCommitBuffer);

   {
      PROFILER(Looper_DoCommit_undo);
      mUndoBuffer->CopyFrom(mBuffer, mLoopLength);
   }

   if (mReplaceOnCommit)
      Clear();

   if (mMute)
   {
      Clear();
      mMute = false;
      mMuteRamp.Start(time, mMute ? 0 : 1, time + 1);
   }

   {
      PROFILER(LooperDoCommit_commit);
      int commitSamplesBack = mCommitMsOffset / gInvSampleRateMs;
      int commitLength = mLoopLength + LOOPER_COMMIT_FADE_SAMPLES;
      for (int i = 0; i < commitLength; ++i)
      {
         int idx = i - LOOPER_COMMIT_FADE_SAMPLES;
         int pos = int(mLoopPos + (idx * GetPlaybackSpeed()) + mLoopLength) % mLoopLength;
         float fade = 1;
         if (idx < 0)
            fade = float(LOOPER_COMMIT_FADE_SAMPLES + idx) / LOOPER_COMMIT_FADE_SAMPLES;
         if (idx >= mLoopLength - LOOPER_COMMIT_FADE_SAMPLES)
            fade = 1 - (float(idx - (mLoopLength - LOOPER_COMMIT_FADE_SAMPLES)) / LOOPER_COMMIT_FADE_SAMPLES);

         for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
         {
            mBuffer->GetChannel(ch)[pos] += mCommitBuffer->GetSample(ofClamp(commitLength - i - commitSamplesBack, 0, MAX_BUFFER_SIZE - 1), ch) * fade;
         }
      }
   }

   mClearCommitBuffer = true;
}

void Looper::Fill(ChannelBuffer* buffer, int length)
{
   mBuffer->CopyFrom(buffer, length);
}

void Looper::DoUndo()
{
   ChannelBuffer* swap = mUndoBuffer;
   mUndoBuffer = mBuffer;
   mBuffer = swap;
   mWantUndo = false;
}

int Looper::GetRecorderNumBars() const
{
   if (mRecorder)
      return mRecorder->GetNumBars();
   return GetNumBars();
}

double Looper::GetPlaybackSpeed() const
{
   return mSpeed * TheTransport->GetTempo() / mBufferTempo;
}

void Looper::ProcessScratch()
{
   mLoopPosOffset = FloatWrap(mLoopPosOffset - GetPlaybackSpeed() + mScratchSpeed, mLoopLength);
}

void Looper::ProcessFourTet(double time, int sampleIdx)
{
   float measurePos = TheTransport->GetMeasurePos(time) + sampleIdx / (TheTransport->MsPerBar() / gInvSampleRateMs);
   measurePos += TheTransport->GetMeasure(time) % mNumBars;
   measurePos /= mNumBars;
   int numSlices = mFourTetSlices * 2 * mNumBars;
   measurePos *= numSlices;
   int slice = (int)measurePos;
   float sliceProgress = measurePos - slice;
   float oldOffset = mLoopPosOffset;
   if (slice % 2 == 0)
      mLoopPosOffset = (sliceProgress + slice / 2) * (mLoopLength / float(numSlices) * 2);
   else
      mLoopPosOffset = (1 - sliceProgress + slice / 2) * (mLoopLength / float(numSlices) * 2);

   //offset regular movement
   mLoopPosOffset = FloatWrap(mLoopPosOffset - (mLoopPos + sampleIdx * GetPlaybackSpeed()), mLoopLength);

   //smooth discontinuity
   if (oldOffset >= mLoopLength * .5f && mLoopPosOffset < mLoopLength * .5f)
      mSwitchAndRamp.StartSwitch();
}

void Looper::ProcessBeatwheel(double time, int sampleIdx)
{
   bool bothHeld = false;
   bool noneHeld = false;
   float clockPos = 0;
   if (mBeatwheelDepthRight > 0 && mBeatwheelDepthLeft > 0 &&
       mBeatwheelPosRight >= 0 && mBeatwheelPosLeft >= 0)
   {
      if (mBeatwheelControlFlip)
         clockPos = mBeatwheelPosRight;
      else
         clockPos = mBeatwheelPosLeft;
      bothHeld = true;
   }
   else if (mBeatwheelDepthRight > 0 && mBeatwheelPosRight >= 0)
   {
      clockPos = mBeatwheelPosRight;
   }
   else if (mBeatwheelDepthLeft > 0 && mBeatwheelPosLeft >= 0)
   {
      clockPos = mBeatwheelPosLeft;
   }
   else
   {
      noneHeld = true;
   }

   int depthLevel = GetBeatwheelDepthLevel();
   if (noneHeld)
      depthLevel = 2;
   int slicesPerBar = TheTransport->GetTimeSigTop() * (1 << depthLevel);
   int lastSlice = GetMeasureSliceIndex(time, sampleIdx - 1, slicesPerBar);
   int slice = GetMeasureSliceIndex(time, sampleIdx, slicesPerBar);
   int numSlices = slicesPerBar * mNumBars;
   int loopLength = mLoopLength;
   if (mBeatwheelSingleMeasure)
   {
      numSlices = slicesPerBar;
      loopLength = mLoopLength / mNumBars;
   }

   if (lastSlice != slice) //on new slices
   {
      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
         mJumpBlender[ch].CaptureForJump(int(GetActualLoopPos(sampleIdx)) % loopLength, mBuffer->GetChannel(ch), loopLength, sampleIdx);

      if (noneHeld)
      {
         mLoopPosOffset = 0;
      }
      else
      {
         if (bothHeld) //we should pingpong
            mBeatwheelControlFlip = !mBeatwheelControlFlip;

         int playSlice = int(clockPos * numSlices);

         mLoopPosOffset = playSlice * (loopLength / numSlices);

         //offset regular movement
         mLoopPosOffset = FloatWrap(mLoopPosOffset - (mLoopPos + sampleIdx * GetPlaybackSpeed()), mLoopLength);
      }
   }
}

int Looper::GetBeatwheelDepthLevel() const
{
   float depth = MAX(mBeatwheelDepthLeft, mBeatwheelDepthRight);
   if (depth == 0)
      return 0;
   if (depth < 1)
      return 1;
   return 2;
}

float Looper::GetActualLoopPos(int samplesIn) const
{
   float pos = FloatWrap(mLoopPos + mLoopPosOffset + samplesIn, mLoopLength);
   return pos;
}

int Looper::GetMeasureSliceIndex(double time, int sampleIdx, int slicesPerBar)
{
   float measurePos = TheTransport->GetMeasurePos(time) + sampleIdx / (TheTransport->MsPerBar() / gInvSampleRateMs);
   measurePos += TheTransport->GetMeasure(time) % mNumBars;
   measurePos /= mNumBars;
   int numSlices = slicesPerBar * mNumBars;
   measurePos *= numSlices;
   int slice = (int)measurePos;
   return slice;
}

void Looper::ResampleForSpeed(float speed)
{
   int oldLoopLength = mLoopLength;
   SetLoopLength(MIN(int(abs(mLoopLength / speed)), MAX_BUFFER_SIZE - 1));
   mLoopPos /= speed;
   while (mLoopPos < 0)
      mLoopPos += mLoopLength;
   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
   {
      float* oldBuffer = new float[oldLoopLength];
      BufferCopy(oldBuffer, mBuffer->GetChannel(ch), oldLoopLength);
      for (int i = 0; i < mLoopLength; ++i)
      {
         float offset = i * speed;
         mBuffer->GetChannel(ch)[i] = GetInterpolatedSample(offset, oldBuffer, oldLoopLength);
      }
      delete[] oldBuffer;
   }

   if (mKeepPitch)
   {
      mKeepPitch = false;
      mPitchShift = 1 / speed;
   }

   mSpeed = 1;
   mBufferTempo = TheTransport->GetTempo();
}

namespace
{
   const float kBufferX = 3;
   const float kBufferY = 3;
   const float kBufferWidth = 170;
   const float kBufferHeight = 93;
}

void Looper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushMatrix();

   ofTranslate(kBufferX, kBufferY);

   assert(mLoopLength > 0);

   float displayPos = GetActualLoopPos(0);
   mBufferMutex.lock();
   DrawAudioBuffer(kBufferWidth, kBufferHeight, mBuffer, 0, mLoopLength, displayPos, mVol);
   mBufferMutex.unlock();
   ofSetColor(255, 255, 0, gModuleDrawAlpha);
   for (int i = 1; i < mNumBars; ++i)
   {
      float x = kBufferWidth / mNumBars * i;
      ofLine(x, kBufferHeight / 2 - 5, x, kBufferHeight / 2 + 5);
   }
   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   ofPopMatrix();

   mClearButton->Draw();
   mNumBarsSelector->Draw();
   mVolSlider->Draw();
   mVolumeBakeButton->Draw();
   mDecaySlider->Draw();
   mDoubleSpeedButton->Draw();
   mHalveSpeedButton->Draw();
   mExtendButton->Draw();
   mUndoButton->Draw();
   mLoopPosOffsetSlider->Draw();
   mWriteOffsetButton->Draw();
   mScratchSpeedSlider->Draw();
   mAllowScratchCheckbox->Draw();
   mFourTetSlider->Draw();
   mFourTetSlicesDropdown->Draw();
   mPitchShiftSlider->Draw();
   mKeepPitchCheckbox->Draw();
   mWriteInputCheckbox->Draw();
   mQueueCaptureButton->Draw();
   mResampleButton->SetShowing(mBufferTempo != TheTransport->GetTempo());
   mResampleButton->Draw();
   if (mCaptureQueued)
   {
      ofPushStyle();
      ofFill();
      if (mWriteInput)
         ofSetColor(255, 0, 0, 150);
      else
         ofSetColor(255, 100, 0, 100 + 50 * (cosf(TheTransport->GetMeasurePos(gTime) * 4 * FTWO_PI)));
      ofRect(mQueueCaptureButton->GetRect(true));
      ofPopStyle();
   }

   mMergeButton->Draw();
   if (mRecorder && mRecorder->GetMergeSource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0, 100);
      ofRect(mMergeButton->GetRect(true));
      ofPopStyle();
   }

   mSwapButton->Draw();
   if (mRecorder && mRecorder->GetSwapSource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(0, 0, 255, 100);
      ofRect(mSwapButton->GetRect(true));
      ofPopStyle();
   }

   mCopyButton->Draw();
   if (mRecorder && mRecorder->GetCopySource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(0, 0, 255, 100);
      ofRect(mCopyButton->GetRect(true));
      ofPopStyle();
   }

   mSaveButton->Draw();
   mMuteCheckbox->Draw();
   mPassthroughCheckbox->Draw();
   mCommitButton->Draw();

   if (mGranulator)
      mGranulator->DrawOverlay(ofRectangle(4, 35, 155, 32), mLoopLength);

   if (mBeatwheel)
      DrawBeatwheel();

   if (mRecorder != nullptr && mRecorder->GetNextCommitTarget() == this)
   {
      ofPushStyle();
      float w, h;
      GetDimensions(w, h);
      ofSetColor(255, 255, 255, 200);
      ofSetLineWidth(3);
      ofRect(0, 0, w, h);
      ofPopStyle();
   }
}

void Looper::DrawBeatwheel()
{
   ofPushMatrix();
   ofPushStyle();

   float size = 197;

   ofTranslate(0, -size);

   ofFill();
   ofSetColor(50, 50, 50, gModuleDrawAlpha * .65f);
   ofRect(0, 0, size, size);

   float centerX = size / 2;
   float centerY = size / 2;
   float innerRad = size * .2f;
   float outerRad = size * .45f;
   float waveformCenter = (innerRad + outerRad) / 2;
   float waveformHeight = (outerRad - innerRad) / 2;

   ofSetCircleResolution(100);
   ofSetColor(100, 100, 100, gModuleDrawAlpha * .8f);
   ofCircle(size / 2, size / 2, outerRad);
   ofSetColor(50, 50, 50, gModuleDrawAlpha * .5f);
   ofCircle(size / 2, size / 2, innerRad);

   ofSetLineWidth(1);
   ofNoFill();

   int depthLevel = GetBeatwheelDepthLevel();
   int slicesPerBar = TheTransport->GetTimeSigTop() * (1 << depthLevel);
   int numSlices = slicesPerBar * mNumBars;
   int loopLength = mLoopLength;
   if (mBeatwheelSingleMeasure)
   {
      numSlices = slicesPerBar;
      loopLength = mLoopLength / mNumBars;
   }

   ofSetColor(0, 0, 0, gModuleDrawAlpha);
   float subdivisions = 600;
   int samplesPerPixel = loopLength / subdivisions;

   for (int i = 0; i < subdivisions; i++)
   {
      float radians = (i * TWO_PI) / subdivisions;
      //ofSetColor(200,200,200,gModuleDrawAlpha);
      float sinR = sin(radians);
      float cosR = cos(radians);
      //ofLine(centerX+sinR*innerRad, centerY-cosR*innerRad, centerX+sinR*outerRad, centerY-cosR*outerRad);
      float mag = 0;
      int position = i * samplesPerPixel;
      //rms
      int j;
      for (j = 0; j < samplesPerPixel && position + j < loopLength - 1; ++j)
      {
         for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
            mag += mBuffer->GetChannel(ch)[position + j];
      }
      mag /= j;
      mag = sqrtf(mag);
      mag = sqrtf(mag);
      mag = MIN(1.0f, mag);
      float inner = (waveformCenter - mag * waveformHeight);
      float outer = (waveformCenter + mag * waveformHeight);
      //ofSetColor(0,0,0,gModuleDrawAlpha);
      ofLine(centerX + sinR * inner, centerY - cosR * inner, centerX + sinR * outer, centerY - cosR * outer);
   }

   ofSetLineWidth(3);
   ofSetColor(0, 255, 0, gModuleDrawAlpha);
   float displayPos = GetActualLoopPos(0);
   float position = displayPos / loopLength;
   float radians = position * TWO_PI;
   float sinR = sin(radians);
   float cosR = cos(radians);
   ofLine(centerX + sinR * innerRad, centerY - cosR * innerRad, centerX + sinR * outerRad, centerY - cosR * outerRad);

   ofSetLineWidth(4);
   ofSetColor(255, 0, 0, gModuleDrawAlpha);
   if (mBeatwheelDepthRight > 0 && mBeatwheelPosRight >= 0)
   {
      radians = mBeatwheelPosRight * TWO_PI;
      sinR = sin(radians);
      cosR = cos(radians);
      ofLine(centerX + sinR * outerRad * .9f, centerY - cosR * outerRad * .9f, centerX + sinR * outerRad, centerY - cosR * outerRad);
   }
   if (mBeatwheelDepthLeft > 0 && mBeatwheelPosLeft >= 0)
   {
      radians = mBeatwheelPosLeft * TWO_PI;
      sinR = sin(radians);
      cosR = cos(radians);
      ofLine(centerX + sinR * outerRad * .9f, centerY - cosR * outerRad * .9f, centerX + sinR * outerRad, centerY - cosR * outerRad);
   }

   if (depthLevel > 0)
   {
      ofSetLineWidth(1);
      ofSetColor(150, 150, 150, gModuleDrawAlpha);
      for (int i = 0; i < numSlices; ++i)
      {
         radians = (i * TWO_PI) / numSlices;
         sinR = sin(radians);
         cosR = cos(radians);
         ofLine(centerX + sinR * waveformCenter, centerY - cosR * waveformCenter, centerX + sinR * outerRad, centerY - cosR * outerRad);
      }
   }

   ofPopStyle();
   ofPopMatrix();
}

bool Looper::DrawToPush2Screen()
{
   ofPushMatrix();

   ofTranslate(60, 3);
   float displayPos = GetActualLoopPos(0);
   mBufferMutex.lock();
   DrawAudioBuffer(180, 74, mBuffer, 0, mLoopLength, displayPos, mVol);
   mBufferMutex.unlock();

   ofPopMatrix();

   return false;
}

void Looper::Clear()
{
   mBuffer->Clear();
   mLastCommitTime = gTime;
   mVol = 1;
   mFourTet = 0;
   mPitchShift = 1;
}

void Looper::SetNumBars(int numBars)
{
   numBars = MIN(numBars, kMaxNumBars);
   int oldNumBars = mNumBars;
   mNumBars = numBars;
   if (mNumBars != oldNumBars)
      UpdateNumBars(oldNumBars);
}

void Looper::BakeVolume()
{
   mUndoBuffer->CopyFrom(mBuffer, mLoopLength);
   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
      Mult(mBuffer->GetChannel(ch), mVol * mVol, mLoopLength);
   mVol = 1;
   mSmoothedVol = 1;
   mWantBakeVolume = false;
}

void Looper::UpdateNumBars(int oldNumBars)
{
   assert(mNumBars > 0);
   int sampsPerBar = abs(int(TheTransport->MsPerBar() / 1000 * gSampleRate));
   SetLoopLength(MIN(sampsPerBar * mNumBars, MAX_BUFFER_SIZE - 1));
   while (mLoopPos > sampsPerBar)
      mLoopPos -= sampsPerBar;
   mLoopPos += sampsPerBar * (TheTransport->GetMeasure(gTime) % mNumBars);
   if (oldNumBars < mNumBars)
   {
      int oldLoopLength = abs(int(TheTransport->MsPerBar() * oldNumBars / 1000 * gSampleRate));
      oldLoopLength = MIN(oldLoopLength, MAX_BUFFER_SIZE - 1);
      for (int i = 1; i < mNumBars / oldNumBars; ++i)
      {
         for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
            BufferCopy(mBuffer->GetChannel(ch) + oldLoopLength * i, mBuffer->GetChannel(ch), oldLoopLength);
      }
   }
}

void Looper::SetLoopLength(int length)
{
   assert(length > 0);
   mLoopLength = length;
   if (mLoopPosOffsetSlider != nullptr)
      mLoopPosOffsetSlider->SetExtents(0, length);
   mBufferTempo = TheTransport->GetTempo();
}

void Looper::MergeIn(Looper* otherLooper)
{
   int newNumBars = MAX(mNumBars, otherLooper->mNumBars);

   SetNumBars(newNumBars);

   otherLooper->SetNumBars(newNumBars);

   if (mVol > 0.01f)
   {
      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
      {
         Mult(otherLooper->mBuffer->GetChannel(ch), (otherLooper->mVol * otherLooper->mVol) / (mVol * mVol), mLoopLength); //keep other looper at same apparent volume
         Add(mBuffer->GetChannel(ch), otherLooper->mBuffer->GetChannel(ch), mLoopLength);
      }
   }
   else //ours was silent, just replace it
   {
      mBuffer->CopyFrom(otherLooper->mBuffer, mLoopLength);
      mVol = 1;
   }

   otherLooper->Clear();

   if (mRecorder)
      mLastCommit = mRecorder->IncreaseCommitCount();
}

void Looper::SwapBuffers(Looper* otherLooper)
{
   assert(otherLooper);
   ChannelBuffer* temp = otherLooper->mBuffer;
   int length = otherLooper->mLoopLength;
   int numBars = otherLooper->mNumBars;
   float vol = otherLooper->mVol;
   otherLooper->mVol = mVol;
   otherLooper->mBuffer = mBuffer;
   otherLooper->SetLoopLength(mLoopLength);
   otherLooper->mNumBars = mNumBars;
   mBuffer = temp;
   SetLoopLength(length);
   mNumBars = numBars;
   mVol = vol;
}

void Looper::CopyBuffer(Looper* sourceLooper)
{
   assert(sourceLooper);
   mBuffer->CopyFrom(sourceLooper->mBuffer, mLoopLength);
   SetLoopLength(sourceLooper->mLoopLength);
   mNumBars = sourceLooper->mNumBars;
}

void Looper::Commit(RollingBuffer* commitBuffer, bool replaceOnCommit, float offsetMs)
{
   if (mRecorder)
   {
      mRecorder->ResetSpeed();
      mLastCommit = mRecorder->IncreaseCommitCount();
   }
   BakeVolume();

   mLastCommitTime = gTime;
   mLoopCount = 0;

   mLoopPosOffset = 0;
   mLoopPosOffsetSlider->DisableLFO();
   mAllowScratch = false;
   mScratchSpeed = 1;
   mFourTet = 0;
   mPitchShift = 1;
   if (mGranulator)
      mGranulator->OnCommit();

   mCommitBuffer = commitBuffer;
   mReplaceOnCommit = replaceOnCommit;
   mCommitMsOffset = offsetMs;
}

void Looper::SetMute(double time, bool mute)
{
   mMute = mute;
   mMuteRamp.Start(time, mMute ? 0 : 1, time + 1);
}

void Looper::FilesDropped(std::vector<std::string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x, y, &sample);
}

void Looper::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   int numSamples = sample->LengthInSamples();

   if (numSamples <= 0)
      return;

   if (sample->GetNumBars() > 0)
      SetNumBars(sample->GetNumBars());

   float lengthRatio = float(numSamples) / mLoopLength;
   mBuffer->SetNumActiveChannels(sample->NumChannels());
   for (int i = 0; i < mLoopLength; ++i)
   {
      float offset = i * lengthRatio;
      for (int ch = 0; ch < sample->NumChannels(); ++ch)
         mBuffer->GetChannel(ch)[i] = GetInterpolatedSample(offset, sample->Data()->GetChannel(ch), numSamples);
   }
}

void Looper::GetModuleDimensions(float& width, float& height)
{
   width = kBufferX * 2 + kBufferWidth;
   height = 182;
}

void Looper::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (x >= kBufferX + kBufferWidth / 3 && x < kBufferX + (kBufferWidth * 2) / 3 &&
       y >= kBufferY + kBufferHeight / 3 && y < kBufferY + (kBufferHeight * 2) / 3 &&
       mBufferTempo == TheTransport->GetTempo() &&
       gHoveredUIControl == nullptr)
   {
      ChannelBuffer grab(mLoopLength);
      grab.SetNumActiveChannels(mBuffer->NumActiveChannels());
      for (int ch = 0; ch < grab.NumActiveChannels(); ++ch)
         BufferCopy(grab.GetChannel(ch), mBuffer->GetChannel(ch), mLoopLength);
      TheSynth->GrabSample(&grab, "loop", false, mNumBars);
   }
}

void Looper::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
   {
      mUndoBuffer->CopyFrom(mBuffer, mLoopLength);
      Clear();
   }
   if (button == mMergeButton && mRecorder)
      mRecorder->RequestMerge(this);
   if (button == mSwapButton && mRecorder)
      mRecorder->RequestSwap(this);
   if (button == mCopyButton && mRecorder)
      mRecorder->RequestCopy(this);
   if (button == mVolumeBakeButton)
      mWantBakeVolume = true;
   if (button == mSaveButton)
   {
      Sample::WriteDataToFile(ofGetTimestampString("loops/loop_%Y-%m-%d_%H-%M-%S.wav").c_str(), mBuffer, mLoopLength);
   }
   if (button == mCommitButton && mRecorder)
      mRecorder->Commit(this);
   if (button == mDoubleSpeedButton)
   {
      if (mSpeed == 1)
      {
         HalveNumBars();
         mSpeed = 2;
         ResampleForSpeed(GetPlaybackSpeed());
      }
   }
   if (button == mHalveSpeedButton)
   {
      if (mSpeed == 1 && mLoopLength < MAX_BUFFER_SIZE / 2)
      {
         DoubleNumBars();
         mSpeed = .5f;
         ResampleForSpeed(GetPlaybackSpeed());
      }
   }
   if (button == mExtendButton)
   {
      int newLength = mNumBars * 2;
      if (newLength <= 16)
         SetNumBars(newLength);
   }
   if (button == mUndoButton)
      mWantUndo = true;
   if (button == mWriteOffsetButton)
      mWantShiftOffset = true;
   if (button == mQueueCaptureButton)
   {
      mCaptureQueued = true;
      mLastCommitTime = time;
   }
   if (button == mResampleButton)
      ResampleForSpeed(GetPlaybackSpeed());
}

void Looper::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mScratchSpeedSlider)
   {
      if (!mAllowScratch && !TheSynth->IsLoadingState())
         mScratchSpeed = 1;
   }
   if (slider == mFourTetSlider)
   {
      if (mFourTet == 0 && !TheSynth->IsLoadingState())
         mLoopPosOffset = 0;
   }
   if (slider == mVolSlider)
   {
      if (mVol > oldVal)
      {
         mLoopCount = 0; //stop fading for a few loops
      }
   }
}

void Looper::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
}

void Looper::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mNumBarsSelector)
      UpdateNumBars(oldVal);
}

void Looper::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mAllowScratchCheckbox)
   {
      if (mAllowScratch == false)
      {
         mScratchSpeed = 1;
         mLoopPosOffset = 0;
      }
   }
   if (checkbox == mMuteCheckbox)
   {
      SetMute(time, mMute);
   }
   if (checkbox == mWriteInputCheckbox)
   {
      if (mWriteInput)
      {
         if (mBufferTempo != TheTransport->GetTempo())
            ResampleForSpeed(GetPlaybackSpeed());
         mWriteInputRamp.Start(time, 1, time + 10);
      }
      else
      {
         mWriteInputRamp.Start(time, 0, time + 10);
      }
   }
}

void Looper::HalveNumBars()
{
   if (mNumBars > 1)
   {
      mNumBars /= 2;
   }
   else
   {
      int bufferSize = int(TheTransport->MsPerBar() * mNumBars / 1000 * gSampleRate);
      if (bufferSize < MAX_BUFFER_SIZE / 2) //if we can fit it
      {
         //copy it over twice to make this just one bar
         mNumBars = 2;
         UpdateNumBars(1);
         mNumBars = 1;
      }
   }
}

void Looper::DoShiftMeasure()
{
   int measureSize = int(TheTransport->MsPerBar() * gSampleRate / 1000);
   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
   {
      float* newBuffer = new float[MAX_BUFFER_SIZE];
      BufferCopy(newBuffer, mBuffer->GetChannel(ch) + measureSize, mLoopLength - measureSize);
      BufferCopy(newBuffer + mLoopLength - measureSize, mBuffer->GetChannel(ch), measureSize);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantShiftMeasure = false;
}

void Looper::DoHalfShift()
{
   int halfMeasureSize = int(TheTransport->MsPerBar() * gSampleRate / 1000 / 2);
   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
   {
      float* newBuffer = new float[MAX_BUFFER_SIZE];
      BufferCopy(newBuffer, mBuffer->GetChannel(ch) + halfMeasureSize, mLoopLength - halfMeasureSize);
      BufferCopy(newBuffer + mLoopLength - halfMeasureSize, mBuffer->GetChannel(ch), halfMeasureSize);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantHalfShift = false;
}

void Looper::DoShiftDownbeat()
{
   int shift = int(mLoopPos);
   for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
   {
      float* newBuffer = new float[MAX_BUFFER_SIZE];
      BufferCopy(newBuffer, mBuffer->GetChannel(ch) + shift, mLoopLength - shift);
      BufferCopy(newBuffer + mLoopLength - shift, mBuffer->GetChannel(ch), shift);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantShiftDownbeat = false;
}

void Looper::DoShiftOffset()
{
   int shift = int(mLoopPosOffset);
   if (shift != 0)
   {
      for (int ch = 0; ch < mBuffer->NumActiveChannels(); ++ch)
      {
         float* newBuffer = new float[MAX_BUFFER_SIZE];
         BufferCopy(newBuffer, mBuffer->GetChannel(ch) + shift, mLoopLength - shift);
         BufferCopy(newBuffer + mLoopLength - shift, mBuffer->GetChannel(ch), shift);
         mBufferMutex.lock();
         mBuffer->SetChannelPointer(newBuffer, ch, true);
         mBufferMutex.unlock();
      }
   }
   mWantShiftOffset = false;
   mLoopPosOffset = 0;
}

void Looper::Rewrite()
{
   mWantRewrite = true;
}

void Looper::PlayNote(NoteMessage note)
{
   //jump around in loop
   if (note.velocity > 0)
   {
      float measurePos = fmod(note.pitch / 16.0f, mNumBars);
      float sampsPerBar = TheTransport->MsPerBar() / 1000.0f * gSampleRate;
      mLoopPosOffset = (measurePos - fmod(TheTransport->GetMeasureTime(note.time), mNumBars)) * sampsPerBar;
      if (mLoopPosOffset < 0)
         mLoopPosOffset += mLoopLength;
      mLoopPosOffsetSlider->DisableLFO();
   }
}

void Looper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("decay", moduleInfo, false);

   SetUpFromSaveData();
}

void Looper::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   mDecay = mModuleSaveData.GetFloat("decay");
}

void Looper::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mLoopLength;
   out << mBufferTempo;
   mBuffer->Save(out, mLoopLength);
}

void Looper::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mLoopLength;
   if (rev >= 1)
      in >> mBufferTempo;
   int readLength;
   mBuffer->Load(in, readLength, ChannelBuffer::LoadMode::kAnyBufferSize);
   assert(mLoopLength == readLength);
}
