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
//  RadioButton.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#include "RadioButton.h"
#include "IDrawableModule.h"
#include "SynthGlobals.h"
#include "FileStream.h"

const int radioSpacing = 15;

//static
int RadioButton::GetSpacing()
{
   return radioSpacing;
}

RadioButton::RadioButton(IRadioButtonListener* owner, const char* name, int x, int y, int* var, RadioDirection direction /*= kRadioVertical*/)
: mVar(var)
, mOwner(owner)
, mDirection(direction)
{
   assert(owner);
   SetName(name);
   SetPosition(x, y);
   SetParent(dynamic_cast<IClickable*>(owner));

   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
}

RadioButton::RadioButton(IRadioButtonListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, RadioDirection direction /*= kRadioVertical*/)
: RadioButton(owner, name, -1, -1, var, direction)
{
   PositionTo(anchor, anchorDirection);
}

RadioButton::~RadioButton()
{
}

void RadioButton::AddLabel(const char* label, int value)
{
   RadioButtonElement element;
   element.mLabel = label;
   element.mValue = value;
   mElements.push_back(element);

   UpdateDimensions();

   CalcSliderVal();
}

void RadioButton::SetLabel(const char* label, int value)
{
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i].mValue == value)
         mElements[i].mLabel = label;
   }

   UpdateDimensions();
}

void RadioButton::RemoveLabel(int value)
{
   for (auto iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      if (iter->mValue == value)
      {
         mElements.erase(iter);
         break;
      }
   }

   UpdateDimensions();
}

void RadioButton::UpdateDimensions()
{
   if (mDirection == kRadioVertical)
   {
      for (int i = 0; i < mElements.size(); ++i)
      {
         int width = GetStringWidth(mElements[i].mLabel) + 5;
         if (width > mWidth)
            mWidth = width;
      }
      mHeight = radioSpacing * (int)mElements.size();
      mElementWidth = mWidth;
   }
   else
   {
      for (int i = 0; i < mElements.size(); ++i)
      {
         int width = GetStringWidth(mElements[i].mLabel) + 5;
         if (width > mElementWidth)
            mElementWidth = width;
      }
      mWidth = mElementWidth * (int)mElements.size();
      mHeight = radioSpacing;
   }

   if (mForcedWidth != -1)
      mWidth = mForcedWidth;
}

void RadioButton::Clear()
{
   mElements.clear();
   mWidth = 40;
   mHeight = 15;

   if (mForcedWidth != -1)
      mWidth = mForcedWidth;
}

void RadioButton::Poll()
{
   if (*mVar != mLastSetValue)
      CalcSliderVal();
}

void RadioButton::Render()
{
   ofPushStyle();

   DrawBeacon(mX + mWidth / 2, mY + mHeight / 2);

   float w, h;
   GetDimensions(w, h);
   ofFill();
   ofSetColor(0, 0, 0, gModuleDrawAlpha * .5f);
   ofRect(mX + 1, mY + 1, mWidth, mHeight);
   ofPushMatrix();
   ofClipWindow(mX, mY, mWidth, mHeight, true);
   for (int i = 0; i < mElements.size(); ++i)
   {
      ofColor color, textColor;
      IUIControl::GetColors(color, textColor);

      bool active = false;
      if (mVar)
      {
         if (mMultiSelect)
            active = (1 << mElements[i].mValue) & *mVar;
         else
            active = mElements[i].mValue == *mVar;
      }

      if (active)
      {
         float h, s, b;
         color.getHsb(h, s, b);
         color.setHsb(42, s, b);
         textColor.set(255, 255, 0, gModuleDrawAlpha);
      }

      ofFill();
      if (active)
         color.setBrightness(ofLerp(color.getBrightness(), 255, .3f));
      ofSetColor(color);

      float x, y;

      if (mDirection == kRadioVertical)
      {
         x = mX;
         y = mY + i * radioSpacing;
         ofRect(x, y, w, radioSpacing);
      }
      else
      {
         x = mX + mElementWidth * i;
         y = mY;
         ofRect(x, y, mElementWidth, radioSpacing);
      }

      ofNoFill();

      ofSetColor(textColor);
      //ofRect(mX,mY+i*radioSpacing,w,15);
      DrawTextNormal(mElements[i].mLabel, x + 2, y + 12);
   }
   ofPopMatrix();
   ofPopStyle();

   DrawHover(mX, mY, w, h);
}

