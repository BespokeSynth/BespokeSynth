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
//  IClickable.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/3/12.
//
//

#ifndef __modularSynth__IClickable__
#define __modularSynth__IClickable__

#include "SynthGlobals.h"

//TODO(Ryan) factor Transformable stuff out of here

class IDrawableModule;

class IClickable
{
public:
   IClickable();
   virtual ~IClickable() {}
   void Draw();
   virtual void Render() {}
   void SetPosition(float x, float y)
   {
      mX = x;
      mY = y;
   }
   void GetPosition(float& x, float& y, bool local = false) const;
   ofVec2f GetPosition(bool local = false) const;
   virtual void Move(float moveX, float moveY)
   {
      mX += moveX;
      mY += moveY;
   }
   virtual bool TestClick(float x, float y, bool right, bool testOnly = false);
   IClickable* GetParent() const { return mParent; }
   void SetParent(IClickable* parent) { mParent = parent; }
   bool NotifyMouseMoved(float x, float y);
   bool NotifyMouseScrolled(float x, float y, float scrollX, float scrollY);
   virtual void MouseReleased() {}
   virtual void GetDimensions(float& width, float& height)
   {
      width = 10;
      height = 10;
   }
   ofVec2f GetDimensions();
   ofRectangle GetRect(bool local = false);
   void SetName(const char* name)
   {
      if (mName != name)
         StringCopy(mName, name, MAX_TEXTENTRY_LENGTH);
   }
   const char* Name() const { return mName; }
   char* NameMutable() { return mName; }
   std::string Path(bool ignoreContext = false);
   virtual bool CheckNeedsDraw();
   virtual void SetShowing(bool showing) { mShowing = showing; }
   bool IsShowing() const { return mShowing; }
   virtual void StartBeacon() { mBeaconTime = gTime; }
   float GetBeaconAmount() const;
   void DrawBeacon(int x, int y);
   IClickable* GetRootParent();
   IDrawableModule* GetModuleParent();

   static void SetLoadContext(IClickable* context) { sPathLoadContext = context->Path() + "~"; }
   static void ClearLoadContext() { sPathLoadContext = ""; }
   static void SetSaveContext(IClickable* context) { sPathSaveContext = context->Path() + "~"; }
   static void ClearSaveContext() { sPathSaveContext = ""; }

   static std::string sPathLoadContext;
   static std::string sPathSaveContext;

protected:
   virtual void OnClicked(float x, float y, bool right) {}
   virtual bool MouseMoved(float x, float y) { return false; }
   virtual bool MouseScrolled(float x, float y, float scrollX, float scrollY) { return false; }

   float mX;
   float mY;
   IClickable* mParent;
   bool mShowing;

private:
   char mName[MAX_TEXTENTRY_LENGTH];
   double mBeaconTime;
};

#endif /* defined(__modularSynth__IClickable__) */
