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

DropdownList::DropdownList(IDropdownListener* owner, const char* name, int x, int y, int* var, float width)
: mWidth(35)
, mHeight(kItemSpacing)
, mVar(var)
, mModalList(this)
, mOwner(owner)
, mMaxPerColumn(40)
, mMaxItemWidth(20)
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

   mModalList.SetTypeName("dropdownlist");
   
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

void DropdownList::AddLabel(std::string label, int value)
{
   DropdownListElement element;
   element.mLabel = label;
   element.mValue = value;
   mElements.push_back(element);

   CalculateWidth();
   mHeight = kItemSpacing;
   
   CalcSliderVal();
}

void DropdownList::RemoveLabel(int value)
{
   for (auto iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      if (iter->mValue == value)
      {
         mElements.erase(iter);
         
         CalculateWidth();
         mHeight = kItemSpacing;
         
         CalcSliderVal();
         break;
      }
   }
}

void DropdownList::CalculateWidth()
{
   mMaxItemWidth = mWidth;
   for (int i=0; i<mElements.size(); ++i)
   {
      int width = GetStringWidth(mElements[i].mLabel) + 15;
      if (width > mMaxItemWidth)
         mMaxItemWidth = width;
   }
   
   if (mAutoCalculateWidth)
      mWidth = MIN(mMaxItemWidth, 180);
}

std::string DropdownList::GetLabel(int val) const
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
   ofClipWindow(mX, mY, w-12, h, true);
   DrawTextNormal(GetDisplayValue(*mVar), mX+2+xOffset, mY+12);
   ofPopMatrix();
   if (mDrawTriangle)
   {
      ofSetLineWidth(.5f);
      ofTriangle(mX+w-11, mY+4, mX+w-3, mY+4, mX+w-7, mY+11);
   }

   ofPopStyle();
   
   DrawHover(mX+xOffset, mY, w-xOffset, h);
   
   if (mLastScrolledTime + 300 > gTime && TheSynth->GetTopModalFocusItem() != &mModalList && !Push2Control::sDrawingPush2Display && mElements.size() < mMaxPerColumn)
   {
      const float kCentering = 7;
      float w, h;
      GetPopupDimensions(w, h);
      
      mModalList.SetPosition(0, 0);
      ofPushMatrix();
      ofTranslate(mX, mY + kCentering - h * mSliderVal);
      mModalList.Render();
      ofPopMatrix();

      ofPushStyle();
      ofFill();
      ofColor color = IDrawableModule::GetColor(GetModuleParent()->GetModuleType());
      color.a = 80;
      ofSetColor(color);
      ofRect(mX, mY, w, mHeight);
      ofPopStyle();      
   }
}

void DropdownList::DrawDropdown(int w, int h)
{
   ofPushStyle();

   int hoverIndex = -1;
   float dropdownW, dropdownH;
   mModalList.GetDimensions(dropdownW, dropdownH);
   if (mModalList.GetMouseX() >= 0 && mModalList.GetMouseY() >= 0 && mModalList.GetMouseX() < dropdownW && mModalList.GetMouseY() < dropdownH)
      hoverIndex = GetItemIndex(mModalList.GetMouseX(), mModalList.GetMouseY());

   int maxPerColumn = mMaxPerColumn;
   if (Push2Control::sDrawingPush2Display)
      maxPerColumn = 9999;

   ofSetColor(0,0,0);
   ofFill();
   ofRect(0,0,w,h);
   for (int i=0; i<mElements.size(); ++i)
   {
      int col = i / maxPerColumn;
      
      if (i == hoverIndex)
      {
         ofSetColor(100, 100, 100, 100);
         ofRect(mMaxItemWidth * col, (i%maxPerColumn)*kItemSpacing, mMaxItemWidth, kItemSpacing);
      }

      if (mVar && mElements[i].mValue == *mVar)
         ofSetColor(255,255,0);
      else
         ofSetColor(255,255,255);
      
      DrawTextNormal(mElements[i].mLabel, 1+mMaxItemWidth*col, (i%maxPerColumn)*kItemSpacing+12);
   }
   ofSetColor(255,255,255);
   ofSetLineWidth(.5f);
   ofNoFill();
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
   int index = GetItemIndex(x,y);
   if (index >= 0 && index < mElements.size())
      SetIndex(index, K(forceUpdate));
}

