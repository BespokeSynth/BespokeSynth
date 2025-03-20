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

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, double* var, double min, double max, int digits /* = -1 */)
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
   mLastComputeCacheValue = new double[gBufferSize];
}

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, double* var, double min, double max, int digits /* = -1 */)
: FloatSlider(owner, label, -1, -1, w, h, var, min, max, digits)
{
   PositionTo(anchor, anchorDir);
}

FloatSlider::~FloatSlider()
{
   if (mIsSmoothing)
      TheTransport->RemoveAudioPoller(this);
}

void FloatSlider::Init()
{
   if (mVar)
      mOriginalValue = *mVar;
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

void FloatSlider::Poll()
{
   if (mLastComputeTime + .1 < gTime)
      Compute();
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
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5);
   ofRect(mX + 1, mY + 1, mWidth, mHeight);
   ofSetColor(color);
   ofRect(mX, mY, mWidth, mHeight);
   ofNoFill();

   bool showSmoothAdjustmentUI = AdjustSmooth() && (gHoveredUIControl == this || mSmooth > 0);

   if (mIsSmoothing && !showSmoothAdjustmentUI)
   {
      ofPushStyle();
      ofSetColor(255, 255, 0, gModuleDrawAlpha);
      double val = ofClamp(mSmoothTarget, mMin, mMax);
      double screenPos = mX + 1 + (mWidth - 2) * ValToPos(val, false);
      ofSetLineWidth(1);
      ofFill();
      ofCircle(screenPos, mY + mHeight / 2, 3);
      ofPopStyle();
   }

   double screenPos;
   if (mModulator && mModulator->Active() && !showSmoothAdjustmentUI)
   {
      screenPos = mX + 1 + (mWidth - 2) * ValToPos(*mVar, true);
      double lfomax = ofClamp(mModulator->GetMax(), mMin, mMax);
      double screenPosMax = mX + 1 + (mWidth - 2) * ValToPos(lfomax, true);
      double lfomin = ofClamp(mModulator->GetMin(), mMin, mMax);
      double screenPosMin = mX + 1 + (mWidth - 2) * ValToPos(lfomin, true);

      ofPushStyle();
      ofSetColor(0, 200, 0, gModuleDrawAlpha * .5);
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
      double val = ofClamp(*mVar, mMin, mMax);
      screenPos = mX + 1 + (mWidth - 2) * ValToPos(val, false);
      ofSetLineWidth(2);
      ofLine(screenPos, mY + 1, screenPos, mY + mHeight - 1); //value bar
      ofPopStyle();
   }

   DrawBeacon(screenPos, mY + mHeight / 2);

   DrawHover(mX, mY, mWidth, mHeight);

   std::string display;
   double textSize = 13;
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
      ofRect(mX, mY, mWidth * .4, mHeight);
      ofRect(mX + mWidth * .6, mY, mWidth * .4, mHeight);
      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(mX, mY, mWidth * .4, mHeight);
      ofRect(mX + mWidth * .6, mY, mWidth * .4, mHeight);

      ofPushMatrix();
      ofClipWindow(mX, mY, mWidth * .4, mHeight, true);
      DrawTextNormal(ofToString(mMin), mX + 2, mY + 4 + mHeight / 2, 10);
      ofPopMatrix();

      ofPushMatrix();
      ofClipWindow(mX + mWidth * .6, mY, mWidth * .4, mHeight, true);
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

      double thisx, thisy;
      GetPosition(thisx, thisy);

      lfo->SetLFOEnabled(true);

      double w, h;
      lfo->GetDimensions(w, h);
      lfo->SetPosition(thisx, thisy + 15);
      lfo->SetOwningContainer(GetModuleParent()->GetOwningContainer());
      TheSynth->PushModalFocusItem(lfo);

      if (TheLFOController)
         TheLFOController->SetSlider(this);
   }
}

