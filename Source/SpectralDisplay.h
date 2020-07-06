/*
  ==============================================================================

    SpectralDisplay.h
    Created: 14 Nov 2019 10:39:24am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "FFT.h"
#include "RollingBuffer.h"

class SpectralDisplay : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   SpectralDisplay();
   virtual ~SpectralDisplay();
   static IDrawableModule* Create() { return new SpectralDisplay(); }
   
   string GetTitleLabel() override { return "spectrum"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;
   
   float* mWindower;
   float* mSmoother;

   ::FFT mFFT;
   FFTData mFFTData;
   RollingBuffer mRollingInputBuffer;
};

