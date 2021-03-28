/*
  ==============================================================================

    CanvasTimeline.h
    Created: 20 Mar 2021 12:35:05am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IUIControl.h"

class Canvas;

class CanvasTimeline : public IUIControl
{
public:
   CanvasTimeline(Canvas* canvas, string name);
   ~CanvasTimeline() {}

   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }

   //IUIControl
   void SetFromMidiCC(float slider) override {}
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

   enum class HoverMode
   {
      kNone,
      kStart,
      kEnd,
      kMiddle
   };

   void DrawTriangle(float posX, int direction);
   float GetQuantizedForX(float posX, HoverMode clampSide);

   float mWidth;
   float mHeight;
   bool mClick;
   ofVec2f mClickMousePos;
   ofVec2f mDragOffset;
   HoverMode mHoverMode;

   Canvas* mCanvas;
};
