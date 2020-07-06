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
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
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
   void GetModuleDimensions(float& width, float& height) override { width = 180; height = 203; }
   bool Enabled() const override { return mEnabled; }

   
   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   FMVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   ADSRDisplay* mAdsrDisplayVol;
   FloatSlider* mPhaseOffsetSlider0;
   
   FloatSlider* mHarmSlider;
   ADSRDisplay* mAdsrDisplayHarm;
   FloatSlider* mModSlider;
   ADSRDisplay* mAdsrDisplayMod;
   int mHarmRatioBase;  //negative means 1/val
   float mHarmRatioTweak;
   DropdownList* mHarmRatioBaseDropdown;
   FloatSlider* mPhaseOffsetSlider1;
   
   FloatSlider* mHarmSlider2;
   ADSRDisplay* mAdsrDisplayHarm2;
   FloatSlider* mModSlider2;
   ADSRDisplay* mAdsrDisplayMod2;
   int mHarmRatioBase2;  //negative means 1/val
   float mHarmRatioTweak2;
   DropdownList* mHarmRatioBaseDropdown2;
   FloatSlider* mPhaseOffsetSlider2;

   ChannelBuffer mWriteBuffer;
};

#endif /* defined(__modularSynth__FMSynth__) */

