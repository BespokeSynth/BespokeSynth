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
//  UIGrid.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/13.
//
//

#pragma once

#include "IUIControl.h"
#include "SynthGlobals.h"

#define MAX_GRID_COLS 1024
#define MAX_GRID_ROWS 128

class UIGrid;

class UIGridListener
{
public:
   virtual ~UIGridListener() {}
   virtual void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) = 0;
};

struct GridCell
{
   GridCell(int col, int row)
   : mCol(col)
   , mRow(row)
   {}
   int mCol;
   int mRow;
};

class UIGrid : public IUIControl
{
public:
   UIGrid(IClickable* parent, std::string name, int x, int y, int w, int h, int cols, int rows);
   void Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent);
   void SetGrid(int cols, int rows);
   int GetRows() { return mRows; }
   int GetCols() { return mCols; }
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;
   float GetVal(int col, int row) const;
   float& GetVal(int col, int row);
   void SetVal(int col, int row, float val, bool notifyListener = true);
   void SetHighlightCol(double time, int col);
   int GetHighlightCol(double time) const;
   void SetMajorColSize(int size) { mMajorCol = size; }
   int GetMajorColSize() const { return mMajorCol; }
   void SetSingleColumnMode(bool set) { mSingleColumn = set; }
   void Clear();
   void SetFlip(bool flip) { mFlip = flip; }
   void SetStrength(float strength) { mStrength = strength; }
   int CurrentHover() { return mCurrentHover; }
   void SetListener(UIGridListener* listener) { mListener = listener; }
   void SetDrawOffset(int row, float amount) { mDrawOffset[row] = amount; }
   void SetDimensions(float width, float height)
   {
      mWidth = width;
      mHeight = height;
   }
   float GetWidth() const { return mWidth; }
   float GetHeight() const { return mHeight; }
   void SetRestrictDragToRow(bool set) { mRestrictDragToRow = set; }
   void SetRequireShiftForMultislider(bool set) { mRequireShiftForMultislider = set; }
   void SetShouldDrawValue(bool draw) { mShouldDrawValue = draw; }
   void SetMomentary(bool momentary) { mMomentary = momentary; }
   const std::array<float, MAX_GRID_COLS * MAX_GRID_ROWS>& GetData() const { return mData; }
   void SetData(std::array<float, MAX_GRID_COLS * MAX_GRID_ROWS>& data) { mData = data; }
   void SetClickValueSubdivisions(int subdivisions) { mClickSubdivisions = subdivisions; }
   float GetSubdividedValue(float position) const;
   bool GetNoHover() const override { return true; }
   bool CanBeTargetedBy(PatchCableSource* source) const override;
   void SetCanBeUIControlTarget(bool targetable) { mCanBeUIControlTarget = targetable; }

   enum GridMode
   {
      kNormal,
      kMultislider,
      kHorislider,
      kMultisliderBipolar,
      kMultisliderGrow
   };
   void SetGridMode(GridMode mode) { mGridMode = mode; }

   GridCell GetGridCellAt(float x, float y, float* clickHeight = nullptr, float* clickWidth = nullptr);
   ofVec2f GetCellPosition(int col, int row);

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   float GetValue() const override;
   float GetMidiValue() const override;
   std::string GetDisplayValue(float val) const override;
   bool IsSliderControl() override { return true; }
   bool IsButtonControl() override { return false; }

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

protected:
   ~UIGrid(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   int GetDataIndex(int col, int row) const { return col + row * MAX_GRID_COLS; }
   float GetX(int col, int row) const;
   float GetY(int row) const;
   bool CanAdjustMultislider() const;

   struct HighlightColBuffer
   {
      double time{ 0 };
      int col{ -1 };
   };

   float mWidth{ 200 };
   float mHeight{ 200 };
   int mRows{ 0 };
   int mCols{ 0 };
   bool mClick{ false };
   float mHoldVal;
   int mHoldCol{ 0 };
   int mHoldRow{ 0 };
   bool mLastClickWasClear{ false };
   std::array<float, MAX_GRID_COLS * MAX_GRID_ROWS> mData{};
   std::array<HighlightColBuffer, 10> mHighlightColBuffer{};
   int mNextHighlightColPointer{ 0 };
   int mMajorCol{ -1 };
   bool mSingleColumn{ false };
   bool mFlip{ false };
   float mStrength{ 1 };
   int mCurrentHover{ -1 };
   float mCurrentHoverAmount{ 1 };
   UIGridListener* mListener{ nullptr };
   std::array<float, MAX_GRID_ROWS> mDrawOffset{};
   GridMode mGridMode{ GridMode::kNormal };
   bool mRestrictDragToRow{ false };
   bool mRequireShiftForMultislider{ false };
   bool mShouldDrawValue{ false };
   bool mMomentary{ false };
   int mClickSubdivisions{ 1 };
   bool mCanBeUIControlTarget{ false };
   int mValueSetTargetCol{ 0 };
   int mValueSetTargetRow{ 0 };
};
