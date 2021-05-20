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
, mNextHighlightColPointer(0)
, mMajorCol(-1)
, mSingleColumn(false)
, mFlip(false)
, mStrength(1)
, mCurrentHover(-1)
, mListener(nullptr)
, mGridMode(kNormal)
, mHoldCol(0)
, mHoldRow(0)
, mLastClickWasClear(false)
, mRestrictDragToRow(false)
, mRequireShiftForMultislider(false)
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
   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   ofSetLineWidth(.5f);
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

         float data = mData[GetDataIndex(i,j)];
         if (data)
         {
            ofFill();
            float sliderFillAmount = ofClamp(ofLerp(.15f, 1, data), 0, 1);
            if (mGridMode == kNormal)
            {
               ofSetColor(255 * data, 255 * data, 255 * data, gModuleDrawAlpha);
               ofRect(x,y,xsize,ysize);
            }
            else if (mGridMode == kMultislider)
            {
               float fadeAmount = ofClamp(ofLerp(.5f, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofRect(x+.5f, y+.5f+(ysize*(1- sliderFillAmount)), xsize-1, ysize*sliderFillAmount -1, 0);
               /*ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofNoFill();
               ofRect(x+1,y+1,xsize-2,ysize-2, gCornerRoundness*.99f);*/
            }
            else if (mGridMode == kHorislider)
            {
               ofSetColor(255,255,255, gModuleDrawAlpha);
               ofRect(x, y, xsize*sliderFillAmount, ysize);
            }
            else if (mGridMode == kMultisliderBipolar)
            {
               float fadeAmount = ofClamp(ofLerp(.5f, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofRect(x, y + ysize * (.5f - sliderFillAmount/2), xsize, ysize * sliderFillAmount);
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
   if (GetHighlightCol(gTime) != -1)
   {
      ofNoFill();
      ofSetColor(0,255,0, gModuleDrawAlpha);
      for (int j=0; j<mRows; ++j)
         ofRect(GetX(GetHighlightCol(gTime),j), GetY(j), xsize, ysize);
   }
   if (mCurrentHover != -1 && mShouldDrawValue)
   {
      ofSetColor(ofColor::grey, gModuleDrawAlpha);
      DrawTextNormal(ofToString(GetVal(mCurrentHover % mCols, mCurrentHover / mCols)), 0, 12);
   }
   ofPopStyle();
   ofPopMatrix();
}

float UIGrid::GetX(int col, int row) const
{
   float xsize = float(mWidth) / mCols;
   return (col+mDrawOffset[row])*xsize;
}

float UIGrid::GetY(int row) const
{
   float ysize = float(mHeight) / mRows;
   if (mFlip)
      return mHeight-(row+1)*ysize;
   else
      return row*ysize;
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
   return ofVec2f(GetX(col, row), GetY(row));
}

bool UIGrid::CanAdjustMultislider() const
{
   return !mRequireShiftForMultislider || (GetKeyModifiers() & kModifier_Shift);
}

void UIGrid::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mClick = true;
   mLastClickWasClear = false;

   float clickHeight, clickWidth;
   GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
   int dataIndex = GetDataIndex(cell.mCol, cell.mRow);
   float oldValue = mData[dataIndex];

   if (mGridMode == kMultislider || mGridMode == kMultisliderBipolar)
   {
      if (CanAdjustMultislider())
      {
         mData[dataIndex] = clickHeight;
      }
      else
      {
         if (mData[dataIndex] > 0)
         {
            mData[dataIndex] = 0;
            mLastClickWasClear = true;
         }
         else
         {
            mData[dataIndex] = 1;
         }
      }
   }
   else if (mGridMode == kHorislider)
   {
      if (CanAdjustMultislider())
      {
         mData[dataIndex] = clickWidth;
      }
      else
      {
         float val = mStrength;
         if (mSingleColumn)
         {
            for (int i = 0; i < MAX_GRID_SIZE; ++i)
            {
               if (mData[GetDataIndex(cell.mCol, i)] != 0)
                  val = mData[GetDataIndex(cell.mCol, i)];
            }
         }

         if (mData[dataIndex] > 0)
         {
            mData[dataIndex] = 0;
            mLastClickWasClear = true;
         }
         else
         {
            mData[dataIndex] = val;
         }
      }
   }
   else
   {
      if (mData[dataIndex] == mStrength)
      {
         mData[dataIndex] = 0;
         mLastClickWasClear = true;
      }
      else
      {
         mData[dataIndex] = mStrength;
      }
   }

   if (mSingleColumn)
   {
      for (int i=0; i<MAX_GRID_SIZE; ++i)
      {
         if (i != cell.mRow)
            mData[GetDataIndex(cell.mCol, i)] = 0;
      }
   }
   
   if (mListener)
      mListener->GridUpdated(this, cell.mCol, cell.mRow, mData[dataIndex], oldValue);

   mHoldVal = mData[dataIndex];
   mHoldCol = cell.mCol;
   mHoldRow = cell.mRow;
}

void UIGrid::MouseReleased()
{
   if (mClick && mMomentary)
   {
      float oldValue = mData[GetDataIndex(mHoldCol, mHoldRow)];
      mData[GetDataIndex(mHoldCol, mHoldRow)] = 0;
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
      
      int dataIndex = GetDataIndex(cell.mCol, cell.mRow);
      float oldValue = mData[dataIndex];
      
      if (mGridMode == kMultislider && mHoldVal != 0 && CanAdjustMultislider())
      {
         mData[dataIndex] = clickHeight;
      }
      else if (mGridMode == kMultisliderBipolar && mHoldVal != 0 && CanAdjustMultislider())
      {
         mData[dataIndex] = clickHeight;
      }
      else if (mGridMode == kHorislider)
      {
         float val = mHoldVal;

         if (mSingleColumn)
         {
            for (int i = 0; i < MAX_GRID_SIZE; ++i)
            {
               if (mData[GetDataIndex(cell.mCol, i)] != 0)
                  val = mData[GetDataIndex(cell.mCol, i)];
            }
         }

         if (CanAdjustMultislider())
            val = clickWidth;
         
         mData[dataIndex] = val;
      }
      else
      {
         mData[dataIndex] = mHoldVal;
      }
      
      if (mSingleColumn)
      {
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (i != cell.mRow || mLastClickWasClear)
               mData[GetDataIndex(cell.mCol, i)] = 0;
         }
      }
      
      if (mListener)
         mListener->GridUpdated(this, cell.mCol, cell.mRow, mData[dataIndex], oldValue);
   }
   
   return false;
}

bool UIGrid::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (mGridMode == kMultislider || mGridMode == kHorislider || mGridMode == kMultisliderBipolar)
   {
      bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);
      
      float clickHeight, clickWidth;
      GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
      if (isMouseOver)
      {
         float& data = mData[GetDataIndex(cell.mCol, cell.mRow)];
         if (!mSingleColumn || data > 0)
         {
            float oldValue = data;
            data = ofClamp(data + scrollY / 100, FLT_EPSILON, 1);
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
   mData.fill(0);
}

float& UIGrid::GetVal(int col, int row)
{
   col = ofClamp(col, 0, MAX_GRID_SIZE-1);
   row = ofClamp(row, 0, MAX_GRID_SIZE-1);
   return mData[GetDataIndex(col,row)];
}

void UIGrid::SetVal(int col, int row, float val, bool notifyListener)
{
   col = ofClamp(col, 0, MAX_GRID_SIZE-1);
   row = ofClamp(row, 0, MAX_GRID_SIZE-1);
   if (val != mData[GetDataIndex(col,row)])
   {
      float oldValue = mData[GetDataIndex(col,row)];
      mData[GetDataIndex(col,row)] = val;
      
      if (mSingleColumn)
      {
         for (int i=0; i<MAX_GRID_SIZE; ++i)
         {
            if (i != row)
               mData[GetDataIndex(col, i)] = 0;
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

void UIGrid::SetHighlightCol(double time, int col)
{
   mHighlightColBuffer[mNextHighlightColPointer].time = time;
   mHighlightColBuffer[mNextHighlightColPointer].col = col;
   mNextHighlightColPointer = (mNextHighlightColPointer + 1) % mHighlightColBuffer.size();
}

int UIGrid::GetHighlightCol(double time) const
{
   int ret = -1;
   double latestTime = -1;
   for (size_t i = 0; i < mHighlightColBuffer.size(); ++i)
   {
      if (mHighlightColBuffer[i].time <= time && mHighlightColBuffer[i].time > latestTime)
      {
         ret = mHighlightColBuffer[i].col;
         latestTime = mHighlightColBuffer[i].time;
      }
   }
   return ret;
}

namespace
{
   const int kSaveStateRev = 1;
}

void UIGrid::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   for (int i=0; i<MAX_GRID_SIZE*MAX_GRID_SIZE; ++i)
   {
      out << mData[i];
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
         int dataIndex = GetDataIndex(j, i);
         float oldVal = mData[dataIndex];
         in >> mData[dataIndex];
         if (mListener)
            mListener->GridUpdated(this, j, i, mData[dataIndex], oldVal);
      }
   }
}
