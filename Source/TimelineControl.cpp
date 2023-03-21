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
   mNumMeasuresEntry = new TextEntry(this, "length", -1, -1, 5, &mNumMeasures, 4, 2048);
   mResetButton = new ClickButton(this, "reset", -1, -1);
   mLoopCheckbox = new Checkbox(this, "loop", -1, -1, &mLoop);
   mDockCheckbox = new Checkbox(this, "dock", -1, -1, &mDock);
   mLoopStartSlider = new IntSlider(this, "loop start", -1, -1, GetSliderWidth(), 15, &mLoopStart, 0, mNumMeasures);
   mLoopEndSlider = new IntSlider(this, "loop end", -1, -1, GetSliderWidth(), 15, &mLoopEnd, 0, mNumMeasures);

   mNumMeasuresEntry->DrawLabel(true);
   mNumMeasuresEntry->PositionTo(mTimeSlider, kAnchor_Below);
   mResetButton->PositionTo(mNumMeasuresEntry, kAnchor_Right_Padded);
   mLoopCheckbox->PositionTo(mResetButton, kAnchor_Right_Padded);
   mDockCheckbox->PositionTo(mLoopCheckbox, kAnchor_Right_Padded);
   mLoopStartSlider->PositionTo(mNumMeasuresEntry, kAnchor_Below);
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

   if (Minimized())
      return;

   if (mDock)
   {
      float w, h;
      GetModuleDimensions(w, h);
      SetPosition(0, ofGetHeight() / GetOwningContainer()->GetDrawScale() - h);
      Resize(ofGetWidth() / GetOwningContainer()->GetDrawScale(), h);
   }

   mDockCheckbox->SetShowing(GetOwningContainer() == TheSynth->GetRootContainer() || GetOwningContainer() == TheSynth->GetUIContainer());

   mTimeSlider->Draw();
   mNumMeasuresEntry->Draw();
   mResetButton->Draw();
   mLoopCheckbox->Draw();
   mDockCheckbox->Draw();
   mLoopStartSlider->Draw();
   mLoopEndSlider->Draw();
}

void TimelineControl::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mLoop ? 74 : 38;
}

void TimelineControl::Resize(float width, float height)
{
   mWidth = width;
   mTimeSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopStartSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopEndSlider->SetDimensions(GetSliderWidth(), 15);
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

   if (checkbox == mDockCheckbox)
   {
      if (mDock && GetOwningContainer() == TheSynth->GetRootContainer())
      {
         TheSynth->GetUIContainer()->TakeModule(this);
         float w, h;
         GetModuleDimensions(w, h);
         Resize(ofGetWidth() / GetOwningContainer()->GetDrawScale(), h);
         gHoveredUIControl = nullptr;
      }

      if (!mDock && GetOwningContainer() == TheSynth->GetUIContainer())
      {
         TheSynth->GetRootContainer()->TakeModule(this);
         float w, h;
         GetModuleDimensions(w, h);
         Resize(ofGetWidth() / GetOwningContainer()->GetDrawScale(), h);
         SetPosition(-TheSynth->GetDrawOffset().x, -TheSynth->GetDrawOffset().y + ofGetHeight() / GetOwningContainer()->GetDrawScale() - h);
         gHoveredUIControl = nullptr;
      }
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

void TimelineControl::TextEntryComplete(TextEntry* entry)
{
   if (entry == mNumMeasuresEntry)
   {
      mNumMeasures = std::max(mNumMeasures, 4);
      mTimeSlider->SetExtents(0, mNumMeasures);
      mLoopStartSlider->SetExtents(0, mNumMeasures);
      mLoopEndSlider->SetExtents(0, mNumMeasures);
   }
}

void TimelineControl::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
      TheTransport->Reset();
}

void TimelineControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadFloat("width", moduleInfo, 390, 100, 99999, K(isTextField));

   SetUpFromSaveData();
}

void TimelineControl::SetUpFromSaveData()
{
   Resize(mModuleSaveData.GetFloat("width"), 0);
}

void TimelineControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
}
