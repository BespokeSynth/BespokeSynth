/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    DiscontinuitySmoother.h
    Created: 30 Sept 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioProcessor.h"
#include "Slider.h"
#include "Checkbox.h"
#include "SwitchAndRamp.h"

class DiscontinuitySmoother : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   DiscontinuitySmoother();
   virtual ~DiscontinuitySmoother();
   static IDrawableModule* Create() { return new DiscontinuitySmoother(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;

   float mThreshold{ 0.1f };
   FloatSlider* mThresholdSlider{ nullptr };
   float mRampLength{ 200 };
   FloatSlider* mRampLengthSlider{ nullptr };
   SwitchAndRamp mSwitchAndRamp;

   float mLastSample[ChannelBuffer::kMaxNumChannels]{ 0 };
   RollingBuffer mDrawOffsetBuffer;
   double mLastDiscontinuityTime{ 0 };
};
