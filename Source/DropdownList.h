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

struct DropdownListElement
{
   string mLabel;
   int mValue;
};

class DropdownList;

class IDropdownListener
{
public:
   virtual ~IDropdownListener() {}
   virtual void DropdownClicked(DropdownList* list) {}
   virtual void DropdownUpdated(DropdownList* list, int oldVal) = 0;
};

class DropdownListModal : public IDrawableModule
{
public:
   DropdownListModal(DropdownList* owner) { mOwner = owner; }
   void DrawModule() override;
   void SetDimensions(int w, int h) { mWidth = w; mHeight = h; }
   bool HasTitleBar() const override { return false; }
   string GetTitleLabel() override { return ""; }
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
private:
   void OnClicked(int x, int y, bool right) override;
   int mWidth;
   int mHeight;
   int mColumnWidth;
   DropdownList* mOwner;
};

class DropdownList : public IUIControl
{
public:
   DropdownList(IDropdownListener* owner, const char* name, int x, int y, int* var, float width = -1);
   DropdownList(IDropdownListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, float width = -1);
   void AddLabel(string label, int value);
   string GetLabel(int val) const;
   void Render() override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void DrawDropdown(int w, int h);
   void DropdownClicked(int x, int y);
   void SetIndex(int i, bool forceUpdate = false);
   void Clear();
   void SetVar(int* var) { mVar = var; }
   EnumMap GetEnumMap();
   void SetUnknownItemString(string str) { mUnknownItemString = str; CalculateWidth(); }
   void DrawLabel(bool draw) { mDrawLabel = draw; }
   void SetWidth(int width) { mWidth = width; }
   void SetDrawTriangle(bool draw) { mDrawTriangle = draw; }
   void GetPopupDimensions(float& width, float& height) { mModalList.GetDimensions(width, height); }
   void SetMaxPerColumn(int max) { mMaxPerColumn = max; CalculateWidth(); }

   //IUIControl
   void SetFromMidiCC(float slider) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   float GetMidiValue() override;
   int GetNumValues() override { return (int)mElements.size(); }
   string GetDisplayValue(float val) const override;
   bool InvertScrollDirection() override { return true; }
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
   void GetDimensions(float& width, float& height) override;
   
protected:
   ~DropdownList();   //protected so that it can't be created on the stack

private:
   void OnClicked(int x, int y, bool right) override;
   void CalcSliderVal();
   int FindItemIndex(float val) const;
   void SetValue(int value, bool forceUpdate);
   void CalculateWidth();
   int mWidth;
   int mHeight;
   int mModalWidth;
   int mColumns;
   int mMaxPerColumn;
   vector<DropdownListElement> mElements;
   int* mVar;
   DropdownListModal mModalList;
   IDropdownListener* mOwner;
   string mUnknownItemString;
   bool mDrawLabel;
   float mLabelSize;
   float mSliderVal;
   int mLastSetValue;
   bool mAutoCalculateWidth;
   bool mDrawTriangle;
   double mLastScrolledTime;
};

#endif /* defined(__modularSynth__DropdownList__) */
