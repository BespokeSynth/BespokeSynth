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
//  Ramper.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#include "Ramper.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

Ramper::Ramper()
: mLength(kInterval_1n)
, mLengthSelector(nullptr)
, mControlCable(nullptr)
, mTriggerButton(nullptr)
, mStartMeasure(0)
, mStartValue(0)
, mRamping(false)
, mTargetValue(0)
, mTargetValueSlider(nullptr)
{
}

void Ramper::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

Ramper::~Ramper()
{
   TheTransport->RemoveAudioPoller(this);
}

void Ramper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLengthSelector = new DropdownList(this, "length", 3, 3, (int*)(&mLength));
   mTriggerButton = new ClickButton(this, "start", 67, 3);
   mTargetValueSlider = new FloatSlider(this, "target", 3, 20, 94, 15, &mTargetValue, 0, 1);

   mControlCable = new PatchCableSource(this, kConnectionType_ValueSetter);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);

   mLengthSelector->AddLabel("64", kInterval_64);
   mLengthSelector->AddLabel("32", kInterval_32);
   mLengthSelector->AddLabel("16", kInterval_16);
   mLengthSelector->AddLabel("8", kInterval_8);
   mLengthSelector->AddLabel("4", kInterval_4);
   mLengthSelector->AddLabel("2", kInterval_2);
   mLengthSelector->AddLabel("1n", kInterval_1n);
   mLengthSelector->AddLabel("2n", kInterval_2n);
   mLengthSelector->AddLabel("4n", kInterval_4n);
   mLengthSelector->AddLabel("4nt", kInterval_4nt);
   mLengthSelector->AddLabel("8n", kInterval_8n);
   mLengthSelector->AddLabel("8nt", kInterval_8nt);
   mLengthSelector->AddLabel("16n", kInterval_16n);
   mLengthSelector->AddLabel("16nt", kInterval_16nt);
   mLengthSelector->AddLabel("32n", kInterval_32n);
   mLengthSelector->AddLabel("64n", kInterval_64n);
}

void Ramper::OnTransportAdvanced(float amount)
{
   if (mRamping)
   {
      float curMeasure = TheTransport->GetMeasure(gTime) + TheTransport->GetMeasurePos(gTime);
      float measureProgress = curMeasure - mStartMeasure;
      float length = TheTransport->GetDuration(mLength) / TheTransport->MsPerBar();
      float progress = measureProgress / length;
      if (progress >= 0 && progress < 1)
      {
         for (auto* control : mUIControls)
         {
            if (control != nullptr)
               control->SetValue(ofLerp(mStartValue, mTargetValue, progress));
         }
      }
      else if (progress >= 1)
      {
         mRamping = false;
      }
   }
}

void Ramper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mLengthSelector->Draw();
   mTriggerButton->Draw();
   mTargetValueSlider->Draw();
}

void Ramper::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void Ramper::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool Ramper::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   return false;
}

void Ramper::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (size_t i = 0; i < mUIControls.size(); ++i)
   {
      if (i < mControlCable->GetPatchCables().size())
      {
         mUIControls[i] = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[i]->GetTarget());
         if (i == 0)
         {
            FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(mUIControls[i]);
            if (floatSlider)
               mTargetValueSlider->MatchExtents(floatSlider);
         }
      }
      else
      {
         mUIControls[i] = nullptr;
      }
   }
}

void Ramper::Go(double time)
{
   if (mUIControls[0] != nullptr)
   {
      mStartValue = mUIControls[0]->GetValue();
      mStartMeasure = TheTransport->GetMeasureTime(time);
      mRamping = true;
   }
}

void Ramper::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled)
      Go(time);
}

void Ramper::ButtonClicked(ClickButton* button)
{
   if (button == mTriggerButton)
      Go(gTime);
}

void Ramper::GetModuleDimensions(float& width, float& height)
{
   width = 100;
   height = 38;
}

void Ramper::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void Ramper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Ramper::SetUpFromSaveData()
{
}
