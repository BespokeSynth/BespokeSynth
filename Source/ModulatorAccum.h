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

    ModulatorAccum.h
    Created: 2 Aug 2021 10:32:56pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"
#include "Transport.h"

class PatchCableSource;

class ModulatorAccum : public IDrawableModule, public IFloatSliderListener, public IModulator, public IAudioPoller
{
public:
   ModulatorAccum();
   virtual ~ModulatorAccum();
   static IDrawableModule* Create() { return new ModulatorAccum(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   FloatSlider* GetTarget() { return GetSliderTarget(); }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };
   float mValue{ 0 };
   float mVelocity{ 0 };

   FloatSlider* mValueSlider{ nullptr };
   FloatSlider* mVelocitySlider{ nullptr };
};
