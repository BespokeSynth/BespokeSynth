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

#ifndef __modularSynth__PSMoveController__
#define __modularSynth__PSMoveController__

#include <iostream>
#include "IDrawableModule.h"
#include "psmove/PSMoveMgr.h"
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
   
   string GetTitleLabel() override { return "ps move"; }
   void CreateUIControls() override;
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void Poll() override;
   void Exit() override;

   void SetPitchControl(IUIControl* control) { mPitchUIControl = control; }
   void SetYawControl(IUIControl* control) { mYawUIControl = control; }
   void SetRollControl(IUIControl* control) { mRollUIControl = control; }
   void SetEnergyControl(IUIControl* control) { mEnergyUIControl = control; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   //ITimeListener
   void OnTimeEvent(double time) override;
   //IFloatSliderLIstener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 152; height = 140; }
   bool Enabled() const override { return mEnabled; }

   PSMoveMgr mMoveMgr;
   Ramp mVibration;
   bool mVibronomeOn;
   Checkbox* mVibronomeCheckbox;
   ClickButton* mConnectButton;
   float mMetronomeLagOffset;
   FloatSlider* mOffsetSlider;
   float mRoll;
   float mPitch;
   float mYaw;
   float mEnergy;
   FloatSlider* mPitchSlider;
   FloatSlider* mYawSlider;
   FloatSlider* mRollSlider;
   FloatSlider* mEnergySlider;
   ClickButton* mBindPitch;
   ClickButton* mBindYaw;
   ClickButton* mBindRoll;
   ClickButton* mBindEnergy;
   IUIControl* mPitchUIControl;
   IUIControl* mYawUIControl;
   IUIControl* mRollUIControl;
   IUIControl* mEnergyUIControl;
   
   bool mPSButtonDown;

   TransportListenerInfo* mTransportListenerInfo;
};

#endif /* defined(__modularSynth__PSMoveController__) */
