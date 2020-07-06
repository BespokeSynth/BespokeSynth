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
#include "FillSaveDropdown.h"

float Looper::mBeatwheelPosRight = 0;
float Looper::mBeatwheelDepthRight = 0;
float Looper::mBeatwheelPosLeft = 0;
float Looper::mBeatwheelDepthLeft = 0;
bool Looper::mBeatwheelSingleMeasure = 0;

Looper::Looper()
: IAudioProcessor(gBufferSize)
, mLoopLength(4 * 60.0f / gDefaultTempo * gSampleRate)
, mLoopPos(0)
, mNumBars(1)
, mCommitBuffer(nullptr)
, mClearButton(nullptr)
, mNumBarsSelector(nullptr)
, mRecordBuffer(nullptr)
, mVol(1)
, mSmoothedVol(1)
, mVolSlider(nullptr)
, mSpeed(1.0f)
, mRecorder(nullptr)
, mMergeButton(nullptr)
, mVolumeBakeButton(nullptr)
, mWantBakeVolume(false)
, mLastCommit(0)
, mSaveButton(nullptr)
, mMute(false)
, mMuteCheckbox(nullptr)
, mWantShiftMeasure(false)
, mWantShiftDownbeat(false)
, mCommitButton(nullptr)
, mSwapButton(nullptr)
, mCopyButton(nullptr)
, mDoubleSpeedButton(nullptr)
, mHalveSpeedButton(nullptr)
, mUndoButton(nullptr)
, mWantUndo(false)
, mLoopPosOffset(0)
, mLoopPosOffsetSlider(nullptr)
, mResetOffsetButton(nullptr)
, mWriteOffsetButton(nullptr)
, mAllowChop(false)
, mAllowChopCheckbox(nullptr)
, mChopMeasure(0)
, mScratchSpeed(1)
, mAllowScratch(false)
, mScratchSpeedSlider(nullptr)
, mAllowScratchCheckbox(nullptr)
, mLastCommitTime(0)
, mFourTet(0)
, mFourTetSlider(nullptr)
, mFourTetSlices(4)
, mFourTetSlicesDropdown(nullptr)
, mShowGranular(false)
, mShowGranularCheckbox(nullptr)
, mGranular(false)
, mGranularCheckbox(nullptr)
, mGranOverlap(nullptr)
, mGranSpeed(nullptr)
, mGranLengthMs(nullptr)
, mPosSlider(nullptr)
, mPausePos(false)
, mPausePosCheckbox(nullptr)
, mGranPosRandomize(nullptr)
, mGranSpeedRandomize(nullptr)
, mGranOctaveCheckbox(nullptr)
, mBeatwheel(false)
, mBeatwheelCheckbox(nullptr)
, mBeatwheelPosRightSlider(nullptr)
, mBeatwheelDepthRightSlider(nullptr)
, mBeatwheelPosLeftSlider(nullptr)
, mBeatwheelDepthLeftSlider(nullptr)
, mBeatwheelControlFlip(false)
, mBeatwheelSingleMeasureCheckbox(nullptr)
, mClearCommitBuffer(false)
, mRewriter(nullptr)
, mWantRewrite(false)
, mLoopCount(0)
, mDecay(0)
, mDecaySlider(nullptr)
, mPitchShift(1)
, mPitchShiftSlider(nullptr)
, mKeepPitch(false)
, mKeepPitchCheckbox(nullptr)
, mWriteInput(false)
, mWriteInputCheckbox(nullptr)
, mQueueCaptureButton(nullptr)
, mCaptureQueued(false)
, mWantShiftOffset(false)
, mWantHalfShift(false)
, mWorkBuffer(gBufferSize)
, mQueuedNewBuffer(nullptr)
{
   //TODO(Ryan) buffer sizes
   mBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
   mUndoBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
   Clear();
   
   mMuteRamp.SetValue(1);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
   {
      mPitchShifter[i] = new PitchShifter(1024);
      mLastInputSample[i] = 0;
   }
}

