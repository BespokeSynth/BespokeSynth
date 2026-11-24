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

    TouchInterfacePanel.cpp
    Created: 23 Nov 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "TouchInterfacePanel.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "ControlSurface.h"
#include "IModulator.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "QuickSpawnMenu.h"
#include "Slider.h"
#include "SynthGlobals.h"
#include "TitleBar.h"

namespace
{
   const int kMaxColumns = 8;
   const float kMargin = 8;
   const float kHeaderHeight = 44;
   const float kPageLabelHeight = 20;
   const float kLabelHeight = 42;
   const float kMaxColumnWidth = 120;
   const float kColumnSpacing = 6;
   const float kDragDeadZone = 20; //fingers don't move in straight lines, so allow some sideways drift before increasing precision
   const float kDetentCatchDistance = 14; //how far to drag to get out of a detent
   const float kSideButtonWidth = 72;
   const float kSideButtonHeight = 44;
   const float kSideButtonSpacing = 4;
   const double kRoutingUpdateMs = 250;
   const double kDoubleTapMs = 300;
   const float kDoubleTapDistance = 15;

   //shrinks text to fit, and then truncates it if it would get too small to read
   void FitText(std::string& text, float& size, float maxWidth)
   {
      const float kMinSize = 9;
      while (size > kMinSize && GetStringWidth(text, size) > maxWidth)
         size -= .5f;
      while (text.length() > 1 && GetStringWidth(text, size) > maxWidth)
         text.pop_back();
   }

   void DrawTextToFit(std::string text, float centerX, float baselineY, float maxWidth, float size)
   {
      FitText(text, size, maxWidth);
      DrawTextNormal(text, centerX - GetStringWidth(text, size) / 2, baselineY, size);
   }

   std::string GetControlLabel(IUIControl* control)
   {
      std::string label = control->GetDisplayName();
      if (label.empty())
         label = control->Name();
      return label;
   }

   //if the control has a modulator with an adjustable range, then that's what we adjust, like dragging a regular slider does
   IModulator* GetAdjustableModulator(IUIControl* control)
   {
      if (dynamic_cast<FloatSlider*>(control) == nullptr)
         return nullptr;
      IModulator* modulator = control->GetModulator();
      if (modulator != nullptr && modulator->Active() && modulator->CanAdjustRange())
         return modulator;
      return nullptr;
   }
}

TouchInterfacePanel::TouchInterfacePanel()
{
   SetShouldDrawOutline(false);
   SetShowing(false);
   TheSynth->AddTouchListener(this);
}

TouchInterfacePanel::~TouchInterfacePanel()
{
   if (TheSynth != nullptr)
      TheSynth->RemoveTouchListener(this);
}

bool TouchInterfacePanel::ShowForModule(IDrawableModule* module)
{
   if (ControlSurface::GetDisplayableControls(module).empty())
      return false;

   SetModule(module);

   gHoveredUIControl = nullptr; //don't let clicks get directed to a control that we're covering up
   GetOwningContainer()->MoveToFront(this);
   SetPosition(0, GetTitleBarHeight());
   SetShowing(true);
   return true;
}

void TouchInterfacePanel::SetModule(IDrawableModule* module)
{
   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
   EndAllTouches(); //while the controls being touched are still in our list
   mModule = module;
   mControls = ControlSurface::GetDisplayableControls(module);
   mPage = 0;
   mLastTappedControl = nullptr;
   mRoutingInputs = RoutingList();
   mRoutingOutputs = RoutingList();
   UpdateRoutingModules();
}

void TouchInterfacePanel::Hide()
{
   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
   EndAllTouches();
   SetShowing(false);
   mModule = nullptr;
   mControls.clear();
   mRoutingInputs = RoutingList();
   mRoutingOutputs = RoutingList();
   mLastTappedControl = nullptr;
}

void TouchInterfacePanel::Poll()
{
   if (!IsShowing())
      return;

   //the title bar is still usable while we're showing, so get out of the way if it was used to do something else
   if (!TheSynth->IsInterfaceMode() || TheQuickSpawnMenu->IsShowing())
   {
      Hide();
      return;
   }

   SetPosition(0, GetTitleBarHeight());
   UpdateControlList();
   if (gTime - mLastRoutingUpdateTime > kRoutingUpdateMs)
      UpdateRoutingModules();
}

