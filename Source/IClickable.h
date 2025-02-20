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

#pragma once

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
   void SetPosition(double x, double y)
   {
      mX = x;
      mY = y;
   }
   void GetPosition(double& x, double& y, bool local = false) const;
   ofVec2d GetPosition(bool local = false) const;
   virtual void Move(double moveX, double moveY)
   {
      mX += moveX;
      mY += moveY;
   }
   virtual bool TestClick(double x, double y, bool right, bool testOnly = false);
   IClickable* GetParent() const { return mParent; }
   void SetParent(IClickable* parent) { mParent = parent; }
   bool NotifyMouseMoved(double x, double y);
   bool NotifyMouseScrolled(double x, double y, double scrollX, double scrollY, bool isSmoothScroll, bool isInvertedScroll);
   virtual void MouseReleased() {}
   virtual void GetDimensions(double& width, double& height)
   {
      width = 10;
      height = 10;
   }

   ofVec2d GetDimensions();
   ofRectangle GetRect(bool local = false);
   void SetName(const char* name)
   {
      if (mName != name)
         StringCopy(mName, name, MAX_TEXTENTRY_LENGTH);
   }
   const char* Name() const { return mName; }
   char* NameMutable() { return mName; }
   std::string Path(bool ignoreContext = false, bool useDisplayName = false);
   virtual bool CheckNeedsDraw();
   virtual void SetShowing(bool showing) { mShowing = showing; }
   bool IsShowing() const { return mShowing; }
   virtual void StartBeacon() { mBeaconTime = gTime; }
   double GetBeaconAmount() const;
   void DrawBeacon(double x, double y);
   IClickable* GetRootParent();
   IDrawableModule* GetModuleParent();
   void SetOverrideDisplayName(std::string name)
   {
      mHasOverrideDisplayName = true;
      mOverrideDisplayName = name;
   }
   std::string GetDisplayName()
   {
      return mHasOverrideDisplayName ? mOverrideDisplayName : mName;
   }

   static void SetLoadContext(IClickable* context) { sPathLoadContext = context->Path() + "~"; }
   static void ClearLoadContext() { sPathLoadContext = ""; }
   static void SetSaveContext(IClickable* context) { sPathSaveContext = context->Path() + "~"; }
   static void ClearSaveContext() { sPathSaveContext = ""; }

   static std::string sPathLoadContext;
   static std::string sPathSaveContext;

protected:
   virtual void OnClicked(double x, double y, bool right) {}
   virtual bool MouseMoved(double x, double y) { return false; }
   virtual bool MouseScrolled(double x, double y, double scrollX, double scrollY, bool isSmoothScroll, bool isInvertedScroll) { return false; }

   double mX{ 0 };
   double mY{ 0 };
   IClickable* mParent{ nullptr };
   bool mShowing{ true };

private:
   char mName[MAX_TEXTENTRY_LENGTH]{};
   double mBeaconTime{ -999 };
   bool mHasOverrideDisplayName{ false };
   std::string mOverrideDisplayName{ "" };
};

class IKeyboardFocusListener
{
public:
   virtual ~IKeyboardFocusListener() {}
   static void SetActiveKeyboardFocus(IKeyboardFocusListener* focus) { sCurrentKeyboardFocus = focus; }
   static IKeyboardFocusListener* GetActiveKeyboardFocus() { return sCurrentKeyboardFocus; }
   static void ClearActiveKeyboardFocus(bool notifyListeners);

   virtual void OnKeyPressed(int key, bool isRepeat) = 0;
   virtual bool ShouldConsumeKey(int key) { return true; }
   virtual bool CanTakeFocus() { return true; }

   static IKeyboardFocusListener* sKeyboardFocusBeforeClick;

private:
   virtual void AcceptEntry(bool pressedEnter) {}
   virtual void CancelEntry() {}
   static IKeyboardFocusListener* sCurrentKeyboardFocus;
};
