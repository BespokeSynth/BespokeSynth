//
//  UIGrid.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/13.
//
//

#include "UIGrid.h"
#include "SynthGlobals.h"
#include "FileStream.h"

UIGrid::UIGrid(int x, int y, int w, int h, int cols, int rows, IClickable* parent)
: mClick(false)
, mWidth(w)
, mHeight(h)
, mMajorCol(-1)
, mSingleColumn(false)
, mFlip(false)
, mStrength(1)
, mCurrentHover(-1)
, mListener(nullptr)
, mGridMode(kNormal)
, mHoldCol(0)
, mHoldRow(0)
, mRestrictDragToRow(false)
, mClickClearsToZero(true)
, mHighlightCol(-1)
, mShouldDrawValue(false)
, mMomentary(false)
{
   SetName("grid");
   SetPosition(x,y);
   SetGrid(cols,rows);
   Clear();
   SetParent(parent);
   bzero(mDrawOffset, MAX_GRID_SIZE*sizeof(float));
}

UIGrid::~UIGrid()
{
}

void UIGrid::Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent)
{
   mWidth = w;
   mHeight = h;
   SetPosition(x,y);
   SetGrid(cols,rows);
   Clear();
   SetParent(parent);
   bzero(mDrawOffset, MAX_GRID_SIZE*sizeof(float));
}

void UIGrid::Render()
{
   ofPushStyle();
   float w,h;
   GetDimensions(w,h);
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;
   for (int j=0; j<mRows; ++j)
   {
      for (int i=0; i<mCols; ++i)
      {
         float x = GetX(i,j);
         float y = GetY(j);

         float data = mData[j][i];
         if (data)
         {
            ofFill();
            if (mGridMode == kNormal)
            {
               ofSetColor(255 * data, 255 * data, 255 * data, gModuleDrawAlpha);
               ofRect(x,y,xsize,ysize);
            }
            float fillAmount = ofClamp(ofLerp(.15f, 1, data), 0, 1);
            if (mGridMode == kMultislider)
            {
               float fadeAmount = ofClamp(ofLerp(.5f, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofRect(x+.5f, y+.5f+(ysize*(1-fillAmount)), xsize-1, ysize*fillAmount-1, 0);
               /*ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofNoFill();
               ofRect(x+1,y+1,xsize-2,ysize-2, gCornerRoundness*.99f);*/
            }
            else if (mGridMode == kHorislider)
            {
               ofSetColor(255,255,255, gModuleDrawAlpha);
               ofRect(x, y, xsize*fillAmount, ysize);
            }
         }
      }
   }
   ofNoFill();
   ofSetColor(100,100,100, gModuleDrawAlpha);
   for (int j=0; j<mRows; ++j)
   {
      for (int i=0; i<mCols; ++i)
         ofRect(GetX(i,j), GetY(j), xsize, ysize);
   }
   ofNoFill();
   ofSetColor(255, 200, 100, gModuleDrawAlpha);
   for (int j=0; j<mRows; ++j)
   {
      for (int i=0; i<mCols; ++i)
      {
         if (mMajorCol > 0 && i % mMajorCol == 0)
            ofRect(GetX(i,j), GetY(j), xsize, ysize);
      }
   }
   if (mHighlightCol != -1)
   {
      ofNoFill();
      ofSetColor(0,255,0, gModuleDrawAlpha);
      for (int j=0; j<mRows; ++j)
         ofRect(GetX(mHighlightCol,j), GetY(j), xsize, ysize);
   }
   if (mCurrentHover != -1 && mShouldDrawValue)
   {
      ofSetColor(ofColor::grey, gModuleDrawAlpha);
      DrawTextNormal(ofToString(GetVal(mCurrentHover % mCols, mCurrentHover / mCols)), mX, mY+12);
   }
   ofPopStyle();
}

float UIGrid::GetX(int col, int row) const
{
   float xsize = float(mWidth) / mCols;
   return mX+(col+mDrawOffset[row])*xsize;
}

float UIGrid::GetY(int row) const
{
   float ysize = float(mHeight) / mRows;
   if (mFlip)
      return mHeight+mY-(row+1)*ysize;
   else
      return mY+row*ysize;
}

GridCell UIGrid::GetGridCellAt(float x, float y, float* clickHeight, float* clickWidth)
{
   if (mFlip)
      y = (mHeight-1) - y;
   
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;
   
   int col = ofClamp(x/xsize, 0, mCols-1);
   int row = ofClamp(y/ysize, 0, mRows-1);
   
   if (clickHeight)
   {
      *clickHeight = ofClamp(1 - (y/ysize - ofClamp((int)(y/ysize),0,mRows-1)),0,1);
      if (mFlip)
         *clickHeight = 1 - *clickHeight;
   }
   
   if (clickWidth)
   {
      *clickWidth = ofClamp(x/xsize - ofClamp((int)(x/xsize),0,mCols-1),0,1);
   }
   
   return GridCell(col,row);
}

ofVec2f UIGrid::GetCellPosition(int col, int row)
{
   ofVec2f ret;
   
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;
   
   ret.x = xsize * col;
   ret.y = ysize * row;
   
   if (mFlip)
      ret.y = (mHeight-1) - ret.y;
   
   return ret;
}

void UIGrid::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mClick = true;

   float clickHeight, clickWidth;
   GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
   float oldValue = mData[cell.mRow][cell.mCol];

   if (mGridMode == kMultislider)// || mGridMode == kHorislider)
   {
      if (mData[cell.mRow][cell.mCol] > 0 && mClickClearsToZero)
         mData[cell.mRow][cell.mCol] = 0;
      else
         mData[cell.mRow][cell.mCol] = mGridMode == kMultislider ? clickHeight : clickWidth;
   }
   else
   {
      float val = mStrength;
      
      if (mSingleColumn && mGridMode == kHorislider)
      {
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (mData[i][cell.mCol] != 0)
               val = mData[i][cell.mCol];
         }
      }
      
      if (mData[cell.mRow][cell.mCol] == mStrength && mClickClearsToZero)
         mData[cell.mRow][cell.mCol] = 0;
      else
         mData[cell.mRow][cell.mCol] = val;
   }
   if (mSingleColumn)
   {
      for (int i=0; i<MAX_GRID_SIZE; ++i)
      {
         if (i != cell.mRow)
            mData[i][cell.mCol] = 0;
      }
   }
   
   if (mListener)
      mListener->GridUpdated(this, cell.mCol, cell.mRow, mData[cell.mRow][cell.mCol], oldValue);

   mHoldVal = mData[cell.mRow][cell.mCol];
   mHoldCol = cell.mCol;
   mHoldRow = cell.mRow;
}

