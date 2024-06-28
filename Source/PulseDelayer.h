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

    PulseDelayer.h
    Created: 15 Feb 2020 2:53:22pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"

class PulseDelayer : public IDrawableModule, public IPulseSource, public IPulseReceiver, public IFloatSliderListener, public IAudioPoller
{
public:
   PulseDelayer();
   ~PulseDelayer();
   static IDrawableModule* Create() { return new PulseDelayer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   struct PulseInfo
   {
      float mVelocity{ 0 };
      int mFlags{ 0 };
      double mTriggerTime{ 0 };
   };

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 108;
      height = 22;
   }

   float mDelay{ .25 };
   FloatSlider* mDelaySlider{ nullptr };

   float mLastPulseTime{ 0 };

   static const int kQueueSize = 50;
   PulseInfo mInputPulses[kQueueSize]{};
   int mConsumeIndex{ 0 };
   int mAppendIndex{ 0 };
};
