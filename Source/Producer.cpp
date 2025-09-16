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
//  Producer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/13/13.
//
//

#include "Producer.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

const float mBufferX = 5;
const float mBufferY = 80;
const float mBufferW = 900;
const float mBufferH = 300;

Producer::Producer()
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   mSample = new Sample();

   for (int i = 0; i < PRODUCER_NUM_BIQUADS; ++i)
   {
      AddChild(&mBiquad[i]);
      mBiquad[i].SetPosition(150 + 100 * i, mBufferY + 10);
      mBiquad[i].SetFilterType(kFilterType_Lowpass);
      mBiquad[i].SetFilterParams(1600, sqrt(2) / 2);
   }
}

void Producer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "volume", 5, 20, 110, 15, &mVolume, 0, 2);
   mPlayCheckbox = new Checkbox(this, "play", 5, 60, &mPlay);
   mLoopCheckbox = new Checkbox(this, "loop", 55, 60, &mLoop);
   mClipStartSlider = new FloatSlider(this, "start", mBufferX, mBufferY + mBufferH - 30, 900, 15, &mClipStart, 0, gSampleRate * 200);
   mClipEndSlider = new FloatSlider(this, "end", mBufferX, mBufferY + mBufferH - 15, 900, 15, &mClipEnd, 0, gSampleRate * 200);
   mZoomStartSlider = new FloatSlider(this, "zoomstart", mBufferX, mBufferY + mBufferH + 5, 900, 15, &mZoomStart, 0, gSampleRate * 200);
   mZoomEndSlider = new FloatSlider(this, "zoomend", mBufferX, mBufferY + mBufferH + 20, 900, 15, &mZoomEnd, 0, gSampleRate * 200);
   mNumBarsSlider = new IntSlider(this, "num bars", 215, 3, 220, 15, &mNumBars, 1, 16);
   mOffsetSlider = new FloatSlider(this, "off", 215, 20, 110, 15, &mOffset, -1, 1, 4);
   mWriteButton = new ClickButton(this, "write", 600, 50);
   mDoubleLengthButton = new ClickButton(this, "double", 600, 10);
   mHalveLengthButton = new ClickButton(this, "halve", 600, 28);
   mTempoSlider = new FloatSlider(this, "tempo", 490, 10, 100, 15, &mTempo, 30, 200);
   mStartOffsetSlider = new IntSlider(this, "start", 490, 28, 100, 15, &mStartOffset, 0, gSampleRate * 4);
   mCalcTempoButton = new ClickButton(this, "calc tempo", 490, 46);
   mRestartButton = new ClickButton(this, "restart", 100, 60);

   for (int i = 0; i < PRODUCER_NUM_BIQUADS; ++i)
      mBiquad[i].CreateUIControls();
}

Producer::~Producer()
{
   delete[] mWriteBuffer;
   delete mSample;
}

void Producer::Process(double time)
{
   PROFILER(Producer);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr || mSample == nullptr || mPlay == false)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume;

   const float* data = mSample->Data()->GetChannel(0);
   int numSamples = mSample->LengthInSamples();

   for (int i = 0; i < bufferSize; ++i)
   {
      if (mPlayhead < numSamples)
      {
         out[i] += data[mPlayhead] * volSq;
      }
      else
      {
         mPlay = false;
         out[i] = 0; //fill the rest with zero
      }
      mPlayhead += 1;

      while (IsSkipMeasure(GetMeasureForSample(mPlayhead)))
      {
         mPlayhead += GetSamplesPerMeasure();
      }

      if (mLoop && mPlayhead > mClipEnd)
         mPlayhead = mClipStart;
   }

   ChannelBuffer buff(out, bufferSize);
   for (int i = 0; i < PRODUCER_NUM_BIQUADS; ++i)
      mBiquad[i].ProcessAudio(gTime, &buff);

   GetVizBuffer()->WriteChunk(out, bufferSize, 0);
}

void Producer::FilesDropped(std::vector<std::string> files, int x, int y)
{
   mSample->Reset();

   mSample->Read(files[0].c_str());

   mClipStart = 0;
   mClipEnd = mSample->LengthInSamples();
   mZoomStart = 0;
   mZoomEnd = mClipEnd;
   mZoomStartSlider->SetExtents(0, mClipEnd);
   mZoomEndSlider->SetExtents(0, mClipEnd);
   UpdateZoomExtents();
}