void FloatSlider::OnClicked(double x, double y, bool right)
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
         mMaxEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMax, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
         mMaxEntry->MakeActiveTextEntry(true);
      }
      else
      {
         if (mMinEntry != nullptr)
            mMinEntry->Delete();
         //mMinEntry = new TextEntry(this, "", mX, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry = new TextEntry(this, "", mX + mWidth - 5 * 9, mY, 5, &mMin, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
         mMinEntry->MakeActiveTextEntry(true);
      }

      return;
   }

   mFineRefX = ofMap(ValToPos(*GetModifyValue(), false), 0.0, 1.0, mX + 1, mX + mWidth - 1, true) - mX;
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

bool FloatSlider::MouseMoved(double x, double y)
{
   CheckHover(x, y);
   if (mMouseDown)
      SetValueForMouse(x, y);
   return mMouseDown;
}

void FloatSlider::SetValueForMouse(double x, double y)
{
   double* var = GetModifyValue();
   double fX = x;
   if (GetKeyModifiers() & kModifier_Shift)
   {
      if (mFineRefX == -999)
      {
         mFineRefX = x;
      }
      double precision = mShowDigits != -1 ? 100 : 10;
      fX = mFineRefX + (fX - mFineRefX) / precision;
   }
   else
   {
      mFineRefX = -999;
   }
   double oldVal = *var;
   double pos = ofMap(fX + mX, mX + 1, mX + mWidth - 1, 0.0, 1.0);

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
      double move = (y - mRefY) * -.003;
      double change = move * (mMax - mMin);
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
      TheTransport->AddAudioPoller(this);
      mSmoothTarget = *mVar;
      mRamp.SetValue(mSmoothTarget);
      mIsSmoothing = true;
   }
   else if (mSmooth <= 0 && mIsSmoothing)
   {
      TheTransport->RemoveAudioPoller(this);
      mIsSmoothing = false;
   }
}

void FloatSlider::SetFromMidiCC(double slider, double time, bool setViaModulator)
{
   SetValue(GetValueForMidiCC(slider), time);
}

double FloatSlider::GetValueForMidiCC(double slider) const
{
   slider = ofClamp(slider, 0, 1);
   return PosToVal(slider, true);
}

double FloatSlider::PosToVal(double pos, bool ignoreSmooth) const
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
      return mMin * pow(mMax / mMin, pos);
   if (mMode == kSquare)
      return mMin + pos * pos * (mMax - mMin);
   if (mMode == kBezier)
   {
      double y = pos * (pos * (pos * (mMax - mMin) + 3 * mMin - 3 * mBezierControl) - 3 * mMin + 3 * mBezierControl) + mMin;
      return y;
   }
   assert(false);
   return 0;
}

