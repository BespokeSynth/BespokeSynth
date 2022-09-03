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
//
//  IUIControl.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 6/13/13.
//
//

#include "IUIControl.h"
#include "Presets.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "Push2Control.h"
#include "TextEntry.h"

//static
IUIControl* IUIControl::sLastHoveredUIControl = nullptr;
//static
bool IUIControl::sLastUIHoverWasSetViaTab = false;

IUIControl::~IUIControl()
{
   if (gHoveredUIControl == this)
      gHoveredUIControl = nullptr;
   if (gBindToUIControl == this)
      gBindToUIControl = nullptr;
}

bool IUIControl::IsPreset()
{
   return VectorContains(this, Presets::sPresetHighlightControls);
}

bool IUIControl::TestHover(int x, int y)
{
   if (mNoHover)
      return false;

   if (!mShowing)
      return false;

   float w, h;
   GetDimensions(w, h);
   if (x >= 0 && x < w && y >= 0 && y < h) //make sure we're hovered over the control
   {
      IDrawableModule* moduleParent = GetModuleParent();
      float thisX, thisY;
      GetPosition(thisX, thisY);
      x += thisX;
      y += thisY;
      if (moduleParent->GetOwningContainer()->GetModuleAt(x, y) == moduleParent)
      {
         float localX, localY;
         GetPosition(localX, localY, K(localOnly));
         float parentW, parentH;
         GetParent()->GetDimensions(parentW, parentH);
         if (localX < parentW && localY < parentH)
            return true;
      }
   }

   return false;
}

void IUIControl::CheckHover(int x, int y)
{
   static long sLastHoveredUIControlFrame = 0;
   if (TheSynth->GetFrameCount() != sLastHoveredUIControlFrame &&
       !TheSynth->IsGroupSelecting() && PatchCable::sActivePatchCable == nullptr &&
       TestHover(x, y) &&
       (gHoveredUIControl == nullptr || !gHoveredUIControl->IsMouseDown()) &&
       GetModuleParent() == TheSynth->GetModuleAtCursor())
   {
      gHoveredUIControl = this;
      sLastHoveredUIControl = this;
      sLastUIHoverWasSetViaTab = false;
      sLastHoveredUIControlFrame = TheSynth->GetFrameCount();
   }
}

void IUIControl::DrawHover(float x, float y, float w, float h)
{
   if (Push2Control::sDrawingPush2Display)
      return;

   if (gHoveredUIControl == this && IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr && TheSynth->GetGroupSelectedModules().empty())
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(0, 255, 255, 255);
      ofRect(x, y, w, h, 4);
      ofPopStyle();
   }

   if (mRemoteControlCount > 0 && TheSynth->InMidiMapMode())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 255, 100);
      ofRect(x, y, w, h);
      ofPopStyle();
   }

   if (gBindToUIControl == this)
   {
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(5);
      ofSetColor(255, 0, 255, 200);
      ofRect(x, y, w, h);
      ofPopStyle();
   }

   DrawPatchCableHover();
}

void IUIControl::DrawPatchCableHover()
{
   if (PatchCable::sActivePatchCable &&
       (PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_Modulator ||
        PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_ValueSetter ||
        PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_UIControl ||
        PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_Grid) &&
       PatchCable::sActivePatchCable->IsValidTarget(this))
   {
      float w, h;
      GetDimensions(w, h);
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(1.5f);
      ofSetColor(255, 0, 255, 200);
      ofRect(mX, mY, w, h);
      ofPopStyle();
   }
}

bool IUIControl::CanBeTargetedBy(PatchCableSource* source) const
{
   return source->GetConnectionType() == kConnectionType_Modulator || source->GetConnectionType() == kConnectionType_ValueSetter || source->GetConnectionType() == kConnectionType_UIControl;
}

void IUIControl::StartBeacon()
{
   IClickable::StartBeacon();
   IClickable* moduleParent = GetModuleParent();
   if (moduleParent)
      moduleParent->StartBeacon();
}

void IUIControl::PositionTo(IUIControl* anchor, AnchorDirection direction)
{
   ofRectangle rect = anchor->GetRect(true);
   if (direction == kAnchor_Below)
   {
      mX = rect.x;
      mY = rect.y + rect.height + 2;
   }
   else if (direction == kAnchor_Right)
   {
      mX = rect.x + rect.width + 3;
      mY = rect.y;
   }
   else if (direction == kAnchor_Right_Padded)
   {
      mX = rect.x + rect.width + 10;
      mY = rect.y;
   }
}

void IUIControl::GetColors(ofColor& color, ofColor& textColor)
{
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(GetParent());
   if (module)
      color = IDrawableModule::GetColor(module->GetModuleCategory());
   else
      color = ofColor::white;
   float h, s, b;
   color.getHsb(h, s, b);
   color.setHsb(h, s * .4f, ofLerp(b, 0, .6f));
   if (IsPreset())
   {
      float h, s, b;
      color.getHsb(h, s, b);
      color.setHsb(85, s, b);
      textColor.set(0, 255, 0, gModuleDrawAlpha);
   }
   else
   {
      textColor.set(255, 255, 255, gModuleDrawAlpha);
   }
}

//static
void IUIControl::SetNewManualHover(int direction)
{
   if (gHoveredUIControl == nullptr)
   {
      gHoveredUIControl = sLastHoveredUIControl;
      sLastUIHoverWasSetViaTab = true;
   }
   else
   {
      IDrawableModule* uiControlModule = gHoveredUIControl->GetModuleParent();
      if (uiControlModule != nullptr)
      {
         const auto& controls = uiControlModule->GetUIControls();
         int controlIndex = 0;
         for (int i = 0; i < (int)controls.size(); ++i)
         {
            if (controls[i] == gHoveredUIControl)
            {
               controlIndex = i;
               break;
            }
         }

         for (int i = 1; i < (int)controls.size(); ++i)
         {
            int newControlIndex = (controlIndex + i * direction + (int)controls.size()) % (int)controls.size();
            if (controls[newControlIndex]->IsShowing())
            {
               gHoveredUIControl = controls[newControlIndex];
               sLastHoveredUIControl = gHoveredUIControl;
               sLastUIHoverWasSetViaTab = true;
               break;
            }
         }
      }
   }
}