void Looper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mNumBarsSelector = new DropdownList(this,"num bars",3, 3, &mNumBars);
   mClearButton = new ClickButton(this,"clear", -1, -1);
   mVolSlider = new FloatSlider(this,"volume", 3, 98, 130, 15, &mVol, 0, 2);
   mVolumeBakeButton = new ClickButton(this,"b", -1, -1);
   mMergeButton = new ClickButton(this," m ", -1, -1);
   mDecaySlider = new FloatSlider(this,"decay", -1, -1, 65, 15, &mDecay, 0, 1, 2);
   mSaveButton = new ClickButton(this,"save",-1,-1);
   mMuteCheckbox = new Checkbox(this,"mute",-1,-1,&mMute);
   mCommitButton = new ClickButton(this,"commit",126,3);
   mQueueCaptureButton = new ClickButton(this,"capture",126,3);
   mWriteInputCheckbox = new Checkbox(this,"write",80,3,&mWriteInput);
   mSwapButton = new ClickButton(this,"swap",137,81);
   mCopyButton = new ClickButton(this,"copy",140,65);
   mDoubleSpeedButton = new ClickButton(this,"2x",151,28);
   mHalveSpeedButton = new ClickButton(this,".5x",147,43);
   mUndoButton = new ClickButton(this,"undo",-1,-1);
   mLoopPosOffsetSlider = new FloatSlider(this,"offset",-1,-1,130,15,&mLoopPosOffset,0,mLoopLength);
   mResetOffsetButton = new ClickButton(this," r ",-1,-1);
   mWriteOffsetButton = new ClickButton(this," w ",-1,-1);
   mScratchSpeedSlider = new FloatSlider(this,"scrspd",-1,-1,130,15,&mScratchSpeed,-2,2);
   mAllowScratchCheckbox = new Checkbox(this,"scr",-1,-1,&mAllowScratch);
   mFourTetSlider = new FloatSlider(this,"fourtet",4,65,65,15,&mFourTet,0,1,1);
   mFourTetSlicesDropdown = new DropdownList(this,"fourtetslices",-1,-1,&mFourTetSlices);
   mShowGranularCheckbox = new Checkbox(this,"granular",-1,-1,&mShowGranular);
   mGranularCheckbox = new Checkbox(this,"g on",3,168,&mGranular);
   mGranOverlap = new FloatSlider(this,"g overlap",-1,-1,84,15,&mGranulator.mGrainOverlap,.5f,MAX_GRAINS);
   mGranSpeed = new FloatSlider(this,"g speed",-1,-1,84,15,&mGranulator.mSpeed,-3,3);
   mGranLengthMs = new FloatSlider(this,"g len ms",-1,-1,84,15,&mGranulator.mGrainLengthMs,1,200);
   mPosSlider = new FloatSlider(this,"pos",-1,-1,140,15,&mLoopPos,0,mLoopLength);
   mPausePosCheckbox = new Checkbox(this,"frz",-1,-1,&mPausePos);
   mGranPosRandomize = new FloatSlider(this,"g pos r",-1,-1,84,15,&mGranulator.mPosRandomizeMs,0,200);
   mGranSpeedRandomize = new FloatSlider(this,"g speed r",-1,-1,84,15,&mGranulator.mSpeedRandomize,0,.3f,2);
   mGranOctaveCheckbox = new Checkbox(this,"g oct",-1,-1,&mGranulator.mOctaves);
   mBeatwheelCheckbox = new Checkbox(this,"beatwheel on",HIDDEN_UICONTROL,HIDDEN_UICONTROL,&mBeatwheel);
   mBeatwheelPosRightSlider = new FloatSlider(this,"beatwheel pos right",HIDDEN_UICONTROL,HIDDEN_UICONTROL,1,1,&mBeatwheelPosRight,0,1);
   mBeatwheelDepthRightSlider = new FloatSlider(this,"beatwheel depth right",HIDDEN_UICONTROL,HIDDEN_UICONTROL,1,1,&mBeatwheelDepthRight,0,1);
   mBeatwheelPosLeftSlider = new FloatSlider(this,"beatwheel pos left",HIDDEN_UICONTROL,HIDDEN_UICONTROL,1,1,&mBeatwheelPosLeft,0,1);
   mBeatwheelDepthLeftSlider = new FloatSlider(this,"beatwheel depth left",HIDDEN_UICONTROL,HIDDEN_UICONTROL,1,1,&mBeatwheelDepthLeft,0,1);
   mBeatwheelSingleMeasureCheckbox = new Checkbox(this,"beatwheel single measure",HIDDEN_UICONTROL,HIDDEN_UICONTROL,&mBeatwheelSingleMeasure);
   mPitchShiftSlider = new FloatSlider(this,"pitch",-1,-1,130,15,&mPitchShift,.5f,2);
   mKeepPitchCheckbox = new Checkbox(this,"auto",-1,-1,&mKeepPitch);
   
   mNumBarsSelector->AddLabel(" 1 ",1);
   mNumBarsSelector->AddLabel(" 2 ",2);
   mNumBarsSelector->AddLabel(" 3 ",3);
   mNumBarsSelector->AddLabel(" 4 ",4);
   mNumBarsSelector->AddLabel(" 6 ",6);
   mNumBarsSelector->AddLabel(" 8 ",8);
   mNumBarsSelector->AddLabel("12 ",12);
   
   mFourTetSlicesDropdown->AddLabel(" 1", 1);
   mFourTetSlicesDropdown->AddLabel(" 2", 2);
   mFourTetSlicesDropdown->AddLabel(" 4", 4);
   mFourTetSlicesDropdown->AddLabel(" 8", 8);
   mFourTetSlicesDropdown->AddLabel("16", 16);
   
   mBeatwheelPosLeftSlider->SetClamped(false);
   mBeatwheelPosRightSlider->SetClamped(false);
   
   mGranPosRandomize->SetMode(FloatSlider::kSquare);
   mGranLengthMs->SetMode(FloatSlider::kSquare);
   
   mDecaySlider->SetMode(FloatSlider::kSquare);
   
   mClearButton->PositionTo(mNumBarsSelector, kAnchor_Right);
   mVolumeBakeButton->PositionTo(mVolSlider, kAnchor_Right);
   mMergeButton->PositionTo(mVolumeBakeButton, kAnchor_Right);
   mDecaySlider->PositionTo(mNumBarsSelector, kAnchor_Below);
   mShowGranularCheckbox->PositionTo(mDecaySlider, kAnchor_Right);
   mFourTetSlicesDropdown->PositionTo(mFourTetSlider, kAnchor_Right);
   mMuteCheckbox->PositionTo(mFourTetSlider, kAnchor_Below);
   mSaveButton->PositionTo(mMuteCheckbox, kAnchor_Right);
   mUndoButton->PositionTo(mSaveButton, kAnchor_Right);
   mPitchShiftSlider->PositionTo(mVolSlider, kAnchor_Below);
   mKeepPitchCheckbox->PositionTo(mPitchShiftSlider, kAnchor_Right);
   mLoopPosOffsetSlider->PositionTo(mPitchShiftSlider, kAnchor_Below);
   mResetOffsetButton->PositionTo(mLoopPosOffsetSlider, kAnchor_Right);
   mWriteOffsetButton->PositionTo(mResetOffsetButton, kAnchor_Right);
   mScratchSpeedSlider->PositionTo(mLoopPosOffsetSlider, kAnchor_Below);
   mAllowScratchCheckbox->PositionTo(mScratchSpeedSlider, kAnchor_Right);
   
   mGranOctaveCheckbox->PositionTo(mGranularCheckbox, kAnchor_Right);
   mGranLengthMs->PositionTo(mGranOctaveCheckbox, kAnchor_Right);
   mGranOverlap->PositionTo(mGranularCheckbox, kAnchor_Below);
   mGranPosRandomize->PositionTo(mGranOverlap, kAnchor_Right);
   mGranSpeed->PositionTo(mGranOverlap, kAnchor_Below);
   mGranSpeedRandomize->PositionTo(mGranSpeed, kAnchor_Right);
   mPosSlider->PositionTo(mGranSpeed, kAnchor_Below);
   mPausePosCheckbox->PositionTo(mPosSlider, kAnchor_Right);
}

