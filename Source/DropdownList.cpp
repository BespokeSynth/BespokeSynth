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
: mVar(var)
, mLastSetValue(*var)
, mModalList(this)
, mOwner(owner)
{
   assert(owner);
   SetName(name);
   mLabelSize = GetStringWidth(name) + 3;
   SetPosition(x, y);
   SetParent(dynamic_cast<IClickable*>(owner));
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);

   mModalList.SetTypeName("dropdownlist", kModuleCategory_Other);

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

void DropdownList::SetLabel(std::string label, int value)
{
   bool found = false;
   for (auto iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      if (iter->mValue == value)
      {
         found = true;
         iter->mLabel = label;

         CalculateWidth();
         mHeight = kItemSpacing;

         CalcSliderVal();
         break;
      }
   }

   if (!found)
      AddLabel(label, value);
}

void DropdownList::CalculateWidth()
{
   mMaxItemWidth = mWidth;
   for (int i = 0; i < mElements.size(); ++i)
   {
      int width = GetStringWidth(mElements[i].mLabel) + (mDrawTriangle ? 15 : 3);
      if (width > mMaxItemWidth)
         mMaxItemWidth = width;
   }

   if (mAutoCalculateWidth)
      mWidth = MIN(mMaxItemWidth, 180);
}

void DropdownList::SetWidth(int width)
{
   if (width != mWidth)
   {
      mWidth = width;
      CalculateWidth();
   }
}

std::string DropdownList::GetLabel(int val) const
{
   for (int i = 0; i < mElements.size(); ++i)
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

   mDropdownIsOpen = (TheSynth->GetTopModalFocusItem() == &mModalList);
}

void DropdownList::Render()
{
   ofPushStyle();

   float w, h;
   GetDimensions(w, h);

   ofColor color, textColor;
   IUIControl::GetColors(color, textColor);
   float xOffset = 0;

   if (mDisplayStyle == DropdownDisplayStyle::kNormal)
   {
      if (mDrawLabel)
      {
         DrawTextNormal(Name(), mX, mY + 12);
         xOffset = mLabelSize;
      }

      DrawBeacon(mX + mWidth / 2 + xOffset, mY + mHeight / 2);

      ofFill();
      ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
      ofRect(mX + 1 + xOffset, mY + 1, w - xOffset, h);
      ofSetColor(color);
      ofRect(mX + xOffset, mY, w - xOffset, h);
      ofNoFill();

      ofSetColor(textColor);

      ofPushMatrix();
      ofClipWindow(mX, mY, w - (mDrawTriangle ? 12 : 0), h, true);
      DrawTextNormal(GetDisplayValue(*mVar), mX + 2 + xOffset, mY + 12);
      ofPopMatrix();
      if (mDrawTriangle)
      {
         ofSetLineWidth(.5f);
         ofTriangle(mX + w - 11, mY + 4, mX + w - 3, mY + 4, mX + w - 7, mY + 11);
      }
   }
   else if (mDisplayStyle == DropdownDisplayStyle::kHamburger)
   {
      ofSetColor(textColor);
      ofSetLineWidth(1.0f);
      ofLine(mX + 6, mY + 4.5f, mX + 14, mY + 4.5f);
      ofLine(mX + 6, mY + 7.5f, mX + 14, mY + 7.5f);
      ofLine(mX + 6, mY + 10.5f, mX + 14, mY + 10.5f);
   }

   ofPopStyle();

   DrawHover(mX + xOffset, mY, w - xOffset, h);

   if (mLastScrolledTime + 300 > gTime && TheSynth->GetTopModalFocusItem() != &mModalList && !Push2Control::sDrawingPush2Display)
   {
      const float kCentering = 7;
      float pw, ph;
      GetPopupDimensions(pw, ph);

      mModalList.SetPosition(0, 0);
      ofPushMatrix();
      ofTranslate(mX, mY + kCentering - ph * mSliderVal);
      mModalList.SetIsScrolling(true);
      mModalList.Render();
      mModalList.SetIsScrolling(false);
      ofPopMatrix();

      ofPushStyle();
      ofFill();
      ofColor categoryColor = IDrawableModule::GetColor(GetModuleParent()->GetModuleCategory());
      categoryColor.a = 80;
      ofSetColor(categoryColor);
      ofRect(mX, mY, pw, mHeight);
      ofPopStyle();
   }
}

