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
#include "Snapshots.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "Push2Control.h"

//static
IUIControl* IUIControl::sLastHoveredUIControl = nullptr;
//static
bool IUIControl::sLastUIHoverWasSetManually = false;

IUIControl::~IUIControl()
{
   if (gHoveredUIControl == this)
      gHoveredUIControl = nullptr;
   if (gBindToUIControl == this)
      gBindToUIControl = nullptr;
}

bool IUIControl::IsPreset()
{
   return VectorContains(this, Snapshots::sSnapshotHighlightControls);
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
      sLastUIHoverWasSetManually = false;
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
       (PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_Pulse ||
        PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_Modulator ||
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
   if (!mCableTargetable)
      return false;
   if (GetNoHover())
      return false;
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
      color.getHsb(h, s, b);
      color.setHsb(85, s, b);
      textColor.set(0, 255, 0, gModuleDrawAlpha);
   }
   else
   {
      textColor.set(255, 255, 255, gModuleDrawAlpha);
   }
}

void IUIControl::RemoveFromOwner()
{
   IDrawableModule* owner = dynamic_cast<IDrawableModule*>(GetParent());
   assert(owner);
   if (owner)
      owner->RemoveUIControl(this);
}

//static
void IUIControl::SetNewManualHoverViaTab(int direction)
{
   if (gHoveredUIControl == nullptr)
   {
      gHoveredUIControl = sLastHoveredUIControl;
      sLastUIHoverWasSetManually = true;
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
            if (controls[newControlIndex]->IsShowing() && !controls[newControlIndex]->GetNoHover())
            {
               gHoveredUIControl = controls[newControlIndex];
               sLastHoveredUIControl = gHoveredUIControl;
               sLastUIHoverWasSetManually = true;
               break;
            }
         }
      }
   }
}

namespace
{
   //only supports cardinal directions
   float GetDistanceScore(ofVec2f direction, ofRectangle rectA, ofRectangle rectB)
   {
      float score = 0;
      ofVec2f edgeA = rectA.getCenter() + ofVec2f(rectA.width * .5f * direction.x, rectA.height * .5f * direction.y);
      ofVec2f edgeB = rectB.getCenter() + ofVec2f(rectB.width * -.5f * direction.x, rectB.height * -.5f * direction.y);
      ofVec2f toRect = edgeB - edgeA;
      float dot = direction.dot(toRect);
      if (dot > 0)
      {
         ofVec2f perpendicularDirection(direction.y, direction.x);
         float minExtentA = fabsf(rectA.getMinX() * perpendicularDirection.x + rectA.getMinY() * perpendicularDirection.y);
         float maxExtentA = fabsf(rectA.getMaxX() * perpendicularDirection.x + rectA.getMaxY() * perpendicularDirection.y);
         float minExtentB = fabsf(rectB.getMinX() * perpendicularDirection.x + rectB.getMinY() * perpendicularDirection.y);
         float maxExtentB = fabsf(rectB.getMaxX() * perpendicularDirection.x + rectB.getMaxY() * perpendicularDirection.y);
         if (minExtentA <= maxExtentB && maxExtentA >= minExtentB) //overlap, score based upon closest in the specified direction
            score = 1 / direction.dot(toRect) + 1000; //bonus points so that overlapping ones win
         else //no overlap,but still in the requested direction. score based upon overall distance
            score = 1 / toRect.distanceSquared();
      }

      return score;
   }
}

//static
void IUIControl::SetNewManualHoverViaArrow(ofVec2f direction)
{
   if (gHoveredUIControl == nullptr)
   {
      gHoveredUIControl = sLastHoveredUIControl;
      sLastUIHoverWasSetManually = true;
   }
   else
   {
      IDrawableModule* uiControlModule = gHoveredUIControl->GetModuleParent();
      if (uiControlModule != nullptr)
      {
         const auto& controls = uiControlModule->GetUIControls();
         ofRectangle currentControlRect = gHoveredUIControl->GetRect();
         float bestScore = 0;
         int bestScoreIndex = -1;
         for (int i = 0; i < (int)controls.size(); ++i)
         {
            if (controls[i]->IsShowing() && !controls[i]->GetNoHover() && controls[i] != gHoveredUIControl)
            {
               float score = GetDistanceScore(direction, currentControlRect, controls[i]->GetRect());
               if (score > bestScore)
               {
                  bestScore = score;
                  bestScoreIndex = i;
               }
            }
         }

         if (bestScoreIndex != -1)
         {
            gHoveredUIControl = controls[bestScoreIndex];
            sLastHoveredUIControl = gHoveredUIControl;
            sLastUIHoverWasSetManually = true;
         }
      }
   }
}

//static
void IUIControl::DestroyCablesTargetingControls(std::vector<IUIControl*> controls)
{
   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);
   std::vector<PatchCable*> cablesToDestroy;
   for (const auto module_iter : modules)
   {
      for (const auto source : module_iter->GetPatchCableSources())
      {
         for (const auto cable : source->GetPatchCables())
         {
            for (const auto control : controls)
            {
               if (cable->GetTarget() == control)
               {
                  cablesToDestroy.push_back(cable);
                  break;
               }
            }
         }
      }
   }
   for (const auto cable : cablesToDestroy)
      cable->Destroy(false);
}
