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
//  Slider.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#include "Slider.h"
#include "IDrawableModule.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "LFOController.h"
#include "FileStream.h"
#include "ModularSynth.h"
#include "IModulator.h"
#include "Push2Control.h"

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, float* var, float min, float max, int digits /* = -1 */)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mModulatorMin(min)
, mModulatorMax(max)
, mShowDigits(digits)
, mOwner(owner)
{
   assert(owner);
   SetName(label);
   SetPosition(x, y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   mLastComputeCacheTime = new double[gBufferSize];
   mLastComputeCacheValue = new float[gBufferSize];
}

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, float* var, float min, float max, int digits /* = -1 */)
: FloatSlider(owner, label, -1, -1, w, h, var, min, max, digits)
{
   PositionTo(anchor, anchorDir);
}

FloatSlider::~FloatSlider()
{
   TheTransport->RemoveAudioPoller(this);
}

void FloatSlider::Init()
{
   if (mVar)
      mOriginalValue = *mVar;

   TheTransport->AddAudioPoller(this);
}

FloatSliderLFOControl* FloatSlider::AcquireLFO()
{
   if (mLFOControl == nullptr)
   {
      if (GetParent() != TheSynth->GetTopModalFocusItem()) //popups don't get these
         SetLFO(LFOPool::GetLFO(this));
   }
   return mLFOControl;
}

void FloatSlider::SetLFO(FloatSliderLFOControl* lfo)
{
   if (lfo != mLFOControl)
   {
      SetModulator(lfo);
      mLFOControl = lfo;
   }
}

void FloatSlider::SetModulator(IModulator* modulator)
{
   if (modulator != mModulator)
   {
      IModulator* oldModulator = mModulator;
      mModulator = modulator;
      mLFOControl = nullptr;
      if (oldModulator != nullptr)
         oldModulator->OnRemovedFrom(this);
   }
}

