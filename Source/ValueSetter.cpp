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
   mValueEntry = new TextEntry(this,"value",40,2,7,&mValue,-99999,99999);
   mValueEntry->SetDescription("value");
   
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mControlCable);
}

void ValueSetter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mValueEntry->Draw();
}

void ValueSetter::PostRepatch(PatchCableSource* cableSource)
{
   mTarget = dynamic_cast<IUIControl*>(mControlCable->GetTarget());
}

void ValueSetter::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (velocity > 0 && mEnabled)
   {
      if (mTarget)
         mTarget->SetValue(mValue);
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
