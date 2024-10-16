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
//  DotGrid.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/23.
//
//

#pragma once

#include "IUIControl.h"
#include "SynthGlobals.h"

class DotGrid : public IUIControl
{
public:
   static constexpr int kMaxCols = 128;
   static constexpr int kMaxRows = 128;

   struct DotPosition
   {
      DotPosition(int col, int row)
      : mCol(col)
      , mRow(row)
      {}
      void Clear()
      {
         mCol = -1;
         mRow = -1;
      }
      bool IsValid() const
      {
         return mCol != -1;
      }
      int mCol;
      int mRow;
   };

   struct DotData
   {
      bool mOn;
      float mVelocity;
      float mLength;
      double mLastPlayTime; //unserialized
   };

   DotGrid(IClickable* parent, std::string name, int x, int y, int w, int h, int cols, int rows);
   void Init(int x, int y, int w, int h, int cols, int rows, IClickable* parent);
   void SetGrid(int cols, int rows);
   int GetRows() { return mRows; }
   int GetCols() { return mCols; }
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;
   void SetHighlightCol(double time, int col);
   int GetHighlightCol(double time) const;
   void SetMajorColSize(int size) { mMajorCol = size; }
   int GetMajorColSize() const { return mMajorCol; }
   void Clear();
   void SetDimensions(float width, float height)
   {
      mWidth = width;
      mHeight = height;
   }
   float GetWidth() const { return mWidth; }
   float GetHeight() const { return mHeight; }
   const std::array<DotData, kMaxCols * kMaxRows>& GetData() const { return mData; }
   void SetData(std::array<DotData, kMaxCols * kMaxRows>& data) { mData = data; }
   bool GetNoHover() const override { return true; }
   bool CanBeTargetedBy(PatchCableSource* source) const override;
   const DotData& GetDataAt(int col, int row) const;
   void OnPlayed(double time, int col, int row);
   float GetDotSize() const;
   int GetMaxColumns() const { return kMaxCols; }
   void CopyDot(DotPosition from, DotPosition to);
   bool IsValidPosition(DotPosition pos) const;

   DotPosition GetGridCellAt(float x, float y, bool clamp = true);
   ofVec2f GetCellPosition(int col, int row);

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override {}
   void SetValue(float value, double time, bool forceUpdate = false) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool ShouldSerializeForSnapshot() const override { return true; }

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

protected:
   ~DotGrid(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void KeyPressed(int key, bool repeat) override;

   int GetDataIndex(int col, int row) const { return col + row * kMaxCols; }
   float GetX(int col) const;
   float GetY(int row) const;
   void DrawGridCircle(int col, int row, float radiusPercent) const;

   struct HighlightColBuffer
   {
      double time{ 0 };
      int col{ -1 };
   };

   enum class DragBehavior
   {
      Pending,
      Length,
      Velocity
   };
   DragBehavior mDragBehavior{ DragBehavior::Pending };

   float mWidth{ 200 };
   float mHeight{ 200 };
   int mRows{ 0 };
   int mCols{ 0 };
   bool mClick{ false };
   DotPosition mHoldCell{ -1, -1 };
   ofVec2f mLastDragPosition{ -1, -1 };
   bool mMouseReleaseCanClear{ false };
   std::array<DotData, kMaxCols * kMaxRows> mData{};
   std::array<HighlightColBuffer, 10> mHighlightColBuffer{};
   int mNextHighlightColPointer{ 0 };
   int mMajorCol{ -1 };
   DotPosition mCurrentHover{ -1, -1 };
};
