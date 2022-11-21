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
//  TimelineControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#include "TimelineControl.h"
#include "Transport.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

TimelineControl::TimelineControl()
{
}

void TimelineControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTimeSlider = new FloatSlider(this, "measure", 3, 3, GetSliderWidth(), 15, &mTime, 0, mNumMeasures);
   mLoopCheckbox = new Checkbox(this, "loop", -1, -1, &mLoop);
   mLoopStartSlider = new IntSlider(this, "loop start", -1, -1, GetSliderWidth(), 15, &mLoopStart, 0, mNumMeasures);
   mLoopEndSlider = new IntSlider(this, "loop end", -1, -1, GetSliderWidth(), 15, &mLoopEnd, 0, mNumMeasures);

   mLoopCheckbox->PositionTo(mTimeSlider, kAnchor_Right);
   mLoopStartSlider->PositionTo(mTimeSlider, kAnchor_Below);
   mLoopEndSlider->PositionTo(mLoopStartSlider, kAnchor_Below);

   mLoopStartSlider->SetShowing(mLoop);
   mLoopEndSlider->SetShowing(mLoop);
}

TimelineControl::~TimelineControl()
{
}

void TimelineControl::DrawModule()
{
   mTime = TheTransport->GetMeasureTime(gTime);

   if (Minimized() || IsVisible() == false)
      return;

   mTimeSlider->Draw();
   mLoopCheckbox->Draw();
   mLoopStartSlider->Draw();
   mLoopEndSlider->Draw();
}

void TimelineControl::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mLoop ? 57 : 21;
}

void TimelineControl::Resize(float width, float height)
{
   mWidth = width;
   mTimeSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopStartSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopEndSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopCheckbox->PositionTo(mTimeSlider, kAnchor_Right);
   mLoopStartSlider->PositionTo(mTimeSlider, kAnchor_Below);
   mLoopEndSlider->PositionTo(mLoopStartSlider, kAnchor_Below);
}

void TimelineControl::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mLoopCheckbox)
   {
      mLoopStartSlider->SetShowing(mLoop);
      mLoopEndSlider->SetShowing(mLoop);
      if (mLoop)
         TheTransport->SetLoop(mLoopStart, mLoopEnd);
      else
         TheTransport->ClearLoop();
   }
}

void TimelineControl::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mTimeSlider)
   {
      TheTransport->SetMeasureTime(mTime);
   }
}

void TimelineControl::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLoopStartSlider || slider == mLoopEndSlider)
   {
      if (slider == mLoopStartSlider)
      {
         mLoopStart = MIN(mLoopStart, mNumMeasures - 1);
         mLoopEnd = MAX(mLoopEnd, mLoopStart + 1);
      }
      if (slider == mLoopEndSlider)
      {
         mLoopEnd = MAX(mLoopEnd, 1);
         mLoopStart = MIN(mLoopStart, mLoopEnd - 1);
      }
      if (mLoop)
         TheTransport->SetLoop(mLoopStart, mLoopEnd);
   }
}

void TimelineControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadFloat("width", moduleInfo, 390, 100, 99999, K(isTextField));
   mModuleSaveData.LoadInt("num_measures", moduleInfo, 32, 1, 1024, K(isTextField));

   SetUpFromSaveData();
}

void TimelineControl::SetUpFromSaveData()
{
   Resize(mModuleSaveData.GetFloat("width"), 0);
   mNumMeasures = mModuleSaveData.GetInt("num_measures");
   mTimeSlider->SetExtents(0, mNumMeasures);
   mLoopStartSlider->SetExtents(0, mNumMeasures);
   mLoopEndSlider->SetExtents(0, mNumMeasures);
}

void TimelineControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
}
