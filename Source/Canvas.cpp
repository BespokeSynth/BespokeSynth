//
//  Canvas.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/29/14.
//
//

#include "Canvas.h"
#include "CanvasControls.h"
#include "CanvasElement.h"
#include "FileStream.h"
#include "ModularSynth.h"

Canvas::Canvas(IDrawableModule* parent, int x, int y, int w, int h, float length, int rows, int cols, CreateCanvasElementFn elementCreator)
: mClick(false)
, mWidth(w)
, mHeight(h)
, mLength(length)
, mNumRows(rows)
, mNumCols(cols)
, mListener(nullptr)
, mStart(0)
, mEnd(length)
, mCursorPos(-1)
, mElementCreator(elementCreator)
, mClickedElement(nullptr)
, mNumVisibleRows(rows)
, mRowOffset(0)
, mScrolling(false)
, mWrap(true)
, mDragSelecting(false)
, mHighlightEnd(kHighlightEnd_None)
, mHighlightEndElement(nullptr)
, mDragEnd(kHighlightEnd_None)
, mMajorColumnInterval(-1)
, mHasDuplicatedThisDrag(false)
, mDragMode(kDragBoth)
{
   SetName("canvas");
   SetPosition(x,y);
   SetParent(parent);
}

Canvas::~Canvas()
{
   for (int i=0; i<mElements.size(); ++i)
      delete mElements[i];
}

namespace
{
   const float scrollBarSize = 10;
}

void Canvas::Render()
{
   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   float w,h;
   GetDimensions(w,h);
   ofNoFill();
   ofRect(0,0,mWidth,mHeight);
   ofRect(0,0,GetGridWidth(),GetGridHeight());
   
   for (int i=1; i<GetNumVisibleRows(); ++i)
   {
      float pos = i*GetGridHeight()/GetNumVisibleRows();
      ofLine(0, pos, GetGridWidth(), pos);
   }
   
   for (int i=0; i<GetNumCols(); ++i)
   {
      float pos = ofMap(float(i)/GetNumCols(),mStart/mLength,mEnd/mLength,0,1) * GetGridWidth();
      if (pos >= 0 && pos < GetGridWidth())
      {
         ofPushStyle();
         if (mMajorColumnInterval != -1 && i % mMajorColumnInterval == 0)
            ofSetColor(255, 255, 255);
         ofLine(pos, 0, pos, GetGridHeight());
         ofPopStyle();
      }
   }

   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i]->mRow >= mRowOffset && mElements[i]->mRow < mRowOffset + GetNumVisibleRows())
         mElements[i]->Draw();
   }
   
   if (mDragSelecting)
   {
      ofPushStyle();
      ofSetColor(255,255,255);
      ofRect(mDragSelectRect);
      ofPopStyle();
   }
   
   float pos = ofMap(mCursorPos,mStart/mLength,mEnd/mLength,0,1) * GetGridWidth();
   if (pos >= 0 && pos < GetGridWidth())
   {
      ofPushStyle();
      ofSetColor(0,255,0);
      ofLine(pos,0,pos,GetGridHeight());
      ofPopStyle();
   }
   
   if (ShowVerticalScrollBar())
   {
      ofFill();
      ofRect(GetGridWidth(), GetScrollBarTop(),
             scrollBarSize, GetScrollBarBottom()-GetScrollBarTop());
   }
   
   if (ShowHorizontalScrollBar())
   {
      assert(false);
   }
   
   ofPopStyle();
   ofPopMatrix();
}

void Canvas::AddElement(CanvasElement* element)
{
   mElements.push_back(element);
}

void Canvas::RemoveElement(CanvasElement* element)
{
   if (mListener)
      mListener->ElementRemoved(element);
   RemoveFromVector(element, mElements, !K(fail));
   //delete element; TODO(Ryan) figure out how to delete without messing up stuff accessing data from other thread
}

void Canvas::SelectElement(CanvasElement* element)
{
   bool commandHeld = GetKeyModifiers() == kModifier_Command;
   
   if (!commandHeld)
   {
      mClickedElement = element;
      mControls->SetElement(element);
      
      if (element && element->GetHighlighted())
         return;
   }
   
   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i] == element)
      {
         if (commandHeld)
            mElements[i]->SetHighlight(!mElements[i]->GetHighlighted());
         else
            mElements[i]->SetHighlight(true);
      }
      else
      {
         if (!commandHeld)
            mElements[i]->SetHighlight(false);
      }
   }
}

