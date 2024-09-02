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
/*
  ==============================================================================

    Sequencer.h
    Created: 17 Oct 2018 9:38:03pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "IPulseReceiver.h"

class PatchCableSource;

class Pulser : public IDrawableModule, public ITimeListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IAudioPoller, public IPulseSource
{
public:
   Pulser();
   virtual ~Pulser();
   static IDrawableModule* Create() { return new Pulser(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   float GetOffset();

   enum TimeMode
   {
      kTimeMode_Step,
      kTimeMode_Sync,
      kTimeMode_Downbeat,
      kTimeMode_Downbeat2,
      kTimeMode_Downbeat4,
      kTimeMode_Free,
      kTimeMode_Align,
      kTimeMode_Reset
   };

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };

   TimeMode mTimeMode{ TimeMode::kTimeMode_Step };
   DropdownList* mTimeModeSelector{ nullptr };

   bool mRandomStep{ false };
   Checkbox* mRandomStepCheckbox{ nullptr };

   bool mWaitingForDownbeat{ false };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };

   FloatSlider* mFreeTimeSlider{ nullptr };
   float mFreeTimeStep{ 30 };
   float mFreeTimeCounter{ 0 };
   int mResetLength{ 8 };
   IntSlider* mResetLengthSlider{ nullptr };
   int mCustomDivisor{ 8 };
   IntSlider* mCustomDivisorSlider{ nullptr };
   ClickButton* mRestartFreeTimeButton{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
