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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnMidiNote(MidiNote& note) override {}
   void OnMidiControl(MidiControl& control) override {}
   void OnMidi(const juce::MidiMessage& message) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void InitDevice();
   void BuildDeviceList();
   float GetRoundedTempo();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };

   enum class TempoRoundMode
   {
      kNone,
      kWhole,
      kHalf,
      kQuarter,
      kTenth
   };

   static constexpr int kMaxHistory = 40;

   int mDeviceIndex{ -1 };
   DropdownList* mDeviceList{ nullptr };
   TempoRoundMode mTempoRoundMode{ TempoRoundMode::kWhole };
   DropdownList* mTempoRoundModeList{ nullptr };
   float mStartOffsetMs{ 0 };
   FloatSlider* mStartOffsetMsSlider{ nullptr };
   int mSmoothAmount{ kMaxHistory / 2 };
   IntSlider* mSmoothAmountSlider{ nullptr };

   MidiDevice mDevice;

   std::array<float, kMaxHistory> mTempoHistory;
   int mTempoIdx{ -1 };
   double mLastTimestamp{ -1 };
};
