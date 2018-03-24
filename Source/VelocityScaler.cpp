//
//  VelocityScaler.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/6/16.
//
//

#include "VelocityScaler.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

VelocityScaler::VelocityScaler()
: mScale(1)
, mScaleSlider(nullptr)
{
}

void VelocityScaler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mScaleSlider = new FloatSlider(this,"scale",4,2,100,15,&mScale,0,2);
}

void VelocityScaler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mScaleSlider->Draw();
}

void VelocityScaler::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      ComputeSliders(0);
      if (velocity > 0)
         velocity = MAX(1,velocity*mScale);
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void VelocityScaler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void VelocityScaler::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
