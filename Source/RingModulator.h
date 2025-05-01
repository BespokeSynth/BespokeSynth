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
//  RingModulator.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Slider.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "INoteReceiver.h"

class RingModulator : public IAudioProcessor, public IDrawableModule, public IButtonListener, public IFloatSliderListener, public INoteReceiver
{
public:
   RingModulator();
   virtual ~RingModulator();
   static IDrawableModule* Create() { return new RingModulator(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 130;
      height = 68;
   }

   ChannelBuffer mDryBuffer;

   float mFreq{ 220 };
   float mDryWet{ 1 };
   float mVolume{ 1 };
   FloatSlider* mFreqSlider{ nullptr };
   FloatSlider* mDryWetSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };

   EnvOscillator mModOsc{ kOsc_Sin };
   float mPhase{ 0 };
   Ramp mFreqRamp;
   float mGlideTime{ 0 };
   FloatSlider* mGlideSlider{ nullptr };
};