void FloatSlider::Render()
{
   float normalWidth = mWidth;
   float normalHeight = mHeight;

   if (Push2Control::sDrawingPush2Display)
   {
      mWidth = 100;
      mHeight = 15;
   }

   mLastDisplayedValue = *mVar;

   ofPushStyle();

   ofColor color;
   ofColor textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX + 1, mY + 1, mWidth, mHeight);
   ofSetColor(color);
   ofRect(mX, mY, mWidth, mHeight);
   ofNoFill();

   bool showSmoothAdjustmentUI = AdjustSmooth() && (gHoveredUIControl == this || mSmooth > 0);

   if (mIsSmoothing && !showSmoothAdjustmentUI)
   {
      ofPushStyle();
      ofSetColor(255, 255, 0, gModuleDrawAlpha);
      float val = ofClamp(mSmoothTarget, mMin, mMax);
      float screenPos = mX + 1 + (mWidth - 2) * ValToPos(val, false);
      ofSetLineWidth(1);
      ofFill();
      ofCircle(screenPos, mY + mHeight / 2, 3);
      ofPopStyle();
   }

   float screenPos;
   if (mModulator && mModulator->Active() && !showSmoothAdjustmentUI)
   {
      screenPos = mX + 1 + (mWidth - 2) * ValToPos(*mVar, true);
      float lfomax = ofClamp(mModulator->GetMax(), mMin, mMax);
      float screenPosMax = mX + 1 + (mWidth - 2) * ValToPos(lfomax, true);
      float lfomin = ofClamp(mModulator->GetMin(), mMin, mMax);
      float screenPosMin = mX + 1 + (mWidth - 2) * ValToPos(lfomin, true);

      ofPushStyle();
      ofSetColor(0, 200, 0, gModuleDrawAlpha * .5f);
      ofFill();
      ofRect(screenPosMin, mY, screenPos - screenPosMin, mHeight, 1); //lfo bar
      ofPopStyle();

      ofPushStyle();
      ofSetColor(0, 255, 0, gModuleDrawAlpha);
      ofSetLineWidth(2);
      ofLine(screenPosMin, mY + 1, screenPosMin, mY + mHeight - 1); //min bar
      ofLine(screenPosMax, mY + 1, screenPosMax, mY + mHeight - 1); //max bar
      ofPopStyle();
   }
   else
   {
      ofPushStyle();
      if (*mVar >= mMin && *mVar <= mMax)
         ofSetColor(255, 0, 0, gModuleDrawAlpha);
      else
         ofSetColor(30, 30, 30, gModuleDrawAlpha);
      if (showSmoothAdjustmentUI)
         ofSetColor(255, 255, 0, gModuleDrawAlpha);
      float val = ofClamp(*mVar, mMin, mMax);
      screenPos = mX + 1 + (mWidth - 2) * ValToPos(val, false);
      ofSetLineWidth(2);
      ofLine(screenPos, mY + 1, screenPos, mY + mHeight - 1); //value bar
      ofPopStyle();
   }

   DrawBeacon(screenPos, mY + mHeight / 2);

   DrawHover(mX, mY, mWidth, mHeight);

   std::string display;
   float textSize = 13;
   if (showSmoothAdjustmentUI)
   {
      display = "smooth: " + ofToString(mSmooth, 3);
      textSize = 9;
   }
   else
   {
      if (mShowName)
         display = GetDisplayName();
      if (display.length() > 0) //only show a colon if there's a label
         display += ":";
      if (mFloatEntry)
      {
         ofSetColor(255, 255, 100);
         display += mFloatEntry->GetText();
      }
      else
      {
         display += GetDisplayValue(*mVar);
      }
   }

   if (mMaxEntry || mMinEntry)
      display = "";

   ofSetColor(textColor);
   DrawTextNormal(display, mX + 2, mY + 5 + mHeight / 2, textSize);

   ofPopStyle();

   if (mMaxEntry)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRectangle rect = mMaxEntry->GetRect(K(local));
      ofRect(rect.x - 28, rect.y, rect.width + 28, rect.height);
      mMaxEntry->Draw();
      ofSetColor(255, 255, 255);
      DrawTextRightJustify("max:", rect.x, rect.y + 5 + mHeight / 2);
      ofPopStyle();
   }
   if (mMinEntry)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRectangle rect = mMinEntry->GetRect(K(local));
      ofRect(rect.x - 28, rect.y, rect.width + 28, rect.height);
      mMinEntry->Draw();
      ofSetColor(255, 255, 255);
      DrawTextRightJustify("min:", rect.x, rect.y + 5 + mHeight / 2);
      ofPopStyle();
   }

   if (gHoveredUIControl == this && (GetKeyModifiers() & kModifier_Command) && mAllowMinMaxAdjustment && mMinEntry == nullptr && mMaxEntry == nullptr && !IUIControl::WasLastHoverSetManually())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRect(mX, mY, mWidth * .4f, mHeight);
      ofRect(mX + mWidth * .6f, mY, mWidth * .4f, mHeight);
      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(mX, mY, mWidth * .4f, mHeight);
      ofRect(mX + mWidth * .6f, mY, mWidth * .4f, mHeight);

      ofPushMatrix();
      ofClipWindow(mX, mY, mWidth * .4f, mHeight, true);
      DrawTextNormal(ofToString(mMin), mX + 2, mY + 4 + mHeight / 2, 10);
      ofPopMatrix();

      ofPushMatrix();
      ofClipWindow(mX + mWidth * .6f, mY, mWidth * .4f, mHeight, true);
      DrawTextRightJustify(ofToString(mMax), mX + mWidth - 2, mY + 4 + mHeight / 2, 10);
      ofPopMatrix();

      ofPopStyle();
   }

   mWidth = normalWidth;
   mHeight = normalHeight;
}

void FloatSlider::DisplayLFOControl()
{
   FloatSliderLFOControl* lfo = AcquireLFO();
   if (lfo)
   {
      if (lfo->IsPinned())
         return;

      float thisx, thisy;
      GetPosition(thisx, thisy);

      lfo->SetLFOEnabled(true);

      float w, h;
      lfo->GetDimensions(w, h);
      lfo->SetPosition(thisx, thisy + 15);
      lfo->SetOwningContainer(GetModuleParent()->GetOwningContainer());
      TheSynth->PushModalFocusItem(lfo);

      if (TheLFOController)
         TheLFOController->SetSlider(this);
   }
}

