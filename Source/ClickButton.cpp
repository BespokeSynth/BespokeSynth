//
//  ClickButton.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "SynthGlobals.h"

ClickButton::ClickButton(IButtonListener* owner, const char* label, int x, int y)
: mClickTime(-9999)
, mOwner(owner)
{
   assert(owner);
   SetLabel(label);
   SetPosition(x,y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

ClickButton::ClickButton(IButtonListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection)
: ClickButton(owner, label, -1, -1)
{
   PositionTo(anchor, anchorDirection);
}

ClickButton::~ClickButton()
{
}

void ClickButton::SetLabel(const char* label)
{
   SetName(label);
   mWidth = GetStringWidth(label)+5;
}

void ClickButton::Render()
{
   ofPushStyle();

   float w,h;
   GetDimensions(w,h);

   ofColor color,textColor;
   IUIControl::GetColors(color, textColor);

   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX+1,mY+1,w,h);
   DrawBeacon(mX+w/2, mY+h/2);
   float press = ofClamp((1 - (gTime - mClickTime) / 200), 0, 1);
   color.r = ofLerp(color.r, 0, press);
   color.g = ofLerp(color.g, 0, press);
   color.b = ofLerp(color.b, 0, press);
   ofSetColor(color);
   ofRect(mX,mY,w,h);
   ofNoFill();

   ofSetColor(textColor);
   DrawTextNormal(Name(), mX+2, mY+12);
   
   ofPopStyle();
   
   DrawHover();
}

bool ClickButton::ButtonLit()
{
   return mClickTime + 200 > gTime;
}

void ClickButton::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mClickTime = gTime;
   mOwner->ButtonClicked(this);
}

void ClickButton::MouseReleased()
{
   mClickTime = 0;
}

bool ClickButton::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   return false;
}

void ClickButton::SetFromMidiCC(float slider)
{
   if (slider > 0)
      OnClicked(0,0,false);
   else
      MouseReleased();
}

void ClickButton::SetValue(float value)
{
   if (value > 0)
      OnClicked(0,0,false);
   else
      MouseReleased();
}

float ClickButton::GetMidiValue()
{
   if (ButtonLit())
      return 1;
   return 0;
}
