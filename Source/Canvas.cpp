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
, mViewStart(0)
, mViewEnd(length)
, mLoopStart(0)
, mLoopEnd(length)
, mControls(nullptr)
, mCursorPos(-1)
, mElementCreator(elementCreator)
, mClickedElement(nullptr)
, mNumVisibleRows(rows)
, mRowOffset(0)
, mWrap(false)
, mDragSelecting(false)
, mDragCanvasMoving(false)
, mDragCanvasZooming(false)
, mHighlightEnd(kHighlightEnd_None)
, mHighlightEndElement(nullptr)
, mDragEnd(kHighlightEnd_None)
, mMajorColumnInterval(-1)
, mHasDuplicatedThisDrag(false)
, mScrollVerticalPartial(0)
, mDragMode(kDragBoth)
{
   SetName("canvas");
   SetPosition(x, y);
   SetParent(parent);

   for (size_t i = 0; i < mRowColors.size(); ++i)
      mRowColors[i] = ofColor(200, 200, 200, 70);
}

Canvas::~Canvas()
{
   for (int i = 0; i < mElements.size(); ++i)
      delete mElements[i];
}

void Canvas::Render()
{
   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   ofSetLineWidth(.5f);
   float w, h;
   GetDimensions(w, h);
   ofNoFill();
   ofRect(0, 0, mWidth, mHeight, 0);
   ofRect(0, 0, GetWidth(), GetHeight(), 0);

   ofPushStyle();
   ofFill();
   const float rowHeight = GetHeight() / GetNumVisibleRows();
   for (int i = 0; i < GetNumVisibleRows(); ++i)
   {
      int row = mRowOffset + i;
      if (row >= 0 && row < mRowColors.size())
         ofSetColorGradient(mRowColors[row], ofColor::lerp(mRowColors[row], ofColor::clear, .1f), ofVec2f(0, i * rowHeight + rowHeight * 0.0f), ofVec2f(0, i * rowHeight + rowHeight));
      ofRect(0, i * rowHeight, GetWidth(), rowHeight, 0);
   }
   ofPopStyle();

   for (int i = 0; i < GetNumCols(); ++i)
   {
      float pos = ofMap(float(i) / GetNumCols(), mViewStart / mLength, mViewEnd / mLength, 0, 1) * GetWidth();
      if (pos >= 0 && pos < GetWidth())
      {
         ofPushStyle();
         if (mMajorColumnInterval != -1 && i % mMajorColumnInterval == 0)
            ofSetColor(255, 255, 255);
         ofLine(pos, 0, pos, GetHeight());
         ofPopStyle();
      }
   }

   for (int i = 0; i < mElements.size(); ++i)
   {
      //ofMap(GetStart() + offset,mCanvas->mStart/mCanvas->GetLength(),mCanvas->mEnd/mCanvas->GetLength(),0,1,clamp)
      bool visibleOnCanvas = mElements[i]->mRow >= mRowOffset && mElements[i]->mRow < mRowOffset + GetNumVisibleRows() &&
                             mElements[i]->GetStart() <= mViewEnd / GetLength() && mElements[i]->GetEnd() >= mViewStart / GetLength();
      if (visibleOnCanvas)
      {
         ofVec2f offset(0, 0);
         if (mClick && mClickedElement != nullptr && mElements[i]->GetHighlighted() && mDragEnd == kHighlightEnd_None)
            offset = (ofVec2f(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY()) - mClickedElementStartMousePos) / gDrawScale;
         mElements[i]->Draw(offset);
      }

      if (!visibleOnCanvas)
         mElements[i]->DrawOffscreen();
   }

   if (mDragSelecting)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofRect(mDragSelectRect);
      ofPopStyle();
   }

   float pos = ofMap(mCursorPos, mViewStart / mLength, mViewEnd / mLength, 0, 1) * GetWidth();
   if (pos >= 0 && pos < GetWidth())
   {
      ofPushStyle();
      ofSetColor(0, 255, 0);
      ofLine(pos, 0, pos, GetHeight());
      ofPopStyle();
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
   bool commandHeld = GetKeyModifiers() & kModifier_Command;

   if (!commandHeld)
   {
      mClickedElement = element;
      if (mControls)
         mControls->SetElement(element);

      if (element && element->GetHighlighted())
         return;
   }

   for (int i = 0; i < mElements.size(); ++i)
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

void Canvas::SelectElements(std::vector<CanvasElement*> elements)
{
   bool commandHeld = GetKeyModifiers() & kModifier_Command;
   if (mControls)
      mControls->SetElement(elements.empty() ? nullptr : elements[0]);

   for (int i = 0; i < mElements.size(); ++i)
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
   return ofVec2f(ofMap(x / mWidth, 0, 1, mViewStart, mViewEnd) * mWidth, y);
}

bool Canvas::IsOnElement(CanvasElement* element, float x, float y) const
{
   return element->GetRect(true, false).contains(x, y) || (mWrap && element->GetRect(true, true).contains(x, y));
}

bool Canvas::IsRowVisible(int row) const
{
   return row >= mRowOffset && row < mRowOffset + GetNumVisibleRows();
}

void Canvas::OnClicked(float x, float y, bool right)
{
   mClick = true;
   mHasDuplicatedThisDrag = false;

   if (mHighlightEnd != kHighlightEnd_None)
   {
      mDragEnd = mHighlightEnd;
      mClickedElement = mHighlightEndElement;
   }
   else
   {
      bool clickedElement = false;
      for (int i = (int)mElements.size() - 1; i >= 0; --i)
      {
         if (IsOnElement(mElements[i], x, y))
         {
            SelectElement(mElements[i]);
            clickedElement = true;
            if (mClickedElement)
               mClickedElementStartMousePos.set(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());

            break;
         }
      }
      if (clickedElement == false)
      {
         SelectElement(nullptr);

         if (GetKeyModifiers() & kModifier_Shift)
         {
            CanvasCoord coord = GetCoordAt(x, y);
            CanvasElement* element = CreateElement(coord.col, coord.row);
            AddElement(element);
            SelectElement(element);
            mHasDuplicatedThisDrag = true; //to prevent a duplicate from being made
            mClickedElementStartMousePos.set(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
         }
         else if (GetKeyModifiers() & kModifier_Alt)
         {
            mDragCanvasMoving = true;
            mDragCanvasStartMousePos.set(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
            mDragCanvasStartCanvasPos.set(mViewStart, mRowOffset);
         }
         else if (GetKeyModifiers() & kModifier_Command)
         {
            mDragCanvasZooming = true;
            mDragCanvasStartMousePos.set(x, y);
            mDragCanvasStartCanvasPos.set(ofMap(x, 0, mWidth, mViewStart, mViewEnd), ofMap(y, 0, mHeight, mRowOffset, mRowOffset + mNumVisibleRows));
            mDragZoomStartDimensions.set(mViewEnd - mViewStart, mNumVisibleRows);
         }
         else
         {
            mDragSelecting = true;
            mDragSelectRect.set(x, y, 1, 1);
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
   CheckHover(x, y);

   bool quantize = GetKeyModifiers() & kModifier_Command;

   if (mDragEnd != kHighlightEnd_None)
   {
      ofVec2f scaled = RescaleForZoom(x, y);
      if (mDragEnd == kHighlightEnd_Start)
      {
         float oldStart = mClickedElement->GetStart();
         float newStart = scaled.x / GetWidth() / mLength;
         float startDelta = newStart - oldStart;
         for (auto* element : mElements)
         {
            if (element->GetHighlighted())
            {
               float start = element->GetStart() + startDelta;
               if (quantize)
                  start = QuantizeToGrid(start);
               element->SetStart(start, false);
            }
         }
      }
      if (mDragEnd == kHighlightEnd_End)
      {
         float oldEnd = mClickedElement->GetEnd();
         float newEnd = scaled.x / GetWidth() / mLength;
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

   if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
   {
      if (mClick)
      {
         if (mClickedElement != nullptr)
         {
            ofVec2f dragOffset = (ofVec2f(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY()) - mClickedElementStartMousePos) / gDrawScale;

            if (GetKeyModifiers() & kModifier_Shift && !mHasDuplicatedThisDrag && dragOffset.distanceSquared() > 9)
            {
               mHasDuplicatedThisDrag = true;
               std::vector<CanvasElement*> newElements;
               for (auto element : mElements)
               {
                  if (element->GetHighlighted())
                     newElements.push_back(element->CreateDuplicate());
               }
               for (auto newElement : newElements)
                  mElements.push_back(newElement);
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
               if (fabsf(startX - x) < 3 / gDrawScale && element->IsResizable())
               {
                  mHighlightEnd = kHighlightEnd_Start;
                  mHighlightEndElement = element;
               }
               if (fabsf(endX - x) < 3 / gDrawScale && element->IsResizable())
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

   if (mDragCanvasMoving && (GetKeyModifiers() & kModifier_Alt))
   {

      ofVec2f mousePos(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
      ofVec2f delta = (mousePos - mDragCanvasStartMousePos) / gDrawScale;

      float viewLength = mViewEnd - mViewStart;
      float moveX = -delta.x / mWidth * viewLength;
      mViewStart = ofClamp(mDragCanvasStartCanvasPos.x + moveX, 0, mLength - viewLength);
      mViewEnd = mViewStart + viewLength;

      float moveY = -delta.y / mHeight * GetNumVisibleRows();
      mRowOffset = ofClamp(int(mDragCanvasStartCanvasPos.y + moveY + .5f), 0, GetNumRows() - GetNumVisibleRows());
   }
   else
   {
      mDragCanvasMoving = false;
   }

   if (mDragCanvasZooming && (GetKeyModifiers() & kModifier_Command))
   {
      ofVec2f mousePos(x, y);
      ofVec2f delta = (mousePos - mDragCanvasStartMousePos) / gDrawScale;

      {
         float originalViewLength = mDragZoomStartDimensions.x;
         float originalViewCenterX = mDragCanvasStartCanvasPos.x;
         float newViewLength = MAX((1 - delta.x / mWidth) * originalViewLength, .01f);
         mViewStart = ofClamp(originalViewCenterX - newViewLength * (mDragCanvasStartMousePos.x / mWidth), 0, mLength);
         mViewEnd = ofClamp(originalViewCenterX + newViewLength * (1 - mDragCanvasStartMousePos.x / mWidth), 0, mLength);
      }

      {
         float originalViewHeight = mDragZoomStartDimensions.y;
         float originalViewCenterY = mDragCanvasStartCanvasPos.y;
         float newViewHeight = MAX((1 + delta.y / mHeight) * originalViewHeight, .01f);
         mRowOffset = int(ofClamp(originalViewCenterY - newViewHeight * (mDragCanvasStartMousePos.y / mHeight), 0, mNumRows - 1) + .5f);
         mNumVisibleRows = int(ofClamp(originalViewCenterY + newViewHeight * (1 - mDragCanvasStartMousePos.y / mHeight), mRowOffset + 1, mNumRows) + .5f) - mRowOffset;
      }
   }
   else
   {
      mDragCanvasZooming = false;
   }

   return false;
}

void Canvas::MouseReleased()
{
   if (mClick && mClickedElement != nullptr && mDragEnd == kHighlightEnd_None)
   {
      for (auto* element : mElements)
      {
         if (element->GetHighlighted())
            element->MoveElementByDrag((ofVec2f(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY()) - mClickedElementStartMousePos) / gDrawScale);
      }

      if (mListener)
         mListener->CanvasUpdated(this);
   }

   mClick = false;
   mClickedElement = nullptr;
   mDragEnd = kHighlightEnd_None;

   if (mDragSelecting)
   {
      std::vector<CanvasElement*> selectedElements;
      for (int i = (int)mElements.size() - 1; i >= 0; --i)
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

   mDragCanvasMoving = false;
   mDragCanvasZooming = false;
}

bool Canvas::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (GetKeyModifiers() & kModifier_Alt)
   {
      scrollX = scrollY;
      scrollY = 0;
   }

   if (GetKeyModifiers() & kModifier_Shift)
   {
      float canvasX, canvasY;
      GetPosition(canvasX, canvasY, false);
      ofVec2f canvasPos = ofVec2f(ofMap(x, canvasX, canvasX + GetWidth(), 0, 1),
                                  ofMap(y, canvasY, canvasY + GetHeight(), 0, 1));
      if (IsInUnitBox(canvasPos))
      {
         float zoomCenter = ofLerp(mViewStart, mViewEnd, canvasPos.x);
         float distFromStart = zoomCenter - mViewStart;
         float distFromEnd = zoomCenter - mViewEnd;

         distFromStart *= 1 - scrollY / 50;
         distFromEnd *= 1 - scrollY / 50;

         mViewStart = ofClamp(zoomCenter - distFromStart, 0, mLength);
         mViewEnd = ofClamp(zoomCenter - distFromEnd, 0, mLength);
         //ofLog() << mStart << " " << mEnd;
         return true;
      }
   }
   else
   {
      if (x >= GetPosition(false).x && y >= GetPosition(false).y &&
          x < GetPosition(false).x + GetWidth() && y < GetPosition(false).y + GetHeight())
      {
         mScrollVerticalPartial -= scrollY;
         int scrollWhole = int(mScrollVerticalPartial);
         mScrollVerticalPartial -= scrollWhole;
         SetRowOffset(GetRowOffset() + scrollWhole);
      }

      float slideX = (mViewEnd - mViewStart) * -scrollX / 50;
      if (slideX > 0)
         slideX = MIN(slideX, mLength - mViewEnd);
      if (slideX < 0)
         slideX = MAX(slideX, -mViewStart);
      mViewStart += slideX;
      mViewEnd += slideX;

      return true;
   }
   return false;
}

void Canvas::KeyPressed(int key, bool isRepeat)
{
   if (TheSynth->GetLastClickedModule() == GetParent() && gHoveredUIControl == this)
   {
      if (key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey)
      {
         if (mControls)
            mControls->SetElement(nullptr);

         std::vector<CanvasElement*> toRemove;
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
      if (key == 'a' && GetKeyModifiers() & kModifier_Command)
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
      element->SetStart(element->GetStart() * ratio, true);
      element->mLength *= ratio;
   }
   mNumCols = cols;
}

void Canvas::SetRowColor(int row, ofColor color)
{
   if (row >= 0 && row <= mRowColors.size())
      mRowColors[row] = color;
}

juce::MouseCursor Canvas::GetMouseCursorType()
{
   using juce::MouseCursor;
   if (GetKeyModifiers() & kModifier_Shift)
      return MouseCursor::CopyingCursor;
   if (GetKeyModifiers() & kModifier_Alt)
      return MouseCursor::DraggingHandCursor;
   if (GetKeyModifiers() & kModifier_Command)
      return MouseCursor::IBeamCursor;
   if (mHighlightEnd == kHighlightEnd_Start)
      return MouseCursor::RightEdgeResizeCursor;
   if (mHighlightEnd == kHighlightEnd_End)
      return MouseCursor::LeftEdgeResizeCursor;
   return MouseCursor::NormalCursor;
}

CanvasElement* Canvas::GetElementAt(float pos, int row)
{
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i]->mRow == row && pos >= mElements[i]->GetStart() && pos < mElements[i]->GetEnd())
         return mElements[i];
      else if (mWrap && pos >= mElements[i]->GetStart() - mLength && pos < mElements[i]->GetEnd() - mLength)
         return mElements[i];
   }
   return nullptr;
}

void Canvas::FillElementsAt(float pos, std::vector<CanvasElement*>& elementsAt) const
{
   for (int i = 0; i < mElements.size(); ++i)
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

void Canvas::EraseElementsAt(float pos)
{
   std::vector<CanvasElement*> toErase;
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i]->mRow == -1 || mElements[i]->mCol == -1)
         continue;

      bool on = false;
      if (pos >= mElements[i]->GetStart() && pos < mElements[i]->GetEnd())
         on = true;
      if (mWrap && pos >= mElements[i]->GetStart() - mLength && pos < mElements[i]->GetEnd() - mLength)
         on = true;
      if (on)
         toErase.push_back(mElements[i]);
   }

   for (auto* elem : toErase)
      RemoveElement(elem);
}

CanvasCoord Canvas::GetCoordAt(int x, int y)
{
   if (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight())
   {
      int col = int(ofMap(x / mWidth, 0, mLength, mViewStart, mViewEnd) * mNumCols);
      int row = (y / GetHeight()) * GetNumVisibleRows() + mRowOffset;
      return CanvasCoord(col, row);
   }
   return CanvasCoord(-1, -1);
}

void Canvas::Clear()
{
   mElements.clear();
}

namespace
{
   const int kSaveStateRev = 3;
}

void Canvas::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mNumCols;
   out << mNumRows;
   out << mNumVisibleRows;
   out << mRowOffset;
   out << mLoopStart;
   out << mLoopEnd;
   out << (int)mElements.size();
   for (int i = 0; i < mElements.size(); ++i)
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
   if (rev >= 3)
   {
      in >> mLoopStart;
      in >> mLoopEnd;
   }
   mElements.clear();
   int size;
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      int col = 0, row = 0;
      if (rev >= 2)
      {
         in >> col;
         in >> row;
      }
      CanvasElement* element = mElementCreator(this, col, row);
      element->LoadState(in);
      mElements.push_back(element);
   }
}
