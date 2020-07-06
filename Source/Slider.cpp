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
#include "Profiler.h"

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, float* var, float min, float max, int digits /* = -1 */)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mModulatorMin(min)
, mModulatorMax(max)
, mMouseDown(false)
, mFineRefX(-999)
, mRefY(-999)
, mShowDigits(digits)
, mOwner(owner)
, mLFOControl(nullptr)
, mRelative(false)
, mTouching(false)
, mRelativeOffset(-999)
, mClamped(true)
, mMode(kNormal)
, mOriginalValue(0)
, mClampIntMin(-999)
, mMinValueDisplay("")
, mMaxValueDisplay("")
, mShowName(true)
, mBezierControl(1)
, mModulator(nullptr)
, mSmooth(0)
, mIsSmoothing(false)
, mComputeHasBeenCalledOnce(false)
, mLastComputeTime(0)
, mLastDisplayedValue(FLT_MAX)
, mFloatEntry(nullptr)
, mAllowMinMaxAdjustment(true)
, mMinEntry(nullptr)
, mMaxEntry(nullptr)
{
   assert(owner);
   SetLabel(label);
   SetPosition(x,y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, float* var, float min, float max, int digits /* = -1 */)
: FloatSlider(owner,label,-1,-1,w,h,var,min,max,digits)
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
   mLFOControl = lfo;
   mModulator = lfo;
}

void FloatSlider::SetLabel(const char* label)
{
   SetName(label);
}

