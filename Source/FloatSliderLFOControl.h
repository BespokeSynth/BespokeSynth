//
//  FloatSliderLFOControl.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/13.
//
//

#ifndef __modularSynth__FloatSliderLFOControl__
#define __modularSynth__FloatSliderLFOControl__

#include <iostream>
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Transport.h"
#include "LFO.h"
#include "RadioButton.h"
#include "Slider.h"
#include "ADSR.h"
#include "ADSRDisplay.h"
#include "ClickButton.h"
#include "PatchCableSource.h"
#include "DropdownList.h"

#define NUM_GLOBAL_ADSRS 7

struct LFOSettings
{
   LFOSettings()
   : mInterval(kInterval_1n)
   , mLFOOffset(0)
   , mMin(0)
   , mMax(0)
   , mBias(0)
   , mAdd(0)
   {
   }
   
   NoteInterval mInterval;
   OscillatorType mOscType;
   float mLFOOffset;
   float mMin;
   float mMax;
   float mBias;
   float mAdd;
   
   void SaveState(FileStreamOut& out) const;
   void LoadState(FileStreamIn& in);
};

class FloatSliderLFOControl : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener
{
public:
   FloatSliderLFOControl();
   static IDrawableModule* Create() { return new FloatSliderLFOControl(); }
   void Delete() { delete this; }
   void DrawModule() override;

   const LFOSettings& GetSettings() { return mLFOSettings; }
   void Load(LFOSettings settings);
   float Value(int samplesIn = 0);
   bool Active() { return mEnabled; }
   float Min() { return mLFOSettings.mMin; }
   float Max() { return mLFOSettings.mMax; }
   void SetMin(float min) { mLFOSettings.mMin = min; }
   void SetMax(float max) { mLFOSettings.mMax = max; }
   float* MinPtr() { return &mLFOSettings.mMin; }
   float* MaxPtr() { return &mLFOSettings.mMax; }
   LFOSettings* GetLFOSettings() { return &mLFOSettings; }
   void SetEnabled(bool enabled) override {} //don't use this one
   void SetLFOEnabled(bool enabled);
   bool IsEnabled() const { return mEnabled; }
   void SetRate(NoteInterval rate);
   void UpdateFromSettings();
   void Reset() { mOwner = NULL; }
   void SetOwner(FloatSlider* owner);
   FloatSlider* GetOwner() { return mOwner; }
   bool Enabled() const override { return mEnabled; }
   bool HasTitleBar() const override { return mPinned; }
   string GetTitleLabel() override { return mPinned ? "lfo" : ""; }
   bool IsSaveable() override { return mPinned; }
   void CreateUIControls() override;
   bool IsPinned() const { return mPinned; }
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;

   static ADSR* GetADSRControl(int index) { return &sADSR[index]; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void GetModuleDimensions(int& width, int& height) override { width = 100; height = 170; }
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

protected:
   ~FloatSliderLFOControl();

private:
   enum LFOControlType
   {
      kLFOControlType_LFO,
      kLFOControlType_ADSR,
      kLFOControlType_Drawn
   };

   void UpdateVisibleControls();

   FloatSlider* mOwner;
   
   LFOSettings mLFOSettings;

   LFO mLFO;
   DropdownList* mIntervalSelector;
   DropdownList* mOscSelector;
   FloatSlider* mOffsetSlider;
   FloatSlider* mBiasSlider;
   FloatSlider* mMinSlider;
   FloatSlider* mMaxSlider;
   FloatSlider* mAddSlider;
   ClickButton* mPinButton;
   Checkbox* mEnableLFOCheckbox;

   static ADSR sADSR[NUM_GLOBAL_ADSRS];
   ADSRDisplay* mADSRDisplay;
   int mADSRIndex;
   RadioButton* mADSRSelector;
   bool mFlipADSR;
   Checkbox* mFlipADSRCheckbox;
   bool mPinned;

   LFOControlType mType;
   RadioButton* mTypeSelector;
   
   PatchCableSource* mSliderCable;
};

class LFOPool
{
public:
   static void Init();
   static void Shutdown();
   static FloatSliderLFOControl* GetLFO(FloatSlider* owner);
private:
#define LFO_POOL_SIZE 50
   static FloatSliderLFOControl* sLFOPool[LFO_POOL_SIZE];
   static int sNextLFOIndex;
   static bool sInitialized;
};

#endif /* defined(__modularSynth__FloatSliderLFOControl__) */

