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

class IClickable
{
public:
   IClickable();
   virtual ~IClickable() {}
   void Draw();
   virtual void Render() {}
   void SetPosition(float x, float y) { mX = x; mY = y; }
   void SetX(float x) { mX = x; }
   void SetY(float y) { mY = y; }
   void GetPosition(int& x, int& y, bool local = false) const; //TODO(Ryan) deprecated
   void GetPosition(float& x, float& y, bool local = false) const;
   ofVec2f GetPosition(bool local = false) const;
   virtual bool TestClick(int x, int y, bool right, bool testOnly = false);
   IClickable* GetParent() const { return mParent; }
   void SetParent(IClickable* parent) { mParent = parent; }
   bool NotifyMouseMoved(float x, float y);
   bool NotifyMouseScrolled(int x, int y, float scrollX, float scrollY);
   virtual void MouseReleased() {}
   virtual void GetDimensions(int& width, int& height) { width = 10; height = 10; }
   ofVec2f GetDimensions();
   ofRectangle GetRect(bool local = false);
   void SetName(const char* name) { StringCopy(mName, name, MAX_TEXTENTRY_LENGTH); }
   const char* Name() const { return mName; }
   char* NameMutable() { return mName; }
   string Path();
   virtual bool CheckNeedsDraw();
   void SetShowing(bool showing) { mShowing = showing; }
   bool IsShowing() const { return mShowing; }
   virtual void StartBeacon() { mBeaconTime = gTime; }
   float GetBeaconAmount() const;
   void DrawBeacon(int x, int y);
   IClickable* GetRootParent();
protected:
   virtual void OnClicked(int x, int y, bool right) {}
   virtual bool MouseMoved(float x, float y) { return false; }
   virtual bool MouseScrolled(int x, int y, float scrollX, float scrollY) { return false; }
   
   float mX;
   float mY;
   IClickable* mParent;
   bool mShowing;
   
private:
   char mName[MAX_TEXTENTRY_LENGTH];
   double mBeaconTime;
};

#endif /* defined(__modularSynth__IClickable__) */
