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
//  IUIControl.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/13/13.
//
//

#pragma once

#include "IClickable.h"
#include "SynthGlobals.h"

class FileStreamIn;
class FileStreamOut;
class PatchCableSource;

#define HIDDEN_UICONTROL 9999

enum AnchorDirection
{
   kAnchor_Below,
   kAnchor_Right,
   kAnchor_Right_Padded
};

class IUIControl : public IClickable
{
public:
   IUIControl()
   {}
   virtual void Delete() { delete this; }
   void AddRemoteController() { ++mRemoteControlCount; }
   void RemoveRemoteController() { --mRemoteControlCount; }
   virtual void SetFromMidiCC(float slider, double time, bool setViaModulator) = 0;
   virtual float GetValueForMidiCC(float slider) const { return 0; }
   virtual void SetValue(float value, double time, bool forceUpdate = false) = 0;
   virtual void SetValueDirect(float value, double time) { SetValue(value, time); } //override if you need special control here
   virtual float GetValue() const { return 0; }
   virtual float GetMidiValue() const { return 0; }
   virtual int GetNumValues() { return 0; } //the number of distinct values that you can have for this control, zero indicates infinite (like a float slider)
   virtual std::string GetDisplayValue(float val) const { return "unimplemented"; }
   virtual void Init() {}
   virtual void Poll() {}
   virtual void KeyPressed(int key, bool isRepeat) {}
   void StartBeacon() override;
   bool IsPreset();
   virtual void GetRange(float& min, float& max)
   {
      min = 0;
      max = 1;
   }
   virtual bool IsBitmask() { return false; }
   bool TestHover(int x, int y);
   void CheckHover(int x, int y);
   void DrawHover(float x, float y, float w, float h);
   void DrawPatchCableHover();
   virtual bool CanBeTargetedBy(PatchCableSource* source) const;
   virtual bool InvertScrollDirection() { return false; }
   virtual void Double() {}
   virtual void Halve() {}
   virtual void ResetToOriginal() {}
   virtual void Increment(float amount) {}
   void SetCableTargetable(bool targetable) { mCableTargetable = targetable; }
   bool GetCableTargetable() const { return mCableTargetable; }
   void SetNoHover(bool noHover) { mNoHover = noHover; }
   virtual bool GetNoHover() const { return mNoHover; }
   virtual bool AttemptTextInput() { return false; }
   void PositionTo(IUIControl* anchor, AnchorDirection direction);
   void GetColors(ofColor& color, ofColor& textColor);
   bool GetShouldSaveState() const { return mShouldSaveState; }
   void SetShouldSaveState(bool save) { mShouldSaveState = save; }
   void RemoveFromOwner();
   virtual bool IsSliderControl() { return true; }
   virtual bool IsButtonControl() { return false; }
   virtual bool IsMouseDown() const { return false; }
   virtual bool IsTextEntry() const { return false; }
   virtual bool ModulatorUsesLiteralValue() const { return false; }
   virtual float GetModulationRangeMin() const { return 0; }
   virtual float GetModulationRangeMax() const { return 1; }
   virtual bool ShouldSerializeForSnapshot() const { return false; }

   static void SetNewManualHoverViaTab(int direction);
   static void SetNewManualHoverViaArrow(ofVec2f direction);
   static bool WasLastHoverSetManually() { return sLastUIHoverWasSetManually; }

   static void DestroyCablesTargetingControls(std::vector<IUIControl*> controls);

   virtual void SaveState(FileStreamOut& out) = 0;
   virtual void LoadState(FileStreamIn& in, bool shouldSetValue = true) = 0;

protected:
   virtual ~IUIControl();

   int mRemoteControlCount{ 0 };
   bool mCableTargetable{ true };
   bool mNoHover{ false };
   bool mShouldSaveState{ true };

   static IUIControl* sLastHoveredUIControl;
   static bool sLastUIHoverWasSetManually;
};