void TouchInterfacePanel::UpdateControlList()
{
   if (mModule == nullptr)
      return;

   if (mModule->IsDeleted())
   {
      mControls.clear(); //they might not exist anymore
      Hide();
      return;
   }

   std::vector<IUIControl*> controls = ControlSurface::GetDisplayableControls(mModule);
   if (controls != mControls)
   {
      std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
      mControls = controls;
      mPage = ofClamp(mPage, 0, GetNumPages() - 1);
      for (auto& touch : mTouches)
      {
         if (!IsValidControl(touch.mControl))
            touch = Touch();
      }
      if (!IsValidControl(mLastTappedControl))
         mLastTappedControl = nullptr;
   }
}

void TouchInterfacePanel::UpdateRoutingModules()
{
   mLastRoutingUpdateTime = gTime;

   std::vector<Routing> inputs;
   std::vector<Routing> outputs;

   auto add = [this](std::vector<Routing>& list, IDrawableModule* module, ofColor color)
   {
      if (module == nullptr || module == mModule || module->IsDeleted())
         return;
      for (const auto& routing : list)
      {
         if (routing.mModule == module)
            return;
      }
      list.push_back({ module, module->GetTitleLabel(), color });
   };

   if (mModule != nullptr)
   {
      std::vector<IDrawableModule*> modules;
      TheSynth->GetAllModules(modules);
      for (auto* module : modules)
      {
         for (auto* source : module->GetPatchCableSources())
         {
            for (auto* cable : source->GetPatchCables())
            {
               if (cable->GetTarget() == mModule)
                  add(inputs, module, source->GetColor());
            }
         }
      }

      for (auto* source : mModule->GetPatchCableSources())
      {
         for (auto* cable : source->GetPatchCables())
            add(outputs, dynamic_cast<IDrawableModule*>(cable->GetTarget()), source->GetColor());
      }
   }

   auto update = [this](RoutingList& list, std::vector<Routing>& modules)
   {
      if (list.mModules == modules)
         return;
      std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
      list.mModules = modules;
      if (list.mPage * GetNumRoutingModulesPerPage(list) >= (int)modules.size())
         list.mPage = 0;
   };
   update(mRoutingInputs, inputs);
   update(mRoutingOutputs, outputs);
}

int TouchInterfacePanel::GetNumRoutingButtons() const
{
   float height = GetContentRect().height - kHeaderHeight;
   if (GetNumPages() > 1)
      height -= kSideButtonHeight + kSideButtonSpacing; //leave room for the page buttons
   return MAX(2, (int)((height + kSideButtonSpacing) / (kSideButtonHeight + kSideButtonSpacing)));
}

int TouchInterfacePanel::GetNumRoutingModulesPerPage(const RoutingList& list) const
{
   const int numButtons = GetNumRoutingButtons();
   return (int)list.mModules.size() <= numButtons ? numButtons : numButtons - 1;
}

ofRectangle TouchInterfacePanel::GetRoutingButtonRect(bool isInput, int index) const
{
   ofRectangle content = GetContentRect();
   return ofRectangle(isInput ? content.x : content.getMaxX() - kSideButtonWidth, content.y + kHeaderHeight + (kSideButtonHeight + kSideButtonSpacing) * index, kSideButtonWidth, kSideButtonHeight);
}

bool TouchInterfacePanel::PressRoutingButton(bool isInput, float x, float y)
{
   RoutingList& list = isInput ? mRoutingInputs : mRoutingOutputs;
   const int perPage = GetNumRoutingModulesPerPage(list);
   const int numModules = (int)list.mModules.size();

   for (int i = 0; i < perPage; ++i)
   {
      int index = list.mPage * perPage + i;
      if (index < numModules && GetRoutingButtonRect(isInput, i).contains(x, y))
      {
         SetModule(list.mModules[index].mModule);
         return true;
      }
   }

   if (numModules > perPage && GetRoutingButtonRect(isInput, perPage).contains(x, y))
   {
      ++list.mPage;
      if (list.mPage * perPage >= numModules)
         list.mPage = 0;
      return true;
   }

   return false;
}

