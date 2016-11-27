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

IUIControl::~IUIControl()
{
   if (gHoveredUIControl == this)
      gHoveredUIControl = NULL;
   if (gBindToUIControl == this)
      gBindToUIControl = NULL;
}

bool IUIControl::IsPreset()
{
   return VectorContains(this, Presets::sPresetHighlightControls);
}

void IUIControl::CheckHover(int x, int y)
{
   if (mNoHover)
      return;
   
   if (!mShowing)
      return;
   
   int w,h;
   GetDimensions(w, h);
   if (x>=0 && x<w && y>=0 && y<h) //make sure we're hovered over the control
   {
      IDrawableModule* moduleParent = GetModuleParent();
      int thisX,thisY;
      GetPosition(thisX, thisY);
      x += thisX;
      y += thisY;
      if (TheSynth->GetModuleAt(x,y) == moduleParent)
      {
         int localX,localY;
         GetPosition(localX, localY, K(localOnly));
         int parentW,parentH;
         GetParent()->GetDimensions(parentW, parentH);
         if (localX < parentW && localY < parentH)
            gHoveredUIControl = this;
      }
   }
}

void IUIControl::DrawHover()
{
   if (gHoveredUIControl == this)
   {
      int w,h;
      GetDimensions(w, h);
      ofPushStyle();
      ofNoFill();
      ofSetColor(0,255,255);
      ofRect(mX-1,mY-1,w+2,h+2);
      ofPopStyle();
   }
   
   if (mRemoteControlCount > 0 && TheSynth->InMidiMapMode())
   {
      int w,h;
      GetDimensions(w, h);
      ofPushStyle();
      ofFill();
      ofSetColor(255,0,255,100);
      ofRect(mX-1,mY-1,w+2,h+2);
      ofPopStyle();
   }
   
   if (gBindToUIControl == this)
   {
      int w,h;
      GetDimensions(w, h);
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(5);
      ofSetColor(255,0,255,200);
      ofRect(mX-1,mY-1,w+2,h+2);
      ofPopStyle();
   }
   
   if (PatchCable::sActivePatchCable &&
       PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_UIControl &&
       PatchCable::sActivePatchCable->IsValidTarget(this))
   {
      int w,h;
      GetDimensions(w, h);
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(5);
      ofSetColor(255,0,255,200);
      ofRect(mX-1,mY-1,w+2,h+2);
      ofPopStyle();
   }
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
   if (direction == kAnchorDirection_Below)
   {
      mX = rect.x;
      mY = rect.y + rect.height + 1;
   }
   else if (direction == kAnchorDirection_Right)
   {
      mX = rect.x + rect.width + 3;
      mY = rect.y;
   }
}
