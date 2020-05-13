//
//  ValueSetter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#include "ValueSetter.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"
#include "UIControlMacros.h"

ValueSetter::ValueSetter()
: mControlCable(nullptr)
, mValue(0)
, mValueEntry(nullptr)
{
}

ValueSetter::~ValueSetter()
{
}

void ValueSetter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UICONTROL_CUSTOM(mValueEntry, new TextEntry(UICONTROL_BASICS("value"),7,&mValue,-99999,99999); mValueEntry->DrawLabel(true););
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mButton, new ClickButton(UICONTROL_BASICS("set")));
   ENDUIBLOCK(mWidth, mHeight);
   
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mControlCable);
}

void ValueSetter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValueEntry->Draw();
   mButton->Draw();
}

void ValueSetter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void ValueSetter::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled)
   {
      Go();
   }
}

void ValueSetter::ButtonClicked(ClickButton* button)
{
   if (button == mButton)
      Go();
}

void ValueSetter::Go()
{
   if (mTarget)
   {
      mTarget->SetValue(mValue);
      mControlCable->AddHistoryEvent(gTime, true);
      mControlCable->AddHistoryEvent(gTime + 15, false);
   }
}

void ValueSetter::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void ValueSetter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ValueSetter::SetUpFromSaveData()
{
   mTarget = TheSynth->FindUIControl(mModuleSaveData.GetString("target"));
   mControlCable->SetTarget(mTarget);
}
