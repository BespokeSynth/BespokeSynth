//
//  Checkbox.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#include "Checkbox.h"
#include "IDrawableModule.h"
#include "SynthGlobals.h"
#include "FileStream.h"

Checkbox::Checkbox(IDrawableModule* owner, const char* label, int x, int y, bool* var)
: mVar(var)
, mOwner(owner)
, mDisplayText(true)
, mUseCircleLook(false)
, mSliderVal(0)
{
   assert(owner);
   SetLabel(label);
   SetPosition(x,y);
   owner->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   CalcSliderVal();
}

Checkbox::Checkbox(IDrawableModule* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, bool* var)
: Checkbox(owner, label, -1, -1, var)
{
   PositionTo(anchor, anchorDirection);
}

Checkbox::~Checkbox()
{
}

void Checkbox::SetLabel(const char* label)
{
   SetName(label);
   if (mDisplayText)
      mWidth = 12+GetStringWidth(label);
   else
      mWidth = 13;
}

void Checkbox::SetDisplayText(bool display)
{
   mDisplayText = display;
   SetLabel(Name()); //recalculate width
}

void Checkbox::UseCircleLook(ofColor color)
{
   mUseCircleLook = true;
   mCustomColor = color;
}

void Checkbox::Poll()
{
   if (*mVar != mLastSetValue)
      CalcSliderVal();
}

void Checkbox::Render()
{
   mLastDisplayedValue = *mVar;
   
   ofPushStyle();
   
   DrawBeacon(mX+6, mY+8);
   
   ofColor color;
   if (IsPreset())
      color.set(0,255,0);
   else if (mUseCircleLook)
      color = mCustomColor;
   else
      color.set(255,255,255);
   color.a = gModuleDrawAlpha;
   
   ofColor darkColor = color;
   darkColor.setBrightness(30);

   ofFill();
   if (mUseCircleLook)
   {
      ofSetColor(darkColor);
      ofCircle(mX+6,mY+8,5);
   }
   else
   {
      ofSetColor(color.r,color.g,color.b,color.a*.2f);
      ofRect(mX,mY+1,12,12);
   }

   ofSetColor(color);
   
   if (mDisplayText)
      DrawTextNormal(Name(), mX+13, mY+12);
   if (*mVar)
   {
      if (mUseCircleLook)
         ofCircle(mX+6,mY+8,3);
      else
         ofRect(mX+2, mY+3, 8, 8, 2);
   }

   ofPopStyle();
   
   DrawHover();
}

void Checkbox::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   *mVar = !(*mVar);
   CalcSliderVal();
   mOwner->CheckboxUpdated(this);
}

void Checkbox::CalcSliderVal()
{
   mLastSetValue = *mVar;
   mSliderVal = *mVar ? 1 : 0;
}

bool Checkbox::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   return false;
}

void Checkbox::SetFromMidiCC(float slider)
{
   slider = ofClamp(slider,0,1);
   mSliderVal = slider;
   bool on = GetValueForMidiCC(slider) > 0;
   if (*mVar != on)
   {
      *mVar = on;
      mLastSetValue = *mVar;
      mOwner->CheckboxUpdated(this);
   }
}

float Checkbox::GetValueForMidiCC(float slider) const
{
   return slider > .5f ? 1 : 0;
}

void Checkbox::SetValue(float value)
{
   bool on = value > 0;
   if (*mVar != on)
   {
      *mVar = on;
      CalcSliderVal();
      mOwner->CheckboxUpdated(this);
   }
}

float Checkbox::GetMidiValue()
{
   return mSliderVal;
}

float Checkbox::GetValue() const
{
   return *mVar;
}

string Checkbox::GetDisplayValue(float val) const
{
   return val>0 ? "on" : "off";
}

void Checkbox::Increment(float amount)
{
   *mVar = !*mVar;
   CalcSliderVal();
}

bool Checkbox::CheckNeedsDraw()
{
   if (IUIControl::CheckNeedsDraw())
      return true;
   
   return *mVar != mLastDisplayedValue;
}

namespace
{
   const int kSaveStateRev = 0;
}

void Checkbox::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << (float)*mVar;
}

void Checkbox::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   float var;
   in >> var;
   if (shouldSetValue)
      SetValueDirect(var);
}