void Canvas::SelectElements(vector<CanvasElement*> elements)
{
   bool commandHeld = GetKeyModifiers() == kModifier_Command;
   mControls->SetElement(elements.empty() ? nullptr : elements[0]);
   
   for (int i=0; i<mElements.size(); ++i)
   {
      if (VectorContains(mElements[i], elements))
      {
         if (commandHeld)
            mElements[i]->SetHighlight(!mElements[i]->GetHighlighted());
         else
            mElements[i]->SetHighlight(true);
      }
      else
      {
         if (!commandHeld)
            mElements[i]->SetHighlight(false);
      }
   }
}

ofVec2f Canvas::RescaleForZoom(float x, float y) const
{
   return ofVec2f(ofMap(x/mWidth,0,1,mStart,mEnd) * mWidth, y);
}

float Canvas::GetGridWidth() const
{
   if (ShowVerticalScrollBar())
      return mWidth - scrollBarSize;
   return mWidth;
}

float Canvas::GetGridHeight() const
{
   if (ShowHorizontalScrollBar())
      return mHeight - scrollBarSize;
   return mHeight;
}

bool Canvas::ShowVerticalScrollBar() const
{
   return GetNumVisibleRows() < mNumRows;
}

bool Canvas::ShowHorizontalScrollBar() const
{
   return false;
}

float Canvas::GetScrollBarTop() const
{
   return ofMap(mRowOffset,0,mNumRows,0,GetGridHeight());
}


float Canvas::GetScrollBarBottom() const
{
   return ofMap(mRowOffset+GetNumVisibleRows(),0,mNumRows,0,GetGridHeight());
}

bool Canvas::IsOnElement(CanvasElement* element, float x, float y) const
{
   return element->GetRect(true, false).contains(x,y) || (mWrap && element->GetRect(true, true).contains(x,y));
}

bool Canvas::IsRowVisible(int row) const
{
   return row >= mRowOffset && row < mRowOffset + GetNumVisibleRows();
}

void Canvas::OnClicked(int x, int y, bool right)
{
   mClick = true;
   mHasDuplicatedThisDrag = false;
   
   if (ShowVerticalScrollBar())
   {
      if (x >= GetGridWidth())
      {
         mScrolling = true;
         mScrollBarOffset = y - GetScrollBarTop();
         return;
      }
   }
   
   if (ShowHorizontalScrollBar())
      assert(false);
   
   if (mHighlightEnd != kHighlightEnd_None)
   {
      mDragEnd = mHighlightEnd;
      mClickedElement = mHighlightEndElement;
   }
   else
   {
      bool clickedElement = false;
      for (int i=(int)mElements.size()-1; i>=0; --i)
      {
         if (IsOnElement(mElements[i], x, y))
         {
            SelectElement(mElements[i]);
            clickedElement = true;
            if (mClickedElement)
            {
               ofRectangle rect = mClickedElement->GetRect(false, false);
               mElementClickOffset.set(x - rect.x, y - rect.y);
            }
            break;
         }
      }
      if (clickedElement == false)
      {
         SelectElement(nullptr);
         
         if (GetKeyModifiers() == kModifier_Shift)
         {
            CanvasCoord coord = GetCoordAt(x, y);
            CanvasElement* element = CreateElement(coord.col,coord.row);
            mElementClickOffset.set(0,0);
            AddElement(element);
            SelectElement(element);
            mHasDuplicatedThisDrag = true;   //to prevent a duplicate from being made
         }
         else
         {
            mDragSelecting = true;
            mDragSelectRect.set(x,y,1,1);
         }
      }
   }
   
   if (mListener)
      mListener->CanvasUpdated(this);
}

float Canvas::QuantizeToGrid(float input) const
{
   float col = int(input * GetNumCols() + .5f);
   return col / GetNumCols();
}

