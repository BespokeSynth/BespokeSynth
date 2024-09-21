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
//  NoteSinger.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/23/13.
//
//

#pragma once

#include "IAudioReceiver.h"
#include "INoteSource.h"
#include "Slider.h"
#include "IDrawableModule.h"
#include "RadioButton.h"
#include "ClickButton.h"
#include "Transport.h"
#include "BiquadFilter.h"
#include "PeakTracker.h"
#include "Scale.h"

#define NOTESINGER_MAX_BUCKETS 40

class NoteSinger : public IAudioReceiver, public INoteSource, public IIntSliderListener, public IFloatSliderListener, public IDrawableModule, public IRadioButtonListener, public IButtonListener, public IAudioPoller, public IScaleListener
{
public:
   NoteSinger();
   ~NoteSinger();
   static IDrawableModule* Create() { return new NoteSinger(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IScaleListener
   void OnScaleChanged() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override {}
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 100;
      height = 50;
   }

   int GetPitchForBucket(int bucket);

   int mOctave{ 0 };
   IntSlider* mOctaveSlider{ nullptr };

   int mPitch{ 0 };

   float* mWorkBuffer{ nullptr };

   int mNumBuckets{ 28 };
   BiquadFilter mBands[NOTESINGER_MAX_BUCKETS];
   PeakTracker mPeaks[NOTESINGER_MAX_BUCKETS];
};
