/*
  ==============================================================================

    NoteChance.cpp
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteChance.h"
#include "SynthGlobals.h"

NoteChance::NoteChance()
: mChance(1)
, mLastRejectTime(0)
, mLastAcceptTime(0)
{
}

NoteChance::~NoteChance()
{
}

void NoteChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mChanceSlider = new FloatSlider(this, "chance", 3, 2, 100, 15, &mChance, 0, 1);
}

void NoteChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mChanceSlider->Draw();
   
   if (gTime - mLastAcceptTime > 0 && gTime - mLastAcceptTime < 200)
   {
      ofPushStyle();
      ofSetColor(0,255,0,255*(1-(gTime - mLastAcceptTime)/200));
      ofFill();
      ofRect(106,2,10,7);
      ofPopStyle();
   }
   
   if (gTime - mLastRejectTime > 0 && gTime - mLastRejectTime < 200)
   {
      ofPushStyle();
      ofSetColor(255,0,0,255*(1-(gTime - mLastRejectTime)/200));
      ofFill();
      ofRect(106,9,10,7);
      ofPopStyle();
   }
}

void NoteChance::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
      ComputeSliders(0);
   
   bool accept = ofRandom(1) <= mChance;
   if (accept || velocity == 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (velocity > 0)
   {
      if (accept)
         mLastAcceptTime = time;
      else
         mLastRejectTime = time;
   }
}

void NoteChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = 20;
}

void NoteChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
