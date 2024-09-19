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

#include <iostream>
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
   virtual void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) = 0;
};

class FloatSlider : public IUIControl, public ITextEntryListener, public IAudioPoller
{
public:
   FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, double* var, double min, double max, int digits = -1);
   FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, double* var, double min, double max, int digits = -1);
   void SetVar(double* var) { mVar = var; }
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool IsMouseDown() const override { return mMouseDown; }
   void SetExtents(double min, double max)
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
   double GetMin() const { return mMin; }
   double GetMax() const { return mMax; }
   void SetMaxValueDisplay(std::string display) { mMaxValueDisplay = display; }
   void SetMinValueDisplay(std::string display) { mMinValueDisplay = display; }
   void SetLFO(FloatSliderLFOControl* lfo);
   void SetShowName(bool show) { mShowName = show; }
   void SetDimensions(int w, int h)
   {
      mWidth = w;
      mHeight = h;
   }
   void SetBezierControl(double control) { mBezierControl = control; }
   void SetModulator(IModulator* modulator);
   IModulator* GetModulator() { return mModulator; }
   double& GetModulatorMin() { return mModulatorMin; }
   double& GetModulatorMax() { return mModulatorMax; }
   bool ModulatorUsesLiteralValue() const override { return true; }
   double GetModulationRangeMin() const override { return mMin; }
   double GetModulationRangeMax() const override { return mMax; }
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
   void SetFromMidiCC(double slider, double time, bool setViaModulator) override;
   double GetValueForMidiCC(double slider) const override;
   void SetValue(double value, double time, bool forceUpdate = false) override;
   double GetValue() const override;
   std::string GetDisplayValue(double val) const override;
   double GetMidiValue() const override;
   void GetRange(double& min, double& max) override
   {
      min = mMin;
      max = mMax;
   }
   void Double() override;
   void Halve() override;
   void ResetToOriginal() override;
   void Poll() override;
   void Increment(double amount) override;

   double PosToVal(double pos, bool ignoreSmooth) const;
   double ValToPos(double val, bool ignoreSmooth) const;

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
   double* GetModifyValue();
   bool AdjustSmooth() const;
   void SmoothUpdated();
   void DoCompute(int samplesIn);

   int mWidth;
   int mHeight;
   double* mVar;
   double mMin;
   double mMax;
   double mModulatorMin;
   double mModulatorMax;
   bool mMouseDown{ false };
   int mFineRefX{ -999 };
   int mRefY{ -999 };
   int mShowDigits;
   IFloatSliderListener* mOwner{ nullptr };
   FloatSliderLFOControl* mLFOControl{ nullptr };
   IModulator* mModulator{ nullptr };
   bool mRelative{ false };
   double mRelativeOffset{ -999 };
   bool mClamped{ true };
   Mode mMode{ Mode::kNormal };
   double mOriginalValue{ 0 };
   std::string mMinValueDisplay{ "" };
   std::string mMaxValueDisplay{ "" };
   bool mShowName{ true };
   double mBezierControl{ 1 };
   double mSmooth{ 0 };
   double mSmoothTarget{ 0 };
   Ramp mRamp;
   bool mIsSmoothing{ false };
   bool mComputeHasBeenCalledOnce{ false };
   double mLastComputeTime{ 0 };
   int mLastComputeSamplesIn{ 0 };
   double* mLastComputeCacheTime;
   double* mLastComputeCacheValue;

   double mLastDisplayedValue{ std::numeric_limits<double>::max() };

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
   void SetFromMidiCC(double slider, double time, bool setViaModulator) override;
   double GetValueForMidiCC(double slider) const override;
   void SetValue(double value, double time, bool forceUpdate = false) override;
   double GetValue() const override;
   double GetMidiValue() const override;
   void GetRange(double& min, double& max) override
   {
      min = mMin;
      max = mMax;
   }
   int GetNumValues() override { return mMax - mMin + 1; }
   bool ModulatorUsesLiteralValue() const override { return true; }
   double GetModulationRangeMin() const override { return mMin; }
   double GetModulationRangeMax() const override { return mMax; }
   std::string GetDisplayValue(double val) const override;
   void GetRange(int& min, int& max)
   {
      min = mMin;
      max = mMax;
   }
   void Double() override;
   void Halve() override;
   void Increment(double amount) override;
   void ResetToOriginal() override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   bool AttemptTextInput() override;
   void TextEntryComplete(TextEntry* entry) override;
   void TextEntryCancelled(TextEntry* entry) override;

protected:
   ~IntSlider() override; //protected so that it can't be created on the stack

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
   bool mMouseDown{ false };
   int mOriginalValue{ 0 };
   IIntSliderListener* mOwner;

   int mLastDisplayedValue{ 0 };
   int mLastSetValue{ 0 };
   float mSliderVal{ 0 };
   bool mShowName{ true };

   TextEntry* mIntEntry{ nullptr };
   char mEntryString[MAX_TEXTENTRY_LENGTH]{ "" };

   bool mAllowMinMaxAdjustment{ true };
   TextEntry* mMinEntry{ nullptr };
   TextEntry* mMaxEntry{ nullptr };
};
