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
//  RadioButton.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#ifndef __modularSynth__RadioButton__
#define __modularSynth__RadioButton__

#include "IUIControl.h"

struct RadioButtonElement
{
   std::string mLabel;
   int mValue{ 0 };
};

enum RadioDirection
{
   kRadioVertical,
   kRadioHorizontal
};

class RadioButton;

class IRadioButtonListener
{
public:
   virtual ~IRadioButtonListener() {}
   virtual void RadioButtonUpdated(RadioButton* radio, int oldVal) = 0;
};

class RadioButton : public IUIControl
{
public:
   RadioButton(IRadioButtonListener* owner, const char* name, int x, int y, int* var, RadioDirection direction = kRadioVertical);
   RadioButton(IRadioButtonListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, RadioDirection direction = kRadioVertical);
   void AddLabel(const char* label, int value);
   void SetLabel(const char* label, int value);
   void RemoveLabel(int value);
   void Render() override;
   void SetMultiSelect(bool on) { mMultiSelect = on; }
   void Clear();
   EnumMap GetEnumMap();
   void SetForcedWidth(int width) { mForcedWidth = width; }

   bool MouseMoved(float x, float y) override;

   static int GetSpacing();

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   void SetValueDirect(float value) override;
   float GetValue() const override;
   float GetMidiValue() const override;
   int GetNumValues() override { return (int)mElements.size(); }
   std::string GetDisplayValue(float val) const override;
   bool IsBitmask() override { return mMultiSelect; }
   bool InvertScrollDirection() override { return mDirection == kRadioVertical; }
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   ofVec2f GetOptionPosition(int optionIndex);

protected:
   ~RadioButton(); //protected so that it can't be created on the stack

private:
   void SetIndex(int i);
   void CalcSliderVal();
   void UpdateDimensions();

   void OnClicked(float x, float y, bool right) override;

   int mWidth{ 15 };
   int mHeight{ 15 };
   float mElementWidth{ 8 };
   std::vector<RadioButtonElement> mElements;
   int* mVar;
   IRadioButtonListener* mOwner;
   bool mMultiSelect{ false }; //makes this... not a radio button. mVar becomes a bitmask
   RadioDirection mDirection;
   float mSliderVal{ 0 };
   int mLastSetValue{ 0 };
   int mForcedWidth{ -1 };
};

#endif /* defined(__modularSynth__RadioButton__) */
