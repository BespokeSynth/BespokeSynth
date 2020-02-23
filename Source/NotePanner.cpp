/*
  ==============================================================================

    NotePanner.cpp
    Created: 24 Mar 2018 8:18:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NotePanner.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NotePanner::NotePanner()
: mPan(0)
, mPanSlider(nullptr)
{
}

void NotePanner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanSlider = new FloatSlider(this,"pan",4,2,100,15,&mPan,-1,1);
}

void NotePanner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPanSlider->Draw();
}

void NotePanner::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      ComputeSliders(0);
      modulation.pan = mPan;
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NotePanner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NotePanner::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