double FloatSlider::ValToPos(double val, bool ignoreSmooth) const
{
   val = ofClamp(val, mMin, mMax);
   if (AdjustSmooth() && (gHoveredUIControl == this || mSmooth > 0) && !ignoreSmooth)
      return sqrt(mSmooth);
   if (mMode == kNormal)
      return (val - mMin) / (mMax - mMin);
   if (mMode == kLogarithmic)
      return log(val / mMin) / log(mMax / mMin);
   if (mMode == kSquare)
      return sqrt((val - mMin) / (mMax - mMin));
   if (mMode == kBezier)
   {
      double closest = 0;
      double closestDist = std::numeric_limits<double>::max();
      for (double pos = 0; pos < 1; pos += .001)
      {
         double dist = abs(PosToVal(pos, true) - val);
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

void FloatSlider::SetValue(double value, double time, bool forceUpdate /*= false*/)
{
   if (TheLFOController && TheLFOController->WantsBinding(this))
   {
      TheLFOController->SetSlider(this);
      return;
   }

   double* var = GetModifyValue();
   double oldVal = *var;
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
      mLFOControl->SetLFOEnabled(false);
}

double FloatSlider::GetValue() const
{
   return *mVar;
}

double FloatSlider::GetMidiValue() const
{
   if (mMin == mMax)
      return 0;

   return ValToPos(*mVar, true);
}

std::string FloatSlider::GetDisplayValue(double val) const
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

   double displayVar = val;
   if (decDigits == 0) //round down if we're showing int value
      displayVar = (int)displayVar;
   return ofToString(displayVar, decDigits);
}

void FloatSlider::DoCompute(int samplesIn /*= 0*/)
{
   if (ofAlmostEquel(mLastComputeTime, gTime) && mLastComputeSamplesIn == samplesIn)
      return; //we've just calculated this, no need to do it again! earlying out avoids wasted work and circular modulation loops

   if (mLFOControl && mLFOControl->Active() && mLFOControl->InLowResMode() && samplesIn != 0)
      return; //only do the math on Compute(0) for low res mode

   mLastComputeTime = gTime;
   mLastComputeSamplesIn = samplesIn;

   double oldVal = *mVar;

   const bool kUseCache = true;
   if (kUseCache && IsAudioThread() && samplesIn >= 0 && samplesIn < gBufferSize && ofAlmostEquel(mLastComputeCacheTime[samplesIn], gTime))
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

      if (IsAudioThread() && samplesIn >= 0 && samplesIn < gBufferSize && !ofAlmostEquel(mLastComputeCacheTime[samplesIn], gTime))
      {
         mLastComputeCacheValue[samplesIn] = *mVar;
         mLastComputeCacheTime[samplesIn] = gTime;
      }
   }

   if (!ofAlmostEquel(oldVal, *mVar))
      mOwner->FloatSliderUpdated(this, oldVal, gTime + samplesIn * gInvSampleRateMs);
}

double* FloatSlider::GetModifyValue()
{
   if (!TheSynth->IsLoadingModule() && mModulator && mModulator->Active() && mModulator->CanAdjustRange())
      return &mModulator->GetMax();
   if (mIsSmoothing)
      return &mSmoothTarget;
   return mVar;
}

void FloatSlider::Double()
{
   double doubl = *GetModifyValue() * 2.0;
   if (doubl >= mMin && doubl <= mMax)
      SetValue(doubl, NextBufferTime(false));
}

void FloatSlider::Halve()
{
   double half = *GetModifyValue() * .5;
   if (half >= mMin && half <= mMax)
      SetValue(half, NextBufferTime(false));
}

void FloatSlider::Increment(double amount)
{
   double val = *GetModifyValue() + amount;
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

   return !ofAlmostEquel(*mVar, mLastDisplayedValue);
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

      double evaluated = 0;
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

void FloatSlider::OnTransportAdvanced(double amount)
{
   mRamp.Start(gTime, mSmoothTarget, gTime + (amount * TheTransport->MsPerBar() * (mSmooth * 300)));
}

namespace
{
   const int kFloatSliderSaveStateRev = 7;
}

void FloatSlider::SaveState(FileStreamOut& out)
{
   out << kFloatSliderSaveStateRev;

   out << (double)*mVar;

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

   float var{ 0 };
   double vard{ 0 };
   if (rev < 7)
   {
      in >> var;
      mRamp.SetValue(var);
   }
   else
   {
      in >> vard;
      mRamp.SetValue(vard);
   }

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
      if (rev < 7)
      {
         float a, b;
         in >> a >> b;
         mModulatorMin = static_cast<double>(a);
         mModulatorMax = static_cast<double>(b);
      }
      else
      {
         in >> mModulatorMin;
         in >> mModulatorMax;
      }
   }

   if (rev >= 3)
   {
      if (rev < 7)
      {
         float a, b;
         in >> a >> b;
         mSmooth = static_cast<double>(a);
         mSmoothTarget = static_cast<double>(b);
      }
      else
      {
         in >> mSmooth;
         in >> mSmoothTarget;
      }
      in >> mIsSmoothing;

      if (mIsSmoothing)
         TheTransport->AddAudioPoller(this);
   }

   if (rev >= 4)
   {
      if (rev < 7)
      {
         float a, b;
         in >> a >> b;
         mMin = static_cast<double>(a);
         mMax = static_cast<double>(b);
      }
      else
      {
         in >> mMin;
         in >> mMax;
      }
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
   {
      if (rev < 7)
         SetValueDirect(var, gTime);
      else
         SetValueDirect(vard, gTime);
   }
}

IntSlider::IntSlider(IIntSliderListener* owner, const char* label, int x, int y, int w, int h, int* var, int min, int max)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mOwner(owner)
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
   double normalWidth = mWidth;
   double normalHeight = mHeight;

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
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5);
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
         double x = mX + 1 + (mWidth - 2) * ((i - mMin) / static_cast<double>(mMax - mMin));
         ofLine(x, mY + 1, x, mY + mHeight - 1);
      }
      ofPopStyle();
   }

   int val = ofClamp(*mVar, mMin, mMax);
   ofPushStyle();
   ofSetLineWidth(2);

   ofSetColor(255, 100, 0);
   double xposfloat = mX + 1 + (mWidth - 2) * mSliderVal;
   ofLine(xposfloat, mY + mHeight / 2 - 1, xposfloat, mY + mHeight / 2 + 1);

   if (*mVar >= mMin && *mVar <= mMax)
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
   else
      ofSetColor(30, 30, 30, gModuleDrawAlpha);
   double xpos = mX + 1 + (mWidth - 2) * ((val - mMin) / double(mMax - mMin));
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
      ofRect(mX, mY, mWidth * .4, mHeight);
      ofRect(mX + mWidth * .6, mY, mWidth * .4, mHeight);
      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(mX, mY, mWidth * .4, mHeight);
      ofRect(mX + mWidth * .6, mY, mWidth * .4, mHeight);
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
   mSliderVal = ofMap(*mVar, mMin, mMax, 0.0, 1.0, K(clamp));
}

