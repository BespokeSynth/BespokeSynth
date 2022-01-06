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
//  MidiClockIn.h
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
#include "Slider.h"

class IAudioSource;

class MidiClockIn : public IDrawableModule, public IDropdownListener, public MidiDeviceListener, public IFloatSliderListener, public IIntSliderListener
{
public:
   MidiClockIn();
   virtual ~MidiClockIn();
   static IDrawableModule* Create() { return new MidiClockIn(); }
   
   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void OnMidiNote(MidiNote& note) override {}
   void OnMidiControl(MidiControl& control) override {}
   void OnMidi(const juce::MidiMessage& message) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void InitDevice();
   void BuildDeviceList();
   float GetRoundedTempo();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   
   float mWidth;
   float mHeight;
   
   enum class TempoRoundMode
   {
      kNone,
      kWhole,
      kHalf,
      kQuarter,
      kTenth
   };

   static constexpr int kMaxHistory = 40;
   
   int mDeviceIndex{-1};
   DropdownList* mDeviceList;
   TempoRoundMode mTempoRoundMode{TempoRoundMode::kWhole};
   DropdownList* mTempoRoundModeList;
   float mStartOffsetMs{ 0 };
   FloatSlider* mStartOffsetMsSlider;
   int mSmoothAmount{ kMaxHistory/2 };
   IntSlider* mSmoothAmountSlider;
   
   MidiDevice mDevice;
   
   std::array<float, kMaxHistory> mTempoHistory;
   int mTempoIdx{-1};
   double mLastTimestamp{-1};
};
