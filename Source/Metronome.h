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
//  Metronome.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#pragma once

#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"

class Metronome : public IAudioSource, public ITimeListener, public IDrawableModule, public IFloatSliderListener
{
public:
   Metronome();
   ~Metronome();
   static IDrawableModule* Create() { return new Metronome(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override {}

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;


   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 80;
      height = 35;
   }

   float mPhase{ 0 };
   float mPhaseInc{ 0 };


   EnvOscillator mOsc{ OscillatorType::kOsc_Sin };

   float mVolume{ .5 };
   FloatSlider* mVolumeSlider{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
