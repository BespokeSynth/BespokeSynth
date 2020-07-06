//
//  Vocoder.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/17/13.
//
//

#ifndef __modularSynth__Vocoder__
#define __modularSynth__Vocoder__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "FFT.h"
#include "RollingBuffer.h"
#include "Slider.h"
#include "GateEffect.h"
#include "BiquadFilterEffect.h"
#include "VocoderCarrierInput.h"

class Vocoder : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public VocoderBase, public IIntSliderListener
{
public:
   Vocoder();
   virtual ~Vocoder();
   static IDrawableModule* Create() { return new Vocoder(); }
   
   string GetTitleLabel() override { return "vocoder"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void SetCarrierBuffer(float* carrier, int bufferSize) override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=235; h=170; }
   bool Enabled() const override { return mEnabled; }

   FFTData mFFTData;
   
   float* mWindower;

   

   ::FFT mFFT;
   RollingBuffer mRollingInputBuffer;
   RollingBuffer mRollingOutputBuffer;

   float* mCarrierInputBuffer;
   RollingBuffer mRollingCarrierBuffer;
   FFTData mCarrierFFTData;

   float mInputPreamp;
   float mCarrierPreamp;
   float mVolume;
   FloatSlider* mInputSlider;
   FloatSlider* mCarrierSlider;
   FloatSlider* mVolumeSlider;
   float mDryWet;
   FloatSlider* mDryWetSlider;
   float mFricativeThresh;
   FloatSlider* mFricativeSlider;
   bool mFricDetected;
   float mWhisper;
   FloatSlider* mWhisperSlider;
   float mPhaseOffset;
   FloatSlider* mPhaseOffsetSlider;
   
   int mCut;
   IntSlider* mCutSlider;

   GateEffect mGate;
};


#endif /* defined(__modularSynth__Vocoder__) */

