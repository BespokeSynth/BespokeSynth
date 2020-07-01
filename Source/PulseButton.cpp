/*
  ==============================================================================

    PulseButton.cpp
    Created: 20 Jun 2020 2:46:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseButton.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

PulseButton::PulseButton()
{
}

PulseButton::~PulseButton()
{
}

void PulseButton::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   BUTTON(mButton,"pulse");
   ENDUIBLOCK(mWidth,mHeight);
}

void PulseButton::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mButton->Draw();
}

void PulseButton::ButtonClicked(ClickButton* button)
{
   if (button == mButton)
      DispatchPulse(GetPatchCableSource(), gTime, 1, 0);
}

void PulseButton::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseButton::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
