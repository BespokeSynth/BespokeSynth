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
/*
  ==============================================================================

    CanvasScrollbar.cpp
    Created: 22 Mar 2021 12:19:47am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "CanvasScrollbar.h"
#include "Canvas.h"
#include "ModularSynth.h"

CanvasScrollbar::CanvasScrollbar(Canvas* canvas, std::string name, Style style)
: mStyle(style)
, mCanvas(canvas)
{
   SetName(name.c_str());
   SetParent(canvas->GetModuleParent());
}

void CanvasScrollbar::Render()
{
   ofRectangle canvasRect = mCanvas->GetRect(true);
   if (mStyle == Style::kHorizontal)
   {
      SetPosition(canvasRect.x, canvasRect.getMaxY());
      SetDimensions(canvasRect.width, 10);
   }
   if (mStyle == Style::kVertical)
   {
      SetPosition(canvasRect.getMaxX(), canvasRect.y);
      SetDimensions(10, canvasRect.height);
   }

   if (mAutoHide && GetBarStart() == 0)
   {
      if (mStyle == Style::kHorizontal && GetBarEnd() == mWidth)
         return;
      if (mStyle == Style::kVertical && GetBarEnd() == mHeight)
         return;
   }

   ofPushMatrix();
   ofTranslate(mX, mY);
   ofPushStyle();
   ofSetLineWidth(.5f);
   float w, h;
   GetDimensions(w, h);
   ofFill();
   ofRect(0, 0, mWidth, mHeight);
   ofSetColor(255, 255, 255);
   if (mStyle == Style::kHorizontal)
      ofRect(GetBarStart(), 0, GetBarEnd() - GetBarStart(), mHeight);
   if (mStyle == Style::kVertical)
      ofRect(0, GetBarStart(), mWidth, GetBarEnd() - GetBarStart());
   ofPopStyle();
   ofPopMatrix();
}

float CanvasScrollbar::GetBarStart() const
{
   if (mStyle == Style::kHorizontal)
      return mCanvas->mViewStart / mCanvas->GetLength() * mWidth;
   if (mStyle == Style::kVertical)
      return ofMap(mCanvas->GetRowOffset(), 0, mCanvas->GetNumRows(), 0, mHeight);
   return 0;
}

float CanvasScrollbar::GetBarEnd() const
{
   if (mStyle == Style::kHorizontal)
      return mCanvas->mViewEnd / mCanvas->GetLength() * mWidth;
   if (mStyle == Style::kVertical)
      return ofMap(mCanvas->GetRowOffset() + mCanvas->GetNumVisibleRows(), 0, mCanvas->GetNumRows(), 0, mHeight);
   return 1;
}

void CanvasScrollbar::OnClicked(float x, float y, bool right)
{
   mClickMousePos.set(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
   mDragOffset.set(0, 0);
   mClick = true;
   if (mStyle == Style::kHorizontal)
      mScrollBarOffset = x - GetBarStart();
   if (mStyle == Style::kVertical)
      mScrollBarOffset = y - GetBarStart();
}

void CanvasScrollbar::MouseReleased()
{
   mClick = false;
}

bool CanvasScrollbar::MouseMoved(float x, float y)
{
   CheckHover(x, y);

   if (mClick)
   {
      mDragOffset = (ofVec2f(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY()) - mClickMousePos) / gDrawScale;
      if (mStyle == Style::kHorizontal)
      {
         float viewLength = mCanvas->mViewEnd - mCanvas->mViewStart;
         mCanvas->mViewStart = ofClamp((x - mScrollBarOffset) / mWidth * mCanvas->GetLength(), 0, mCanvas->GetLength() - viewLength);
         mCanvas->mViewEnd = mCanvas->mViewStart + viewLength;
      }
      if (mStyle == Style::kVertical)
         mCanvas->SetRowOffset(ofClamp(int(ofMap(y - mScrollBarOffset, 0, mHeight, 0, mCanvas->GetNumRows()) + .5f), 0, mCanvas->GetNumRows() - mCanvas->GetNumVisibleRows()));
   }

   return false;
}

bool CanvasScrollbar::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   return false;
}

void CanvasScrollbar::SaveState(FileStreamOut& out)
{
}

void CanvasScrollbar::LoadState(FileStreamIn& in, bool shouldSetValue)
{
}