Checkbox* TouchInterfacePanel::GetEnabledCheckbox() const
{
   return mModule != nullptr ? mModule->GetEnabledCheckbox() : nullptr;
}

bool TouchInterfacePanel::IsValidControl(IUIControl* control) const
{
   return control != nullptr && VectorContains(control, mControls);
}

int TouchInterfacePanel::GetNumPages() const
{
   return MAX(1, ((int)mControls.size() + kMaxColumns - 1) / kMaxColumns);
}

int TouchInterfacePanel::GetNumColumns() const
{
   return ofClamp((int)mControls.size(), 1, kMaxColumns);
}

void TouchInterfacePanel::ChangePage(int direction)
{
   mPage = ofClamp(mPage + direction, 0, GetNumPages() - 1);
   mLastTappedControl = nullptr;
}

float TouchInterfacePanel::GetTitleBarHeight() const
{
   return TheTitleBar != nullptr ? TheTitleBar->GetRect().height : 0;
}

void TouchInterfacePanel::GetModuleDimensions(float& width, float& height)
{
   float scale = GetOwningContainer()->GetDrawScale();
   width = ofGetWidth() / scale;
   height = ofGetHeight() / scale - GetTitleBarHeight();
}

ofRectangle TouchInterfacePanel::GetContentRect() const
{
   float scale = GetOwningContainer()->GetDrawScale();
   float left = 0, right = 0, top = 0, bottom = 0;
#if BESPOKE_MOBILE
   if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
   {
      left = display->safeAreaInsets.getLeft() / scale;
      right = display->safeAreaInsets.getRight() / scale;
      top = display->safeAreaInsets.getTop() / scale;
      bottom = display->safeAreaInsets.getBottom() / scale;
   }
#endif
   const float titleBarHeight = GetTitleBarHeight();
   top = MAX(0, top - titleBarHeight); //we're positioned below the title bar
   const float height = ofGetHeight() / scale - titleBarHeight;
   return ofRectangle(left + kMargin, top + kMargin, ofGetWidth() / scale - left - right - kMargin * 2, height - top - bottom - kMargin * 2);
}

ofRectangle TouchInterfacePanel::GetControlsRect() const
{
   ofRectangle rect = GetContentRect();
   rect.y += kHeaderHeight;
   rect.height -= kHeaderHeight;
   const bool hasPages = GetNumPages() > 1;
   if (hasPages)
      rect.height -= kPageLabelHeight;
   if (hasPages || !mRoutingInputs.mModules.empty())
   {
      rect.x += kSideButtonWidth + kMargin;
      rect.width -= kSideButtonWidth + kMargin;
   }
   if (hasPages || !mRoutingOutputs.mModules.empty())
      rect.width -= kSideButtonWidth + kMargin;
   return rect;
}

ofRectangle TouchInterfacePanel::GetExitButtonRect() const
{
   ofRectangle content = GetContentRect();
   const float kWidth = 64;
   return ofRectangle(content.getMaxX() - kWidth, content.y, kWidth, kHeaderHeight - 8);
}

ofRectangle TouchInterfacePanel::GetPageButtonRect(int direction) const
{
   ofRectangle content = GetContentRect();
   return ofRectangle(direction < 0 ? content.x : content.getMaxX() - kSideButtonWidth, content.getMaxY() - kSideButtonHeight, kSideButtonWidth, kSideButtonHeight);
}

ofRectangle TouchInterfacePanel::GetColumnRect(int column) const
{
   ofRectangle area = GetControlsRect();
   int numColumns = GetNumColumns();
   float columnWidth = MIN(area.width / numColumns, kMaxColumnWidth);
   float startX = area.x + (area.width - columnWidth * numColumns) / 2;
   return ofRectangle(startX + columnWidth * column, area.y, columnWidth, area.height);
}

ofRectangle TouchInterfacePanel::GetTrackRect(int column) const
{
   ofRectangle rect = GetColumnRect(column);
   return ofRectangle(rect.x + kColumnSpacing / 2, rect.y, rect.width - kColumnSpacing, MAX(1, rect.height - kLabelHeight));
}

ofRectangle TouchInterfacePanel::GetSquareRect(int column) const
{
   ofRectangle track = GetTrackRect(column);
   float size = MIN(track.width, track.height);
   return ofRectangle(track.getCenter().x - size / 2, track.getCenter().y - size / 2, size, size);
}

