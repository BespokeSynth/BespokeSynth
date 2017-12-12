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

namespace
{
   const int itemSpacing = 15;
}

DropdownList::DropdownList(IDropdownListener* owner, const char* name, int x, int y, int* var, float width)
: mWidth(40)
, mHeight(itemSpacing)
, mColumns(1)
, mVar(var)
, mModalList(this)
, mOwner(owner)
, mMaxPerColumn(40)
, mModalWidth(20)
, mUnknownItemString("???")
, mSliderVal(0)
, mAutoCalculateWidth(false)
{
   assert(owner);
   SetName(name);
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
   mColumns = 1 + ((int)mElements.size()-1) / mMaxPerColumn;
   
   mModalList.SetDimensions(mModalWidth*mColumns, itemSpacing * MIN((int)mElements.size(), mMaxPerColumn));
   
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
   
   DrawBeacon(mX+mWidth/2, mY+mHeight/2);

   int w,h;
   GetDimensions(w,h);

   ofColor color,textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX+1,mY+1,mWidth,mHeight);
   ofSetColor(color);
   ofRect(mX,mY,w,h);
   ofNoFill();

   ofSetColor(textColor);
   
   DrawText(GetDisplayValue(*mVar), mX+2, mY+12);
   ofTriangle(mX+w-11, mY+4, mX+w-3, mY+4, mX+w-7, mY+11);
   
   if (mDescription != "")
      DrawText(mDescription, mX - 3 - GetStringWidth(mDescription), mY+12);

   ofPopStyle();
   
   DrawHover();
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
      
      DrawText(mElements[i].mLabel, 1+mModalWidth*col, (i%mMaxPerColumn)*itemSpacing+12);
   }
   ofSetColor(255,255,255);
   ofSetLineWidth(.5f);
   ofRect(0,0,w,h);

   ofPopStyle();
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

   int thisx,thisy;
   GetPosition(thisx,thisy);
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
      mWidth = 40;
   mHeight = itemSpacing;
}

void DropdownList::SetFromMidiCC(float slider)
{
   slider = ofClamp(slider,0,1);
   SetIndex(int(slider*mElements.size()));
   mSliderVal = slider;
   mLastSetValue = *mVar;
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
   
   if (itemIndex != -1)
      mSliderVal = ofMap(itemIndex, 0, mElements.size(), 0, 1);
   else
      mSliderVal = -1;
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
