/*
  ==============================================================================

    OSCOutput.cpp
    Created: 27 Sep 2018 9:36:01pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "OSCOutput.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

OSCOutput::OSCOutput()
{
   for (int i=0; i<OSC_OUTPUT_MAX_PARAMS; ++i)
   {
      mParams[i] = 0;
      mLabels[i] = new char[32];
      mLabels[i][0] = 0;
      strcat(mLabels[i], "slider");
      strcat(mLabels[i], ofToString(i).c_str());
   }
}

OSCOutput::~OSCOutput()
{
}

void OSCOutput::Init()
{
   IDrawableModule::Init();
   
   mOscOut.connect( "127.0.0.1", 56456 );
}

void OSCOutput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   for (int i=0; i<5; ++i)
   {
      mLabelEntry.push_back(new TextEntry(this, ("label"+ofToString(i)).c_str(), 5, 5 + i*15, 10, mLabels[i]));
      mSliders.push_back(new FloatSlider(this, mLabels[i], 100, 5 + i * 15, 100, 15, &mParams[i], 0, 1));
   }
}

void OSCOutput::Poll()
{
   ComputeSliders(0);
}

void OSCOutput::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (auto* entry : mLabelEntry)
      entry->Draw();
   for (auto* slider : mSliders)
      slider->Draw();
}

void OSCOutput::GetModuleDimensions(float& w, float& h)
{
   w = 200;
   h = (int)mSliders.size() * 15 + 10;
}

void OSCOutput::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   char address[120];
   address[0] = 0;
   strcat(address, "/bespoke/");
   strcat(address, slider->Name());
   OSCMessage msg(address);
   msg.addFloat32(slider->GetValue());
   mOscOut.send(msg);
}

void OSCOutput::TextEntryComplete(TextEntry* entry)
{
   int i=0;
   for (auto* iter : mLabelEntry)
   {
      if (iter == entry)
      {
         auto sliderIter = mSliders.begin();
         advance(sliderIter, i);
         (*sliderIter)->SetName(mLabels[i]);
      }
      ++i;
   }
}

void OSCOutput::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void OSCOutput::SetUpFromSaveData()
{
}

void OSCOutput::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}
