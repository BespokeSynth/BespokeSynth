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
//  ClipArranger.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 8/26/14.
//
//

#include "ClipArranger.h"
#include "ArrangementController.h"
#include "ModularSynth.h"

ClipArranger::ClipArranger()
{
}

ClipArranger::~ClipArranger()
{
}

void ClipArranger::Poll()
{
}

void ClipArranger::Process(double time, float* left, float* right, int bufferSize)
{
   if (mEnabled == false)
      return;

   if (ArrangementController::mPlay)
   {
      for (int i = 0; i < MAX_CLIPS; ++i)
         mClips[i].Process(left, right, bufferSize);
   }
}

void ClipArranger::DrawModule()
{
   ofPushStyle();
   ofSetColor(0, 0, 0);
   ofFill();
   ofRect(BUFFER_MARGIN_X, BUFFER_MARGIN_Y, mBufferWidth, mBufferHeight);

   for (int i = 0; i < MAX_CLIPS; ++i)
   {
      if (mClips[i].mSample != nullptr)
      {
         float xStart = SampleToX(mClips[i].mStartSample);
         float xPos = xStart;
         float xEnd = SampleToX(mClips[i].mEndSample);
         float sampleWidth = SampleToX(mClips[i].mStartSample + mClips[i].mSample->LengthInSamples()) - xStart;
         for (; xPos < xEnd; xPos += sampleWidth)
         {
            ofPushMatrix();
            ofTranslate(xPos, BUFFER_MARGIN_Y);
            float length = mClips[i].mSample->LengthInSamples();
            if (xPos + sampleWidth > xEnd)
            {
               float newWidth = xEnd - xPos;
               length *= newWidth / sampleWidth;
               sampleWidth = newWidth;
            }
            DrawAudioBuffer(sampleWidth, mBufferHeight, mClips[i].mSample->Data(), 0, length, 0);
            ofPopMatrix();
         }

         ofSetColor(0, 255, 255);
         ofNoFill();
         ofSetLineWidth(i == mHighlightClip ? 3 : 1);
         ofRect(xStart, BUFFER_MARGIN_Y, xEnd - xStart, mBufferHeight);
      }
   }

   ofPopStyle();
}

void ClipArranger::GetModuleDimensions(float& w, float& h)
{
   w = mBufferWidth + 100;
   h = 25 + mBufferHeight;
}

void ClipArranger::OnClicked(float x, float y, bool right)
{
   mMouseDown = true;

   if (mHighlightClip != -1)
   {
      if (IsKeyHeld('x'))
      {
         delete mClips[mHighlightClip].mSample;
         mClips[mHighlightClip].mSample = nullptr;
      }
      else
      {
         int mouseSample = MouseXToSample(x);
         if (abs(mouseSample - mClips[mHighlightClip].mStartSample) < abs(mouseSample - mClips[mHighlightClip].mEndSample))
            mMoveMode = kMoveMode_Start;
         else
            mMoveMode = kMoveMode_End;
      }
   }
}

void ClipArranger::MouseReleased()
{
   if (mMouseDown)
   {
      mMouseDown = false;
      mMoveMode = kMoveMode_None;
   }

   if (IsMousePosWithinClip(mLastMouseX, mLastMouseY))
   {
      Sample* heldSample = TheSynth->GetHeldSample();
      if (heldSample)
      {
         Sample* sample = new Sample();
         sample->Create(heldSample->Data());
         AddSample(sample, mLastMouseX, mLastMouseY);
         TheSynth->ClearHeldSample();
      }
   }
}

bool ClipArranger::MouseMoved(float x, float y)
{
   mLastMouseX = x;
   mLastMouseY = y;
   int mouseSample = MouseXToSample(x);
   if (!mMouseDown)
   {
      bool found = false;

      if (IsMousePosWithinClip(x, y))
      {
         for (int i = 0; i < MAX_CLIPS; ++i)
         {
            if (mClips[i].mSample != nullptr)
            {
               if (mouseSample >= mClips[i].mStartSample && mouseSample < mClips[i].mEndSample)
               {
                  mHighlightClip = i;
                  found = true;
                  break;
               }
            }
         }
      }
      if (!found)
         mHighlightClip = -1;
   }
   else
   {
      if (mHighlightClip != -1)
      {
         if (mMoveMode == kMoveMode_Start)
            mClips[mHighlightClip].mStartSample = mouseSample;
         else if (mMoveMode == kMoveMode_End)
            mClips[mHighlightClip].mEndSample = mouseSample;
      }
   }
   return false;
}

bool ClipArranger::IsMousePosWithinClip(int x, int y)
{
   float w, h;
   GetDimensions(w, h);
   return x >= 0 && x < w && y >= 0 && y < h;
}

void ClipArranger::FilesDropped(std::vector<std::string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   AddSample(sample, x, y);
}

void ClipArranger::AddSample(Sample* sample, int x, int y)
{
   Clip* clip = GetEmptyClip();
   clip->mSample = sample;
   clip->mStartSample = MouseXToSample(x);
   clip->mEndSample = MIN(clip->mStartSample + clip->mSample->LengthInSamples(), ArrangementController::mSampleLength);
}

float ClipArranger::MouseXToBufferPos(float mouseX)
{
   return (mouseX - BUFFER_MARGIN_X) / mBufferWidth;
}

int ClipArranger::MouseXToSample(float mouseX)
{
   return MouseXToBufferPos(mouseX) * ArrangementController::mSampleLength;
}

float ClipArranger::SampleToX(int sample)
{
   return float(sample) / ArrangementController::mSampleLength * mBufferWidth + BUFFER_MARGIN_X;
}

ClipArranger::Clip* ClipArranger::GetEmptyClip()
{
   for (int i = 0; i < MAX_CLIPS; ++i)
   {
      if (mClips[i].mSample == nullptr)
         return &mClips[i];
   }
   return nullptr;
}

void ClipArranger::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void ClipArranger::ButtonClicked(ClickButton* button)
{
}

void ClipArranger::CheckboxUpdated(Checkbox* checkbox)
{
}

void ClipArranger::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ClipArranger::SetUpFromSaveData()
{
}

void ClipArranger::Clip::Process(float* left, float* right, int bufferSize)
{
   if (mSample == nullptr)
      return;

   for (int i = 0; i < bufferSize; ++i)
   {
      int playhead = ArrangementController::mPlayhead + i;

      if (playhead >= mStartSample &&
          playhead < mEndSample)
      {
         int clipPos = playhead - mStartSample;
         int samplePos = clipPos % mSample->LengthInSamples();
         left[i] += mSample->Data()->GetChannel(0)[samplePos];
         right[i] += mSample->Data()->GetChannel(1)[samplePos];
      }
   }
}