void UIGrid::MouseReleased()
{
   if (mClick && mMomentary)
   {
      float oldValue = mData[mHoldRow][mHoldCol];
      mData[mHoldRow][mHoldCol] = 0;
      mListener->GridUpdated(this, mHoldCol, mHoldRow, 0, oldValue);
   }
   
   mClick = false;
}

bool UIGrid::MouseMoved(float x, float y)
{
   bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);
   
   float clickHeight, clickWidth;
   GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
   if (isMouseOver)
      mCurrentHover = cell.mCol + cell.mRow * mCols;
   else
      mCurrentHover = -1;

   if (mClick && !mMomentary)
   {
      if (mRestrictDragToRow)
      {
         if (cell.mRow > mHoldRow)
            clickHeight = mFlip ? 1 : 0;
         if (cell.mRow < mHoldRow)
            clickHeight = mFlip ? 0 : 1;
         cell.mRow = mHoldRow;
      }
      
      float oldValue = mData[cell.mRow][cell.mCol];
      
      if (mGridMode == kMultislider && mHoldVal != 0)
      {
         mData[cell.mRow][cell.mCol] = clickHeight;
      }
      else if (mGridMode == kHorislider && mSingleColumn)
      {
         float val = mHoldVal;
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (mData[i][cell.mCol] != 0)
               val = mData[i][cell.mCol];
         }
         
         mData[cell.mRow][cell.mCol] = val;
      }
      else
      {
         mData[cell.mRow][cell.mCol] = mHoldVal;
      }
      
      if (mSingleColumn)
      {
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (i != cell.mRow)
               mData[i][cell.mCol] = 0;
         }
      }
      
      if (mListener)
         mListener->GridUpdated(this, cell.mCol, cell.mRow, mData[cell.mRow][cell.mCol], oldValue);
   }
   
   return false;
}

bool UIGrid::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (mGridMode == kMultislider || mGridMode == kHorislider)
   {
      bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);
      
      float clickHeight, clickWidth;
      GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
      if (isMouseOver)
      {
         float& data = mData[cell.mRow][cell.mCol];
         if (!mSingleColumn || data > 0)
         {
            float oldValue = data;
            data = ofClamp(data - scrollY / 100, FLT_EPSILON, 1);
            if (mListener)
               mListener->GridUpdated(this, cell.mCol, cell.mRow, data, oldValue);
         }
      }
   }
   
   return false;
}

void UIGrid::SetGrid(int cols, int rows)
{
   cols = ofClamp(cols, 0, MAX_GRID_SIZE);
   rows = ofClamp(rows, 0, MAX_GRID_SIZE);
   mRows = rows;
   mCols = cols;
}

void UIGrid::Clear()
{
   bzero(mData, MAX_GRID_SIZE*MAX_GRID_SIZE*sizeof(int));
}

float& UIGrid::GetVal(int col, int row)
{
   col = ofClamp(col, 0, MAX_GRID_SIZE-1);
   row = ofClamp(row, 0, MAX_GRID_SIZE-1);
   return mData[row][col];
}

void UIGrid::SetVal(int col, int row, float val, bool notifyListener)
{
   col = ofClamp(col, 0, MAX_GRID_SIZE-1);
   row = ofClamp(row, 0, MAX_GRID_SIZE-1);
   if (val != mData[row][col])
   {
      float oldValue = mData[row][col];
      mData[row][col] = val;
      
      if (mSingleColumn)
      {
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (i != row)
               mData[i][col] = 0;
         }
      }
      
      if (notifyListener && mListener)
         mListener->GridUpdated(this, col, row, val, oldValue);
   }
}

float UIGrid::GetValRefactor(int row, int col)
{
   return GetVal(col,row);
}

void UIGrid::SetValRefactor(int row, int col, float val)
{
   SetVal(col, row, val);
}

namespace
{
   const int kSaveStateRev = 1;
}

void UIGrid::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   for (int i=0; i<MAX_GRID_SIZE; ++i)
   {
      for (int j=0; j<MAX_GRID_SIZE; ++j)
      {
         out << mData[i][j];
      }
   }
}

void UIGrid::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   int gridSize = MAX_GRID_SIZE;
   
   if (rev < 1)
      gridSize = 100;
   
   for (int i=0; i<gridSize; ++i)
   {
      for (int j=0; j<gridSize; ++j)
      {
         float oldVal = mData[i][j];
         in >> mData[i][j];
         if (mListener)
            mListener->GridUpdated(this, j, i, mData[i][j], oldVal);
      }
   }
}