float TouchInterfacePanel::GetControlPos(IUIControl* control) const
{
   if (IModulator* modulator = GetAdjustableModulator(control))
      return static_cast<FloatSlider*>(control)->ValToPos(modulator->GetMax(), K(ignoreSmooth));
   return ofClamp(control->GetMidiValue(), 0, 1);
}

void TouchInterfacePanel::SetControlPos(IUIControl* control, float pos)
{
   if (IModulator* modulator = GetAdjustableModulator(control))
      modulator->GetMax() = static_cast<FloatSlider*>(control)->PosToVal(pos, K(ignoreSmooth));
   else
      control->SetFromMidiCC(pos, NextBufferTime(false), SetValueMethod::Direct);
}

//finds the detent closest to rangeStart that's between rangeStart (exclusive) and rangeEnd (inclusive)
bool TouchInterfacePanel::FindDetent(IUIControl* control, float rangeStart, float rangeEnd, float& detentPos, float& detentValue) const
{
   FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
   if (slider == nullptr)
      return false;

   bool found = false;
   for (float detent : slider->GetDetents())
   {
      if (detent <= slider->GetMin() || detent >= slider->GetMax())
         continue;
      float pos = slider->ValToPos(detent, K(ignoreSmooth));
      bool inRange = (pos > rangeStart && pos <= rangeEnd) || (pos < rangeStart && pos >= rangeEnd);
      if (inRange && (!found || fabsf(pos - rangeStart) < fabsf(detentPos - rangeStart)))
      {
         found = true;
         detentPos = pos;
         detentValue = detent;
      }
   }
   return found;
}

void TouchInterfacePanel::SetCaughtDetent(Touch& touch, float detentPos, float detentValue)
{
   touch.mIsCaughtOnDetent = true;
   touch.mDragPos = detentPos;

   //set the detent's value directly, so that it's exact
   if (IModulator* modulator = GetAdjustableModulator(touch.mControl))
      modulator->GetMax() = detentValue;
   else
      touch.mControl->SetValue(detentValue, NextBufferTime(false));
}

void TouchInterfacePanel::BeginDrag(Touch& touch, float x, float y)
{
   if (FloatSlider* slider = dynamic_cast<FloatSlider*>(touch.mControl))
   {
      slider->mTouching = true;
      slider->UpdateTouching();
   }

   touch.mIsDragging = true;
   touch.mDragStart.set(x, y);
   touch.mLastDrag.set(x, y);
   touch.mDragOffset = 0;
   touch.mDragPrecision = 1;
   touch.mIsCaughtOnDetent = false;
   touch.mDragPos = ofClamp((touch.mDragTrack.getMaxY() - y) / touch.mDragTrack.height, 0, 1);

   //catch on a detent if we touched down right next to one
   float catchDistance = kDetentCatchDistance / 2 / touch.mDragTrack.height;
   float detentPos, detentValue;
   if (FindDetent(touch.mControl, touch.mDragPos - catchDistance, touch.mDragPos + catchDistance, detentPos, detentValue))
      SetCaughtDetent(touch, detentPos, detentValue);
   else
      SetControlPos(touch.mControl, touch.mDragPos);
}