void FloatSlider::Render()
{
   if (mLastComputeTime + .1f < gTime)
      Compute();
   
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
   ofRect(mX+1,mY+1,mWidth,mHeight);
   ofSetColor(color);
   ofRect(mX,mY,mWidth,mHeight);
   ofNoFill();
   
   if (mIsSmoothing && !AdjustSmooth())
   {
      ofPushStyle();
      ofSetColor(255,255,0,gModuleDrawAlpha);
      float val = ofClamp(mSmoothTarget,mMin,mMax);
      float screenPos = mX+1+(mWidth-2)*ValToPos(val, false);
      ofSetLineWidth(1);
      ofFill();
      ofCircle(screenPos,mY+mHeight/2,3);
      ofPopStyle();
   }
   
   float screenPos;
   if (mModulator && mModulator->Active() && !AdjustSmooth())
   {
      screenPos = mX+1+(mWidth-2)*ValToPos(*mVar, true);
      float lfomax = ofClamp(mModulator->GetMax(),mMin,mMax);
      float screenPosMax = mX+1+(mWidth-2)*ValToPos(lfomax, true);
      float lfomin = ofClamp(mModulator->GetMin(),mMin,mMax);
      float screenPosMin = mX+1+(mWidth-2)*ValToPos(lfomin, true);
      
      ofPushStyle();
      ofSetColor(0,200,0,gModuleDrawAlpha*.5f);
      ofFill();
      ofRect(screenPosMin,mY,screenPos-screenPosMin,mHeight, 1); //lfo bar
      ofPopStyle();

      ofPushStyle();
      ofSetColor(0,255,0,gModuleDrawAlpha);
      ofSetLineWidth(2);
      ofLine(screenPosMin,mY+1,screenPosMin,mY+mHeight-1); //min bar
      ofLine(screenPosMax,mY+1,screenPosMax,mY+mHeight-1); //max bar
      ofPopStyle();
   }
   else
   {
      ofPushStyle();
      if (*mVar >= mMin && *mVar <= mMax)
         ofSetColor(255,0,0,gModuleDrawAlpha);
      else
         ofSetColor(30,30,30,gModuleDrawAlpha);
      if (AdjustSmooth())
         ofSetColor(255,255, 0, gModuleDrawAlpha);
      float val = ofClamp(*mVar,mMin,mMax);
      screenPos = mX+1+(mWidth-2)*ValToPos(val, false);
      ofSetLineWidth(2);
      ofLine(screenPos,mY+1,screenPos,mY+mHeight-1);  //value bar
      ofPopStyle();
   }
   
   DrawBeacon(screenPos, mY+mHeight/2);
   
   DrawHover();

   string display;
   if (AdjustSmooth())
   {
      display = "s:"+ofToString(mSmooth, 3);
   }
   else
   {
      if (mShowName)
         display = string(Name());
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
   
   if (mMaxEntry)
      display = "set max:";
   if (mMinEntry)
      display = "set min:";
   
   ofSetColor(textColor);
   DrawTextNormal(display, mX+2, mY+5+mHeight/2);

   ofPopStyle();
   
   if (mMaxEntry)
      mMaxEntry->Draw();
   if (mMinEntry)
      mMinEntry->Draw();
   
   mWidth = normalWidth;
   mHeight = normalHeight;
}

void FloatSlider::DisplayLFOControl()
{
   FloatSliderLFOControl* lfo = AcquireLFO();
   if (lfo)
   {
      float thisx,thisy;
      GetPosition(thisx,thisy);
      
      lfo->SetLFOEnabled(true);
      
      float w,h;
      lfo->GetDimensions(w, h);
      lfo->SetPosition(thisx,thisy+15);
      TheSynth->PushModalFocusItem(lfo);
      
      if (TheLFOController)
         TheLFOController->SetSlider(this);
   }
}

void FloatSlider::OnClicked(int x, int y, bool right)
{
   if (right)
   {
      DisplayLFOControl();
      return;
   }
   
   if ((GetKeyModifiers() & kModifier_Command) && mAllowMinMaxAdjustment)
   {
      bool adjustMax;
      if (x > mWidth/2)
         adjustMax = true;
      else
         adjustMax = false;
      
      if (adjustMax)
      {
         mMaxEntry = new TextEntry(this, "", mX+mWidth-5*9, mY, 5, &mMax, -FLT_MAX, FLT_MAX);
         mMaxEntry->MakeActiveTextEntry(true);
      }
      else
      {
         //mMinEntry = new TextEntry(this, "", mX, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry = new TextEntry(this, "", mX+mWidth-5*9, mY, 5, &mMin, -FLT_MAX, FLT_MAX);
         mMinEntry->MakeActiveTextEntry(true);
      }
      
      return;
   }

   mFineRefX = ofMap(ValToPos(*GetModifyValue(), false),0.0f,1.0f,mX+1,mX+mWidth-1,true)-mX;
   mRefY = y;
   SetValueForMouse(x,y);
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
      SetValue(0);
}

bool FloatSlider::MouseMoved(float x, float y)
{
   CheckHover(x,y);
   if (mMouseDown)
      SetValueForMouse(x,y);
   return mMouseDown;
}

void FloatSlider::SetValueForMouse(int x, int y)
{
   float* var = GetModifyValue();
   float fX = x;
   bool clampInt = false;
   if (GetKeyModifiers() & kModifier_Shift)
   {
      if (mFineRefX == -999)
      {
         mFineRefX = x;
      }
      float precision = mShowDigits != -1 ? 100 : 10;
      fX = mFineRefX + (fX-mFineRefX)/precision;
   }
   else
   {
      mFineRefX = -999;
      mClampIntMin = -999;
   }
   float oldVal = *var;
   float pos = ofMap(fX+mX,mX+1,mX+mWidth-1,0.0f,1.0f);
   
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
   *var = ofClamp(*var,mMin,mMax);
   if (clampInt)
      *var = ofClamp(*var,mClampIntMin,mClampIntMax);
   
   if (oldVal != *var)
   {
      mOwner->FloatSliderUpdated(this, oldVal);
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
          mComputeHasBeenCalledOnce;   //no smoothing if we're not calling Compute()
}

void FloatSlider::SmoothUpdated()
{
   if (mSmooth > 0 && !mIsSmoothing)
   {
      TheTransport->AddAudioPoller(this);
      mSmoothTarget = *mVar;
   }
   if (mSmooth <= 0 && mIsSmoothing)
      TheTransport->RemoveAudioPoller(this);
   mIsSmoothing = mSmooth > 0;
}

void FloatSlider::SetFromMidiCC(float slider)
{
   SetValue(GetValueForMidiCC(slider));
}

float FloatSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider,0,1);
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
      return pos*pos;
   }
   if (pos < 0)
      return mMin;
   if (pos > 1)
      return mMax;
   if (mMode == kNormal)
      return mMin + pos*(mMax-mMin);
   if (mMode == kLogarithmic)
      return mMin * powf(mMax/mMin, pos);
   if (mMode == kSquare)
      return mMin + pos*pos*(mMax-mMin);
   if (mMode == kBezier)
   {
      float y = pos * (pos * (pos * (mMax-mMin) + 3 * mMin - 3 * mBezierControl) - 3 * mMin + 3 * mBezierControl) + mMin;
      return y;
   }
   assert(false);
   return 0;
}

