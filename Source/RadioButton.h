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

#pragma once

#include "IPulseReceiver.h"
#include "IUIControl.h"
#include "PatchCableSource.h"

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
class DropdownList;

class IRadioButtonListener
{
public:
   virtual ~IRadioButtonListener() {}
   virtual void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) = 0;
};

class RadioButton : public IUIControl, public IPulseReceiver
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
   void CopyContentsTo(DropdownList* list) const;

   bool MouseMoved(float x, float y) override;

   static int GetSpacing();

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   void SetValueDirect(float value, double time) override;
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
   bool CanBeTargetedBy(PatchCableSource* source) const override;

   void GetDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   ofVec2f GetOptionPosition(int optionIndex);

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

protected:
   ~RadioButton(); //protected so that it can't be created on the stack

private:
   void SetIndex(int i, double time);
   void CalcSliderVal();
   void UpdateDimensions();
   void SetValueDirect(float value, double time, bool forceUpdate);

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