void FloatSlider::OnClicked(float x, float y, bool right)
{
   if (right)
   {
      DisplayLFOControl();
      return;
   }

   if ((GetKeyModifiers() & kModifier_Command) && mAllowMinMaxAdjustment && !IUIControl::WasLastHoverSetManually())
   {
      bool adjustMax;
      if (x > mWidth / 2)
         adjustMax = true;
      else
         adjustMax = false;

      if (adjustMax)
      {
         if (mMaxEntry != nullptr)
            mMaxEntry->Delete();
         mMaxEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMax, -FLT_MAX, FLT_MAX);
         mMaxEntry->MakeActiveTextEntry(true);
      }
      else
      {
         if (mMinEntry != nullptr)
            mMinEntry->Delete();
         //mMinEntry = new TextEntry(this, "", mX, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry->MakeActiveTextEntry(true);
      }

      return;
   }

   mFineRefX = ofMap(ValToPos(*GetModifyValue(), false), 0.0f, 1.0f, mX + 1, mX + mWidth - 1, true) - mX;
   mRefY = y;
   SetValueForMouse(x, y);
   mMouseDown = true;
   mTouching = true;
   UpdateTouching();
}

void FloatSlider::MouseReleased()
{
   if (mMouseDown)
      mTouching = false;
   mMouseDown = false;
   mRefY = -999;
   if (mRelative && (mModulator == nullptr || mModulator->Active() == false))
      SetValue(0, NextBufferTime(false));
}

bool FloatSlider::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   if (mMouseDown)
      SetValueForMouse(x, y);
   return mMouseDown;
}

void FloatSlider::SetValueForMouse(float x, float y)
{
   float* var = GetModifyValue();
   float fX = x;
   if (GetKeyModifiers() & kModifier_Shift)
   {
      if (mFineRefX == -999)
      {
         mFineRefX = x;
      }
      float precision = mShowDigits != -1 ? 100 : 10;
      fX = mFineRefX + (fX - mFineRefX) / precision;
   }
   else
   {
      mFineRefX = -999;
   }
   float oldVal = *var;
   float pos = ofMap(fX + mX, mX + 1, mX + mWidth - 1, 0.0f, 1.0f);

   if (AdjustSmooth())
   {
      mSmooth = PosToVal(pos, false);
      SmoothUpdated();
      return;
   }

   *var = PosToVal(pos, false);
   if (mRelative && (mModulator == nullptr || mModulator->Active() == false))
   {
      if (!mTouching || mRelativeOffset == -999)
      {
         mRelativeOffset = *var;
         *var = 0;
      }
      else
      {
         *var -= mRelativeOffset;
      }
   }
   *var = ofClamp(*var, mMin, mMax);

   if (oldVal != *var)
   {
      mOwner->FloatSliderUpdated(this, oldVal, NextBufferTime(false));
   }

   if (mModulator && mModulator->Active() && mModulator->CanAdjustRange())
   {
      float move = (y - mRefY) * -.003f;
      float change = move * (mMax - mMin);
      mModulator->GetMin() = ofClamp(mModulator->GetMin() + change, mMin, mModulator->GetMax());
      mRefY = y;
   }
}

bool FloatSlider::AdjustSmooth() const
{
   return (GetKeyModifiers() & kModifier_Alt) &&
          mComputeHasBeenCalledOnce; //no smoothing if we're not calling Compute()
}

void FloatSlider::SmoothUpdated()
{
   if (mSmooth > 0 && !mIsSmoothing)
   {
      mSmoothTarget = *mVar;
      mRamp.SetValue(mSmoothTarget);
      mIsSmoothing = true;
   }
   else if (mSmooth <= 0 && mIsSmoothing)
      mIsSmoothing = false;
}

void FloatSlider::SetFromMidiCC(float slider, double time, bool setViaModulator)
{
   SetValue(GetValueForMidiCC(slider), time);
}

float FloatSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider, 0, 1);
   return PosToVal(slider, true);
}