float FloatSlider::ValToPos(float val, bool ignoreSmooth) const
{
   if (AdjustSmooth() && !ignoreSmooth)
      return sqrtf(mSmooth);
   if (mMode == kNormal)
      return (val - mMin) / (mMax-mMin);
   if (mMode == kLogarithmic)
      return log(val/mMin) / log(mMax/mMin);
   if (mMode == kSquare)
      return sqrtf((val - mMin) / (mMax-mMin));
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

void FloatSlider::SetValue(float value)
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
   if (mClamped)
      *var = ofClamp(value,mMin,mMax);
   else
      *var = value;
   DisableLFO();
   if (oldVal != *var)
   {
      mOwner->FloatSliderUpdated(this, oldVal);
   }
}

void FloatSlider::UpdateTouching()
{
   if (mRelative && (mModulator == nullptr || mModulator->Active() == false))
   {
      if (!mTouching)
         SetValue(0);
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

float FloatSlider::GetMidiValue()
{
   if (mMin == mMax)
      return 0;
   
   return ValToPos(*mVar, true);
}

string FloatSlider::GetDisplayValue(float val) const
{
   if (val == mMin && mMinValueDisplay != "")
      return mMinValueDisplay;
   if (val == mMax && mMaxValueDisplay != "")
      return mMaxValueDisplay;
   
   int decDigits = 3;
   if (mShowDigits != -1)
      decDigits = mShowDigits;
   else if (mMax-mMin > 1000)
      decDigits = 0;
   else if (mMax-mMin > 100)
      decDigits = 1;
   else if (mMax-mMin > 10)
      decDigits = 2;
   
   float displayVar = val;
   if (decDigits == 0)  //round down if we're showing int value
      displayVar = (int)displayVar;
   return ofToString(displayVar,decDigits);
}

void FloatSlider::Compute(int samplesIn /*= 0*/)
{
   mComputeHasBeenCalledOnce = true;
   mLastComputeTime = gTime;
   
   if (mModulator && mModulator->Active())
   {
      float* var = mIsSmoothing ? &mSmoothTarget : mVar;
      float oldVal = *var;
      *var = mModulator->Value(samplesIn);
      if (oldVal != *var && !mIsSmoothing)
      {
         //PROFILER(FloatSlider_Compute_UpdateSlider);
         mOwner->FloatSliderUpdated(this, oldVal);
      }
   }
   if (mIsSmoothing)
   {
      float oldVal = *mVar;
      *mVar = ofClamp(mRamp.Value(gTime + samplesIn * gInvSampleRateMs), mMin, mMax);
      if (oldVal != *mVar)
      {
         mOwner->FloatSliderUpdated(this, oldVal);
      }
   }
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
      SetValue(doubl);
}

void FloatSlider::Halve()
{
   float half = *GetModifyValue() * .5f;
   if (half >= mMin && half <= mMax)
      SetValue(half);
}

void FloatSlider::Increment(float amount)
{
   float val = *GetModifyValue() + amount;
   if (val >= mMin && val <= mMax)
      SetValue(val);
}

void FloatSlider::ResetToOriginal()
{
   SetValue(mOriginalValue);
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
      if (expressionValid)
         SetValue(evaluated);
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

void FloatSlider::OnTransportAdvanced(float amount)
{
   mRamp.Start(mSmoothTarget, (amount * TheTransport->MsPerBar() * (mSmooth*300)));
}

namespace
{
   const int kFloatSliderSaveStateRev = 5;
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
            mLFOControl->LoadState(in);
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
      
      if (mIsSmoothing)
         TheTransport->AddAudioPoller(this);
   }
   
   if (rev >= 4)
   {
      in >> mMin;
      in >> mMax;
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
            mLFOControl->LoadState(in);
         }
         if (shouldSetValue)
            lfo->UpdateFromSettings();
      }
   }
   
   if (shouldSetValue)
      SetValueDirect(var);
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
   SetLabel(label);
   SetPosition(x,y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   CalcSliderVal();
}

IntSlider::IntSlider(IIntSliderListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDir, int w, int h, int* var, int min, int max)
: IntSlider(owner,label,-1,-1,w,h,var,min,max)
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

void IntSlider::SetLabel(const char* label)
{
   SetName(label);
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

   ofColor color,textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX+1,mY+1,mWidth,mHeight);
   ofSetColor(color);
   ofRect(mX,mY,mWidth,mHeight);
   ofNoFill();

   if (mWidth / MAX(1, (mMax - mMin)) > 3)   //hash marks
   {
      ofPushStyle();
      ofSetColor(100,100,100,gModuleDrawAlpha);
      for (int i=mMin+1; i<mMax; ++i)
      {
         float x = mX+1+(mWidth-2)*((i-mMin)/float(mMax-mMin));
         ofLine(x,mY+1,x,mY+mHeight-1);
      }
      ofPopStyle();
   }

   int val = ofClamp(*mVar,mMin,mMax);
   ofPushStyle();
   ofSetLineWidth(2);
   
   ofSetColor(255,100,0);
   float xposfloat = mX+1+(mWidth-2)*mSliderVal;
   ofLine(xposfloat,mY+mHeight/2-1,xposfloat,mY+mHeight/2+1);
   
   if (*mVar >= mMin && *mVar <= mMax)
      ofSetColor(255,0,0,gModuleDrawAlpha);
   else
      ofSetColor(30,30,30,gModuleDrawAlpha);
   float xpos = mX+1+(mWidth-2)*((val-mMin)/float(mMax-mMin));
   ofLine(xpos,mY+1,xpos,mY+mHeight-1);
   
   ofPopStyle();
   
   DrawBeacon(xpos, mY+mHeight/2);
   
   DrawHover();

   string display;
   if (mShowName)
      display = string(Name());
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
   ofSetColor(textColor);
   DrawTextNormal(display, mX+2, mY+5+mHeight/2);

   ofPopStyle();
   
   if (mMaxEntry)
      mMaxEntry->Draw();
   if (mMinEntry)
      mMinEntry->Draw();
   
   mWidth = normalWidth;
   mHeight = normalHeight;
}

