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

#include "AbletonMoveLCD.h"
#include "IDrawableModule.h"
#include "ITouchListener.h"
#include <array>

class IUIControl;
class Checkbox;
class FloatSlider;
class FloatSliderLFOControl;

class TouchInterfacePanel : public IDrawableModule, public ITouchListener
{
public:
   TouchInterfacePanel();
   ~TouchInterfacePanel();

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
   bool IsZoomed() const { return mIsZoomed; } //temporarily zoomed in on the module, to interact with it directly
   bool TestClick(float x, float y, bool right, bool testOnly = false) override;
   void DrawModuleHighlight() const; //draws on the canvas, rather than in the panel

   //ITouchListener
   bool WantsTouch(float screenX, float screenY) override;
   void TouchPressed(int touchId, float screenX, float screenY) override;
   void TouchMoved(int touchId, float screenX, float screenY) override;
   void TouchReleased(int touchId) override;

private:
   //a module that's connected to the one we're showing
   struct Routing
   {
      IDrawableModule* mModule{ nullptr };
      std::string mLabel;
      ofColor mConnectionColor;
      bool operator==(const Routing& other) const
      {
         return mModule == other.mModule && mLabel == other.mLabel && mConnectionColor.r == other.mConnectionColor.r &&
                mConnectionColor.g == other.mConnectionColor.g && mConnectionColor.b == other.mConnectionColor.b;
      }
   };

   //the list of modules along one side of the panel
   struct RoutingList
   {
      std::vector<Routing> mModules;
      int mPage{ 0 };
   };

   struct Touch
   {
      int mTouchId{ kMouseTouchId };
      IUIControl* mControl{ nullptr }; //null if this touch isn't in use
      int mOrder{ 0 }; //higher for more recent touches
      bool mIsDragging{ false };
      bool mIsTooltip{ false }; //pressing on the control's name, rather than adjusting it
      bool mIsModulatorTap{ false }; //might turn out to be a tap on a modulated control, if it doesn't move before it's released
      ofRectangle mDragTrack;
      ofVec2f mDragStart;
      ofVec2f mLastDrag;
      float mDragOffset{ 0 };
      float mDragPrecision{ 1 };
      float mDragPos{ 0 }; //0-1 position of the dragged slider
      bool mIsCaughtOnDetent{ false };
   };
   static constexpr int kMouseTouchId = -1; //for input from the usual mouse handling
   static constexpr int kMaxTouches = 11; //ten fingers, and the mouse
   static constexpr int kMaxColumns = 8;

   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   int GetNumPages() const;
   int GetNumRows() const;
   int GetNumColumns() const;
   int GetNumControlsPerPage() const { return GetNumRows() * kMaxColumns; }
   void ChangePage(int direction);
   void SetModule(IDrawableModule* module);
   void UpdateControlList();
   IDrawableModule* GetModulatorModule(IUIControl* control) const; //the module that's modulating this control, if any
   FloatSliderLFOControl* GetUnpinnedLFO() const; //null if the module isn't an LFO that only exists as a popup
   bool IsUnpinnedLFOValid() const;
   std::string GetModuleLabel(IDrawableModule* module) const;
   void UpdateRoutingModules();
   int GetNumRoutingButtons() const; //how many fit along each side
   int GetNumRoutingModulesPerPage(const RoutingList& list) const; //one button is for paging, if they don't all fit
   ofRectangle GetRoutingButtonRect(bool isInput, int index) const;
   bool PressRoutingButton(bool isInput, float x, float y);
   void DrawRoutingButtons(bool isInput) const;
   Checkbox* GetEnabledCheckbox() const; //null if the module can't be enabled/disabled
   bool IsValidControl(IUIControl* control) const;

   ofRectangle GetContentRect() const; //the area of the screen that isn't cut off by notches, rounded corners, etc
   ofRectangle GetControlsRect() const; //what's left of the content area below the header, and between the routing buttons
   ofRectangle GetExitButtonRect() const;
   ofRectangle GetZoomButtonRect() const;
   IDrawableModule* GetCanvasModule() const; //the module to look at on the canvas, null if there isn't one
   ofRectangle GetCanvasModuleRect(IDrawableModule* module) const; //including its title bar
   bool ZoomToModule();
   void DrawZoomFrame();
   void EndZoom();
   ofRectangle GetPageButtonRect(int direction) const;
   ofRectangle GetSlotRect(int slot) const; //where the control at this index on the current page goes
   ofRectangle GetTrackRect(int slot) const;
   ofRectangle GetLabelRect(int slot) const;
   ofRectangle GetSquareRect(int slot) const; //for buttons and checkboxes
   float GetTitleBarHeight() const;
   void DrawButton(ofRectangle rect, std::string label, bool enabled) const;
   void DrawTitle();
   void DrawControl(IUIControl* control, int slot) const;
   void DrawControlVisualization();
   void DrawTooltip() const;

   ofVec2f ScreenToLocal(float screenX, float screenY) const;
   void Press(int touchId, float x, float y);
   void Move(int touchId, float x, float y);
   Touch* FindTouch(int touchId);
   Touch* FindFreeTouch();
   const Touch* FindTouch(const IUIControl* control) const;
   void AssignTouch(Touch& touch, int touchId, IUIControl* control);
   void BeginDrag(Touch& touch, float x, float y);
   void UpdateDrag(Touch& touch, float x, float y);
   void EndTouch(Touch& touch);
   void EndAllTouches();
   float GetControlPos(IUIControl* control) const;
   void SetControlPos(IUIControl* control, float pos);
   void SetCaughtDetent(Touch& touch, float detentPos, float detentValue);
   bool FindDetent(IUIControl* control, float rangeStart, float rangeEnd, float& detentPos, float& detentValue) const;

   IDrawableModule* mModule{ nullptr };
   FloatSlider* mLFOTarget{ nullptr }; //if the module is an unpinned LFO, the slider that it's modulating
   IDrawableModule* mLFOTargetParent{ nullptr };
   IDrawableModule* mLFOTargetModule{ nullptr };
   std::vector<IUIControl*> mControls;
   int mPage{ 0 };
   RoutingList mRoutingInputs; //previous modules in the signal chain
   RoutingList mRoutingOutputs; //next modules in the signal chain
   double mLastRoutingUpdateTime{ 0 };
   ofRectangle mTitleRect; //where the module's name was last drawn, tappable to enable/disable the module

   //fixed size, since it's accessed while drawing
   std::array<Touch, kMaxTouches> mTouches;
   int mTouchCounter{ 0 };
   bool mIsZoomed{ false };
   ofVec2f mZoomScreenSize; //the screen size that we zoomed to fit
   AbletonMoveLCD mVisualizationLCD; //control visualizers draw to this
   IUIControl* mLastTappedControl{ nullptr };
   double mLastTapTime{ -9999 };
   ofVec2f mLastTapPos;
};
