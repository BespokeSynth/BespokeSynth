/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    TouchInterfacePanel.h
    Created: 23 Nov 2026
    Author:  Ryan Challinor

    Translucent panel of touch-friendly vertical sliders for a module's controls,
    covering everything below the title bar. shown when tapping a module in TouchInteractionMode::Interface.

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"

class IUIControl;

class TouchInterfacePanel : public IDrawableModule
{
public:
   TouchInterfacePanel();

   void Render() override;
   void DrawModule() override;
   void Poll() override;
   bool HasTitleBar() const override { return false; }
   bool AlwaysOnTop() override { return true; }
   bool CanMinimize() override { return false; }
   bool IsSaveable() override { return false; }
   bool IsSingleton() const override { return true; }
   bool IsEnabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   void MouseReleased() override;

   bool ShowForModule(IDrawableModule* module); //returns false if the module doesn't have any controls to show
   void Hide();

private:
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   int GetNumPages() const;
   int GetNumColumns() const;
   void ChangePage(int direction);
   void UpdateControlList();
   bool IsValidControl(IUIControl* control) const;

   ofRectangle GetContentRect() const; //the area of the screen that isn't cut off by notches, rounded corners, etc
   ofRectangle GetExitButtonRect() const;
   ofRectangle GetPageButtonRect(int direction) const;
   ofRectangle GetColumnRect(int column) const; //column on the current page
   ofRectangle GetTrackRect(int column) const;
   ofRectangle GetSquareRect(int column) const; //for buttons and checkboxes
   float GetTitleBarHeight() const;
   void DrawButton(ofRectangle rect, std::string label, bool enabled) const;
   void DrawControl(IUIControl* control, int column) const;

   void BeginDrag(float x, float y);
   void UpdateDrag(float x, float y);
   void EndTouch();
   float GetControlPos(IUIControl* control) const;
   void SetControlPos(IUIControl* control, float pos);
   void SetCaughtDetent(float detentPos, float detentValue);
   bool FindDetent(float rangeStart, float rangeEnd, float& detentPos, float& detentValue) const;

   IDrawableModule* mModule{ nullptr };
   std::vector<IUIControl*> mControls;
   int mPage{ 0 };

   //touch state
   IUIControl* mTouchedControl{ nullptr };
   bool mIsDragging{ false };
   ofRectangle mDragTrack;
   ofVec2f mDragStart;
   ofVec2f mLastDrag;
   float mDragOffset{ 0 };
   float mDragPrecision{ 1 };
   float mDragPos{ 0 }; //0-1 position of the dragged slider
   bool mIsCaughtOnDetent{ false };
   IUIControl* mLastTappedControl{ nullptr };
   double mLastTapTime{ -9999 };
   ofVec2f mLastTapPos;
};