Looper::~Looper()
{
   delete mBuffer;
   delete mUndoBuffer;
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
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
   {
      SetTarget(dynamic_cast<IDrawableModule*>(recorder->GetTarget()));
      mRecordBuffer = recorder->GetRecordBuffer();
      GetBuffer()->SetNumActiveChannels(mRecordBuffer->NumChannels());
   }
   else
   {
      mRecordBuffer = nullptr;
   }
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
}

void Looper::Process(double time)
{
   PROFILER(Looper);

   if (!mEnabled || GetTarget() == nullptr)
      return;

   ComputeSliders(0);
   GetBuffer()->SetNumActiveChannels(MAX(GetBuffer()->NumActiveChannels(), mBuffer->NumActiveChannels()));
   SyncBuffers();
   mBuffer->SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mWorkBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mUndoBuffer->SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   if (mWantBakeVolume)
      BakeVolume();
   if (mWantShiftDownbeat)
      DoShiftDownbeat();

   int bufferSize = GetBuffer()->BufferSize();

   float oldLoopPos = mLoopPos;
   int sampsPerBar = mLoopLength / mNumBars;
   if (!mPausePos)
      mLoopPos = sampsPerBar * ((TheTransport->GetMeasure(time) % mNumBars) + TheTransport->GetMeasurePos(time));
   
   if (oldLoopPos > mLoopLength - bufferSize * mSpeed - 1 && mLoopPos < oldLoopPos)
   {
      ++mLoopCount;
      /*if (mLoopCount > 6 && mMute == false && mDecay)
         mVol *= ofMap(TheTransport->GetTempo(), 80.0f, 160.0f, .95f, .99f, true);*/
      if (mMute == false)
         mVol *= 1 - mDecay;
      
      if (mCaptureQueued && !mWriteInput)
      {
         mWriteInputRamp.Start(1,10);
         mWriteInput = true;
      }
      else if (mWriteInput && mCaptureQueued)
      {
         mCaptureQueued = false;
         mWriteInputRamp.Start(0,10);
         mWriteInput = false;
      }
   }

   if (mSpeed == 1)
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
      for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
         mJumpBlender[ch].CaptureForJump(mLoopPos, mBuffer->GetChannel(ch), mLoopLength, 0);
      mBuffer = mQueuedNewBuffer;
      mBufferMutex.unlock();
      mQueuedNewBuffer = nullptr;
   }
   
   if (mKeepPitch)
      mPitchShift = 1/mSpeed;
   int latencyOffset = 0;
   if (mPitchShift != 1)
      latencyOffset = mPitchShifter[0]->GetLatency();

   for (int i=0; i<bufferSize; ++i)
   {
      float smooth = .001f;
      mSmoothedVol = mSmoothedVol * (1-smooth) + mVol * smooth;
      float volSq = mSmoothedVol * mSmoothedVol;
      
      mLoopPosOffsetSlider->Compute(i);
      
      if (mAllowScratch)
         ProcessScratch();
      
      if (mFourTet > 0)
         ProcessFourTet(time, i);
      
      if (mBeatwheel)
         ProcessBeatwheel(time, i);
      
      float offset = mLoopPos+i*mSpeed+mLoopPosOffset+latencyOffset;
      float output[ChannelBuffer::kMaxNumChannels];
      ::Clear(output, ChannelBuffer::kMaxNumChannels);
      
      if (mGranular)
         ProcessGranular(time, offset, output);
      
      for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
      {
         if (!mGranular)
         {
            output[ch] = GetInterpolatedSample(offset, mBuffer->GetChannel(ch), mLoopLength);
            output[ch] = mJumpBlender[ch].Process(output[ch],i);
         }
         
         if (mFourTet > 0 && mFourTet < 1)   //fourtet wet/dry
         {
            output[ch] *= mFourTet;
            float normalOffset = mLoopPos+i*mSpeed;
            output[ch] += GetInterpolatedSample(normalOffset, mBuffer->GetChannel(ch), mLoopLength) * (1-mFourTet);
         }
         
         //write one sample the past so we don't end up feeding into the next output
         float writeAmount = mWriteInputRamp.Value(time);
         if (writeAmount > 0)
            WriteInterpolatedSample(offset-1, mBuffer->GetChannel(ch), mLoopLength, mLastInputSample[ch] * writeAmount);
         mLastInputSample[ch] = GetBuffer()->GetChannel(ch)[i];

         output[ch] *= volSq;
         
         mWorkBuffer.GetChannel(ch)[i] = output[ch] * mMuteRamp.Value(time);

         GetVizBuffer()->Write(mWorkBuffer.GetChannel(ch)[i] + GetBuffer()->GetChannel(ch)[i], ch);
      }
      
      time += gInvSampleRateMs;
   }
   
   if (mPitchShift != 1)
   {
      for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
      {
         mPitchShifter[ch]->SetRatio(mPitchShift);
         mPitchShifter[ch]->Process(mWorkBuffer.GetChannel(ch), bufferSize);
      }
   }
   
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
      Add(GetTarget()->GetBuffer()->GetChannel(ch), mWorkBuffer.GetChannel(ch), bufferSize);
   }
   
   GetBuffer()->Reset();
   
   if (mCommitBuffer && !mClearCommitBuffer && !mWantRewrite)
      DoCommit();
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
         mRewriter->Go();
   }
}

