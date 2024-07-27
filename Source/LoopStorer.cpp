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
//  LoopStorer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/22/15.
//
//

#include "LoopStorer.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Looper.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "ChannelBuffer.h"

LoopStorer::LoopStorer()
{
}

void LoopStorer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_None, OffsetInfo(0, true), false);
}

void LoopStorer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRewriteToSelectionCheckbox = new Checkbox(this, "rewrite", 4, 2, &mRewriteToSelection);
   mQuantizationDropdown = new DropdownList(this, "quantization", 135, 2, ((int*)&mQuantization));
   mClearButton = new ClickButton(this, "clear", 70, 2);

   mQuantizationDropdown->AddLabel("none", kInterval_None);
   mQuantizationDropdown->AddLabel("4n", kInterval_4n);
   mQuantizationDropdown->AddLabel("1n", kInterval_1n);

   mLooperCable = new PatchCableSource(this, kConnectionType_Special);
   mLooperCable->AddTypeFilter("looper");
   AddPatchCableSource(mLooperCable);
}

LoopStorer::~LoopStorer()
{
   for (int i = 0; i < mSamples.size(); ++i)
      delete mSamples[i];
   TheTransport->RemoveListener(this);
}

void LoopStorer::Poll()
{
   if (mLooper == nullptr)
      return;

   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);

   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i]->mIsCurrentBuffer = (i == mCurrentBufferIdx);

   if (mIsSwapping)
   {
      int loopLength;
      ChannelBuffer* buffer = mLooper->GetLoopBuffer(loopLength);
      if (buffer == mSamples[mCurrentBufferIdx]->mBuffer) //finished swap
         mIsSwapping = false;
   }

   if (!mIsSwapping)
   {
      mSwapMutex.lock();
      int loopLength;
      mSamples[mCurrentBufferIdx]->mBuffer = mLooper->GetLoopBuffer(loopLength);
      mSamples[mCurrentBufferIdx]->mNumBars = mLooper->GetNumBars();
      mSamples[mCurrentBufferIdx]->mBufferLength = loopLength;
      mSwapMutex.unlock();
   }

   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);
}

void LoopStorer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRewriteToSelectionCheckbox->Draw();
   mQuantizationDropdown->Draw();
   mClearButton->Draw();

   mSwapMutex.lock();
   mLoadMutex.lock();
   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);
   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i]->Draw();
   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);
   mLoadMutex.unlock();
   mSwapMutex.unlock();
}

void LoopStorer::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mLooper = dynamic_cast<Looper*>(cableSource->GetTarget());
}

int LoopStorer::GetRowY(int idx)
{
   return 20 + idx * 40;
}

void LoopStorer::OnTimeEvent(double time)
{
   if (mQueuedSwapBufferIdx != -1)
   {
      SwapBuffer(mQueuedSwapBufferIdx);
      mQueuedSwapBufferIdx = -1;
   }
}

void LoopStorer::SwapBuffer(int swapToIdx)
{
   if (mLooper == nullptr)
      return;

   mSwapMutex.lock();
   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);
   //TODO(Ryan) make loopstorer actually use ChannelBuffers
   //mLooper->SetLoopBuffer(mSamples[swapToIdx]->mBuffer);
   if (mRewriteToSelection)
   {
      mSamples[swapToIdx]->mNumBars = mLooper->GetRecorderNumBars();
      mLooper->Rewrite();
      mRewriteToSelection = false;
   }
   else
   {
      mLooper->SetNumBars(mSamples[swapToIdx]->mNumBars);
   }
   mCurrentBufferIdx = swapToIdx;
   mIsSwapping = true;
   assert(mSamples[0]->mBuffer != mSamples[1]->mBuffer);
   mSwapMutex.unlock();
}

void LoopStorer::DropdownClicked(DropdownList* list)
{
}

void LoopStorer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mQuantizationDropdown)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mQuantization;
   }
}

void LoopStorer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
   {
      mSwapMutex.lock();
      if (mLooper)
         mLooper->LockBufferMutex();
      for (int i = 0; i < mSamples.size(); ++i)
      {
         mSamples[i]->mBuffer->Clear();
      }
      if (mLooper)
         mLooper->UnlockBufferMutex();
      mSwapMutex.unlock();
   }
}

void LoopStorer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (int i = 0; i < mSamples.size(); ++i)
   {
      if (checkbox == mSamples[i]->mSelectCheckbox)
      {
         if (mQuantization == kInterval_None)
            SwapBuffer(i);
         else
            mQueuedSwapBufferIdx = i;
      }
   }
}

void LoopStorer::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = GetRowY((int)mSamples.size());
}

void LoopStorer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void LoopStorer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void LoopStorer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("looper", moduleInfo, "", FillDropdown<Looper*>);
   mModuleSaveData.LoadInt("numclips", moduleInfo, 4, 1, 16);
   mModuleSaveData.LoadEnum<NoteInterval>("quantization", moduleInfo, kInterval_None, mQuantizationDropdown);

   SetUpFromSaveData();
}