float FloatSlider::PosToVal(float pos, bool ignoreSmooth) const
{
   if (AdjustSmooth() && !ignoreSmooth)
   {
      if (pos < 0)
         return 0;
      if (pos > 1)
         return 1;
      return pos * pos;
   }
   if (pos < 0)
      return mMin;
   if (pos > 1)
      return mMax;
   if (mMode == kNormal)
      return mMin + pos * (mMax - mMin);
   if (mMode == kLogarithmic)
      return mMin * powf(mMax / mMin, pos);
   if (mMode == kSquare)
      return mMin + pos * pos * (mMax - mMin);
   if (mMode == kBezier)
   {
      float y = pos * (pos * (pos * (mMax - mMin) + 3 * mMin - 3 * mBezierControl) - 3 * mMin + 3 * mBezierControl) + mMin;
      return y;
   }
   assert(false);
   return 0;
}

float FloatSlider::ValToPos(float val, bool ignoreSmooth) const
{
   val = ofClamp(val, mMin, mMax);
   if (AdjustSmooth() && (gHoveredUIControl == this || mSmooth > 0) && !ignoreSmooth)
      return sqrtf(mSmooth);
   if (mMode == kNormal)
      return (val - mMin) / (mMax - mMin);
   if (mMode == kLogarithmic)
      return log(val / mMin) / log(mMax / mMin);
   if (mMode == kSquare)
      return sqrtf((val - mMin) / (mMax - mMin));
   if (mMode == kBezier)
   {
      float closest = 0;
      float closestDist = FLT_MAX;
      for (float pos = 0; pos < 1; pos += .001f)
      {
         float dist = fabsf(PosToVal(pos, true) - val);
         if (dist < closestDist)
         {
            closestDist = dist;
            closest = pos;
         }
      }
      return closest;
   }
   return 0;
}

void FloatSlider::SetValue(float value, double time, bool forceUpdate /*= false*/)
{
   if (TheLFOController && TheLFOController->WantsBinding(this))
   {
      TheLFOController->SetSlider(this);
      return;
   }

   float* var = GetModifyValue();
   float oldVal = *var;
   if (mRelative)
   {
      if (!mTouching || mRelativeOffset == -999)
      {
         mRelativeOffset = value;
         value = 0;
      }
      else
      {
         value -= mRelativeOffset;
      }
   }
   /*if (mClamped)
      *var = ofClamp(value,mMin,mMax);
   else*/
   *var = value;
   DisableLFO();
   if (oldVal != *var || forceUpdate)
   {
      mOwner->FloatSliderUpdated(this, oldVal, time);
   }
}

void FloatSlider::UpdateTouching()
{
   if (mRelative && (mModulator == nullptr || mModulator->Active() == false))
   {
      if (!mTouching)
         SetValue(0, NextBufferTime(false));
      mRelativeOffset = -999;
   }
}

void FloatSlider::MatchExtents(FloatSlider* slider)
{
   mMax = slider->mMax;
   mMin = slider->mMin;
}

void FloatSlider::DisableLFO()
{
   if (mLFOControl)
      mLFOControl->SetEnabled(false);
}

float FloatSlider::GetValue() const
{
   return *mVar;
}

float FloatSlider::GetMidiValue() const
{
   if (mMin == mMax)
      return 0;

   return ValToPos(*mVar, true);
}

std::string FloatSlider::GetDisplayValue(float val) const
{
   if (val == mMin && mMinValueDisplay != "")
      return mMinValueDisplay;
   if (val == mMax && mMaxValueDisplay != "")
      return mMaxValueDisplay;

   int decDigits = 3;
   if (mShowDigits != -1)
      decDigits = mShowDigits;
   else if (mMax - mMin > 1000)
      decDigits = 0;
   else if (mMax - mMin > 100)
      decDigits = 1;
   else if (mMax - mMin > 10)
      decDigits = 2;

   float displayVar = val;
   if (decDigits == 0) //round down if we're showing int value
      displayVar = (int)displayVar;
   return ofToString(displayVar, decDigits);
}