void DropdownList::DrawDropdown(int w, int h, bool isScrolling)
{
   ofPushStyle();

   if (isScrolling)
      mModalList.SetDimensions(mMaxItemWidth, (int)mElements.size() * kItemSpacing);

   int hoverIndex = -1;
   float dropdownW, dropdownH;
   mModalList.GetDimensions(dropdownW, dropdownH);
   if (mModalList.GetMouseX() >= 0 && mModalList.GetMouseY() >= 0 && mModalList.GetMouseX() < dropdownW && mModalList.GetMouseY() < dropdownH)
      hoverIndex = GetItemIndexAt(mModalList.GetMouseX(), mModalList.GetMouseY());

   int maxPerColumn = mMaxPerColumn;
   int displayColumns = mDisplayColumns;
   int totalColumns = mTotalColumns;
   int currentPagedColumn = mCurrentPagedColumn;
   if (isScrolling)
   {
      maxPerColumn = 9999;
      displayColumns = 1;
      totalColumns = 1;
      mCurrentPagedColumn = 0;
   }

   bool paged = (displayColumns < totalColumns);
   float pageHeaderShift = (paged ? kPageBarSpacing : 0);
   mModalList.SetShowPagingControls(paged);

   ofSetColor(0, 0, 0);
   ofFill();
   ofRect(0, 0, w, h);
   for (int i = 0; i < mElements.size(); ++i)
   {
      int col = i / maxPerColumn - currentPagedColumn;

      if (col < 0)
         continue;

      if (col >= displayColumns)
         break;

      if (i == hoverIndex)
      {
         ofSetColor(100, 100, 100, 100);
         ofRect(mMaxItemWidth * col, (i % maxPerColumn) * kItemSpacing + pageHeaderShift, mMaxItemWidth, kItemSpacing);
      }

      if (VectorContains(i, mSeparators))
      {
         ofPushStyle();
         ofSetColor(100, 100, 100, 100);
         ofSetLineWidth(1);
         ofLine(mMaxItemWidth * col + 3, (i % maxPerColumn) * kItemSpacing + pageHeaderShift, mMaxItemWidth * (col + 1) - 3, (i % maxPerColumn) * kItemSpacing + pageHeaderShift);
         ofPopStyle();
      }

      if (mVar && mElements[i].mValue == *mVar)
         ofSetColor(255, 255, 0);
      else
         ofSetColor(255, 255, 255);

      DrawTextNormal(mElements[i].mLabel, 1 + mMaxItemWidth * col, (i % maxPerColumn) * kItemSpacing + 12 + pageHeaderShift);
   }
   ofSetColor(255, 255, 255);

   if (paged)
   {
      DrawTextNormal("page " + ofToString(mCurrentPagedColumn / displayColumns + 1) + "/" + ofToString(totalColumns / displayColumns + 1), 30, 14);
   }

   ofSetLineWidth(.5f);
   ofNoFill();
   ofRect(0, 0, w, h);

   ofPopStyle();
}

void DropdownList::ChangePage(int direction)
{
   int newColumn = mCurrentPagedColumn + direction * mDisplayColumns;
   if (newColumn >= 0 && newColumn < ceil(float(mTotalColumns) / mDisplayColumns) * mDisplayColumns)
      mCurrentPagedColumn = newColumn;
}

void DropdownList::CopyContentsTo(DropdownList* list) const
{
   list->Clear();
   for (auto& element : mElements)
      list->AddLabel(element.mLabel, element.mValue);
}

void DropdownList::GetDimensions(float& width, float& height)
{
   width = mWidth;
   if (mDrawLabel)
      width += mLabelSize;
   height = mHeight;
}