void IntSlider::CalcSliderVal()
{
   mLastSetValue = *mVar;
   mSliderVal = ofMap(*mVar,mMin,mMax,0.0f,1.0f,K(clamp));
}

void IntSlider::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   SetValueForMouse(x,y);
   mMouseDown = true;
}

bool IntSlider::MouseMoved(float x, float y)
{
   CheckHover(x,y);
   if (mMouseDown)
      SetValueForMouse(x,y);
   return mMouseDown;
}

void IntSlider::SetValueForMouse(int x, int y)
{
   int oldVal = *mVar;
   *mVar = (int)round(ofMap(x+mX,mX+1,mX+mWidth-1,mMin,mMax));
   *mVar = ofClamp(*mVar,mMin,mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->IntSliderUpdated(this, oldVal);
   }
}

void IntSlider::SetFromMidiCC(float slider)
{
   slider = ofClamp(slider,0,1);
   SetValue(GetValueForMidiCC(slider));
   mSliderVal = slider;
   mLastSetValue = *mVar;
}

float IntSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider,0,1);
   return (int)round(ofMap(slider,0,1,mMin,mMax));
}

void IntSlider::SetValue(float value)
{
   int oldVal = *mVar;
   *mVar = (int)ofClamp(value,mMin,mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      gControlTactileFeedback = 1;
      mOwner->IntSliderUpdated(this, oldVal);
   }
}

float IntSlider::GetValue() const
{
   return *mVar;
}

float IntSlider::GetMidiValue()
{
   return mSliderVal;
}

string IntSlider::GetDisplayValue(float val) const
{
   return ofToString(val,0);
}

void IntSlider::Increment(float amount)
{
   int val = *mVar + (int)amount;
   if (val >= mMin && val <= mMax)
      SetValue(val);
}

void IntSlider::ResetToOriginal()
{
   SetValue(mOriginalValue);
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
      if (expressionValid)
         SetValue(round(evaluated));
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

namespace
{
   const int kIntSliderSaveStateRev = 0;
}

void IntSlider::SaveState(FileStreamOut& out)
{
   out << kIntSliderSaveStateRev;
   
   out << (float)*mVar;
}

void IntSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kIntSliderSaveStateRev);
   
   float var;
   in >> var;
   if (shouldSetValue)
      SetValueDirect(var);
}
