//
//  IClickable.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/3/12.
//
//

#include "IClickable.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"

IClickable::IClickable()
: mX(0)
, mY(0)
, mParent(NULL)
, mShowing(true)
, mBeaconTime(-999)
{
   mName[0] = 0;
}

void IClickable::Draw()
{
   if (!mShowing)
      return;
   
   Render();
}

bool IClickable::TestClick(int x, int y, bool right, bool testOnly /* = false */)
{
   if (!mShowing)
      return false;
   
   int w, h;
   GetDimensions(w,h);
   
   float titleBarHeight = 0;
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(this);
   if (module && module->HasTitleBar())
      titleBarHeight = IDrawableModule::TitleBarHeight();
   
   if (x >= mX && y >= mY-titleBarHeight && x <= mX+w && y <= mY+h)
   {
      if (!testOnly)
         OnClicked(x-mX, y-mY, right);
      return true;
   }
   return false;
}

bool IClickable::NotifyMouseMoved(float x, float y)
{
   if (!mShowing)
      return false;
   return MouseMoved(x-mX, y-mY);
}

bool IClickable::NotifyMouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (!mShowing)
      return false;
   return MouseScrolled(x-mX, y-mY, scrollX, scrollY);
}

void IClickable::GetPosition(int& x, int& y, bool local /*= false*/) const //TODO(Ryan) deprecated
{
   float fX, fY;
   GetPosition(fX, fY, local);
   x = (int)fX;
   y = (int)fY;
}

void IClickable::GetPosition(float& x, float& y, bool local /*= false*/) const
{
   if (mParent && !local)
   {
      mParent->GetPosition(x,y);
      x += mX;
      y += mY;
   }
   else
   {
      x = mX;
      y = mY;
   }
}

ofVec2f IClickable::GetPosition(bool local /*= false*/) const
{
   int x,y;
   GetPosition(x, y, local);
   return ofVec2f(x,y);
}

ofVec2f IClickable::GetDimensions()
{
   int w,h;
   GetDimensions(w,h);
   return ofVec2f(w,h);
}

ofRectangle IClickable::GetRect(bool local /*=false*/)
{
   int x,y,w,h;
   GetPosition(x, y, local);
   GetDimensions(w,h);
   return ofRectangle(x,y,w,h);
}

IClickable* IClickable::GetRootParent()
{
   IClickable* parent = GetParent();
   while (parent && parent->GetParent())  //keep going up until it's an IDrawableModule
      parent = parent->GetParent();
   if (parent == NULL)
      return this;   //we are the root
   return parent;
}

string IClickable::Path()
{
   if (mParent == NULL)
      return mName;
   return mParent->Path() + "~" + mName;
}

bool IClickable::CheckNeedsDraw()
{
   return mShowing;
}

float IClickable::GetBeaconAmount() const
{
   return ofClamp(((mBeaconTime + 250) - gTime) / 250, 0, 1);
}

void IClickable::DrawBeacon(int x, int y)
{
   float size = GetBeaconAmount();
   if (size > 0)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,255,0,40);
      ofCircle(x, y, size*40);
      ofSetColor(255,255,0,150);
      ofCircle(x, y, size*3);
      ofPopStyle();
   }
}
