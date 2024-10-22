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
//  Canvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/29/14.
//
//

#pragma once

#include "IUIControl.h"
#include "CanvasElement.h"

#include "juce_gui_basics/juce_gui_basics.h"

#define MAX_CANVAS_MASK_ELEMENTS 128

class Canvas;
class CanvasControls;
class CanvasElement;

typedef CanvasElement* (*CreateCanvasElementFn)(Canvas* canvas, int col, int row);

class ICanvasListener
{
public:
   virtual ~ICanvasListener() {}
   virtual void CanvasUpdated(Canvas* canvas) = 0;
   virtual void ElementRemoved(CanvasElement* element) {}
};

struct CanvasCoord
{
   CanvasCoord(int _col, int _row)
   : col(_col)
   , row(_row)
   {}
   int col;
   int row;
};

class Canvas : public IUIControl
{
public:
   enum HighlightEnd
   {
      kHighlightEnd_None,
      kHighlightEnd_Start,
      kHighlightEnd_End
   };

   enum DragMode
   {
      kDragNone = 0,
      kDragHorizontal = 1,
      kDragVertical = 2,
      kDragBoth = kDragHorizontal | kDragVertical
   };

public:
   Canvas(IDrawableModule* parent, int x, int y, int w, int h, float length, int rows, int cols, CreateCanvasElementFn elementCreator);
   ~Canvas();

   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;
   void Clear();
   void SetListener(ICanvasListener* listener) { mListener = listener; }
   void SetDimensions(int width, int height)
   {
      mWidth = width;
      mHeight = height;
   }
   float GetWidth() const { return mWidth; }
   float GetHeight() const { return mHeight; }
   void SetLength(float length) { mLength = length; }
   float GetLength() const { return mLength; }
   void SetNumRows(int rows) { mNumRows = rows; }
   void SetNumCols(int cols) { mNumCols = cols; }
   int GetNumRows() const { return mNumRows; }
   int GetNumCols() const { return mNumCols; }
   void RescaleNumCols(int cols);
   void AddElement(CanvasElement* element);
   void RemoveElement(CanvasElement* element);
   void SelectElement(CanvasElement* element);
   void SelectElements(std::vector<CanvasElement*> elements);
   void SetControls(CanvasControls* controls) { mControls = controls; }
   CanvasControls* GetControls() { return mControls; }
   std::vector<CanvasElement*>& GetElements() { return mElements; }
   void FillElementsAt(float pos, std::vector<CanvasElement*>& elements) const;
   void EraseElementsAt(float pos);
   CanvasElement* GetElementAt(float pos, int row);
   void SetCursorPos(float pos) { mCursorPos = pos; }
   float GetCursorPos() const { return mCursorPos; }
   CanvasElement* CreateElement(int col, int row) { return mElementCreator(this, col, row); }
   CanvasCoord GetCoordAt(int x, int y);
   void SetNumVisibleRows(int rows) { mNumVisibleRows = rows; }
   int GetNumVisibleRows() const { return MIN(mNumVisibleRows, mNumRows); }
   void SetRowOffset(int offset) { mRowOffset = ofClamp(offset, 0, mNumRows - mNumVisibleRows); }
   int GetRowOffset() const { return mRowOffset; }
   bool ShouldWrap() const { return mWrap; }
   HighlightEnd GetHighlightEnd() const { return mHighlightEnd; }
   void SetMajorColumnInterval(int interval) { mMajorColumnInterval = interval; }
   void SetDragMode(DragMode mode) { mDragMode = mode; }
   DragMode GetDragMode() const { return mDragMode; }
   bool IsRowVisible(int row) const;
   void SetRowColor(int row, ofColor color);
   juce::MouseCursor GetMouseCursorType();
   ofVec2f RescaleForZoom(float x, float y) const;

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override {}
   void SetValue(float value, double time, bool forceUpdate = false) override {}
   void KeyPressed(int key, bool isRepeat) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool GetNoHover() const override { return true; }
   bool CanBeTargetedBy(PatchCableSource* source) const override;
   bool ShouldSerializeForSnapshot() const override { return true; }

   float mViewStart{ 0 };
   float mViewEnd;
   float mLoopStart{ 0 };
   float mLoopEnd;

private:
   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   bool IsOnElement(CanvasElement* element, float x, float y) const;
   float QuantizeToGrid(float input) const;

   bool mClick{ false };
   CanvasElement* mClickedElement{ nullptr };
   ofVec2f mClickedElementStartMousePos;
   float mWidth;
   float mHeight;
   float mLength;
   ICanvasListener* mListener{ nullptr };
   std::vector<CanvasElement*> mElements;
   CanvasControls* mControls{ nullptr };
   float mCursorPos{ -1 };
   CreateCanvasElementFn mElementCreator;
   int mRowOffset{ 0 };
   bool mWrap{ false };
   bool mDragSelecting{ false };
   ofRectangle mDragSelectRect;
   bool mDragCanvasMoving{ false };
   bool mDragCanvasZooming{ false };
   ofVec2f mDragCanvasStartMousePos;
   ofVec2f mDragCanvasStartCanvasPos;
   ofVec2f mDragZoomStartDimensions;
   HighlightEnd mHighlightEnd{ HighlightEnd::kHighlightEnd_None };
   CanvasElement* mHighlightEndElement{ nullptr };
   HighlightEnd mDragEnd{ HighlightEnd::kHighlightEnd_None };
   int mMajorColumnInterval{ -1 };
   bool mHasDuplicatedThisDrag{ false };
   float mScrollVerticalPartial{ 0 };
   std::array<ofColor, 128> mRowColors;

   int mNumRows;
   int mNumCols;
   int mNumVisibleRows;
   DragMode mDragMode{ DragMode::kDragBoth };

   friend CanvasControls;
};
