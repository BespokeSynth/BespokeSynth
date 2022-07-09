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

#ifndef __modularSynth__ClickButton__
#define __modularSynth__ClickButton__

#include "IUIControl.h"

class ClickButton;

class IButtonListener
{
public:
   virtual ~IButtonListener() {}
   virtual void ButtonClicked(ClickButton* button) = 0;
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
   kArrowLeft
};

class ClickButton : public IUIControl
{
public:
   ClickButton(IButtonListener* owner, const char* label, int x, int y, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   ClickButton(IButtonListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   void SetLabel(const char* label);
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
   void SetFromMidiCC(float slider, bool setViaModulator = false) override;
   void SetValue(float value) override;
   float GetValue() const override { return GetMidiValue(); }
   float GetMidiValue() const override;
   std::string GetDisplayValue(float val) const override;
   int GetNumValues() override { return 2; }
   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void SaveState(FileStreamOut& out) override {}
   void LoadState(FileStreamIn& in, bool shouldSetValue) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }

protected:
   ~ClickButton(); //protected so that it can't be created on the stack

private:
   bool ButtonLit() const;

   void OnClicked(float x, float y, bool right) override;
   float mWidth;
   float mHeight;
   double mClickTime;
   IButtonListener* mOwner;
   ButtonDisplayStyle mDisplayStyle;
};

#endif /* defined(__modularSynth__ClickButton__) */
