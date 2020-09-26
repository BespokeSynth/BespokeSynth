/*
  ==============================================================================

    GlobalControls.cpp
    Created: 26 Sep 2020 11:34:18am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "GlobalControls.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

GlobalControls::GlobalControls()
{
}

GlobalControls::~GlobalControls()
{
}

void GlobalControls::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   FLOATSLIDER(mZoomSlider, "zoom", &gDrawScale, .1f, 8.0f);
   FLOATSLIDER(mXSlider, "x pos", &(TheSynth->GetDrawOffset().x), -10000, 10000);
   FLOATSLIDER(mYSlider, "y pos", &(TheSynth->GetDrawOffset().y), -10000, 10000);
   ENDUIBLOCK(mWidth, mHeight);
}

void GlobalControls::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mZoomSlider->Draw();
   mXSlider->Draw();
   mYSlider->Draw();
}

void GlobalControls::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mZoomSlider)
   {
      //zoom in on center of screen
      float zoomAmount = (gDrawScale - oldVal) / oldVal;
      ofVec2f zoomCenter = ofVec2f(TheSynth->GetMouseX(), TheSynth->GetMouseY()) + TheSynth->GetDrawOffset();
      TheSynth->GetDrawOffset() -= zoomCenter * zoomAmount;
   }
}

void GlobalControls::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void GlobalControls::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void GlobalControls::SetUpFromSaveData()
{
}

void GlobalControls::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

vector<IUIControl*> GlobalControls::ControlsToNotSetDuringLoadState() const
{
   vector<IUIControl*> ignore;
   ignore.push_back(mZoomSlider);
   ignore.push_back(mXSlider);
   ignore.push_back(mYSlider);
   return ignore;
}

namespace
{
   const int kSaveStateRev = 0;
}

void GlobalControls::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;
}

void GlobalControls::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}
