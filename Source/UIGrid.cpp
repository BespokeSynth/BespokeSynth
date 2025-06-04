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
//  UIGrid.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/13.
//
//

#include "UIGrid.h"
#include "SynthGlobals.h"
#include "FileStream.h"
#include "IDrawableModule.h"
#include "PatchCableSource.h"
#include "Snapshots.h"

#include <cstring>

UIGrid::UIGrid(IClickable* parent, std::string name, int x, int y, int w, int h, int cols, int rows)
: mWidth(w)
, mHeight(h)
{
   SetName(name.c_str());
   SetPosition(x, y);
   SetGrid(cols, rows);
   Clear();
   SetParent(parent);
   mDrawOffset.fill(0);
   dynamic_cast<IDrawableModule*>(parent)->AddUIGrid(this);
}

UIGrid::~UIGrid()
{
}

void UIGrid::Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent)
{
   mWidth = w;
   mHeight = h;
   SetPosition(x, y);
   SetGrid(cols, rows);
   Clear();
   SetParent(parent);
   mDrawOffset.fill(0);
}

void UIGrid::Render()
{
   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   ofSetLineWidth(.5);
   double w, h;
   GetDimensions(w, h);
   double xsize = mWidth / mCols;
   double ysize = mHeight / mRows;
   for (int j = 0; j < mRows; ++j)
   {
      for (int i = 0; i < mCols; ++i)
      {
         double x = GetX(i, j);
         double y = GetY(j);
         bool drawDragLevels = false;

         double data = mData[GetDataIndex(i, j)];
         if (data)
         {
            ofFill();
            double sliderFillAmount = ofClamp(ofLerp(.15, 1, data), 0, 1);
            if (mGridMode == kNormal)
            {
               ofSetColor(255 * data, 255 * data, 255 * data, gModuleDrawAlpha);
               ofRect(x, y, xsize, ysize);
            }
            else if (mGridMode == kMultislider)
            {
               double fadeAmount = ofClamp(ofLerp(.5, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofRect(x + .5, y + .5 + (ysize * (1 - sliderFillAmount)), xsize - 1, ysize * sliderFillAmount - 1, 0);
               /*ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofNoFill();
               ofRect(x + 1, y + 1, xsize - 2, ysize - 2, gCornerRoundness * .99);*/
            }
            else if (mGridMode == kHorislider)
            {
               ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofRect(x, y, xsize * sliderFillAmount, ysize);
            }
            else if (mGridMode == kMultisliderBipolar)
            {
               double fadeAmount = ofClamp(ofLerp(.5, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofRect(x, y + ysize * (.5 - sliderFillAmount / 2), xsize, ysize * sliderFillAmount);
               drawDragLevels = true;
            }
            else if (mGridMode == kMultisliderGrow)
            {
               double fadeAmount = ofClamp(ofLerp(.5, 1, data), 0, 1);
               ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
               ofVec2d center(x + xsize * 0.5, y + ysize * 0.5);
               ofRect(center.x - (sliderFillAmount * sliderFillAmount * xsize * 0.5), center.y - (sliderFillAmount * sliderFillAmount * ysize * 0.5), sliderFillAmount * sliderFillAmount * xsize, sliderFillAmount * sliderFillAmount * ysize);
               drawDragLevels = true;
            }

            if (drawDragLevels)
            {
               if (mClick && mHoldVal != 0 && CanAdjustMultislider())
               {
                  if (j == mHoldRow)
                  {
                     ofSetColor(0, 255, 0, gModuleDrawAlpha);
                     ofRect(x + .5, y + .5 + (ysize * (1 - sliderFillAmount)), xsize - 1, 2, 0);
                  }
               }
            }
         }

         if (mCurrentHover == i + j * mCols && gHoveredUIControl == nullptr)
         {
            ofFill();
            ofSetColor(180, 180, 0, 160);
            ofRect(x + 2, y + 2, std::min(xsize * mCurrentHoverAmount, xsize - 4), ysize - 4);
         }
      }
   }
   ofNoFill();
   ofSetColor(100, 100, 100, gModuleDrawAlpha);
   for (int j = 0; j < mRows; ++j)
   {
      for (int i = 0; i < mCols; ++i)
         ofRect(GetX(i, j), GetY(j), xsize, ysize);
   }
   ofNoFill();
   if (mMajorCol > 0)
   {
      ofSetColor(255, 200, 100, gModuleDrawAlpha);
      for (int j = 0; j < mRows; ++j)
      {
         for (int i = 0; i < mCols; i += mMajorCol)
         {
            ofRect(GetX(i, j), GetY(j), xsize, ysize);
         }
      }
      if (mCols > mMajorCol * mMajorCol)
      {
         ofSetColor(255, 255, 100, gModuleDrawAlpha);
         for (int j = 0; j < mRows; ++j)
         {
            for (int i = 0; i < mCols; i += mMajorCol * mMajorCol)
            {
               ofRect(GetX(i, j), GetY(j), xsize, ysize);
            }
         }
         if (mCols > mMajorCol * mMajorCol * mMajorCol)
         {
            ofSetColor(255, 255, 200, gModuleDrawAlpha);
            for (int j = 0; j < mRows; ++j)
            {
               for (int i = 0; i < mCols; i += mMajorCol * mMajorCol * mMajorCol)
               {
                  ofRect(GetX(i, j), GetY(j), xsize, ysize);
               }
            }
         }
      }
   }
   if (GetHighlightCol(gTime) != -1)
   {
      ofNoFill();
      ofSetColor(0, 255, 0, gModuleDrawAlpha);
      for (int j = 0; j < mRows; ++j)
         ofRect(GetX(GetHighlightCol(gTime), j), GetY(j), xsize, ysize);
   }
   if (mCurrentHover != -1 && mShouldDrawValue)
   {
      ofSetColor(ofColor::grey, gModuleDrawAlpha);
      DrawTextNormal(ofToString(GetVal(mCurrentHover % mCols, mCurrentHover / mCols)), 0, 12);
   }
   ofPopStyle();

   ofPopMatrix();
}

double UIGrid::GetX(int col, int row) const
{
   double xsize = double(mWidth) / mCols;
   return (col + mDrawOffset[std::clamp(row, 0, (int)mDrawOffset.size() - 1)]) * xsize;
}

double UIGrid::GetY(int row) const
{
   double ysize = double(mHeight) / mRows;
   if (mFlip)
      return mHeight - (row + 1) * ysize;
   else
      return row * ysize;
}

GridCell UIGrid::GetGridCellAt(double x, double y, double* clickHeight, double* clickWidth)
{
   if (mFlip)
      y = (mHeight - 1) - y;

   const double xsize = mWidth / mCols;
   const double ysize = mHeight / mRows;

   int col = ofClamp(x / xsize, 0, mCols - 1);
   int row = ofClamp(y / ysize, 0, mRows - 1);

   if (clickHeight)
   {
      *clickHeight = ofClamp(1 - (y / ysize - ofClamp(static_cast<int>(y / ysize), 0, mRows - 1)), 0, 1);
      if (mFlip)
         *clickHeight = 1 - *clickHeight;
   }

   if (clickWidth)
   {
      *clickWidth = ofClamp(x / xsize - ofClamp(static_cast<int>(x / xsize), 0, mCols - 1), 0, 1);
   }

   return { col, row };
}

ofVec2d UIGrid::GetCellPosition(int col, int row)
{
   return { GetX(col, row), GetY(row) };
}

bool UIGrid::CanAdjustMultislider() const
{
   return !mRequireShiftForMultislider || (GetKeyModifiers() & kModifier_Shift);
}

double UIGrid::GetSubdividedValue(double position) const
{
   return ofClamp(ceil(position * mClickSubdivisions) / mClickSubdivisions, 1.0 / mClickSubdivisions, 1);
}

bool UIGrid::CanBeTargetedBy(PatchCableSource* source) const
{
   if (source->GetConnectionType() == kConnectionType_UIControl)
   {
      if (mCanBeUIControlTarget)
         return true;
      if (dynamic_cast<Snapshots*>(source->GetOwner()) != nullptr)
         return true;
   }
   return false;
}

void UIGrid::OnClicked(double x, double y, bool right)
{
   if (right)
      return;

   mClick = true;
   mLastClickWasClear = false;

   double clickHeight, clickWidth;
   GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
   int dataIndex = GetDataIndex(cell.mCol, cell.mRow);
   double oldValue = mData[dataIndex];

   if (mGridMode == kMultislider || mGridMode == kMultisliderBipolar || mGridMode == kMultisliderGrow)
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
            mData[dataIndex] = mStrength;
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
         double val = mStrength;

         if (mSingleColumn)
         {
            for (int i = 0; i < MAX_GRID_ROWS; ++i)
            {
               if (mData[GetDataIndex(cell.mCol, i)] != 0)
                  val = mData[GetDataIndex(cell.mCol, i)];
            }
         }

         if (mClickSubdivisions != 1)
            val = GetSubdividedValue(clickWidth);

         if (mData[dataIndex] == val)
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
      if (mData[dataIndex] > 0)
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
      for (int i = 0; i < MAX_GRID_ROWS; ++i)
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
      double oldValue = mData[GetDataIndex(mHoldCol, mHoldRow)];
      mData[GetDataIndex(mHoldCol, mHoldRow)] = 0;
      mListener->GridUpdated(this, mHoldCol, mHoldRow, 0, oldValue);
   }

   mClick = false;
}

bool UIGrid::MouseMoved(double x, double y)
{
   bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);

   double clickHeight, clickWidth;
   GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);

   if (mClick && mRestrictDragToRow)
   {
      if (cell.mRow > mHoldRow)
         clickHeight = mFlip ? 1 : 0;
      if (cell.mRow < mHoldRow)
         clickHeight = mFlip ? 0 : 1;
      cell.mRow = mHoldRow;
   }

   if (mClick && mGridMode == kHorislider && CanAdjustMultislider())
   {
      if (cell.mCol > mHoldCol)
         clickWidth = 1;
      if (cell.mCol < mHoldCol)
         clickWidth = 0;
      cell.mCol = mHoldCol;
   }

   if (isMouseOver)
   {
      mCurrentHover = cell.mCol + cell.mRow * mCols;

      if (mGridMode == kHorislider && CanAdjustMultislider())
         mCurrentHoverAmount = clickWidth;
      else if (mClickSubdivisions != -1)
         mCurrentHoverAmount = GetSubdividedValue(clickWidth);
      else
         mCurrentHoverAmount = 1;
   }
   else if (!mClick)
   {
      mCurrentHover = -1;
   }

   if (mClick && !mMomentary)
   {
      int dataIndex = GetDataIndex(cell.mCol, cell.mRow);
      double oldValue = mData[dataIndex];

      if ((mGridMode == kMultislider || mGridMode == kMultisliderBipolar || mGridMode == kMultisliderGrow) && mHoldVal != 0 && CanAdjustMultislider())
      {
         mData[dataIndex] = clickHeight;
      }
      else if (mGridMode == kHorislider)
      {
         double val = mHoldVal;
         mHoldCol = cell.mCol;

         if (mSingleColumn)
         {
            for (int i = 0; i < MAX_GRID_ROWS; ++i)
            {
               if (mData[GetDataIndex(cell.mCol, i)] != 0)
                  val = mData[GetDataIndex(cell.mCol, i)];
            }
         }

         if (CanAdjustMultislider())
            val = clickWidth;
         else if (mClickSubdivisions != 1)
            val = GetSubdividedValue(clickWidth);

         mData[dataIndex] = val;
      }
      else
      {
         mData[dataIndex] = mHoldVal;
      }

      if (mSingleColumn)
      {
         for (int i = 0; i < MAX_GRID_ROWS; ++i)
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

bool UIGrid::MouseScrolled(double x, double y, double scrollX, double scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   if (mGridMode == kMultislider || mGridMode == kHorislider || mGridMode == kMultisliderBipolar || mGridMode == kMultisliderGrow)
   {
      bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);

      double clickHeight, clickWidth;
      GridCell cell = GetGridCellAt(x, y, &clickHeight, &clickWidth);
      if (isMouseOver)
      {
         double& data = mData[GetDataIndex(cell.mCol, cell.mRow)];
         if (!mSingleColumn || data > 0)
         {
            double oldValue = data;
            data = ofClamp(data + scrollY / 100, std::numeric_limits<double>::epsilon(), 1);
            if (mListener)
               mListener->GridUpdated(this, cell.mCol, cell.mRow, data, oldValue);
         }
      }
   }

   return false;
}

void UIGrid::SetGrid(int cols, int rows)
{
   cols = ofClamp(cols, 0, MAX_GRID_COLS);
   rows = ofClamp(rows, 0, MAX_GRID_ROWS);
   mRows = rows;
   mCols = cols;
}

void UIGrid::Clear()
{
   mData.fill(0);
}

double UIGrid::GetVal(int col, int row) const
{
   col = ofClamp(col, 0, MAX_GRID_COLS - 1);
   row = ofClamp(row, 0, MAX_GRID_ROWS - 1);
   return mData[GetDataIndex(col, row)];
}

double& UIGrid::GetVal(int col, int row)
{
   col = ofClamp(col, 0, MAX_GRID_COLS - 1);
   row = ofClamp(row, 0, MAX_GRID_ROWS - 1);
   return mData[GetDataIndex(col, row)];
}

void UIGrid::SetVal(int col, int row, double val, bool notifyListener)
{
   col = ofClamp(col, 0, MAX_GRID_COLS - 1);
   row = ofClamp(row, 0, MAX_GRID_ROWS - 1);
   if (val != mData[GetDataIndex(col, row)])
   {
      double oldValue = mData[GetDataIndex(col, row)];
      mData[GetDataIndex(col, row)] = val;

      if (mSingleColumn && val > 0)
      {
         for (int i = 0; i < MAX_GRID_ROWS; ++i)
         {
            if (i != row)
               mData[GetDataIndex(col, i)] = 0;
         }
      }

      if (notifyListener && mListener)
         mListener->GridUpdated(this, col, row, val, oldValue);
   }
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

void UIGrid::SetFromMidiCC(double slider, double time, bool setViaModulator)
{
   SetValue(slider, time);
}

double UIGrid::GetValueForMidiCC(double slider) const
{
   return slider;
}

void UIGrid::SetValue(double value, double time, bool forceUpdate)
{
   if (mValueSetTargetCol >= 0 && mValueSetTargetRow < mCols && mValueSetTargetRow >= 0 && mValueSetTargetRow < mRows)
   {
      double currentValue = GetVal(mValueSetTargetCol, mValueSetTargetRow);
      if (value != currentValue || forceUpdate)
         SetVal(mValueSetTargetCol, mValueSetTargetRow, value);
   }
}

double UIGrid::GetMidiValue() const
{
   return GetValue();
}

double UIGrid::GetValue() const
{
   if (mValueSetTargetCol >= 0 && mValueSetTargetRow < mCols && mValueSetTargetRow >= 0 && mValueSetTargetRow < mRows)
      return GetVal(mValueSetTargetCol, mValueSetTargetRow);
   return 0;
}

std::string UIGrid::GetDisplayValue(double val) const
{
   return ofToString(GetValue());
}

namespace
{
   const int kSaveStateRev = 2;
}

void UIGrid::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mCols;
   out << mRows;
   for (int col = 0; col < mCols; ++col)
   {
      for (int row = 0; row < mRows; ++row)
         out << mData[GetDataIndex(col, row)];
   }
}

void UIGrid::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   int cols;
   int rows;

   if (rev < 1)
   {
      cols = 100;
      rows = 100;
   }
   else if (rev == 1)
   {
      cols = 128;
      rows = 128;
   }
   else
   {
      in >> mCols;
      in >> mRows;
      cols = mCols;
      rows = mRows;
   }

   for (int col = 0; col < cols; ++col)
   {
      for (int row = 0; row < rows; ++row)
      {
         int dataIndex;
         if (rev < 2)
            dataIndex = GetDataIndex(row, col);
         else
            dataIndex = GetDataIndex(col, row);
         double oldVal = mData[dataIndex];
         in >> FloatAsDouble >> mData[dataIndex];
         if (mListener)
            mListener->GridUpdated(this, col, row, mData[dataIndex], oldVal);
      }
   }
}
