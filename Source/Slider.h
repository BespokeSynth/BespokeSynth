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
//  Slider.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#pragma once

#include <limits>
#include "TextEntry.h"
#include "Ramp.h"
#include "IAudioPoller.h"

class FloatSlider;
class FloatSliderLFOControl;
class IModulator;

class IFloatSliderListener
{
public:
   virtual ~IFloatSliderListener() {}
   virtual void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) = 0;
};

class FloatSlider : public IUIControl, public ITextEntryListener, public IAudioPoller
{
public:
   FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, float* var, float min, float max, int digits = -1);
   FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, float* var, float min, float max, int digits = -1);
   void SetVar(float* var) { mVar = var; }
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool IsMouseDown() const override { return mMouseDown; }
   void SetExtents(float min, float max)
   {
      mMin = min;
      mMax = max;
   }
   void Compute(int samplesIn = 0)
   {
      mComputeHasBeenCalledOnce = true; //mark this slider as one whose owner calls compute on it
      if (mIsSmoothing || mModulator != nullptr)
         DoCompute(samplesIn);
   }
   void DisplayLFOControl();
   void DisableLFO();
   FloatSliderLFOControl* GetLFO() { return mLFOControl; }
   FloatSliderLFOControl* AcquireLFO();
   void MatchExtents(FloatSlider* slider);
   void SetRelative(bool relative) { mRelative = relative; }
   void SetClamped(bool clamped) { mClamped = clamped; }
   float GetMin() const { return mMin; }
   float GetMax() const { return mMax; }
   void SetMaxValueDisplay(std::string display) { mMaxValueDisplay = display; }
   void SetMinValueDisplay(std::string display) { mMinValueDisplay = display; }
   void SetLFO(FloatSliderLFOControl* lfo);
   void SetShowName(bool show) { mShowName = show; }
   void SetDimensions(int w, int h)
   {
      mWidth = w;
      mHeight = h;
   }
   void SetBezierControl(float control) { mBezierControl = control; }
   void SetModulator(IModulator* modulator);
   IModulator* GetModulator() { return mModulator; }
   float& GetModulatorMin() { return mModulatorMin; }
   float& GetModulatorMax() { return mModulatorMax; }
   bool ModulatorUsesLiteralValue() const override { return true; }
   float GetModulationRangeMin() const override { return mMin; }
   float GetModulationRangeMax() const override { return mMax; }
   void OnTransportAdvanced(float amount) override;

   void Init() override;

   enum Mode
   {
      kNormal,
      kLogarithmic,
      kSquare,
      kBezier
   };
   void SetMode(Mode mode) { mMode = mode; }
   Mode GetMode() const { return mMode; }

   bool CheckNeedsDraw() override;

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   float GetValue() const override;
   std::string GetDisplayValue(float val) const override;
   float GetMidiValue() const override;
   void GetRange(float& min, float& max) override
   {
      min = mMin;
      max = mMax;
   }
   void Double() override;
   void Halve() override;
   void ResetToOriginal() override;
   void Poll() override;
   void Increment(float amount) override;

   float PosToVal(float pos, bool ignoreSmooth) const;
   float ValToPos(float val, bool ignoreSmooth) const;

   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   bool AttemptTextInput() override;
   void TextEntryComplete(TextEntry* entry) override;
   void TextEntryCancelled(TextEntry* entry) override;

   void UpdateTouching();
   bool mTouching{ false }; //to be controlled with external checkbox for "relative" sliders

