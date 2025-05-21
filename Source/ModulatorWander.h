/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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

    ModulatorWander.h
    Created: 16 Mar 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"
#include "PerlinNoise.h"
#include "IAudioPoller.h"

class PatchCableSource;

class ModulatorWander : public IDrawableModule, public IFloatSliderListener, public IModulator, public IAudioPoller
{
public:
   ModulatorWander();
   virtual ~ModulatorWander();
   static IDrawableModule* Create() { return new ModulatorWander(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }
   bool ShouldSuppressAutomaticOutputCable() override { return true; }

   void Init() override;
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void OnTransportAdvanced(double amount) override;

   //IModulator
   double Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateRange();
   double GetPerlin(double sampleOffset);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& w, double& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   double mWidth{ 100 };
   double mHeight{ 100 };

   PerlinNoise mPerlinNoise;
   int mPerlinSeed{ 0 };
   double mPerlinPos{ 0.0 };
   double mCenter{ 0.5 };
   FloatSlider* mCenterSlider{ nullptr };
   double mRange{ 1.0 };
   FloatSlider* mRangeSlider{ nullptr };
   double mSpeed{ 1.0 };
   FloatSlider* mSpeedSlider{ nullptr };
   double mBias{ 0.5 };
   FloatSlider* mBiasSlider{ nullptr };
};
