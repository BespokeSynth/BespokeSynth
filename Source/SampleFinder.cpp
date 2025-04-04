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
//  SampleFinder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#include "SampleFinder.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

SampleFinder::SampleFinder()
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   mSample = new Sample();

   mSampleDrawer.SetSample(mSample);
   mSampleDrawer.SetPosition(5, 80);
   mSampleDrawer.SetDimensions(200, 40);
}

void SampleFinder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this, "volume", 5, 20, 110, 15, &mVolume, 0, 2);
   mPlayCheckbox = new Checkbox(this, "play", 5, 60, &mPlay);
   mLoopCheckbox = new Checkbox(this, "loop", 60, 60, &mLoop);
   mEditCheckbox = new Checkbox(this, "edit", 100, 2, &mEditMode);
   mClipStartSlider = new IntSlider(this, "start", 5, 395, 900, 15, &mClipStart, 0, gSampleRate * 200);
   mClipEndSlider = new IntSlider(this, "end", 5, 410, 900, 15, &mClipEnd, 0, gSampleRate * 200);
   mNumBarsSlider = new IntSlider(this, "num bars", 215, 3, 220, 15, &mNumBars, 1, 16);
   mOffsetSlider = new FloatSlider(this, "offset", 215, 20, 110, 15, &mOffset, gSampleRate * -.5f, gSampleRate * .5f, 4);
   mWriteButton = new ClickButton(this, "write", 600, 50);
   mDoubleLengthButton = new ClickButton(this, "double", 600, 10);
   mHalveLengthButton = new ClickButton(this, "halve", 600, 28);
   mReverseCheckbox = new Checkbox(this, "reverse", 500, 10, &mReverse);
}

SampleFinder::~SampleFinder()
{
   delete[] mWriteBuffer;
   delete mSample;
}

void SampleFinder::Process(double time)
{
   PROFILER(SampleFinder);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr || mSample == nullptr || mPlay == false)
      return;

   ComputeSliders(0);

   if (mWantWrite)
   {
      DoWrite();
      mWantWrite = false;
   }

   int bufferSize = target->GetBuffer()->BufferSize();
   float* out = target->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   float volSq = mVolume * mVolume;

   float speed = GetSpeed();

   //TODO(Ryan) multichannel
   const float* data = mSample->Data()->GetChannel(0);
   int numSamples = mSample->LengthInSamples();
   float sampleRateRatio = mSample->GetSampleRateRatio();

   mPlayhead = TheTransport->GetMeasurePos(time) + (TheTransport->GetMeasure(time) % mNumBars);
   if (mReverse)
      mPlayhead = 1 - mPlayhead;
   mPlayhead /= mNumBars;
   mPlayhead *= mClipEnd - mClipStart;
   mPlayhead += mClipStart;
   mPlayhead += mOffset;

   for (int i = 0; i < bufferSize; ++i)
   {
      if (mPlayhead >= mClipEnd)
         mPlayhead -= (mClipEnd - mClipStart);
      if (mPlayhead < mClipStart)
         mPlayhead += (mClipEnd - mClipStart);

      int pos = int(mPlayhead);
      int posNext = int(mPlayhead) + 1;
      if (pos < numSamples)
      {
         float sample = pos < 0 ? 0 : data[pos];
         float nextSample = posNext >= numSamples ? 0 : data[posNext];
         float a = mPlayhead - pos;
         out[i] += ((1 - a) * sample + a * nextSample) * volSq; //interpolate
      }
      else
      {
         out[i] = 0; //fill the rest with zero
      }
      GetVizBuffer()->Write(out[i], 0);
      mPlayhead += speed * sampleRateRatio;
   }
}

