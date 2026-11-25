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
#include "FloatSliderLFOControl.h"
#include "HelpDisplay.h"
#include "IControlVisualizer.h"
#include "IModulator.h"
#include "LocationZoomer.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "PatchCableSource.h"
#include "QuickSpawnMenu.h"
#include "Slider.h"
#include "SynthGlobals.h"
#include "TitleBar.h"
#include "nanovg/nanovg.h"

namespace
{
   const float kMargin = 8;
   const float kHeaderHeight = 44;
   const float kHeaderButtonWidth = 64;
   const float kPageLabelHeight = 20;
   const float kLabelHeight = 42;
   const float kMaxColumnWidth = 120;
   const float kColumnSpacing = 6;
   const float kDragDeadZone = 20; //fingers don't move in straight lines, so allow some sideways drift before increasing precision
   const float kTapDistance = 10; //touches on modulated controls that move less than this are taps
   const float kDetentCatchDistance = 14; //how far to drag to get out of a detent
   const float kSideButtonWidth = 72;
   const float kSideButtonHeight = 44;
   const float kSideButtonSpacing = 4;
   const float kRowSpacing = 8;
   const float kTargetTrackInches = 2; //on displays with room for multiple rows of sliders, how tall the sliders should be, at minimum
   const double kRoutingUpdateMs = 250;
   const float kZoomBlendMs = 500;
   const ofColor kBackgroundColor(20, 20, 20, 153);
   const double kDoubleTapMs = 300;
   const float kDoubleTapDistance = 15;

   //JUCE's display DPI is nominal on most platforms, so just use what's typical for the type of display.
   //these are in the same units as ofGetWidth()/ofGetHeight(), i.e., before applying the display's pixel ratio
   float GetPointsPerInch()
   {
#if JUCE_IOS
      static const bool sIsTablet = juce::SystemStats::getDeviceDescription().containsIgnoreCase("iPad");
      return sIsTablet ? 132 : 153;
#elif JUCE_ANDROID
      return 160;
#elif JUCE_MAC
      return 110;
#else
      return 96;
#endif
   }

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
   mVisualizationLCD.Init();
   TheSynth->AddTouchListener(this);
}

TouchInterfacePanel::~TouchInterfacePanel()
{
   if (TheSynth != nullptr)
      TheSynth->RemoveTouchListener(this);
}

bool TouchInterfacePanel::ShowForModule(IDrawableModule* module)
{
   if (ControlSurface::GetDisplayableControls(module, ControlSurfaceDeviceType::None).empty())
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
   mLFOTarget = nullptr;
   mLFOTargetParent = nullptr;
   mLFOTargetModule = nullptr;
   if (FloatSliderLFOControl* lfo = GetUnpinnedLFO())
   {
      mLFOTarget = lfo->GetOwner();
      if (mLFOTarget != nullptr)
      {
         mLFOTargetParent = dynamic_cast<IDrawableModule*>(mLFOTarget->GetParent());
         mLFOTargetModule = mLFOTarget->GetModuleParent();
      }
   }
   mControls = ControlSurface::GetDisplayableControls(module, ControlSurfaceDeviceType::None);
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
   EndZoom();
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
   if (TheSynth->GetTouchInteractionMode() != TouchInteractionMode::Interface || (TheQuickSpawnMenu->IsShowing() && !mIsZoomed))
   {
      Hide();
      return;
   }

   SetPosition(0, GetTitleBarHeight());
   UpdateControlList();
   mPage = ofClamp(mPage, 0, GetNumPages() - 1); //resizing can change how many rows fit
   if (IsShowing() && gTime - mLastRoutingUpdateTime > kRoutingUpdateMs)
      UpdateRoutingModules();

   if (mIsZoomed)
   {
      if (GetCanvasModule() == nullptr)
         EndZoom();
      else if (mZoomScreenSize.x != ofGetWidth() || mZoomScreenSize.y != ofGetHeight()) //e.g. the device was rotated
         ZoomToModule();
   }
}

