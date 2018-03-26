/*
  ==============================================================================

    NotePanAlternator.cpp
    Created: 25 Mar 2018 9:27:26pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NotePanAlternator.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NotePanAlternator::NotePanAlternator()
: mPanOne(-1)
, mPanOneSlider(nullptr)
, mPanTwo(1)
, mPanTwoSlider(nullptr)
, mFlip(false)
{
}

void NotePanAlternator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPanOneSlider = new FloatSlider(this,"one",4,2,100,15,&mPanOne,-1,1);
   mPanTwoSlider = new FloatSlider(this,"two",mPanOneSlider,kAnchor_Below,100,15,&mPanTwo,-1,1);
}

void NotePanAlternator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mPanOneSlider->Draw();
   mPanTwoSlider->Draw();
   
   ofPushStyle();
   ofSetColor(0, 255, 0, 50);
   ofFill();
   ofVec2f activePos = mFlip ? mPanTwoSlider->GetPosition(true) : mPanOneSlider->GetPosition(true);
   ofRect(activePos.x, activePos.y, 100, 15);
   ofPopStyle();
}

void NotePanAlternator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && velocity > 0)
   {
      modulation.pan = mFlip ? mPanTwo : mPanOne;
      mFlip = !mFlip;
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NotePanAlternator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NotePanAlternator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
