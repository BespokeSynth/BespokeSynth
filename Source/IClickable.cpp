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
//  IClickable.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/3/12.
//
//

#include "IClickable.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "Prefab.h"

#include <cstring>

std::string IClickable::sPathLoadContext = "";
std::string IClickable::sPathSaveContext = "";

IClickable::IClickable()
: mX(0)
, mY(0)
, mParent(nullptr)
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

bool IClickable::TestClick(float x, float y, bool right, bool testOnly /* = false */)
{
   if (!mShowing)
      return false;

   float w, h;
   GetDimensions(w, h);

   float titleBarHeight = 0;
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(this);
   if (module && module->HasTitleBar())
      titleBarHeight = IDrawableModule::TitleBarHeight();

   if (x >= mX && y >= mY - titleBarHeight && x <= mX + w && y <= mY + h)
   {
      if (!testOnly)
         OnClicked(x - mX, y - mY, right);
      return true;
   }
   return false;
}

bool IClickable::NotifyMouseMoved(float x, float y)
{
   if (!mShowing)
      return false;
   return MouseMoved(x - mX, y - mY);
}

bool IClickable::NotifyMouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (!mShowing)
      return false;
   return MouseScrolled(x - mX, y - mY, scrollX, scrollY);
}

void IClickable::GetPosition(float& x, float& y, bool local /*= false*/) const
{
   if (mParent && !local)
   {
      mParent->GetPosition(x, y, false);
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
   float x, y;
   GetPosition(x, y, local);
   return ofVec2f(x, y);
}

ofVec2f IClickable::GetDimensions()
{
   float w, h;
   GetDimensions(w, h);
   return ofVec2f(w, h);
}

ofRectangle IClickable::GetRect(bool local /*=false*/)
{
   float x, y, w, h;
   GetPosition(x, y, local);
   GetDimensions(w, h);
   return ofRectangle(x, y, w, h);
}

IClickable* IClickable::GetRootParent()
{
   IClickable* parent = this;
   while (parent->GetParent()) //keep going up until there are no parents
      parent = parent->GetParent();
   return parent;
}

IDrawableModule* IClickable::GetModuleParent()
{
   IClickable* parent = this;
   while (parent->GetParent()) //keep going up until there are no parents
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(parent);
      if (module && module->CanMinimize()) //"minimizable" modules doesn't include effects in effectchains, which we want to avoid
         return module;
      parent = parent->GetParent();
   }
   return dynamic_cast<IDrawableModule*>(parent);
}

std::string IClickable::Path(bool ignoreContext)
{
   if (mName[0] == 0) //must have a name
      return "";

   std::string path = mName;
   if (mParent != nullptr)
      path = mParent->Path(true) + "~" + mName;

   if (!ignoreContext)
   {
      if (sPathLoadContext != "")
      {
         if (path[0] == '$')
         {
            if (Prefab::sLoadingPrefab)
               path = "";
            else
               path = path.substr(1, path.length() - 1);
         }
         else
         {
            path = sPathLoadContext + path;
         }
      }
      if (sPathSaveContext != "")
      {
         if (strstr(path.c_str(), sPathSaveContext.c_str()) == path.c_str()) //path starts with sSaveContext
            path = path.substr(sPathSaveContext.length(), path.length() - sPathSaveContext.length());
         else
            path = "$" + path; //path is outside of our context, and therefore invalid
      }
   }
   return path;
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
      ofSetColor(255, 255, 0, 40);
      ofCircle(x, y, size * 40);
      ofSetColor(255, 255, 0, 150);
      ofCircle(x, y, size * 3);
      ofPopStyle();
   }
}