void FloatSlider::DoCompute(int samplesIn /*= 0*/)
{
   if (mLastComputeTime == gTime && mLastComputeSamplesIn == samplesIn)
      return; //we've just calculated this, no need to do it again! earlying out avoids wasted work and circular modulation loops

   if (mLFOControl && mLFOControl->Active() && mLFOControl->InLowResMode() && samplesIn != 0)
      return; //only do the math on Compute(0) for low res mode

   mLastComputeTime = gTime;
   mLastComputeSamplesIn = samplesIn;

   float oldVal = *mVar;

   const bool kUseCache = true;
   if (kUseCache && IsAudioThread() && samplesIn >= 0 && samplesIn < gBufferSize && mLastComputeCacheTime[samplesIn] == gTime)
   {
      *mVar = mLastComputeCacheValue[samplesIn];
   }
   else
   {
      if (mModulator && mModulator->Active())
      {
         if (mIsSmoothing)
            mSmoothTarget = mModulator->Value(samplesIn);
         else
            *mVar = mModulator->Value(samplesIn);
      }

      if (mIsSmoothing)
         *mVar = mRamp.Value(gTime + samplesIn * gInvSampleRateMs);

      if (IsAudioThread() && samplesIn >= 0 && samplesIn < gBufferSize && mLastComputeCacheTime[samplesIn] != gTime)
      {
         mLastComputeCacheValue[samplesIn] = *mVar;
         mLastComputeCacheTime[samplesIn] = gTime;
      }
   }

   if (oldVal != *mVar)
      mOwner->FloatSliderUpdated(this, oldVal, gTime + samplesIn * gInvSampleRateMs);
}

float* FloatSlider::GetModifyValue()
{
   if (!TheSynth->IsLoadingModule() && mModulator && mModulator->Active() && mModulator->CanAdjustRange())
      return &mModulator->GetMax();
   if (mIsSmoothing)
      return &mSmoothTarget;
   return mVar;
}

void FloatSlider::Double()
{
   float doubl = *GetModifyValue() * 2.0f;
   if (doubl >= mMin && doubl <= mMax)
      SetValue(doubl, NextBufferTime(false));
}

void FloatSlider::Halve()
{
   float half = *GetModifyValue() * .5f;
   if (half >= mMin && half <= mMax)
      SetValue(half, NextBufferTime(false));
}

void FloatSlider::Increment(float amount)
{
   float val = *GetModifyValue() + amount;
   if (val >= mMin && val <= mMax)
      SetValue(val, NextBufferTime(false));
}

void FloatSlider::ResetToOriginal()
{
   SetValue(mOriginalValue, NextBufferTime(false));
}

bool FloatSlider::CheckNeedsDraw()
{
   if (IUIControl::CheckNeedsDraw())
      return true;

   return *mVar != mLastDisplayedValue;
}

bool FloatSlider::AttemptTextInput()
{
   if (mFloatEntry)
      mFloatEntry->Delete();
   mFloatEntry = new TextEntry(this, "", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 10, mEntryString);
   mFloatEntry->MakeActiveTextEntry(true);
   mFloatEntry->SetRequireEnter(true);
   mFloatEntry->ClearInput();
   return true;
}

void FloatSlider::TextEntryComplete(TextEntry* entry)
{
   if (entry == mFloatEntry)
   {
      mFloatEntry->Delete();
      mFloatEntry = nullptr;

      float evaluated = 0;
      bool expressionValid = EvaluateExpression(mEntryString, *GetModifyValue(), evaluated);
      if (expressionValid && ((evaluated >= mMin && evaluated <= mMax) || (GetKeyModifiers() & kModifier_Shift)))
         SetValue(evaluated, NextBufferTime(false));
   }
   if (entry == mMaxEntry)
   {
      mMaxEntry->Delete();
      mMaxEntry = nullptr;
   }
   if (entry == mMinEntry)
   {
      mMinEntry->Delete();
      mMinEntry = nullptr;
   }
}

void FloatSlider::TextEntryCancelled(TextEntry* entry)
{
   if (entry == mFloatEntry)
   {
      mFloatEntry->Delete();
      mFloatEntry = nullptr;
   }
}

void FloatSlider::OnTransportAdvanced(float amount)
{
   if (mLastComputeTime + .1f < gTime)
      Compute();

   if (mIsSmoothing)
      mRamp.Start(gTime, mSmoothTarget, gTime + (amount * TheTransport->MsPerBar() * (mSmooth * 300)));
}

namespace
{
   const int kFloatSliderSaveStateRev = 6;
}

