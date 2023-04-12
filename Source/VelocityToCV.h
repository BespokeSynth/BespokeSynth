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

    VelocityToCV.h
    Created: 28 Nov 2017 9:43:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IModulator.h"
#include "Slider.h"
#include "Checkbox.h"

class PatchCableSource;

class VelocityToCV : public IDrawableModule, public INoteReceiver, public IModulator, public IFloatSliderListener
{
public:
   VelocityToCV();
   virtual ~VelocityToCV();
   static IDrawableModule* Create() { return new VelocityToCV(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IModulator
   virtual float Value(int samplesIn = 0) override;
   virtual bool Active() const override { return mEnabled; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 106;
      height = 17 * 3 + 2;
   }

   int mVelocity{ 0 };
   bool mPassZero{ false };
   Checkbox* mPassZeroCheckbox{ nullptr };
};
