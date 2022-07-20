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
//  DropdownList.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/18/12.
//
//

#ifndef __modularSynth__DropdownList__
#define __modularSynth__DropdownList__

#include <iostream>
#include "IUIControl.h"
#include "IDrawableModule.h"
#include "ClickButton.h"

struct DropdownListElement
{
   std::string mLabel;
   int mValue{ 0 };
};

class DropdownList;

class IDropdownListener
{
public:
   virtual ~IDropdownListener() {}
   virtual void DropdownClicked(DropdownList* list) {}
   virtual void DropdownUpdated(DropdownList* list, int oldVal) = 0;
};

class DropdownListModal : public IDrawableModule, public IButtonListener
{
public:
   DropdownListModal(DropdownList* owner) { mOwner = owner; }

   void CreateUIControls() override;

   void SetUpModal();
   void DrawModule() override;
   void SetDimensions(int w, int h)
   {
      mWidth = w;
      mHeight = h;
   }
   bool HasTitleBar() const override { return false; }

   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool ShouldClipContents() override { return false; }
   DropdownList* GetOwner() const { return mOwner; }
   bool MouseMoved(float x, float y) override;
   std::string GetHoveredLabel();
   float GetMouseX() { return mMouseX; }
   float GetMouseY() { return mMouseY; }
   void SetShowPagingControls(bool show);
   void SetIsScrolling(bool scrolling) { mIsScrolling = scrolling; }

   void ButtonClicked(ClickButton* button) override;

private:
   void OnClicked(float x, float y, bool right) override;
   int mWidth{ 1 };
   int mHeight{ 1 };
   int mColumnWidth{ 1 };
   float mMouseX{ -1 };
   float mMouseY{ -1 };
   DropdownList* mOwner;
   ClickButton* mPagePrevButton{ nullptr };
   ClickButton* mPageNextButton{ nullptr };
   bool mIsScrolling{ false };
};

class DropdownList : public IUIControl
{
public:
   DropdownList(IDropdownListener* owner, const char* name, int x, int y, int* var, float width = -1);
   DropdownList(IDropdownListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, float width = -1);
   void AddLabel(std::string label, int value);
   void RemoveLabel(int value);
   std::string GetLabel(int val) const;
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void DrawDropdown(int w, int h, bool isScrolling);
   bool DropdownClickedAt(int x, int y);
   void SetIndex(int i, bool forceUpdate = false);
   void Clear();
   void SetVar(int* var) { mVar = var; }
   EnumMap GetEnumMap();
   void SetUnknownItemString(std::string str)
   {
      mUnknownItemString = str;
      CalculateWidth();
   }
   void DrawLabel(bool draw) { mDrawLabel = draw; }
   void SetWidth(int width);
   void SetDrawTriangle(bool draw) { mDrawTriangle = draw; }
   void GetPopupDimensions(float& width, float& height) { mModalList.GetDimensions(width, height); }
   void SetMaxPerColumn(int max)
   {
      mMaxPerColumn = max;
      CalculateWidth();
   }
   int GetItemIndexAt(int x, int y);
   DropdownListElement GetElement(int index) { return mElements[index]; }
   DropdownListModal* GetModalDropdown() { return &mModalList; }
   float GetMaxItemWidth() const { return mMaxItemWidth; }
   void ChangePage(int direction);
   void AddSeparator(int index) { mSeparators.push_back(index); }
   void ClearSeparators() { mSeparators.clear(); }

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   float GetMidiValue() const override;
   int GetNumValues() override { return (int)mElements.size(); }
   std::string GetDisplayValue(float val) const override;
   bool InvertScrollDirection() override { return true; }
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   void GetDimensions(float& width, float& height) override;

   static constexpr int kItemSpacing = 15;
   static constexpr int kPageBarSpacing = 20;

protected:
   ~DropdownList(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void CalcSliderVal();
   int FindItemIndex(float val) const;
   void SetValue(int value, bool forceUpdate);
   void CalculateWidth();
   ofVec2f GetModalListPosition() const;

   int mWidth;
   int mHeight;
   int mMaxItemWidth;
   int mMaxPerColumn;
   int mDisplayColumns{ 1 };
   int mTotalColumns{ 1 };
   int mCurrentPagedColumn{ 0 };
   std::vector<DropdownListElement> mElements;
   int* mVar;
   DropdownListModal mModalList;
   IDropdownListener* mOwner;
   std::string mUnknownItemString;
   bool mDrawLabel;
   float mLabelSize;
   float mSliderVal;
   int mLastSetValue;
   bool mAutoCalculateWidth;
   bool mDrawTriangle;
   double mLastScrolledTime;
   std::vector<int> mSeparators;
};

#endif /* defined(__modularSynth__DropdownList__) */