void Looper::DoCommit()
{
   PROFILER(LooperDoCommit);
   
   if (mRecorder == nullptr)
      return;
   
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
      mMuteRamp.Start(mMute ? 0 : 1, 1);
   }

   {
      PROFILER(LooperDoCommit_commit);
      int commitSamplesBack = mRecorder->GetCommitDelay() * TheTransport->MsPerBar() / gInvSampleRateMs;
      int commitLength = mLoopLength+LOOPER_COMMIT_FADE_SAMPLES;
      for (int i=0; i<commitLength; ++i)
      {
         int idx = i - LOOPER_COMMIT_FADE_SAMPLES;
         int pos = int(mLoopPos+(idx*mSpeed)+(mLoopLength-commitSamplesBack)) % mLoopLength;
         float fade = 1;
         if (idx < 0)
            fade = float(LOOPER_COMMIT_FADE_SAMPLES + idx) / LOOPER_COMMIT_FADE_SAMPLES;
         if (idx >= mLoopLength-LOOPER_COMMIT_FADE_SAMPLES)
            fade = 1 - (float(idx-(mLoopLength-LOOPER_COMMIT_FADE_SAMPLES)) / LOOPER_COMMIT_FADE_SAMPLES);
         
         for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
         {
            mBuffer->GetChannel(ch)[pos] += mCommitBuffer->GetSample(ofClamp(commitLength - i + commitSamplesBack,0,MAX_BUFFER_SIZE-1), ch) * fade;
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
      return mRecorder->NumBars();
   return NumBars();
}

void Looper::ProcessScratch()
{
   mLoopPosOffset = mLoopPosOffset - mSpeed + mScratchSpeed;
   FloatWrap(mLoopPosOffset, mLoopLength);
}

void Looper::ProcessFourTet(double time, int sampleIdx)
{
   float measurePos = TheTransport->GetMeasurePos(time) + sampleIdx/(TheTransport->MsPerBar() / gInvSampleRateMs);
   measurePos += TheTransport->GetMeasure(time) % mNumBars;
   measurePos /= mNumBars;
   int numSlices = mFourTetSlices * 2 * mNumBars;
   measurePos *= numSlices;
   int slice = (int)measurePos;
   float sliceProgress = measurePos - slice;
   if (slice % 2 == 0)
      mLoopPosOffset = (sliceProgress + slice/2) * (mLoopLength/numSlices * 2);
   else
      mLoopPosOffset = (1 - sliceProgress + slice/2) * (mLoopLength/numSlices * 2);
   
   //offset regular movement
   mLoopPosOffset -= mLoopPos+sampleIdx*mSpeed;
   
   FloatWrap(mLoopPosOffset, mLoopLength);
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
   int lastSlice = GetMeasureSliceIndex(time, sampleIdx-1, slicesPerBar);
   int slice = GetMeasureSliceIndex(time, sampleIdx, slicesPerBar);
   int numSlices = slicesPerBar*mNumBars;
   int loopLength = mLoopLength;
   if (mBeatwheelSingleMeasure)
   {
      numSlices = slicesPerBar;
      loopLength = mLoopLength / mNumBars;
   }
   
   if (lastSlice != slice) //on new slices
   {
      for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
         mJumpBlender[ch].CaptureForJump(int(GetActualLoopPos(sampleIdx))%loopLength, mBuffer->GetChannel(ch), loopLength, sampleIdx);
      
      if (noneHeld)
      {
         mLoopPosOffset = 0;
      }
      else
      {
         if (bothHeld) //we should pingpong
            mBeatwheelControlFlip = !mBeatwheelControlFlip;
         
         int playSlice = int(clockPos * numSlices);
         
         mLoopPosOffset = playSlice * (loopLength/numSlices);
         
         //offset regular movement
         mLoopPosOffset -= mLoopPos+sampleIdx*mSpeed;
         
         FloatWrap(mLoopPosOffset, mLoopLength);
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
   float pos = mLoopPos+mLoopPosOffset + samplesIn;
   FloatWrap(pos,mLoopLength);
   return pos;
}

int Looper::GetMeasureSliceIndex(double time, int sampleIdx, int slicesPerBar)
{
   float measurePos = TheTransport->GetMeasurePos(time) + sampleIdx/(TheTransport->MsPerBar() / gInvSampleRateMs);
   measurePos += TheTransport->GetMeasure(time) % mNumBars;
   measurePos /= mNumBars;
   int numSlices = slicesPerBar * mNumBars;
   measurePos *= numSlices;
   int slice = (int)measurePos;
   return slice;
}

void Looper::ProcessGranular(double time, float bufferOffset, float* output)
{
   mGranulator.Process(time, mBuffer, mLoopLength, bufferOffset, output);
}

void Looper::ResampleForNewSpeed()
{
   int oldLoopLength = mLoopLength;
   SetLoopLength(MIN(int(abs(mLoopLength/mSpeed)), MAX_BUFFER_SIZE-1));
   mLoopPos /= mSpeed;
   while (mLoopPos < 0)
      mLoopPos += mLoopLength;
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      float* oldBuffer = new float[oldLoopLength];
      BufferCopy(oldBuffer, mBuffer->GetChannel(ch), oldLoopLength);
      for (int i=0; i<mLoopLength; ++i)
      {
         float offset = i*mSpeed;
         mBuffer->GetChannel(ch)[i] = GetInterpolatedSample(offset, oldBuffer, oldLoopLength);
      }
      delete[] oldBuffer;
   }
   
   if (mKeepPitch)
   {
      mKeepPitch = false;
      mPitchShift = 1/mSpeed;
   }
   
   mSpeed = 1;
}

void Looper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushMatrix();
   
   ofTranslate(BUFFER_X,BUFFER_Y);
   ofPushStyle();
   ofFill();
   float age = ofClamp((gTime-mLastCommitTime)/240000,0,1);
   ofSetColor(100,0,0,gModuleDrawAlpha*.2f);
   ofRect(0,0,BUFFER_W,BUFFER_H*ofClamp(age*2,0,1));
   if (age > .5f)
   {
      ofSetColor(200,0,0,gModuleDrawAlpha*.2f);
      ofRect(0,0,BUFFER_W,BUFFER_H*((age - .5f)*2));
   }
   //ofSetColor(255,255,255,gModuleDrawAlpha);
   //DrawTextNormal(ofToString(mLoopCount),2,12);
   ofPopStyle();
   
   assert(mLoopLength > 0);
   
   float displayPos = GetActualLoopPos(0);
   mBufferMutex.lock();
   DrawAudioBuffer(BUFFER_W, BUFFER_H, mBuffer, 0, mLoopLength, displayPos, mVol);
   mBufferMutex.unlock();
   ofSetColor(255,255,0,gModuleDrawAlpha);
   for (int i=1; i<mNumBars; ++i)
   {
      float x = BUFFER_W/mNumBars * i;
      ofLine(x,BUFFER_H/2-5,x,BUFFER_H/2+5);
   }
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   ofPopMatrix();

   mClearButton->Draw();
   mNumBarsSelector->Draw();
   mVolSlider->Draw();
   mVolumeBakeButton->Draw();
   mDecaySlider->Draw();
   mDoubleSpeedButton->Draw();
   mHalveSpeedButton->Draw();
   mUndoButton->Draw();
   mLoopPosOffsetSlider->Draw();
   mResetOffsetButton->Draw();
   mWriteOffsetButton->Draw();
   mScratchSpeedSlider->Draw();
   mAllowScratchCheckbox->Draw();
   mFourTetSlider->Draw();
   mFourTetSlicesDropdown->Draw();
   mShowGranularCheckbox->Draw();
   mPitchShiftSlider->Draw();
   mKeepPitchCheckbox->Draw();
   mWriteInputCheckbox->Draw();
   mQueueCaptureButton->Draw();
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
   
   if (mRecorder && mRecorder->GetMergeSource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,0,0);
      ofRect(165,61,26,15);
      ofPopStyle();
   }
   mMergeButton->Draw();

   if (mRecorder && mRecorder->GetSwapSource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(0,0,255);
      ofRect(120,82,35,15);
      ofPopStyle();
   }
   mSwapButton->Draw();
   
   if (mRecorder && mRecorder->GetCopySource() == this)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(0,0,255);
      ofRect(120,65,35,15);
      ofPopStyle();
   }
   mCopyButton->Draw();

   mSaveButton->Draw();
   mMuteCheckbox->Draw();
   mCommitButton->Draw();
   
   if (mShowGranular)
   {
      ofPushStyle();
      ofSetColor(100,100,200,100);
      ofFill();
      ofRect(0,165,GetRect().width,73);
      ofPopStyle();
      mGranularCheckbox->Draw();
      mGranOverlap->Draw();
      mGranSpeed->Draw();
      mGranLengthMs->Draw();
      mPosSlider->Draw();
      mPausePosCheckbox->Draw();
      mGranPosRandomize->Draw();
      mGranSpeedRandomize->Draw();
      mGranOctaveCheckbox->Draw();
      if (mGranular)
         mGranulator.Draw(4,35,155,32,0,mLoopLength);
   }
   
   if (mBeatwheel)
   {
      DrawBeatwheel();
   }
}

