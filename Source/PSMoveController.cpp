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
//  PSMoveController.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/18/13.
//
//

#include "PSMoveController.h"
#include "SynthGlobals.h"
#include "psmove/psmove.h"
#include "ModularSynth.h"

PSMoveController::PSMoveController()
{
   mMoveMgr.Setup();

   mVibration.SetValue(0);
}

void PSMoveController::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, kInterval_4n, OffsetInfo(mMetronomeLagOffset, false), false);
}

void PSMoveController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mConnectButton = new ClickButton(this, "connect", 5, 35);
   mVibronomeCheckbox = new Checkbox(this, "metronome", 5, 20, &mVibronomeOn);
   mOffsetSlider = new FloatSlider(this, "offset", 5, 54, 100, 15, &mMetronomeLagOffset, -100, 100);
   mPitchSlider = new FloatSlider(this, "pitch", 5, 73, 90, 15, &mPitch, 0, 1);
   mYawSlider = new FloatSlider(this, "yaw", 5, 89, 90, 15, &mYaw, 0, 1);
   mRollSlider = new FloatSlider(this, "roll", 5, 105, 90, 15, &mRoll, 0, 1);
   mEnergySlider = new FloatSlider(this, "energy", 5, 121, 90, 15, &mEnergy, 0, 1);
   mBindPitch = new ClickButton(this, "bindp", 100, 73);
   mBindYaw = new ClickButton(this, "bindy", 100, 89);
   mBindRoll = new ClickButton(this, "bindr", 100, 105);
   mBindEnergy = new ClickButton(this, "binde", 100, 121);
}

PSMoveController::~PSMoveController()
{
   TheTransport->RemoveListener(this);
}

void PSMoveController::Poll()
{
   if (!mEnabled)
      return;

   mMoveMgr.Update();
   mMoveMgr.SetVibration(0, mVibration.Value(gTime));

   ofVec3f gyros(0, 0, 0);
   mMoveMgr.GetGyros(0, gyros);

   bool isButtonDown = false;
   if (mMoveMgr.IsButtonDown(0, Btn_MOVE))
   {
      mPitch = ofClamp(mPitch + gyros.x / 50000, 0, 1);
      if (mPitchUIControl)
         mPitchUIControl->SetFromMidiCC(mPitch, NextBufferTime(false), false);
      isButtonDown = true;
   }
   if (mMoveMgr.IsButtonDown(0, Btn_SQUARE))
   {
      mYaw = ofClamp(mYaw - gyros.z / 50000, 0, 1);
      if (mYawUIControl)
         mYawUIControl->SetFromMidiCC(mYaw, NextBufferTime(false), false);
      isButtonDown = true;
   }
   if (mMoveMgr.IsButtonDown(0, Btn_T))
   {
      mRoll = ofClamp(mRoll + gyros.y / 80000, 0, 1);
      if (mRollUIControl)
         mRollUIControl->SetFromMidiCC(mRoll, NextBufferTime(false), false);
      isButtonDown = true;
   }
   if (isButtonDown)
      mMoveMgr.SetColor(0, mRoll, mYaw, mPitch);
   else
      mMoveMgr.SetColor(0, 0, 0, 0);

   if (mMoveMgr.IsButtonDown(0, Btn_PS))
   {
      if (!mPSButtonDown)
      {
         mPSButtonDown = true;
         mVibronomeOn = !mVibronomeOn;
      }
   }
   else
   {
      mPSButtonDown = false;
   }

   ofVec3f accel(0, 0, 0);
   mMoveMgr.GetAccel(0, accel);
   mEnergy = ofClamp(accel.length() / 5000 - .8f, 0, 1);
   if (mEnergyUIControl)
      mEnergyUIControl->SetFromMidiCC(mEnergy, NextBufferTime(false), false);
}

void PSMoveController::Exit()
{
   IDrawableModule::Exit();
   mMoveMgr.Exit();
}

void PSMoveController::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mVibronomeCheckbox->Draw();
   mConnectButton->Draw();
   mOffsetSlider->Draw();
   mPitchSlider->Draw();
   mYawSlider->Draw();
   mRollSlider->Draw();
   mEnergySlider->Draw();
   mBindPitch->Draw();
   mBindYaw->Draw();
   mBindRoll->Draw();
   mBindEnergy->Draw();


   DrawTextNormal("b: " + ofToString(mMoveMgr.GetBattery(0), 1), 80, 14);
}

void PSMoveController::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void PSMoveController::ButtonClicked(ClickButton* button, double time)
{
   if (button == mConnectButton)
      mMoveMgr.AddMoves();
   if (button == mBindPitch)
   {
      mPitchUIControl = gBindToUIControl;
      gBindToUIControl = nullptr;
   }
   if (button == mBindYaw)
   {
      mYawUIControl = gBindToUIControl;
      gBindToUIControl = nullptr;
   }
   if (button == mBindRoll)
   {
      mRollUIControl = gBindToUIControl;
      gBindToUIControl = nullptr;
   }
   if (button == mBindEnergy)
   {
      mEnergyUIControl = gBindToUIControl;
      gBindToUIControl = nullptr;
   }
}

void PSMoveController::OnTimeEvent(double time)
{
   if (mVibronomeOn)
   {
      float length = 100;
      if (TheTransport->GetQuantized(time, mTransportListenerInfo) == 0)
         length = 200;
      mVibration.Start(1, 0, length);
      mMoveMgr.SetVibration(0, 1);
   }
}

void PSMoveController::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mOffsetSlider)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
      {
         transportListenerInfo->mInterval = kInterval_4n;
         transportListenerInfo->mOffsetInfo = OffsetInfo(mMetronomeLagOffset, true);
      }
   }
}

void PSMoveController::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("pitchcontrol", moduleInfo);
   mModuleSaveData.LoadString("yawcontrol", moduleInfo);
   mModuleSaveData.LoadString("rollcontrol", moduleInfo);
   mModuleSaveData.LoadString("energycontrol", moduleInfo);

   SetUpFromSaveData();
}

void PSMoveController::SetUpFromSaveData()
{
   SetPitchControl(TheSynth->FindUIControl(mModuleSaveData.GetString("pitchcontrol")));
   SetYawControl(TheSynth->FindUIControl(mModuleSaveData.GetString("yawcontrol")));
   SetRollControl(TheSynth->FindUIControl(mModuleSaveData.GetString("rollcontrol")));
   SetEnergyControl(TheSynth->FindUIControl(mModuleSaveData.GetString("energycontrol")));
}
