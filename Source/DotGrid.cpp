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
//  DotGrid.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/23.
//
//

#include "DotGrid.h"
#include "SynthGlobals.h"
#include "FileStream.h"
#include "IDrawableModule.h"
#include "PatchCableSource.h"
#include "Snapshots.h"

#include <cstring>

DotGrid::DotGrid(IClickable* parent, std::string name, int x, int y, int w, int h, int cols, int rows)
: mWidth(w)
, mHeight(h)
{
   SetName(name.c_str());
   SetPosition(x, y);
   SetGrid(cols, rows);
   Clear();
   SetParent(parent);
   //dynamic_cast<IDrawableModule*>(parent)->AddUIGrid(this);
}

DotGrid::~DotGrid()
{
}

void DotGrid::Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent)
{
   mWidth = w;
   mHeight = h;
   SetPosition(x, y);
   SetGrid(cols, rows);
   Clear();
   SetParent(parent);
}

void DotGrid::DrawGridCircle(int col, int row, float radiusPercent) const
{
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;
   ofCircle(GetX(col) + xsize * .5f, GetY(row) + ysize * .5f, GetDotSize() * .5f * radiusPercent);
}

float DotGrid::GetDotSize() const
{
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;
   return std::min(xsize, ysize);
}

void DotGrid::Render()
{
   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   ofSetLineWidth(.5f);
   float w, h;
   GetDimensions(w, h);
   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;

   if (GetHighlightCol(gTime) != -1)
   {
      ofFill();
      ofSetColor(255, 255, 255, gModuleDrawAlpha * .2f);
      ofRect(GetX(GetHighlightCol(gTime)), 0, xsize, mHeight);
   }

   ofNoFill();
   int superMajor = mMajorCol * 4;
   int superDuperMajor = mMajorCol * 16;
   for (int j = 0; j < mRows; ++j)
   {
      for (int i = 0; i < mCols; ++i)
      {
         if (mMajorCol > 0)
         {
            if (mCols > superDuperMajor && i % superDuperMajor == 0)
               ofSetColor(255, 255, 200, gModuleDrawAlpha);
            else if (mCols > superMajor && i % superMajor == 0)
               ofSetColor(255, 255, 100, gModuleDrawAlpha);
            else if (mCols > mMajorCol && i % mMajorCol == 0)
               ofSetColor(255, 200, 100, gModuleDrawAlpha);
            else
               ofSetColor(100, 100, 100, gModuleDrawAlpha);
         }
         else
         {
            ofSetColor(100, 100, 100, gModuleDrawAlpha);
         }
         DrawGridCircle(i, j, .3f);
      }
   }
   for (int j = 0; j < mRows; ++j)
   {
      for (int i = 0; i < mCols; ++i)
      {
         DotData& data = mData[GetDataIndex(i, j)];
         if (data.mOn)
         {
            float bump = ofClamp((data.mLastPlayTime + 250.0f - gTime) / 250.0f, 0, 1);
            float radius = ofLerp(.65f, 1.0f, bump);

            //white outer ring
            ofFill();
            ofSetColor(0, 0, 0);
            DrawGridCircle(i, j, radius);
            ofNoFill();
            ofSetColor(255, 255, 255);
            DrawGridCircle(i, j, radius);

            //line + center circle
            ofFill();
            ofSetColor(255 * data.mVelocity, 255 * data.mVelocity, 255 * data.mVelocity);
            ofPushStyle();
            ofSetLineWidth(GetDotSize() * radius * .23f);
            ofLine(GetX(i) + xsize * .5f, GetY(j) + ysize * .5f, GetX(i) + xsize * .5f + xsize * data.mLength, GetY(j) + ysize * .5f);
            ofPopStyle();
            DrawGridCircle(i, j, radius * .9f * data.mVelocity);
         }

         if (mCurrentHover.mCol == i && mCurrentHover.mRow == j && gHoveredUIControl == nullptr)
         {
            if (mClick)
            {
               if (mDragBehavior == DragBehavior::Velocity)
               {
                  DotData& currentHoverData = mData[GetDataIndex(mCurrentHover.mCol, mCurrentHover.mRow)];
                  ofSetColor(0, 255, 0);
                  DrawTextNormal(ofToString(currentHoverData.mVelocity, 2), GetX(i), GetY(j), 8.0f);
               }
            }
            else
            {
               ofFill();
               ofSetColor(180, 180, 0, 160);
               DrawGridCircle(i, j, .8f);
            }
         }
      }
   }

   ofPopStyle();

   ofPopMatrix();
}

