/*
  ==============================================================================

    PitchPanner.cpp
    Created: 25 Mar 2018 9:57:24am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PitchPanner.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

PitchPanner::PitchPanner()
: mPitchLeft(36)
, mPitchLeftSlider(nullptr)
, mPitchRight(96)
, mPitchRightSlider(nullptr)
{
}

void PitchPanner::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPitchLeftSlider = new IntSlider(this,"left",4,2,100,15,&mPitchLeft,0,127);
   mPitchRightSlider = new IntSlider(this,"right",mPitchLeftSlider,kAnchor_Below,100,15,&mPitchRight,0,127);
}

void PitchPanner::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPitchLeftSlider->Draw();
   mPitchRightSlider->Draw();
}

void PitchPanner::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
      modulation.pan = ofMap(pitch, mPitchLeft, mPitchRight, -1, 1);
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void PitchPanner::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PitchPanner::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