bool DropdownList::DropdownClickedAt(int x, int y)
{
   int index = GetItemIndexAt(x, y);
   if (index >= 0 && index < mElements.size())
   {
      SetIndex(index, NextBufferTime(false), K(forceUpdate));
      return true;
   }
   return false;
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

void DropdownList::OnClicked(float x, float y, bool right)
{
   if (right || mDropdownIsOpen)
      return;

   mOwner->DropdownClicked(this);

   if (mElements.empty())
      return;

   mModalList.SetUpModal();

   ofVec2f modalPos = GetModalListPosition();
   mModalList.SetOwningContainer(GetModuleParent()->GetOwningContainer());

   float screenY = (modalPos.y + GetModuleParent()->GetOwningContainer()->GetDrawOffset().y) * GetModuleParent()->GetOwningContainer()->GetDrawScale();
   float maxX = ofGetWidth() - 5;
   float maxY = ofGetHeight() - 5;

   const int kMinPerColumn = 3;
   mMaxPerColumn = std::max(kMinPerColumn, int((maxY - screenY) / (kItemSpacing * GetModuleParent()->GetOwningContainer()->GetDrawScale()))) - 1;
   mTotalColumns = 1 + ((int)mElements.size() - 1) / mMaxPerColumn;
   int maxDisplayColumns = std::max(1, int((ofGetWidth() / GetModuleParent()->GetOwningContainer()->GetDrawScale()) / mMaxItemWidth));
   mDisplayColumns = std::min(mTotalColumns, maxDisplayColumns);
   mCurrentPagedColumn = 0;

   bool paged = (mDisplayColumns < mTotalColumns);

   ofVec2f modalDimensions(mMaxItemWidth * mDisplayColumns, kItemSpacing * std::min((int)mElements.size(), mMaxPerColumn + (paged ? 1 : 0)));
   modalPos.x = std::max(FromScreenPosX(5.0f, GetModuleParent()), std::min(modalPos.x, FromScreenPosX(maxX - modalDimensions.x * GetModuleParent()->GetOwningContainer()->GetDrawScale(), GetModuleParent())));
   mModalList.SetPosition(modalPos.x, modalPos.y);
   mModalList.SetDimensions(modalDimensions.x, modalDimensions.y);

   TheSynth->PushModalFocusItem(&mModalList);
}

int DropdownList::GetItemIndexAt(int x, int y)
{
   bool paged = (mDisplayColumns < mTotalColumns);
   int indexOffset = 0;
   if (paged)
   {
      y -= kPageBarSpacing;
      if (y < 0)
         return -1;
      indexOffset = mCurrentPagedColumn * mMaxPerColumn;
   }
   return y / kItemSpacing + x / mMaxItemWidth * mMaxPerColumn + indexOffset;
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

void DropdownList::SetFromMidiCC(float slider, double time, bool setViaModulator)
{
   slider = ofClamp(slider, 0, 1);
   SetIndex(int(slider * mElements.size()), time, false);
   mSliderVal = slider;
   mLastSetValue = *mVar;

   if (!setViaModulator)
      mLastScrolledTime = time; //don't do scrolling display if a modulator is changing our value
}

float DropdownList::GetValueForMidiCC(float slider) const
{
   if (mElements.empty())
      return 0;

   int index = int(slider * mElements.size());
   index = ofClamp(index, 0, mElements.size() - 1);
   return mElements[index].mValue;
}

bool DropdownList::CanBeTargetedBy(PatchCableSource* source) const
{
   if (source->GetConnectionType() == kConnectionType_Pulse)
      return true;
   return IUIControl::CanBeTargetedBy(source);
}

void DropdownList::OnPulse(double time, float velocity, int flags)
{
   int length = static_cast<int>(mElements.size());
   if (length <= 0)
      length = 1;
   int direction = 1;
   if (flags & kPulseFlag_Backward)
      direction = -1;
   if (flags & kPulseFlag_Repeat)
      direction = 0;

   int newindex = 0;
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i].mValue == *mVar)
      {
         newindex = i;
         break;
      }
   }
   newindex = (newindex + direction + length) % length;
   if (flags & kPulseFlag_Reset)
      newindex = 0;
   else if (flags & kPulseFlag_Random)
      newindex = gRandom() % length;
   if (newindex >= mElements.size() || newindex < 0)
      newindex = 0;
   SetIndex(newindex, time, K(forceUpdate));
}

void DropdownList::SetIndex(int i, double time, bool forceUpdate)
{
   if (mElements.empty())
      return;

   i = ofClamp(i, 0, mElements.size() - 1);

   SetValue(mElements[i].mValue, time, forceUpdate);
}

void DropdownList::SetValue(float value, double time, bool forceUpdate /*= false*/)
{
   int intValue = (int)value;
   if (intValue != *mVar || forceUpdate)
   {
      int oldVal = *mVar;
      *mVar = intValue;
      CalcSliderVal();
      gControlTactileFeedback = 1;
      mOwner->DropdownUpdated(this, oldVal, time);
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
   for (int i = 0; i < mElements.size(); ++i)
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

   SetIndex(itemIndex + (int)amount, gTime, false);
}

EnumMap DropdownList::GetEnumMap()
{
   EnumMap ret;
   for (int i = 0; i < mElements.size(); ++i)
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
         SetValueDirect(var, gTime);
   }
   else
   {
      std::string label;
      in >> label;
      if (shouldSetValue)
      {
         for (int i = 0; i < mElements.size(); ++i)
         {
            if (mElements[i].mLabel == label)
            {
               SetIndex(i, gTime, false);
               break;
            }
         }
      }
   }
}

void DropdownListModal::SetUpModal()
{
   if (mPagePrevButton == nullptr)
      CreateUIControls();
}

void DropdownListModal::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPagePrevButton = new ClickButton(this, " < ", 3, 3);
   mPageNextButton = new ClickButton(this, " > ", 100, 3);
}

void DropdownListModal::SetShowPagingControls(bool show)
{
   if (mPagePrevButton)
      mPagePrevButton->SetShowing(show);
   if (mPageNextButton)
      mPageNextButton->SetShowing(show);
}

void DropdownListModal::DrawModule()
{
   mOwner->DrawDropdown(mWidth, mHeight, mIsScrolling);

   if (mPagePrevButton)
      mPagePrevButton->Draw();
   if (mPageNextButton)
      mPageNextButton->Draw();
}

std::string DropdownListModal::GetHoveredLabel()
{
   int index = mOwner->GetItemIndexAt((int)mMouseX, (int)mMouseY);
   if (index >= 0 && index < mOwner->GetNumValues())
      return mOwner->GetElement(index).mLabel;
   return "";
}

bool DropdownListModal::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mMouseX = x;
   mMouseY = y;
   return false;
}

void DropdownListModal::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (mOwner->DropdownClickedAt(x, y))
      TheSynth->PopModalFocusItem();
}

void DropdownListModal::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPagePrevButton)
      mOwner->ChangePage(-1);
   if (button == mPageNextButton)
      mOwner->ChangePage(1);
}
