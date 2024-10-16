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
//  ClickButton.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#pragma once

#include "IPulseReceiver.h"
#include "IUIControl.h"

class ClickButton;

class IButtonListener
{
public:
   virtual ~IButtonListener() {}
   virtual void ButtonClicked(ClickButton* button, double time) = 0;
};

enum class ButtonDisplayStyle
{
   kText,
   kNoLabel,
   kPlay,
   kPause,
   kStop,
   kGrabSample,
   kSampleIcon,
   kFolderIcon,
   kArrowRight,
   kArrowLeft,
   kPlus,
   kMinus,
   kHamburger
};

class ClickButton : public IUIControl, public IPulseReceiver
{
public:
   ClickButton(IButtonListener* owner, const char* label, int x, int y, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   ClickButton(IButtonListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   void SetLabel(const char* label);
   void UpdateWidth();
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   void SetDisplayText(bool display) { mDisplayStyle = ButtonDisplayStyle::kNoLabel; }
   void SetDisplayStyle(ButtonDisplayStyle style) { mDisplayStyle = style; }
   void SetDimensions(float width, float height)
   {
      mWidth = width;
      mHeight = height;
   }

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   float GetValue() const override { return GetMidiValue(); }
   float GetMidiValue() const override;
   std::string GetDisplayValue(float val) const override;
   int GetNumValues() override { return 2; }
   void Increment(float amount) override;
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void SaveState(FileStreamOut& out) override {}
   void LoadState(FileStreamIn& in, bool shouldSetValue) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }

   bool CanBeTargetedBy(PatchCableSource* source) const override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

protected:
   ~ClickButton(); //protected so that it can't be created on the stack

private:
   void DoClick(double time);
   bool ButtonLit() const;

   void OnClicked(float x, float y, bool right) override;
   float mWidth{ 20 };
   float mHeight{ 15 };
   double mClickTime{ -9999 };
   IButtonListener* mOwner{ nullptr };
   ButtonDisplayStyle mDisplayStyle{ ButtonDisplayStyle::kText };
};
