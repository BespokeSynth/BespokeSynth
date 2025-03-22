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
//  PSMoveController.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/18/13.
//
//

#pragma once

#include "IDrawableModule.h"
#include "PSMoveMgr.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "Ramp.h"
#include "Transport.h"
#include "Slider.h"

class IUIControl;

class PSMoveController : public IDrawableModule, public IButtonListener, public ITimeListener, public IFloatSliderListener
{
public:
   PSMoveController();
   ~PSMoveController();
   static IDrawableModule* Create() { return new PSMoveController(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void Poll() override;
   void Exit() override;

   void SetPitchControl(IUIControl* control) { mPitchUIControl = control; }
   void SetYawControl(IUIControl* control) { mYawUIControl = control; }
   void SetRollControl(IUIControl* control) { mRollUIControl = control; }
   void SetEnergyControl(IUIControl* control) { mEnergyUIControl = control; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;
   //ITimeListener
   void OnTimeEvent(double time) override;
   //IFloatSliderLIstener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 152;
      height = 140;
   }

   PSMoveMgr mMoveMgr;
   Ramp mVibration;
   bool mVibronomeOn{ false };
   Checkbox* mVibronomeCheckbox{ nullptr };
   ClickButton* mConnectButton{ nullptr };
   float mMetronomeLagOffset{ 50 };
   FloatSlider* mOffsetSlider{ nullptr };
   float mRoll{ .5 };
   float mPitch{ .5 };
   float mYaw{ 0 };
   float mEnergy{ 0 };
   FloatSlider* mPitchSlider{ nullptr };
   FloatSlider* mYawSlider{ nullptr };
   FloatSlider* mRollSlider{ nullptr };
   FloatSlider* mEnergySlider{ nullptr };
   ClickButton* mBindPitch{ nullptr };
   ClickButton* mBindYaw{ nullptr };
   ClickButton* mBindRoll{ nullptr };
   ClickButton* mBindEnergy{ nullptr };
   IUIControl* mPitchUIControl{ nullptr };
   IUIControl* mYawUIControl{ nullptr };
   IUIControl* mRollUIControl{ nullptr };
   IUIControl* mEnergyUIControl{ nullptr };

   bool mPSButtonDown{ false };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
