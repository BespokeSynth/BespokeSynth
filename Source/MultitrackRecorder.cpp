//
//  MultitrackRecorder.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/13/14.
//
//

#include "MultitrackRecorder.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Scale.h"
#include "Sample.h"
#include "ArrangementMaster.h"

MultitrackRecorder* TheMultitrackRecorder = nullptr;

MultitrackRecorder::MultitrackRecorder()
: mRecordingLength(RECORD_CHUNK_SIZE)
, mRecording(false)
, mRecordCheckbox(nullptr)
, mPlayCheckbox(nullptr)
, mAddTrackButton(nullptr)
, mResetPlayheadButton(nullptr)
, mFixLengthsButton(nullptr)
, mBufferWidth(800)
, mBufferHeight(80)
, mActiveStructureIdx(-1)
, mRecordIdx(0)
, mMaxRecordedLength(-1)
, mNumMeasures(0)
, mSelectedMeasureStart(-1)
, mSelectedMeasureEnd(-1)
, mMergeBufferIdx(-1)
, mUndoBuffer(nullptr)
, mUndoRecordButton(nullptr)
{
   TheMultitrackRecorder = this;
   
   mMeasurePos = new float[mRecordingLength];
   bzero(mMeasurePos, sizeof(float)*mRecordingLength);
   AddRecordBuffer();
   
   mUndoBuffer = new RecordBuffer(mRecordingLength);
   
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
      AddChild(&mClipArranger[i]);
}

void MultitrackRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRecordCheckbox = new Checkbox(this,"rec",100,2,&mRecording);
   mPlayCheckbox = new Checkbox(this,"play",140,2,&ArrangementMaster::mPlay);
   mAddTrackButton = new ClickButton(this,"add",200,2);
   mResetPlayheadButton = new ClickButton(this,"reset",230,2);
   mFixLengthsButton = new ClickButton(this,"fix lengths",270,2);
   mUndoRecordButton = new ClickButton(this,"undo rec",450,2);
   
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
      mClipArranger[i].CreateUIControls();
}

MultitrackRecorder::~MultitrackRecorder()
{
   TheMultitrackRecorder = nullptr;
   
   for (int i=0; i<mRecordBuffers.size(); ++i)
      delete mRecordBuffers[i];
   
   delete mUndoBuffer;
}

void MultitrackRecorder::Poll()
{
   int reallocDist = gSampleRate; //1 second from the end
   ArrangementMaster::mSampleLength = mRecordingLength;
   
   float cW, cH;
   mClipArranger[0].GetDimensions(cW, cH);
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
      mClipArranger[i].SetPosition(0, 25 + mBufferHeight * mRecordBuffers.size() + i*cH);
   
   if (mRecording &&
       ArrangementMaster::mPlayhead > mRecordingLength - reallocDist)  //we're a second from the end
   {
      int newChunk = RECORD_CHUNK_SIZE;
      int newLength = mRecordingLength + newChunk;
      
      float* newLeft = new float[newLength];
      float* newRight = new float[newLength];
      float* newMeasurePos = new float[newLength];
      float* oldLeft = mRecordBuffers[mRecordIdx]->mLeft;
      float* oldRight = mRecordBuffers[mRecordIdx]->mRight;
      float* oldMeasurePos = mMeasurePos;
      
      BufferCopy(newLeft, oldLeft, mRecordingLength-reallocDist);
      BufferCopy(newRight, oldRight, mRecordingLength-reallocDist);
      BufferCopy(newMeasurePos, mMeasurePos, mRecordingLength-reallocDist);
      Clear(newLeft+mRecordingLength, newChunk);
      Clear(newRight+mRecordingLength, newChunk);
      Clear(newMeasurePos+mRecordingLength, newChunk);
      
      mMutex.Lock("main thread");
      BufferCopy(newLeft+(mRecordingLength-reallocDist), oldLeft+(mRecordingLength-reallocDist), reallocDist);
      BufferCopy(newRight+(mRecordingLength-reallocDist), oldRight+(mRecordingLength-reallocDist), reallocDist);
      BufferCopy(newMeasurePos+(mRecordingLength-reallocDist), mMeasurePos+(mRecordingLength-reallocDist), reallocDist);
      mRecordBuffers[mRecordIdx]->mLeft = newLeft;
      mRecordBuffers[mRecordIdx]->mRight = newRight;
      mRecordBuffers[mRecordIdx]->mLength = newLength;
      mMeasurePos = newMeasurePos;
      mRecordingLength = newLength;
      mMutex.Unlock();
      
      delete[] oldLeft;
      delete[] oldRight;
      delete[] oldMeasurePos;
   }
}