IDrawableModule* TouchInterfacePanel::GetCanvasModule() const
{
   IDrawableModule* module = GetUnpinnedLFO() != nullptr ? mLFOTargetModule : mModule; //unpinned LFOs aren't on the canvas
   if (module == nullptr || module->IsDeleted() || module->GetOwningContainer() == nullptr || !module->IsShowing())
      return nullptr;
   return module;
}

ofRectangle TouchInterfacePanel::GetCanvasModuleRect(IDrawableModule* module) const
{
   ofRectangle rect = module->GetRect();
   if (module->HasTitleBar())
   {
      rect.y -= IDrawableModule::TitleBarHeight();
      rect.height += IDrawableModule::TitleBarHeight();
   }
   return rect;
}

bool TouchInterfacePanel::ZoomToModule()
{
   IDrawableModule* module = GetCanvasModule();
   if (module == nullptr)
      return false;

   const ofRectangle moduleRect = GetCanvasModuleRect(module);
   if (moduleRect.width <= 0 || moduleRect.height <= 0)
      return false;

   EndAllTouches();

   //fit below our buttons, or between them if the module is narrow enough to get bigger that way, so that they don't cover it
   const float uiScale = GetOwningContainer()->GetDrawScale();
   const ofRectangle content = GetContentRect();
   const ofRectangle zoomButton = GetZoomButtonRect();
   const ofRectangle exitButton = GetExitButtonRect();
   const ofRectangle below(content.x, content.y + kHeaderHeight, content.width, content.height - kHeaderHeight);
   const ofRectangle between(exitButton.getMaxX() + kMargin, content.y, zoomButton.x - exitButton.getMaxX() - kMargin * 2, content.height);
   auto getFit = [&moduleRect, uiScale](const ofRectangle& rect)
   {
      return ofClamp(MIN(rect.width / moduleRect.width, rect.height / moduleRect.height) * uiScale, .1f, 8);
   };
   const ofRectangle area = getFit(between) > getFit(below) ? between : below;
   const float scale = getFit(area);
   ofVec2f screenCenter(area.getCenter().x * uiScale, (area.getCenter().y + GetTitleBarHeight()) * uiScale);

   mIsZoomed = true;
   mZoomScreenSize.set(ofGetWidth(), ofGetHeight());
   gHoveredUIControl = nullptr;
   TheSynth->GetLocationZoomer()->SetTemporaryView(scale, screenCenter / scale - moduleRect.getCenter(), kZoomBlendMs);
   return true;
}

void TouchInterfacePanel::EndZoom()
{
   if (!mIsZoomed)
      return;
   mIsZoomed = false;
   gHoveredUIControl = nullptr; //don't let clicks get directed to a control that we're covering up
   TheSynth->GetLocationZoomer()->EndTemporaryView(kZoomBlendMs);
}

