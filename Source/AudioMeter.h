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
//  AudioMeter.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/18/15.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "PeakTracker.h"
#include "LevelMeterDisplay.h"

class AudioMeter : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   AudioMeter();
   virtual ~AudioMeter();
   static IDrawableModule* Create() { return new AudioMeter(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 120;
      h = 40;
   }

   float mLevel{ 0 };
   float mMaxLevel{ 1 };
   FloatSlider* mLevelSlider{ nullptr };
   PeakTracker mPeakTracker;
   float* mAnalysisBuffer{ nullptr };
   int mNumChannels{ 1 };
   LevelMeterDisplay mLevelMeterDisplay{};
};
