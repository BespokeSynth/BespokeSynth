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
#include "QuickSpawnMenu.h"
#include "Slider.h"
#include "SynthGlobals.h"
#include "TitleBar.h"

namespace
{
   const int kMaxColumns = 8;
   const float kMargin = 8;
   const float kHeaderHeight = 44;
   const float kPageButtonWidth = 64;
   const float kPageLabelWidth = 50;
   const float kLabelHeight = 42;
   const float kMaxColumnWidth = 120;
   const float kColumnSpacing = 6;
   const float kDragDeadZone = 20; //fingers don't move in straight lines, so allow some sideways drift before increasing precision
   const float kDetentCatchDistance = 14; //how far to drag to get out of a detent
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
}

bool TouchInterfacePanel::ShowForModule(IDrawableModule* module)
{
   std::vector<IUIControl*> controls = ControlSurface::GetDisplayableControls(module);
   if (controls.empty())
      return false;

   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
   mModule = module;
   mControls = controls;
   mPage = 0;
   mLastTappedControl = nullptr;
   EndTouch();

   gHoveredUIControl = nullptr; //don't let clicks get directed to a control that we're covering up
   GetOwningContainer()->MoveToFront(this);
   SetPosition(0, GetTitleBarHeight());
   SetShowing(true);
   return true;
}

void TouchInterfacePanel::Hide()
{
   std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
   EndTouch();
   SetShowing(false);
   mModule = nullptr;
   mControls.clear();
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
}

