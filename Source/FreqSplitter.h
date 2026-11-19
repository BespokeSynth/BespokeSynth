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

    FreqSplitter.h
    Created: Nov 18 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "PatchCableSource.h"
#include "Slider.h"
#include "RadioButton.h"
#include "BiquadFilter.h"

class FreqSplitter : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IRadioButtonListener
{
public:
   FreqSplitter();
   virtual ~FreqSplitter();
   static IDrawableModule* Create() { return new FreqSplitter(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   bool CanMinimize() override { return false; }

   //IAudioSource
   void Process(double time) override;
   int GetNumTargets() override { return 2; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;

   void UpdateFilter();

   FloatSlider* mFSlider{ nullptr };
   FloatSlider* mQSlider{ nullptr };
   RadioButton* mTypeSelector{ nullptr };

   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels];

   RollingBuffer mVizBuffer2;
   PatchCableSource* mPatchCableSource2{ nullptr };

   float mLowLevel{ 0.0f };
   float mHighLevel{ 0.0f };
};