void MultitrackRecorder::Process(double time, float* left, float* right, int bufferSize)
{
   PROFILER(MultitrackRecorder);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   mMutex.Lock("audio thread");
   
   if (mRecording || ArrangementMaster::mPlay)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         int recordIdx = GetRecordIdx();
         
         if (IsRecordingStructure())
            RecordStructure(i);
         else
            ApplyStructure();
         
         if (mRecording)
         {
            mRecordBuffers[recordIdx]->mLeft[ArrangementMaster::mPlayhead] = left[i];
            mRecordBuffers[recordIdx]->mRight[ArrangementMaster::mPlayhead] = right[i];
         }
         
         for (int j=0; j<mRecordBuffers.size(); ++j)
         {
            if (j != recordIdx &&
                mRecordBuffers[j]->mControls.mMute == false &&
                ArrangementMaster::mPlayhead < mRecordBuffers[j]->mLength)
            {
               float volSq = mRecordBuffers[j]->mControls.mVol * mRecordBuffers[j]->mControls.mVol;
               left[i] += mRecordBuffers[j]->mLeft[ArrangementMaster::mPlayhead] * volSq;
               right[i] += mRecordBuffers[j]->mRight[ArrangementMaster::mPlayhead] * volSq;
            }
         }
         
         if (ArrangementMaster::mPlayhead < mRecordingLength - 1)
            ++ArrangementMaster::mPlayhead;
      }
   }
   
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
      mClipArranger[i].Process(time, left, right, bufferSize);
   
   mMutex.Unlock();
}

void MultitrackRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mRecordCheckbox->Draw();
   mPlayCheckbox->Draw();
   mAddTrackButton->Draw();
   mResetPlayheadButton->Draw();
   mFixLengthsButton->Draw();
   mUndoRecordButton->Draw();
   
   ofPushStyle();
   ofPushMatrix();
   ofTranslate(5,20);
   for (int i=0; i<mRecordBuffers.size(); ++i)
   {
      ofPushMatrix();
      DrawAudioBuffer(mBufferWidth * mRecordBuffers[i]->mLength/mRecordingLength,mBufferHeight*.45f,mRecordBuffers[i]->mLeft,0,mRecordBuffers[i]->mLength,ArrangementMaster::mPlayhead);
      ofTranslate(0,mBufferHeight*.47f);
      DrawAudioBuffer(mBufferWidth * mRecordBuffers[i]->mLength/mRecordingLength,mBufferHeight*.45f,mRecordBuffers[i]->mRight,0,mRecordBuffers[i]->mLength,ArrangementMaster::mPlayhead);
      ofTranslate(0,mBufferHeight*.53f);
      ofPopMatrix();
      
      if (mRecordBuffers[i]->mControls.mMute)
      {
         ofFill();
         ofSetColor(0, 0, 0, 100);
         ofRect(0,0,mBufferWidth,mBufferHeight*.92f);
      }
      
      if (i == mMergeBufferIdx)
      {
         ofFill();
         ofSetColor(255,0,0,150);
         ofRect(0,0,mBufferWidth,mBufferHeight*.92f);
      }
      
      if (i == mRecordIdx)
      {
         if (mRecording && i == mRecordIdx)
         {
            ofNoFill();
            ofSetColor(255,0,0);
            ofRect(0,0,mBufferWidth,mBufferHeight*.92f);
         }
         else
         {
            ofNoFill();
            ofSetColor(100,100,255);
            ofRect(0,0,mBufferWidth,mBufferHeight*.92f);
         }
         
         ofSetColor(255,255,0);
         for (int j=0; j<mNumMeasures; ++j)
         {
            float pos = MeasureToPos(j)*mBufferWidth;
            ofLine(pos,0,pos,mBufferHeight*.1f);
         }
         
         if (mSelectedMeasureStart != -1)
         {
            float posStart = MeasureToPos(mSelectedMeasureStart)*mBufferWidth;
            float posEnd = MeasureToPos(mSelectedMeasureEnd)*mBufferWidth;
            ofSetColor(255,255,255,100);
            ofFill();
            ofRect(posStart,0,posEnd-posStart,mBufferHeight*.92f);
         }
      }
      ofTranslate(0, mBufferHeight);
      mRecordBuffers[i]->mControls.mVolSlider->SetPosition(mBufferWidth + 10, mBufferHeight*i+20);
      mRecordBuffers[i]->mControls.mMuteCheckbox->SetPosition(mBufferWidth + 10, mBufferHeight*i+40);
   }
   ofPopMatrix();
   ofPopStyle();
   
   for (int i=0; i<mRecordBuffers.size(); ++i)
   {
      mRecordBuffers[i]->mControls.mVolSlider->Draw();
      mRecordBuffers[i]->mControls.mMuteCheckbox->Draw();
   }
   
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
      mClipArranger[i].Draw();
}