void Looper::DrawBeatwheel()
{
   ofPushMatrix();
   ofPushStyle();
   
   float size = 197;
   
   ofTranslate(0,-size);
   
   ofFill();
   ofSetColor(50,50,50,gModuleDrawAlpha*.65f);
   ofRect(0,0,size,size);
   
   float centerX = size/2;
   float centerY = size/2;
   float innerRad = size * .2f;
   float outerRad = size * .45f;
   float waveformCenter = (innerRad+outerRad)/2;
   float waveformHeight = (outerRad-innerRad)/2;
   
   ofSetCircleResolution(100);
   ofSetColor(100,100,100,gModuleDrawAlpha*.8f);
   ofCircle(size/2,size/2,outerRad);
   ofSetColor(50,50,50,gModuleDrawAlpha*.5f);
   ofCircle(size/2,size/2,innerRad);
   
   ofSetLineWidth(1);
   ofNoFill();
   
   int depthLevel = GetBeatwheelDepthLevel();
   int slicesPerBar = TheTransport->GetTimeSigTop() * (1 << depthLevel);
   int numSlices = slicesPerBar*mNumBars;
   int loopLength = mLoopLength;
   if (mBeatwheelSingleMeasure)
   {
      numSlices = slicesPerBar;
      loopLength = mLoopLength / mNumBars;
   }
   
   ofSetColor(0,0,0,gModuleDrawAlpha);
   float subdivisions = 600;
   int samplesPerPixel = loopLength / subdivisions;
   
   for (int i=0; i<subdivisions; i++)
   {
      float radians = (i*TWO_PI)/subdivisions;
      //ofSetColor(200,200,200,gModuleDrawAlpha);
      float sinR = sin(radians);
      float cosR = cos(radians);
      //ofLine(centerX+sinR*innerRad, centerY-cosR*innerRad, centerX+sinR*outerRad, centerY-cosR*outerRad);
      float mag = 0;
      int position = i*samplesPerPixel;
      //rms
      int j;
      for (j=0; j<samplesPerPixel && position+j < loopLength-1; ++j)
      {
         for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
            mag += mBuffer->GetChannel(ch)[position+j];
      }
      mag /= j;
      mag = sqrtf(mag);
      mag = sqrtf(mag);
      mag = MIN(1.0f,mag);
      float inner = (waveformCenter-mag*waveformHeight);
      float outer = (waveformCenter+mag*waveformHeight);
      //ofSetColor(0,0,0,gModuleDrawAlpha);
      ofLine(centerX+sinR*inner, centerY-cosR*inner, centerX+sinR*outer, centerY-cosR*outer);
   }
   
   ofSetLineWidth(3);
   ofSetColor(0,255,0,gModuleDrawAlpha);
   float displayPos = GetActualLoopPos(0);
   float position = displayPos / loopLength;
   float radians = position*TWO_PI;
   float sinR = sin(radians);
   float cosR = cos(radians);
   ofLine(centerX+sinR*innerRad, centerY-cosR*innerRad, centerX+sinR*outerRad, centerY-cosR*outerRad);
   
   ofSetLineWidth(4);
   ofSetColor(255,0,0,gModuleDrawAlpha);
   if (mBeatwheelDepthRight > 0 && mBeatwheelPosRight >= 0)
   {
      float radians = mBeatwheelPosRight*TWO_PI;
      float sinR = sin(radians);
      float cosR = cos(radians);
      ofLine(centerX+sinR*outerRad*.9f, centerY-cosR*outerRad*.9f, centerX+sinR*outerRad, centerY-cosR*outerRad);
   }
   if (mBeatwheelDepthLeft > 0 && mBeatwheelPosLeft >= 0)
   {
      float radians = mBeatwheelPosLeft*TWO_PI;
      float sinR = sin(radians);
      float cosR = cos(radians);
      ofLine(centerX+sinR*outerRad*.9f, centerY-cosR*outerRad*.9f, centerX+sinR*outerRad, centerY-cosR*outerRad);
   }

   if (depthLevel > 0)
   {
      ofSetLineWidth(1);
      ofSetColor(150,150,150,gModuleDrawAlpha);
      for (int i=0; i<numSlices; ++i)
      {
         float radians = (i*TWO_PI)/numSlices;
         float sinR = sin(radians);
         float cosR = cos(radians);
         ofLine(centerX+sinR*waveformCenter, centerY-cosR*waveformCenter, centerX+sinR*outerRad, centerY-cosR*outerRad);
      }
   }
   
   ofPopStyle();
   ofPopMatrix();
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
   int oldNumBars = mNumBars;
   mNumBars = numBars;
   UpdateNumBars(oldNumBars);
}