void TouchInterfacePanel::UpdateControlList()
{
   if (mModule == nullptr)
      return;

   if (mModule->IsDeleted())
   {
      mTouchedControl = nullptr; //it might not exist anymore
      Hide();
      return;
   }

   std::vector<IUIControl*> controls = ControlSurface::GetDisplayableControls(mModule);
   if (controls != mControls)
   {
      std::lock_guard<std::recursive_mutex> renderLock(TheSynth->GetRenderLock());
      mControls = controls;
      mPage = ofClamp(mPage, 0, GetNumPages() - 1);
      if (!IsValidControl(mTouchedControl))
      {
         mTouchedControl = nullptr;
         mIsDragging = false;
      }
      if (!IsValidControl(mLastTappedControl))
         mLastTappedControl = nullptr;
   }
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

ofRectangle TouchInterfacePanel::GetExitButtonRect() const
{
   ofRectangle content = GetContentRect();
   const float kWidth = 64;
   return ofRectangle(content.getMaxX() - kWidth, content.y, kWidth, kHeaderHeight - 8);
}

ofRectangle TouchInterfacePanel::GetPageButtonRect(int direction) const
{
   ofRectangle content = GetContentRect();
   return ofRectangle(direction < 0 ? content.x : content.x + kPageButtonWidth + kPageLabelWidth, content.y, kPageButtonWidth, kHeaderHeight - 8);
}

ofRectangle TouchInterfacePanel::GetColumnRect(int column) const
{
   ofRectangle content = GetContentRect();
   int numColumns = GetNumColumns();
   float columnWidth = MIN(content.width / numColumns, kMaxColumnWidth);
   float startX = content.x + (content.width - columnWidth * numColumns) / 2;
   float top = content.y + kHeaderHeight;
   return ofRectangle(startX + columnWidth * column, top, columnWidth, content.getMaxY() - top);
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
bool TouchInterfacePanel::FindDetent(float rangeStart, float rangeEnd, float& detentPos, float& detentValue) const
{
   FloatSlider* slider = dynamic_cast<FloatSlider*>(mTouchedControl);
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

void TouchInterfacePanel::SetCaughtDetent(float detentPos, float detentValue)
{
   mIsCaughtOnDetent = true;
   mDragPos = detentPos;

   //set the detent's value directly, so that it's exact
   if (IModulator* modulator = GetAdjustableModulator(mTouchedControl))
      modulator->GetMax() = detentValue;
   else
      mTouchedControl->SetValue(detentValue, NextBufferTime(false));
}

void TouchInterfacePanel::BeginDrag(float x, float y)
{
   if (FloatSlider* slider = dynamic_cast<FloatSlider*>(mTouchedControl))
   {
      slider->mTouching = true;
      slider->UpdateTouching();
   }

   mIsDragging = true;
   mDragStart.set(x, y);
   mLastDrag.set(x, y);
   mDragOffset = 0;
   mDragPrecision = 1;
   mIsCaughtOnDetent = false;
   mDragPos = ofClamp((mDragTrack.getMaxY() - y) / mDragTrack.height, 0, 1);

   //catch on a detent if we touched down right next to one
   float catchDistance = kDetentCatchDistance / 2 / mDragTrack.height;
   float detentPos, detentValue;
   if (FindDetent(mDragPos - catchDistance, mDragPos + catchDistance, detentPos, detentValue))
      SetCaughtDetent(detentPos, detentValue);
   else
      SetControlPos(mTouchedControl, mDragPos);
}

void TouchInterfacePanel::UpdateDrag(float x, float y)
{
   if (x == mLastDrag.x && y == mLastDrag.y)
      return;

   auto getSidewaysDistance = [this](float dragX)
   {
      return MAX(0, fabsf(dragX - mDragStart.x) - kDragDeadZone);
   };

   float distance = getSidewaysDistance(x);
   float cursorPos = (mDragTrack.getMaxY() - y) / mDragTrack.height;
   float lastCursorPos = (mDragTrack.getMaxY() - mLastDrag.y) / mDragTrack.height;
   float pos = PrecisionDrag::Apply(mDragOffset, cursorPos, lastCursorPos, distance, getSidewaysDistance(mLastDrag.x), 1, true);
   mDragPrecision = PrecisionDrag::Damp(distance);
   mLastDrag.set(x, y);

   const float catchDistance = kDetentCatchDistance / mDragTrack.height * mDragPrecision;

   if (mIsCaughtOnDetent)
   {
      float move = pos - mDragPos;
      if (fabsf(move) <= catchDistance)
         return;

      //pull free of the detent, and absorb the distance that took, so that the value doesn't jump
      mIsCaughtOnDetent = false;
      float absorb = move > 0 ? catchDistance : -catchDistance;
      mDragOffset -= absorb;
      pos -= absorb;
   }

   float detentPos, detentValue;
   if (FindDetent(mDragPos, pos, detentPos, detentValue) &&
       fabsf(pos - detentPos) <= catchDistance) //let fast movements blow right past it
   {
      SetCaughtDetent(detentPos, detentValue);
      return;
   }

   mDragPos = pos;
   SetControlPos(mTouchedControl, mDragPos);
}

void TouchInterfacePanel::EndTouch()
{
   if (IsValidControl(mTouchedControl))
   {
      if (mIsDragging)
      {
         FloatSlider* slider = dynamic_cast<FloatSlider*>(mTouchedControl);
         if (slider != nullptr)
         {
            slider->mTouching = false;
            slider->UpdateTouching(); //"relative" sliders spring back when released
         }
      }
      else if (dynamic_cast<ClickButton*>(mTouchedControl) != nullptr)
      {
         mTouchedControl->MouseReleased();
      }
   }
   mTouchedControl = nullptr;
   mIsDragging = false;
}

void TouchInterfacePanel::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   EndTouch();

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

   UpdateControlList();

   for (int column = 0; column < kMaxColumns; ++column)
   {
      int index = mPage * kMaxColumns + column;
      if (index >= (int)mControls.size() || !GetColumnRect(column).contains(x, y))
         continue;

      IUIControl* control = mControls[index];
      if (control->IsButtonControl() && !GetSquareRect(column).contains(x, y))
         return;

      mTouchedControl = control;

      if (control->IsButtonControl())
      {
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
         mTouchedControl = nullptr;
         return;
      }

      mDragTrack = GetTrackRect(column);
      BeginDrag(x, y);
      return;
   }
}

bool TouchInterfacePanel::MouseMoved(float x, float y)
{
   if (mIsDragging && IsValidControl(mTouchedControl))
      UpdateDrag(x, y);
   return false;
}

void TouchInterfacePanel::MouseReleased()
{
   EndTouch();
}

void TouchInterfacePanel::DrawButton(ofRectangle rect, std::string label, bool enabled) const
{
   ofPushStyle();
   ofFill();
   ofSetColor(70, 70, 70, enabled ? 255 : 90);
   ofRect(rect, 6);
   ofSetColor(255, 255, 255, enabled ? 255 : 90);
   DrawTextToFit(label, rect.getCenter().x, rect.getCenter().y + 6, rect.width - 8, 18);
   ofPopStyle();
}

void TouchInterfacePanel::DrawControl(IUIControl* control, int column) const
{
   const ofRectangle track = GetTrackRect(column);
   const bool isTouched = control == mTouchedControl;
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
      const bool isDragging = isTouched && mIsDragging;
      const float rawPos = isDragging ? mDragPos : GetControlPos(control);
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
         ofSetLineWidth(isDragging ? MAX(4 * sqrtf(mDragPrecision), .5f) : 4);
         if (isDragging && mIsCaughtOnDetent)
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
   const float left = GetNumPages() > 1 ? GetPageButtonRect(1).getMaxX() + kMargin : content.x;
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
      DrawTextToFit(ofToString(mPage + 1) + "/" + ofToString(numPages), content.x + kPageButtonWidth + kPageLabelWidth / 2, GetPageButtonRect(1).getCenter().y + 6, kPageLabelWidth - 8, 18);
   }

   ofPopStyle();
}
