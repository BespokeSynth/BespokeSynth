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
//  PitchChorus.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "PitchShifter.h"
#include "INoteReceiver.h"
#include "Ramp.h"
#include "Checkbox.h"

class PitchChorus : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public INoteReceiver
{
public:
   PitchChorus();
   virtual ~PitchChorus();
   static IDrawableModule* Create() { return new PitchChorus(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 120;
      h = 22;
   }

   static const int kNumShifters = 5;

   struct PitchShifterVoice
   {
      PitchShifterVoice()
      : mShifter(1024)
      , mOn(false)
      , mPitch(-1)
      {}
      PitchShifter mShifter;
      bool mOn;
      Ramp mRamp;
      int mPitch;
   };

   float* mOutputBuffer;
   PitchShifterVoice mShifters[kNumShifters];
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
};