void TouchInterfacePanel::UpdateDrag(Touch& touch, float x, float y)
{
   if (x == touch.mLastDrag.x && y == touch.mLastDrag.y)
      return;

   auto getSidewaysDistance = [&touch](float dragX)
   {
      return MAX(0, fabsf(dragX - touch.mDragStart.x) - kDragDeadZone);
   };

   float distance = getSidewaysDistance(x);
   float cursorPos = (touch.mDragTrack.getMaxY() - y) / touch.mDragTrack.height;
   float lastCursorPos = (touch.mDragTrack.getMaxY() - touch.mLastDrag.y) / touch.mDragTrack.height;
   float pos = PrecisionDrag::Apply(touch.mDragOffset, cursorPos, lastCursorPos, distance, getSidewaysDistance(touch.mLastDrag.x), 1, true);
   touch.mDragPrecision = PrecisionDrag::Damp(distance);
   touch.mLastDrag.set(x, y);

   const float catchDistance = kDetentCatchDistance / touch.mDragTrack.height * touch.mDragPrecision;

   if (touch.mIsCaughtOnDetent)
   {
      float move = pos - touch.mDragPos;
      if (fabsf(move) <= catchDistance)
         return;

      //pull free of the detent, and absorb the distance that took, so that the value doesn't jump
      touch.mIsCaughtOnDetent = false;
      float absorb = move > 0 ? catchDistance : -catchDistance;
      touch.mDragOffset -= absorb;
      pos -= absorb;
   }

   float detentPos, detentValue;
   if (FindDetent(touch.mControl, touch.mDragPos, pos, detentPos, detentValue) &&
       fabsf(pos - detentPos) <= catchDistance) //let fast movements blow right past it
   {
      SetCaughtDetent(touch, detentPos, detentValue);
      return;
   }

   touch.mDragPos = pos;
   SetControlPos(touch.mControl, touch.mDragPos);
}

TouchInterfacePanel::Touch* TouchInterfacePanel::FindTouch(int touchId)
{
   for (auto& touch : mTouches)
   {
      if (touch.mControl != nullptr && touch.mTouchId == touchId)
         return &touch;
   }
   return nullptr;
}

TouchInterfacePanel::Touch* TouchInterfacePanel::FindFreeTouch()
{
   for (auto& touch : mTouches)
   {
      if (touch.mControl == nullptr)
         return &touch;
   }
   return nullptr;
}

const TouchInterfacePanel::Touch* TouchInterfacePanel::FindTouch(const IUIControl* control) const
{
   for (const auto& touch : mTouches)
   {
      if (touch.mControl != nullptr && touch.mControl == control)
         return &touch;
   }
   return nullptr;
}

void TouchInterfacePanel::EndAllTouches()
{
   for (auto& touch : mTouches)
      EndTouch(touch);
}

void TouchInterfacePanel::EndTouch(Touch& touch)
{
   if (IsValidControl(touch.mControl))
   {
      if (touch.mIsDragging)
      {
         FloatSlider* slider = dynamic_cast<FloatSlider*>(touch.mControl);
         if (slider != nullptr)
         {
            slider->mTouching = false;
            slider->UpdateTouching(); //"relative" sliders spring back when released
         }
      }
      else if (dynamic_cast<ClickButton*>(touch.mControl) != nullptr)
      {
         touch.mControl->MouseReleased();
      }
   }
   touch = Touch();
}

void TouchInterfacePanel::OnClicked(float x, float y, bool right)
{
   if (!right)
      Press(kMouseTouchId, x, y);
}

bool TouchInterfacePanel::WantsTouch(float screenX, float screenY)
{
   if (!IsShowing() || TheQuickSpawnMenu->IsShowing())
      return false;

   ModuleContainer* container = GetOwningContainer();
   return container->GetModuleAt(TheSynth->GetMouseX(container, screenX), TheSynth->GetMouseY(container, screenY)) == this; //could be covered, e.g. by the title bar
}

ofVec2f TouchInterfacePanel::ScreenToLocal(float screenX, float screenY) const
{
   ModuleContainer* container = GetOwningContainer();
   return ofVec2f(TheSynth->GetMouseX(container, screenX), TheSynth->GetMouseY(container, screenY)) - GetPosition();
}

void TouchInterfacePanel::TouchPressed(int touchId, float screenX, float screenY)
{
   ofVec2f pos = ScreenToLocal(screenX, screenY);
   Press(touchId, pos.x, pos.y);
}

void TouchInterfacePanel::TouchMoved(int touchId, float screenX, float screenY)
{
   ofVec2f pos = ScreenToLocal(screenX, screenY);
   Move(touchId, pos.x, pos.y);
}

