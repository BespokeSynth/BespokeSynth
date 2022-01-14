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

#include <iostream>
#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Transport.h"
#include "Slider.h"

class IAudioSource;

class MidiClockOut : public IDrawableModule, public IDropdownListener, public IAudioPoller, public IFloatSliderListener
{
public:
   MidiClockOut();
   virtual ~MidiClockOut();
   static IDrawableModule* Create() { return new MidiClockOut(); }
   
   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void InitDevice();
   void BuildDeviceList();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   
   float mWidth;
   float mHeight;
   
   int mDeviceIndex{-1};
   DropdownList* mDeviceList;
   
   MidiDevice mDevice;
};