void IntSlider::OnClicked(double x, double y, bool right)
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

bool IntSlider::MouseMoved(double x, double y)
{
   CheckHover(x, y);
   if (mMouseDown)
      SetValueForMouse(x, y);
   return mMouseDown;
}

void IntSlider::SetValueForMouse(double x, double y)
{
   int oldVal = *mVar;
   float fX = x;
   if (GetKeyModifiers() & kModifier_Shift)
   {
      if (mFineRefX == -999)
      {
         mFineRefX = x;
      }
      fX = mFineRefX + (fX - mFineRefX) / 10;
   }
   else
   {
      mFineRefX = -999;
   }

   *mVar = (int)round(ofMap(fX + mX, mX + 1, mX + mWidth - 1, mMin, mMax));
   *mVar = ofClamp(*mVar, mMin, mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->IntSliderUpdated(this, oldVal, NextBufferTime(false));
   }
}

void IntSlider::SetFromMidiCC(double slider, double time, bool setViaModulator)
{
   slider = ofClamp(slider, 0, 1);
   SetValue(GetValueForMidiCC(slider), time);
   mSliderVal = slider;
   mLastSetValue = *mVar;
}

double IntSlider::GetValueForMidiCC(double slider) const
{
   slider = ofClamp(slider, 0, 1);
   return (int)round(ofMap(slider, 0, 1, mMin, mMax));
}

void IntSlider::SetValue(double value, double time, bool forceUpdate /*= false*/)
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

double IntSlider::GetValue() const
{
   return *mVar;
}

double IntSlider::GetMidiValue() const
{
   return mSliderVal;
}

std::string IntSlider::GetDisplayValue(double val) const
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

void IntSlider::Increment(double amount)
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

      double evaluated = 0;
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
   const int kIntSliderSaveStateRev = 2;
}

void IntSlider::SaveState(FileStreamOut& out)
{
   out << kIntSliderSaveStateRev;

   out << static_cast<double>(*mVar);
   out << mMin;
   out << mMax;
}

void IntSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kIntSliderSaveStateRev);

   double var;
   if (rev < 2)
   {
      float a;
      in >> a;
      var = static_cast<double>(a);
   }
   else
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
