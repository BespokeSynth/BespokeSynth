//
//  Slider.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#ifndef __modularSynth__Slider__
#define __modularSynth__Slider__

#include <iostream>
#include "IUIControl.h"
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
   virtual void FloatSliderUpdated(FloatSlider* slider, float oldVal) = 0;
};

class FloatSlider : public IUIControl, public ITextEntryListener, public IAudioPoller
{
public:
   FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, float* var, float min, float max, int digits = -1);
   FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, float* var, float min, float max, int digits = -1);
   void SetLabel(const char* label);
   void SetVar(float* var) { mVar = var; }
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   bool IsMouseDown() const { return mMouseDown; }
   void SetExtents(float min, float max) { mMin = min; mMax = max; }
   void Compute(int samplesIn = 0);
   void DisplayLFOControl();
   void DisableLFO();
   FloatSliderLFOControl* GetLFO() { return mLFOControl; }
   FloatSliderLFOControl* AcquireLFO();
   void MatchExtents(FloatSlider* slider);
   void SetRelative(bool relative) { mRelative = relative; }
   void SetClamped(bool clamped) { mClamped = clamped; }
   float GetMin() const { return mMin; }
   float GetMax() const { return mMax; }
   void SetMaxValueDisplay(string display) { mMaxValueDisplay = display; }
   void SetMinValueDisplay(string display) { mMinValueDisplay = display; }
   void SetLFO(FloatSliderLFOControl* lfo);
   void SetShowName(bool show) { mShowName = show; }
   void SetDimensions(int w, int h) { mWidth = w; mHeight = h; }
   void SetBezierControl(float control) { mBezierControl = control; }
   void SetModulator(IModulator* modulator) { mModulator = modulator; }
   float& GetModulatorMin() { return mModulatorMin; }
   float& GetModulatorMax() { return mModulatorMax; }
   void OnTransportAdvanced(float amount) override;
   
   void Init() override;
   
   enum Mode { kNormal, kLogarithmic, kSquare, kBezier };
   void SetMode(Mode mode) { mMode = mode; }
   Mode GetMode() const { return mMode; }
   
   bool CheckNeedsDraw() override;

   //IUIControl
   void SetFromMidiCC(float slider) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   string GetDisplayValue(float val) const override;
   float GetMidiValue() override;
   void GetRange(float& min, float& max) override { min = mMin; max = mMax; }
   void Double() override;
   void Halve() override;
   void ResetToOriginal() override;
   void Increment(float amount) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
   bool AttemptTextInput() override;
   void TextEntryComplete(TextEntry* entry) override;
   
   void UpdateTouching();
   bool mTouching;   //to be controlled with external checkbox for "relative" sliders
   
protected:
   ~FloatSlider();   //protected so that it can't be created on the stack
   
private:
   void OnClicked(int x, int y, bool right) override;
   void SetValueForMouse(int x, int y);
   float* GetModifyValue();
   float PosToVal(float pos, bool ignoreSmooth) const;
   float ValToPos(float val, bool ignoreSmooth) const;
   bool AdjustSmooth() const;
   void SmoothUpdated();
   
   int mWidth;
   int mHeight;
   float* mVar;
   float mMin;
   float mMax;
   float mModulatorMin;
   float mModulatorMax;
   bool mMouseDown;
   int mFineRefX;
   int mRefY;
   int mShowDigits;
   IFloatSliderListener* mOwner;
   FloatSliderLFOControl* mLFOControl;
   IModulator* mModulator;
   float mClampIntMin;
   float mClampIntMax;
   bool mRelative;
   float mRelativeOffset;
   bool mClamped;
   Mode mMode;
   float mOriginalValue;
   string mMinValueDisplay;
   string mMaxValueDisplay;
   bool mShowName;
   float mBezierControl;
   float mSmooth;
   float mSmoothTarget;
   Ramp mRamp;
   bool mIsSmoothing;
   bool mComputeHasBeenCalledOnce;
   double mLastComputeTime;
   
   float mLastDisplayedValue;
   
   TextEntry* mFloatEntry;
   char mEntryString[MAX_TEXTENTRY_LENGTH];
   
   bool mAllowMinMaxAdjustment;
   TextEntry* mMinEntry;
   TextEntry* mMaxEntry;
};

class IntSlider;

class IIntSliderListener
{
public:
   virtual ~IIntSliderListener() {}
   virtual void IntSliderUpdated(IntSlider* slider, int oldVal) = 0;
};

class IntSlider : public IUIControl, public ITextEntryListener
{
public:
   IntSlider(IIntSliderListener* owner, const char* label, int x, int y, int w, int h, int* var, int min, int max);
   IntSlider(IIntSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, int* var, int min, int max);
   void SetLabel(const char* label);
   void SetVar(int* var) { mVar = var; }
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override { mMouseDown = false; }
   bool IsMouseDown() const { return mMouseDown; }
   void SetExtents(int min, int max) { mMin = min; mMax = max; CalcSliderVal(); }
   void SetShowName(bool show) { mShowName = show; }
   void SetDimensions(int w, int h) { mWidth = w; mHeight = h; }
   
   void Init() override;
   
   bool CheckNeedsDraw() override;

   //IUIControl
   void SetFromMidiCC(float slider) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   float GetMidiValue() override;
   int GetNumValues() override { return mMax - mMin + 1; }
   string GetDisplayValue(float val) const override;
   void GetRange(int& min, int& max) { min = mMin; max = mMax; }
   void Increment(float amount) override;
   void ResetToOriginal() override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
   bool AttemptTextInput() override;
   void TextEntryComplete(TextEntry* entry) override;
   
protected:
   ~IntSlider();   //protected so that it can't be created on the stack
   
private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void SetValueForMouse(int x, int y);
   void CalcSliderVal();
   
   int mWidth;
   int mHeight;
   int* mVar;
   int mMin;
   int mMax;
   bool mMouseDown;
   int mOriginalValue;
   IIntSliderListener* mOwner;
   
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

#endif /* defined(__modularSynth__Slider__) */
