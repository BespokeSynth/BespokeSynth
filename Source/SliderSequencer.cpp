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
//  SliderSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/25/14.
//
//

#include "SliderSequencer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"

SliderSequencer::SliderSequencer()
{
   for (int i = 0; i < 8; ++i)
      mSliderLines.push_back(new SliderLine(this, 10, 40 + i * 15, i));
}

void SliderSequencer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void SliderSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDivisionSlider = new IntSlider(this, "division", 10, 20, 100, 15, &mDivision, 1, 4);

   for (int i = 0; i < mSliderLines.size(); ++i)
      mSliderLines[i]->CreateUIControls();
}

SliderSequencer::~SliderSequencer()
{
   TheTransport->RemoveAudioPoller(this);

   for (int i = 0; i < mSliderLines.size(); ++i)
      delete mSliderLines[i];
}

double SliderSequencer::MeasurePos(double time)
{
   double pos = TheTransport->GetMeasurePos(time) * mDivision;
   while (pos > 1)
      pos -= 1;

   return pos;
}

void SliderSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(SliderSequencer);

   if (!mEnabled)
      return;

   ComputeSliders(0);

   double time = NextBufferTime(true);
   double current = MeasurePos(time);

   for (int i = 0; i < mSliderLines.size(); ++i)
   {
      if (mSliderLines[i]->mVelocity == 0)
         continue;

      if ((mSliderLines[i]->mPoint > mLastMeasurePos || mLastMeasurePos > current) && mSliderLines[i]->mPoint <= current)
      {
         double remainder = DoubleWrap(current - mSliderLines[i]->mPoint, 1);
         double remainderMs = TheTransport->MsPerBar() * remainder;
         PlayNoteOutput(NoteMessage(time - remainderMs, mSliderLines[i]->mPitch, mSliderLines[i]->mVelocity * 127));
         PlayNoteOutput(NoteMessage(time - remainderMs + TheTransport->GetDuration(kInterval_16n), mSliderLines[i]->mPitch, 0));
         mSliderLines[i]->mPlayTime = time;
      }

      mSliderLines[i]->mPlaying = mSliderLines[i]->mPlayTime + 100 > time;
   }

   mLastMeasurePos = current;
}

void SliderSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDivisionSlider->Draw();
   for (int i = 0; i < mSliderLines.size(); ++i)
      mSliderLines[i]->Draw();

   if (mEnabled)
   {
      ofPushStyle();
      ofSetLineWidth(1);
      ofSetColor(0, 255, 0);
      ofFill();
      ofRect(10 + 180 * MeasurePos(gTime), 40, 1, 120);
      ofPopStyle();
   }
}

void SliderSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void SliderSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void SliderSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void SliderSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SliderSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SliderSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


SliderLine::SliderLine(SliderSequencer* owner, int x, int y, int index)
: mX(x)
, mY(y)
, mOwner(owner)
, mIndex(index)
{
}

void SliderLine::CreateUIControls()
{
   mSlider = new FloatSlider(mOwner, ("time" + ofToString(mIndex)).c_str(), mX, mY, 180, 15, &mPoint, 0, 1);
   mVelocitySlider = new FloatSlider(mOwner, ("vel" + ofToString(mIndex)).c_str(), mX + 185, mY, 80, 15, &mVelocity, 0, .99f, 2);
   mNoteSelector = new TextEntry(mOwner, ("note" + ofToString(mIndex)).c_str(), mX + 270, mY, 4, &mPitch, 0, 127);
   mPlayingCheckbox = new Checkbox(mOwner, ("playing" + ofToString(mIndex)).c_str(), HIDDEN_UICONTROL, HIDDEN_UICONTROL, &mPlaying);
}

void SliderLine::Draw()
{
   ofPushStyle();
   ofFill();
   ofSetColor(255, 255, 0);
   if (mPlaying)
      ofRect(mX, mY, 10, 10);
   ofPopStyle();

   mSlider->Draw();
   mNoteSelector->Draw();
   mVelocitySlider->Draw();
}