protected:
   ~FloatSlider(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void SetValueForMouse(float x, float y);
   float* GetModifyValue();
   bool AdjustSmooth() const;
   void SmoothUpdated();
   void DoCompute(int samplesIn);

   int mWidth;
   int mHeight;
   float* mVar;
   float mMin;
   float mMax;
   float mModulatorMin;
   float mModulatorMax;
   bool mMouseDown{ false };
   int mFineRefX{ -999 };
   int mRefY{ -999 };
   int mShowDigits;
   IFloatSliderListener* mOwner{ nullptr };
   FloatSliderLFOControl* mLFOControl{ nullptr };
   IModulator* mModulator{ nullptr };
   bool mRelative{ false };
   float mRelativeOffset{ -999 };
   bool mClamped{ true };
   Mode mMode{ Mode::kNormal };
   float mOriginalValue{ 0 };
   std::string mMinValueDisplay{ "" };
   std::string mMaxValueDisplay{ "" };
   bool mShowName{ true };
   float mBezierControl{ 1 };
   float mSmooth{ 0 };
   float mSmoothTarget{ 0 };
   Ramp mRamp;
   bool mIsSmoothing{ false };
   bool mComputeHasBeenCalledOnce{ false };
   double mLastComputeTime{ 0 };
   int mLastComputeSamplesIn{ 0 };
   double* mLastComputeCacheTime;
   float* mLastComputeCacheValue;

   float mLastDisplayedValue{ std::numeric_limits<float>::max() };

   TextEntry* mFloatEntry{ nullptr };
   char mEntryString[MAX_TEXTENTRY_LENGTH]{};

   bool mAllowMinMaxAdjustment{ true };
   TextEntry* mMinEntry{ nullptr };
   TextEntry* mMaxEntry{ nullptr };
};

class IntSlider;

class IIntSliderListener
{
public:
   virtual ~IIntSliderListener() {}
   virtual void IntSliderUpdated(IntSlider* slider, int oldVal, double time) = 0;
};

class IntSlider : public IUIControl, public ITextEntryListener
{
public:
   IntSlider(IIntSliderListener* owner, const char* label, int x, int y, int w, int h, int* var, int min, int max);
   IntSlider(IIntSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, int* var, int min, int max);
   void SetVar(int* var) { mVar = var; }
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override { mMouseDown = false; }
   bool IsMouseDown() const override { return mMouseDown; }
   int GetMin() const { return mMin; }
   int GetMax() const { return mMax; }
   void SetExtents(int min, int max)
   {
      mMin = min;
      mMax = max;
      CalcSliderVal();
   }
   void SetShowName(bool show) { mShowName = show; }
   void SetDimensions(int w, int h)
   {
      mWidth = w;
      mHeight = h;
   }

   void Init() override;

   bool CheckNeedsDraw() override;

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   float GetValue() const override;
   float GetMidiValue() const override;
   void GetRange(float& min, float& max) override
   {
      min = mMin;
      max = mMax;
   }
   int GetNumValues() override { return mMax - mMin + 1; }
   bool ModulatorUsesLiteralValue() const override { return true; }
   float GetModulationRangeMin() const override { return mMin; }
   float GetModulationRangeMax() const override { return mMax; }
   std::string GetDisplayValue(float val) const override;
   void GetRange(int& min, int& max)
   {
      min = mMin;
      max = mMax;
   }
   void Double() override;
   void Halve() override;
   void Increment(float amount) override;
   void ResetToOriginal() override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   bool AttemptTextInput() override;
   void TextEntryComplete(TextEntry* entry) override;
   void TextEntryCancelled(TextEntry* entry) override;

protected:
   ~IntSlider(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void SetValueForMouse(float x, float y);
   void CalcSliderVal();

   int mWidth;
   int mHeight;
   int* mVar;
   int mMin;
   int mMax;
   bool mMouseDown;
   int mOriginalValue;
   IIntSliderListener* mOwner;
   int mFineRefX{ -999 };

   int mLastDisplayedValue;
   int mLastSetValue;
   float mSliderVal;
   bool mShowName;

   TextEntry* mIntEntry;
   char mEntryString[MAX_TEXTENTRY_LENGTH];

   bool mAllowMinMaxAdjustment;
   TextEntry* mMinEntry;
   TextEntry* mMaxEntry;
};
