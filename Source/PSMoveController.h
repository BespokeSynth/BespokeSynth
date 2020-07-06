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
};

#endif /* defined(__modularSynth__PSMoveController__) */