void Looper::BakeVolume()
{
   mUndoBuffer->CopyFrom(mBuffer);
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
      Mult(mBuffer->GetChannel(ch), mVol*mVol, mLoopLength);
   mVol = 1;
   mSmoothedVol = 1;
   mWantBakeVolume = false;
}

void Looper::UpdateNumBars(int oldNumBars)
{
   assert(mNumBars > 0);
   int sampsPerBar = abs(int(TheTransport->MsPerBar() / 1000 * gSampleRate));
   SetLoopLength(MIN(sampsPerBar * mNumBars, MAX_BUFFER_SIZE-1));
   while (mLoopPos > sampsPerBar)
      mLoopPos -= sampsPerBar;
   mLoopPos += sampsPerBar * (TheTransport->GetMeasure(gTime) % mNumBars);
   if (oldNumBars < mNumBars)
   {
      int oldLoopLength = abs(int(TheTransport->MsPerBar() * oldNumBars / 1000 * gSampleRate));
      oldLoopLength = MIN(oldLoopLength, MAX_BUFFER_SIZE-1);
      for (int i=1; i<mNumBars/oldNumBars; ++i)
      {
         for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
            BufferCopy(mBuffer->GetChannel(ch)+oldLoopLength*i, mBuffer->GetChannel(ch), oldLoopLength);
      }
   }
}

