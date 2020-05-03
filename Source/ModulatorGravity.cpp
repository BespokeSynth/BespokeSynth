/*
  ==============================================================================

    ModulatorGravity.cpp
    Created: 30 Apr 2020 3:56:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorGravity.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "MathUtils.h"
#include "UIControlMacros.h"

ModulatorGravity::ModulatorGravity()
: mValue(0)
, mGravity(-.1f)
, mKickAmount(1)
, mDrag(.005f)
, mGravitySlider(nullptr)
, mKickAmountSlider(nullptr)
, mDragSlider(nullptr)
, mKickButton(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

void ModulatorGravity::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   FLOATSLIDER(mGravitySlider, "gravity", &mGravity, -1, 1);
   FLOATSLIDER(mKickAmountSlider, "kick amt", &mKickAmount, -5, 5);
   FLOATSLIDER(mDragSlider, "drag", &mDrag, 0, .01f);
   BUTTON(mKickButton, "kick");
   ENDUIBLOCK(mWidth, mHeight);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
}

ModulatorGravity::~ModulatorGravity()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorGravity::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGravitySlider->Draw();
   mKickAmountSlider->Draw();
   mDragSlider->Draw();
   mKickButton->Draw();
}

void ModulatorGravity::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void ModulatorGravity::OnTransportAdvanced(float amount)
{
   float dt = amount * TheTransport->MsPerBar();
   float newVelocity = mVelocity + mGravity / 100000 * dt;
   newVelocity -= newVelocity * mDrag * dt;
   float newValue = ofClamp(mValue + newVelocity * dt, 0, 1);
   mVelocity = (newValue - mValue) / dt;
   mValue = newValue;
}

float ModulatorGravity::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   //return ofClamp(mRamp.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax());
   return ofLerp(GetMin(), GetMax(), mValue); //TODO(integrate over samples)
}

void ModulatorGravity::ButtonClicked(ClickButton* button)
{
   if (button == mKickButton)
      mVelocity += mKickAmount / 1000;
}

void ModulatorGravity::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ModulatorGravity::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModulatorGravity::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}
