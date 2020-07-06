/*
  ==============================================================================

    PulseChance.cpp
    Created: 4 Feb 2020 12:17:59pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseChance.h"
#include "SynthGlobals.h"

PulseChance::PulseChance()
: mChance(1)
, mLastRejectTime(0)
, mLastAcceptTime(0)
{
}

PulseChance::~PulseChance()
{
}

void PulseChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mChanceSlider = new FloatSlider(this, "chance", 3, 2, 100, 15, &mChance, 0, 1);
}

void PulseChance::DrawModule()
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

void PulseChance::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);
   
   bool accept = ofRandom(1) <= mChance;
   if (accept)
      DispatchPulse(GetPatchCableSource(), time, velocity, flags);
   
   if (accept)
      mLastAcceptTime = gTime;
   else
      mLastRejectTime = gTime;
}

void PulseChance::GetModuleDimensions(float& width, float& height)
{
   width = 118;
   height = 20;
}

void PulseChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