void Looper::SetLoopLength(int length)
{
   assert(length > 0);
   mLoopLength = length;
   mLoopPosOffsetSlider->SetExtents(0, length);
   mPosSlider->SetExtents(0, length);
}

void Looper::MergeIn(Looper* otherLooper)
{
   int newNumBars = MAX(mNumBars, otherLooper->mNumBars);

   SetNumBars(newNumBars);

   otherLooper->SetNumBars(newNumBars);

   if (mVol > 0.01f)
   {
      for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
      {
         Mult(otherLooper->mBuffer->GetChannel(ch), (otherLooper->mVol*otherLooper->mVol) / (mVol*mVol), mLoopLength); //keep other looper at same apparent volume
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
   mBuffer->CopyFrom(sourceLooper->mBuffer);
   SetLoopLength(sourceLooper->mLoopLength);
   mNumBars = sourceLooper->mNumBars;
}

void Looper::Commit(RollingBuffer* commitBuffer /* = nullptr */)
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
   mGranular = false;
   mShowGranular = false;
   mPausePos = false;
   mPitchShift = 1;
   
   if (commitBuffer) //specified buffer
   {
      mCommitBuffer = commitBuffer;
      mReplaceOnCommit = true;
   }
   else  //default buffer
   {
      mCommitBuffer = mRecordBuffer;
      mReplaceOnCommit = false;
   }
}

void Looper::FilesDropped(vector<string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x,y,&sample);
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
   for (int i=0; i<mLoopLength; ++i)
   {
      float offset = i*lengthRatio;
      for (int ch=0; ch<sample->NumChannels(); ++ch)
         mBuffer->GetChannel(ch)[i] = GetInterpolatedSample(offset, sample->Data()->GetChannel(ch), numSamples);
   }
}

void Looper::GetModuleDimensions(float& width, float& height)
{
   if (mShowGranular)
   {
      width = BUFFER_X*2+BUFFER_W;
      height = 238;
   }
   else
   {
      width = BUFFER_X*2+BUFFER_W;
      height = 165;
   }
}

void Looper::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   if (x >= BUFFER_X + BUFFER_W / 3 && x < BUFFER_X + (BUFFER_W * 2) / 3 &&
       y >= BUFFER_Y + BUFFER_H / 3 && y < BUFFER_Y + (BUFFER_H * 2) / 3)
   {
      ChannelBuffer grab(mLoopLength);
      grab.SetNumActiveChannels(mBuffer->NumActiveChannels());
      for (int ch=0; ch<grab.NumActiveChannels(); ++ch)
         BufferCopy(grab.GetChannel(ch), mBuffer->GetChannel(ch), mLoopLength);
      TheSynth->GrabSample(&grab, false, mNumBars);
   }
}

void Looper::ButtonClicked(ClickButton* button)
{
   if (button == mClearButton)
   {
      mUndoBuffer->CopyFrom(mBuffer);
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
      Sample::WriteDataToFile(ofGetTimestampString("loops/loop_%Y-%m-%d_%H-%M.wav").c_str(), mBuffer, mLoopLength);
   }
   if (button == mCommitButton && mRecorder)
      mRecorder->Commit(this);
   if (button == mDoubleSpeedButton)
   {
      if (mSpeed == 1)
      {
         HalveNumBars();
         mSpeed = 2;
         ResampleForNewSpeed();
      }
   }
   if (button == mHalveSpeedButton)
   {
      if (mSpeed == 1 && mLoopLength < MAX_BUFFER_SIZE/2)
      {
         DoubleNumBars();
         mSpeed = .5f;
         ResampleForNewSpeed();
      }
   }
   if (button == mUndoButton)
      mWantUndo = true;
   if (button == mResetOffsetButton)
   {
      mLoopPosOffset = 0;
      mLoopPosOffsetSlider->DisableLFO();
   }
   if (button == mWriteOffsetButton)
      mWantShiftOffset = true;
   if (button == mQueueCaptureButton)
   {
      mCaptureQueued = true;
      mLastCommitTime = gTime;
   }
}