void SampleFinder::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mVolumeSlider->Draw();
   mPlayCheckbox->Draw();
   mLoopCheckbox->Draw();
   mEditCheckbox->Draw();

   if (mSample)
   {
      if (!mEditMode)
      {
         mSampleDrawer.SetDimensions(200, 40);
         mSampleDrawer.SetRange(mClipStart, mClipEnd);
         mSampleDrawer.Draw((int)mPlayhead);
      }
      else
      {
         mSampleDrawer.SetDimensions(900, 310);
         mSampleDrawer.SetRange(mZoomStart, mZoomEnd);
         mSampleDrawer.Draw((int)mPlayhead);
         mSampleDrawer.DrawLine(mClipStart, ofColor::red);
         mSampleDrawer.DrawLine(mClipEnd, ofColor::red);

         mClipStartSlider->Draw();
         mClipEndSlider->Draw();
         mNumBarsSlider->Draw();
         mOffsetSlider->Draw();
         mWriteButton->Draw();
         mDoubleLengthButton->Draw();
         mHalveLengthButton->Draw();
         mReverseCheckbox->Draw();
         DrawTextNormal(ofToString(mSample->GetPlayPosition()), 335, 50);
         DrawTextNormal("speed: " + ofToString(GetSpeed()), 4, 55);
      }
   }
}

bool SampleFinder::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   ofVec2f bufferPos = ofVec2f(ofMap(x, 5, 5 + 900, 0, 1),
                               ofMap(y, 80, 80 + 310, 0, 1));
   if (IsInUnitBox(bufferPos))
   {
      float zoomCenter = ofLerp(mZoomStart, mZoomEnd, bufferPos.x);
      float distFromStart = zoomCenter - mZoomStart;
      float distFromEnd = zoomCenter - mZoomEnd;

      distFromStart *= 1 - scrollY / 100;
      distFromEnd *= 1 - scrollY / 100;

      float slideX = (mZoomEnd - mZoomStart) * -scrollX / 300;

      mZoomStart = ofClamp(zoomCenter - distFromStart + slideX, 0, mSample->LengthInSamples());
      mZoomEnd = ofClamp(zoomCenter - distFromEnd + slideX, 0, mSample->LengthInSamples());
      UpdateZoomExtents();
      return true;
   }
   return false;
}

void SampleFinder::FilesDropped(std::vector<std::string> files, int x, int y)
{
   mSample->Reset();

   mSample->Read(files[0].c_str());

   mClipStart = 0;
   mClipEnd = mSample->LengthInSamples();
   mZoomStart = 0;
   mZoomEnd = mClipEnd;
   UpdateZoomExtents();
}

float SampleFinder::GetSpeed()
{
   return float(mClipEnd - mClipStart) * gInvSampleRateMs / TheTransport->MsPerBar() / mNumBars * (mReverse ? -1 : 1);
}

void SampleFinder::DropdownClicked(DropdownList* list)
{
}

void SampleFinder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void SampleFinder::UpdateSample()
{
}

void SampleFinder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mWriteButton)
   {
      mWantWrite = true;
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
}

void SampleFinder::DoWrite()
{
   if (mSample)
   {
      mSample->ClipTo(mClipStart, mClipEnd);
      int shift = mOffset * (mClipEnd - mClipStart);
      if (shift < 0)
         shift += mClipEnd - mClipStart;
      mSample->ShiftWrap(shift);
      mSample->Write(ofGetTimestampString("loops/sample_%Y-%m-%d_%H-%M.wav").c_str());
      mClipStart = 0;
      mClipEnd = mSample->LengthInSamples();
      mOffset = 0;

      mZoomStart = 0;
      mZoomEnd = mClipEnd;
      UpdateZoomExtents();
   }
}

void SampleFinder::UpdateZoomExtents()
{
   mClipStartSlider->SetExtents(mZoomStart, mZoomEnd);
   mClipEndSlider->SetExtents(mZoomStart, mZoomEnd);
}

void SampleFinder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mPlayCheckbox)
   {
      if (mSample)
         mSample->Reset();
   }
}

void SampleFinder::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = 910;
      height = 430;
   }
   else
   {
      width = 210;
      height = 125;
   }
}

void SampleFinder::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void SampleFinder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
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
}

void SampleFinder::PlayNote(NoteMessage note)
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

void SampleFinder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SampleFinder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
