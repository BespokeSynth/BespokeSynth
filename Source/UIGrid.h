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
   void SetHighlightCol(int col) { mHighlightCol = col; }
   int GetHighlightCol() const { return mHighlightCol; }
   void SetMajorColSize(int size) { mMajorCol = size; }
   void SetSingleColumnMode(int set) { mSingleColumn = set; }
   void Clear();
   void SetFlip(bool flip) { mFlip = flip; }
   void SetStrength(float strength) { mStrength = strength; }
   int CurrentHover() { return mCurrentHover; }
   void SetListener(UIGridListener* listener) { mListener = listener; }
   void SetDrawOffset(int row, float amount) { mDrawOffset[row] = amount; }
   void SetDimensions(int width, int height) { mWidth = width; mHeight = height; }
   int GetWidth() const { return mWidth; }
   int GetHeight() const { return mHeight; }
   void SetRestrictDragToRow(bool set) { mRestrictDragToRow = set; }
   void SetClickClearsToZero(bool set) { mClickClearsToZero = set; }
   void SetShouldDrawValue(bool draw) { mShouldDrawValue = draw; }
   void SetMomentary(bool momentary) { mMomentary = momentary; }
   
   enum GridMode
   {
      kNormal,
      kMultislider,
      kHorislider
   };
   void SetGridMode(GridMode mode) { mGridMode = mode; }
   
   GridCell GetGridCellAt(float x, float y, float* clickHeight = nullptr, float* clickWidth = nullptr);
   ofVec2f GetCellPosition(int col, int row);

   //IUIControl
   void SetFromMidiCC(float slider) override {}
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
   
   float GetX(int col, int row) const;
   float GetY(int row) const;
   
   int mWidth;
   int mHeight;
   int mRows;
   int mCols;
   bool mClick;
   float mHoldVal;
   int mHoldCol;
   int mHoldRow;
   float mData[MAX_GRID_SIZE][MAX_GRID_SIZE];
   int mHighlightCol;
   int mMajorCol;
   bool mSingleColumn;
   bool mFlip;
   float mStrength;
   int mCurrentHover;
   UIGridListener* mListener;
   float mDrawOffset[MAX_GRID_SIZE];
   GridMode mGridMode;
   bool mRestrictDragToRow;
   bool mClickClearsToZero;
   bool mShouldDrawValue;
   bool mMomentary;
};

#endif /* defined(__modularSynth__Grid__) */