void Producer::DropdownClicked(DropdownList* list)
{
}

void Producer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void Producer::UpdateSample()
{
}

void Producer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mWriteButton)
   {
      DoWrite();
   }
   if (button == mDoubleLengthButton)
   {
      float newEnd = (mClipEnd - mClipStart) * 2 + mClipStart;
      if (newEnd < mSample->LengthInSamples())
      {
         mClipEnd = newEnd;
         mNumBars *= 2;
      }
   }
   if (button == mHalveLengthButton)
   {
      if (mNumBars % 2 == 0)
      {
         float newEnd = (mClipEnd - mClipStart) / 2 + mClipStart;
         mClipEnd = newEnd;
         mNumBars /= 2;
      }
   }
   if (button == mCalcTempoButton)
   {
      if (mClipStart < mClipEnd)
      {
         float samplesPerMeasure = mClipEnd - mClipStart;
         float secondsPerMeasure = samplesPerMeasure / gSampleRate;
         mTempo = 1 / secondsPerMeasure * 4 * 60 * mNumBars;
         mStartOffset = ((mClipStart / samplesPerMeasure) - int(mClipStart / samplesPerMeasure)) * samplesPerMeasure;
      }
   }
   if (button == mRestartButton)
      mPlayhead = mClipStart;
}

void Producer::DoWrite()
{
   if (mSample)
   {
      ChannelBuffer sample(mSample->LengthInSamples());
      sample.CopyFrom(mSample->Data());
      for (int i = 0; i < PRODUCER_NUM_BIQUADS; ++i)
         mBiquad[i].ProcessAudio(gTime, &sample);

      float* toWrite = new float[mSample->LengthInSamples()];
      int pos = 0;
      for (int i = 0; i < mSample->LengthInSamples(); ++i)
      {
         if (IsSkipMeasure(GetMeasureForSample(i)) == false)
         {
            toWrite[pos] = sample.GetChannel(0)[i];
            ++pos;
         }
      }

      Sample::WriteDataToFile(ofGetTimestampString("producer/producer_%Y-%m-%d_%H-%M.wav").c_str(), &toWrite, pos);
      mClipStart = 0;
      mClipEnd = mSample->LengthInSamples();
      mOffset = 0;

      mZoomStart = 0;
      mZoomEnd = mClipEnd;
      mZoomStartSlider->SetExtents(0, mClipEnd);
      mZoomEndSlider->SetExtents(0, mClipEnd);
      UpdateZoomExtents();
   }
}

void Producer::UpdateZoomExtents()
{
   mClipStartSlider->SetExtents(mZoomStart, mZoomEnd);
   mClipEndSlider->SetExtents(mZoomStart, mZoomEnd);
}

int Producer::GetMeasureSample(int measure)
{
   return mStartOffset + measure * GetSamplesPerMeasure();
}

float Producer::GetBufferPos(int sample)
{
   return (sample - mZoomStart) / (mZoomEnd - mZoomStart);
}

int Producer::GetMeasureForSample(int sample)
{
   return (sample - mStartOffset) / GetSamplesPerMeasure();
}

int Producer::GetSamplesPerMeasure()
{
   return gSampleRate / (mTempo / 60 / 4);
}

bool Producer::IsSkipMeasure(int measure)
{
   return ListContains(measure, mSkipMeasures);
}