bool RadioButton::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   return false;
}

void RadioButton::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   if (mDirection == kRadioVertical)
      SetIndex(y / radioSpacing);
   else //kRadioHorizontal
      SetIndex(int(x / mElementWidth));
}

ofVec2f RadioButton::GetOptionPosition(int optionIndex)
{
   float x, y;
   GetPosition(x, y, false);
   if (mDirection == kRadioVertical)
      return ofVec2f(x + mWidth, y + float(mHeight) / GetNumValues() * (optionIndex + .5f));
   else //kRadioHorizontal
      return ofVec2f(x + float(mWidth) / GetNumValues() * (optionIndex + .5f), y + mHeight);
}

void RadioButton::SetIndex(int i)
{
   if (mElements.empty())
      return;

   i = ofClamp(i, 0, mElements.size() - 1);
   int oldVal = *mVar;
   if (mMultiSelect)
      *mVar ^= 1 << mElements[i].mValue;
   else
      *mVar = mElements[i].mValue;
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->RadioButtonUpdated(this, oldVal);
      gControlTactileFeedback = 1;
   }
}

void RadioButton::SetFromMidiCC(float slider, bool setViaModulator /*= false*/)
{
   slider = ofClamp(slider, 0, 1);
   SetIndex(int(slider * mElements.size()));
   mSliderVal = slider;
   mLastSetValue = *mVar;
}

float RadioButton::GetValueForMidiCC(float slider) const
{
   if (mElements.empty())
      return 0;

   int index = int(slider * mElements.size());
   index = ofClamp(index, 0, mElements.size() - 1);
   return mElements[index].mValue;
}

void RadioButton::SetValue(float value)
{
   if (mMultiSelect)
      value = *mVar ^ (1 << (int)value);
   SetValueDirect(value);
}

void RadioButton::SetValueDirect(float value)
{
   int oldVal = *mVar;

   *mVar = (int)value;
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->RadioButtonUpdated(this, oldVal);
      gControlTactileFeedback = 1;
   }
}

float RadioButton::GetValue() const
{
   return *mVar;
}

float RadioButton::GetMidiValue() const
{
   if (mMultiSelect)
      return GetValue();

   return mSliderVal;
}

std::string RadioButton::GetDisplayValue(float val) const
{
   if (mMultiSelect)
      return "multiselect";

   int curIndex = -1;

   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i].mValue == val)
         curIndex = i;
   }

   if (curIndex >= 0 && curIndex < mElements.size())
      return mElements[curIndex].mLabel;
   else
      return "-----";
}

void RadioButton::Increment(float amount)
{
   if (mMultiSelect)
      return;

   int current = 0;
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i].mValue == *mVar)
      {
         current = i;
         break;
      }
   }

   SetIndex(current + (int)amount);
}

EnumMap RadioButton::GetEnumMap()
{
   EnumMap ret;
   for (int i = 0; i < mElements.size(); ++i)
      ret[mElements[i].mLabel] = mElements[i].mValue;
   return ret;
}

void RadioButton::CalcSliderVal()
{
   int current = -1;
   for (int i = 0; i < mElements.size(); ++i)
   {
      if (mElements[i].mValue == *mVar)
      {
         current = i;
         break;
      }
   }
   if (current != -1)
      mSliderVal = ofMap(current, 0, mElements.size(), 0, 1);
   else
      mSliderVal = -1;
   mLastSetValue = *mVar;
}

namespace
{
   const int kSaveStateRev = 0;
}

void RadioButton::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << (float)*mVar;
}

void RadioButton::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   float var;
   in >> var;
   if (shouldSetValue)
      SetValueDirect(var);
}
