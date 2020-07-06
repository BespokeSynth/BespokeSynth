//
//  DropdownList.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/18/12.
//
//

#include "DropdownList.h"
#include "SynthGlobals.h"
#include "FileStream.h"
#include "ModularSynth.h"
#include "Push2Control.h"

namespace
{
   const int itemSpacing = 15;
}

DropdownList::DropdownList(IDropdownListener* owner, const char* name, int x, int y, int* var, float width)
: mWidth(35)
, mHeight(itemSpacing)
, mColumns(1)
, mVar(var)
, mModalList(this)
, mOwner(owner)
, mMaxPerColumn(40)
, mModalWidth(20)
, mUnknownItemString("-----")
, mDrawLabel(false)
, mSliderVal(0)
, mAutoCalculateWidth(false)
, mDrawTriangle(true)
, mLastScrolledTime(-9999)
{
   assert(owner);
   SetName(name);
   mLabelSize = GetStringWidth(name) + 3;
   SetPosition(x,y);
   SetParent(dynamic_cast<IClickable*>(owner));
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   
   if (width == -1)
      mAutoCalculateWidth = true;
   else
      mWidth = width;
}

DropdownList::DropdownList(IDropdownListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, float width)
: DropdownList(owner, name, -1, -1, var, width)
{
   PositionTo(anchor, anchorDirection);
}

DropdownList::~DropdownList()
{
}

void DropdownList::AddLabel(string label, int value)
{
   DropdownListElement element;
   element.mLabel = label;
   element.mValue = value;
   mElements.push_back(element);

   CalculateWidth();
   mHeight = itemSpacing;
   
   CalcSliderVal();
}

void DropdownList::CalculateWidth()
{
   mModalWidth = mWidth;
   for (int i=0; i<mElements.size(); ++i)
   {
      int width = GetStringWidth(mElements[i].mLabel) + 15;
      if (width > mModalWidth)
         mModalWidth = width;
   }
   
   if (mAutoCalculateWidth)
      mWidth = MIN(mModalWidth, 180);
   
   mColumns = 1 + ((int)mElements.size()-1) / mMaxPerColumn;
   mModalList.SetDimensions(mModalWidth*mColumns, itemSpacing * MIN((int)mElements.size(), mMaxPerColumn));
}

string DropdownList::GetLabel(int val) const
{
   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i].mValue == val)
         return mElements[i].mLabel;
   }
   return "";
}

void DropdownList::Poll()
{
   if (*mVar != mLastSetValue)
      CalcSliderVal();
}

void DropdownList::Render()
{
   ofPushStyle();
   
   float xOffset = 0;
   if (mDrawLabel)
   {
      DrawTextNormal(Name(), mX, mY+12);
      xOffset = mLabelSize;
   }
   
   DrawBeacon(mX+mWidth/2+xOffset, mY+mHeight/2);

   float w,h;
   GetDimensions(w,h);

   ofColor color,textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX+1+xOffset,mY+1,w-xOffset,h);
   ofSetColor(color);
   ofRect(mX+xOffset,mY,w-xOffset,h);
   ofNoFill();

   ofSetColor(textColor);
   
   ofPushMatrix();
   ofClipWindow(mX, mY, w-12, h);
   DrawTextNormal(GetDisplayValue(*mVar), mX+2+xOffset, mY+12);
   ofPopMatrix();
   if (mDrawTriangle)
      ofTriangle(mX+w-11, mY+4, mX+w-3, mY+4, mX+w-7, mY+11);

   ofPopStyle();
   
   DrawHover();
   
   if (mLastScrolledTime + 300 > gTime && TheSynth->GetTopModalFocusItem() != &mModalList && !Push2Control::sDrawingPush2Display)
   {
      const float kCentering = 7;
      float w, h;
      GetPopupDimensions(w, h);
      ofPushMatrix();
      ofPushStyle();
      ofTranslate(mX, mY + kCentering - h * mSliderVal);
      mModalList.Render();
      ofFill();
      ofColor color = IDrawableModule::GetColor(GetModuleParent()->GetModuleType());
      color.a = 25;
      ofSetColor(color);
      ofRect(0,h * mSliderVal - kCentering,w,mHeight);
      ofPopStyle();
      ofPopMatrix();
   }
}

