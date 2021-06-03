/*
  ==============================================================================

    CanvasScrollbar.h
    Created: 22 Mar 2021 12:19:47am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IUIControl.h"

class Canvas;

class CanvasScrollbar : public IUIControl
{
public:
   enum class Style
   {
      kHorizontal,
      kVertical
   };

   CanvasScrollbar(Canvas* canvas, string name, Style style);
   ~CanvasScrollbar() {}

   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override {}
   void SetValue(float value) override {}
   void KeyPressed(int key, bool isRepeat) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }

   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }

   float GetBarStart() const;
   float GetBarEnd() const;   

   float mWidth;
   float mHeight;
   bool mClick;
   ofVec2f mClickMousePos;
   ofVec2f mDragOffset;
   float mScrollBarOffset;
   Style mStyle;
   bool mAutoHide;

   Canvas* mCanvas;
};