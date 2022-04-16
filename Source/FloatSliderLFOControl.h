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
#include "ClickButton.h"
#include "PatchCableSource.h"
#include "DropdownList.h"
#include "IModulator.h"

struct LFOSettings
{
   LFOSettings()
   : mInterval(kInterval_1n)
   , mLFOOffset(0)
   , mBias(0)
   , mSpread(0)
   , mSoften(0)
   , mShuffle(0)
   , mFreeRate(1)
   , mLength(1)
   , mLowResMode(false)
   {
   }

   NoteInterval mInterval;
   OscillatorType mOscType;
   float mLFOOffset;
   float mBias;
   float mSpread;
   float mSoften;
   float mShuffle;
   float mFreeRate;
   float mLength;
   bool mLowResMode;

   void SaveState(FileStreamOut& out) const;
   void LoadState(FileStreamIn& in);
};

class FloatSliderLFOControl : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IModulator
{
public:
   FloatSliderLFOControl();
   static IDrawableModule* Create() { return new FloatSliderLFOControl(); }
   void Delete() { delete this; }
   void DrawModule() override;

   const LFOSettings& GetSettings() { return mLFOSettings; }
   void Load(LFOSettings settings);
   LFOSettings* GetLFOSettings() { return &mLFOSettings; }
   void SetEnabled(bool enabled) override {} //don't use this one
   void SetLFOEnabled(bool enabled);
   bool IsEnabled() const { return mEnabled; }
   void SetRate(NoteInterval rate);
   void UpdateFromSettings();
   void SetOwner(FloatSlider* owner);
   FloatSlider* GetOwner() { return mTarget; }
   bool Enabled() const override { return mEnabled; }
   bool HasTitleBar() const override { return mPinned; }

   bool IsSaveable() override { return mPinned; }
   void CreateUIControls() override;
   bool IsPinned() const { return mPinned; }
   void RandomizeSettings();
   bool InLowResMode() const { return mLFOSettings.mLowResMode; }
   bool HasSpecialDelete() const override { return true; }
   void DoSpecialDelete() override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool InitializeWithZeroRange() const override { return true; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

protected:
   virtual ~FloatSliderLFOControl();

private:
   void UpdateVisibleControls();
   float GetLFOValue(int samplesIn = 0, float forcePhase = -1);
   float GetTargetMin() const;
   float GetTargetMax() const;

   LFOSettings mLFOSettings;

   LFO mLFO;
   DropdownList* mIntervalSelector;
   DropdownList* mOscSelector;
   FloatSlider* mOffsetSlider;
   FloatSlider* mBiasSlider;
   FloatSlider* mSpreadSlider;
   FloatSlider* mSoftenSlider;
   FloatSlider* mShuffleSlider;
   FloatSlider* mFreeRateSlider;
   FloatSlider* mLengthSlider;
   ClickButton* mPinButton;
   Checkbox* mEnableLFOCheckbox;
   Checkbox* mLowResModeCheckbox;
   float mWidth;
   float mHeight;

   bool mPinned;
};

class LFOPool
{
public:
   static void Init();
   static void Shutdown();
   static FloatSliderLFOControl* GetLFO(FloatSlider* owner);

private:
#define LFO_POOL_SIZE 256
   static FloatSliderLFOControl* sLFOPool[LFO_POOL_SIZE];
   static int sNextLFOIndex;
   static bool sInitialized;
};

#endif /* defined(__modularSynth__FloatSliderLFOControl__) */
