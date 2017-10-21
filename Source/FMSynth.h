//
//  FMSynth.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#ifndef __modularSynth__FMSynth__
#define __modularSynth__FMSynth__

#include <iostream>
#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "FMVoice.h"
#include "ADSR.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"

class FMSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener
{
public:
   FMSynth();
   ~FMSynth();
   static IDrawableModule* Create() { return new FMSynth(); }
   
   string GetTitleLabel() override { return "FM"; }
   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void UpdateHarmonicRatio();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 270; height = 64; }
   bool Enabled() const override { return mEnabled; }

   
   PolyphonyMgr mPolyMgr;
   FMVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   FloatSlider* mHarmSlider;
   FloatSlider* mModSlider;
   int mHarmRatioBase;  //negative means 1/val
   float mHarmRatioTweak;
   DropdownList* mHarmRatioBaseDropdown;
   ADSRDisplay* mAdsrDisplayOsc;
   ADSRDisplay* mAdsrDisplayHarm;
   ADSRDisplay* mAdsrDisplayMod;

   float* mWriteBuffer;
};

#endif /* defined(__modularSynth__FMSynth__) */