void FloatSlider::SaveState(FileStreamOut& out)
{
   out << kFloatSliderSaveStateRev;

   out << (float)*mVar;

   out << mModulatorMin;
   out << mModulatorMax;

   out << mSmooth;
   out << mSmoothTarget;
   out << mIsSmoothing;

   out << mMin;
   out << mMax;
   out << (int)mMode;

   bool hasLFO = mLFOControl && mLFOControl->Active();
   out << hasLFO;
   if (hasLFO)
      mLFOControl->SaveState(out);
}

void FloatSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;

   float var;
   in >> var;
   mRamp.SetValue(var);

   if (rev < 5)
   {
      bool hasLFO;
      in >> hasLFO;
      if (hasLFO)
      {
         FloatSliderLFOControl* lfo = AcquireLFO();
         if (shouldSetValue)
            lfo->SetLFOEnabled(true);

         if (rev == 0)
         {
            mLFOControl->GetLFOSettings()->LoadState(in);
         }
         else if (rev > 0)
         {
            mLFOControl->LoadState(in, mLFOControl->LoadModuleSaveStateRev(in));
         }
         if (shouldSetValue)
            lfo->UpdateFromSettings();
      }
   }

   if (rev >= 2)
   {
      in >> mModulatorMin;
      in >> mModulatorMax;
   }

   if (rev >= 3)
   {
      in >> mSmooth;
      in >> mSmoothTarget;
      in >> mIsSmoothing;
   }

   if (rev >= 4)
   {
      in >> mMin;
      in >> mMax;
   }

   if (rev >= 6)
   {
      int modeInt;
      in >> modeInt;
      mMode = (Mode)modeInt;
   }

   if (rev >= 5)
   {
      bool hasLFO;
      in >> hasLFO;
      if (hasLFO)
      {
         FloatSliderLFOControl* lfo = AcquireLFO();
         if (shouldSetValue)
            lfo->SetLFOEnabled(true);

         if (rev == 0)
         {
            mLFOControl->GetLFOSettings()->LoadState(in);
         }
         else if (rev > 0)
         {
            mLFOControl->LoadState(in, mLFOControl->LoadModuleSaveStateRev(in));
         }
         if (shouldSetValue)
            lfo->UpdateFromSettings();
      }
   }

   if (shouldSetValue && (mModulator == nullptr || !mModulator->Active()))
      SetValueDirect(var, gTime);
}

