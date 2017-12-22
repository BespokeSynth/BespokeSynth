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
            return true;
      }
   }
   
   return false;
}

void IUIControl::CheckHover(int x, int y)
{
   if (TestHover(x, y))
      gHoveredUIControl = this;
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
      ofRect(mX-1,mY-1,w+2,h+2,4);
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
      color = IDrawableModule::GetColor(module->GetModuleType());
   else
      color = ofColor::white;
   float h,s,b;
   color.getHsb(h, s, b);
   color.setHsb(h, s * .4f, ofLerp(b,0,.6f));
   if (IsPreset())
   {
      float h,s,b;
      color.getHsb(h, s, b);
      color.setHsb(85, s, b);
      textColor.set(0,255,0,gModuleDrawAlpha);
   }
   else
   {
      textColor.set(255,255,255,gModuleDrawAlpha);
   }
}
