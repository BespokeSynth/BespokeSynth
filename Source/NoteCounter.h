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
/*
  ==============================================================================

    NoteCounter.h
    Created: 24 Apr 2021 3:47:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"

class NoteCounter : public IDrawableModule, public INoteSource, public ITimeListener, public IIntSliderListener, public IDropdownListener
{
public:
   NoteCounter();
   ~NoteCounter();
   static IDrawableModule* Create() { return new NoteCounter(); }
   
   
   void CreateUIControls() override;
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   int mStart;
   IntSlider* mStartSlider;
   int mLength;
   IntSlider* mLengthSlider;
   int mStep;
   Checkbox* mSyncCheckbox;
   bool mSync;
   int mCustomDivisor;
   IntSlider* mCustomDivisorSlider;
   bool mRandom;
   Checkbox* mRandomCheckbox;
   
   TransportListenerInfo* mTransportListenerInfo;
};