bool MultitrackRecorder::IsRecordingStructure()
{
   return mRecording && ArrangementMaster::mPlayhead > mMaxRecordedLength;
}

void MultitrackRecorder::RecordStructure(int offset)
{
   mMeasurePos[ArrangementMaster::mPlayhead] = TheTransport->GetMeasurePos(gTime + offset * gInvSampleRateMs);
   mMaxRecordedLength = MAX(ArrangementMaster::mPlayhead, mMaxRecordedLength);
   
   if (ArrangementMaster::mPlayhead == 0 || mMeasurePos[ArrangementMaster::mPlayhead-1] > mMeasurePos[ArrangementMaster::mPlayhead])
   {
      mMeasures[mNumMeasures] = ArrangementMaster::mPlayhead;
      ++mNumMeasures;
   }
   
   bool needToRecord = false;
   
   if (mStructureInfoPoints.empty())
   {
      needToRecord = true;
   }
   else
   {
      const StructureInfo& lastStructure = *mStructureInfoPoints.rbegin();
      
      if (lastStructure.mScaleRoot != TheScale->ScaleRoot() ||
          lastStructure.mScaleType != TheScale->GetType() ||
          lastStructure.mTimeSigTop != TheTransport->GetTimeSigTop() ||
          lastStructure.mTimeSigBottom != TheTransport->GetTimeSigBottom() ||
          lastStructure.mTempo != TheTransport->GetTempo() ||
          lastStructure.mSwing != TheTransport->GetSwing())
         needToRecord = true;
   }
   
   if (needToRecord)
   {
      StructureInfo structure;
      structure.mSample = ArrangementMaster::mPlayhead;
      structure.mScaleRoot = TheScale->ScaleRoot();
      structure.mScaleType = TheScale->GetType();
      structure.mTimeSigTop = TheTransport->GetTimeSigTop();
      structure.mTimeSigBottom = TheTransport->GetTimeSigBottom();
      structure.mTempo = TheTransport->GetTempo();
      structure.mSwing = TheTransport->GetSwing();
      mStructureInfoPoints.push_back(structure);
      
      mActiveStructureIdx = (int)mStructureInfoPoints.size() - 1;
   }
}

void MultitrackRecorder::ApplyStructure()
{
   if (mMeasurePos[ArrangementMaster::mPlayhead] != 0)
      TheTransport->SetMeasurePos(mMeasurePos[ArrangementMaster::mPlayhead]);
   
   if (mStructureInfoPoints.empty())
      return;
   
   if (mActiveStructureIdx == -1 ||
       (mActiveStructureIdx < mStructureInfoPoints.size()-1 &&
       mStructureInfoPoints[mActiveStructureIdx+1].mSample <= ArrangementMaster::mPlayhead))
   {
      ++mActiveStructureIdx;
      const StructureInfo& structure = mStructureInfoPoints[mActiveStructureIdx];
      TheScale->SetRoot(structure.mScaleRoot);
      TheScale->SetScaleType(structure.mScaleType);
      TheTransport->SetTimeSignature(structure.mTimeSigTop, structure.mTimeSigBottom);
      TheTransport->SetTempo(structure.mTempo);
      TheTransport->SetSwing(structure.mSwing);
   }
}

void MultitrackRecorder::AddRecordBuffer()
{
   mMutex.Lock("main thread");
   mRecordBuffers.push_back(new RecordBuffer(mRecordingLength));
   mRecordIdx = (int)mRecordBuffers.size() - 1;
   mMutex.Unlock();
}

int MultitrackRecorder::GetRecordIdx()
{
   if (!mRecording)
      return -1;
   return mRecordIdx;
}

