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
//  ClipLauncher.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/17/15.
//
//

#include "ClipLauncher.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Looper.h"
#include "FillSaveDropdown.h"

ClipLauncher::ClipLauncher()
{
}

void ClipLauncher::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_1n, OffsetInfo(0, true), false);
}

void ClipLauncher::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "volume", 5, 2, 110, 15, &mVolume, 0, 2);
}

ClipLauncher::~ClipLauncher()
{
   TheTransport->RemoveListener(this);
}

void ClipLauncher::Process(double time)
{
   PROFILER(ClipLauncher);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   int sampleToPlay = -1;
   for (int i = 0; i < mSamples.size(); ++i)
   {
      if (mSamples[i].mPlay)
         sampleToPlay = i;
   }

   Sample* sample = nullptr;
   float volSq = 1;
   if (sampleToPlay != -1)
   {
      mSampleMutex.lock();

      sample = mSamples[sampleToPlay].mSample;
      volSq = mVolume * mSamples[sampleToPlay].mVolume;
      volSq *= volSq;

      float speed = sample->LengthInSamples() * gInvSampleRateMs / TheTransport->MsPerBar() / mSamples[sampleToPlay].mNumBars;
      RecalcPos(time, sampleToPlay);
      sample->SetRate(speed);
   }

   if (sample)
   {
      gWorkChannelBuffer.SetNumActiveChannels(1);
      sample->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true);
   }

   for (int i = 0; i < bufferSize; ++i)
   {
      float samp = 0;
      if (sample)
         samp = gWorkChannelBuffer.GetChannel(0)[i] * volSq;
      samp = mJumpBlender.Process(samp, i);
      out[i] += samp;
      GetVizBuffer()->Write(samp, 0);
   }

   if (sampleToPlay != -1)
      mSampleMutex.unlock();
}

void ClipLauncher::DropdownClicked(DropdownList* list)
{
}

void ClipLauncher::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void ClipLauncher::OnTimeEvent(double time)
{
}

void ClipLauncher::DrawModule()
{

   DrawConnection(mLooper);
   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();

   for (int i = 0; i < mSamples.size(); ++i)
   {
      mSamples[i].Draw();
   }
}

int ClipLauncher::GetRowY(int idx)
{
   return 20 + idx * 40;
}

void ClipLauncher::ButtonClicked(ClickButton* button, double time)
{
}

void ClipLauncher::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (int i = 0; i < mSamples.size(); ++i)
   {
      if (checkbox == mSamples[i].mPlayCheckbox)
      {
         int currentlyPlaying = -1;
         for (int j = 0; j < mSamples.size(); ++j)
         {
            if (j != i && mSamples[j].mPlay)
               currentlyPlaying = j;
         }
         for (int j = 0; j < mSamples.size(); ++j)
         {
            if (j != i)
               mSamples[j].mPlay = false;
         }
         int newPlaying = -1;
         for (int j = 0; j < mSamples.size(); ++j)
         {
            if (mSamples[j].mPlay)
               newPlaying = j;
         }
         if (currentlyPlaying != newPlaying)
         {
            mSampleMutex.lock();
            float data[JUMP_BLEND_SAMPLES];
            ChannelBuffer temp(data, JUMP_BLEND_SAMPLES);
            if (currentlyPlaying != -1)
               mSamples[currentlyPlaying].mSample->ConsumeData(time, &temp, JUMP_BLEND_SAMPLES, true);
            mJumpBlender.CaptureForJump(0, data, JUMP_BLEND_SAMPLES, gBufferSize);
            mSampleMutex.unlock();
         }
      }

      if (checkbox == mSamples[i].mGrabCheckbox)
      {
         if (mSamples[i].mHasSample)
         {
            if (mSamples[i].mPlay)
               mSampleMutex.lock();

            int bufferSize;
            mSamples[i].mSample->Create(mLooper->GetLoopBuffer(bufferSize));
            mSamples[i].mNumBars = mLooper->NumBars();
            mLooper->Clear();

            if (mSamples[i].mPlay)
               mSampleMutex.unlock();

            for (int j = 0; j < mSamples.size(); ++j)
               mSamples[j].mPlay = (j == i);
         }
         else
         {
            mSamples[i].mPlay = false;
            mSamples[i].mSample->Create(1);
         }
      }
   }
}

void ClipLauncher::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 180;
}

void ClipLauncher::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void ClipLauncher::RecalcPos(double time, int idx)
{
   int numBars = mSamples[idx].mNumBars;
   Sample* sample = mSamples[idx].mSample;

   if (sample)
   {
      float measurePos = TheTransport->GetMeasure(time) % numBars + TheTransport->GetMeasurePos(time);
      int pos = ofMap(measurePos / numBars, 0, 1, 0, sample->LengthInSamples(), true);
      sample->SetPlayPosition(pos);
   }
}

void ClipLauncher::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void ClipLauncher::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("looper", moduleInfo, "", FillDropdown<Looper*>);
   mModuleSaveData.LoadInt("numclips", moduleInfo, 4, 1, 16);

   SetUpFromSaveData();
}

void ClipLauncher::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mLooper = dynamic_cast<Looper*>(TheSynth->FindModule(mModuleSaveData.GetString("looper"), false));
   mSamples.resize(mModuleSaveData.GetInt("numclips"));
   for (int i = 0; i < mSamples.size(); ++i)
   {
      mSamples[i].Init(this, i);
   }
}

ClipLauncher::SampleData::~SampleData()
{
   delete mSample;
}

void ClipLauncher::SampleData::Init(ClipLauncher* launcher, int index)
{
   mClipLauncher = launcher;
   mIndex = index;

   delete mSample;
   if (mGrabCheckbox)
   {
      launcher->RemoveUIControl(mGrabCheckbox);
      mGrabCheckbox->Delete();
   }
   if (mPlayCheckbox)
   {
      launcher->RemoveUIControl(mPlayCheckbox);
      mPlayCheckbox->Delete();
   }

   std::string indexStr = ofToString(index + 1);

   mSample = new Sample();
   mSample->SetLooping(true);
   int y = launcher->GetRowY(index);
   mGrabCheckbox = new Checkbox(launcher, ("grab" + indexStr).c_str(), 110, y, &mHasSample);
   mPlayCheckbox = new Checkbox(launcher, ("play" + indexStr).c_str(), 110, y + 20, &mPlay);
}

void ClipLauncher::SampleData::Draw()
{
   ofPushMatrix();
   ofTranslate(5, mClipLauncher->GetRowY(mIndex));
   DrawAudioBuffer(100, 36, mSample->Data(), 0, mSample->LengthInSamples(), mPlay ? mSample->GetPlayPosition() : -1);
   ofPopMatrix();
   mGrabCheckbox->Draw();
   mPlayCheckbox->Draw();
}
