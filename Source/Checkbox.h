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
//  Checkbox.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#ifndef __modularSynth__Checkbox__
#define __modularSynth__Checkbox__

#include <iostream>
#include "IUIControl.h"

class Checkbox;

class Checkbox : public IUIControl
{
public:
   Checkbox(IDrawableModule* owner, const char* label, int x, int y, bool* var);
   Checkbox(IDrawableModule* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, bool* var);
   void SetLabel(const char* label);
   void SetVar(bool* var) { mVar = var; }
   void Render() override;
   void SetDisplayText(bool display);
   void UseCircleLook(ofColor color);
   void DisableCircleLook() { mUseCircleLook = false; }

   bool MouseMoved(float x, float y) override;

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   float GetMidiValue() const override;
   int GetNumValues() override { return 2; }
   std::string GetDisplayValue(float val) const override;
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }
   void SetBoxSize(float size) { mHeight = size; }

   bool CheckNeedsDraw() override;

protected:
   ~Checkbox(); //protected so that it can't be created on the stack

private:
   void OnClicked(float x, float y, bool right) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void CalcSliderVal();
   void UpdateWidth();

   float mWidth;
   float mHeight;
   bool* mVar;
   IDrawableModule* mOwner;
   bool mLastDisplayedValue;
   bool mDisplayText;
   bool mUseCircleLook;
   ofColor mCustomColor;
   float mSliderVal;
   bool mLastSetValue;
};

#endif /* defined(__modularSynth__Checkbox__) */
