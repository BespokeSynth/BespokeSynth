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
   CanvasTimeline(Canvas* canvas, std::string name);
   ~CanvasTimeline() {}

   void SetDimensions(double width, double height)
   {
      mWidth = width;
      mHeight = height;
   }

   //IUIControl
   void SetFromMidiCC(double slider, double time, bool setViaModulator) override {}
   void SetValue(double value, double time, bool forceUpdate = false) override {}
   void KeyPressed(int key, bool isRepeat) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool GetNoHover() const override { return true; }

   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(double x, double y) override;
   bool MouseScrolled(double x, double y, double scrollX, double scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

private:
   void OnClicked(double x, double y, bool right) override;
   void GetDimensions(double& width, double& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   enum class HoverMode
   {
      kNone,
      kStart,
      kEnd,
      kMiddle
   };

   void DrawTriangle(double posX, int direction);
   double GetQuantizedForX(double posX, HoverMode clampSide);

   double mWidth{ 200 };
   double mHeight{ 20 };
   bool mClick{ false };
   ofVec2d mClickMousePos;
   ofVec2d mDragOffset;
   HoverMode mHoverMode{ HoverMode::kNone };

   Canvas* mCanvas{ nullptr };
};
