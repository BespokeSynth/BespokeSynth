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
   
   std::string GetTitleLabel() override { return "pulse delayer"; }
   void CreateUIControls() override;
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;
   
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   struct PulseInfo
   {
      float mVelocity;
      int mFlags;
      double mTriggerTime;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   float mDelay;
   FloatSlider* mDelaySlider;
   
   float mLastPulseTime;
   
   static const int kQueueSize = 50;
   PulseInfo mInputPulses[kQueueSize];
   int mConsumeIndex;
   int mAppendIndex;
};
