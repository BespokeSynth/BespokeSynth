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
//  Vocoder.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/17/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "FFT.h"
#include "Slider.h"
#include "GateEffect.h"
#include "BiquadFilterEffect.h"
#include "VocoderCarrierInput.h"

#define VOCODER_WINDOW_SIZE 1024
#define FFT_FREQDOMAIN_SIZE VOCODER_WINDOW_SIZE / 2 + 1

class Vocoder : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public VocoderBase, public IIntSliderListener
{
public:
   Vocoder();
   virtual ~Vocoder();
   static IDrawableModule* Create() { return new Vocoder(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void SetCarrierBuffer(float* carrier, int bufferSize) override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 235;
      h = 170;
   }

   FFTData mFFTData{ VOCODER_WINDOW_SIZE, FFT_FREQDOMAIN_SIZE };

   float* mWindower{ nullptr };


   ::FFT mFFT{ VOCODER_WINDOW_SIZE };
   RollingBuffer mRollingInputBuffer{ VOCODER_WINDOW_SIZE };
   RollingBuffer mRollingOutputBuffer{ VOCODER_WINDOW_SIZE };

   float* mCarrierInputBuffer{ nullptr };
   RollingBuffer mRollingCarrierBuffer{ VOCODER_WINDOW_SIZE };
   FFTData mCarrierFFTData{ VOCODER_WINDOW_SIZE, FFT_FREQDOMAIN_SIZE };

   float mInputPreamp{ 1 };
   float mCarrierPreamp{ 1 };
   float mVolume{ 1 };
   FloatSlider* mInputSlider{ nullptr };
   FloatSlider* mCarrierSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };
   float mDryWet{ 1 };
   FloatSlider* mDryWetSlider{ nullptr };
   float mFricativeThresh{ .07 };
   FloatSlider* mFricativeSlider{ nullptr };
   bool mFricDetected{ false };
   float mWhisper{ 0 };
   FloatSlider* mWhisperSlider{ nullptr };
   float mPhaseOffset{ 0 };
   FloatSlider* mPhaseOffsetSlider{ nullptr };

   int mCut{ 1 };
   IntSlider* mCutSlider{ nullptr };

   GateEffect mGate;

   bool mCarrierDataSet{ false };
};