float MultitrackRecorder::MeasureToPos(int measure)
{
   return (float)mMeasures[measure] / mRecordingLength;
}

int MultitrackRecorder::PosToMeasure(float pos)
{
   for (int i=0; i<mNumMeasures; ++i)
   {
      if (pos < MeasureToPos(i))
         return i-1;
   }
   return -1;
}

float MultitrackRecorder::MouseXToBufferPos(float mouseX)
{
   return (mouseX-5)/mBufferWidth;
}

void MultitrackRecorder::FilesDropped(vector<string> files, int x, int y)
{
   bool droppedClip = false;
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
   {
      if (mClipArranger[i].TestClick(x, y, false, true))
      {
         mClipArranger[i].FilesDropped(files, x, y);
         droppedClip = true;
      }
   }
   
   if (droppedClip == false)
   {
      mMutex.Lock("main thread");
      
      ResetAll();
      
      Sample sample;
      sample.Read(files[0].c_str());
      
      mRecordingLength = sample.LengthInSamples();
      RecordBuffer* buffer = new RecordBuffer(mRecordingLength);
      Mult(sample.Data()->GetChannel(0), .5f, mRecordingLength);
      BufferCopy(buffer->mLeft, sample.Data()->GetChannel(0), mRecordingLength);
      BufferCopy(buffer->mRight, sample.Data()->GetChannel(0), mRecordingLength);
      mRecordBuffers.push_back(buffer);
      
      delete[] mMeasurePos;
      mMeasurePos = new float[mRecordingLength];
      bzero(mMeasurePos, sizeof(float)*mRecordingLength);
      
      mMutex.Unlock();
   }
}

void MultitrackRecorder::GetModuleDimensions(float& width, float& height)
{
   float cW, cH;
   mClipArranger[0].GetDimensions(cW, cH);
   
   width = mBufferWidth + 100;
   height = 25 + mBufferHeight * mRecordBuffers.size() + cH*NUM_CLIP_ARRANGERS;
}

void MultitrackRecorder::ResetAll()
{
   for (int i=0; i<mRecordBuffers.size(); ++i)
      delete mRecordBuffers[i];
   mRecordBuffers.clear();
   mRecordingLength = RECORD_CHUNK_SIZE;
   mMaxRecordedLength = -1;
   mNumMeasures = 0;
   mRecording = false;
   mRecordIdx = 0;
   ArrangementMaster::mPlayhead = 0;
}

void MultitrackRecorder::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   for (int i=0; i<NUM_CLIP_ARRANGERS; ++i)
   {
      if (mClipArranger[i].TestClick(x, y, false, true))
         return;
   }
   
   if (y > 20 && x>5 && x<mBufferWidth+5)
   {
      int clickedIdx = ofClamp((y-20)/mBufferHeight,0,mRecordBuffers.size()-1);
      float clickPos = MouseXToBufferPos(x);
      
      if (IsKeyHeld('x'))
      {
         mMutex.Lock("main thread");
         
         DeleteBuffer(clickedIdx);
         
         if (mRecordBuffers.empty())  //deleted the last one
         {
            ResetAll();
            AddRecordBuffer();
         }
         
         mMutex.Unlock();
      }
      else if (IsKeyHeld('s'))
      {
         mSelectedMeasureStart = PosToMeasure(clickPos);
         mSelectedMeasureEnd = mSelectedMeasureStart+1;
         mSelecting = true;
      }
      else if (IsKeyHeld('a'))
      {
         if (mMergeBufferIdx == clickedIdx)
            mMergeBufferIdx = -1;
         else
            mMergeBufferIdx = clickedIdx;
      }
      else if (mMergeBufferIdx != -1)
      {
         if (mMergeBufferIdx != clickedIdx)
         {
            mMutex.Lock("main thread");
            FixLengths();
            Add(mRecordBuffers[clickedIdx]->mLeft, mRecordBuffers[mMergeBufferIdx]->mLeft, mRecordingLength);
            Add(mRecordBuffers[clickedIdx]->mRight, mRecordBuffers[mMergeBufferIdx]->mRight, mRecordingLength);
            DeleteBuffer(mMergeBufferIdx);
            mMutex.Unlock();
         }
         mMergeBufferIdx = -1;
      }
      else
      {
         ArrangementMaster::mPlayhead = ofClamp(clickPos * mRecordingLength,0,mRecordingLength-1);
         if (clickedIdx != mRecordIdx)
         {
            mRecordIdx = clickedIdx;
            mRecording = false;
         }
         mActiveStructureIdx = -1;
      }
   }
}

