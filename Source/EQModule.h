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

    EQModule.h
    Created: 2 Nov 2020 10:47:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "BiquadFilter.h"
#include "DropdownList.h"

class EQModule : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IDropdownListener
{
public:
   EQModule();
   virtual ~EQModule();
   static IDrawableModule* Create() { return new EQModule(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;
   void KeyPressed(int key, bool isRepeat) override;
   void MouseReleased() override;

   static const int kNumFFTBins = 1024 * 8;
   static const int kBinIgnore = 2;
   static const int kDrawYOffset = 100;

   float FreqForBin(int bin) { return (float(bin) / kNumFFTBins) * gSampleRate; };
   float PosForFreq(float freq) { return log2(freq / 20) / 10; };
   float FreqForPos(float pos) { return 20.0 * std::pow(2.0, pos * 10); };
   float PosForGain(float gain) { return .5f - gain / 30.0f; };
   float GainForPos(float pos) { return (.5f - pos) * 30; }

   float mWidth{ 825 };
   float mHeight{ 255 };

   float* mWindower{ nullptr };
   float* mSmoother{ nullptr };

   ::FFT mFFT;
   FFTData mFFTData;
   RollingBuffer mRollingInputBuffer;

   struct Filter
   {
      bool mEnabled{ false };
      std::array<BiquadFilter, 2> mFilter;
      Checkbox* mEnabledCheckbox{ nullptr };
      DropdownList* mTypeSelector{ nullptr };
      FloatSlider* mFSlider{ nullptr };
      FloatSlider* mGSlider{ nullptr };
      FloatSlider* mQSlider{ nullptr };
      bool mNeedToCalculateCoefficients{ true };

      bool UpdateCoefficientsIfNecessary();
   };

   std::array<Filter, 8> mFilters;
   int mHoveredFilterHandleIndex{ -1 };
   int mDragging{ false };
   std::array<float, 1024> mFrequencyResponse{};
   bool mNeedToUpdateFrequencyResponseGraph{ true };
   float mDrawGain{ 1 };
   bool mLiteCpuModulation{ true };
};