bool Canvas::MouseMoved(float x, float y)
{
   if (mScrolling)
   {
      mRowOffset = ofClamp(int(ofMap(y - mScrollBarOffset, 0, GetGridHeight(), 0, mNumRows)+.5f), 0, mNumRows - GetNumVisibleRows());
      return true;
   }
   
   if (mDragEnd != kHighlightEnd_None)
   {
      ofVec2f scaled = RescaleForZoom(x, y);
      bool quantize = GetKeyModifiers() == kModifier_Command;
      if (mDragEnd == kHighlightEnd_Start)
      {
         float oldStart = mClickedElement->GetStart();
         float newStart = scaled.x / GetGridWidth() / mLength;
         float startDelta = newStart - oldStart;
         for (auto* element : mElements)
         {
            if (element->GetHighlighted())
            {
               float start = element->GetStart() + startDelta;
               if (quantize)
                  start = QuantizeToGrid(start);
               element->SetStart(start);
            }
         }
      }
      if (mDragEnd == kHighlightEnd_End)
      {
         float oldEnd = mClickedElement->GetEnd();
         float newEnd = scaled.x / GetGridWidth() / mLength;
         float endDelta = newEnd - oldEnd;
         for (auto* element : mElements)
         {
            if (element->GetHighlighted())
            {
               float end = element->GetEnd() + endDelta;
               if (quantize)
                  end = QuantizeToGrid(end);
               element->SetEnd(end);
            }
         }
      }
      return true;
   }
   
   if (x >= 0 && x <mWidth && y>=0 && y<mHeight)
   {
      if (mClick)
      {
         int colShift = 0;
         int rowShift = 0;
         
         if (mClickedElement)
         {
            x -= mElementClickOffset.x;
            y -= mElementClickOffset.y;
            ofVec2f scaled = RescaleForZoom(x, y);
            int newCol = mClickedElement->mCol;
            if (mDragMode & kDragHorizontal)
               newCol = ofClamp(int(((scaled.x / GetGridWidth() / mLength) * mNumCols) + .5f), 0, mNumCols-1);
            int newRow = mClickedElement->mRow;;
            if (mDragMode & kDragVertical)
               newRow = ofClamp(int(((scaled.y / GetGridHeight()) * GetNumVisibleRows()) + .5f), 0, GetNumVisibleRows()-1) + mRowOffset;
            colShift = newCol - mClickedElement->mCol;
            rowShift = newRow - mClickedElement->mRow;
            
            if (colShift != 0 || rowShift != 0) //duplicate only if we've dragged to a new position
            {
               if (GetKeyModifiers() == kModifier_Shift && !mHasDuplicatedThisDrag)
               {
                  mHasDuplicatedThisDrag = true;
                  vector<CanvasElement*> newElements;
                  for (auto element : mElements)
                  {
                     if (element->GetHighlighted())
                        newElements.push_back(element->CreateDuplicate());
                  }
                  for (auto newElement : newElements)
                     mElements.push_back(newElement);
               }
            }
            
            mClickedElement->mCol = newCol;
            mClickedElement->mRow = newRow;
            if (mListener)
               mListener->CanvasUpdated(this);
         }
         for (auto* element : mElements)
         {
            if (element->GetHighlighted() && element != mClickedElement)
            {
               element->mCol += colShift;
               element->mRow += rowShift;
            }
         }
      }
      mHighlightEnd = kHighlightEnd_None;
      mHighlightEndElement = nullptr;
      for (auto* element : mElements)
      {
         if (element->GetHighlighted())
         {
            ofRectangle rect = element->GetRect(!K(clamp), !K(wrapped));
            float startX = rect.x;
            if (element->GetEnd() > 1 && mWrap)
               rect = element->GetRect(!K(clamp), K(wrapped));
            float endX = rect.x + rect.width;
            if (y >= rect.y && y < rect.y + rect.height)
            {
               if (fabsf(startX-x) < 3/gDrawScale)
               {
                  mHighlightEnd = kHighlightEnd_Start;
                  mHighlightEndElement = element;
               }
               if (fabsf(endX-x) < 3/gDrawScale && element->IsResizable())
               {
                  mHighlightEnd = kHighlightEnd_End;
                  mHighlightEndElement = element;
               }
            }
         }
      }
   }
   else
   {
   }
   
   if (mDragSelecting)
   {
      mDragSelectRect.width = x - mDragSelectRect.x;
      mDragSelectRect.height = y - mDragSelectRect.y;
   }
   
   return false;
}

void Canvas::MouseReleased()
{
   mClick = false;
   mClickedElement = nullptr;
   mScrolling = false;
   mDragEnd = kHighlightEnd_None;
   
   if (mDragSelecting)
   {
      vector<CanvasElement*> selectedElements;
      for (int i=(int)mElements.size()-1; i>=0; --i)
      {
         if (mElements[i]->GetRect(true, false).intersects(mDragSelectRect) ||
             (mWrap && mElements[i]->GetRect(true, true).intersects(mDragSelectRect)))
         {
            selectedElements.push_back(mElements[i]);
         }
      }
      SelectElements(selectedElements);
      mDragSelecting = false;
   }
}