void Producer::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
   mPlayCheckbox->Draw();
   mLoopCheckbox->Draw();

   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(mBufferX, mBufferY);
      ofPushStyle();

      mSample->LockDataMutex(true);
      DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), mZoomStart, mZoomEnd, (int)mPlayhead);
      mSample->LockDataMutex(false);

      ofFill();
      for (int measure = 0; GetMeasureSample(measure) < mZoomEnd; ++measure)
      {
         if (GetMeasureSample(measure) >= mZoomStart)
         {
            float pos = GetBufferPos(GetMeasureSample(measure));
            ofSetColor(0, 0, 255);
            ofRect(pos * mBufferW, 0, 1, mBufferH);

            if (IsSkipMeasure(measure))
            {
               ofSetColor(255, 0, 0, 100);
               ofRect(pos * mBufferW, 0, (GetBufferPos(GetMeasureSample(measure + 1)) - pos) * mBufferW, mBufferH);
            }
         }
      }

      /*int start = ofMap(mClipStart, 0, length, 0, width, true);
       int end = ofMap(mClipEnd, 0, length, 0, width, true);
       
       for (int i = 0; i < mNumBars; i++)
       {
       float barSpacing = float(end-start)/mNumBars;
       int x =  barSpacing * i + start;
       x += barSpacing * -mOffset;
       ofSetColor(255,255,0);
       ofLine(x, 0, x, height);
       }
       
       ofSetColor(255,0,0);
       ofLine(start,0,start,height);
       ofLine(end,0,end,height);
       
       ofSetColor(0,255,0);
       int position =  ofMap(pos, 0, length, 0, width, true);
       ofLine(position,0,position,height);*/

      ofPopStyle();
      ofPopMatrix();

      mClipStartSlider->Draw();
      mClipEndSlider->Draw();
      mZoomStartSlider->Draw();
      mZoomEndSlider->Draw();
      mNumBarsSlider->Draw();
      mOffsetSlider->Draw();
      mWriteButton->Draw();
      mDoubleLengthButton->Draw();
      mHalveLengthButton->Draw();
      mTempoSlider->Draw();
      mStartOffsetSlider->Draw();
      mCalcTempoButton->Draw();
      mRestartButton->Draw();
      if (mSample)
         DrawTextNormal(ofToString(mSample->GetPlayPosition()), 335, 50);
   }

   for (int i = 0; i < PRODUCER_NUM_BIQUADS; ++i)
      mBiquad[i].Draw();
}

void Producer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (x >= mBufferX && y >= mBufferY + 100 && x < mBufferX + mBufferW && y < mBufferY + mBufferH)
   {
      if (IsKeyHeld('x'))
      {
         float pos = (x - mBufferX) / mBufferW;
         float sample = pos * (mZoomEnd - mZoomStart) + mZoomStart;
         int measure = GetMeasureForSample(sample);
         if (IsSkipMeasure(measure))
            mSkipMeasures.remove(measure);
         else
            mSkipMeasures.push_back(measure);
      }
      else
      {
         mPlayhead = ofMap(x, mBufferX, mBufferX + mBufferW, mZoomStart, mZoomEnd, true);
      }
   }
}

void Producer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mPlayCheckbox)
   {
      if (mSample)
         mSample->Reset();
   }
}

void Producer::GetModuleDimensions(float& width, float& height)
{
   width = 910;
   height = 430;
}

void Producer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mClipStartSlider)
   {
      if (mSample && mClipStart > mSample->LengthInSamples())
         mClipStart = mSample->LengthInSamples();
   }
   if (slider == mClipEndSlider)
   {
      if (mSample && mClipEnd > mSample->LengthInSamples())
         mClipEnd = mSample->LengthInSamples();
   }
   if (slider == mZoomStartSlider)
   {
      if (mSample && mZoomStart > mSample->LengthInSamples())
         mZoomStart = mSample->LengthInSamples();
      UpdateZoomExtents();
   }
   if (slider == mZoomEndSlider)
   {
      if (mSample && mZoomEnd > mSample->LengthInSamples())
         mZoomEnd = mSample->LengthInSamples();
      UpdateZoomExtents();
   }
}

void Producer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void Producer::PlayNote(NoteMessage note)
{
   if (mSample)
   {
      mPlay = false;
      if (note.pitch == 16)
      {
         mSample->Reset();
      }
      else if (note.pitch >= 0 && note.pitch < 16 && note.velocity > 0)
      {
         int slice = (note.pitch / 8) * 8 + 7 - (note.pitch % 8);
         int barLength = (mClipEnd - mClipStart) / mNumBars;
         int position = -mOffset * barLength + (barLength / 4) * slice + mClipStart;
         mSample->Play(note.time, 1, position);
      }
   }
}

void Producer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Producer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