void TouchInterfacePanel::Press(int touchId, float x, float y)
{
   if (!IsShowing())
      return;

   TouchReleased(touchId); //in case we missed the release

   if (GetExitButtonRect().contains(x, y))
   {
      Hide();
      return;
   }

   if (Checkbox* enabledCheckbox = GetEnabledCheckbox())
   {
      if (mTitleRect.contains(x, y))
      {
         enabledCheckbox->SetFromMidiCC(enabledCheckbox->GetValue() > 0 ? 0 : 1, NextBufferTime(false), SetValueMethod::Direct);
         return;
      }
   }

   if (GetNumPages() > 1)
   {
      for (int direction : { -1, 1 })
      {
         if (GetPageButtonRect(direction).contains(x, y))
         {
            ChangePage(direction);
            return;
         }
      }
   }

   UpdateRoutingModules(); //make sure that they all still exist
   if (PressRoutingButton(true, x, y) || PressRoutingButton(false, x, y))
      return;

   UpdateControlList();

   for (int column = 0; column < kMaxColumns; ++column)
   {
      int index = mPage * kMaxColumns + column;
      if (index >= (int)mControls.size() || !GetColumnRect(column).contains(x, y))
         continue;

      IUIControl* control = mControls[index];
      if (control->IsButtonControl() && !GetSquareRect(column).contains(x, y))
         return;

      Touch* touch = FindFreeTouch();
      if (touch == nullptr || FindTouch(control) != nullptr) //another touch already has this control
         return;

      if (control->IsButtonControl())
      {
         touch->mTouchId = touchId;
         touch->mControl = control;
         if (dynamic_cast<Checkbox*>(control) != nullptr)
            control->SetFromMidiCC(control->GetValue() > 0 ? 0 : 1, NextBufferTime(false), SetValueMethod::Direct);
         else
            control->SetFromMidiCC(1, NextBufferTime(false), SetValueMethod::Direct);
         return;
      }

      bool isDoubleTap = control == mLastTappedControl && gTime - mLastTapTime < kDoubleTapMs &&
                         mLastTapPos.distanceSquared(ofVec2f(x, y)) < kDoubleTapDistance * kDoubleTapDistance;
      mLastTappedControl = isDoubleTap ? nullptr : control;
      mLastTapTime = gTime;
      mLastTapPos.set(x, y);
      if (isDoubleTap)
      {
         control->ResetToDefault();
         return;
      }

      touch->mTouchId = touchId;
      touch->mControl = control;
      touch->mDragTrack = GetTrackRect(column);
      BeginDrag(*touch, x, y);
      return;
   }
}

void TouchInterfacePanel::Move(int touchId, float x, float y)
{
   if (FindTouch(touchId) == nullptr)
      return;

   UpdateControlList(); //another touch might have changed which controls exist

   Touch* touch = FindTouch(touchId);
   if (touch != nullptr && touch->mIsDragging)
      UpdateDrag(*touch, x, y);
}

void TouchInterfacePanel::TouchReleased(int touchId)
{
   if (FindTouch(touchId) == nullptr)
      return;

   UpdateControlList();

   if (Touch* touch = FindTouch(touchId))
      EndTouch(*touch);
}

bool TouchInterfacePanel::MouseMoved(float x, float y)
{
   Move(kMouseTouchId, x, y);
   return false;
}

void TouchInterfacePanel::MouseReleased()
{
   TouchReleased(kMouseTouchId);
}

void TouchInterfacePanel::DrawButton(ofRectangle rect, std::string label, bool enabled) const
{
   ofPushStyle();
   ofFill();
   ofSetColor(70, 70, 70, enabled ? 255 : 90);
   ofRect(rect, 6);
   ofSetColor(255, 255, 255, enabled ? 255 : 90);
   DrawTextToFit(label, rect.getCenter().x, rect.getCenter().y + 6, rect.width - 14, 18);
   ofPopStyle();
}