void Canvas::KeyPressed(int key, bool isRepeat)
{
   if (TheSynth->GetLastClickedModule() == GetParent() && gHoveredUIControl == nullptr)
   {
      if (key == OF_KEY_BACKSPACE)
      {
         mControls->SetElement(nullptr);
         
         vector<CanvasElement*> toRemove;
         for (auto element : mElements)
         {
            if (element->GetHighlighted())
               toRemove.push_back(element);
         }
         for (auto element : toRemove)
         {
            RemoveElement(element);
         }
      }
      if (key == 'a' && GetKeyModifiers() == kModifier_Command)
      {
         for (auto* element : mElements)
            element->SetHighlight(true);
      }
      if (key == OF_KEY_LEFT || key == OF_KEY_RIGHT)
      {
         int direction = (key == OF_KEY_LEFT) ? -1 : 1;
         for (auto* element : mElements)
         {
            if (element->GetHighlighted())
               element->mCol += direction;
         }
      }
      if (key == OF_KEY_UP || key == OF_KEY_DOWN)
      {
         int direction = (key == OF_KEY_UP) ? -1 : 1;
         for (auto* element : mElements)
         {
            if (element->GetHighlighted())
               element->mRow += direction;
         }
      }
   }
}

void Canvas::RescaleNumCols(int cols)
{
   float ratio = (float)cols / mNumCols;
   for (auto* element : mElements)
   {
      element->SetStart(element->GetStart() * ratio);
      element->mLength *= ratio;
   }
   mNumCols = cols;
}

CanvasElement* Canvas::GetElementAt(float pos, int row)
{
   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i]->mRow == row && pos >= mElements[i]->GetStart() && pos < mElements[i]->GetEnd())
         return mElements[i];
      else if (mWrap && pos >= mElements[i]->GetStart() - mLength && pos < mElements[i]->GetEnd() - mLength)
         return mElements[i];
   }
   return nullptr;
}

void Canvas::FillElementsAt(float pos, vector<CanvasElement*>& elementsAt) const
{
   for (int i=0; i<mElements.size(); ++i)
   {
      if (mElements[i]->mRow == -1 || mElements[i]->mCol == -1 || mElements[i]->mRow >= elementsAt.size())
         continue;
      
      bool on = false;
      if (pos >= mElements[i]->GetStart() && pos < mElements[i]->GetEnd())
         on = true;
      if (mWrap && pos >= mElements[i]->GetStart() - mLength && pos < mElements[i]->GetEnd() - mLength)
         on = true;
      if (on)
         elementsAt[mElements[i]->mRow] = mElements[i];
   }
}

CanvasCoord Canvas::GetCoordAt(int x, int y)
{
   ofVec2f scaled = RescaleForZoom(x, y);
   x = scaled.x;
   y = scaled.y;
   if (x >= 0 && x <GetGridWidth() && y>=0 && y<GetGridHeight())
      return CanvasCoord((x / GetGridWidth() / mLength) * mNumCols, (y / GetGridHeight()) * GetNumVisibleRows() + mRowOffset);
   return CanvasCoord(-1,-1);
}

void Canvas::Clear()
{
   mElements.clear();
}

namespace
{
   const int kSaveStateRev = 2;
}

void Canvas::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mNumCols;
   out << mNumRows;
   out << mNumVisibleRows;
   out << mRowOffset;
   out << (int)mElements.size();
   for (int i=0; i<mElements.size(); ++i)
   {
      out << mElements[i]->mCol;
      out << mElements[i]->mRow;
      mElements[i]->SaveState(out);
   }
}

void Canvas::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   in >> mNumCols;
   in >> mNumRows;
   in >> mNumVisibleRows;
   in >> mRowOffset;
   mElements.clear();
   int size;
   in >> size;
   for (int i=0; i<size; ++i)
   {
      int col=0,row=0;
      if (rev >= 2)
      {
         in >> col;
         in >> row;
      }
      CanvasElement* element = mElementCreator(this,col,row);
      element->LoadState(in);
      mElements.push_back(element);
   }
}
