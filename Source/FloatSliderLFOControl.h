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

#pragma once

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
#include "IPulseReceiver.h"

struct LFOSettings
{
   NoteInterval mInterval{ kInterval_1n };
   OscillatorType mOscType{ kOsc_Sin };
   float mLFOOffset{ 0 };
   float mBias{ 0 };
   float mSpread{ 0 };
   float mSoften{ 0 };
   float mShuffle{ 0 };
   float mFreeRate{ 1 };
   float mLength{ 1 };
   float mMinValue{ 0 };
   float mMaxValue{ 1 };
   bool mLowResMode{ false };

   void SaveState(FileStreamOut& out) const;
   void LoadState(FileStreamIn& in);
};

class FloatSliderLFOControl : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IModulator, public IPulseReceiver
{
public:
   FloatSliderLFOControl();
   static IDrawableModule* Create() { return new FloatSliderLFOControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }
   void Delete() { delete this; }
   void DrawModule() override;

   const LFOSettings& GetSettings() { return mLFOSettings; }
   void Load(LFOSettings settings);
   LFOSettings* GetLFOSettings() { return &mLFOSettings; }
   void SetEnabled(bool enabled) override {} //don't use this one
   void SetLFOEnabled(bool enabled);
   bool IsEnabled() const override { return mEnabled; }
   void SetRate(NoteInterval rate);
   void UpdateFromSettings();
   void SetOwner(FloatSlider* owner);
   FloatSlider* GetOwner() { return mTargets[0].mSliderTarget; }
   bool HasTitleBar() const override { return mPinned; }

   bool IsSaveable() override { return mPinned; }
   void CreateUIControls() override;
   bool IsPinned() const { return mPinned; }
   void RandomizeSettings();
   bool InLowResMode() const { return mLFOSettings.mLowResMode; }
   bool HasSpecialDelete() const override { return true; }
   void DoSpecialDelete() override;
   bool DrawToPush2Screen() override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool InitializeWithZeroRange() const override { return true; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

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
   DropdownList* mIntervalSelector{ nullptr };
   DropdownList* mOscSelector{ nullptr };
   FloatSlider* mOffsetSlider{ nullptr };
   FloatSlider* mBiasSlider{ nullptr };
   FloatSlider* mSpreadSlider{ nullptr };
   FloatSlider* mSoftenSlider{ nullptr };
   FloatSlider* mShuffleSlider{ nullptr };
   FloatSlider* mFreeRateSlider{ nullptr };
   FloatSlider* mLengthSlider{ nullptr };
   ClickButton* mPinButton{ nullptr };
   Checkbox* mEnableLFOCheckbox{ nullptr };
   Checkbox* mLowResModeCheckbox{ nullptr };
   float mWidth{ 100 };
   float mHeight{ 20 };

   bool mPinned{ false };
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