void DropdownList::DrawDropdown(int w, int h)
{
   ofPushStyle();

   ofSetColor(0,0,0);
   ofFill();
   ofRect(0,0,w,h);
   ofNoFill();
   for (int i=0; i<mElements.size(); ++i)
   {
      int col = i/mMaxPerColumn;
      
      if (mVar && mElements[i].mValue == *mVar)
         ofSetColor(255,255,0);
      else
         ofSetColor(255,255,255);
      
      DrawTextNormal(mElements[i].mLabel, 1+mModalWidth*col, (i%mMaxPerColumn)*itemSpacing+12);
   }
   ofSetColor(255,255,255);
   ofSetLineWidth(.5f);
   ofRect(0,0,w,h);

   ofPopStyle();
}

void DropdownList::GetDimensions(float& width, float& height)
{
   width = mWidth;
   if (mDrawLabel)
      width += mLabelSize;
   height = mHeight;
}

void DropdownList::DropdownClicked(int x, int y)
{
   int index = y/itemSpacing + x/mModalWidth * mMaxPerColumn;
   if (index >= 0 && index < mElements.size())
      SetIndex(index, K(forceUpdate));
}

void DropdownList::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mOwner->DropdownClicked(this);
   
   if (mElements.empty())
      return;

   float thisx,thisy;
   GetPosition(thisx,thisy);
   if (mDrawLabel)
      thisx += mLabelSize;
   mModalList.SetPosition(thisx,thisy+itemSpacing);
   TheSynth->PushModalFocusItem(&mModalList);
}

bool DropdownList::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   return false;
}

void DropdownList::MouseReleased()
{
}

void DropdownList::Clear()
{
   mElements.clear();
   if (mAutoCalculateWidth)
      mWidth = 35;
   mHeight = itemSpacing;
}

void DropdownList::SetFromMidiCC(float slider)
{
   slider = ofClamp(slider,0,1);
   SetIndex(int(slider*mElements.size()));
   mSliderVal = slider;
   mLastSetValue = *mVar;
   
   mLastScrolledTime = gTime;
}

float DropdownList::GetValueForMidiCC(float slider) const
{
   if (mElements.empty())
      return 0;
   
   int index = int(slider*mElements.size());
   index = ofClamp(index,0,mElements.size()-1);
   return mElements[index].mValue;
}

void DropdownList::SetIndex(int i, bool forceUpdate /*= false*/)
{
   if (mElements.empty())
      return;
   
   i = ofClamp(i,0,mElements.size()-1);
   
   SetValue(mElements[i].mValue, forceUpdate);
}

void DropdownList::SetValue(float value)
{
   SetValue((int)value, false);
}

void DropdownList::SetValue(int value, bool forceUpdate)
{
   if (value != *mVar || forceUpdate)
   {
      int oldVal = *mVar;
      *mVar = value;
      CalcSliderVal();
      gControlTactileFeedback = 1;
      mOwner->DropdownUpdated(this, oldVal);
   }
}

float DropdownList::GetValue() const
{
   return *mVar;
}

float DropdownList::GetMidiValue()
{
   return mSliderVal;
}

int DropdownList::FindItemIndex(float val) const
{
   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i].mValue == val)
         return i;
   }
   
   return -1;
}

string DropdownList::GetDisplayValue(float val) const
{
   int itemIndex = FindItemIndex(val);
   
   if (itemIndex >= 0 && itemIndex < mElements.size())
      return mElements[itemIndex].mLabel;
   else
      return mUnknownItemString.c_str();
}

void DropdownList::CalcSliderVal()
{
   int itemIndex = FindItemIndex(*mVar);
   
   mSliderVal = ofMap(itemIndex + .5f, 0, mElements.size(), 0, 1);

   mLastSetValue = *mVar;
}

void DropdownList::Increment(float amount)
{
   int itemIndex = FindItemIndex(*mVar);
   
   SetIndex(itemIndex + (int)amount);
}

EnumMap DropdownList::GetEnumMap()
{
   EnumMap ret;
   for (int i=0; i<mElements.size(); ++i)
      ret[mElements[i].mLabel] = mElements[i].mValue;
   return ret;
}

namespace
{
   const int kSaveStateRev = 1;
}

void DropdownList::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << GetLabel(*mVar);
}

void DropdownList::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   if (rev < 1)
   {
      float var;
      in >> var;
      if (shouldSetValue)
         SetValueDirect(var);
   }
   else
   {
      string label;
      in >> label;
      if (shouldSetValue)
      {
         for (int i=0; i<mElements.size(); ++i)
         {
            if (mElements[i].mLabel == label)
            {
               SetIndex(i);
               break;
            }
         }
      }
   }
}

void DropdownListModal::DrawModule()
{
   mOwner->DrawDropdown(mWidth, mHeight);
}

void DropdownListModal::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mOwner->DropdownClicked(x,y);
   TheSynth->PopModalFocusItem();
}