void LoopStorer::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["looper"] = mLooper ? mLooper->Name() : "";
}

void LoopStorer::SetUpFromSaveData()
{
   mLooperCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("looper"), false));

   for (int i = 0; i < mSamples.size(); ++i)
      delete mSamples[i];
   mSamples.resize(mModuleSaveData.GetInt("numclips"));
   for (int i = 0; i < mSamples.size(); ++i)
   {
      mSamples[i] = new SampleData();
      mSamples[i]->Init(this, i);
   }

   mQuantization = mModuleSaveData.GetEnum<NoteInterval>("quantization");
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
      transportListenerInfo->mInterval = mQuantization;
}

void LoopStorer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mCurrentBufferIdx;

   for (auto* sampleData : mSamples)
   {
      if (!sampleData->mIsCurrentBuffer)
      {
         out << sampleData->mNumBars;
         out << sampleData->mBufferLength;
         if (sampleData->mBufferLength != -1)
            sampleData->mBuffer->Save(out, sampleData->mBufferLength);
      }
   }
}

void LoopStorer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mCurrentBufferIdx;
   mQueuedSwapBufferIdx = -1;
   for (int i = 0; i < mSamples.size(); ++i)
      mSamples[i]->mIsCurrentBuffer = (i == mCurrentBufferIdx);

   if (mCurrentBufferIdx != 0)
   {
      mSamples[0]->mBuffer = mSamples[mCurrentBufferIdx]->mBuffer;

      int loopLength;
      mSamples[mCurrentBufferIdx]->mBuffer = mLooper->GetLoopBuffer(loopLength);
      mSamples[mCurrentBufferIdx]->mNumBars = mLooper->GetNumBars();
      mSamples[mCurrentBufferIdx]->mBufferLength = loopLength;
   }

   mLoadMutex.lock();
   for (auto* sampleData : mSamples)
   {
      if (!sampleData->mIsCurrentBuffer)
      {
         in >> sampleData->mNumBars;
         in >> sampleData->mBufferLength;
         if (sampleData->mBufferLength != -1)
         {
            int readLength;
            sampleData->mBuffer->Load(in, readLength, ChannelBuffer::LoadMode::kAnyBufferSize);
            assert(sampleData->mBufferLength == sampleData->mBuffer->BufferSize());
         }
      }
   }
   mLoadMutex.unlock();
}

LoopStorer::SampleData::SampleData()
{
}

LoopStorer::SampleData::~SampleData()
{
   if (mIsCurrentBuffer == false)
      delete mBuffer;
   if (mSelectCheckbox)
   {
      mLoopStorer->RemoveUIControl(mSelectCheckbox);
      mSelectCheckbox->Delete();
   }
}

void LoopStorer::SampleData::Init(LoopStorer* storer, int index)
{
   mLoopStorer = storer;
   mIndex = index;

   if (mIsCurrentBuffer == false)
      delete mBuffer;

   if (index == 0) //we're the first one, grab our buffer from the looper
   {
      Looper* looper = storer->GetLooper();
      if (looper)
      {
         mBuffer = looper->GetLoopBuffer(mBufferLength);
         mNumBars = looper->GetNumBars();
      }
      mIsCurrentBuffer = true;
   }
   else
   {
      mBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
      mNumBars = 1;
      mIsCurrentBuffer = false;
   }

   if (mSelectCheckbox)
   {
      storer->RemoveUIControl(mSelectCheckbox);
      mSelectCheckbox->Delete();
   }

   std::string indexStr = ofToString(index + 1);

   int y = storer->GetRowY(index);
   mSelectCheckbox = new Checkbox(storer, ("select " + indexStr).c_str(), 110, y + 20, &mIsCurrentBuffer);
}

void LoopStorer::SampleData::Draw()
{
   ofPushMatrix();
   ofTranslate(5, mLoopStorer->GetRowY(mIndex));
   ChannelBuffer* buffer = mBuffer;
   int bufferLength = mBufferLength;
   bool useLooper = mIsCurrentBuffer && mLoopStorer && mLoopStorer->GetLooper();
   if (useLooper)
   {
      mLoopStorer->GetLooper()->LockBufferMutex();
      buffer = mLoopStorer->GetLooper()->GetLoopBuffer(bufferLength); //make sure we have the latest and greatest
   }
   DrawAudioBuffer(100, 36, buffer, 0, bufferLength, -1);
   if (useLooper)
      mLoopStorer->GetLooper()->UnlockBufferMutex();
   DrawTextNormal(ofToString(mNumBars), 4, 12);
   if (mLoopStorer->GetQueuedBufferIdx() == mIndex)
   {
      ofPushStyle();
      ofSetColor(255, 100, 0);
      ofFill();
      ofRect(107, 24, 8, 8);
      ofPopStyle();
   }
   ofPopMatrix();
   mSelectCheckbox->Draw();
}
