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

#ifndef __modularSynth__Grid__
#define __modularSynth__Grid__

#include <iostream>
#include "IUIControl.h"
#include "SynthGlobals.h"

#define MAX_GRID_SIZE 128

class UIGrid;

class UIGridListener
{
public:
   virtual ~UIGridListener() {}
   virtual void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) = 0;
};

struct GridCell
{
   GridCell(int col, int row) : mCol(col), mRow(row) {}
   int mCol;
   int mRow;
};

class UIGrid : public IUIControl
{
public:
   UIGrid(int x, int y, int w, int h, int cols, int rows, IClickable* parent);
   void Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent);
   void SetGrid(int cols, int rows);
   int GetRows() { return mRows; }
   int GetCols() { return mCols; }
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   float& GetVal(int col, int row);
   void SetVal(int col, int row, float val, bool notifyListener = true);
   float GetValRefactor(int row, int col);
   void SetValRefactor(int row, int col, float val);
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
   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }
   float GetWidth() const { return mWidth; }
   float GetHeight() const { return mHeight; }
   void SetRestrictDragToRow(bool set) { mRestrictDragToRow = set; }
   void SetRequireShiftForMultislider(bool set) { mRequireShiftForMultislider = set; }
   void SetShouldDrawValue(bool draw) { mShouldDrawValue = draw; }
   void SetMomentary(bool momentary) { mMomentary = momentary; }
   const std::array<float, MAX_GRID_SIZE*MAX_GRID_SIZE>& GetData() const { return mData; }
   void SetData(std::array<float, MAX_GRID_SIZE*MAX_GRID_SIZE>& data) { mData = data; }
   
   enum GridMode
   {
      kNormal,
      kMultislider,
      kHorislider,
      kMultisliderBipolar
   };
   void SetGridMode(GridMode mode) { mGridMode = mode; }
   
   GridCell GetGridCellAt(float x, float y, float* clickHeight = nullptr, float* clickWidth = nullptr);
   ofVec2f GetCellPosition(int col, int row);

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override {}
   void SetValue(float value) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
protected:
   ~UIGrid();   //protected so that it can't be created on the stack
   
private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   
   int GetDataIndex(int col, int row) { return col + row * MAX_GRID_SIZE; }
   float GetX(int col, int row) const;
   float GetY(int row) const;
   bool CanAdjustMultislider() const;

   struct HighlightColBuffer
   {
      HighlightColBuffer() : time(0), col(-1) {}
      double time;
      int col;
   };
   
   float mWidth;
   float mHeight;
   int mRows;
   int mCols;
   bool mClick;
   float mHoldVal;
   int mHoldCol;
   int mHoldRow;
   bool mLastClickWasClear;
   std::array<float, MAX_GRID_SIZE*MAX_GRID_SIZE> mData;
   std::array<HighlightColBuffer, 10> mHighlightColBuffer;
   int mNextHighlightColPointer;
   int mMajorCol;
   bool mSingleColumn;
   bool mFlip;
   float mStrength;
   int mCurrentHover;
   UIGridListener* mListener;
   float mDrawOffset[MAX_GRID_SIZE];
   GridMode mGridMode;
   bool mRestrictDragToRow;
   bool mRequireShiftForMultislider;
   bool mShouldDrawValue;
   bool mMomentary;
};

#endif /* defined(__modularSynth__Grid__) */
