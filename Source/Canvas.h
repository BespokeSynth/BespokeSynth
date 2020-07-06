//
//  Canvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/29/14.
//
//

#ifndef __Bespoke__Canvas__
#define __Bespoke__Canvas__

#include <iostream>
#include "IUIControl.h"
#include "CanvasElement.h"

#define MAX_CANVAS_MASK_ELEMENTS 128

class Canvas;
class CanvasControls;
class CanvasElement;

typedef CanvasElement* (*CreateCanvasElementFn)(Canvas* canvas,int col,int row);

class ICanvasListener
{
public:
   virtual ~ICanvasListener() {}
   virtual void CanvasUpdated(Canvas* canvas) = 0;
   virtual void ElementRemoved(CanvasElement* element) {}
};

struct CanvasCoord
{
   CanvasCoord(int _col, int _row) : col(_col), row(_row) {}
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
   void Clear();
   void SetListener(ICanvasListener* listener) { mListener = listener; }
   void SetDimensions(int width, int height) { mWidth = width; mHeight = height; }
   int GetWidth() const { return mWidth; }
   int GetHeight() const { return mHeight; }
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
   void SelectElements(vector<CanvasElement*> elements);
   void SetControls(CanvasControls* controls) { mControls = controls; }
   CanvasControls* GetControls() { return mControls; }
   vector<CanvasElement*>& GetElements() { return mElements; }
   void FillElementsAt(float pos, vector<CanvasElement*>& elements) const;
   CanvasElement* GetElementAt(float pos, int row);
   void SetCursorPos(float pos) { mCursorPos = pos; }
   float GetCursorPos() const { return mCursorPos; }
   CanvasElement* CreateElement(int col, int row) { return mElementCreator(this,col,row); }
   CanvasCoord GetCoordAt(int x, int y);
   void SetNumVisibleRows(int rows) { mNumVisibleRows = rows; }
   int GetNumVisibleRows() const { return MIN(mNumVisibleRows, mNumRows); }
   void SetRowOffset(int offset) { mRowOffset = ofClamp(offset,0,mNumRows-mNumVisibleRows); }
   int GetRowOffset() const { return mRowOffset; }
   float GetGridWidth() const;
   float GetGridHeight() const;
   bool ShouldWrap() const { return mWrap; }
   HighlightEnd GetHighlightEnd() const { return mHighlightEnd; }
   void SetMajorColumnInterval(int interval) { mMajorColumnInterval = interval; }
   void SetDragMode(DragMode mode) { mDragMode = mode; }
   bool IsRowVisible(int row) const;
   
   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void KeyPressed(int key, bool isRepeat) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   
   float mStart;
   float mEnd;
   
private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   ofVec2f RescaleForZoom(float x, float y) const;
   
   bool ShowVerticalScrollBar() const;
   bool ShowHorizontalScrollBar() const;
   float GetScrollBarTop() const;
   float GetScrollBarBottom() const;
   bool IsOnElement(CanvasElement* element, float x, float y) const;
   float QuantizeToGrid(float input) const;
   
   bool mClick;
   CanvasElement* mClickedElement;
   ofVec2f mElementClickOffset;
   float mWidth;
   float mHeight;
   float mLength;
   ICanvasListener* mListener;
   vector<CanvasElement*> mElements;
   CanvasControls* mControls;
   float mCursorPos;
   CreateCanvasElementFn mElementCreator;
   int mRowOffset;
   bool mScrolling;
   float mScrollBarOffset;
   bool mWrap;
   bool mDragSelecting;
   ofRectangle mDragSelectRect;
   HighlightEnd mHighlightEnd;
   CanvasElement* mHighlightEndElement;
   HighlightEnd mDragEnd;
   int mMajorColumnInterval;
   bool mHasDuplicatedThisDrag;
   
   int mNumRows;
   int mNumCols;
   int mNumVisibleRows;
   DragMode mDragMode;
   
   friend CanvasControls;
};

#endif /* defined(__Bespoke__Canvas__) */
