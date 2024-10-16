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
//  Pumper.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#pragma once

#include "IAudioEffect.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "LFO.h"

class Pumper : public IAudioEffect, public IDropdownListener, public IFloatSliderListener
{
public:
   Pumper();
   virtual ~Pumper();

   static IAudioEffect* Create() { return new Pumper(); }


   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "pumper"; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }
   double GetIntervalPos(double time);
   void SyncToAdsr();

   FloatSlider* mAmountSlider{ nullptr };
   FloatSlider* mLengthSlider{ nullptr };
   FloatSlider* mCurveSlider{ nullptr };
   FloatSlider* mAttackSlider{ nullptr };

   ::ADSR mAdsr;
   NoteInterval mInterval{ NoteInterval::kInterval_4n };
   DropdownList* mIntervalSelector{ nullptr };
   float mLastValue{ 0 };
   float mAmount{ 0 };
   float mLength{ 0 };
   float mAttack{ 0 };

   float mWidth{ 200 };
   float mHeight{ 20 };
};