void Looper::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mScratchSpeedSlider)
   {
      if (!mAllowScratch)
         mScratchSpeed = 1;
   }
   if (slider == mFourTetSlider)
   {
      if (mFourTet == 0)
         mLoopPosOffset = 0;
   }
   if (slider == mVolSlider)
   {
      if (mVol > oldVal)
      {
         mLoopCount = 0;   //stop fading for a few loops
      }
   }
}

void Looper::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void Looper::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mNumBarsSelector)
      UpdateNumBars(oldVal);
}

void Looper::CheckboxUpdated(Checkbox* checkbox)
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
      mMuteRamp.Start(mMute ? 0 : 1, 1);
   }
   if (checkbox == mShowGranularCheckbox)
   {
      mGranulator.Reset();
      
      if (mShowGranular == false)
      {
         mGranular = false;
         mPausePos = false;
      }
   }
   if (checkbox == mGranularCheckbox)
   {
      if (mGranular == false)
         mPausePos = false;
   }
   if (checkbox == mPausePosCheckbox)
   {
      if (mPausePos)
      {
         mShowGranular = true;
         mGranular = true;
      }
   }
   if (checkbox == mWriteInputCheckbox)
   {
      if (mWriteInput)
         mWriteInputRamp.Start(1,10);
      else
         mWriteInputRamp.Start(0,10);
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
      if (bufferSize < MAX_BUFFER_SIZE / 2)  //if we can fit it
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
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      float* newBuffer = new float[MAX_BUFFER_SIZE];
      BufferCopy(newBuffer, mBuffer->GetChannel(ch)+measureSize, mLoopLength-measureSize);
      BufferCopy(newBuffer+mLoopLength-measureSize, mBuffer->GetChannel(ch), measureSize);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantShiftMeasure = false;
}

void Looper::DoHalfShift()
{
   int halfMeasureSize = int(TheTransport->MsPerBar() * gSampleRate / 1000 / 2);
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      float* newBuffer = new float[MAX_BUFFER_SIZE];
      BufferCopy(newBuffer, mBuffer->GetChannel(ch)+halfMeasureSize, mLoopLength-halfMeasureSize);
      BufferCopy(newBuffer+mLoopLength-halfMeasureSize, mBuffer->GetChannel(ch), halfMeasureSize);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantHalfShift = false;
}

void Looper::DoShiftDownbeat()
{
   float* newBuffer = new float[MAX_BUFFER_SIZE];
   int shift = int(mLoopPos);
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      BufferCopy(newBuffer, mBuffer->GetChannel(ch)+shift, mLoopLength-shift);
      BufferCopy(newBuffer+mLoopLength-shift, mBuffer->GetChannel(ch), shift);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantShiftDownbeat = false;
}

void Looper::DoShiftOffset()
{
   float* newBuffer = new float[MAX_BUFFER_SIZE];
   int shift = int(mLoopPosOffset);
   for (int ch=0; ch<mBuffer->NumActiveChannels(); ++ch)
   {
      BufferCopy(newBuffer, mBuffer->GetChannel(ch)+shift, mLoopLength-shift);
      BufferCopy(newBuffer+mLoopLength-shift, mBuffer->GetChannel(ch), shift);
      mBufferMutex.lock();
      mBuffer->SetChannelPointer(newBuffer, ch, true);
      mBufferMutex.unlock();
   }
   mWantShiftOffset = false;
   mLoopPosOffset = 0;
}

void Looper::Rewrite()
{
   mWantRewrite = true;
}

void Looper::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   //jump around in loop
   if (velocity > 0)
   {
      int chop = pitch - 36;
      if (chop == 3)
      {
         mLoopPosOffset = 0;
         mLoopPosOffsetSlider->DisableLFO();
      }
      if (chop >= 9 && chop < 16 && chop % 2 == 1)
      {
         mChopMeasure = (chop/2 - 4) % mNumBars;
         float sampsPerBar = TheTransport->MsPerBar() / 1000.0f * gSampleRate;
         mLoopPosOffset = -mLoopPos + mChopMeasure * sampsPerBar;
         mLoopPosOffsetSlider->DisableLFO();
      }
      if (chop >= 0 && chop < 16 && chop % 2 == 0)
      {
         int slice = chop/2;
         float measurePos = slice / 8.0f;
         float sampsPerBar = TheTransport->MsPerBar() / 1000.0f * gSampleRate;
         mLoopPosOffset = -mLoopPos + (mChopMeasure + measurePos) * sampsPerBar;
         if (mLoopPosOffset < 0)
            mLoopPosOffset += mLoopLength;
         mLoopPosOffsetSlider->DisableLFO();
      }
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

namespace
{
   const int kSaveStateRev = 0;
}

void Looper::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mLoopLength;
   mBuffer->Save(out, mLoopLength);
}

void Looper::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mLoopLength;
   int readLength;
   mBuffer->Load(in, readLength, false);
   assert(mLoopLength == readLength);
}