namespace
{
   float ToScreenPosX(float x, IDrawableModule* parent)
   {
      return (x + parent->GetOwningContainer()->GetDrawOffset().x) * parent->GetOwningContainer()->GetDrawScale();
   }

   float ToScreenPosY(float y, IDrawableModule* parent)
   {
      return (y + parent->GetOwningContainer()->GetDrawOffset().y) * parent->GetOwningContainer()->GetDrawScale();
   }

   float FromScreenPosX(float x, IDrawableModule* parent)
   {
      return (x / parent->GetOwningContainer()->GetDrawScale()) - parent->GetOwningContainer()->GetDrawOffset().x;
   }
}

void DropdownList::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mOwner->DropdownClicked(this);
   
   if (mElements.empty())
      return;

   ofVec2f modalPos = GetModalListPosition();
   mModalList.SetOwningContainer(GetModuleParent()->GetOwningContainer());

   float screenX = ToScreenPosX(modalPos.x, GetModuleParent());
   float screenY = (modalPos.y + GetModuleParent()->GetOwningContainer()->GetDrawOffset().y) * GetModuleParent()->GetOwningContainer()->GetDrawScale();
   float maxX = ofGetWidth() - 5;
   float maxY = ofGetHeight() - 5;

   const int kMinPerColumn = 1;
   mMaxPerColumn = std::max(kMinPerColumn, int((maxY - screenY) / (kItemSpacing * GetModuleParent()->GetOwningContainer()->GetDrawScale())));

   int columns = 1 + ((int)mElements.size() - 1) / mMaxPerColumn;
   ofVec2f modalDimensions(mMaxItemWidth*columns, kItemSpacing * std::min((int)mElements.size(), mMaxPerColumn));
   modalPos.x = std::max(FromScreenPosX(5.0f, GetModuleParent()), std::min(modalPos.x, FromScreenPosX(maxX - modalDimensions.x * GetModuleParent()->GetOwningContainer()->GetDrawScale(), GetModuleParent())));
   mModalList.SetPosition(modalPos.x, modalPos.y);
   mModalList.SetDimensions(modalDimensions.x, modalDimensions.y);
   
   TheSynth->PushModalFocusItem(&mModalList);
}

int DropdownList::GetItemIndex(int x, int y)
{
   return y / kItemSpacing + x / mMaxItemWidth * mMaxPerColumn;
}

ofVec2f DropdownList::GetModalListPosition() const
{
   float thisx, thisy;
   GetPosition(thisx, thisy);
   if (mDrawLabel)
      thisx += mLabelSize;
   return ofVec2f(thisx, thisy + kItemSpacing);
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
   mHeight = kItemSpacing;
}

void DropdownList::SetFromMidiCC(float slider, bool setViaModulator /*= false*/)
{
   slider = ofClamp(slider,0,1);
   SetIndex(int(slider*mElements.size()));
   mSliderVal = slider;
   mLastSetValue = *mVar;
   
   if (!setViaModulator)
      mLastScrolledTime = gTime; //don't do scrolling display if a modulator is changing our value
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

float DropdownList::GetMidiValue() const
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

std::string DropdownList::GetDisplayValue(float val) const
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
      std::string label;
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

std::string DropdownListModal::GetHoveredLabel()
{
   int index = mOwner->GetItemIndex((int)mMouseX, (int)mMouseY);
   if (index >= 0 && index < mOwner->GetNumValues())
      return mOwner->GetElement(index).mLabel;
   return "";
}

bool DropdownListModal::MouseMoved(float x, float y)
{
   mMouseX = x;
   mMouseY = y;
   return false;
}

void DropdownListModal::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mOwner->DropdownClicked(x,y);
   TheSynth->PopModalFocusItem();
}
