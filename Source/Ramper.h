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
//  Ramper.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__Ramper__
#define __Bespoke__Ramper__

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "IPulseReceiver.h"

class PatchCableSource;

class Ramper : public IDrawableModule, public IDropdownListener, public IAudioPoller, public IButtonListener, public IFloatSliderListener, public IPulseReceiver
{
public:
   Ramper();
   ~Ramper();
   static IDrawableModule* Create() { return new Ramper(); }

   void CreateUIControls() override;
   void Init() override;

   //IDrawableModule
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   void Go(double time);

   std::array<IUIControl*, 16> mUIControls{ nullptr };
   NoteInterval mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   ClickButton* mTriggerButton;
   FloatSlider* mTargetValueSlider;
   float mTargetValue;
   float mStartMeasure;
   float mStartValue;
   bool mRamping;
};

#endif /* defined(__Bespoke__Ramper__) */
