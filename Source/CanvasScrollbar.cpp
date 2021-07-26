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

CanvasScrollbar::CanvasScrollbar(Canvas* canvas, string name, Style style)
   : mClick(false)
   , mStyle(style)
   , mAutoHide(true)
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

void CanvasScrollbar::OnClicked(int x, int y, bool right)
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
