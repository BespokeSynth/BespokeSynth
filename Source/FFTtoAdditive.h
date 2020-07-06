//
//  FFTtoAdditive.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/24/13.
//
//

#ifndef __modularSynth__FFTtoAdditive__
#define __modularSynth__FFTtoAdditive__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "GateEffect.h"
#include "BiquadFilterEffect.h"

#define VIZ_WIDTH 1000
#define RAZOR_HISTORY 100

class FFTtoAdditive : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   FFTtoAdditive();
   virtual ~FFTtoAdditive();
   static IDrawableModule* Create() { return new FFTtoAdditive(); }
   
   string GetTitleLabel() override { return "fft to additive"; }
   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   //IButtonListener
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:

   void DrawViz();
   float SinSample(float phase);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=235; h=170; }
   bool Enabled() const override { return mEnabled; }

   FFTData mFFTData;
   
   float* mWindower;

   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;

   float mInputPreamp;
   float mValue1;
   float mVolume;
   FloatSlider* mInputSlider;
   FloatSlider* mValue1Slider;
   FloatSlider* mVolumeSlider;
   float mDryWet;
   FloatSlider* mDryWetSlider;
   float mValue2;
   FloatSlider* mValue2Slider;
   float mValue3;
   FloatSlider* mValue3Slider;
   float mPhaseOffset;
   FloatSlider* mPhaseOffsetSlider;

   float mPeakHistory[RAZOR_HISTORY][VIZ_WIDTH+1];
   int mHistoryPtr;
   float* mPhaseInc;
};

#endif /* defined(__modularSynth__FFTtoAdditive__) */

