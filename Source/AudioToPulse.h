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

    AudioToPulse.h
    Created: 31 Mar 2021 10:18:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IPulseReceiver.h"
#include "Slider.h"

class PatchCableSource;

class AudioToPulse : public IDrawableModule, public IPulseSource, public IAudioProcessor, public IFloatSliderListener
{
public:
   AudioToPulse();
   virtual ~AudioToPulse();
   static IDrawableModule* Create() { return new AudioToPulse(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void Process(double time) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };

   FloatSlider* mThresholdSlider{ nullptr };
   FloatSlider* mReleaseSlider{ nullptr };
   float mPeak{ 0 };
   float mEnvelope{ 0 };
   float mThreshold{ 0.5 };
   float mRelease{ 150 };
   float mReleaseFactor{ 0.99 };
};
