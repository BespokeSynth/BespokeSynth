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
//  FeedbackModule.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/1/16.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DelayEffect.h"

class PatchCableSource;

class FeedbackModule : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   FeedbackModule();
   virtual ~FeedbackModule();
   static IDrawableModule* Create() { return new FeedbackModule(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 115;
      h = 125;
   }

   DelayEffect mDelay;

   IAudioReceiver* mFeedbackTarget{ nullptr };
   PatchCableSource* mFeedbackTargetCable{ nullptr };
   RollingBuffer mFeedbackVizBuffer;
   float mSignalLimit{ 1 };
   double mGainScale[ChannelBuffer::kMaxNumChannels];
   FloatSlider* mSignalLimitSlider{ nullptr };
};
