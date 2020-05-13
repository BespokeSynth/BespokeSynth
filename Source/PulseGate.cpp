/*
  ==============================================================================

    PulseGate.cpp
    Created: 22 Feb 2020 10:39:40pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseGate.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

PulseGate::PulseGate()
: mAllow(true)
{
}

PulseGate::~PulseGate()
{
}

void PulseGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   CHECKBOX(mAllowCheckbox,"allow",&mAllow);
   ENDUIBLOCK(mWidth,mHeight);
}

void PulseGate::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mAllowCheckbox->Draw();
}

void PulseGate::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);
   
   if (mAllow)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PulseGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseGate::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
