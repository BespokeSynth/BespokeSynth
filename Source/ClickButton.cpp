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

   int w,h;
   GetDimensions(w,h);
   
   DrawBeacon(mX+w/2, mY+h/2);
   DrawHover();

   ofColor color;
   if (ButtonLit())
      color.set(255,255,0,gModuleDrawAlpha);
   else
      color.set(255,255,255,gModuleDrawAlpha);

   ofFill();
   ofSetColor(color.r,color.g,color.b,color.a*.2f);
   ofRect(mX,mY,w,h);
   ofNoFill();

   ofSetColor(color);
   DrawText(Name(), mX+2, mY+12);

   ofPopStyle();
}

bool ClickButton::ButtonLit()
{
   return mClickTime + 400 > gTime;
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