void TouchInterfacePanel::UpdateControlList()
{
   if (mModule == nullptr)
      return;

   if (mModule->IsDeleted() || !IsUnpinnedLFOValid())
   {
      mControls.clear(); //they might not exist anymore
      Hide();
      return;
   }

   std::vector<IUIControl*> controls = ControlSurface::GetDisplayableControls(mModule, ControlSurfaceDeviceType::None);
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

IDrawableModule* TouchInterfacePanel::GetModulatorModule(IUIControl* control) const
{
   if (dynamic_cast<FloatSlider*>(control) != nullptr)
   {
      IModulator* modulator = control->GetModulator();
      if (modulator == nullptr || !modulator->Active())
         return nullptr;
      if (IDrawableModule* module = dynamic_cast<IDrawableModule*>(modulator))
         return module;
   }

   //only float sliders know about their modulators, and some modulators are just a part of a module, so look for the cable
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   for (auto* module : modules)
   {
      if (!module->IsEnabled())
         continue;
      for (auto* source : module->GetPatchCableSources())
      {
         if (source->GetConnectionType() != kConnectionType_Modulator)
            continue;
         for (auto* cable : source->GetPatchCables())
         {
            if (cable->GetTarget() == control)
               return module;
         }
      }
   }
   return nullptr;
}

FloatSliderLFOControl* TouchInterfacePanel::GetUnpinnedLFO() const
{
   FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(mModule);
   return lfo != nullptr && !lfo->IsPinned() ? lfo : nullptr;
}

//unpinned LFOs are pooled, so they outlive the slider that they're modulating, and can get reassigned to a different one
bool TouchInterfacePanel::IsUnpinnedLFOValid() const
{
   FloatSliderLFOControl* lfo = GetUnpinnedLFO();
   if (lfo == nullptr)
      return true;

   if (mLFOTarget == nullptr || lfo->GetOwner() != mLFOTarget || mLFOTargetModule == nullptr || mLFOTargetModule->IsDeleted())
      return false;

   //the slider might not exist anymore, so make sure that we can find it before touching it
   if (mLFOTargetParent != mLFOTargetModule && !VectorContains(mLFOTargetParent, mLFOTargetModule->GetChildren())) //e.g., effects in an effectchain
      return false;
   return VectorContains(static_cast<IUIControl*>(mLFOTarget), mLFOTargetParent->GetUIControls());
}

std::string TouchInterfacePanel::GetModuleLabel(IDrawableModule* module) const
{
   if (module == mModule && mLFOTarget != nullptr && GetUnpinnedLFO() != nullptr)
      return "lfo: " + GetControlLabel(mLFOTarget);
   return module->GetTitleLabel();
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
         {
            IDrawableModule* target = dynamic_cast<IDrawableModule*>(cable->GetTarget());
            if (IUIControl* control = dynamic_cast<IUIControl*>(cable->GetTarget())) //e.g., something that we're modulating
               target = control->GetModuleParent();
            add(outputs, target, source->GetColor());
         }
      }

      if (mLFOTarget != nullptr && GetUnpinnedLFO() != nullptr) //these don't have cables
         add(outputs, mLFOTargetModule, IDrawableModule::GetColor(kModuleCategory_Modulator));
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
   return MAX(1, ((int)mControls.size() + GetNumControlsPerPage() - 1) / GetNumControlsPerPage());
}

int TouchInterfacePanel::GetNumRows() const
{
   //doesn't use GetControlsRect(), because that depends on whether there are multiple pages, which depends on this
   const float height = GetContentRect().height - kHeaderHeight - kPageLabelHeight;
   const float targetRowHeight = kTargetTrackInches * GetPointsPerInch() / GetOwningContainer()->GetDrawScale() + kLabelHeight;
   const int numRowsThatFit = MAX(1, (int)((height + kRowSpacing) / (targetRowHeight + kRowSpacing)));
   const int numRowsNeeded = MAX(1, ((int)mControls.size() + kMaxColumns - 1) / kMaxColumns);
   return MIN(numRowsThatFit, numRowsNeeded);
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
   return ofRectangle(content.x, content.y, kHeaderButtonWidth, kHeaderHeight - 8);
}

ofRectangle TouchInterfacePanel::GetZoomButtonRect() const
{
   ofRectangle content = GetContentRect();
   return ofRectangle(content.getMaxX() - kHeaderButtonWidth, content.y, kHeaderButtonWidth, kHeaderHeight - 8);
}

ofRectangle TouchInterfacePanel::GetPageButtonRect(int direction) const
{
   ofRectangle content = GetContentRect();
   return ofRectangle(direction < 0 ? content.x : content.getMaxX() - kSideButtonWidth, content.getMaxY() - kSideButtonHeight, kSideButtonWidth, kSideButtonHeight);
}

ofRectangle TouchInterfacePanel::GetSlotRect(int slot) const
{
   ofRectangle area = GetControlsRect();
   int numColumns = GetNumColumns();
   float columnWidth = MIN(area.width / numColumns, kMaxColumnWidth);
   float startX = area.x + (area.width - columnWidth * numColumns) / 2;
   const int numRows = GetNumRows();
   const float rowHeight = (area.height - kRowSpacing * (numRows - 1)) / numRows;
   return ofRectangle(startX + columnWidth * (slot % kMaxColumns), area.y + (rowHeight + kRowSpacing) * (slot / kMaxColumns), columnWidth, rowHeight);
}

