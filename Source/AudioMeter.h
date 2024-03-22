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

#ifndef __Bespoke__AudioMeter__
#define __Bespoke__AudioMeter__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "PeakTracker.h"

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

   void CheckboxUpdated(Checkbox* checkbox, double time) override {}

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

   float mWidth{ 120 };
   float mHeight{ 62 };
   float mLevel{ 0 };
   float mMaxLevel{ 1 };
   FloatSlider* mLevelSlider{ nullptr };
   bool mVUMode{ false };
   Checkbox* mVUCheckbox{ nullptr };
   PeakTracker mPeakTracker;
   float* mAnalysisBuffer{ nullptr };
   float mLimit{ 1 };
   int mNumChannels{ 1 };

   struct LevelMeter
   {
      PeakTracker mPeakTracker;
      PeakTracker mPeakTrackerSlow;
   };

   std::array<LevelMeter, 2> mLevelMeters;
};

#endif /* defined(__Bespoke__AudioMeter__) */