IntSlider::IntSlider(IIntSliderListener* owner, const char* label, int x, int y, int w, int h, int* var, int min, int max)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mMouseDown(false)
, mOwner(owner)
, mOriginalValue(0)
, mSliderVal(0)
, mShowName(true)
, mIntEntry(nullptr)
, mAllowMinMaxAdjustment(true)
, mMinEntry(nullptr)
, mMaxEntry(nullptr)
{
   assert(owner);
   SetName(label);
   SetPosition(x, y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   CalcSliderVal();
}

IntSlider::IntSlider(IIntSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, int* var, int min, int max)
: IntSlider(owner, label, -1, -1, w, h, var, min, max)
{
   PositionTo(anchor, anchorDir);
}

IntSlider::~IntSlider()
{
}

void IntSlider::Init()
{
   if (mVar)
      mOriginalValue = *mVar;
}

void IntSlider::Poll()
{
   if (*mVar != mLastSetValue)
      CalcSliderVal();
}

void IntSlider::Render()
{
   float normalWidth = mWidth;
   float normalHeight = mHeight;

   if (Push2Control::sDrawingPush2Display)
   {
      mWidth = 100;
      mHeight = 15;
   }

   mLastDisplayedValue = *mVar;

   ofPushStyle();

   ofColor color, textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX + 1, mY + 1, mWidth, mHeight);
   ofSetColor(color);
   ofRect(mX, mY, mWidth, mHeight);
   ofNoFill();

   if (mWidth / MAX(1, (mMax - mMin)) > 3) //hash marks
   {
      ofPushStyle();
      ofSetColor(100, 100, 100, gModuleDrawAlpha);
      for (int i = mMin + 1; i < mMax; ++i)
      {
         float x = mX + 1 + (mWidth - 2) * ((i - mMin) / float(mMax - mMin));
         ofLine(x, mY + 1, x, mY + mHeight - 1);
      }
      ofPopStyle();
   }

   int val = ofClamp(*mVar, mMin, mMax);
   ofPushStyle();
   ofSetLineWidth(2);

   ofSetColor(255, 100, 0);
   float xposfloat = mX + 1 + (mWidth - 2) * mSliderVal;
   ofLine(xposfloat, mY + mHeight / 2 - 1, xposfloat, mY + mHeight / 2 + 1);

   if (*mVar >= mMin && *mVar <= mMax)
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
   else
      ofSetColor(30, 30, 30, gModuleDrawAlpha);
   float xpos = mX + 1 + (mWidth - 2) * ((val - mMin) / float(mMax - mMin));
   ofLine(xpos, mY + 1, xpos, mY + mHeight - 1);

   ofPopStyle();

   DrawBeacon(xpos, mY + mHeight / 2);

   DrawHover(mX, mY, mWidth, mHeight);

   std::string display;
   if (mShowName)
      display = GetDisplayName();
   if (display.length() > 0) //only show a colon if there's a label
      display += ":";
   if (mIntEntry)
   {
      ofSetColor(255, 255, 100);
      display += mIntEntry->GetText();
   }
   else
   {
      display += GetDisplayValue(*mVar);
   }

   if (mMaxEntry || mMinEntry)
      display = "";

   ofSetColor(textColor);
   DrawTextNormal(display, mX + 2, mY + 5 + mHeight / 2);

   ofPopStyle();

   if (mMaxEntry)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRectangle rect = mMaxEntry->GetRect(K(local));
      ofRect(rect.x - 28, rect.y, rect.width + 28, rect.height);
      mMaxEntry->Draw();
      ofSetColor(255, 255, 255);
      DrawTextRightJustify("max:", rect.x, rect.y + 5 + mHeight / 2);
      ofPopStyle();
   }
   if (mMinEntry)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRectangle rect = mMinEntry->GetRect(K(local));
      ofRect(rect.x - 28, rect.y, rect.width + 28, rect.height);
      mMinEntry->Draw();
      ofSetColor(255, 255, 255);
      DrawTextRightJustify("min:", rect.x, rect.y + 5 + mHeight / 2);
      ofPopStyle();
   }

   if (gHoveredUIControl == this && (GetKeyModifiers() & kModifier_Command) && mAllowMinMaxAdjustment && mMinEntry == nullptr && mMaxEntry == nullptr && !IUIControl::WasLastHoverSetManually())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(120, 120, 120, 255);
      ofRect(mX, mY, mWidth * .4f, mHeight);
      ofRect(mX + mWidth * .6f, mY, mWidth * .4f, mHeight);
      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(mX, mY, mWidth * .4f, mHeight);
      ofRect(mX + mWidth * .6f, mY, mWidth * .4f, mHeight);
      DrawTextNormal(ofToString(mMin), mX + 2, mY + 4 + mHeight / 2, 10);
      DrawTextRightJustify(ofToString(mMax), mX + mWidth - 2, mY + 4 + mHeight / 2, 10);
      ofPopStyle();
   }

   mWidth = normalWidth;
   mHeight = normalHeight;
}

void IntSlider::CalcSliderVal()
{
   mLastSetValue = *mVar;
   mSliderVal = ofMap(*mVar, mMin, mMax, 0.0f, 1.0f, K(clamp));
}

void IntSlider::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   if ((GetKeyModifiers() & kModifier_Command) && mAllowMinMaxAdjustment && !IUIControl::WasLastHoverSetManually())
   {
      bool adjustMax;
      if (x > mWidth / 2)
         adjustMax = true;
      else
         adjustMax = false;

      if (adjustMax)
      {
         if (mMaxEntry != nullptr)
            mMaxEntry->Delete();
         mMaxEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMax, -INT_MAX, INT_MAX);
         mMaxEntry->MakeActiveTextEntry(true);
      }
      else
      {
         if (mMinEntry != nullptr)
            mMinEntry->Delete();
         //mMinEntry = new TextEntry(this, "", mX, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMin, -INT_MAX, INT_MAX);
         mMinEntry->MakeActiveTextEntry(true);
      }

      return;
   }

   SetValueForMouse(x, y);
   mMouseDown = true;
}

bool IntSlider::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   if (mMouseDown)
      SetValueForMouse(x, y);
   return mMouseDown;
}