void TouchInterfacePanel::DrawControl(IUIControl* control, int column) const
{
   const ofRectangle track = GetTrackRect(column);
   const Touch* touch = FindTouch(control);
   const bool isTouched = touch != nullptr;
   ofColor color = IDrawableModule::GetColor(mModule->GetModuleCategory());

   ofPushStyle();
   ofFill();

   if (control->IsButtonControl())
   {
      const ofRectangle square = GetSquareRect(column);
      if (dynamic_cast<Checkbox*>(control) != nullptr)
      {
         ofSetColor(110, 110, 110, 150);
         ofNoFill();
         ofRect(square, 6);
         ofFill();
         if (control->GetValue() > 0)
         {
            const float inset = square.width * .2f;
            ofSetColor(255, 255, 255);
            ofFill();
            ofRect(square.x + inset, square.y + inset, square.width - inset * 2, square.height - inset * 2, 4);
         }
      }
      else
      {
         ofSetColor(isTouched ? color * .9f : color * .55f, isTouched ? 255 : 180);
         ofRect(square, 6);
      }
   }
   else
   {
      const bool isDragging = isTouched && touch->mIsDragging;
      const float rawPos = isDragging ? touch->mDragPos : GetControlPos(control);
      const int numValues = control->GetNumValues(); //0 for continuous controls
      const float pos = numValues > 1 ? roundf(rawPos * (numValues - 1)) / (numValues - 1) : rawPos;
      auto posToY = [&track](float p)
      {
         return track.getMaxY() - ofClamp(p, 0, 1) * track.height;
      };

      ofSetColor(color * .3f, 100);
      ofRect(track, 6);

      const float y = posToY(pos);
      ofSetColor(color * .55f, 150);
      ofRect(track.x, y, track.width, track.getMaxY() - y, 6);

      if (numValues > 1 && track.height / (numValues - 1) > 3) //hash marks
      {
         ofSetColor(130, 130, 130);
         ofSetLineWidth(1);
         for (int i = 1; i < numValues - 1; ++i)
         {
            float hashY = posToY(i / float(numValues - 1));
            ofLine(track.x + 1, hashY, track.getMaxX() - 1, hashY);
         }
      }

      FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
      if (slider != nullptr)
      {
         ofSetColor(150, 150, 150, 255);
         ofSetLineWidth(1);
         for (float detent : slider->GetDetents())
         {
            if (detent > slider->GetMin() && detent < slider->GetMax())
            {
               float detentY = posToY(slider->ValToPos(detent, K(ignoreSmooth)));
               ofLine(track.x, detentY, track.getMaxX(), detentY);
            }
         }
      }

      if (IModulator* modulator = GetAdjustableModulator(control))
      {
         //modulation range, and the current value within it
         float minY = posToY(slider->ValToPos(modulator->GetMin(), K(ignoreSmooth)));
         float valueY = posToY(control->GetMidiValue());
         ofSetColor(0, 200, 0, 130);
         ofRect(track.x, MIN(minY, valueY), track.width, fabsf(minY - valueY), 1);
         ofSetColor(0, 255, 0);
         ofSetLineWidth(2);
         ofLine(track.x, minY, track.getMaxX(), minY);
         ofLine(track.x, y, track.getMaxX(), y);
      }
      else
      {
         //thinner line when dragging with more precision
         ofSetLineWidth(isDragging ? MAX(4 * sqrtf(touch->mDragPrecision), .5f) : 4);
         if (isDragging && touch->mIsCaughtOnDetent)
            ofSetColor(255, 255, 0);
         else
            ofSetColor(255, 0, 0);
         ofLine(track.x, y, track.getMaxX(), y);

         if (isDragging && numValues > 1) //where we are between the discrete values
         {
            ofSetColor(255, 100, 0);
            ofCircle(track.getCenter().x, posToY(rawPos), 4);
         }
      }
   }

   //backing, to keep the labels readable over the canvas
   ofSetColor(0, 0, 0, 150);
   ofRect(track.x, track.getMaxY() + 3, track.width, kLabelHeight - 3, 4);

   ofSetColor(255, 255, 255);
   DrawTextToFit(GetControlLabel(control), track.getCenter().x, track.getMaxY() + 18, track.width, 16);
   if (dynamic_cast<ClickButton*>(control) == nullptr)
      DrawTextToFit(control->GetDisplayValue(control->GetValue()), track.getCenter().x, track.getMaxY() + 36, track.width, 14);

   ofPopStyle();
}