ofRectangle TouchInterfacePanel::GetTrackRect(int slot) const
{
   ofRectangle rect = GetSlotRect(slot);
   return ofRectangle(rect.x + kColumnSpacing / 2, rect.y, rect.width - kColumnSpacing, MAX(1, rect.height - kLabelHeight));
}

ofRectangle TouchInterfacePanel::GetLabelRect(int slot) const
{
   const ofRectangle track = GetTrackRect(slot);
   return ofRectangle(track.x, track.getMaxY() + 3, track.width, kLabelHeight - 3);
}

ofRectangle TouchInterfacePanel::GetSquareRect(int slot) const
{
   ofRectangle track = GetTrackRect(slot);
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
   control->StartBeacon();
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
   touch.mControl->StartBeacon();

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
      if (touch.mControl != nullptr && touch.mControl == control && !touch.mIsTooltip)
         return &touch;
   }
   return nullptr;
}

void TouchInterfacePanel::AssignTouch(Touch& touch, int touchId, IUIControl* control)
{
   touch.mTouchId = touchId;
   touch.mControl = control;
   touch.mOrder = ++mTouchCounter;
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
      if (touch.mIsModulatorTap || touch.mIsTooltip)
      {
         //didn't do anything to the control
      }
      else if (touch.mIsDragging)
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

bool TouchInterfacePanel::TestClick(float x, float y, bool right, bool testOnly /*=false*/)
{
   if (mIsZoomed && !GetZoomButtonRect().contains(x - mX, y - mY) && !GetExitButtonRect().contains(x - mX, y - mY))
      return false; //let it through to the canvas
   return IDrawableModule::TestClick(x, y, right, testOnly);
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

   UpdateControlList(); //make sure that everything still exists
   if (!IsShowing())
      return;

   if (Touch* touch = FindTouch(touchId)) //in case we missed the release
      EndTouch(*touch);

   if (GetExitButtonRect().contains(x, y))
   {
      Hide();
      return;
   }

   if (GetZoomButtonRect().contains(x, y))
   {
      if (mIsZoomed)
         EndZoom();
      else
         ZoomToModule();
      return;
   }

   if (mIsZoomed)
      return;

   if (Checkbox* enabledCheckbox = GetEnabledCheckbox())
   {
      if (mTitleRect.contains(x, y))
      {
         enabledCheckbox->StartBeacon();
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

   for (int slot = 0; slot < GetNumControlsPerPage(); ++slot)
   {
      int index = mPage * GetNumControlsPerPage() + slot;
      if (index >= (int)mControls.size() || !GetSlotRect(slot).contains(x, y))
         continue;

      IUIControl* control = mControls[index];
      Touch* touch = FindFreeTouch();

      if (y >= GetLabelRect(slot).y)
      {
         if (touch != nullptr)
         {
            AssignTouch(*touch, touchId, control);
            touch->mIsTooltip = true;
         }
         return;
      }

      if (control->IsButtonControl() && !GetSquareRect(slot).contains(x, y))
         return;

      if (touch == nullptr || FindTouch(control) != nullptr) //another touch already has this control
         return;

      if (GetModulatorModule(control) != nullptr)
      {
         AssignTouch(*touch, touchId, control);
         touch->mIsModulatorTap = true;
         touch->mDragTrack = GetTrackRect(slot);
         touch->mDragStart.set(x, y);
         return;
      }

      if (control->IsButtonControl())
      {
         AssignTouch(*touch, touchId, control);
         control->StartBeacon();
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
         control->StartBeacon();
         control->ResetToDefault();
         return;
      }

      AssignTouch(*touch, touchId, control);
      touch->mDragTrack = GetTrackRect(slot);
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
   if (touch == nullptr)
      return;

   if (touch->mIsModulatorTap && touch->mDragStart.distanceSquared(ofVec2f(x, y)) > kTapDistance * kTapDistance)
   {
      if (touch->mControl->IsButtonControl())
      {
         EndTouch(*touch);
         return;
      }
      touch->mIsModulatorTap = false;
      BeginDrag(*touch, x, y);
   }

   if (touch->mIsDragging)
      UpdateDrag(*touch, x, y);
}

void TouchInterfacePanel::TouchReleased(int touchId)
{
   if (FindTouch(touchId) == nullptr)
      return;

   UpdateControlList();

   if (Touch* touch = FindTouch(touchId))
   {
      IDrawableModule* modulatorModule = touch->mIsModulatorTap ? GetModulatorModule(touch->mControl) : nullptr;
      EndTouch(*touch);
      if (modulatorModule != nullptr)
         SetModule(modulatorModule);
   }
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

void TouchInterfacePanel::DrawControl(IUIControl* control, int slot) const
{
   const ofRectangle track = GetTrackRect(slot);
   const Touch* touch = FindTouch(control);
   const bool isTouched = touch != nullptr;
   ofColor color = IDrawableModule::GetColor(mModule->GetModuleCategory());

   ofPushStyle();
   ofFill();

   if (control->IsButtonControl())
   {
      const ofRectangle square = GetSquareRect(slot);
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
   ofRect(GetLabelRect(slot), 4);

   ofSetColor(255, 255, 255);
   DrawTextToFit(GetControlLabel(control), track.getCenter().x, track.getMaxY() + 18, track.width, 16);
   if (dynamic_cast<ClickButton*>(control) == nullptr)
      DrawTextToFit(control->GetDisplayValue(control->GetValue()), track.getCenter().x, track.getMaxY() + 36, track.width, 14);

   ofPopStyle();
}

void TouchInterfacePanel::DrawTitle()
{
   const ofRectangle content = GetContentRect();
   const float left = GetExitButtonRect().getMaxX() + kMargin;
   const float maxWidth = GetZoomButtonRect().x - kMargin - left;
   const float centerX = left + maxWidth / 2;
   const float centerY = GetExitButtonRect().getCenter().y;
   float size = 22;

   ofPushStyle();

   Checkbox* enabledCheckbox = GetEnabledCheckbox();
   const float kCircleRadius = 9;
   const float kCircleSpace = enabledCheckbox != nullptr ? kCircleRadius * 2 + 8 : 0;
   const float kPadding = 10;

   std::string title = GetModuleLabel(mModule);
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

//highlights the module out on the canvas
void TouchInterfacePanel::DrawModuleHighlight() const
{
   if (!IsShowing() || mIsZoomed)
      return;

   IDrawableModule* module = GetCanvasModule();
   if (module == nullptr)
      return;

   const ofRectangle rect = GetCanvasModuleRect(module);

   ofPushStyle();
   ofNoFill();
   ofSetColor(255, 255, 255, ofMap(sin(gTime / 1000 * PI * 2), -1, 1, 100, 180));
   ofSetLineWidth(3);
   ofRect(rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6, 6);
   ofPopStyle();
}

//darkens everything around the module that we're zoomed in on
void TouchInterfacePanel::DrawZoomFrame()
{
   float w, h;
   GetModuleDimensions(w, h);

   ofPushStyle();
   ofFill();
   ofSetColor(kBackgroundColor);
   IDrawableModule* module = GetCanvasModule();
   if (module == nullptr)
   {
      ofRect(0, 0, w, h, 0);
      ofPopStyle();
      return;
   }

   const float kPadding = 3;
   const float scale = gDrawScale / GetOwningContainer()->GetDrawScale();
   const ofRectangle rect = GetCanvasModuleRect(module);
   const ofVec2f offset = TheSynth->GetDrawOffset();
   const float left = ofClamp((rect.x - kPadding + offset.x) * scale - mX, 0, w);
   const float right = ofClamp((rect.getMaxX() + kPadding + offset.x) * scale - mX, 0, w);
   const float top = ofClamp((rect.y - kPadding + offset.y) * scale - mY, 0, h);
   const float bottom = ofClamp((rect.getMaxY() + kPadding + offset.y) * scale - mY, 0, h);

   //four rectangles, since paths with holes don't draw correctly on iOS
   auto drawRect = [](float x, float y, float width, float height)
   {
      if (width > 0 && height > 0)
         ofRect(x, y, width, height, 0);
   };
   drawRect(0, 0, w, top);
   drawRect(0, bottom, w, h - bottom);
   drawRect(0, top, left, bottom - top);
   drawRect(right, top, w - right, bottom - top);
   ofPopStyle();
}

//shows the visualization for the most recently touched control that has one, or the module's LFO, next to the module's name
void TouchInterfacePanel::DrawControlVisualization()
{
   const Touch* newest = nullptr;
   for (const auto& touch : mTouches)
   {
      if (touch.mControl != nullptr && !touch.mIsTooltip && touch.mControl->GetControlVisualizer() != nullptr && (newest == nullptr || touch.mOrder > newest->mOrder))
         newest = &touch;
   }
   FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(mModule);
   if ((newest == nullptr && lfo == nullptr) || mVisualizationLCD.GetPixels() == nullptr)
      return;

   const int lcdWidth = mVisualizationLCD.GetWidth();
   const int lcdHeight = mVisualizationLCD.GetHeight();

   const ofRectangle content = GetContentRect();
   ofRectangle rect(mTitleRect.getMaxX() + kMargin, content.y, 0, kHeaderHeight - 8);
   rect.width = rect.height * lcdWidth / lcdHeight;
   if (rect.getMaxX() > GetZoomButtonRect().x - kMargin)
   {
      rect.x = mTitleRect.x - kMargin - rect.width; //try the other side
      if (rect.x < GetExitButtonRect().getMaxX() + kMargin)
         return;
   }

   mVisualizationLCD.Clear();
   if (newest == nullptr)
      lfo->DrawToAbletonMoveScreen(&mVisualizationLCD);
   else
      newest->mControl->GetControlVisualizer()->DrawVisualizationToScreen(&mVisualizationLCD, newest->mControl);

   static std::vector<unsigned char> sPixels;
   static int sImage = -1;
   static int sImageContextGeneration = -1;

   sPixels.resize(lcdWidth * lcdHeight * 4);
   for (int y = 0; y < lcdHeight; ++y)
   {
      for (int x = 0; x < lcdWidth; ++x)
      {
         //white, with the lcd's value as the opacity
         unsigned char* pixel = &sPixels[(x + y * lcdWidth) * 4];
         pixel[0] = pixel[1] = pixel[2] = 255;
         pixel[3] = mVisualizationLCD.GetPixel(x, y) > 0 ? 255 : 0;
      }
   }

   //images don't survive the GL context being recreated
   if (sImageContextGeneration != gNanoVGContextGeneration)
   {
      sImage = -1;
      sImageContextGeneration = gNanoVGContextGeneration;
   }

   if (sImage == -1)
      sImage = nvgCreateImageRGBA(gNanoVG, lcdWidth, lcdHeight, 0, sPixels.data());
   else
      nvgUpdateImage(gNanoVG, sImage, sPixels.data());

   ofPushStyle();
   ofFill();
   ofSetColor(0, 0, 0, 180);
   ofRect(rect, 4);
   ofPopStyle();

   if (sImage <= 0)
   {
      sImage = -1;
      return;
   }

   NVGpaint paint = nvgImagePattern(gNanoVG, rect.x, rect.y, rect.width, rect.height, 0, sImage, 1);
   nvgBeginPath(gNanoVG);
   nvgRect(gNanoVG, rect.x, rect.y, rect.width, rect.height);
   nvgFillPaint(gNanoVG, paint);
   nvgFill(gNanoVG);
}

//for the control whose name is being pressed
void TouchInterfacePanel::DrawTooltip() const
{
   const Touch* newest = nullptr;
   for (const auto& touch : mTouches)
   {
      if (touch.mControl != nullptr && touch.mIsTooltip && (newest == nullptr || touch.mOrder > newest->mOrder))
         newest = &touch;
   }
   if (newest == nullptr)
      return;

   int slot = -1;
   for (int i = 0; i < GetNumControlsPerPage(); ++i)
   {
      const int index = mPage * GetNumControlsPerPage() + i;
      if (index < (int)mControls.size() && mControls[index] == newest->mControl)
         slot = i;
   }
   if (slot == -1)
      return;

   const std::string tooltip = TheTitleBar->GetHelpDisplay()->GetUIControlTooltip(newest->mControl);
   const ofRectangle label = GetLabelRect(slot);
   const ofRectangle content = GetContentRect();
   const float kFontSize = 18;
   const float kPadding = 8;
   const float maxWidth = MIN(360, content.width - kPadding * 2);

   nvgFontFaceId(gNanoVG, gFont.GetFontHandle());
   nvgFontSize(gNanoVG, kFontSize);
   float bounds[4];
   nvgTextBoxBounds(gNanoVG, 0, 0, maxWidth, tooltip.c_str(), nullptr, bounds);

   //above the name, so that it isn't under the finger
   ofRectangle rect(0, 0, bounds[2] - bounds[0] + kPadding * 2, bounds[3] - bounds[1] + kPadding * 2);
   rect.x = ofClamp(label.getCenter().x - rect.width / 2, content.x, content.getMaxX() - rect.width);
   rect.y = MAX(label.y - 24 - rect.height, content.y);

   ofPushStyle();
   ofFill();
   ofSetColor(50, 50, 50, 240);
   ofRect(rect, 4);
   ofNoFill();
   ofSetColor(255, 255, 255, 180);
   ofSetLineWidth(1);
   ofRect(rect, 4);
   ofSetColor(255, 255, 255);
   gFont.DrawStringWrap(tooltip, kFontSize, rect.x + kPadding - bounds[0], rect.y + kPadding - bounds[1], maxWidth);
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

   if (mIsZoomed)
   {
      DrawZoomFrame();
      DrawButton(GetZoomButtonRect(), "return", true);
      DrawButton(GetExitButtonRect(), "exit", true);
      return;
   }

   float w, h;
   GetModuleDimensions(w, h);

   ofPushStyle();
   ofFill();
   ofSetColor(kBackgroundColor);
   ofRect(0, 0, w, h, 0);

   const ofRectangle content = GetContentRect();

   DrawTitle();
   DrawControlVisualization();

   DrawButton(GetZoomButtonRect(), "zoom", GetCanvasModule() != nullptr);
   DrawButton(GetExitButtonRect(), "exit", true);

   DrawRoutingButtons(true);
   DrawRoutingButtons(false);

   if (mControls.empty()) //we can get here through the signal chain
   {
      ofRectangle area = GetControlsRect();
      ofSetColor(255, 255, 255, 130);
      DrawTextToFit("no controls", area.getCenter().x, area.getCenter().y + 6, area.width, 18);
   }

   for (int slot = 0; slot < GetNumControlsPerPage(); ++slot)
   {
      int index = mPage * GetNumControlsPerPage() + slot;
      if (index < (int)mControls.size())
         DrawControl(mControls[index], slot);
   }

   const int numPages = GetNumPages();
   if (numPages > 1)
   {
      DrawButton(GetPageButtonRect(-1), "<", mPage > 0);
      DrawButton(GetPageButtonRect(1), ">", mPage < numPages - 1);
      ofSetColor(255, 255, 255);
      DrawTextToFit(ofToString(mPage + 1) + "/" + ofToString(numPages), w / 2, content.getMaxY() - 2, 80, 18);
   }

   DrawTooltip();

   ofPopStyle();
}