void IntSlider::SetValueForMouse(float x, float y)
{
   int oldVal = *mVar;
   *mVar = (int)round(ofMap(x + mX, mX + 1, mX + mWidth - 1, mMin, mMax));
   *mVar = ofClamp(*mVar, mMin, mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->IntSliderUpdated(this, oldVal, NextBufferTime(false));
   }
}

void IntSlider::SetFromMidiCC(float slider, double time, bool setViaModulator)
{
   slider = ofClamp(slider, 0, 1);
   SetValue(GetValueForMidiCC(slider), time);
   mSliderVal = slider;
   mLastSetValue = *mVar;
}

float IntSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider, 0, 1);
   return (int)round(ofMap(slider, 0, 1, mMin, mMax));
}

void IntSlider::SetValue(float value, double time, bool forceUpdate /*= false*/)
{
   int oldVal = *mVar;
   *mVar = (int)round(ofClamp(value, mMin, mMax));
   if (oldVal != *mVar || forceUpdate)
   {
      CalcSliderVal();
      gControlTactileFeedback = 1;
      mOwner->IntSliderUpdated(this, oldVal, time);
   }
}

float IntSlider::GetValue() const
{
   return *mVar;
}

float IntSlider::GetMidiValue() const
{
   return mSliderVal;
}

std::string IntSlider::GetDisplayValue(float val) const
{
   return ofToString(val, 0);
}

void IntSlider::Double()
{
   int doubl = *mVar * 2;
   if (doubl >= mMin && doubl <= mMax)
      SetValue(doubl, NextBufferTime(false));
}

void IntSlider::Halve()
{
   int half = *mVar / 2;
   if (half >= mMin && half <= mMax)
      SetValue(half, NextBufferTime(false));
}

void IntSlider::Increment(float amount)
{
   int val = *mVar + (int)amount;
   if (val >= mMin && val <= mMax)
      SetValue(val, NextBufferTime(false));
}

void IntSlider::ResetToOriginal()
{
   SetValue(mOriginalValue, NextBufferTime(false));
}

bool IntSlider::CheckNeedsDraw()
{
   if (IUIControl::CheckNeedsDraw())
      return true;

   return *mVar != mLastDisplayedValue;
}

bool IntSlider::AttemptTextInput()
{
   if (mIntEntry)
      mIntEntry->Delete();
   mIntEntry = new TextEntry(this, "", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 10, mEntryString);
   mIntEntry->MakeActiveTextEntry(true);
   mIntEntry->SetRequireEnter(true);
   mIntEntry->ClearInput();
   return true;
}

void IntSlider::TextEntryComplete(TextEntry* entry)
{
   if (entry == mIntEntry)
   {
      mIntEntry->Delete();
      mIntEntry = nullptr;

      float evaluated = 0;
      bool expressionValid = EvaluateExpression(mEntryString, *mVar, evaluated);
      int evaluatedInt = round(evaluated);
      if (expressionValid && ((evaluatedInt >= mMin && evaluatedInt <= mMax) || (GetKeyModifiers() & kModifier_Shift)))
         SetValue(evaluatedInt, NextBufferTime(false));
   }
   if (entry == mMaxEntry)
   {
      mMaxEntry->Delete();
      mMaxEntry = nullptr;
   }
   if (entry == mMinEntry)
   {
      mMinEntry->Delete();
      mMinEntry = nullptr;
   }
}

void IntSlider::TextEntryCancelled(TextEntry* entry)
{
   if (entry == mIntEntry)
   {
      mIntEntry->Delete();
      mIntEntry = nullptr;
   }
}

namespace
{
   const int kIntSliderSaveStateRev = 1;
}

void IntSlider::SaveState(FileStreamOut& out)
{
   out << kIntSliderSaveStateRev;

   out << (float)*mVar;
   out << mMin;
   out << mMax;
}

void IntSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kIntSliderSaveStateRev);

   float var;
   in >> var;

   if (rev >= 1)
   {
      in >> mMin;
      in >> mMax;
   }
   else
   {
      if (var < mMin)
         mMin = var;
      if (var > mMax)
         mMax = var;
   }

   if (shouldSetValue)
      SetValueDirect(var, gTime);
}