void MultitrackRecorder::MouseReleased()
{
   IDrawableModule::MouseReleased();
   if (mSelecting)
      mSelecting = false;
}

bool MultitrackRecorder::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   if (mSelecting)
   {
      float pos = MouseXToBufferPos(x);
      int measure = PosToMeasure(pos);
      if (measure != -1)
      {
         mSelectedMeasureStart = MIN(measure, mSelectedMeasureStart);
         mSelectedMeasureEnd = MAX(measure+1, mSelectedMeasureEnd);
      }
   }
   return false;
}

void MultitrackRecorder::FixLengths()
{
   mMutex.Lock("main thread");
   for (int i=0; i<mRecordBuffers.size(); ++i)
   {
      if (mRecordBuffers[i]->mLength < mRecordingLength)
      {
         float* newLeft = new float[mRecordingLength];
         float* newRight = new float[mRecordingLength];
         float* oldLeft = mRecordBuffers[i]->mLeft;
         float* oldRight = mRecordBuffers[i]->mRight;
         int oldLength = mRecordBuffers[i]->mLength;
         
         BufferCopy(newLeft, oldLeft, oldLength);
         BufferCopy(newRight, oldRight, oldLength);
         Clear(newLeft+oldLength, mRecordingLength-oldLength);
         Clear(newRight+oldLength, mRecordingLength-oldLength);
         
         mRecordBuffers[i]->mLeft = newLeft;
         mRecordBuffers[i]->mRight = newRight;
         mRecordBuffers[i]->mLength = mRecordingLength;
         
         delete[] oldLeft;
         delete[] oldRight;
      }
   }
   mMutex.Unlock();
}

void MultitrackRecorder::DeleteBuffer(int idx)
{
   mMutex.Lock("main thread");
   auto iter = mRecordBuffers.begin();
   for (int i=0; i<idx; ++i)
      ++iter;
   delete *iter;
   mRecordBuffers.erase(iter);
   mMutex.Unlock();
}

void MultitrackRecorder::CopyRecordBufferContents(RecordBuffer* dst, RecordBuffer* src)
{
   if (dst->mLength != src->mLength)
   {
      delete[] dst->mLeft;
      delete[] dst->mRight;
      dst->mLeft = new float[src->mLength];
      dst->mRight = new float[src->mLength];
      dst->mLength = src->mLength;
   }
   
   BufferCopy(dst->mLeft, src->mLeft, src->mLength);
   BufferCopy(dst->mRight, src->mRight, src->mLength);
}

void MultitrackRecorder::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void MultitrackRecorder::ButtonClicked(ClickButton* button)
{
   if (button == mAddTrackButton)
      AddRecordBuffer();
   if (button == mResetPlayheadButton)
      ArrangementMaster::mPlayhead = 0;
   if (button == mFixLengthsButton)
      FixLengths();
   if (button == mUndoRecordButton)
   {
      mRecording = false;
      CopyRecordBufferContents(mRecordBuffers[mRecordIdx], mUndoBuffer);
   }
}

void MultitrackRecorder::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mRecordCheckbox)
   {
      if (mRecordIdx == 0 && ArrangementMaster::mPlayhead == 0)
         TheTransport->Reset();
      CopyRecordBufferContents(mUndoBuffer, mRecordBuffers[mRecordIdx]);
   }
}

void MultitrackRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void MultitrackRecorder::SetUpFromSaveData()
{
}

MultitrackRecorder::RecordBuffer::RecordBuffer(int length)
: mLength(length)
{
   mLeft = new float[length];
   mRight = new float[length];
   bzero(mLeft, sizeof(float)*length);
   bzero(mRight, sizeof(float)*length);
   mControls.mVolSlider = new FloatSlider(TheMultitrackRecorder,"vol",0,0,90,15,&mControls.mVol,0,2);
   mControls.mMuteCheckbox = new Checkbox(TheMultitrackRecorder,"mute",0,0,&mControls.mMute);
}

MultitrackRecorder::RecordBuffer::~RecordBuffer()
{
   delete[] mLeft;
   delete[] mRight;
}

MultitrackRecorder::BufferControls::BufferControls()
: mVol(1)
, mVolSlider(nullptr)
, mMute(false)
, mMuteCheckbox(nullptr)
{
}

