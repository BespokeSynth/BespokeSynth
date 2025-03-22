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

    AudioSend.h
    Created: 22 Oct 2017 1:23:39pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "PatchCableSource.h"
#include "Slider.h"

class AudioSend : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   AudioSend();
   virtual ~AudioSend();
   static IDrawableModule* Create() { return new AudioSend(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetSend(float amount, bool crossfade)
   {
      mAmount = amount;
      mCrossfade = crossfade;
   }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   int GetNumTargets() override { return 2; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 86;
      h = 38;
   }

   bool mCrossfade{ false };
   Checkbox* mCrossfadeCheckbox{ nullptr };
   float mAmount{ 0 };
   FloatSlider* mAmountSlider{ nullptr };
   RollingBuffer mVizBuffer2;
   PatchCableSource* mPatchCableSource2{ nullptr };
};
