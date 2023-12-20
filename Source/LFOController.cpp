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
//  LFOController.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 10/22/13.
//
//

#include "LFOController.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"

LFOController* TheLFOController = nullptr;

LFOController::LFOController()
{
   assert(TheLFOController == nullptr);
   TheLFOController = this;
}

void LFOController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this, "interval", 5, 40, &dummy);
   mOscSelector = new DropdownList(this, "osc", 50, 40, &dummy);
   mMinSlider = new FloatSlider(this, "low", 5, 4, 120, 15, &dummy2, 0, 1);
   mMaxSlider = new FloatSlider(this, "high", 5, 22, 120, 15, &dummy2, 0, 1);
   mBindButton = new ClickButton(this, "bind", 5, 60);

   mIntervalSelector->AddLabel("free", kInterval_Free);
   mIntervalSelector->AddLabel("64", kInterval_64);
   mIntervalSelector->AddLabel("32", kInterval_32);
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("2nt", kInterval_2nt);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);

   mOscSelector->AddLabel("sin", kOsc_Sin);
   mOscSelector->AddLabel("saw", kOsc_Saw);
   mOscSelector->AddLabel("-saw", kOsc_NegSaw);
   mOscSelector->AddLabel("squ", kOsc_Square);
   mOscSelector->AddLabel("tri", kOsc_Tri);
   mOscSelector->AddLabel("rand", kOsc_Random);
}

LFOController::~LFOController()
{
   assert(TheLFOController == this || TheLFOController == nullptr);
   TheLFOController = nullptr;
}

bool LFOController::WantsBinding(FloatSlider* slider)
{
   if (slider == mMinSlider || slider == mMaxSlider)
      return false;
   return mWantBind;
}

void LFOController::SetSlider(FloatSlider* slider)
{
   mSlider = slider;
   mLFO = slider->AcquireLFO();

   if (mLFO == nullptr)
      return;

   LFOSettings* lfoSettings = mLFO->GetLFOSettings();
   assert(lfoSettings);

   mMinSlider->MatchExtents(slider);
   mMaxSlider->MatchExtents(slider);
   mMinSlider->SetVar(&mLFO->GetMin());
   mMaxSlider->SetVar(&mLFO->GetMax());
   mIntervalSelector->SetVar((int*)(&lfoSettings->mInterval));
   mOscSelector->SetVar((int*)(&lfoSettings->mOscType));
   mLFO->SetEnabled(true);

   mStopBindTime = gTime + 1000;
}

void LFOController::Poll()
{
   if (mWantBind && mStopBindTime != -1 && gTime > mStopBindTime)
   {
      mWantBind = false;
      mStopBindTime = -1;
   }
}

void LFOController::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mIntervalSelector->Draw();
   mOscSelector->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mBindButton->Draw();

   if (mSlider && mLFO && mLFO->IsEnabled())
   {
      DrawTextNormal(mSlider->Name(), 50, 70);
      DrawConnection(mSlider);
   }
}

void LFOController::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (mLFO == nullptr)
      return;

   mLFO->UpdateFromSettings();
}

void LFOController::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (mLFO == nullptr)
      return;

   LFOSettings* lfoSettings = mLFO->GetLFOSettings();
   assert(lfoSettings);

   if (slider == mMinSlider)
      mLFO->GetMax() = MAX(mLFO->GetMax(), mLFO->GetMin());
   if (slider == mMaxSlider)
      mLFO->GetMin() = MIN(mLFO->GetMax(), mLFO->GetMin());
}

void LFOController::ButtonClicked(ClickButton* button, double time)
{
   if (button == mBindButton)
      mWantBind = true;
}