float DotGrid::GetX(int col) const
{
   float xsize = float(mWidth) / mCols;
   return (col)*xsize;
}

float DotGrid::GetY(int row) const
{
   float ysize = float(mHeight) / mRows;
   return mHeight - (row + 1) * ysize;
}

DotGrid::DotPosition DotGrid::GetGridCellAt(float x, float y, bool clamp /*= true*/)
{
   y = (mHeight - 1) - y; //flip

   float xsize = float(mWidth) / mCols;
   float ysize = float(mHeight) / mRows;

   int col = x / xsize;
   int row = y / ysize;

   if (clamp)
   {
      col = ofClamp(col, 0, mCols - 1);
      row = ofClamp(row, 0, mRows - 1);
   }

   return DotPosition(col, row);
}

ofVec2f DotGrid::GetCellPosition(int col, int row)
{
   return ofVec2f(GetX(col), GetY(row));
}

bool DotGrid::CanBeTargetedBy(PatchCableSource* source) const
{
   return source->GetConnectionType() == kConnectionType_UIControl && dynamic_cast<Snapshots*>(source->GetOwner()) != nullptr;
}

void DotGrid::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   mClick = true;

   DotPosition cell = GetGridCellAt(x, y);
   int dataIndex = GetDataIndex(cell.mCol, cell.mRow);

   if (mData[dataIndex].mOn)
   {
      mMouseReleaseCanClear = true;
   }
   else
   {
      mData[dataIndex].mOn = true;
      mData[dataIndex].mVelocity = gStepVelocityLevels[(int)StepVelocityType::Normal];
      mData[dataIndex].mLength = 0;
      mMouseReleaseCanClear = false;
   }

   mDragBehavior = DragBehavior::Pending;
   mHoldCell = cell;
   mLastDragPosition.set(x, y);
}

void DotGrid::MouseReleased()
{
   mClick = false;

   if (mMouseReleaseCanClear && !TheSynth->MouseMovedSignificantlySincePressed() && mHoldCell.IsValid())
   {
      int dataIndex = GetDataIndex(mHoldCell.mCol, mHoldCell.mRow);
      mData[dataIndex].mOn = false;
   }

   mHoldCell.Clear();
   mMouseReleaseCanClear = false;
}

bool DotGrid::MouseMoved(float x, float y)
{
   bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);

   if (mClick)
   {
      if (mHoldCell.IsValid())
      {
         if (mDragBehavior == DragBehavior::Pending)
         {
            if (std::abs(x - mLastDragPosition.x) < 2 && std::abs(y - mLastDragPosition.y) < 2)
               mDragBehavior = DragBehavior::Pending;
            else if (std::abs(x - mLastDragPosition.x) > std::abs(y - mLastDragPosition.y))
               mDragBehavior = DragBehavior::Length;
            else
               mDragBehavior = DragBehavior::Velocity;
         }

         if (mDragBehavior == DragBehavior::Length)
         {
            DotPosition cell = GetGridCellAt(x, y, !K(clamp));
            int newLength = std::max(cell.mCol - mHoldCell.mCol, 0);
            int dataIndex = GetDataIndex(mHoldCell.mCol, mHoldCell.mRow);
            mData[dataIndex].mLength = newLength;
         }

         if (mDragBehavior == DragBehavior::Velocity)
         {
            int dataIndex = GetDataIndex(mHoldCell.mCol, mHoldCell.mRow);
            mData[dataIndex].mVelocity = std::clamp(mData[dataIndex].mVelocity - (y - mLastDragPosition.y) * .01f, 0.0f, 1.0f);
         }
      }

      mLastDragPosition.set(x, y);
   }
   else
   {
      DotPosition cell = GetGridCellAt(x, y, K(clamp));
      if (isMouseOver)
         mCurrentHover = cell;
      else
         mCurrentHover.Clear();
   }

   return false;
}

bool DotGrid::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   bool isMouseOver = (x >= 0 && x < mWidth && y >= 0 && y < mHeight);

   if (isMouseOver && mCurrentHover.IsValid())
   {
      DotData& data = mData[GetDataIndex(mCurrentHover.mCol, mCurrentHover.mRow)];
      if (data.mOn)
         data.mLength = std::max(data.mLength + scrollY * .1f, 0.0f);
   }

   return false;
}