void TouchInterfacePanel::DrawTitle()
{
   const ofRectangle content = GetContentRect();
   const float left = content.x;
   const float maxWidth = GetExitButtonRect().x - kMargin - left;
   const float centerX = left + maxWidth / 2;
   const float centerY = GetExitButtonRect().getCenter().y;
   float size = 22;

   ofPushStyle();

   Checkbox* enabledCheckbox = GetEnabledCheckbox();
   const float kCircleRadius = 9;
   const float kCircleSpace = enabledCheckbox != nullptr ? kCircleRadius * 2 + 8 : 0;
   const float kPadding = 10;

   std::string title = mModule->GetTitleLabel();
   FitText(title, size, maxWidth - kCircleSpace - kPadding * 2);
   const float width = GetStringWidth(title, size) + kCircleSpace;
   float x = centerX - width / 2;
   mTitleRect = ofRectangle(x - kPadding, content.y, width + kPadding * 2, kHeaderHeight - 8);

   bool enabled = true;
   ofColor color = IDrawableModule::GetColor(mModule->GetModuleCategory());
   if (enabledCheckbox != nullptr)
   {
      enabled = enabledCheckbox->GetValue() > 0;

      ofFill();
      ofSetColor(70, 70, 70, 180);
      ofRect(mTitleRect, 6);

      //like the enabled indicator in a module's title bar
      ofSetColor(color * .3f, 255);
      ofCircle(x + kCircleRadius, centerY, kCircleRadius);
      if (enabled)
      {
         ofSetColor(color, 255);
         ofCircle(x + kCircleRadius, centerY, kCircleRadius * .6f);
      }
      x += kCircleSpace;
   }

   ofSetColor(255, 255, 255, enabled ? 255 : 110);
   DrawTextNormal(title, x, centerY + 7, size);

   ofPopStyle();
}

void TouchInterfacePanel::DrawRoutingButtons(bool isInput) const
{
   const RoutingList& list = isInput ? mRoutingInputs : mRoutingOutputs;
   const int perPage = GetNumRoutingModulesPerPage(list);
   const int numModules = (int)list.mModules.size();

   ofPushStyle();
   ofFill();
   for (int i = 0; i < perPage; ++i)
   {
      const int index = list.mPage * perPage + i;
      if (index >= numModules)
         break;

      const Routing& routing = list.mModules[index];
      const ofRectangle rect = GetRoutingButtonRect(isInput, i);
      DrawButton(rect, routing.mLabel, true);

      //strip along the outside edge, in the color of the cable
      const float kStripWidth = 5;
      ofSetColor(routing.mConnectionColor);
      ofRect(isInput ? rect.x : rect.getMaxX() - kStripWidth, rect.y, kStripWidth, rect.height, 2);
   }

   if (numModules > perPage)
   {
      const int numPages = (numModules + perPage - 1) / perPage;
      DrawButton(GetRoutingButtonRect(isInput, perPage), ofToString(list.mPage + 1) + "/" + ofToString(numPages) + " >", true);
   }
   ofPopStyle();
}

//skips the usual module frame, since its background would make us too opaque
void TouchInterfacePanel::Render()
{
   if (!IsShowing())
      return;

   ofPushMatrix();
   ofTranslate(mX, mY);
   DrawModule();
   ofPopMatrix();
}

void TouchInterfacePanel::DrawModule()
{
   if (mModule == nullptr)
      return;

   float w, h;
   GetModuleDimensions(w, h);

   ofPushStyle();
   ofFill();
   ofSetColor(20, 20, 20, 153);
   ofRect(0, 0, w, h, 0);

   const ofRectangle content = GetContentRect();

   DrawTitle();

   DrawButton(GetExitButtonRect(), "exit", true);

   DrawRoutingButtons(true);
   DrawRoutingButtons(false);

   if (mControls.empty()) //we can get here through the signal chain
   {
      ofRectangle area = GetControlsRect();
      ofSetColor(255, 255, 255, 130);
      DrawTextToFit("no controls", area.getCenter().x, area.getCenter().y + 6, area.width, 18);
   }

   for (int column = 0; column < kMaxColumns; ++column)
   {
      int index = mPage * kMaxColumns + column;
      if (index < (int)mControls.size())
         DrawControl(mControls[index], column);
   }

   const int numPages = GetNumPages();
   if (numPages > 1)
   {
      DrawButton(GetPageButtonRect(-1), "<", mPage > 0);
      DrawButton(GetPageButtonRect(1), ">", mPage < numPages - 1);
      ofSetColor(255, 255, 255);
      DrawTextToFit(ofToString(mPage + 1) + "/" + ofToString(numPages), w / 2, content.getMaxY() - 2, 80, 18);
   }

   ofPopStyle();
}
