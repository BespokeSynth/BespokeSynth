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
//  MidiClockOut.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/22.
//
//

#pragma once

#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Transport.h"
#include "Slider.h"
#include "ClickButton.h"

class IAudioSource;

class MidiClockOut : public IDrawableModule, public IDropdownListener, public IAudioPoller, public IFloatSliderListener, public IButtonListener
{
public:
   MidiClockOut();
   virtual ~MidiClockOut();
   static IDrawableModule* Create() { return new MidiClockOut(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void InitDevice();
   void BuildDeviceList();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };

   enum class ClockMultiplier
   {
      Quarter,
      Half,
      One,
      Two,
      Four
   };

   int mDeviceIndex{ -1 };
   DropdownList* mDeviceList{ nullptr };
   bool mClockStartQueued{ false };
   ClickButton* mStartButton{ nullptr };
   ClockMultiplier mMultiplier{ ClockMultiplier::One };
   DropdownList* mMultiplierSelector{ nullptr };

   MidiDevice mDevice{ nullptr };
};