void DotGrid::KeyPressed(int key, bool repeat)
{
   if (key == OF_KEY_UP || key == OF_KEY_DOWN || key == OF_KEY_RIGHT || key == OF_KEY_LEFT)
   {
      if (mCurrentHover.IsValid())
      {
         DotData& data = mData[GetDataIndex(mCurrentHover.mCol, mCurrentHover.mRow)];
         if (data.mOn)
         {
            if (GetKeyModifiers() == kModifier_Shift)
            {
               if (key == OF_KEY_UP)
               {
                  for (int i = 0; i < (int)gStepVelocityLevels.size(); ++i)
                  {
                     if (data.mVelocity < gStepVelocityLevels[i])
                     {
                        data.mVelocity = gStepVelocityLevels[i];
                        break;
                     }
                  }
               }

               if (key == OF_KEY_DOWN)
               {
                  for (int i = (int)gStepVelocityLevels.size() - 1; i >= 0; --i)
                  {
                     if (data.mVelocity > gStepVelocityLevels[i])
                     {
                        data.mVelocity = gStepVelocityLevels[i];
                        break;
                     }
                  }
               }
            }
            else
            {
               int dirX = 0;
               int dirY = 0;
               if (key == OF_KEY_RIGHT)
                  dirX = 1;
               if (key == OF_KEY_LEFT)
                  dirX = -1;
               if (key == OF_KEY_UP)
                  dirY = 1;
               if (key == OF_KEY_DOWN)
                  dirY = -1;
               DotPosition newPos(mCurrentHover.mCol + dirX, mCurrentHover.mRow + dirY);
               if (newPos.mCol >= 0 && newPos.mCol < mCols && newPos.mRow >= 0 && newPos.mRow < mRows)
               {
                  mData[GetDataIndex(newPos.mCol, newPos.mRow)] = data;
                  data.mOn = false;
                  mCurrentHover = newPos;
               }
            }
         }
      }
   }
}

void DotGrid::SetGrid(int cols, int rows)
{
   cols = ofClamp(cols, 0, kMaxCols);
   rows = ofClamp(rows, 0, kMaxRows);
   mRows = rows;
   mCols = cols;
}

const DotGrid::DotData& DotGrid::GetDataAt(int col, int row) const
{
   col = ofClamp(col, 0, kMaxCols - 1);
   row = ofClamp(row, 0, kMaxRows - 1);
   return mData[GetDataIndex(col, row)];
}

void DotGrid::OnPlayed(double time, int col, int row)
{
   if (IsValidPosition(DotPosition(col, row)))
      mData[GetDataIndex(col, row)].mLastPlayTime = time;
}

void DotGrid::Clear()
{
   for (auto& data : mData)
      data.mOn = false;
}

bool DotGrid::IsValidPosition(DotPosition pos) const
{
   return pos.mCol >= 0 && pos.mCol < kMaxCols && pos.mRow >= 0 && pos.mRow < kMaxRows;
}

void DotGrid::CopyDot(DotPosition from, DotPosition to)
{
   if (IsValidPosition(from) && IsValidPosition(to))
      mData[GetDataIndex(to.mCol, to.mRow)] = mData[GetDataIndex(from.mCol, from.mRow)];
}

void DotGrid::SetHighlightCol(double time, int col)
{
   mHighlightColBuffer[mNextHighlightColPointer].time = time;
   mHighlightColBuffer[mNextHighlightColPointer].col = col;
   mNextHighlightColPointer = (mNextHighlightColPointer + 1) % mHighlightColBuffer.size();
}

int DotGrid::GetHighlightCol(double time) const
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
   const int kSaveStateRev = 0;
}

void DotGrid::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mCols;
   out << mRows;
   for (int col = 0; col < mCols; ++col)
   {
      for (int row = 0; row < mRows; ++row)
      {
         out << mData[GetDataIndex(col, row)].mOn;
         out << mData[GetDataIndex(col, row)].mVelocity;
         out << mData[GetDataIndex(col, row)].mLength;
      }
   }
}

void DotGrid::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   in >> mCols;
   in >> mRows;

   for (int col = 0; col < mCols; ++col)
   {
      for (int row = 0; row < mRows; ++row)
      {
         int dataIndex = GetDataIndex(col, row);
         in >> mData[dataIndex].mOn;
         in >> mData[dataIndex].mVelocity;
         in >> mData[dataIndex].mLength;
      }
   }
}
